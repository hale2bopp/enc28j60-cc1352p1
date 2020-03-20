/*
 * ethernet.c
 *
 *  Created on: Oct 28, 2019
 *      Author: sramnath
 */


//#include "ethernet.h"
#include "registerlib.h"
#include "spimaster.h"
#include <stdint.h>
#include <stdlib.h>
//#include "spimaster.h"

/* ======== Ethernet GLobals ========
 *
 * ==================================
 */

//static uint8_t mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static uint8_t mymac[] = { 0x70,0x69,0x69,0x2D,0x30,0x31};
static uint8_t destmac[] = {0xb0,0x0c,0xd1,0x4c,0x15,0x5f};
static uint8_t myip[] = { 192,168,1,203};
static uint8_t serverip[] = {169,254,147,160};


/* ======== Ethernet Functions =======
 *
 * ===================================
 */

void ethernetConfig(void){
    /* Initialize the receive buffer by programming ERXST and ERXND pointers */
    /* All memory between and including the two addresses will be dedicated to receive hardware */
    /* Let's configure it for now to allocate half the memory to the transmit buffer and half to the receive buffer */
    /* Recommended to program ERXST pointer with an even address so I'll give it 0x1002*/

    /* 0x1FFF is the address space available : 0 - 0x1000 for Transmit buffer, and 0x1001 - 0x1fff for receive buffer */
    /* Write to ERXSTH (bank 0, 0x9 :   and to ERXSTL bank0, 0x8:     - lower bound i.e. 0x1001 */
//    selectMemBank(0);
    spi_write(ERXSTL,0x02);
    spi_write(ERXSTH,0x10);

    /* Write to ERXNDH bank 0 0x0b    and ERXNDL bank0 0x0a   registers the value 0x1fff */
    spi_write(ERXNDL,0xff);
    spi_write(ERXNDH,0x1f);

    /* Program the ERXRDPT to the same value as ERXST. */
    /* First write to ERXRDPTL bank 0 0xc, then ERXRDPTH bank 0 0xd*/
    /* ERXRDPTL */
    spi_write(ERXRDPTL, 0x02);

    /* ERXRDPTH */
    spi_write(ERXRDPTH, 0x10);

    /* Memory not used by the receive buffer is part of the transmission buffer, so in this case 0x0 to 0x1002 */
    /* No need to initialize the transmission buffer */

    /* Enable the appropriate receive filters by writing to ERXFCON register  */
    /* Unicast
        • Pattern Match
        • Magic Packet™
        • Hash Table
        • Multicast
        • Broadcast
     */
    /* Promiscuous mode - clear the ERXFCON - bank 1, 0x18 */
//    selectMemBank(1);
    spi_write(ERXFCON,0);


}


void ethernet_initializeMAC(void){
    /* Configure the MAC registers */
    /* 1. Set the MARXEN bit in MACON1 to enable MAC to receive frames. Also set RxPAUS and TxPAUS*/
    /* MACON1 : bank 2 0x0 */
    selectMemBank(2);
    uint8_t macon1val = spi_readMACReg(MACON1);
    spi_write(MACON1, (macon1val & 0x12) | 0xD);

    /* 2. Configure the PADCFG, TXCRCEN and FULDPX bits of MACON3
     * MACON3 : bank 2, 0x2
     * */
    uint8_t macon3val = spi_readMACReg(MACON3);
    /* 0b 0000 1110 */
    /* 0b 001 1 000 1  */
    /*
     * Configuration : pad atleast 60 bits and add a CRC, regardless of the PDCFG bits, and full-duplex
     */
    spi_write(MACON3, (macon3val & 0xe) | 0x31) ;

    /* If in Full-Duplex mode, PDPXMD in PHCON1 must also be set */
    uint16_t phcon1val = spi_readPHYReg(PHCON1);
    uint16_t relevant_phcon1 = phcon1val & 0xfeff | (~0xfeff);
    spi_writePHYReg(PHCON1, relevant_phcon1 & 0xff00 >> 8 , relevant_phcon1 & 0x00ff);

    /* 3. Configure the bits in MACON4 - set the DEFER bit to conform to IEEE 802.3 standard
     * MACON4 : bank2 , 0x3
     * set DEFER bit, ;eave the rest as before. then, read mask = 0b01000000
     */
    uint8_t macon4val = spi_readMACReg(MACON4);
    spi_write(MACON4, macon4val & ~(0x40) | (0x40));

    /* 4. Program the MAMXFL registrs with max frame length to be permitted to be received or transmitted.
     * Normally - network nodes are designed to handle packets that are 1518 packets or less
     * MAMXFLL: bank 2, 0x0a, MAMXFLH bank2 , 0x0b
     * MAMXFLL : 0xee
     * MAMXFLH : 0x05
     */
    spi_write(MAMXFLL, 0xee);
    spi_write(MAMXFLH, 0x05);

    /* 5. Configure the Back-To-Back Inter-Packet Gap register, MABBIPG. most apps will program
     * this register with 0x15 when Full-Duplex mode is used and 0x12 when half-duplex mode is used.
     * MABBIPG: bank 2, 0x4
     * write the value 0x15
     *
     */
    spi_write(MABBIPG,0x15);

    /* 6. Configure the Non-Bank-To-Bank Inter-Packet Gapregister low byte, MAIPGL.
     * Most apps will configure as 0x12
     * MAIPGH : bank2, 0x6
     */
    spi_write(MAIPGL,0x12);

    /* 7. If half-duplex mode is used, non-back-to-back inter-packet gap register high byte, MAIPGH,
     * should be programmed, and most applications will program the register to 0xc. Leave it unprogrammed
     * since you have programmed this application as full-duplex.
     */

    /* 8. If half-duplex mode is used, program retransmission and collision window registers. MACLCON1 and MACLCON2.
     * most apps will not change default reset values. Leave this alone.
     */

    /* 9. Program the local MAC address into the MAADR1:MAADR6 registers.
     * local MAC address :
     * Bank3
     * MAADR1 : 0x4, MAADR2 : 0x5, MAADR3: 0x2, MAADR4 0x3, MAADR5 0x0, MAADR6 0x1
     * set the MAC to whatever you choose : I've set it to mymac.
     */
    selectMemBank(3);
    spi_write(MAADR1,mymac[0]);
    spi_write(MAADR2,mymac[1]);
    spi_write(MAADR3,mymac[2]);
    spi_write(MAADR4,mymac[3]);
    spi_write(MAADR5,mymac[4]);
    spi_write(MAADR6,mymac[5]);
}


