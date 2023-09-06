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
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/drivers/Pin.h>

/* Example/Board Header files */
#include "Board.h"

/* Library of Registers */
#include "registerlib.h"
#include "spimaster.h"
#include "ethernet.h"

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


//#define WRITE_CONTROL_MSG       0b 0100 0000 1010 1010
//#define READ_CONTROL_MSG        0b 0000 0000 0000 0000

#define WRITE_CONTROL_MSG       0x40AA
#define READ_CONTROL_MSG        0x0000


#define MAX_LOOP        (10)

static Display_Handle display;

uint16_t masterTxBuffer_write[SPI_MSG_LENGTH];
//unsigned char masterTxBuffer_write[SPI_MSG_LENGTH];
//unsigned char masterRxBuffer[SPI_MSG_LENGTH];
uint16_t masterRxBuffer[SPI_MSG_LENGTH];

//uint16_t masterTxBuffer_MAC[SPI_MSG_LENGTH_MAC];
//uint16_t masterRxBuffer_MAC[SPI_MSG_LENGTH_MAC];

uint8_t masterTxBuffer_MAC[SPI_MSG_LENGTH_MAC];
uint8_t masterRxBuffer_MAC[SPI_MSG_LENGTH_MAC];

uint8_t masterTxBuffer_eight[SPI_MSG_LENGTH];
uint8_t masterRxBuffer_eight[SPI_MSG_LENGTH];

//unsigned char masterTxBuffer[SPI_MSG_LENGTH];

/* Semaphore to block master until slave is ready for transfer */
sem_t masterSem;

/*
 *  ======== slaveReadyFxn ========
 *  Callback function for the GPIO interrupt on Board_SPI_SLAVE_READY.
 */
void slaveReadyFxn(uint_least8_t index)
{
    sem_post(&masterSem);
}


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

uint8_t setBitSetField(uint8_t address, uint8_t data){
    uint8_t controlWord =  (1<<7) | (address & 0x1F);
    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = data;
    return controlWord;
}



uint8_t setBitClearField(uint8_t address, uint8_t data){
    uint8_t controlWord =  (5<<5) | (address & 0x1F);
    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = data;
    return controlWord;
}



uint8_t setBufWriteToAddress(uint8_t address, uint8_t data){
    uint8_t controlWord =  (2<<5) | (address & 0x1F) ;
    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = data;
    return controlWord;
}


uint8_t setBufReadFromAddress(uint8_t address){
    uint8_t controlWord =  address & 0x1F;

    masterTxBuffer_eight[0] = controlWord;
    masterTxBuffer_eight[1] = 1;

    return controlWord;
}


/* operations */

void spi_write(uint8_t reg, uint8_t data){
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
        while(1);
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);


    sleep(0.5);
}



uint8_t spi_read(uint8_t reg){
    uint8_t readVal;
//    uint16_t controlWord = setBufReadFromAddress(address);
    bool transferOK;
    uint8_t bank_selector = whichBank(reg);
    if ((bank_selector != 0) && (bank_selector != 1) && (bank_selector != 2) && (bank_selector != 3) && (bank_selector != 4)){
        Display_printf(display, 0, 0, "Fatal Error - Wrong Register");
        while(1);
    }

    selectMemBank(bank_selector);

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
        while(1);
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);

//    readVal = masterRxBuffer[0] & 0x00ff;
    readVal = masterRxBuffer_eight[1];
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);

    sleep(0.5);
    return readVal;
}

void systemSoftReset(void){
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
        while(1);
    }
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
}


void bitFieldSet(uint8_t address, uint8_t data){
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
        while(1);
    }
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
}



void bitFieldClear(uint8_t address, uint8_t data){
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
        while(1);
    }
    GPIO_write(Board_GPIO_CSN0, 1);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.05);
}

/* Buffer Memory */

void sendRBMOpcode(void){
    /* 0b 001 11010  */
    bool transferOK;
    masterTxBuffer_eight[0] = 0x3a;
    masterTxBuffer_eight[1] = 0x00;

    memset((void *) masterRxBuffer_eight, 0, SPI_MSG_LENGTH);
    controlReg.count = SPI_MSG_LENGTH;
    controlReg.txBuf = (void *) masterTxBuffer_eight;
    controlReg.rxBuf = (void *) masterRxBuffer_eight;

    /* Toggle user LED, indicating a SPI transfer is in progress */
//    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);
//    GPIO_write(Board_GPIO_CSN0, 0);

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);

    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
        while(1);
    }

    /* Do NOT do anything with the CS pin */
    sleep(0.5);
}


void sendWBMOpcode(void){
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
        while(1);
    }
    /* Do NOT raise the CS pin */

    sleep(0.5);
}


