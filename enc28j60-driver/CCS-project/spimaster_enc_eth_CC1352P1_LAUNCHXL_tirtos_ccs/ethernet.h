/*
 * ethernet.h
 *
 *  Created on: Oct 29, 2019
 *      Author: sramnath
 */

#ifndef ETHERNET_H_
#define ETHERNET_H_

void ethernetConfig(void);

void ethernet_initializeMAC(void);

void ethernet_initializePHY(void);

void ethernet_Init(void);

void ethernet_transmitPackets(char* payload, uint8_t msglen);

void ethernet_receivePackets(char* receiveBuffer, uint8_t msglen);


#endif /* ETHERNET_H_ */
