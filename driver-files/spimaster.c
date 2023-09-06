/*
 * Copyright (c) 2018-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== spimaster.c ========
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* POSIX Header files */
#include <pthread.h>
#include <registerlib.h>
#include <unistd.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/drivers/Pin.h>

/* Example/Board Header files */
#include "Board.h"
#include "spimaster.h"

#define THREADSTACKSIZE (1024)

//#define SPI_MSG_LENGTH  (30)
#define SPI_MSG_LENGTH  (2)
#define SPI_MSG_LENGTH_MAC  (3)
#define MASTER_MSG      ("Hello from master, msg#: ")


// Register I want to write to :
// WCR, then read it back to make sure it is what you wrote

// WCR : instruction
// 010 aaaaa dddddddd

// RCW : instruction
// 000 aaaaa --------


#define WRITE_CONTROL_MSG       0x40AA
#define READ_CONTROL_MSG        0x0000


#define MAX_LOOP        (10)

Display_Handle display;



uint16_t masterTxBuffer_write[SPI_MSG_LENGTH];
uint16_t masterRxBuffer[SPI_MSG_LENGTH];


uint8_t masterTxBuffer_MAC[SPI_MSG_LENGTH_MAC];
uint8_t masterRxBuffer_MAC[SPI_MSG_LENGTH_MAC];

uint8_t masterTxBuffer_eight[SPI_MSG_LENGTH];
uint8_t masterRxBuffer_eight[SPI_MSG_LENGTH];





/*
 * Global SPI config
 *
 *
 */

SPI_Handle      masterSpi;
SPI_Params      spiParams;
SPI_Transaction controlReg;

/* =========== SPI Access functions ==========
 *
 * ===========================================
 */

/* Helpers for all the opcodes */

/*! @brief Helper Function to set bit(s) in a register
 *  @param[in] address     address of register
 *  @param[in] data        position of the bit to be set, e.g. to set third bit, send in 0b00000100
 *  @return             not important
 */
uint8_t setBitSetField(uint8_t address, uint8_t data){
    uint8_t controlWord =  (1<<7) | (address & 0x1F);
    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = data;
    return controlWord;
}


/*! @brief Helper Function to clear bit(s) in a register
 *  @param[in] address     address of register
 *  @param[in] data        position of the bit to be clear, e.g. to clear third bit, send in 0b00000100
 *  @return             not important
 */
uint8_t setBitClearField(uint8_t address, uint8_t data){
    uint8_t controlWord =  (5<<5) | (address & 0x1F);
    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = data;
    return controlWord;
}


/*! @brief Helper function for spi_write/read, set the master Rx and Tx buffers
 *  @param[in] address     address of register
 *  @param[in] data        data to write to address
 *  @return             controlWord which is the word written to the Rx/Tx buffer
 */
uint8_t setBufWriteToAddress(uint8_t address, uint8_t data){
    uint8_t controlWord =  (2<<5) | (address & 0x1F) ;
    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = data;
    return controlWord;
}


/*! @brief Helper function for spi_write/read, set the master Rx and Tx buffers
 *  @param[in] address     address of register
 *  @return             controlWord which is the word written to the Rx/Tx buffer
 */
uint8_t setBufReadFromAddress(uint8_t address){
    uint8_t controlWord =  address & 0x1F;

    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = 1;

    return controlWord;
}


/* operations */


/*! @brief perform spi write operation to specified register
 *  @param[in] reg         name of register
 *  @param[in] data        data to write to register
 *  @return		   Return 0 (transferOK) on success
 */
spierr_t spi_write(uint8_t reg, uint8_t data){
    //uint16_t controlWord = setBufWriteToAddress(address, data);
    uint8_t bank_selector = whichBank(reg);
    if ((bank_selector != 0) && (bank_selector != 1) && (bank_selector != 2) && (bank_selector != 3) && (bank_selector != 4)){
        Display_printf(display, 0, 0, "Fatal Error - Wrong Register");
        while(1);
    }

    selectMemBank(bank_selector);
    uint8_t address = reg & 0x1f;

    bool transferOK;
    masterTxBuffer_eight[0] = 0x2<<5 | (address & 0x1f);//((1<<14) | (address & 0x1F) << 8 ) ;
    masterTxBuffer_eight[1] = data;

    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;

    /* Toggle user LED, indicating a SPI transfer is in progress */
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 0);

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);
    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
       	return (spierr_t) ERR_DRIVER_FAIL;
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);


    sleep(0.5);
    return (spierr_t) ERR_SUCCESS;	
}