/* Higher functions  */
void selectMemBank(uint8_t bank_no){

    if (bank_no == 0){
        bitFieldClear(0x1f, 0x3);
    }
    else if (bank_no == 1){
        bitFieldClear(0x1f, 0x3);
        bitFieldSet(0x1f,0x1);
    }
    else if (bank_no == 2){
        bitFieldClear(0x1f, 0x3);
        bitFieldSet(0x1f,0x2);
    }
    else if (bank_no == 3){
        bitFieldSet(0x1f,0x3);
    }
}


uint8_t spi_readMACReg(uint8_t reg){

//    uint16_t controlWord = (address & 0x1f)<< 8;
    uint8_t returnVal;
    uint8_t bank_selector = whichBank(reg);
    if ((bank_selector != 0) && (bank_selector != 1) && (bank_selector != 2) && (bank_selector != 3) && (bank_selector != 4)){
        Display_printf(display, 0, 0, "Fatal Error - Wrong Register");
        while(1);
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
        while(1);
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

void spi_writePHYReg(uint8_t address, uint8_t higher_bits, uint8_t lower_bits){
    /* First write the address of the PHY register to write to into the MIREGADR register */
    spi_write(MIREGADR, address);
    /* Write the lower 8 bits of data to write to into the MIWRL register */
    /* 0b 010 10000 1010 0010 0r 0x5*/
    spi_write(MIWRL, lower_bits);
//    selectMemBank(0);
    /* Write the upper 8 bits of data to write to into the MIWRH register */
    spi_write(MIWRH, higher_bits);
    /* Wait until the MISTAT.busy bit is clear*/
    while(spi_readMACReg(MISTAT) & 0x1);
}


uint16_t spi_readPHYReg(uint8_t address){
    /* Write the address of the PHY reg to read from MIREGADR */
    uint8_t readValH;
    uint8_t readValL;
    uint16_t readVal;
    spi_write(MIREGADR, address);

    /* Begin operation by setting MIRRD bit */
    uint8_t MICMD_val = 0x3 & (spi_readMACReg(0x12));
    spi_write(MICMD, (MICMD_val & 0xfe) | 0x1 );
    //MICMD_val = 0x3 & (spi_readMACReg(0x12));

    usleep(20);

    /* Polling until the PHY read completes */
//    selectMemBank(3);
    while(spi_readMACReg(MISTAT) & 0x1);

    /* Clear MICMD bit when you are done */
    MICMD_val = 0x3 & (spi_readMACReg(MICMD));
    spi_write(MICMD,(MICMD_val & 0xfe));

    readValH = spi_readMACReg(MIRDH);
    readValL = spi_readMACReg(MIRDL);

    readVal = readValH << 8 | readValL;
    return readVal;
}


/* =========== Register access functions ====
 *
 * ==========================================
 */

uint8_t whichBank(uint8_t reg){
    uint8_t bank_selector = (reg & 0x60) >> 5;
    return bank_selector;
}



/* ======== Test Functions ==========
 *
 * ==================================
 */

void writeBufferMemory(uint8_t* test_TxBuf, uint16_t address, uint8_t length){
//    uint8_t test_TxBuf[54];

    uint8_t* test_RxBuf = (uint8_t*) malloc(sizeof(uint8_t)*length);

    /* Set Test Stuff to send in */


    /* Set AUTOINC */
    selectMemBank(0);
    bitFieldSet(0x1e, 0x80);
    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);


    spi_write(EWRPTL, address & 0x00ff);
    spi_write(EWRPTH, (address & 0xff00) >>8);

    GPIO_write(Board_GPIO_CSN0, 0);

    sendWBMOpcode();

    bool transferOK;
    memset((void *) test_RxBuf, 0, length);
    controlReg.count = length;
    controlReg.txBuf = (void *) test_TxBuf;
    controlReg.rxBuf = (void *) test_RxBuf;

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);
    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
        while(1);
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);


    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
    free(test_RxBuf);
}

void readBufferMemory(uint8_t* test_RxBuf, uint16_t address, uint8_t length){
    uint8_t* test_TxBuf = (uint8_t*) malloc(sizeof(uint8_t)*length);
//    uint8_t test_TxBuf[54];
//    uint8_t* test_RxBuf = (uint8_t*) malloc(sizeof(uint8_t)*54);
//    uint8_t test_RxBuf;


    /* Set AUTOINC */
    selectMemBank(0);
    bitFieldSet(0x1e, 0x80);
    spi_write(ERDPTL, address & 0x00ff);
    spi_write(ERDPTH, (address & 0xff00) >> 8);

    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_ON);

    GPIO_write(Board_GPIO_CSN0, 0);


    sendRBMOpcode();


    bool transferOK;
    memset((void *) test_TxBuf, 0, 54);
    memset((void *) test_RxBuf, 0, 54);
    controlReg.count = 54;
    controlReg.txBuf = (void *) test_TxBuf;
    controlReg.rxBuf = (void *) test_RxBuf;

    /* Perform SPI transfer */
    transferOK = SPI_transfer(masterSpi, &controlReg);
    if (!transferOK){
        Display_printf(display, 0, 0, "Unsuccessful SPI transfer\n");
        while(1);
    }

    /* Bitbanging attempt for Chip Select */
    GPIO_write(Board_GPIO_CSN0, 1);

    GPIO_write(Board_GPIO_LED1, Board_GPIO_LED_OFF);
    sleep(0.5);
    free(test_TxBuf);
