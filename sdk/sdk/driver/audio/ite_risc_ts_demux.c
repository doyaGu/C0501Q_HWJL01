/*
 * Copyright (c) 2004 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Co-processor API functoin file.
 *      Date: 2012/09/17
 *
 */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "ite/audio.h"
#include "ite/ith.h"
#include "ite/ite_risc_ts_demux.h"
#include <sys/time.h>
#include "ite/itp.h"
//=============================================================================
//                              Constant Definition
//=============================================================================

#define INVALID_QUEUE_ID    (0)
#define TOTAL_QUEUE_COUNT   MAX_PES_SAMPLE_COUNT

typedef enum RISC_STATE_TAG
{
    RISC_STOP               = 0,
    RISC_RUN
} RISC_STATE;

typedef enum TS_DEMUX_QUEUE_ERROR_CODE_TAG
{
    TS_DEMUX_QUEUE_NOT_INIT              = -6,
    QUEUE_INVALID_INPUT             = -5,
    QUEUE_IS_EMPTY                  = -4,
    QUEUE_IS_FULL                   = -3,
    QUEUE_NO_ENOUGH_HANDLE          = -2,
    QUEUE_NOT_EXIST                 = -1,
    QUEUE_NO_ERROR                  = 0
} TS_DEMUX_QUEUE_ERROR_CODE;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct QUEUE_DATA_BLOCK_TAG
{
    PID_INFO*       ptPidInfo;

    uint32          pid;
    uint8*          pOutPesBuffer;
    uint32          pOutPesSampleSize;
    uint32          validPesSampleCount;
    uint32          pesBufferWriteIdx;
    uint32          pesBufferReadIdx;
} QUEUE_DATA_BLOCK;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

static CHANNEL_INFO_ARRAY*          gptShareInfo = 0;
static RISC_STATE                   gtRiscState = RISC_STOP;
static CHANNEL_SPS_PPS_INFO_ARRAY   gtSpsPpsInfoArray = { 0 };
static QUEUE_DATA_BLOCK             gtDataArray[MAX_CHANNEL_COUNT][MAX_PID_COUNT_PER_CHANNEL] = { 0 };

#ifdef CFG_RISC_TS_DEMUX_PLUGIN
static unsigned char gTsDemux[] = {
    #include "ts.hex"
};
#else
static unsigned char gTsDemux[] = {
    0x0
};
#endif

//=============================================================================
/**
 * Used to Create a simple queue.
 * @param pQueueId      An identifier of the queue.
 * @param pQueueStart   pointer to the start of the buffer
 *                      the totol size of the queue is equal to
 *                      (sampleSize * sampleCount)
 * @param sampleSize    The size of data sample of the queue.
 * @param sampleCount   The total sample counts of the queue.
 * @return              TS_DEMUX_QUEUE_ERROR_CODE whether the creation is success
 *                      or not.
 */
//=============================================================================
QUEUE_DATA_BLOCK*
_TsDemuxQueue_CreateQueue(
    uint        channelId,
    uint        pesIndex,
    PID_INFO*   ptPidInfo)
{
    QUEUE_DATA_BLOCK*   pQ = NULL;

    if (channelId <  MAX_CHANNEL_COUNT
     && pesIndex  <  MAX_PID_COUNT_PER_CHANNEL
     && NULL      != ptPidInfo)
    {
        pQ = &gtDataArray[channelId][pesIndex];

        memset(pQ, 0, sizeof(QUEUE_DATA_BLOCK));
        pQ->ptPidInfo           = ptPidInfo;
        pQ->pid                 = GET32(ptPidInfo->pid);
        pQ->pOutPesBuffer       = GET32(ptPidInfo->pOutPesBuffer);   // add memory offset?
        pQ->pOutPesSampleSize   = GET32(ptPidInfo->pOutPesSampleSize);
        pQ->validPesSampleCount = GET32(ptPidInfo->validPesSampleCount);
        pQ->pesBufferWriteIdx   = GET32(ptPidInfo->pesBufferWriteIdx);
        pQ->pesBufferReadIdx    = GET32(ptPidInfo->pesBufferReadIdx);
    }

    return pQ;
}