/*! @brief perform spi read operation from specified register
 *  @param[in] reg         name of register
 *  @return 		   8 bit value read back from register : -1 on failure
 */
uint8_t spi_read(uint8_t reg){
    uint8_t readVal;
//    uint16_t controlWord = setBufReadFromAddress(address);
    bool transferOK;
    uint8_t bank_selector = whichBank(reg);
    if ((bank_selector != 0) && (bank_selector != 1) && (bank_selector != 2) && (bank_selector != 3) && (bank_selector != 4)){
        Display_printf(display, 0, 0, "Fatal Error - Wrong Register");
        //while(1);
	return (uint8_t) ERR_DRIVER_FAIL;
    }

    if(selectMemBank(bank_selector)!=ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;

    uint8_t address = reg & 0x1f;

    masterTxBuffer_eight[0] = address & 0x1f;
    masterTxBuffer_eight[1] = 0;
    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;

    /* Toggle user LED, indicating a SPI transfer is in progress */
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 0);

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);

    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (uint8_t) ERR_DRIVER_FAIL;
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);

    readVal = masterRxBuffer_eight[1];
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);

    sleep(0.5);
    return readVal;
}



/*! @brief perform software reset of ENC28J60
 *  @return 	ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t systemSoftReset(void){
    /* 0b 1111 1111 1111 1111 */

    masterTxBuffer_eight[0] = 0xFF;
    masterTxBuffer_eight[1] = 0xFF;
    bool transferOK;

    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;

    /* Toggle user LED, indicating a SPI transfer is in progress */
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);
    GPIO_write(Board_GPIO_CSN0, 0);

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);

    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (spierr_t) ERR_DRIVER_FAIL;
    }
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
    return (spierr_t) ERR_SUCCESS;	
}



/*! @brief perform bit set operation for address
 *  @param[in] address     address of register
 *  @param[in] data        data to write to address
 *  @return 		   return ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t bitFieldSet(uint8_t address, uint8_t data){
//    uint8_t readVal;
    /* 0b 100 aaaaa dddddddd */
    uint8_t controlWord =  setBitSetField(address, data);
    bool transferOK;

    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;

    /* Toggle user LED, indicating a SPI transfer is in progress */
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);

    GPIO_write(Board_GPIO_CSN0, 0);
    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);

    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (spierr_t) ERR_DRIVER_FAIL;
    }
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
    return (spierr_t) ERR_SUCCESS;
}




/*! @brief perform bit clear operation for address
 *  @param[in] address     address of register
 *  @return 		   return ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t bitFieldClear(uint8_t address, uint8_t data){
    uint8_t controlWord = setBitClearField(address, data);
    bool transferOK;

    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;

    /* Toggle user LED, indicating a SPI transfer is in progress */
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);
    GPIO_write(Board_GPIO_CSN0, 0);

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);

    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (spierr_t) ERR_DRIVER_FAIL;
    }
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.05);
    return (spierr_t) ERR_SUCCESS;	
}

/* Buffer Memory */



/*! @brief send opcode to Read Buffer Memory
 *  @return 		   return ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t sendRBMOpcode(void){
    /* 0b 001 11010  */
    bool transferOK;
    masterTxBuffer_eight[0] = 0x3a;
    masterTxBuffer_eight[1] = 0x00;

    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;

    /* Toggle user LED, indicating a SPI transfer is in progress */

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);

    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (spierr_t) ERR_DRIVER_FAIL;
    }

    /* Do NOT do anything with the CS pin */
    sleep(0.5);
    return (spierr_t) ERR_SUCCESS;
}

/*! @brief perform bit clear operation for address
 */
spierr_t sendWBMOpcode(void){
    /* 0b 011 11010 dddddddd */
    bool transferOK;

    masterTxBuffer_eight[0] = 0x7a;
    masterTxBuffer_eight[1] = 0x00;
    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;



    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);
    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (spierr_t) ERR_DRIVER_FAIL;
    }
    /* Do NOT raise the CS pin */

    sleep(0.5);
    return (spierr_t) ERR_SUCCESS;
}


/* Higher functions  */