//    return test_RxBuf;
}


void testReadWriteMemory(uint16_t address){
    uint8_t length = 54;
    uint8_t* testTxBuf = (uint8_t*) malloc(sizeof(uint8_t) * length);
    uint8_t* testRxBuf = (uint8_t*) malloc(sizeof(uint8_t) * length);

//    uint8_t *testRxBuf ;
//    uint8_t* rxPtr = testRxBuf;
    int i;
    for (i=0;i<54;i++)
        testTxBuf[i] = rand() % 100;

    /* set pointers to write buffer memory */
    /* set EWRPT to where you want it say 0x120 - EWRPTL : 0x20, EWRPTH: 0x01 */
//    spi_write(EWRPTL, 0x20);
//    spi_write(EWRPTH, 0x01);

    writeBufferMemory(testTxBuf, address, length);

    /* Set ERDPT to the same point in memory , say 0x120 :
     * ERDPTL = 0x20, ERDPTH = 0x01  */
//    spi_write(ERDPTL, 0x20);
//    spi_write(ERDPTH, 0x01);

    readBufferMemory(testRxBuf, address, length);

    for (i=0;i<54;i++){
        if (testTxBuf[i]!=testRxBuf[i]){
            Display_printf(display, 0, 0, "Unsuccessful Read/WRite to memory\n");
            while(1);
        }
    }
    free(testTxBuf);
    free(testRxBuf);
}


uint8_t readRevID(void){
    uint8_t revisionID;
//    spi_write(0x1f, 3);
//    selectMemBank(3);
    revisionID = spi_read(EREVID);
//    revisionID = masterRxBuffer[0] & 0x001F;
    selectMemBank(0);
//    spi_write(0x1f, 0);
    return revisionID & 0x1f;
}

void LEDA_On(void){
    /* 0b 0011 1000 1000 0001*/
    /* Switch both LEDs on*/
    spi_writePHYReg(MIREGADR, 56, 130);
//    spi_writePHYRegWrap(0x14, 56, 130);
//    systemSoftReset();
}

void LEDA_Off(void){
    spi_writePHYReg(MIREGADR, 57, 130);
//    spi_writePHYRegWrap(0x14, 57, 130);
//    systemSoftReset();
}

void LEDA_Blink(void){
    /* 0b 0011 1011 1000 0010  */
    spi_writePHYReg(MIREGADR, 0x3A, 0x82 );
//    systemSoftReset();
}

void LEDA_Default(void){
    spi_writePHYReg(MIREGADR, 52, 34);
//    systemSoftReset();
}

void LEDB_On(void){
    /* 0b 0011 1000 1000 0001*/
    /* Switch both LEDs on*/
    spi_writePHYReg(MIREGADR, 57, 130);
//    systemSoftReset();
}

void LEDB_Off(void){
    spi_writePHYReg(MIREGADR, 57, 146);
//    systemSoftReset();
}


void setClock(uint8_t divider){
//    spi_write(0x1f, 3);
//    selectMemBank(3);
    if (divider == 0){
        spi_write(ECOCON, 0x00);
    }
    else if (divider == 1){
        spi_write(ECOCON, 0x01);
    }
    else if (divider == 2){
        spi_write(ECOCON, 0x02);
    }
    else if (divider == 3){
        spi_write(ECOCON, 0x03);
    }
    else if (divider == 4){
        spi_write(ECOCON, 0x04);
    }
    else if (divider == 8){
        spi_write(ECOCON, 0x05);
    }
    else
        Display_printf(display, 0, 0, "Wrong clock config, can be either 0,1,2,3,4,5\n");
//    selectMemBank(0);
//    spi_write(0x1f, 0);
}




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
        // Generate a random number in between 0 and 99 to write to the register
//        writeVal = rand() % 100;
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
        // Generate a random number in between 0 and 99 to write to the register
//        writeVal = rand() % 255;
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

