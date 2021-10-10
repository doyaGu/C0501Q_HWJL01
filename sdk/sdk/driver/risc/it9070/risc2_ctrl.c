/*
 * Copyright (c) 2004 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Co-processor API functoin file.
 *      Date: 2015/11/19
 *
 */
#include <pthread.h>
#include <assert.h>

#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/audio.h"
#include "ite/ite_risc.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

typedef struct CODEC_CONTEXT_TAG
{
    int commandBuffer;
    int commandBufferLength;
    int wiegandBuffer;
    int wiegandBufferLength;
    int printfBuffer;
    int uarttxBuffer;
    int uartrxBuffer;
    int uarttxdbgBuffer;
    int ctrlboardBuffer;
    int heartbeatBuffer;
} CODEC_CONTEXT;

#define TOINT(n)   ((((n) >> 24) & 0xff) + (((n) >> 8) & 0xff00) + (((n) << 8) & 0xff0000) + (((n) << 24) & 0xff000000))
#define TOSHORT(n) ((((n) >> 8) & 0x00ff) + (((n) << 8) & 0xff00))

#define CODEC_BASE    ((int)iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET) - 0x1000)
#define CODEC_EX_BASE ((int)iteRiscGetTargetMemAddress(RISC2_IMAGE_MEM_TARGET))

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static pthread_mutex_t   RiscMutex = PTHREAD_MUTEX_INITIALIZER;

static uint32_t         Bootmode = 1;
static CODEC_CONTEXT    *gCodecCtxt = 0;
struct _codec_header    *__codec_ex_header = 0;

