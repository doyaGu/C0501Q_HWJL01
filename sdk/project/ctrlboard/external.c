#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ctrlboard.h"
#include "scene.h"

#define EXT_MAX_QUEUE_SIZE      8
#define MAX_OUTDATA_SIZE        64

#define UartHeader     		0xFF
#define UartTotalLength     16
#define UartPayloadLength   13

// Example for UART payload reference
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
#define UartCheckSum		0x4E  // CheckSum = payload0 + payload1 + ...... payload12

typedef enum GET_UART_COMMAND_STATE_TAG
{
    GET_HEADER,
    GET_LENGTH,
    GET_PAYLOAD,
    GET_CHECKSUM,
} GET_UART_COMMAND_STATE;

static mqd_t extInQueue = -1;
static mqd_t extOutQueue = -1;
static pthread_t extTask;
static volatile bool extQuit;
static unsigned char inDataBuf[EXTERNAL_BUFFER_SIZE];
static unsigned char cmdBuf[EXTERNAL_BUFFER_SIZE];
static unsigned int cmdPos = 0;
static GET_UART_COMMAND_STATE gState = GET_HEADER;

static void* ExternalTask(void* arg)
{
    while (!extQuit)
    {
       	ExternalEvent evOut;
		ExternalEvent evIn;		
		unsigned char readLen = 0;
		unsigned char paramBuf[EXTERNAL_BUFFER_SIZE];
		unsigned char outDataBuf[MAX_OUTDATA_SIZE];
		unsigned int  i = 0, count = 0;
		unsigned int  payloadCount = 0, checkSum = 0;
		bool isToMc = false;
		bool isCmdCompleted = false;

#if defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)
		// Receive UI command
		if (mq_receive(extOutQueue, (char*)&evIn, sizeof(ExternalEvent), 0) > 0)
		{
			isToMc = true;
			memset(outDataBuf, 0, MAX_OUTDATA_SIZE);

			outDataBuf[0] = 0x00;
			outDataBuf[1] = 0x01;
		}

		// Check if need to send data to main controller
		if (isToMc)
		{
			write(ITP_DEVICE_UART0, outDataBuf, MAX_OUTDATA_SIZE);
		}

		memset(inDataBuf, 0, EXTERNAL_BUFFER_SIZE);
		// Read data from UART port
		readLen = read(ITP_DEVICE_UART0, inDataBuf, EXTERNAL_BUFFER_SIZE);

		// Example for UART error correction to avoid data shift
		if (readLen)
		{
			while (readLen--)
			{
				switch (gState)
				{
					case GET_HEADER:
						if (UartHeader == inDataBuf[count])
						{
							cmdBuf[cmdPos++] = inDataBuf[count];
							gState = GET_LENGTH;
						}
						else
						{
							// printf("[GET_HEADER] Wrong, getstr=0x%x, len=%d\n", getstr[count], len);
							cmdPos = 0;
							memset(cmdBuf, 0, EXTERNAL_BUFFER_SIZE);
							gState = GET_HEADER;
						}
						break;

					case GET_LENGTH:
						if (UartTotalLength == inDataBuf[count])
						{
							cmdBuf[cmdPos++] = inDataBuf[count];
							gState = GET_PAYLOAD;
						}
						else
						{
							// printf("[GET_LENGTH] Wrong, getstr=0x%x\n", getstr[count]);
							cmdPos = 0;
							memset(cmdBuf, 0, EXTERNAL_BUFFER_SIZE);
							gState = GET_HEADER;
						}
						break;

					case GET_PAYLOAD:
						if (payloadCount != (UartPayloadLength - 1))
						{
							gState = GET_PAYLOAD;
							payloadCount++;
						}
						else
						{
							gState = GET_CHECKSUM;
							payloadCount = 0;
						}
						cmdBuf[cmdPos++] = inDataBuf[count];
						break;

					case GET_CHECKSUM:
					    // checkSum is sum of all payloads
						for (i=0; i<UartPayloadLength; i++)
							checkSum += cmdBuf[2+i];

						if (checkSum == inDataBuf[count])
						{
							// Get one command
							cmdBuf[cmdPos++] = inDataBuf[count];
							isCmdCompleted = true;
						}
						else
						{
							printf("[GET_CHECKSUM] Wrong, checkSum=0x%x\n", checkSum);
						}
						checkSum = 0;
						cmdPos = 0;
						gState = GET_HEADER;
						break;
				}
				count++;
			}
		}

		// If read data is completed, start to parse data
		if (isCmdCompleted)
		{
			// Parse data and check if need to send data to UI or main controller
			// case 0x00~0x04 and default send data to UI
			// case 0x05 send data to main controller
			switch(cmdBuf[3])
			{
			case 0x00:
				evIn.type = EXTERNAL_TEST0;
				mq_send(extInQueue, (const char*)&evIn, sizeof (ExternalEvent), 0);
				break;

			case 0x01:
				evIn.type = EXTERNAL_TEST1;
				evIn.arg1 = cmdBuf[4];
				mq_send(extInQueue, (const char*)&evIn, sizeof (ExternalEvent), 0);
				break;

			case 0x02:
				evIn.type = EXTERNAL_TEST2;
				evIn.arg1 = cmdBuf[4];
				evIn.arg2 = cmdBuf[5];
				mq_send(extInQueue, (const char*)&evIn, sizeof (ExternalEvent), 0);
				break;

			case 0x03:
				evIn.type = EXTERNAL_TEST3;
				evIn.arg1 = cmdBuf[4];
				evIn.arg2 = cmdBuf[5];
				evIn.arg3 = cmdBuf[6];
				mq_send(extInQueue, (const char*)&evIn, sizeof (ExternalEvent), 0);
				break;

			case 0x04:
				evIn.type = EXTERNAL_TEST4;
				evIn.arg1 = cmdBuf[4];
				evIn.arg2 = cmdBuf[5];
				evIn.arg3 = cmdBuf[6];
				paramBuf[0] = 0x0;
				paramBuf[1] = 0x1;
				paramBuf[2] = 0x2;
				memcpy(evIn.buf1, paramBuf, EXTERNAL_BUFFER_SIZE); 
				mq_send(extInQueue, (const char*)&evIn, sizeof (ExternalEvent), 0);
				break;

			case 0x05:
				memset(outDataBuf, 0 , MAX_OUTDATA_SIZE);
				outDataBuf[0] = 0xFF;
				outDataBuf[1] = 0xEE;
				outDataBuf[2] = 0xCC;
				write(ITP_DEVICE_UART0, outDataBuf, MAX_OUTDATA_SIZE);
				break;
				
			default:
				evIn.type = EXTERNAL_SHOW_MSG;
			    memset(evIn.buf1, 0 , EXTERNAL_BUFFER_SIZE);
                memcpy(evIn.buf1, cmdBuf, EXTERNAL_BUFFER_SIZE);
				mq_send(extInQueue, (const char*)&evIn, sizeof (ExternalEvent), 0);
                break;
			}
			memset(cmdBuf, 0, EXTERNAL_BUFFER_SIZE);
			isCmdCompleted = false;
		}

#endif
        usleep(10000);
    }
    mq_close(extInQueue);
	mq_close(extOutQueue);
    extInQueue = -1;
	extOutQueue = -1;

    return NULL;
}

void ExternalInit(void)
{
    struct mq_attr qattr;
    pthread_attr_t attr;

    qattr.mq_flags = 0;
    qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
    qattr.mq_msgsize = sizeof(ExternalEvent);

    extInQueue = mq_open("external_in", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extInQueue != -1);

    extOutQueue = mq_open("external_out", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extOutQueue != -1);

    extQuit = false;

#if defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)	
	itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, CFG_UART0_BAUDRATE);	
#endif

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&extTask, &attr, ExternalTask, NULL);
}

void ExternalExit(void)
{
    extQuit = true;
}

int ExternalReceive(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return 0;

    if (mq_receive(extInQueue, (char*)ev, sizeof(ExternalEvent), 0) > 0)
        return 1;

    return 0;
}

int ExternalSend(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return -1;

    return mq_send(extOutQueue, (char*)ev, sizeof(ExternalEvent), 0);
}
