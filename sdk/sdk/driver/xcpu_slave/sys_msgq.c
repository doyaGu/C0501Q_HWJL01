/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file sys_msgq.c
 *
 * @author I-Chun Lai
 * @version 0.1
 */

//#include "../mmp/include/pal/pal.h"
#include "pal/pal.h"
#include "sys_msgq.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MAX_QUEUE_SIZE  (20)

#define MAX_SYS_KBD_MSGQ_SIZE   (2)
#define MAX_SYS_FILE_MSGQ_SIZE  (16)
#define MAX_SYS_CMD_MSGQ_SIZE   (16)

//=============================================================================
//                              Macro Definition
//=============================================================================

#define _COUNT_OF_MSG( w, r, max)   ((r <= w) ? (w - r) : (max - (r - w)))
#define _IS_MSGQ_FULL( w, r, max)   (_COUNT_OF_MSG(w, r, max) == max - 1)
#define _IS_MSGQ_EMPTY(w, r, max)   (_COUNT_OF_MSG(w, r, max) == 0)

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct SYS_MSGQ_TAG {
    SYS_MSG_OBJ* const  pBuffer;
    const MMP_UINT32    capacity;
    MMP_UINT32          readIndex;
    MMP_UINT32          writeIndex;
} SYS_MSGQ;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

static SYS_MSG_OBJ gtSysKbdMsgQ[MAX_SYS_KBD_MSGQ_SIZE];
static SYS_MSG_OBJ gtSysFileMsgQ[MAX_SYS_FILE_MSGQ_SIZE];
static SYS_MSG_OBJ gtSysCmdMsgQ[MAX_SYS_CMD_MSGQ_SIZE];

static SYS_MSGQ gtSysMsgQ[SYS_MSGQ_ID_COUNT] =
{
    {gtSysKbdMsgQ,  MAX_SYS_KBD_MSGQ_SIZE,  0, 0},
    {gtSysFileMsgQ, MAX_SYS_FILE_MSGQ_SIZE, 0, 0},
    {gtSysCmdMsgQ,  MAX_SYS_CMD_MSGQ_SIZE,  0, 0}
};

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Create a system message queue for storing a specific type of messages.
 *
 * @param queueId   An identifier of the queue.
 * @return  none
 */
//=============================================================================
void
sysMsgQ_Init(
    SYS_MSGQ_ID queueId)
{
    if (queueId < SYS_MSGQ_ID_COUNT)
    {
        gtSysMsgQ[queueId].readIndex  = 0;
        gtSysMsgQ[queueId].writeIndex = 0;
    }
}

//=============================================================================
/**
 * Send a message into a specific type of system message queue.
 *
 * @param queueId       An identifier of the queue.
 * @param ptInputMsg    Specifies the message to be sent.
 * @return one of the following values that describe the results of the
 *         operation:
 *          1. QUEUE_NO_ERROR       => The message was sent successful.
 *          2. QUEUE_NOT_EXIST      => The message queue does not exist.
 *          3. QUEUE_IS_FULL        => The message queue is full and cannot
 *                                     receive any new message.
 *          4. QUEUE_INVALID_INPUT  => The input message is null.
 */
//=============================================================================
SYS_MSGQ_ERROR_CODE
sysMsgQ_SendMessage(
    SYS_MSGQ_ID     queueId,
    SYS_MSG_OBJ*    ptInputMsg)
{
    SYS_MSGQ_ERROR_CODE result = QUEUE_NO_ERROR;

    do
    {
        if (SYS_MSGQ_ID_COUNT <= queueId || MMP_NULL == ptInputMsg)
        {
            result = QUEUE_INVALID_INPUT;
            break;
        }
        if (MMP_NULL == gtSysMsgQ[queueId].pBuffer)
        {
            result = QUEUE_NOT_EXIST;
            break;
        }
        if (_IS_MSGQ_FULL(gtSysMsgQ[queueId].writeIndex,
                           gtSysMsgQ[queueId].readIndex,
                           gtSysMsgQ[queueId].capacity))
        {
            result = QUEUE_IS_FULL;
            break;
        }

        PalMemcpy(
            gtSysMsgQ[queueId].pBuffer + gtSysMsgQ[queueId].writeIndex,
            ptInputMsg,
            sizeof(SYS_MSG_OBJ));

        if (gtSysMsgQ[queueId].capacity <= ++gtSysMsgQ[queueId].writeIndex)
        {
            gtSysMsgQ[queueId].writeIndex = 0;
        }
    } while(0);

    if (result != 0)
        dbg_msg(DBG_MSG_TYPE_ERROR, "sysMsgQ_SendMessage error! %d\n", result);

    return result;
}