/* Higher functions  */
/*! @brief Helper function to select a memory bank
 * need not be used for spi_write/read but must be used before basic operations
 *  @param[in] bank_no     bank number - 0,1,2,3
 *  @return 		   return ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t selectMemBank(uint8_t bank_no){
    spierr_t errno;	
    if (bank_no == 0){
        errno = bitFieldClear(0x1f, 0x3);
    }
    else if (bank_no == 1){
        errno = bitFieldClear(0x1f, 0x3);
        errno = bitFieldSet(0x1f,0x1);
    }
    else if (bank_no == 2){
        errno = bitFieldClear(0x1f, 0x3);
        errno = bitFieldSet(0x1f,0x2);
    }
    else if (bank_no == 3){
        errno = bitFieldSet(0x1f,0x3);
    }
    
    return errno; 
}


/*! @brief Function to read a MAC register - send 24 clock cycles insteado of 16
 *  @param[in] reg     name of MAC register to read from
 *  @return 		   return read value from MAC reg success - ERR_DRIVER_FAIL on failure
 */
uint8_t spi_readMACReg(uint8_t reg){

    uint8_t returnVal;
    uint8_t bank_selector = whichBank(reg);
    if ((bank_selector != 0) && (bank_selector != 1) && (bank_selector != 2) && (bank_selector != 3) && (bank_selector != 4)){
        Display_printf(display, 0, 0, "Fatal Error - Wrong Register");
        return (uint8_t) ERR_DRIVER_FAIL;
	//while(1);
    }

    selectMemBank(bank_selector);

    uint8_t address = reg & 0x1f;

    masterTxBuffer_MAC[0] = address & 0x1f;
    masterTxBuffer_MAC[1] = 0;
    masterTxBuffer_MAC[2] = 0;

    bool transferOK;
    memset((void *) masterRxBuffer_MAC, 0, SPI_MSG_LENGTH_MAC);
    controlReg.count = SPI_MSG_LENGTH_MAC;
    controlReg.txBuf = (void *) masterTxBuffer_MAC;
    controlReg.rxBuf = (void *) masterRxBuffer_MAC;

    /* Toggle user LED, indicating a SPI transfer is in progress */
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 0);

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);
    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (uint8_t) ERR_DRIVER_FAIL;
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);

    returnVal = masterRxBuffer_MAC[2];

    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
    return returnVal;

}

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
 *  @return 		   return ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t spi_writePHYReg(uint8_t address, uint8_t higher_bits, uint8_t lower_bits){
    
    /* First write the address of the PHY register to write to into the MIREGADR register */
    if(spi_write(MIREGADR, address) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    /* Write the lower 8 bits of data to write to into the MIWRL register */
    /* 0b 010 10000 1010 0010 0r 0x5*/
    if(spi_write(MIWRL, lower_bits) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    /* Write the upper 8 bits of data to write to into the MIWRH register */
    if( spi_write(MIWRH, higher_bits) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    /* Wait until the MISTAT.busy bit is clear*/
    while(spi_readMACReg(MISTAT) & 0x1);
    return (spierr_t) ERR_SUCCESS;
}

/*! @brief Read from a physical register - address of Physical register
 * written into MIREGADR, set MICMD.MIRRD bit, then sleep for 14.2 us and
 * then poll MISTAT.busy until it is low, clear MICMD.MIRRD bit
 * read the higher byte from MIRDH, lower byte from MIRDL
 *  @param[in] address     address of physical register
 *  @param[in]             16 bit value read from PHY reg
 *  @return 		   return 16 bit read value on success - ERR_DRIVER_FAIL on failure
 */
uint16_t spi_readPHYReg(uint8_t address){
    /* Write the address of the PHY reg to read from MIREGADR */
    uint8_t readValH;
    uint8_t readValL;
    uint16_t readVal;
    uint16_t errno;
    errno = spi_write(MIREGADR, address);
    if (errno != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    /* Begin operation by setting MIRRD bit */
    uint8_t MICMD_val = 0x3 & (spi_readMACReg(0x12));
    errno = spi_write(MICMD, (MICMD_val & 0xfe) | 0x1 );
    if (errno != (spierr_t) ERR_SUCCESS)
	return (uint16_t) ERR_DRIVER_FAIL;

    usleep(20);

    /* Polling until the PHY read completes */
    while(spi_readMACReg(MISTAT) & 0x1);

    /* Clear MICMD bit when you are done */
    MICMD_val = 0x3 & (spi_readMACReg(MICMD));
    errno = spi_write(MICMD,(MICMD_val & 0xfe));
    if (errno != (spierr_t) ERR_SUCCESS)
	return (uint16_t) ERR_DRIVER_FAIL;
    readValH = spi_readMACReg(MIRDH);
    readValL = spi_readMACReg(MIRDL);

    readVal = readValH << 8 | readValL;
    return readVal;
}


/* =========== Register access functions ====
 *
 * ==========================================
 */

/*! @brief Helper function to figure out which bank a register belongs to
 *  @param[in] reg     name of register
 *  @param[in]         bank that register belongs to
 *  @return 	       return 8 bit read value
 */
uint8_t whichBank(uint8_t reg){
    uint8_t bank_selector = (reg & 0x60) >> 5;
    return bank_selector;
}



/* ======== Test Functions ==========
 *
 * ==================================
 */

/*! @brief Write to buffer memory
 *  @param[in] test_TxBuf      buffer with values to be written to the buffer memory
 *  @param[in] address         address inside buffer memory to write to
 *  @param[in] length          length of payload to write
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t writeBufferMemory(uint8_t* test_TxBuf, uint16_t address, uint16_t length){
    uint8_t* test_RxBuf = (uint8_t*) malloc(sizeof(uint8_t)*length);
 
    /* Set Test Stuff to send in */


    /* Set AUTOINC */
    selectMemBank(0);
    bitFieldSet(0x1e, 0x80);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);


    if(spi_write(EWRPTL, address & 0x00ff)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;
    if(spi_write(EWRPTH, (address & 0xff00) >>8)!=ERR_SUCCESS)
	return ERR_DRIVER_FAIL;

    GPIO_write(Board_GPIO_CSN0, 0);

    if (sendWBMOpcode()!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;

    bool transferOK;
    memset((void *) test_RxBuf, 0, length);
    controlReg.count = length;
    controlReg.txBuf = (void *) test_TxBuf;
    controlReg.rxBuf = (void *) test_RxBuf;

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);
    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (spierr_t) ERR_DRIVER_FAIL;
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);


    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
    free(test_RxBuf);
    /* Clear AUTOINC */
    selectMemBank(0);
    bitFieldClear(0x1e, 0x80);
    return (spierr_t) ERR_SUCCESS;
}


/*! @brief Read to buffer memory
 *  @param[in] test_RxBuf      buffer which holds data read from buffer memory
 *  @param[in] address         address inside buffer memory to read from
 *  @param[in] length          length of payload to read
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t readBufferMemory(uint8_t* test_RxBuf, uint16_t address, uint16_t length){
    uint8_t* test_TxBuf = (uint8_t*) malloc(sizeof(uint8_t)*length);


    /* Set AUTOINC */
    selectMemBank(0);
    bitFieldSet(0x1e, 0x80);
    if(spi_write(ERDPTL, address & 0x00ff)!=(spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    if(spi_write(ERDPTH, (address & 0xff00) >> 8)!= (spierr_t) ERR_SUCCESS )
	return (spierr_t) ERR_DRIVER_FAIL;

    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);

    GPIO_write(Board_GPIO_CSN0, 0);

    if (sendRBMOpcode()!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;

    bool transferOK;
    memset((void *) test_TxBuf, 0, length);
    memset((void *) test_RxBuf, 0, length);
    controlReg.count = length;
    controlReg.txBuf = (void *) test_TxBuf;
    controlReg.rxBuf = (void *) test_RxBuf;

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);
    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
	return (spierr_t) ERR_DRIVER_FAIL;
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);

    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
//    Display_printf(display,0,0,"Just after reading from buffer memory - if this is what it is, then it means the issue is somewhere reading from buffer memory if the length is > 256 bytes \n");
//    int i;
//    for (i=0;i<length;i++)
//	Display_printf(display,0,0,"test_RxBuf[%d] = %x\n", i, test_RxBuf[i]);	
//    Display_printf(display,0,0,"That's the end in read buffer mem \n");
    free(test_TxBuf);

    /* Clear AUTOINC */
    selectMemBank(0);
    bitFieldClear(0x1e, 0x80);	
    return (spierr_t) ERR_SUCCESS;	
}


/*! @brief test to write to and read from buffer memory
 *  @param[in] address         address inside buffer memory to write to
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t testReadWriteMemory(uint16_t address, uint16_t length){
    uint8_t* testTxBuf = (uint8_t*) malloc(sizeof(uint8_t) * length);
    uint8_t* testRxBuf = (uint8_t*) malloc(sizeof(uint8_t) * length);

    int i;
    for (i=0;i<length;i++)
        testTxBuf[i] = rand() % 100;

    if( writeBufferMemory(testTxBuf, address, length) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;


    if (readBufferMemory(testRxBuf, address, length) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;

    	
    for (i=0;i<length;i++){
        if (testTxBuf[i]!=testRxBuf[i]){
            Display_printf(display, 0, 0, "Unsuccessful Read/WRite to memory\n");
            return (spierr_t) ERR_TEST_FAIL;
        }
    }
	Display_printf(display, 0, 0,"Here's proof that this is new firmware\n");

    free(testTxBuf);
    free(testRxBuf);
    return (spierr_t) ERR_SUCCESS;
}

/*! @brief Read Silicon Revision ID - test for operation
 *  @param[in]      Silicon revision ID
 *  @return         return revisionID on success, ERR_DRIVER_FAIL on failure
 */
uint8_t readRevID(void){
    uint8_t revisionID;
    revisionID = spi_read(EREVID);
    if (revisionID == (uint8_t) ERR_DRIVER_FAIL)	
	return (uint8_t) ERR_DRIVER_FAIL;
    if (revisionID != 6)
	return (uint8_t) ERR_TEST_FAIL;
    return revisionID & 0x1f;
}


/*! @brief Turn on LED A on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return 	return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t LEDA_On(void){
    /* 0b 0011 1000 1000 0001*/
    /* Read the value of LEDB */
    uint8_t LEDBVal = (spi_readPHYReg(0x14) & 0x00f0);
    		  	
    /* Switch both LEDs on*/
    uint8_t higher_bits = 56;
    uint8_t lower_bits = LEDBVal | 1;
    if (spi_writePHYReg(0x14, higher_bits, lower_bits) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    return (spierr_t) ERR_SUCCESS;
}


/*! @brief Turn off LED A on the ENC28J60
 *  by writing to the appropriate PHY register
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t LEDA_Off(void){
    uint8_t LEDBVal = (spi_readPHYReg(0x14) & 0x00f0);
    /* Switch both LEDs on*/
    uint8_t higher_bits = 57;
    uint8_t lower_bits = LEDBVal | 1;
    if( spi_writePHYReg(0x14, higher_bits, lower_bits) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    return (spierr_t) ERR_SUCCESS;
}


/*! @brief Blink LED A on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t LEDA_Blink(void){
    uint8_t LEDBVal = (spi_readPHYReg(0x14) & 0x00f0);
    /* Switch both LEDs on*/
    uint8_t higher_bits = 0x3A;
    uint8_t lower_bits = LEDBVal | 1;
    if( spi_writePHYReg(0x14, higher_bits, lower_bits)!=(spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    return (spierr_t) ERR_SUCCESS;
}


/*! @brief Default LEDs on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t LED_Default(void){
    /* Switch both LEDs on*/
    /* Default: 0b 0011 0100 0010 0010 */
    /* higher bits: 0x34 lower bits: 0x22*/
    uint8_t higher_bits = 0x34;
    uint8_t lower_bits = 0x22;
    if( spi_writePHYReg(0x14, higher_bits, lower_bits) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    return (spierr_t) ERR_SUCCESS;
}

/*! @brief Turn on LED B on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t LEDB_On(void){
    /* 0b 0011 1000 1000 0001*/
    /* Read the value of LEDB */
    uint8_t LEDAVal = (spi_readPHYReg(0x14) & 0x0f00)>>16;
	  	
    uint8_t higher_bits = 48 | LEDAVal;
    uint8_t lower_bits = 130;
    if( spi_writePHYReg(0x14, higher_bits, lower_bits) != (spierr_t) ERR_SUCCESS) 
	return (spierr_t) ERR_SUCCESS;
    return (spierr_t) ERR_SUCCESS;
}


/*! @brief Turn off LED B on the ENC28J60
 * by writing to the appropriate PHY register
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t LEDB_Off(void){
    
    if(spi_writePHYReg(MIREGADR, 57, 146) != (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    /* 0b 0011 1000 1000 0001*/
    /* Read the value of LEDB */
    uint8_t LEDAVal = (spi_readPHYReg(0x14) & 0x0f00)>>16;
	  	
    uint8_t higher_bits = 48 | LEDAVal;
    uint8_t lower_bits = 146;
    if(spi_writePHYReg(0x14, higher_bits, lower_bits) != (spierr_t) ERR_SUCCESS) 
	return (spierr_t) ERR_DRIVER_FAIL;
    return (spierr_t) ERR_SUCCESS;
}