//=============================================================================
//                              Macro Definition
//=============================================================================
//#define AUDIO_PLUGIN_MESSAGE_QUEUE // defined in def.cmake
//#ifdef DEBUG_MODE
//#define LOG_ZONES    (ITH_BIT_ALL & ~ITH_ZONE_ENTER & ~ITH_ZONE_LEAVE & ~ITH_ZONE_DEBUG)
////#define LOG_ZONES    (ITH_BIT_ALL & ~ITH_ZONE_ENTER & ~ITH_ZONE_LEAVE)
////#define LOG_ZONES    (ITH_BIT_ALL)
//
//#define LOG_ERROR   ((void) ((ITH_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][AUD][ERROR]"
//#define LOG_WARNING ((void) ((ITH_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][AUD][WARNING]"
//#define LOG_INFO    ((void) ((ITH_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][AUD][INFO]"
//#define LOG_DEBUG   ((void) ((ITH_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][AUD][DEBUG]"
//#define LOG_ENTER   ((void) ((ITH_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][AUD][ENTER]"
//#define LOG_LEAVE   ((void) ((ITH_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][AUD][LEAVE]"
//#define LOG_DATA    ((void) ((ITH_FALSE) ? (printf(
//#define LOG_END     )), 1 : 0));
//#else
#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_END ;

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static unsigned char gWiegand[] = {
    #include "wiegand.hex"
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Definition
//=============================================================================
__attribute__((used)) bool
ithCodecCommand(
    int command,
    int parameter0,
    int parameter1,
    int parameter2)
{
    int timeout             = 10;
    int commandBuffer       = TOINT(gCodecCtxt->commandBuffer) + CODEC_BASE;
    int *oriCommandBuffer   = (int *) commandBuffer;
    int commandBufferLength = TOINT(gCodecCtxt->commandBufferLength);

    if (gCodecCtxt->commandBuffer == 0)
        return false;

    ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
    timeout = 1000;
    while (*((int *)oriCommandBuffer) && timeout && ithReadRegH(0x16A4) != 0x0)
    {
        if (Bootmode)
        {
            int i;
            for (i = 0; i < 10000; i++)
                asm ("");
        }
        else
            usleep(1000);
        timeout--;
        ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
    }

    if (timeout == 0)
        return false;

    /*RISC command should final adding , because RISC will get the command first and start it.*/
    oriCommandBuffer[1] = parameter0;
    oriCommandBuffer[2] = parameter1;
    oriCommandBuffer[3] = parameter2;
    oriCommandBuffer[0] = command;
    ithWriteRegH(0x16A4, 0x1111);
    ithFlushDCacheRange(oriCommandBuffer, commandBufferLength);
    ithFlushMemBuffer();

    ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
    timeout = 1000;
    while (*((int *)oriCommandBuffer) && timeout)
    {
        if (!Bootmode)
            usleep(1000);
        timeout--;
        ithInvalidateDCacheRange((void*)oriCommandBuffer, 4);
    }
    if (timeout == 0)
        return false;

    return true;
}

int
ithCodecWiegandReadCard(
    int index,
    unsigned long long *card_id)
{
    unsigned long *value;
    int           bit_count           = 0;
    int           wiegandBuffer       = TOINT(gCodecCtxt->wiegandBuffer) + CODEC_BASE;
    int           wiegandBufferLength = TOINT(gCodecCtxt->wiegandBufferLength);

    if (gCodecCtxt->wiegandBuffer == 0)
        return;

    // get back card id from buffer of wiegand
    value     = (unsigned long *)wiegandBuffer;
    ithInvalidateDCache();
    *card_id  = (((unsigned long long)TOINT(value[2 * index]) << 32) | (unsigned long long)TOINT(value[2 * index + 1]));

    // get back bit count of wiegand
    bit_count = TOINT(value[index + 4]);

    // reset buffer
    memset((void *)wiegandBuffer, 0, wiegandBufferLength);
    ithFlushDCacheRange((void *)wiegandBuffer, wiegandBufferLength);
    return bit_count;
}

void
ithCodecPrintfWrite(
    char *string,
    int length)
{
    ithCodecUartWrite(string, length);
}

int
ithCodecUartRead(
    char *string,
    int length)
{
    int           timeout             = 100;
    int           commandBuffer       = TOINT(gCodecCtxt->commandBuffer) + CODEC_BASE;
    int           *oriCommandBuffer   = (int *) commandBuffer;
    int           commandBufferLength = TOINT(gCodecCtxt->commandBufferLength);
    int           returnLen           = 0;
    unsigned char *uartRxBuffer       = (unsigned char *)(TOINT(gCodecCtxt->uartrxBuffer) + CODEC_BASE);

    if (gCodecCtxt->commandBuffer == 0)
        return 0;

    ithInvalidateDCacheRange((void*)commandBuffer, 4);
    while (*((int *)commandBuffer) && timeout && ithReadRegH(0x16A4) != 0x0)
    {
        usleep(1000);
        timeout--;
        ithInvalidateDCacheRange((void*)commandBuffer, 4);
    }

    if (timeout == 0)
        return 0;

    /*RISC command should final adding , because RISC will get the command first and start it.*/
    oriCommandBuffer[1] = length;
    oriCommandBuffer[0] = 43;
    ithWriteRegH(0x16A4, 0x1111);
    ithFlushDCacheRange((void *)commandBuffer, commandBufferLength);

    ithInvalidateDCacheRange((void*)commandBuffer, 4);
    timeout = 100;
    while (*((int *)commandBuffer) && timeout)
    {
        usleep(1000);
        timeout--;
        ithInvalidateDCacheRange((void*)commandBuffer, 4);
    }

    if (timeout)
    {
        ithInvalidateDCacheRange((void*)uartRxBuffer, 320);
        returnLen = uartRxBuffer[0];
        //printf("len: %d\n", returnLen);
        if (returnLen)
        {
            memcpy(string, &uartRxBuffer[1], returnLen);
            uartRxBuffer[0] = 0;
            ithFlushDCacheRange((void *)uartRxBuffer, 1);
        }
    }

    return returnLen;
}

void
ithCodecUartWrite(
    char *string,
    int length)
{
    int timeout             = 100;
    int commandBuffer       = TOINT(gCodecCtxt->commandBuffer) + CODEC_BASE;
    int *oriCommandBuffer   = (int *) commandBuffer;
    int commandBufferLength = TOINT(gCodecCtxt->commandBufferLength);
    int uarttxBuffer        = TOINT(gCodecCtxt->uarttxBuffer) + CODEC_BASE;

    if (gCodecCtxt->commandBuffer == 0)
        return;

    ithInvalidateDCacheRange((void *)commandBuffer, 4);
    while (*((int *)commandBuffer) && timeout && ithReadRegH(0x16A4) != 0x0)
    {
        if (!Bootmode)
        {            
            timeout--;
        }
        ithInvalidateDCacheRange((void *)commandBuffer, 4);
    }

    if (timeout == 0)
        return;

    /*RISC command should final adding , because RISC will get the command first and start it.*/
    ithInvalidateDCacheRange((void *)uarttxBuffer, 320);

    memcpy((void*)uarttxBuffer, string, length);
    ithFlushDCacheRange((void *)uarttxBuffer, length);
    oriCommandBuffer[1] = length;
    oriCommandBuffer[0] = 44;
    ithWriteRegH(0x16A4, 0x1111);
    ithFlushDCacheRange((void *)commandBuffer, commandBufferLength);

    timeout = 100;
    ithInvalidateDCacheRange((void *)commandBuffer, 4);
    while (*((int *)commandBuffer) && timeout)
    {
        if (!Bootmode)
        {           
            timeout--;
        }
        ithInvalidateDCacheRange((void *)commandBuffer, 4);
    }
}

__attribute__((used))  void
ithCodecUartDbgWrite(
    char *string,
    int length)
{
    int timeout             = 100;
    int commandBuffer       = TOINT(gCodecCtxt->commandBuffer) + CODEC_BASE;
    int *oriCommandBuffer   = (int *) commandBuffer;
    int commandBufferLength = TOINT(gCodecCtxt->commandBufferLength);
    int returnLen           = 0;
    int uarttxdbgBuffer     = TOINT(gCodecCtxt->uarttxdbgBuffer) + CODEC_BASE;

    if (gCodecCtxt->commandBuffer == 0)
        return 0;

    ithInvalidateDCacheRange((void *)commandBuffer, 4);
    while (*((int *)commandBuffer) && timeout && ithReadRegH(0x16A4) != 0x0)
    {
        if (!Bootmode)
        {            
            timeout--;
        }
        ithInvalidateDCacheRange((void *)commandBuffer, 4);
    }

    if (timeout == 0)
        return 0;

    /*RISC command should final adding , because RISC will get the command first and start it.*/
    ithInvalidateDCacheRange((void *)uarttxdbgBuffer, 320);
    memcpy(uarttxdbgBuffer, string, length);
    ithFlushDCacheRange((void *)uarttxdbgBuffer, length);
    oriCommandBuffer[1] = length;
    oriCommandBuffer[0] = 52;
    ithWriteRegH(0x16A4, 0x1111);
    ithFlushDCacheRange((void *)commandBuffer, commandBufferLength);

    timeout = 100;
    ithInvalidateDCacheRange((void *)commandBuffer, 4);
    while (*((int *)commandBuffer) && timeout)
    {
        if (!Bootmode)
        {            
            timeout--;
        }
        ithInvalidateDCacheRange((void *)commandBuffer, 4);
    }
    return returnLen;
}

void
ithCodecCtrlBoardWrite(
    uint8_t *data,
    int length)
{
    int ctrlboardBuffer = TOINT(gCodecCtxt->ctrlboardBuffer) + CODEC_BASE;

    if (gCodecCtxt->ctrlboardBuffer == 0)
        return;

    ithInvalidateDCache();

    memcpy((void *)ctrlboardBuffer, data, length);
    ithFlushDCacheRange((void *)ctrlboardBuffer, length);
}

void
ithCodecCtrlBoardRead(
    uint8_t *data,
    int length)
{
    int ctrlboardBuffer = TOINT(gCodecCtxt->ctrlboardBuffer) + CODEC_BASE;

    if (gCodecCtxt->ctrlboardBuffer == 0)
        return;

    ithInvalidateDCache();
    memcpy(data, ctrlboardBuffer, length);
}

void
ithCodecHeartBeatRead(
    uint8_t *data,
    int length)
{
    int            heartbeatBuffer        = TOINT(gCodecCtxt->heartbeatBuffer) + CODEC_BASE;
    HEARTBEAT_INFO *pHeartbeat_RiscInfo   = (HEARTBEAT_INFO *)heartbeatBuffer;
    HEARTBEAT_INFO *pHeartbeat_ResultInfo = (HEARTBEAT_INFO *)data;
    int            writeIndex;

    if (gCodecCtxt->heartbeatBuffer == 0)
        return;

    ithInvalidateDCache();

    writeIndex = pHeartbeat_RiscInfo->WriteIndex;
    if (pHeartbeat_RiscInfo->ReadIndex != writeIndex)
    {
        if (writeIndex > pHeartbeat_RiscInfo->ReadIndex)
        {
            pHeartbeat_ResultInfo->WriteIndex = writeIndex - pHeartbeat_RiscInfo->ReadIndex;
            memcpy(pHeartbeat_ResultInfo->data,
                   &pHeartbeat_RiscInfo->data[pHeartbeat_RiscInfo->ReadIndex],
                   sizeof(pHeartbeat_RiscInfo->data[0]) * (writeIndex - pHeartbeat_RiscInfo->ReadIndex));
        }
        else
        {
            pHeartbeat_ResultInfo->WriteIndex = MAX_BUFFER_COUNT
                                                - pHeartbeat_RiscInfo->ReadIndex
                                                + writeIndex;
            memcpy(pHeartbeat_ResultInfo->data,
                   &pHeartbeat_RiscInfo->data[pHeartbeat_RiscInfo->ReadIndex],
                   sizeof(pHeartbeat_RiscInfo->data[0]) * (MAX_BUFFER_COUNT - pHeartbeat_RiscInfo->ReadIndex));

            if (writeIndex > 0)
                memcpy(&pHeartbeat_ResultInfo->data[MAX_BUFFER_COUNT - pHeartbeat_RiscInfo->ReadIndex],
                       &pHeartbeat_RiscInfo->data[0],
                       sizeof(pHeartbeat_RiscInfo->data[0]) * (writeIndex));
        }
        pHeartbeat_RiscInfo->ReadIndex = writeIndex;
    }
}

static void
RISC2_ResetProcessor(
    void)
{
    uint32_t i;
    uint32_t prevPc         = 0;
    uint32_t curPc          = 0;
    uint16_t communicateReg = 0;
    int32_t  timeoutCount   = 1000;
    LOG_ENTER "RISC2_ResetProcessor()\n" LOG_END

    prevPc = iteRiscGetProgramCounter(RISC2_CPU);
    for (i = 0; i < 64; i++)
    {
        asm ("");
    }
    curPc = iteRiscGetProgramCounter(RISC2_CPU);

    // Step 1. If CPU is runnning, then we try to wait to ensure system process is halt.
    if (gCodecCtxt->commandBuffer || gCodecCtxt->commandBufferLength || prevPc != curPc)
    {
        ithWriteRegH(RISC_COMMUNICATE_REG, 0x1234);
        while (timeoutCount-- > 0)
        {
            for (i = 0; i < 10000; i++)
            {
                asm ("");
            }
            communicateReg = ithReadRegH(RISC_COMMUNICATE_REG);
            if (communicateReg == 0x5678)
            {
                break;
            }
        }
    }

    iteRiscResetCpu(RISC2_CPU);
}


static void
RISC2_FireProcessor(
    void)
{
    iteRiscFireCpu(RISC2_CPU);
}

static int32_t
RISC2_LoadEngineImage(ITE_RISC_ENGINE engine_type)
{
    int imageSize = 0;
    switch (engine_type)
    {
    case ITE_SW_PERIPHERAL_ENGINE:
        // copy wiegand hex to gBuf
        imageSize = sizeof(gWiegand);
        iteRiscLoadData(RISC2_IMAGE_MEM_TARGET, gWiegand, imageSize);
        break;

    default:
        return 0;
    }

    __codec_ex_header          = (struct _codec_header *)(CODEC_EX_BASE);
    
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    __codec_ex_header->pStream = (char *)TOINT((int)gCodecCtxt - CODEC_BASE);
#endif

    return imageSize;
}

int32_t
iteRiscOpenEngine(
    ITE_RISC_ENGINE engine_type, uint32_t bootmode)
{
	int ticks = 0;
    pthread_mutex_lock(&RiscMutex);
    
    switch (engine_type)
    {
    case ITE_SW_PERIPHERAL_ENGINE:
        break;

    default:
        //printf("Engine Type (%d) is invalid.\n", engine_type);
        return -1;
    }
    gCodecCtxt = (CODEC_CONTEXT *) (iteRiscGetTargetMemAddress(SHARE_MEM1_TARGET) + sizeof(AUDIO_CODEC_STREAM));
    Bootmode = !!bootmode;
            
    // Reset RISC and then Fire RISC
    RISC2_ResetProcessor();
	ithWriteRegH(RISC_COMMUNICATE_REG, 0x0);
    if (RISC2_LoadEngineImage(engine_type))
    {
        RISC2_FireProcessor();
		while(1)
		{
			if (ithReadRegH(RISC_COMMUNICATE_REG) == 0x1)
			{
				//printf("RISC IS RUNNING COST (%d) ticks\n", ticks);
				break;
			}				
			if (ticks > 5000)
			{
				//printf("RISC TIMEOUT\n");
				break;
			}		
			ticks++;
		}
    }
    else
    {
        //printf("Load Engine (%d) is failed.\n", engine_type);
        pthread_mutex_unlock(&RiscMutex);
        return -2;
    }
    pthread_mutex_unlock(&RiscMutex);
    return 0;
}

int32_t
iteRiscTerminateEngine(
    void)
{
    LOG_ENTER "iteRiscTerminateEngine()\n" LOG_END
    pthread_mutex_lock(&RiscMutex);
    // Reset RISC and then Fire RISC
    RISC2_ResetProcessor();
    __codec_ex_header          = 0;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    __codec_ex_header->pStream = 0;
#endif
    pthread_mutex_unlock(&RiscMutex);
    return 0;
}