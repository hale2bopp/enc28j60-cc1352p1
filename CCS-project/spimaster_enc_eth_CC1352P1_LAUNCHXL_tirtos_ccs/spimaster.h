/*
 * spimaster.h
 *
 *  Created on: Oct 28, 2019
 *      Author: sramnath
 */

#ifndef SPIMASTER_H_
#define SPIMASTER_H_

#include <stdint.h>

/* =========== SPI Access functions ==========
 *
 * ===========================================
 */

uint8_t setBitSetField(uint8_t address, uint8_t data);

uint8_t setBitClearField(uint8_t address, uint8_t data);

uint8_t setBufWriteToAddress(uint8_t address, uint8_t data);

uint8_t setBufReadFromAddress(uint8_t address);

void spi_write(uint8_t reg, uint8_t data);

uint8_t spi_read(uint8_t reg);

void systemSoftReset(void);

void bitFieldSet(uint8_t address, uint8_t data);

void bitFieldClear(uint8_t address, uint8_t data);

void sendRBMOpcode(void);

void sendWBMOpcode(void);

/* Higher functions */

void selectMemBank(uint8_t bank_no);




uint8_t spi_readMACReg(uint8_t reg);

/* ========== Functions meant only for Physical register =====
 *
 * ===========================================================
 */

void spi_writePHYReg(uint8_t address, uint8_t higher_bits, uint8_t lower_bits);

uint16_t spi_readPHYReg(uint8_t address);

/* =========== Register access functions ====
 *
 * ==========================================
 */


uint8_t whichBank(uint8_t reg);


/* ======== Test Functions ==========
 *
 * ==================================
 */

void writeBufferMemory(uint8_t* test_TxBuf, uint16_t address, uint8_t length);

void readBufferMemory(uint8_t* test_RxBuf, uint16_t address, uint8_t length);

void testReadWriteMemory(uint16_t address);

uint8_t readRevID(void);

void LEDA_On(void);

void LEDA_Off(void);

void LEDA_Blink(void);

void LEDA_Default(void);

void LEDB_On(void);

void LEDB_Off(void);

void setClock(uint8_t divider);

uint8_t regBankTest_ETH(uint8_t bank_no, uint8_t start, uint8_t end);

uint8_t regBankTest_MAC(uint8_t bank_no, uint8_t start, uint8_t end);

void regBankTest(void);

void test_clocks(void);


#endif /* SPIMASTER_H_ */
