/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

/* Example/Board Header files */
//#include <ti/drivers/Board.h>
/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/drivers/Pin.h>

/* enc28j60 driver header files */
#include "registerlib.h"
#include "spimaster.h"
#include "enc_ethernet.h"
#include "Board.h"


extern void *mainThread(void *arg0);
extern void *gpioThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE    1024


/*
 * Global SPI config
 *
 *
 */

extern SPI_Handle      masterSpi;
extern SPI_Params      spiParams;
extern SPI_Transaction controlReg;
Display_Handle display;


/*
 *  ======== masterThread ========
 *  Master SPI sends a message to slave while simultaneously receiving a
 *  message from the slave.
 */
void *masterThread(void *arg0)
{
    /* Open SPI as master (default) */
    SPI_Params_init(&spiParams);
    spiParams.dataSize = 8;
    spiParams.frameFormat = SPI_POL0_PHA0;
    spiParams.bitRate = 800000;
    spiParams.transferMode = SPI_MODE_BLOCKING;
    masterSpi = SPI_open(Board_SPI_MASTER, &spiParams);
    if (masterSpi == NULL) {
        Display_printf(display, 0, 0, "Error initializing master SPI\n");
        while (1);
    }
    else {
        Display_printf(display, 0, 0, "Master SPI initialized\n");
    }


	/* Testing buffer memory */
	/* System Reset */        
	systemSoftReset();
        Display_printf(display, 0, 0, "Testing buffer memory\n");
	spierr_t errno;
	errno = testReadWriteMemory(0,100);
	if (errno == ERR_DRIVER_FAIL)
		Display_printf(display, 0, 0, "Issue in driver\n");
	else if (errno == ERR_TEST_FAIL)
		Display_printf(display, 0, 0, "Values read back did not match values written\n");
	else
		Display_printf(display, 0, 0, "ENC28J60 Buffer read write test passed\n");
			
  	
	
        Display_printf(display, 0, 0, "Done Read Write ! Clearing ENC82J60 receive buffer memory\n");
        if(clearWholeBuf()!=ERR_SUCCESS)
		Display_printf(display, 0, 0, "Failed to clear ENC28J60 buffer\n");        

	
        Display_printf(display, 0, 0, "Done Clear! Test Clocks. Scope CLKOUT pin.\n");
   	if(test_clocks()!=ERR_SUCCESS)
		Display_printf(display, 0, 0, "Clock test failed\n");	

	
        Display_printf(display, 0, 0, "Done Clock tests ! Blink LED A Green \n");
	int i;
	for (i=0;i<2;i++){
		if (LEDA_On()!=ERR_SUCCESS)
			Display_printf(display, 0, 0, "Failed to switch on LEDA \n");
		sleep(1);
		if(LEDA_Off()!=ERR_SUCCESS)
			Display_printf(display, 0, 0, "Failed to switch off LEDA\n");
		sleep(1);
	}
		
	
        Display_printf(display, 0, 0, "Blink LED B Orange\n");
	for (i=0;i<2;i++){
		if(LEDB_On()!=ERR_SUCCESS)
			Display_printf(display, 0, 0, "Failed to switch on LEDB\n");
		sleep(1);
		if(LEDB_Off()!=ERR_SUCCESS)
			Display_printf(display, 0, 0, "Failed to switch off LEDB\n");
		sleep(1);
	}
	Display_printf(display, 0, 0, "Switching LEDs back to default mode\n");
	if(LED_Default()!=ERR_SUCCESS)
		Display_printf(display, 0, 0, "Failed to switch LEDs to default mode\n");

	
        Display_printf(display, 0, 0, "Read revision ID of ENC28J60\n");
	uint8_t revID = readRevID();
	if (revID ==ERR_DRIVER_FAIL)
		Display_printf(display, 0, 0, "Revision ID : Error in Driver\n");
	else if(revID == ERR_TEST_FAIL)
		Display_printf(display, 0, 0, "Revision ID read back incorrect value\n");
	else
		Display_printf(display, 0, 0, "Revision ID correctly read back as : %d\n", revID);

	Display_printf(display, 0, 0, "Preliminary Tests Done\n");

    /* Close the SPI module */
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

    /* Create master thread  */
    priParam.sched_priority = 1;
    pthread_attr_setschedparam(&attrs, &priParam);

    retc = pthread_create(&thread0, &attrs, masterThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1);
    }


    return (NULL);
}


/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t           threadMain;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;

    /* Call driver init functions */
    Board_init();

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /* Set priority, detach state, and stack size attributes */
    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* failed to set attributes */
        while (1) {}
    }

    retc = pthread_create(&threadMain, &attrs, mainThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1) {}
    }

    BIOS_start();

    return (0);
}
