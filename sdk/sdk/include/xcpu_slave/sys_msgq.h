/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file sys_msgq.h
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef SYS_MSGQ_H
#define SYS_MSGQ_H

#include "ite/mmp_types.h"
#include "pal/pal.h"
#include "queue_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

/* Message type definitions */
enum
{
    SYS_MSG_TYPE_KBD = 0x1,
    SYS_MSG_TYPE_FILE,
    SYS_MSG_TYPE_CMD,
};

/* Message ID definitions */
// for SYS_MSG_TYPE_KBD
enum
{
    SYS_MSG_ID_KBD = 0x1,
    SYS_MSG_ID_IR,
    SYS_MSG_ID_TOUCH,
    SYS_MSG_ID_UART_CMD,
    SYS_MSG_ID_TOUCH_PANEL
};

// for SYS_MSG_TYPE_FILE
enum
{
    SYS_MSG_ID_FINFO = 0x1,
    SYS_MSG_ID_FINFO_RET,
    SYS_MSG_ID_FLOAD,
    SYS_MSG_ID_FLOAD_RET,
    SYS_MSG_ID_FSTORE,
    SYS_MSG_ID_FSTORE_RET,
    SYS_MSG_ID_FHOST_READY,
    SYS_MSG_ID_FHOST_READY_RET
};

// for SYS_MSG_TYPE_CMD
enum
{
	// System defined message,
    SYS_MSG_ID_SHUTDOWN = 0x1,
    SYS_MSG_ID_SHUTDOWN_RET,
    SYS_MSG_ID_START_ENCODER,
    SYS_MSG_ID_START_ENCODER_RET,
    SYS_MSG_ID_STOP_ENCODER,
    SYS_MSG_ID_STOP_ENCODER_RET,
    SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE,
    SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE_RET,
    SYS_MSG_ID_GET_INPUT_VIDEO_INFO,
    SYS_MSG_ID_GET_INPUT_VIDEO_INFO_RET,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_RESOLUTION,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_RESOLUTION_RET,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_RESOLUTION,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_RESOLUTION_RET,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_FRAME_RATE,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_FRAME_RATE_RET,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_FRAME_RATE,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_FRAME_RATE_RET,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_BIT_RATE,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_BIT_RATE_RET,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_BIT_RATE,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_BIT_RATE_RET,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_IFRAME_PERIOD,
    SYS_MSG_ID_GET_OUTPUT_ENCODER_IFRAME_PERIOD_RET,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_IFRAME_PERIOD,
    SYS_MSG_ID_SET_OUTPUT_ENCODER_IFRAME_PERIOD_RET,
    SYS_MSG_ID_GET_DEINTERLACE,
    SYS_MSG_ID_GET_DEINTERLACE_RET,
    SYS_MSG_ID_SET_DEINTERLACE,
    SYS_MSG_ID_SET_DEINTERLACE_RET,
    SYS_MSG_ID_GET_AUDIO_CODEC_TYPE,
    SYS_MSG_ID_GET_AUDIO_CODEC_TYPE_RET,
    SYS_MSG_ID_SET_AUDIO_CODEC_TYPE,
    SYS_MSG_ID_SET_AUDIO_CODEC_TYPE_RET,
    SYS_MSG_ID_GET_AUDIO_SAMPLE_RATE,
    SYS_MSG_ID_GET_AUDIO_SAMPLE_RATE_RET,
    SYS_MSG_ID_SET_AUDIO_SAMPLE_RATE,
    SYS_MSG_ID_SET_AUDIO_SAMPLE_RATE_RET,
    SYS_MSG_ID_GET_AUDIO_BIT_RATE,
    SYS_MSG_ID_GET_AUDIO_BIT_RATE_RET,
    SYS_MSG_ID_SET_AUDIO_BIT_RATE,
    SYS_MSG_ID_SET_AUDIO_BIT_RATE_RET,
    SYS_MSG_ID_GET_INPUT_SOURCE_CAPABILITY,
    SYS_MSG_ID_GET_INPUT_SOURCE_CAPABILITY_RET,
    SYS_MSG_ID_SET_INPUT_SOURCE_CAPABILITY,
    SYS_MSG_ID_SET_INPUT_SOURCE_CAPABILITY_RET,
    SYS_MSG_ID_GET_TIME,
    SYS_MSG_ID_GET_TIME_RET,
    SYS_MSG_ID_SET_TIME,
    SYS_MSG_ID_SET_TIME_RET,
    SYS_MSG_ID_GET_FW_VERSION,
    SYS_MSG_ID_GET_FW_VERSION_RET,
    SYS_MSG_ID_GET_STATE,
    SYS_MSG_ID_GET_STATE_RET,
    SYS_MSG_ID_GET_MODULATOR_PARA,
    SYS_MSG_ID_GET_MODULATOR_PARA_RET,
    SYS_MSG_ID_SET_MODULATOR_PARA,
    SYS_MSG_ID_SET_MODULATOR_PARA_RET,
	// User defined message,
    SYS_MSG_ID_CUSTOM_DATA,
};

// for Sub Command of SYS_MSG_ID_TOOL
enum
{
    TOOL_SUBCMD_READ_REG = 0x1,
    TOOL_SUBCMD_WRITE_REG,
    TOOL_SUBCMD_READ_DEMOD_REG,
    TOOL_SUBCMD_WRITE_DEMOD_REG,
    TOOL_SUBCMD_AHB_READ_REG,
    TOOL_SUBCMD_AHB_WRITE_REG,
    TOOL_SUBCMD_ACQUIRE_CHANNEL,
    TOOL_SUBCMD_READ_TUNER_REG,
    TOOL_SUBCMD_WRITE_TUNER_REG,
};

typedef enum SYS_MSGQ_ID_TAG
{
    SYS_MSGQ_ID_KBD,
    SYS_MSGQ_ID_FILE,
    SYS_MSGQ_ID_CMD,
    SYS_MSGQ_ID_COUNT
} SYS_MSGQ_ID;

typedef QUEUE_MGR_ERROR_CODE SYS_MSGQ_ERROR_CODE ;
/*
typedef enum SYS_MSGQ_ERROR_CODE_TAG
{
	QUEUE_MGR_NOT_INIT  = -6,
    QUEUE_INVALID_INPUT = -5,
    QUEUE_IS_EMPTY      = -4,
    QUEUE_IS_FULL       = -3,
    QUEUE_EXIST         = -2,
    QUEUE_NOT_EXIST     = -1,
    QUEUE_NO_ERROR      = 0
} SYS_MSGQ_ERROR_CODE;
*/

#define MAX_SYS_MSG_PARAM_COUNT (256)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct SYS_MSG_OBJ_TAG
{
    MMP_UINT16   type;
    MMP_UINT16   id;
    //MMP_UINT16   msg[MAX_SYS_MSG_PARAM_COUNT];
    MMP_UINT8   msg[MAX_SYS_MSG_PARAM_COUNT];
} SYS_MSG_OBJ;

//=============================================================================
//                              Function  Definition
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
    SYS_MSGQ_ID queueId);

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
    SYS_MSG_OBJ*    ptInputMsg);

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
    SYS_MSG_OBJ*    ptOutputMsg);

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
    SYS_MSG_OBJ*    ptOutputMsg);

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
    SYS_MSGQ_ID queueId);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef SYS_MSGQ_H

