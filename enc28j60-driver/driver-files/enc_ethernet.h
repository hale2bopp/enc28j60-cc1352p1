/*
 * ethernet.h
 *
 *  Created on: Oct 29, 2019
 *      Author: sramnath
 */

#ifndef ENC_ETHERNET_H_
#define ENC_ETHERNET_H_

#include <stdint.h>
#include "spimaster.h"

/*! @brief function to configure Ethernet on the ENC28J60
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernetConfig(void);

/*! @brief function to initialize MAC registers on the ENC28J60
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_initializeMAC(void);


/*! @brief function to initialize PHY registers
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_initializePHY(void);


/*! @brief function to initialize ethernet on the ENC28J60, calls
 * ethernetConfig,ethernet_initializeMAC and ethernet_initializePHY
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_Init(void);


/*! @brief function to transmit packets to the dest MAC address
 * @param[in] payload    message payload
 * @param[in] msglen   length of message payload
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_transmitPackets(uint8_t* payload, uint16_t msglen);


/*! @brief function to enable the ENC28J60 to receive packets
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_receiveEnable(void);


/*! @brief function to disable the ENC28J60 to receive packets
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t ethernet_receiveDisable(void);


/*! @brief function to read a slice of the incoming packet
 * @param[in] dest         Destination buffer
 * @param[in] maxlength    Maximum number of bytes to read from packet
 * @param[in] packetOffset offset from start of packet to start reading
 * @return number of bytes to copy or ERR_DRIVER_FAIL for failure
 */
uint16_t readPacketSlice(char* dest, int16_t maxlength, int16_t packetOffset);


/*! @brief function to read the ethernet header
 * @param[in] header         Destination buffer
 * @return return the pointer to the next packet in the
 *         receive buffer or ERR_DRIVER_FAIL for failure
 */
uint16_t readEthHeader(uint8_t* header);


/*! @brief copy from the enc buffer
 * @param[in] dest         Destination buffer
 * @param[in] source       Source address
 * @param[in] num          number of elements to read from buffer
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t memcpy_from_enc(void* dest, uint16_t source, int16_t num);


/*! @brief function to get the length of the incoming packet
 * @param[in] header        Destination buffer for the header
 * @return 	 	    total length of packet to pass to lwIP or ERR_DRIVER_FAIL for failure
 */
uint16_t ethernet_getRecvLength(uint8_t* header);


/*! @brief function to receive packets from dest MAC
 * @param[in] receiveBuffer  	Buffer in which to receieve message
 * @param[in] len	    	length of packet to read
 * @return 			ERR_SUCCESS for success or ERR_DRIVER_FAIL for failure
 */
spierr_t ethernet_packetReceive(uint8_t* receiveBuffer, uint16_t len);




/* ============= Helper functions to clear buffer, peek at buffer, 
 *               calculate free space ============================
 */

/*! @brief function to calculate free space
 */
uint16_t ethernet_calcfreeSpaceBuffer(void);


/*! @brief function to clear the receive buffer on ENC28J60
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t clearRxBuf(void);


/*! @brief function to clear the transmit buffer on ENC28J60
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t clearTxBuf(void);


/*! @brief function to clear the entire buffer
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t clearWholeBuf(void);


/*! @brief function to dump contents of the receive buffer onto display
 * @param[in] receiveBuffer  Buffer in which to receieve contents
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t RxBufDump(uint8_t *receiveBuffer);


/*! @brief function to peek at the first x values in the receive buffer
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t justATest(void);


/*! @brief function to peek at a slice of contents in the buffer
 *  @return     ERR_SUCCESS if success, ERR_DRIVER_FAIL if failure
 */
spierr_t bufferSliceRead(void);

#endif /* ENC_ETHERNET_H_ */