/*! @brief Set clock frequency divider to 0,1,2,3,4,5
 * so 25MHz / clock divider = clock_frequency
 * 0 - default setting
 * by writing to the appropriate PHY register
 *  @return 		       return ERR_SUCCESS on success, ERR_DRIVER_FAIL on failure
 */
spierr_t setClock(uint8_t divider){
    spierr_t errno;
    if (divider == 0){
        errno = spi_write(ECOCON, 0x00);
    }
    else if (divider == 1){
        errno = spi_write(ECOCON, 0x01);
    }
    else if (divider == 2){
        errno = spi_write(ECOCON, 0x02);
    }
    else if (divider == 3){
        errno = spi_write(ECOCON, 0x03);
    }
    else if (divider == 4){
        errno = spi_write(ECOCON, 0x04);
    }
    else if (divider == 8){
        errno = spi_write(ECOCON, 0x05);
    }
    else{
        Display_printf(display, 0, 0, "Wrong clock config, can be either 0,1,2,3,4,5\n");
	errno = (spierr_t) ERR_DRIVER_FAIL;
    }
    return errno;
}



/*! @brief WRite to all w/r register banks
 * read the value back, and then restore the previous values
 * NOT a fool-proof test
 * @param[in] bank_no      bank to test
 * @param[in] start        start address of bank
 * @param[in] end          end address of bank
 * @param[in]              number of errors
 */
