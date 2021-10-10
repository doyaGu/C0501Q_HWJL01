/*
 * Copyright (c) 2007 ITE technology Corp. All Rights Reserved.
 */
/** @file stream_mgr.h
 * A simple message queue and queue buffer implementation.
 *
 * @author Steven Hsiao
 * @version 0.01
 */
#ifndef QUEUE_MGR_H
#define QUEUE_MGR_H

#include "def.h"
#include "share_info.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define INVALID_QUEUE_ID    (0)

typedef enum QUEUE_MGR_ERROR_CODE_TAG
{
    QUEUE_MGR_NOT_INIT              = -6,
    QUEUE_INVALID_INPUT             = -5,
    QUEUE_IS_EMPTY                  = -4,
    QUEUE_IS_FULL                   = -3,
    QUEUE_NO_ENOUGH_HANDLE          = -2,
    QUEUE_NOT_EXIST                 = -1,
    QUEUE_NO_ERROR                  = 0
} QUEUE_MGR_ERROR_CODE;

//=============================================================================
//                              Macro Definition
//=============================================================================
#define TOTAL_QUEUE_COUNT   MAX_PES_SAMPLE_COUNT

typedef uint                QUEUE_ID;

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
//                              Function Declaration
//=============================================================================

// output buffer control API
//      1. create
//          param 1. ring start address
//          param 2. ring block size
//          param 3. ring block count
//          return output buffer id
//      2. delete
//      3. get free (get next free block)
//      4. set ready
//      5. get ready
//      6. set free

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
queueMgr_CreateQueue(
    uint        channelId,
    uint        pesIndex,
    PID_INFO*   ptPidInfo,
    uint8*      pRiscBaseAddr);

//=============================================================================
/**
 * Used to Delete a simple queue
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_MGR_ERROR_CODE whether the deletion is success or
 *                  not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_DestroyQueue(
    QUEUE_DATA_BLOCK*   ptQueue);

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
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_GetFree(
    QUEUE_DATA_BLOCK*   ptQueue,
    void**              pptSample);

//=============================================================================
/**
 * To release a used sample and update the readIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a used sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
//QUEUE_MGR_ERROR_CODE
//queueMgr_SetFree(
//    uint        channelId,
//    uint        pesIndex,
//    void**      pptSample);

//=============================================================================
/**
 * To get a ready sample from queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a ready sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
//QUEUE_MGR_ERROR_CODE
//queueMgr_GetReady(
//    uint        channelId,
//    uint        pesIndex,
//    void**      pptSample,
//    uint32*     pSampleSize);

//=============================================================================
/**
 * To flagged a sample to ready state and update the writeIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a prepared ready sample pointer of queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetReady(
    QUEUE_DATA_BLOCK*   ptQueue,
    void**              pptSample,
    uint32*             pSampleSize);

//=============================================================================
/**
 * To get a the current available entry count of queue
 * @param queueId       An identifier of the queue.
 * @return              Number of available entries.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_GetAvailableCount(
    uint                channelId,
    uint                pesIndex,
    uint*               ptCount);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef QUEUE_MGR_H
