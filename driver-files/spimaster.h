/*
 * spimaster.h
 *
 *  Created on: Oct 28, 2019
 *      Author: sramnath
 */

#ifndef SPIMASTER_H_
#define SPIMASTER_H_

#include <stdint.h>
#define spierr_t uint8_t

enum errors{
	ERR_SUCCESS = 0,
	ERR_DRIVER_FAIL = -1,
	ERR_TEST_FAIL = -2
};

/* =========== SPI Access functions ==========
 *
 * ===========================================
 */

/*! @brief Helper Function to set bit(s) in a register
 *  @param[in] address     address of register
 *  @param[in] data        position of the bit to be set, e.g. to set third bit, send in 0b00000100
 *  @return             not important
 */
uint8_t setBitSetField(uint8_t address, uint8_t data);


/*! @brief Helper Function to clear bit(s) in a register
 *  @param[in] address     address of register
 *  @param[in] data        position of the bit to be clear, e.g. to clear third bit, send in 0b00000100
 *  @return             not important
 */
uint8_t setBitClearField(uint8_t address, uint8_t data);


/*! @brief Helper function for spi_write/read, set the master Rx and Tx buffers
 *  @param[in] address     address of register
 *  @param[in] data        data to write to address
 *  @return             controlWord which is the word written to the Rx/Tx buffer
 */
uint8_t setBufWriteToAddress(uint8_t address, uint8_t data);


/*! @brief Helper function for spi_write/read, set the master Rx and Tx buffers
 *  @param[in] address     address of register
 *  @return             controlWord which is the word written to the Rx/Tx buffer
 */
uint8_t setBufReadFromAddress(uint8_t address);


/*! @brief perform spi write operation to specified register
 *  @param[in] reg         name of register
 *  @param[in] data        data to write to register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t spi_write(uint8_t reg, uint8_t data);


/*! @brief perform spi read operation from specified register
 *  @param[in] reg         name of register
 */
uint8_t spi_read(uint8_t reg);


/*! @brief perform software reset of ENC28J60
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t systemSoftReset(void);


/*! @brief perform bit set operation for address
 *  @param[in] address     address of register
 *  @param[in] data        data to write to address
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t bitFieldSet(uint8_t address, uint8_t data);


/*! @brief perform bit clear operation for address
 *  @param[in] address    	address of register
 *  @param[in] data     	data to write to register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t bitFieldClear(uint8_t address, uint8_t data);


/*! @brief send opcode to Read Buffer Memory
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t sendRBMOpcode(void);


/*! @brief perform bit clear operation for address
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t sendWBMOpcode(void);

/* Higher functions */


/* Higher functions  */
/*! @brief Helper function to select a memory bank
 * need not be used for spi_write/read but must be used before basic operations
 *  @param[in] bank_no     bank number - 0,1,2,3
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t selectMemBank(uint8_t bank_no);


/*! @brief Function to read a MAC register - send 24 clock cycles insteado of 16
 *  @param[in] reg     name of MAC register to read from
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
uint8_t spi_readMACReg(uint8_t reg);

/* ========== Functions meant only for Physical register =====
 *
 * ===========================================================
 */


/*! @brief Write to a physical register - address of Physical register
 * written into MIREGADR, lower bits written to MIWRL, higher bits written to
 * MIWRH, then poll MISTAT.busy until it is low
 *  @param[in] address     address of physical register
 *  @param[in] higher_bits higher 8 bits of 16-bit value to be written into PHY register
 *  @param[in] lower_bits  lower 8 bits of 16-bit value to be written into PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t spi_writePHYReg(uint8_t address, uint8_t higher_bits, uint8_t lower_bits);


/*! @brief Read from a physical register - address of Physical register
 * written into MIREGADR, set MICMD.MIRRD bit, then sleep for 14.2 us and
 * then poll MISTAT.busy until it is low, clear MICMD.MIRRD bit
 * read the higher byte from MIRDH, lower byte from MIRDL
 *  @param[in] address     address of physical register
 *  @return             16 bit value read from PHY reg - ERR_DRIVER_FAIL on failure
 */
uint16_t spi_readPHYReg(uint8_t address);

/* =========== Register access functions ====
 *
 * ==========================================
 */


/*! @brief Helper function to figure out which bank a register belongs to
 *  @param[in] reg     name of register
 *  @return         bank that register belongs to
 */
uint8_t whichBank(uint8_t reg);


/* ======== Test Functions ==========
 *
 * ==================================
 */


/*! @brief Write to buffer memory
 *  @param[in] test_TxBuf      buffer with values to be written to the buffer memory
 *  @param[in] address         address inside buffer memory to write to
 *  @param[in] length          length of payload to write
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t writeBufferMemory(uint8_t* test_TxBuf, uint16_t address, uint16_t length);


/*! @brief Read to buffer memory
 *  @param[in] test_RxBuf      buffer which holds data read from buffer memory
 *  @param[in] address         address inside buffer memory to read from
 *  @param[in] length          length of payload to read
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t readBufferMemory(uint8_t* test_RxBuf, uint16_t address, uint16_t length);


/*! @brief test to write to and read from buffer memory
 *  @param[in] address         	address inside buffer memory to write to
 *  @param[in] length         	number of bytes to test
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t testReadWriteMemory(uint16_t address, uint16_t length);


/*! @brief Read Silicon Revision ID - test for operation
 * @return      Silicon revision ID - ERR_DRIVER_FAIL on failure
 */
uint8_t readRevID(void);


/*! @brief Turn on LED A on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t LEDA_On(void);


/*! @brief Turn off LED A on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t LEDA_Off(void);


/*! @brief Blink LED A on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t LEDA_Blink(void);


/*! @brief Default LEDs on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t LED_Default(void);


/*! @brief Turn on LED B on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t LEDB_On(void);


/*! @brief Turn off LED B on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t LEDB_Off(void);


/*! @brief Set clock frequency divider to 0,1,2,3,4,5
 * so 25MHz / clock divider = clock_frequency
 * 0 - default setting
 * by writing to the appropriate PHY register
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t setClock(uint8_t divider);


/*! @brief WRite to all w/r register banks
 * read the value back, and then restore the previous values
 * NOT a fool-proof test
 * @param[in] bank_no      bank to test
 * @param[in] start        start address of bank
 * @param[in] end          end address of bank
 * @return              number of errors
 */
uint8_t regBankTest_ETH(uint8_t bank_no, uint8_t start, uint8_t end);


/*! @brief WRite to all w/r MAC register banks
 * read the value back, and then restore the previous values
 * NOT a fool-proof test
 * @param[in] bank_no      bank to test
 * @param[in] start        start address of bank
 * @param[in] end          end address of bank
 * @return              number of errors
 */
uint8_t regBankTest_MAC(uint8_t bank_no, uint8_t start, uint8_t end);


/*! @brief WRite to all w/r register banks
 * read the value back, and then restore the previous values
 * NOT a fool-proof test
 */
void regBankTest(void);


/*! @brief Set all clock dividers - CLKOUT pin can be observed
 * on an oscilloscope
 *  @return     ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t test_clocks(void);


#endif /* SPIMASTER_H_ */
