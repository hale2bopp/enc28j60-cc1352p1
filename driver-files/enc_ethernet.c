/*
 * ethernet.c
 *
 *  Created on: Oct 28, 2019
 *      Author: sramnath
 */


#include "enc_ethernet.h"
#include "registerlib.h"
#include "spimaster.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ti/display/Display.h>

extern Display_Handle display;

/* ======== Ethernet GLobals ========
 *
 * ==================================
 */


static uint8_t mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31};

static uint16_t numPackets = 0;
static uint16_t gnextPacketPtr;
uint16_t nextpktptr;
uint32_t status;
/* ======== Ethernet Defines =======
 *
 * ===================================
 */

#define RXSTART_INIT 0x0000
#define RXSTOP_INIT  0x0BFF
#define TXSTART_INIT 0x0C00
#define TXSTOP_INIT  0x11FF


/* ======== Ethernet Functions =======
 *
 * ===================================
 */



/*! @brief function to configure Ethernet on the ENC28J60
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t ethernetConfig(void){
    /* Soft Reset before starting anything */
    if (systemSoftReset() != ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    sleep(2);
    while(!(spi_read(ESTAT) & 0x01));

    /* Initialize the receive buffer by programming ERXST and ERXND pointers */
    /* All memory between and including the two addresses will be dedicated to receive hardware */
    /* Let's configure it for now to allocate half the memory to the transmit buffer and half to the receive buffer */
    /* Recommended to program ERXST pointer with an even address so I'll give it 0x1002*/

    /* 0x1FFF is the address space available : 0 - 0x1000 for Transmit buffer, and 0x1001 - 0x1fff for receive buffer */
    /* Write to ERXSTH (bank 0, 0x9 :   and to ERXSTL bank0, 0x8:     - lower bound i.e. 0x1001 */
    if(spi_write(ERXSTL,(RXSTART_INIT+1) & 0x00ff) != ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(ERXSTH,((RXSTART_INIT+1) & 0xff00)>>8) != ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* Write to ERXNDH bank 0 0x0b    and ERXNDL bank0 0x0a   registers the value 0x1fff */
    if(spi_write(ERXNDL,RXSTOP_INIT & 0x00ff)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(ERXNDH,(RXSTOP_INIT & 0xff00)>>8)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* Program the ERXRDPT to the same value as ERXST. */
    /* First write to ERXRDPTL bank 0 0xc, then ERXRDPTH bank 0 0xd*/
    /* ERXRDPTL */
    if(spi_write(ERXRDPTL, (RXSTART_INIT+1) & 0x00ff)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* ERXRDPTH */
    if(spi_write(ERXRDPTH, ((RXSTART_INIT+1) & 0xff00)>>8)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* ETXST */
    if(spi_write(ETXSTL,TXSTART_INIT & 0x00ff) != ERR_SUCCESS)
        return ERR_DRIVER_FAIL;
    if(spi_write(ETXSTH,(TXSTART_INIT & 0xff00)>>8) != ERR_SUCCESS)
        return ERR_DRIVER_FAIL;

    /* ETXND */
    if(spi_write(ETXNDL,TXSTOP_INIT & 0x00ff) != ERR_SUCCESS)
        return ERR_DRIVER_FAIL;
    if(spi_write(ETXNDH,(TXSTOP_INIT & 0xff00)>>8) != ERR_SUCCESS)
        return ERR_DRIVER_FAIL;


    /* EWRPT */
    if(spi_write(EWRPTL,TXSTART_INIT & 0x00ff) != ERR_SUCCESS)
        return ERR_DRIVER_FAIL;
    if(spi_write(EWRPTH,(TXSTART_INIT & 0xff00)>>8) != ERR_SUCCESS)
        return ERR_DRIVER_FAIL;
 	
    /* Memory not used by the receive buffer is part of the transmission buffer, so in this case 0x0 to 0x1002 */
    /* No need to initialize the transmission buffer */

    /* Enable the appropriate receive filters by writing to ERXFCON register  */
    /*  • Unicast
        • Pattern Match
        • Magic Packet™
        • Hash Table
        • Multicast
        • Broadcast
     */
    /* Promiscuous mode - clear the ERXFCON - bank 1, 0x18 */
    /* UCEN : 1 (UNICAST) , ANDOR: 0 (OR), CRCEN: 0, PMEN: 0, MPEN: 0, HTEN: 0, MCEN: 0, BCEN: 1  */	
    /* 0b1000 0001: 0x81 */
    if(spi_write(ERXFCON,0x81)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    return ERR_SUCCESS;
}

/*! @brief function to initialize MAC registers on the ENC28J60
 *  @return 	ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_initializeMAC(void){
    /* Configure the MAC registers */
    /* 1. Set the MARXEN bit in MACON1 to enable MAC to receive frames. Also set RxPAUS and TxPAUS*/
    /* MACON1 : bank 2 0x0 */
    selectMemBank(2);
    uint8_t macon1val = spi_readMACReg(MACON1);
    if(spi_write(MACON1, (macon1val & 0x12) | 0xD)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 2. Configure the PADCFG, TXCRCEN and FULDPX bits of MACON3
     * MACON3 : bank 2, 0x2
     * */
    uint8_t macon3val = spi_readMACReg(MACON3);
    /* 0b 0000 1110 */
    /* 0b 001 1 000 1  */
    /*
     * Configuration : pad atleast 60 bits and add a CRC, regardless of the PDCFG bits, and full-duplex
     */
    if(spi_write(MACON3, (macon3val & 0xe) | 0x31)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* If in Full-Duplex mode, PDPXMD in PHCON1 must also be set */
    uint16_t phcon1val = spi_readPHYReg(PHCON1);
    uint16_t relevant_phcon1 = phcon1val & 0xfeff | (~0xfeff);
    if(spi_writePHYReg(PHCON1, relevant_phcon1 & 0xff00 >> 8 , relevant_phcon1 & 0x00ff) != ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 3. Configure the bits in MACON4 - set the DEFER bit to conform to IEEE 802.3 standard
     * MACON4 : bank2 , 0x3
     * set DEFER bit, leave the rest as before. then, read mask = 0b01000000
     */
    uint8_t macon4val = spi_readMACReg(MACON4);
    if(spi_write(MACON4, macon4val & ~(0x40) | (0x40))!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 4. Program the MAMXFL registrs with max frame length to be permitted to be received or transmitted.
     * Normally - network nodes are designed to handle packets that are 1518 packets or less
     * MAMXFLL: bank 2, 0x0a, MAMXFLH bank2 , 0x0b
     * MAMXFLL : 0xee
     * MAMXFLH : 0x05
     */
    if(spi_write(MAMXFLL, 0xee)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(MAMXFLH, 0x05)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 5. Configure the Back-To-Back Inter-Packet Gap register, MABBIPG. most apps will program
     * this register with 0x15 when Full-Duplex mode is used and 0x12 when half-duplex mode is used.
     * MABBIPG: bank 2, 0x4
     * write the value 0x15
     *
     */
    if(spi_write(MABBIPG,0x15)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 6. Configure the Non-Bank-To-Bank Inter-Packet Gapregister low byte, MAIPGL.
     * Most apps will configure as 0x12
     * MAIPGH : bank2, 0x6
     */
    if(spi_write(MAIPGL,0x12)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

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
    if (spi_write(MAADR1,mymac[0])!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(MAADR2,mymac[1])!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(MAADR3,mymac[2])!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(MAADR4,mymac[3])!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(MAADR5,mymac[4])!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(MAADR6,mymac[5])!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* set the next packet pointer to position zero in the receive buffer */
    gnextPacketPtr = RXSTART_INIT;
    return ERR_SUCCESS;
}


/*! @brief function to initialize PHY registers
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t ethernet_initializePHY(void){
    /* 1. PHCON1.PDPXMD bit may have to be configured (done in the MAC initialize function)
     */

    /* 2. If using half-duplex, host controller may wish to set the PHCON2.HDLDIS bit to prevent automatic loopback of the data
     * which is transmitted
     */

    /* 3. If the app requires LED config other than default, PHLCON must be altered to match new requirements.
     * Use defaults for now.
     */
     if(LED_Default()!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
}


/*! @brief function to initialize ethernet on the ENC28J60, calls 
 * ethernetConfig,ethernet_initializeMAC and ethernet_initializePHY
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t ethernet_Init(void){
    
    if(ethernetConfig()!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(ethernet_initializeMAC()!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(ethernet_initializePHY()!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(ethernet_receiveEnable()!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    return ERR_SUCCESS;
}


/*! @brief function to transmit packets to the dest MAC address 
 * @param[in] char* payload    message payload
 * @param[in] uint16_t msglen   length of message payload
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t ethernet_transmitPackets(uint8_t* payload, uint16_t msglen){
    /* Indicate that ethernet transmit is about to start */
    
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
    uint16_t start_addr = TXSTART_INIT ;
    uint8_t start_addr_l = (start_addr) & 0x00ff;
    uint8_t start_addr_h = ((start_addr) & 0xff00) >> 8;
    if(spi_write(ETXSTL, start_addr_l)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(ETXSTH, start_addr_h)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;


    /* 2. Use the WBM SPI command to write the per packet control byte, the destination address,
     * the source MAC address, the type/length and the data payload
     */

     	
    if(writeBufferMemory(payload,start_addr,msglen)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;


    /* 3. Appropriately program the ETXND pointer, points to the last byte
     * in the data payload. ex, to 0x0156. Length is 54 bytes
     */
    uint16_t end_addr = start_addr + msglen;
    uint8_t end_addr_l = (end_addr) & 0x00ff;
    uint8_t end_addr_h = ((end_addr) & 0xff00) >> 8;

    if(spi_write(ETXNDL, end_addr_l)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(ETXNDH, end_addr_h)!=ERR_SUCCESS)	
	return ERR_DRIVER_FAIL;

    /* 4. Clear EIR.TXIF, set EIE.TXIE, set EIE.INTIE to enable an interrupt
     * when done (if desired)
     */
    if(selectMemBank(0)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(bitFieldClear(0x1c, 0x08)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(bitFieldSet(0x1b,0x88)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 5. Start the transmission process by setting ECON1.TXRTS
     *
     */
    /* Start transmission */
    if(bitFieldSet(0x1f,0x8)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    return ERR_SUCCESS;
}



/*! @brief function to enable the ENC28J60 to receive packets 
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t ethernet_receiveEnable(void){
    /* 1. If interrupt is desired whenever packet is received, set EIE.PKTIE,
     * EIE.INTIE
     */
    if(selectMemBank(0)!=ERR_SUCCESS)	
	return ERR_DRIVER_FAIL;
    if(bitFieldSet(0x1b, 0xC0)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 2. If an interrupt is desired whenever a packet is dropped due to
     * insufficient buffer space, clear the EIR.RXERIF and set both the
     * EIE.RXERIE and EIE.INTIE.
     */

    if(bitFieldClear(0x1c, 0x1)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(bitFieldSet(0x1b, 0x81)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 3. Set ECON2.AUTOINC */
    if(bitFieldSet(0x1e, 0x80)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    /* 3. Enable reception by setting ECON1.RXEN
     */
    if(bitFieldSet(0x1f,0x04)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    return ERR_SUCCESS;
}

/*! @brief function to disable the ENC28J60 to receive packets
 * @param[in] none
 * @return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t ethernet_receiveDisable(void){
    /* 3. Disable reception by clearing ECON1.RXEN
     */
    if(bitFieldClear(0x1f,0x04)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    return ERR_SUCCESS;
}


/*! @brief function to read a slice of the incoming packet
 * @param[in] dest		Destination buffer
 * @param[in] maxLength	Maximum number of bytes to read from packet
 * @param[in] packetOffset	offset from start of packet to start reading
 * @return number of bytes to copy
 */
uint16_t readPacketSlice(char* dest, int16_t maxlength, int16_t packetOffset){
    uint8_t erxrdptL = spi_read(ERXRDPTL);
    uint8_t erxrdptH = spi_read(ERXRDPTH);
    uint16_t erxrdpt = erxrdptH << 8 | erxrdptL;
    int16_t packetLength;

    memcpy_from_enc((char*) &packetLength, (erxrdpt+18)%(RXSTOP_INIT+1), 2);
    packetLength -= 4; // remove crc

    int16_t bytesToCopy = packetLength - packetOffset;
    if (bytesToCopy > maxlength) bytesToCopy = maxlength;
    if (bytesToCopy <= 0) bytesToCopy = 0;

    int16_t startofSlice = (erxrdpt+7+4+packetOffset)%(RXSTOP_INIT+1);
    if(memcpy_from_enc(dest, startofSlice, bytesToCopy)!=ERR_SUCCESS)
	return (uint16_t) ERR_DRIVER_FAIL;
    dest[bytesToCopy] = 0;

    return bytesToCopy;

}


/*! @brief function to read the ethernet header
 * @param[in] header         Destination buffer
 * @return return pointer to the next packet in the 
 * 	   receive buffer, if failure return ERR_DRIVER_FAIL 
 */
uint16_t readEthHeader(uint8_t* header){
    /* Read the 14 byte header */
    if(memcpy_from_enc(header, (gnextPacketPtr)%(RXSTOP_INIT+1), 6+6+6+6)!=ERR_SUCCESS)
	return (uint16_t) ERR_DRIVER_FAIL;
    return (header[1] << 8 | header[0]);
}

/*! @brief copy from the enc buffer
 * @param[in] dest 	Destination buffer
 * @param[in] source 	Source address
 * @param[in] num	number of elements to read from buffer
 * @return		ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t memcpy_from_enc(void* dest, uint16_t source, int16_t num) {
    if(readBufferMemory((uint8_t*) dest, source,  num)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    return ERR_SUCCESS;
}


/*! @brief function to get the length of the incoming packet
 * @param[in] pkthdr	Destination buffer for the header
 * @return 		total length of packet to pass to lwIP, ERR_DRIVER_FAIL on failure
 */
uint16_t ethernet_getRecvLength(uint8_t pkthdr[24]){
   /* If it is the first packet received, I can't for some reason find the next packet pointer */
   nextpktptr = readEthHeader(pkthdr);
   /* If failure */
   if(nextpktptr == ERR_DRIVER_FAIL)
	return (uint16_t) ERR_DRIVER_FAIL;
   uint16_t len, type;
//   if (numPackets == 0){
//	nextpktptr = 70;
	
//	status =  pkthdr[4] << 24 | pkthdr[3] << 16 | pkthdr[2]<< 8 | pkthdr[1];

//    	uint8_t dest_mac[6];
//    	uint8_t src_mac[6];

//    	memcpy(dest_mac, pkthdr+5, 6);
//    	memcpy(src_mac, pkthdr+11, 6);
	
//	type =  pkthdr[21] << 8 | pkthdr[22];
//    	if (type == 0x0800)
//    	    len = 64;
//    	else
//    	    len = type;	
//    	
//    }
//    else{
        status =  pkthdr[5] << 24 | pkthdr[4] << 16 | pkthdr[3]<< 8 | pkthdr[2];

        uint8_t dest_mac[6];
        uint8_t src_mac[6];

        memcpy(dest_mac, pkthdr+6, 6);
        memcpy(src_mac, pkthdr+12, 6);

        type =  pkthdr[22] << 8 | pkthdr[23];
        if (type == 0x0800)
            len = 64;
        else
            len = type;
        
//    }	
   
    uint8_t lastVal;
    memcpy_from_enc(&lastVal, 3071, 1);
    Display_printf(display,0,0,"LastVal : %d", lastVal);		 
    /* Add the length of the ethernet header (14 bytes) to the computed length */
    return len+14;
}


/*! @brief function to receive packets from dest MAC
 * @param[in] char* receiveBuffer  Buffer in which to receieve message
 * @return length, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_packetReceive(uint8_t* receiveBuffer, uint16_t len){
    /* If the number of packets to be read is >0 */
    uint8_t RxRdPtrL = spi_read(ERXRDPTL);
    uint8_t RxRdPtrH = spi_read(ERXRDPTH);
    uint16_t RxRdPt = RxRdPtrH<<8 | RxRdPtrL;
    if (numPackets ==0){
//    	gnextPacketPtr+=5;
	if(spi_write(ERXRDPTL,(gnextPacketPtr) & 0x00ff)!=ERR_SUCCESS)
                return ERR_DRIVER_FAIL;
        if(spi_write(ERXRDPTH,((gnextPacketPtr) & 0xff00 )>>8)!=ERR_SUCCESS)
                return ERR_DRIVER_FAIL;
    }
//    else
	gnextPacketPtr+=6;	
        
	if (len ==0)
            return  ERR_DRIVER_FAIL;
        else{
            if(readBufferMemory(receiveBuffer, gnextPacketPtr, len)!=0)
		return  ERR_DRIVER_FAIL;
            receiveBuffer[len] = 0;
        }
        gnextPacketPtr = nextpktptr-1;
//        gnextPacketPtr = nextpktptr;
//        if(gnextPacketPtr-1 > RXSTOP_INIT){
        if(gnextPacketPtr > RXSTOP_INIT){
            if(spi_write(ERXRDPTL,RXSTOP_INIT & 0x00ff)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
            if(spi_write(ERXRDPTH,(RXSTOP_INIT & 0xff00 )>>8)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
        }
        else{
            if(spi_write(ERXRDPTL,(gnextPacketPtr ) & 0x00ff)!=ERR_SUCCESS)
//            if(spi_write(ERXRDPTL,(gnextPacketPtr -1) & 0x00ff)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
            if(spi_write(ERXRDPTH,((gnextPacketPtr ) & 0xff00) >> 8 )!=ERR_SUCCESS)
//            if(spi_write(ERXRDPTH,((gnextPacketPtr -1) & 0xff00) >> 8 )!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
        }

        /* set ECON2.PKTDEC */
        if(selectMemBank(0)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
        if(bitFieldSet(0x1e, 0x40)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
//    }
    numPackets++;
    return ERR_SUCCESS;
}


/* ============= Helper functions to clear buffer, peek at buffer,   
 * 		 calculate free space ============================
 */

/*! @brief function to clear the receive buffer on ENC28J60
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t clearRxBuf(void){
    uint16_t num_units = (RXSTOP_INIT-RXSTART_INIT)/10;
    uint16_t unit_len = (RXSTOP_INIT-RXSTART_INIT)/num_units;
    Display_printf(display,0,0,"Length is : %d, num_units: %d, unit_len : %d", RXSTOP_INIT-RXSTART_INIT, num_units, unit_len);
    uint8_t test_TxBuf[10];
    int i;
    memset(test_TxBuf, 0, unit_len);
    for(i=0;i<num_units;i++){
	if(writeBufferMemory(test_TxBuf, unit_len*i, unit_len)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
    }
    /* And the last one */
    
    /* Restore parameters to initial states */
    return ERR_SUCCESS;
}


/*! @brief function to clear the transmit buffer on ENC28J60
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t clearTxBuf(void){
    //uint16_t length = TXSTOP_INIT - TXSTART_INIT;
    uint16_t num_units = (TXSTOP_INIT-TXSTART_INIT)/10;
    uint16_t unit_len = (TXSTOP_INIT-TXSTART_INIT)/num_units;
    Display_printf(display,0,0,"Length is : %d, num_units: %d, unit_len : %d", TXSTOP_INIT-TXSTART_INIT, num_units, unit_len);
    uint8_t test_TxBuf[10];
    int i;
    memset(test_TxBuf, 0, unit_len);
    for(i=0;i<num_units;i++){
        if(writeBufferMemory(test_TxBuf, unit_len*i, unit_len)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
    }	
    return ERR_SUCCESS;
}


/*! @brief function to clear the entire buffer
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t clearWholeBuf(void){
    uint16_t num_units = (TXSTOP_INIT-RXSTART_INIT)/10;
    uint16_t unit_len = (TXSTOP_INIT-RXSTART_INIT)/num_units;
    Display_printf(display,0,0,"Length is : %d, num_units: %d, unit_len : %d", TXSTOP_INIT-RXSTART_INIT, num_units, unit_len);
    uint8_t test_Buf[10];
    int i;
    memset(test_Buf, 0, unit_len);
    for(i=0;i<num_units;i++){
        if(writeBufferMemory(test_Buf, unit_len*i, unit_len)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
    }
    return ERR_SUCCESS;
}


/*! @brief function to calculate free space
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
uint16_t ethernet_calcfreeSpaceBuffer(void){
    uint8_t num_packets = spi_read(EPKTCNT);
    uint8_t ReadWrtPtrL = spi_read(ERXWRPTL);
    uint8_t ReadWrtPtrH = spi_read(ERXWRPTH);
    /* Assure that you get a matching set of RdWrPTL and H bytes */
    while (spi_read(EPKTCNT)!= num_packets){
        num_packets = spi_read(EPKTCNT);
        ReadWrtPtrL = spi_read(ERXWRPTL);
        ReadWrtPtrH = spi_read(ERXWRPTH);
    }

    uint8_t RxRdPtrL = spi_read(ERXRDPTL);
    uint8_t RxRdPtrH = spi_read(ERXRDPTH);
    uint16_t RxRdPt = RxRdPtrH<<8 | RxRdPtrL;
    uint16_t ReadWrPtr = ReadWrtPtrH << 8 | ReadWrtPtrL;
    uint16_t FreeSpace;

    uint8_t RxNdL = spi_read(ERXNDL);
    uint8_t RxNdH = spi_read(ERXNDH);
    uint16_t RxNd = RxNdH << 8 | RxNdL;

    uint8_t RxStL = spi_read(ERXSTL);
    uint8_t RxStH = spi_read(ERXSTH);
    uint16_t RxSt = RxStH << 8 | RxStL;

    if (ReadWrPtr > RxRdPt){
        FreeSpace = (RxNd - RxSt) - (ReadWrPtr - RxRdPt);
    }
    else{
        FreeSpace = RxRdPt - ReadWrPtr -1;
    }
    return FreeSpace;


}

/*! @brief function to dump contents of the receive buffer onto display
 * @param[in] char* bufferMemoryContents  Buffer in which to receieve contents
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t RxBufDump(uint8_t* bufferMemoryContents){
    uint16_t length = RXSTOP_INIT-RXSTART_INIT;	
    uint16_t unit = length/32;	
    /* Read the entire contents of memory in 8 parts */
    int i,j;
    for (i=0;i<32;i++){
    	if(readBufferMemory(bufferMemoryContents, unit*i,unit)!=ERR_SUCCESS)
		return ERR_DRIVER_FAIL;
	for (j=0;j<unit;j++)
		Display_printf(display,0,0,"buffer[%d] : %x",unit*i+j, bufferMemoryContents[j] );
	memset(bufferMemoryContents,0,unit);
    }
    /* Restore all parameters to initial states */
    return ERR_SUCCESS;	
}


/*! @brief function to peek at the first x values in the receive buffer
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t justATest(void){
    uint16_t unit = 64;//(RXSTOP_INIT-RXSTART_INIT)/32;
    /* Read the entire contents of memory in 8 parts */
    int j;
    uint8_t bufferMemoryContents[(RXSTOP_INIT-RXSTART_INIT)/32];
    if(readBufferMemory(bufferMemoryContents, 0,unit)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    for (j=0;j<unit;j++)
	Display_printf(display,0,0,"buffer[%d] : %x",j, bufferMemoryContents[j] );
    memset(bufferMemoryContents,0,unit);
    uint8_t lastValue;	
    if(readBufferMemory(&lastValue, 3070, 1)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
 
	Display_printf(display,0,0,"buffer[%d] : %x",3070, bufferMemoryContents[3070] );
    /* Restore all parameters to initial states */
    return ERR_SUCCESS;
}



void ethernet_dropPacket(void){
	


}


/*! @brief function to peek at a slice of contents in the buffer
 *  @return 	ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t bufferSliceRead(void){
    uint16_t unit = 64;//(RXSTOP_INIT-RXSTART_INIT)/32;
    /* Read the entire contents of memory in 8 parts */
    int j;
    uint8_t bufferMemoryContents[(RXSTOP_INIT-RXSTART_INIT)/32];
    if(readBufferMemory(bufferMemoryContents, 64,unit)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    for (j=0;j<unit;j++)
        Display_printf(display,0,0,"buffer[%d] : %x",j, bufferMemoryContents[j] );
    memset(bufferMemoryContents,0,unit);
    uint8_t lastValue;
    if(readBufferMemory(&lastValue, 3070, 1)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    return ERR_SUCCESS;
}