/* function: ethernet_initializePHY
 *
 */
void ethernet_initializePHY(void){
    /* 1. PHCON1.PDPXMD bit may have to be configured (done in the MAC initialize function)
     */

    /* 2. If using half-duplex, host controller may wish to set the PHCON2.HDLDIS bit to prevent automatic loopback of the data
     * which is transmitted
     */

    /* 3. If the app requires LED config other than default, PHLCON must be altered to match new requirements.
     * Use defaults for now.
     */
}

void ethernet_Init(void){
    ethernetConfig();
    ethernet_initializeMAC();
    ethernet_initializePHY();
}

void ethernet_transmitPackets(char* payload, uint8_t msglen){
    /* Single per-packet control byte required to precede the packet for transmission
     * To be written into transmit buffer */

    /* PHUGEEN PPADEN PCRCEN POVERRIDE */
    /* Let MACON3 set everything - POVERRIDE = 1 */
    /* 0b 0000 0000 */

    /* 1. Program the ETXST pointer to point to an unused location in memory */
    /* Will point to the per packet control byte (which will be written in memory
     * for example, 0x0120. Recommended to use an even address.
     * ETXSTL :0x20    ETXSTH : 0x01
     */
    uint16_t start_addr = 0x0112;
    uint8_t start_addr_l = (start_addr) & 0x00ff;
    uint8_t start_addr_h = ((start_addr) & 0xff00) >> 8;
    spi_write(ETXSTL, start_addr_l);
    spi_write(ETXSTH, start_addr_h);


    /* 2. Use the WBM SPI command to write the per packet control byte, the destination address,
     * the source MAC address, the type/length and the data payload
     */

    uint8_t length = 1+6+6+1+msglen;
//    uint8_t payload[56];
//    int i;
//    for(i=0;i<56;i++)
////        payload[i]= rand()%100;
//        payload[i]= 3;

    uint8_t* transmitBuffer = (uint8_t*) malloc(sizeof(uint8_t)*length);
    transmitBuffer[0] = 0;
    memcpy(transmitBuffer+1,destmac,sizeof(uint8_t)*6);
    memcpy(transmitBuffer+7,mymac,sizeof(uint8_t)*6);
    transmitBuffer[14] = msglen;
    memcpy(transmitBuffer+15,payload,sizeof(char)*msglen);
    writeBufferMemory(transmitBuffer,start_addr,length);


    /* 3. Appropriately program the ETXND pointer, points to the last byte
     * in the data payload. ex, to 0x0156. Length is 54 bytes
     */
    uint16_t end_addr = start_addr + length;
    uint8_t end_addr_l = (end_addr) & 0x00ff;
    uint8_t end_addr_h = ((end_addr) & 0xff00) >> 8;

    spi_write(ETXNDL, end_addr_l);
    spi_write(ETXNDH, end_addr_h);

    /* 4. Clear EIR.TXIF, set EIE.TXIE, set EIE.INTIE to enable an interrupt
     * when done (if desired)
     */
    selectMemBank(0);
    bitFieldClear(0x1c, 0x08);
    bitFieldSet(0x1b,0x88);

    /* 5. Start the transmission process by setting ECON1.TXRTS
     *
     */
    /* Start transmission */
    bitFieldSet(0x1f,0x8);
}


void ethernet_receivePackets(char* receiveBuffer, uint8_t msglen){
    /* 1. If interrupt is desired whenever packet is received, set EIE.PKTIE,
     * EIE.INTIE
     */
    selectMemBank(0);
    bitFieldSet(0x1b, 0xC0);

    /* 2. If an interrupt is desired whenever a packet is dropped due to
     * insufficient buffer space, clear the EIR.RXERIF and set both the
     * EIE.RXERIE and EIE.INTIE.
     */

    bitFieldClear(0x1c, 0x1);
    bitFieldSet(0x1b, 0x81);

    /* 3. Enable reception by setting ECON1.RXEN
     */

    bitFieldSet(0x1f,0x04);

    // to actually read the received packets, I may actually have to figure out where the read pointer is
    uint8_t read_address_l = spi_read(ERDPTL);
    uint8_t read_address_h = spi_read(ERDPTH);
    uint8_t read_address = read_address_h<<8 | read_address_l;
    readBufferMemory(receiveBuffer, read_address, msglen);

}