uint8_t regBankTest_ETH(uint8_t bank_no, uint8_t start, uint8_t end){
    /* Bank 0 */
    selectMemBank(bank_no);
    uint8_t bufValues[32];
    int i;
    uint8_t err_count=0;
    /* First read in the existing values so you don't lose them */
    for (i=start;i<end;i++){
            // Generate a random number in between 0 and 99 to write to the register
        bufValues[i] = spi_read(i);
    }

    uint8_t readVal;
    uint8_t writeVal;

    for (i=start;i<end;i++){
        writeVal = 3;
        spi_write(i, writeVal);
        readVal = spi_read(i);
        if (readVal!=writeVal){
            err_count++;
            Display_printf(display, 0, 0, "Error Writing to/reading from bank %d, register number %d\n",bank_no,i);
            Display_printf(display, 0, 0, "value written : %d, value read: %d\n",writeVal, readVal);
        }
    }
    /* Write back the initial values */
    for (i=start;i<end;i++){
        spi_write(i, bufValues[i]);
        readVal = spi_read(i);
        if (readVal!=bufValues[i]){
            Display_printf(display, 0, 0, "Error Writing to/reading from bank %d, register number %d, and you may have lost a default value, please check manually \n",bank_no,i);
            Display_printf(display, 0, 0, "value written : %d, value read: %d\n",bufValues[i], readVal);
        }
    }
    return err_count;
}



