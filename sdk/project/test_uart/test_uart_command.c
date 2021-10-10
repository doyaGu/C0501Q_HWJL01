#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"

#define TEST_PORT ITP_DEVICE_UART0
//#define TEST_PORT ITP_DEVICE_UART1
//#define TEST_PORT ITP_DEVICE_UART2
//#define TEST_PORT ITP_DEVICE_UART3

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

typedef enum GET_UART_COMMAND_STATE_TAG
{
    GET_HEADER,
    GET_LENGTH,
    GET_PAYLOAD,
    GET_CHECKSUM,
} GET_UART_COMMAND_STATE;

static sem_t UartSem;

static void UartCallback(void* arg1, uint32_t arg2)
{
	//don`t add any codes here.
	sem_post(&UartSem);
}

void* TestFunc(void* arg)
{
    char getstr[256];
    char sendtr[256];
	char sendtr1[16] = {0xFF, 0x10, 0x00, 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x4E};
	int len = 0, count =0 ,sendPos =0;
	int i=0,PayloadCount =0,GetCheckSum =0;
	GET_UART_COMMAND_STATE   gState = GET_HEADER;

	printf("Start uart test!\n");

	sem_init(&UartSem, 0, 0);

    itpRegisterDevice(TEST_PORT, &itpDeviceUart0);
    ioctl(TEST_PORT, ITP_IOCTL_INIT, NULL);
#ifdef CFG_UART0_ENABLE
    ioctl(TEST_PORT, ITP_IOCTL_RESET, CFG_UART0_BAUDRATE);
#endif
	ioctl(TEST_PORT, ITP_IOCTL_REG_UART_CB, (void*)UartCallback);

	memset(getstr, 0, 256);
	memset(sendtr, 0, 256);
	
    while(1)
    {
        sem_wait(&UartSem);
		len = read(TEST_PORT,getstr,256);
		count =0;
		
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
						for(i=0;i<UartPayloadLength;i++) //GetChecksum	=  sum of all payloads.
							GetCheckSum += sendtr[2+i];
		
						if(GetCheckSum == getstr[count])
						{
							//Get one command.
							sendtr[sendPos++] = getstr[count];			
							write(TEST_PORT,sendtr, 16);
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
}
