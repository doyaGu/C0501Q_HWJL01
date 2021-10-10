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

#include "core_config.h"
#include "ite/mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define INPUT_STREAM_SAMPLE_BUFFER_SIZE        (262144) //64K

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef enum QUEUE_ID_TAG
{
    INPUT_STREAM_QUEUE_ID = 0,
    VIDEO_QUEUE_ID,                         // 1
    SUBTITLE_QUEUE_ID,                      // 2
    TELETEXT_QUEUE_ID,                      // 3
    AUDIO_QUEUE_ID,                         // 4
    LAST_DATA_QUEUE_ID = AUDIO_QUEUE_ID,    // 4
    CMD_QUEUE1_ID,                          // 5
    CMD_QUEUE2_ID,                          // 6
    CMD_QUEUE3_ID,                          // 7
    CMD_QUEUE4_ID,                          // 8
    CMD_QUEUE5_ID,                          // 9
    CMD_QUEUE6_ID,                          // 10
    CMD_QUEUE7_ID,                          // 11
    CMD_QUEUE8_ID,                          // 12
    EVENT_QUEUE_ID,                         // 13
    SPECIAL_EVENT_QUEUE_ID,                 // 14
    TOTAL_QUEUE_COUNT                       // 15
} QUEUE_ID;

typedef enum QUEUE_MGR_ERROR_CODE_TAG
{
    QUEUE_MGR_NOT_INIT              = -6,
    QUEUE_INVALID_INPUT             = -5,
    QUEUE_IS_EMPTY                  = -4,
    QUEUE_IS_FULL                   = -3,
    QUEUE_EXIST                     = -2,
    QUEUE_NOT_EXIST                 = -1,
    QUEUE_NO_ERROR                  = 0
} QUEUE_MGR_ERROR_CODE;

typedef QUEUE_MGR_ERROR_CODE (*QUEUE_OPERATION) (QUEUE_ID queueId, void** pptSample);

typedef struct QUEUE_CTRL_HANDLE_TAG
{
    QUEUE_OPERATION pfGetFree;
    QUEUE_OPERATION pfSetFree;
    QUEUE_OPERATION pfGetReady;
    QUEUE_OPERATION pfSetReady;
} QUEUE_CTRL_HANDLE;

typedef struct INPUT_STREAM_SAMPLE_TAG
{
    MMP_UINT8*  pData;
    MMP_UINT32  bufferSize;
} INPUT_STREAM_SAMPLE;

typedef struct AUDIO_SAMPLE_TAG
{
    MMP_UINT8*      pData;
    MMP_UINT32      ringId;
    MMP_UINT8*      pAudioStart;
    MMP_UINT8*      pAudioEnd;
    MMP_UINT8*      pRingStart;
    MMP_UINT8*      pRingEnd;

    MMP_BOOL        bTimeStamp;
    MMP_UINT32      timeStamp;
    MMP_UINT32      errorCount;

    // for check dma copy ready
    MMP_BOOL        bWrap;
    MMP_UINT        sampleIndexCh0;
    MMP_UINT        sampleIndexCh1;
} AUDIO_SAMPLE;

typedef struct VIDEO_SAMPLE_TAG
{
    MMP_UINT8*      pData;
    MMP_UINT32      ringId;
    MMP_UINT8*      pVideoStart;
    MMP_UINT8*      pVideoEnd;
    MMP_UINT8*      pRingStart;
    MMP_UINT8*      pRingEnd;

    MMP_BOOL        bTimeStamp;
    MMP_UINT32      timeStamp;
    MMP_UINT32      errorCount;
    MMP_UINT32      stream_type;

    MMP_BOOL        bStartSegment;
    MMP_BOOL        bEndSegment;
    MMP_BOOL        bAVCIDR;

    MMP_BOOL        bSliceAlign;
    MMP_BOOL        bStuffing;

    // for check dma copy ready
    MMP_BOOL        bWrap;
    MMP_UINT        sampleIndexCh0;
    MMP_UINT        sampleIndexCh1;
} VIDEO_SAMPLE;

typedef struct SUBTITLE_SAMPLE_TAG
{
    MMP_UINT8*      pData;
    MMP_UINT32      ringId;
    MMP_UINT8*      pSubtitleStart;
    MMP_UINT8*      pSubtitleEnd;
    MMP_UINT8*      pRingStart;
    MMP_UINT8*      pRingEnd;

    MMP_BOOL        bTimeStamp;
    MMP_UINT32      timeStamp;
    MMP_UINT32      errorCount;
} SUBTITLE_SAMPLE;

typedef struct TELETEXT_SAMPLE_TAG
{
    MMP_UINT8*      pData;
    MMP_UINT32      ringId;
    MMP_UINT8*      pTeletextStart;
    MMP_UINT8*      pTeletextEnd;
    MMP_UINT8*      pRingStart;
    MMP_UINT8*      pRingEnd;

    MMP_BOOL        bTimeStamp;
    MMP_UINT32      timeStamp;
    MMP_UINT32      errorCount;
} TELETEXT_SAMPLE;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Used to initialize the simple queue manager.
 * @return  none
 */
//=============================================================================
void
queueMgr_InitManager(
    void);


//=============================================================================
/**
 * Used to destroy the simple queue manager.
 * @return  none
 */
//=============================================================================
void
queueMgr_DestroyManager(
    void);


//=============================================================================
/**
 * Used to Create a simple queue.
 * @param queueId       An identifier of the queue.
 * @param sampleSize    The size of data sample of the queue.
 * @param sampleCount   The total sample counts of the queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the creation is success
 *                      or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_CreateQueue(
    QUEUE_ID    queueId,
    MMP_UINT32  sampleSize,
    MMP_UINT32  sampleCount);


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
    QUEUE_ID    queueId);


//=============================================================================
/**
 * Set the queue control operation handle
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_MGR_ERROR_CODE whether the set operation is success
 *                  or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetCtrlHandle(
    QUEUE_ID            queueId,
    QUEUE_CTRL_HANDLE*  ptHandle);


//=============================================================================
/**
 * Get the queue control operation handle
 * @param queueId   An identifier of the queue.
 * @return          QUEUE_CTRL_HANDLE* the control operation handle of the
 *                  specific queue, However, if it is a null pointer, indicate
 *                  the get operation is failed.
 */
//=============================================================================
QUEUE_CTRL_HANDLE*
queueMgr_GetCtrlHandle(
    QUEUE_ID    queueId);


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
    QUEUE_ID    queueId,
    void**      pptSample);


//=============================================================================
/**
 * To release a used sample and update the readIndex of queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the pointer to a used sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_SetFree(
    QUEUE_ID    queueId,
    void**      pptSample);


//=============================================================================
/**
 * To get a ready sample from queue.
 * @param queueId       An identifier of the queue.
 * @param ptSample      the output pointer to a ready sample pointer of a queue.
 * @return              QUEUE_MGR_ERROR_CODE whether the queue operation is
 *                      success or not.
 */
//=============================================================================
QUEUE_MGR_ERROR_CODE
queueMgr_GetReady(
    QUEUE_ID    queueId,
    void**      pptSample);


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
    QUEUE_ID    queueId,
    void**      pptSample);


//=============================================================================
/**
 * To get a the current available entry count of queue
 * @param queueId       An identifier of the queue.
 * @return              Number of available entries.
 */
//=============================================================================
MMP_INT32
queueMgr_GetAvailableCount(
    QUEUE_ID    queueId);


#ifdef __cplusplus
}
#endif

#endif // End of #ifndef QUEUE_MGR_H