/*! @brief WRite to all w/r MAC register banks
 * read the value back, and then restore the previous values
 * NOT a fool-proof test
 * @param[in] bank_no      bank to test
 * @param[in] start        start address of bank
 * @param[in] end          end address of bank
 * @param[in]              number of errors
 */
uint8_t regBankTest_MAC(uint8_t bank_no, uint8_t start, uint8_t end){
    /* Bank 0 */
    selectMemBank(bank_no);
    uint8_t bufValues[32];
    int i;
    uint8_t err_count = 0;

    /* First read in the existing values so you don't lose them */
    for (i=start;i<end;i++){
            // Generate a random number in between 0 and 99 to write to the register
        bufValues[i] = spi_readMACReg(i);
    }

    uint8_t readVal;
    uint8_t writeVal;

    for (i=start;i<end;i++){
        writeVal = 3;
        spi_write(i, writeVal);
        readVal = spi_readMACReg(i);
        if (readVal!=writeVal){
            err_count++;
            Display_printf(display, 0, 0, "Error Writing to/reading from bank %d, register number %d\n",bank_no,i);
            Display_printf(display, 0, 0, "value written : %d, value read: %d\n",writeVal, readVal);
        }
    }
    /* Write back the initial values */
    for (i=start;i<end;i++){
        spi_write(i, bufValues[i]);
        readVal = spi_readMACReg(i);
        if (readVal!=bufValues[i]){
            Display_printf(display, 0, 0, "Error Writing to/reading from bank %d, register number %d, and you may have lost a default value, please check manually \n",bank_no,i);
            Display_printf(display, 0, 0, "value written : %d, value read: %d\n",bufValues[i], readVal);
        }
    }
    return err_count;
}