//=============================================================================
/**
 * Retrieve a message from a specific type of system message queue (if any
 * exist).
 *
 * @param queueId       An identifier of the queue.
 * @param ptOutputMsg   Pointer to an SYS_MSG_OBJ structure that contains the
 *                      message information received.
 * @return one of the following values that describe the results of the
 *         operation:
 *          1. QUEUE_NO_ERROR       => The message was sent successful.
 *          2. QUEUE_NOT_EXIST      => The message queue does not exist.
 *          3. QUEUE_IS_EMPTY       => The message queue is empty.
 *          4. QUEUE_INVALID_INPUT  => The input message is null.
 */
//=============================================================================
SYS_MSGQ_ERROR_CODE
sysMsgQ_ReceiveMessage(
    SYS_MSGQ_ID     queueId,
    SYS_MSG_OBJ*    ptOutputMsg)
{
    SYS_MSGQ_ERROR_CODE result = QUEUE_NO_ERROR;

    do
    {
        if (SYS_MSGQ_ID_COUNT <= queueId || MMP_NULL == ptOutputMsg)
        {
            result = QUEUE_INVALID_INPUT;
            break;
        }
        if (MMP_NULL == gtSysMsgQ[queueId].pBuffer)
        {
            result = QUEUE_NOT_EXIST;
            break;
        }
        if (_IS_MSGQ_EMPTY(gtSysMsgQ[queueId].writeIndex,
                           gtSysMsgQ[queueId].readIndex,
                           gtSysMsgQ[queueId].capacity))
        {
            result = QUEUE_IS_EMPTY;
            break;
        }

        PalMemcpy(
            ptOutputMsg,
            gtSysMsgQ[queueId].pBuffer + gtSysMsgQ[queueId].readIndex,
            sizeof(SYS_MSG_OBJ));

        if (gtSysMsgQ[queueId].capacity <= ++gtSysMsgQ[queueId].readIndex)
        {
            gtSysMsgQ[queueId].readIndex = 0;
        }
    } while(0);

    return result;
}

//=============================================================================
/**
 * retrieve a message from a specific type of system message queue (if any
 * exist) but not remove the retrieved message from the message queue.
 *
 * @param queueId       An identifier of the queue.
 * @param ptOutputMsg   Pointer to an SYS_MSG_OBJ structure that contains the
 *                      message information received.
 * @return one of the following values that describe the results of the
 *         operation:
 *          1. QUEUE_NO_ERROR       => The message was sent successful.
 *          2. QUEUE_NOT_EXIST      => The message queue does not exist.
 *          3. QUEUE_IS_EMPTY       => The message queue is empty.
 *          4. QUEUE_INVALID_INPUT  => The input message is null.
 */
//=============================================================================
SYS_MSGQ_ERROR_CODE
sysMsgQ_CheckMessage(
    SYS_MSGQ_ID     queueId,
    SYS_MSG_OBJ*    ptOutputMsg)
{
    SYS_MSGQ_ERROR_CODE result = QUEUE_NO_ERROR;

    do
    {
        if (SYS_MSGQ_ID_COUNT <= queueId || MMP_NULL == ptOutputMsg)
        {
            result = QUEUE_INVALID_INPUT;
            break;
        }
        if (MMP_NULL == gtSysMsgQ[queueId].pBuffer)
        {
            result = QUEUE_NOT_EXIST;
            break;
        }
        if (_IS_MSGQ_EMPTY(gtSysMsgQ[queueId].writeIndex,
                           gtSysMsgQ[queueId].readIndex,
                           gtSysMsgQ[queueId].capacity))
        {
            result = QUEUE_IS_EMPTY;
            break;
        }

        PalMemcpy(
            ptOutputMsg,
            gtSysMsgQ[queueId].pBuffer + gtSysMsgQ[queueId].readIndex,
            sizeof(SYS_MSG_OBJ));
    } while(0);

    return result;
}

//=============================================================================
/**
 * Destroy a specific type of system message queue.
 *
 * @param queueId   An identifier of the queue.
 * @return  none
 */
//=============================================================================
void
sysMsgQ_Terminate(
    SYS_MSGQ_ID queueId)
{
    if (queueId < SYS_MSGQ_ID_COUNT)
    {
        gtSysMsgQ[queueId].readIndex  = 0;
        gtSysMsgQ[queueId].writeIndex = 0;
    }
}
