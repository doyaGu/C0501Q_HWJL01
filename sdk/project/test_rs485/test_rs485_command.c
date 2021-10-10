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
#include "ite/itp.h" //for all ith driver
#include <pthread.h>
#include <time.h>

#define UartHeader     		0xFF
#define UartTotalLength     16
#define UartPayloadLength   13
#define UartPayload0   		0x00
#define UartPayload1   		0x01
#define UartPayload2   		0x02
#define UartPayload3   		0x03
#define UartPayload4   		0x04
#define UartPayload5   		0x05
#define UartPayload6   		0x06
#define UartPayload7   		0x07
#define UartPayload8   		0x08
#define UartPayload9   		0x09
#define UartPayload10  		0x0A
#define UartPayload11  		0x0B
#define UartPayload12  		0x0C
#define UartCheckSum		0x4E  //CheckSum = payload0 + payload1 + ...... payload12

static bool RunRS485Quit;
static sem_t RS485Sem;

typedef enum GET_UART_COMMAND_STATE_TAG
{
    GET_HEADER,
    GET_LENGTH,
    GET_PAYLOAD,
    GET_CHECKSUM,
} GET_UART_COMMAND_STATE;

static void RS485Callback(void* arg1, uint32_t arg2)
{	
	//don`t add any codes here.
	sem_post(&RS485Sem);
}

void* TestFunc(void* arg)
{
	uint8_t getstr[256];
	uint8_t sendtr[256];
	uint8_t sendtr1[16] = {0xFF, 0x10, 0x00, 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x4E};
	int len = 0 ,count =0,sendPos =0;	
	int i=0,PayloadCount =0,GetCheckSum =0;
	ITHRS485Port RS485_port0;
	ITHUartParity RS485_UartParity;
	GET_UART_COMMAND_STATE   gState = GET_HEADER;
	
	
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

	memset(getstr, 0, 256);
	memset(sendtr, 0, 256);

	while(!RunRS485Quit)
	{	
		sem_wait(&RS485Sem);
      	len = read(ITP_DEVICE_RS485_0,getstr,256);

		count = 0;
		if(len)
		{
			while(len--)
			{
				switch(gState)
				{
					case GET_HEADER:
						if(UartHeader == getstr[count])
						{
							sendtr[sendPos++] = getstr[count];
							gState = GET_LENGTH;		
						}else
						{							
							//printf("[GET_HEADER] Wrong,getstr=0x%x,len=%d\n",getstr[count],len);						
							sendPos = 0;
							memset(sendtr, 0, 256);
							gState = GET_HEADER;
						}
						break;
					case GET_LENGTH:
						if(UartTotalLength == getstr[count])
						{
							sendtr[sendPos++] = getstr[count];
							gState = GET_PAYLOAD;
						}else
						{
							//printf("[GET_LENGTH] Wrong,getstr=0x%x\n",getstr[count]);
							sendPos = 0;
							memset(sendtr, 0, 256);
							gState = GET_HEADER;
						}
						break;
					case GET_PAYLOAD:
						if(PayloadCount != (UartPayloadLength-1))
						{
							gState = GET_PAYLOAD;
							PayloadCount++;
						}else
						{
							gState = GET_CHECKSUM;
							PayloadCount = 0;
						}
						sendtr[sendPos++] = getstr[count];
						break;
					case GET_CHECKSUM:
						for(i=0;i<UartPayloadLength;i++) //GetChecksum  =  sum of all payloads.
							GetCheckSum += sendtr[2+i];

						if(GetCheckSum == getstr[count])
						{
							//Get one command.
							sendtr[sendPos++] = getstr[count];			
							write(ITP_DEVICE_RS485_0,sendtr, 16);
							printf("uart read[0]:0x%x,sendPos=%d\n",sendtr[0],sendPos);
							memset(sendtr, 0, 256);
						}else {
							printf("[GET_CHECKSUM] Wrong,CheckSum=0x%x\n",GetCheckSum);
						}
						GetCheckSum = 0;
						sendPos = 0;
						gState = GET_HEADER;
						break;
				}	
				count++;
			}
		}
	}
		
	return NULL;
}