//=============================================================================
/**
 * Used to Delete a simple queue
 * @param queueId   An identifier of the queue.
 * @return          TS_DEMUX_QUEUE_ERROR_CODE wheter the deletion is successed or
 *                  not.
 */
//=============================================================================
TS_DEMUX_QUEUE_ERROR_CODE
_TsDemuxQueue_DestroyQueue(
    QUEUE_DATA_BLOCK*   ptQueue)
{
    // No such queue.
    if (NULL == ptQueue)
        return QUEUE_NOT_EXIST;

    memset(ptQueue, 0x0, sizeof(QUEUE_DATA_BLOCK));

    return QUEUE_NO_ERROR;
}

TS_DEMUX_QUEUE_ERROR_CODE
_TsDemuxQueue_DestroyQueueByIdx(
    uint        channelId,
    uint        pesIndex)
{
    QUEUE_DATA_BLOCK*   ptQueue;

    if (MAX_CHANNEL_COUNT           <= channelId
     || MAX_PID_COUNT_PER_CHANNEL   <= pesIndex)
        return QUEUE_INVALID_INPUT;

    ptQueue = &gtDataArray[channelId][pesIndex];

    return _TsDemuxQueue_DestroyQueue(ptQueue);
}

//=============================================================================
/**
 * To relese a used sample and update the readIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a used sample pointer of a queue.
 * @return              TS_DEMUX_QUEUE_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
TS_DEMUX_QUEUE_ERROR_CODE
_TsDemuxQueue_SetFree(
    uint        channelId,
    uint        pesIndex,
    void**      pptSample)
{
    QUEUE_DATA_BLOCK*   ptQueue;

    if (MAX_CHANNEL_COUNT           <= channelId
     || MAX_PID_COUNT_PER_CHANNEL   <= pesIndex
     || NULL                        == *pptSample)
        return QUEUE_INVALID_INPUT;

    ptQueue = &gtDataArray[channelId][pesIndex];

    if (*pptSample
     == (void*) (ptQueue->pOutPesBuffer
              + (ptQueue->pesBufferReadIdx * ptQueue->pOutPesSampleSize)))
    {
        if ((ptQueue->pesBufferReadIdx + 1) == ptQueue->validPesSampleCount)
            ptQueue->pesBufferReadIdx = 0;
        else
            ++ptQueue->pesBufferReadIdx;

        ptQueue->ptPidInfo->pesBufferReadIdx = ptQueue->pesBufferReadIdx;
        ithFlushMemBuffer();
        return QUEUE_NO_ERROR;
    }
    return QUEUE_INVALID_INPUT;
}

//=============================================================================
/**
 * To get a ready sample from queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a ready sample pointer of a queue.
 * @return              TS_DEMUX_QUEUE_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
TS_DEMUX_QUEUE_ERROR_CODE
_TsDemuxQueue_GetReady(
    uint        channelId,
    uint        pesIndex,
    void**      pptSample,
    uint32*     pSampleSize)
{
    QUEUE_DATA_BLOCK*   ptQueue;
    uint32 sampleSize = 0;
    if (MAX_CHANNEL_COUNT           <= channelId
     || MAX_PID_COUNT_PER_CHANNEL   <= pesIndex
     || NULL                        == pptSample
     || NULL                        == pSampleSize)
    {
        return QUEUE_INVALID_INPUT;
    }
    ptQueue = &gtDataArray[channelId][pesIndex];

    ithInvalidateDCacheRange(ptQueue->ptPidInfo, sizeof(PID_INFO));
    ptQueue->pesBufferWriteIdx = ptQueue->ptPidInfo->pesBufferWriteIdx;
    //printf("c(%d)p(%d) w(%d)r(%d)\n", channelId, pesIndex, ptQueue->pesBufferWriteIdx, ptQueue->pesBufferReadIdx);
    if (ptQueue->pesBufferReadIdx == ptQueue->pesBufferWriteIdx)
        return QUEUE_IS_EMPTY;
    //printf("r: %u, w: %u\n", ptQueue->pesBufferReadIdx, ptQueue->pesBufferWriteIdx);
    *pptSample = (void*) (ptQueue->pOutPesBuffer
                       + (ptQueue->pesBufferReadIdx * ptQueue->pOutPesSampleSize));
    //ptQueue->ptPidInfo->pesBufferReadIdx = ptQueue->pesBufferReadIdx;

    *pSampleSize = ptQueue->ptPidInfo->pOutputPesValidSize[ptQueue->pesBufferReadIdx];
    //ithInvalidateDCacheRange(*pptSample, *pSampleSize);
    return QUEUE_NO_ERROR;
}

// private?
static void
_TsDemux_SetCommand(
    XCPU_COMMAND    cmd)
{
    ithWriteRegH(XCPU_CMD_REG, (CPU_ID_ARM << 14) | cmd);
}

// private?
static XCPU_RETURN_CODE
_TsDemux_GetReturnCode(
    void)
{
    XCPU_RETURN_CODE    result      = XCPU_RETURN_CODE_NULL;
    uint16_t            commandReg  = ithReadRegH(XCPU_CMD_REG);
    uint32_t            cpuId       = (commandReg >> 14);
    uint32_t            code        = (commandReg &  0x3FFF);

    if (CPU_ID_RISC == cpuId
     && XCPU_RETURN_CODE_NULL < code && code < XCPU_RETURN_CODE_MAX)
    {
        result = (XCPU_RETURN_CODE)code;
    }

    return result;
}

// private?
static uint32_t
_TsDemux_WaitCommandDone(
    void)
{
    uint32_t i      = 60;
    uint32_t result = (uint32_t)-1;

    for (; 0 < i; --i)
    {
        result = _TsDemux_GetReturnCode();
        printf("result(%X), %s(%d)\n", result, __FUNCTION__, __LINE__);
        if (result == XCPU_RETURN_CODE_NULL)
            usleep(1000);
        else
        {
            if (result == XCPU_RETURN_CODE_DONE)
                result = 0;
            break;
        }
    }

    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    return result;
}

// for ts demux+
int32_t
iteTsDemux_OpenEngine(
    void)
{
    printf("%s(%d), size: %u\n", __FILE__, __LINE__, sizeof(gTsDemux));
    return iteAudioOpenExternalEngine(gTsDemux, sizeof(gTsDemux), (uint8**) &gptShareInfo);
}

CHANNEL_INFO_ARRAY*
iteTsDemux_GetChannelInfoArrayHandle(
    void)
{
    gptShareInfo->pRiscBaseAddr = (uint8*) iteAudioGetAudioCodecBufferBaseAddress();
    return gptShareInfo;
}

uint32_t
iteTsDemux_EnableBlockMode(
    uint        channelId)
{
    if (0 <= channelId && channelId < MAX_CHANNEL_COUNT)
    {
        CHANNEL_INFO*   ptChanInfo      = &gptShareInfo->tChannelArray[channelId];
        ptChanInfo->bBlockMode          = true;
        return 0;
    }
    return -1;
}

uint32_t
iteTsDemux_DisableBlockMode(
    uint        channelId)
{
    if (0 <= channelId && channelId < MAX_CHANNEL_COUNT)
    {
        CHANNEL_INFO*   ptChanInfo      = &gptShareInfo->tChannelArray[channelId];
        ptChanInfo->bBlockMode          = false;
        return 0;
    }
    return -1;
}

uint32_t
iteTsDemux_EnableChannel(
    uint        channelId)
{
    if (0 <= channelId && channelId < MAX_CHANNEL_COUNT)
    {
        CHANNEL_INFO*   ptChanInfo      = &gptShareInfo->tChannelArray[channelId];
        uint32          validPidCount   = ptChanInfo->validPidCount;
        uint32          i;

        ptChanInfo->bValidChannel       = true;
        _TsDemux_SetCommand(XCPU_COMMAND_CFG_CHANGE);
        _TsDemux_WaitCommandDone();

        gtSpsPpsInfoArray.tChannelSpsPpsArray[channelId].bValidChannel      = ptChanInfo->bValidChannel;
        gtSpsPpsInfoArray.tChannelSpsPpsArray[channelId].validServiceCount  = ptChanInfo->validPidCount;
        for (i = 0; i < validPidCount; ++i)
        {
            printf("%s port(%d), service(%d), &(ptChanInfo->tPidInfo[j])(0x%X)\n", __FUNCTION__,
                channelId, i, &(ptChanInfo->tPidInfo[i]));
            _TsDemuxQueue_CreateQueue(channelId, i, &(ptChanInfo->tPidInfo[i]));
        }

        return 0;
    }
    return -1;
}

uint32_t
iteTsDemux_DisableChannel(
    uint        channelId)
{
    if (0 <= channelId && channelId < MAX_CHANNEL_COUNT)
    {
        CHANNEL_INFO*   ptChanInfo      = &gptShareInfo->tChannelArray[channelId];
        uint32          validPidCount   = ptChanInfo->validPidCount;
        uint32          i;

        ptChanInfo->bValidChannel       = false;
        ptChanInfo->validPidCount       = 0;
        _TsDemux_SetCommand(XCPU_COMMAND_CFG_CHANGE);
        _TsDemux_WaitCommandDone();

        for (i = 0; i < validPidCount; ++i)
        {
            _TsDemuxQueue_DestroyQueueByIdx(channelId, i);
        }
        return 0;
    }
    return -1;
}

bool
iteTsDemux_GetNextFrame(
    uint        channelId,
    uint        pesIndex,
    uint8**     ppFrameStartAddr,
    uint32*     pFrameSize)
{
    return !_TsDemuxQueue_GetReady(channelId, pesIndex, (void**)ppFrameStartAddr, pFrameSize);
}

void
iteTsDemux_ReleaseFrame(
    uint        channelId,
    uint        pesIndex,
    uint8*      pFrameStartAddr)
{
    _TsDemuxQueue_SetFree(channelId, pesIndex, (void**)&pFrameStartAddr);
}

void
iteTsDemux_ExtractSpsPpsInfo(
    void)
{
    uint32 i = 0, j = 0;
    VIDEO_SPS_PPS_INFO* pVideoSpsPpsInfo = 0;
    uint8* pFrameBuffer = 0;
    uint32 frameSize = 0;
    uint32 state = 0;
    int32  remainSize = 0;
    uint8* pCurPtr = 0;
    uint8* pVideoStartPtr = 0;
    uint8* pNalBoundary = 0;
    uint32 pesHeaderLen = 0;
    uint32 nalType = 0;
    uint32 bufferSize = 0;

    struct timeval startT, curT;

    for (i = 0; i < MAX_CHANNEL_COUNT; i++)
    {
        CHANNEL_SPS_PPS_INFO* ptChanInfo = &gtSpsPpsInfoArray.tChannelSpsPpsArray[i];
        if (!ptChanInfo->bValidChannel)
            continue;

        iteTsDemux_DisableBlockMode(i);
        for (j = 0; j < gtSpsPpsInfoArray.tChannelSpsPpsArray[i].validServiceCount; j++)
        {
            pVideoSpsPpsInfo = &gtSpsPpsInfoArray.tChannelSpsPpsArray[i].tSpsPpsInfo[j];

            gettimeofday(&startT, NULL);
            curT = startT;

            while( (curT.tv_sec - startT.tv_sec) < 5 /* 8 sec timeout */ &&
                   (0 == pVideoSpsPpsInfo->spsSize || 0 == pVideoSpsPpsInfo->ppsSize) )
            {
                gettimeofday(&curT, NULL);

                state = 0;
                if (iteTsDemux_GetNextFrame(i, j, &pFrameBuffer, &frameSize))
                {
                    gettimeofday(&startT, NULL);
                    remainSize = frameSize;
                    pCurPtr = pFrameBuffer;
                    while (remainSize > 0)
                    {
                        switch(state)
                        {
                        case 0: // Find start code
                            {
                                if (pCurPtr[0] == 0x0 && pCurPtr[1] == 0x0 && pCurPtr[2] == 0x1 && pCurPtr[3] == 0xE0)
                                {
                                    pesHeaderLen = pCurPtr[8];
                                    pCurPtr += (9 + pesHeaderLen);
                                    remainSize -= (9 + pesHeaderLen);
                                    state = 1;
                                }
                                else
                                {
                                    pCurPtr++;
                                    remainSize--;
                                }
                            }
                            break;
                        case 1:
                            {
                                // Get Nal Type
                                nalType = (pCurPtr[4] & 0x1F);
                                pCurPtr += 4;
                                remainSize -= 4;
                                pVideoStartPtr = pCurPtr;
                                if (nalType != 7 && nalType != 8)
                                {
                                    while (remainSize > 0)
                                    {
                                        if (pCurPtr[0] == 0x0 && pCurPtr[1] == 0x0 && pCurPtr[2] == 0x00 && pCurPtr[3] == 0x01)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            pCurPtr++;
                                            remainSize--;
                                        }
                                    }
                                    //remainSize = 0;
                                    break;
                                }

                                // sps
                                if (nalType == 7)
                                {
                                    while (remainSize > 0)
                                    {
                                        if (pCurPtr[0] == 0x0 && pCurPtr[1] == 0x0 && pCurPtr[2] == 0x00 && pCurPtr[3] == 0x01)
                                        {
                                            pVideoSpsPpsInfo->spsSize = (uint32) (pCurPtr - pVideoStartPtr);
                                            pVideoSpsPpsInfo->pSps = (uint8*) malloc(pVideoSpsPpsInfo->spsSize);
                                            memcpy(pVideoSpsPpsInfo->pSps, pVideoStartPtr, pVideoSpsPpsInfo->spsSize);
                                            break;
                                        }
                                        else
                                        {
                                            pCurPtr++;
                                            remainSize--;
                                        }
                                    }
                                }
                                // pps
                                if (nalType == 8)
                                {
                                    while (remainSize > 0)
                                    {
                                        if (pCurPtr[0] == 0x0 && pCurPtr[1] == 0x0 && pCurPtr[2] == 0x00 && pCurPtr[3] == 0x01)
                                        {
                                            pVideoSpsPpsInfo->ppsSize = (uint32) (pCurPtr - pVideoStartPtr);
                                            pVideoSpsPpsInfo->pPps = (uint8*) malloc(pVideoSpsPpsInfo->ppsSize);
                                            memcpy(pVideoSpsPpsInfo->pPps, pVideoStartPtr, pVideoSpsPpsInfo->ppsSize);
                                            break;
                                        }
                                        else
                                        {
                                            pCurPtr++;
                                            remainSize--;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                    iteTsDemux_ReleaseFrame(i, j, pFrameBuffer);
                }
            }

            if( 0 == pVideoSpsPpsInfo->spsSize || 0 == pVideoSpsPpsInfo->ppsSize )
            {
                printf("err ! %d-th channel, %d-th server => spsSize=%d, ppsSize=%d\n", i, j, pVideoSpsPpsInfo->spsSize, pVideoSpsPpsInfo->ppsSize);
            }
        }

        iteTsDemux_EnableBlockMode(i);
    }

    for (i = 0; i < MAX_CHANNEL_COUNT; i++)
    {
        CHANNEL_SPS_PPS_INFO* ptChanInfo = &gtSpsPpsInfoArray.tChannelSpsPpsArray[i];
        if (!ptChanInfo->bValidChannel)
            continue;

        for (j = 0; j < gtSpsPpsInfoArray.tChannelSpsPpsArray[i].validServiceCount; j++)
        {
            pVideoSpsPpsInfo = &gtSpsPpsInfoArray.tChannelSpsPpsArray[i].tSpsPpsInfo[j];
        }
    }
}

void
iteTsDemux_GetSpsPpsInfo(
    uint32   channelId,
    uint32   serviceId,
    uint8**  ppSps,
    uint32*  pSpsSize,
    uint8**  ppPps,
    uint32*  pPpsSize)
{
    VIDEO_SPS_PPS_INFO* pVideoSpsPpsInfo = 0;
    if (0 <= channelId && channelId < MAX_CHANNEL_COUNT
     && gtSpsPpsInfoArray.tChannelSpsPpsArray[channelId].bValidChannel
     && serviceId <= gtSpsPpsInfoArray.tChannelSpsPpsArray[channelId].validServiceCount)
    {
        pVideoSpsPpsInfo = &gtSpsPpsInfoArray.tChannelSpsPpsArray[channelId].tSpsPpsInfo[serviceId];
        *ppSps = pVideoSpsPpsInfo->pSps;
        *pSpsSize = pVideoSpsPpsInfo->spsSize;
        *ppPps = pVideoSpsPpsInfo->pPps;
        *pPpsSize = pVideoSpsPpsInfo->ppsSize;
    }
}

// for ts demux-
