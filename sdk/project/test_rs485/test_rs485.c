/*
 * Copyright (c) 2015 ITE Tech. Inc. All Rights Reserved.
 *
 * @file (%project)/project/test_rs485/test_rs485.c
 *
 * @author Benson Lin
 * @version 1.0.0
 *
 * example code for how to using rs485.
 */
#include "ite/itp.h"	//for all ith driver
#include <pthread.h>
#include <time.h>

#define RS485CommandLen 4
static bool RunRS485Quit;
static sem_t RS485Sem;

/*
UART0: FIFO 32 Bytes , It means if we choice UART0 , we can transter/recevier 32 bytes once a time.
UART1: FIFO 16 Bytes , It means if we choice UART1 , we can transter/recevier 16 bytes once a time.
UART2: FIFO   8 Bytes,  It means if we choice UART2 , we can transter/recevier  8 bytes once a time.
UART3: FIFO   8 Bytes,  It means if we choice UART3 , we can transter/recevier  8 bytes once a time.
*/


static void RS485Callback(void* arg1, uint32_t arg2)
{
	uint8_t getstr1[256];
	uint8_t sendtr1[8] = {0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	int len = 0;
	static int totalcount =0;	

#if 1  // Turn on  ITP_IOCTL_REG_RS485_CB  flag , should using these codes.
	sem_post(&RS485Sem);
#else  // Turn on ITP_IOCTL_REG_RS485_DEFER_CB flag , should using these codes.
	
	len = read(ITP_DEVICE_RS485_0,getstr1+totalcount,256);
	totalcount += len;
	
	if(totalcount >= RS485CommandLen)
	{	  
		write(ITP_DEVICE_RS485_0,sendtr1,8);
		totalcount =0;
		memset(getstr1, 0, 256);
	}
#endif
}

void* TestFunc(void* arg)
{
	uint8_t getstr1[256];
	uint8_t sendtr1[8] = {0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
	int len = 0;
	int count =0;	
	ITHRS485Port RS485_port0;
	ITHUartParity RS485_UartParity;
	pthread_t task;
	
	RunRS485Quit			 = false;
  	RS485_port0              = ITH_RS485_0;
	RS485_UartParity         = ITH_UART_NONE;	    // Here enter the odd or even or normal parameter.
													// normal :  ITH_UART_NONE
													// odd	   :  ITH_UART_ODD
													// even    :  ITH_UART_EVEN
    itpInit();
	sem_init(&RS485Sem, 0, 0);
   	printf("Start RS485 test !\n");

    ioctl(ITP_DEVICE_RS485_0, ITP_IOCTL_ON, (void*)&RS485_port0);
	ioctl(ITP_DEVICE_RS485_0, ITP_IOCTL_RESET, (void*)&RS485_UartParity);
	ioctl(ITP_DEVICE_RS485_0, ITP_IOCTL_REG_RS485_CB, (void*)RS485Callback);
	//ioctl(ITP_DEVICE_RS485_0, ITP_IOCTL_REG_RS485_DEFER_CB , (void*)RS485Callback);


	while(!RunRS485Quit)
	{	
	
		sem_wait(&RS485Sem);
      	len = read(ITP_DEVICE_RS485_0,getstr1+count,256);
		count += len;
	
		if(count >= RS485CommandLen)
		{	 
			write(ITP_DEVICE_RS485_0,sendtr1,8);
			count =0;
			memset(getstr1, 0, 256);
		}
	}
	
	return NULL;
}
