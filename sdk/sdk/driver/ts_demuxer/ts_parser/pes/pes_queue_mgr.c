/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file queue_mgr.c
 * A simple message queue and queue buffer implementation.
 *
 * @author Steven Hsiao
 * @version 0.01
 */

#include "pes_queue_mgr.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

static QUEUE_DATA_BLOCK     gtDataArray[MAX_CHANNEL_COUNT][MAX_PID_COUNT_PER_CHANNEL] = { 0 };

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Used to Create a simple queue.
 * @param pQueueId      An identifier of the queue.
 * @param pQueueStart   pointer to the start of the buffer
 *                      the totol size of the queue is equal to
 *                      (sampleSize * sampleCount)
 * @param sampleSize    The size of data sample of the queue.
 * @param sampleCount   The total sample counts of the queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the creation is success
 *                      or not.
 */
//=============================================================================
QUEUE_DATA_BLOCK*
pesQm_CreateQueue(
    uint            channelId,
    PES_PID_TYPE    pesIndex,
    PID_INFO*       ptPidInfo,
    uint8*          pRiscBaseAddr)
{
    QUEUE_DATA_BLOCK*   pQ = MMP_NULL;

    if (channelId <  MAX_CHANNEL_COUNT
     && pesIndex <  MAX_PID_COUNT_PER_CHANNEL
     && MMP_NULL != ptPidInfo)
    {
        pQ = &gtDataArray[channelId][pesIndex];

        PalMemset(pQ, 0, sizeof(QUEUE_DATA_BLOCK));
        pQ->ptPidInfo           = ptPidInfo;
        pQ->pid                 = GET32(ptPidInfo->pid);
        pQ->pOutPesBuffer       = (uint8*)(GET32((uint)ptPidInfo->pOutPesBuffer)
                                    - (uint)pRiscBaseAddr);
        pQ->pesOutSampleSize    = GET32(ptPidInfo->pesOutSampleSize);
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
 * @return          QUEUE_MGR_ERROR_CODE wheter the deletion is successed or
 *                  not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
pesQm_DestroyQueue(
    QUEUE_DATA_BLOCK*   ptQueue)
{
    // No such queue.
    if (MMP_NULL == ptQueue)
        return QUEUE_NOT_EXIST;

    PalMemset(ptQueue, 0x0, sizeof(QUEUE_DATA_BLOCK));

    return QUEUE_NO_ERROR;
}

// Note1: If a user create a queue as a queue buffer, these API can be
//        hanged on CTRL_HANDLE block of queue then be used directly.
// Note2: However, if a user use the queue as a message queue, sometimes,
//        the user module must be notified to free or clear some buffer or
//        data pointer. Under such a case, the user has to implement proper
//        queue operation functions. A simple example code is shown below for
//        reference.
//        QUEUE_MGR_ERROR_CODE_TAG
//        testExample_SetFree(
//            QUEUE_ID            queueId,
//            void**              pptSample)
//        {
//            QUEUE_MGR_ERROR_CODE_TAG result = QUEUE_NO_ERROR;
//            if ((result = queueMgr_SetFree(queueId, pptSample))
//             != QUEUE_NO_ERROR)
//            {
//                return result;
//            }
//            PalHeapFree(PAL_HEAP_DEAFULT, ((XXX_SAMPLE*)*pptSample)->pData);
//        }
//
//=============================================================================
/**
 * To get a free sample from queue
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a free sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
pesQm_GetFree(
    QUEUE_DATA_BLOCK*   ptQueue,
    void**              pptSample)
{
    if (MMP_NULL == ptQueue
     || MMP_NULL == pptSample)
        return QUEUE_INVALID_INPUT;

    *pptSample = (void*) (ptQueue->pOutPesBuffer
                       + (ptQueue->pesBufferWriteIdx * ptQueue->pesOutSampleSize));
    return QUEUE_NO_ERROR;
}

//=============================================================================
/**
 * To relese a used sample and update the readIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a used sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
pesQm_SetFree(
    uint            channelId,
    PES_PID_TYPE    pesIndex,
    void**          pptSample)
{
    QUEUE_DATA_BLOCK*   ptQueue;

    if (MAX_CHANNEL_COUNT           <= channelId
     || MAX_PID_COUNT_PER_CHANNEL   <= pesIndex
     || MMP_NULL                    == *pptSample)
        return QUEUE_INVALID_INPUT;

    ptQueue = &gtDataArray[channelId][pesIndex];

    if (*pptSample
        == (void*) (ptQueue->pOutPesBuffer
                 + (ptQueue->pesBufferReadIdx * ptQueue->pesOutSampleSize)))
    {
        if ((ptQueue->pesBufferReadIdx + 1) == ptQueue->validPesSampleCount)
            ptQueue->pesBufferReadIdx = 0;
        else
            ++ptQueue->pesBufferReadIdx;

        ptQueue->ptPidInfo->pesBufferReadIdx = ptQueue->pesBufferReadIdx;
        return QUEUE_NO_ERROR;
    }
    return QUEUE_INVALID_INPUT;
}

//=============================================================================
/**
 * To get a ready sample from queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a ready sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
pesQm_GetReady(
    uint            channelId,
    PES_PID_TYPE    pesIndex,
    void**          pptSample,
    uint32*         pSampleSize)
{
    QUEUE_DATA_BLOCK*   ptQueue;

    if (MAX_CHANNEL_COUNT           <= channelId
     || MAX_PID_COUNT_PER_CHANNEL   <= pesIndex
     || MMP_NULL                    == pptSample
     || MMP_NULL                    == pSampleSize)
        return QUEUE_INVALID_INPUT;

    ptQueue = &gtDataArray[channelId][pesIndex];

    ptQueue->pesBufferWriteIdx = ptQueue->ptPidInfo->pesBufferWriteIdx;
    if (ptQueue->pesBufferReadIdx == ptQueue->pesBufferWriteIdx)
        return QUEUE_IS_EMPTY;

    *pptSample = (void*) (ptQueue->pOutPesBuffer
                       + (ptQueue->pesBufferReadIdx * ptQueue->pesOutSampleSize));

    *pSampleSize = ptQueue->ptPidInfo->pOutputPesValidSize[ptQueue->pesBufferReadIdx];
    return QUEUE_NO_ERROR;
}

//=============================================================================
/**
 * To flagged a sample to ready state and update the writeIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a prepared ready sample pointer of queue.
 * @return              QUEUE_MGR_ERROR_CODE wheter the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
pesQm_SetReady(
    QUEUE_DATA_BLOCK*   ptQueue,
    void**              pptSample,
    uint32*             pSampleSize)
{
    if (MMP_NULL == ptQueue
     || MMP_NULL == pptSample
     || MMP_NULL == *pptSample)
    {
        return QUEUE_INVALID_INPUT;
    }

    if (*pptSample
     == (void*) (ptQueue->pOutPesBuffer
              + (ptQueue->pesBufferWriteIdx * ptQueue->pesOutSampleSize)))
    {
        SET32(ptQueue->ptPidInfo->pOutputPesValidSize[ptQueue->pesBufferWriteIdx], (*pSampleSize));
        if ((ptQueue->pesBufferWriteIdx + 1) == ptQueue->validPesSampleCount)
        {
            ptQueue->pesBufferWriteIdx = 0;
        }
        else
        {
            ++ptQueue->pesBufferWriteIdx;
        }
        SET32(ptQueue->ptPidInfo->pesBufferWriteIdx, ptQueue->pesBufferWriteIdx);

        return QUEUE_NO_ERROR;
    }
    return QUEUE_INVALID_INPUT;
}

//=============================================================================
/**
 * To get a the current available entry count of queue
 * @param queueId       An identifier of the queue.
 * @return              Number of available entries.
 */
//=============================================================================
//int32
//pesQm_GetAvailableCount(
//    QUEUE_DATA_BLOCK*   ptQueue)
//{
//    if (MMP_NULL == ptQueue)
//        return 0;
//
//    return (ptQueue->pesBufferWriteIdx >= ptQueue->pesBufferReadIdx)
//         ? (int32)(ptQueue->validPesSampleCount - ptQueue->pesBufferWriteIdx + ptQueue->pesBufferReadIdx)
//         : (int32)(ptQueue->pesBufferReadIdx - ptQueue->pesBufferWriteIdx);
//}