/*! @brief WRite to all w/r register banks
 * read the value back, and then restore the previous values
 * NOT a fool-proof test
 */
void regBankTest(void){

    uint8_t err_bank_0 =0;
    uint8_t err_bank_1 =0;
    uint8_t err_bank_2 =0;
    uint8_t err_bank_3 =0;
    uint8_t spierr_total =0;

    /* Bank 0 */
    err_bank_0 = err_bank_0 + regBankTest_ETH(0,0x10,0x18);
    err_bank_0 = err_bank_0 + regBankTest_ETH(0,27,32);

    /* Bank 1 */
    err_bank_1 = err_bank_1 + regBankTest_ETH(1,0,18);
    err_bank_1 = err_bank_1 + regBankTest_ETH(1,20,22);
    err_bank_1 = err_bank_1 + regBankTest_ETH(1,24,26);
    err_bank_1 = err_bank_1 + regBankTest_ETH(1,27,32);


    /* Bank 2 */
    err_bank_2 = err_bank_2 + regBankTest_MAC(2,0,1);
    err_bank_2 = err_bank_2 + regBankTest_MAC(2,2,5);
    err_bank_2 = err_bank_2 + regBankTest_MAC(2,0x6,0x0c);
    err_bank_2 = err_bank_2 + regBankTest_MAC(2,0x12,0x13);
    err_bank_2 = err_bank_2 + regBankTest_MAC(2,0x14,0x15);
    err_bank_2 = err_bank_2 + regBankTest_MAC(2,0x16,0x1a);
    err_bank_2 = err_bank_2 + regBankTest_ETH(2,0x1b,32);

    /* Bank 3 */
    err_bank_3 = err_bank_3 + regBankTest_MAC(3,0,0x6);
    err_bank_3 = err_bank_3 + regBankTest_ETH(3,0x6,0x09);
    err_bank_3 = err_bank_3 + regBankTest_MAC(3,0x0a,0x0b);
    err_bank_3 = err_bank_3 + regBankTest_ETH(3,0x15,0x16);
    err_bank_3 = err_bank_3 + regBankTest_ETH(3,0x17,0x1a);
    err_bank_3 = err_bank_3 + regBankTest_ETH(3,0x1b,32);

    spierr_total = err_bank_0 + err_bank_1 +err_bank_2+err_bank_3;
    uint16_t total_tests = 0x18 - 0x10 + 32-27 + 18+2+2+32-27 + 1 + 3 + 0x0c-6+1+1+0x1a-0x16 + 32 - 0x1b + 6 + 3 + 1 + 1 + 0x1a-0x17+32-0x1b;
    Display_printf(display, 0, 0, "Errors: \n   \
            Bank 0 : %d,  \
            Bank 1 : %d,   \
            Bank 2 : %d,    \
            Bank 3 : %d", err_bank_0,err_bank_1, err_bank_2,err_bank_3);
    Display_printf(display, 0, 0, " Total errors  : %d, \
                                    Total Tests: :%d\n", spierr_total,total_tests);
}


/*! @brief Set all clock dividers - CLKOUT pin can be observed
 * on an oscilloscope
 *  @return 	ERR_SUCCESS on success - ERR_DRIVER_FAIL on failure
 */
spierr_t test_clocks(void){
    if (setClock(1)!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    sleep(1);
    if (setClock(2)!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    sleep(1);
    if (setClock(3)!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    sleep(1);
    if (setClock(4)!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    sleep(1);
    if (setClock(8)!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    sleep(1);
    //if (setClock(7)!= (spierr_t) ERR_SUCCESS)
//	return (spierr_t) ERR_DRIVER_FAIL;
    sleep(1);
    if (setClock(4)!= (spierr_t) ERR_SUCCESS)
	return (spierr_t) ERR_DRIVER_FAIL;
    return (spierr_t) ERR_SUCCESS;
}