void regBankTest(void){

    uint8_t err_bank_0 =0;
    uint8_t err_bank_1 =0;
    uint8_t err_bank_2 =0;
    uint8_t err_bank_3 =0;
    uint8_t err_total =0;

    /* Bank 0 */
//    err_bank_0 = err_bank_0 + regBankTest_ETH(0,0,0x0d);
    err_bank_0 = err_bank_0 + regBankTest_ETH(0,0x10,0x18);
    err_bank_0 = err_bank_0 + regBankTest_ETH(0,27,32);

    /* Bank 1 */
    err_bank_1 = err_bank_1 + regBankTest_ETH(1,0,18);
    err_bank_1 = err_bank_1 + regBankTest_ETH(1,20,22);
    err_bank_1 = err_bank_1 + regBankTest_ETH(1,24,26);
//    err_bank_1 = err_bank_1 + regBankTest_ETH(1,24,26);
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

    err_total = err_bank_0 + err_bank_1 +err_bank_2+err_bank_3;
    uint16_t total_tests = 0x18 - 0x10 + 32-27 + 18+2+2+32-27 + 1 + 3 + 0x0c-6+1+1+0x1a-0x16 + 32 - 0x1b + 6 + 3 + 1 + 1 + 0x1a-0x17+32-0x1b;
    Display_printf(display, 0, 0, "Errors: \n   \
            Bank 0 : %d,  \
            Bank 1 : %d,   \
            Bank 2 : %d,    \
            Bank 3 : %d", err_bank_0,err_bank_1, err_bank_2,err_bank_3);
    Display_printf(display, 0, 0, " Total errors  : %d, \
                                    Total Tests: :%d\n", err_total,total_tests);
}

void test_clocks(void){
    setClock(1);
    sleep(1);
    setClock(2);
    sleep(1);
    setClock(3);
    sleep(1);
    setClock(4);
    sleep(1);
    setClock(8);
    sleep(1);
    setClock(7);
    sleep(1);
    setClock(4);
}


static uint8_t dest_mac[] = {0xb0,0x0c,0xd1,0x4c,0x15,0x5f  };

/*
 *  ======== masterThread ========
 *  Master SPI sends a message to slave while simultaneously receiving a
 *  message from the slave.
 */
void *masterThread(void *arg0)
{
    /* Open SPI as master (default) */
    SPI_Params_init(&spiParams);
//    spiParams.dataSize = 16;
    spiParams.dataSize = 8;
    spiParams.frameFormat = SPI_POL0_PHA0;
    spiParams.bitRate = 8000000;
//    spiParams.bitRate = 8000000;
    spiParams.transferMode = SPI_MODE_BLOCKING;
    masterSpi = SPI_open(Board_SPI_MASTER, &spiParams);
    if (masterSpi == NULL) {
        Display_printf(display, 0, 0, "Error initializing master SPI\n");
        while (1);
    }
    else {
        Display_printf(display, 0, 0, "Master SPI initialized\n");
    }

//    test_clocks();
//
//    int i;
//    for(i=0;i<5;i++){
//        LEDA_On();
//        sleep(1);
//        LEDA_Off();
//        sleep(1);
//    }


//    testReadWriteMemory(0x120);
//
//    selectMemBank(2);
    ethernet_Init();
    char send_msg[25] = "Hello this is samyukta";
    uint8_t msglen = strlen(send_msg);
    char* receive_Buffer = (char*) malloc(sizeof(char)*msglen);
    memset(receive_Buffer, 0, msglen);

    while(1){
        ethernet_transmitPackets(send_msg, msglen);
        ethernet_receivePackets(receive_Buffer, msglen);
        Display_printf(display, 0, 0, "Message sent: %s, Message received: %s\n", send_msg, receive_Buffer);
    }
//
    uint8_t revisionID = 0;
    while(1){
        revisionID = 0;
        revisionID = readRevID();
        if (revisionID != 6){
            Display_printf(display, 0, 0, "Error in SPI!!\n");
            break;
        }
    }

    SPI_close(masterSpi);

    Display_printf(display, 0, 0, "\nDone");

    return (NULL);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    pthread_t           thread0;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* Call driver init functions. */
    Display_init();
    GPIO_init();
    SPI_init();

    /* Configure the LED pins */
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(Board_GPIO_LED1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        /* Failed to open display driver */
        while (1);
    }

    /* Turn on user LED */
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    Display_printf(display, 0, 0, "Starting the SPI master example");
    Display_printf(display, 0, 0, "This example requires external wires to be "
        "connected to the header pins. Please see the Board.html for details.\n");

    /* Create application threads */
    pthread_attr_init(&attrs);

    detachState = PTHREAD_CREATE_DETACHED;
    /* Set priority and stack size attributes */
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        while (1);
    }

    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        while (1);
    }

    /* Create master thread */
    priParam.sched_priority = 1;
    pthread_attr_setschedparam(&attrs, &priParam);

    retc = pthread_create(&thread0, &attrs, masterThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1);
    }

    return (NULL);
}
