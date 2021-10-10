/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file sys_msgq.h
 *
 * @version 0.1
 */

#ifndef SYS_MSGQ_H
#define SYS_MSGQ_H

#include "pal.h"
//#include "../driver/mmp/include/mmp_types.h"

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
    SYS_MSG_ID_TOUCH
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
    SYS_MSG_ID_SHUTDOWN = 0x1,
    SYS_MSG_ID_SHUTDOWN_RET,
    SYS_MSG_ID_INIT_ENCODER,
    SYS_MSG_ID_INIT_ENCODER_RET,
    SYS_MSG_ID_START_ENCODER,
    SYS_MSG_ID_START_ENCODER_RET,
    SYS_MSG_ID_STOP_ENCODER,
    SYS_MSG_ID_STOP_ENCODER_RET,
    SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE,
    SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE_RET,
    SYS_MSG_ID_SET_INPUT_SOURCE_DEVICE,
    SYS_MSG_ID_SET_INPUT_SOURCE_DEVICE_RET,
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
    SYS_MSG_ID_GET_MAX_OUTPUT_ENCODER_FRAME_RATE,
    SYS_MSG_ID_GET_MAX_OUTPUT_ENCODER_FRAME_RATE_RET,
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
    SYS_MSG_ID_SET_RETURN_CHANNEL_PARA,
    SYS_MSG_ID_SET_RETURN_CHANNEL_PARA_RET,
    SYS_MSG_ID_GET_MODULATOR_PARA,
    SYS_MSG_ID_GET_MODULATOR_PARA_RET,
    SYS_MSG_ID_SET_MODULATOR_PARA,
    SYS_MSG_ID_SET_MODULATOR_PARA_RET,
    SYS_MSG_ID_SET_ROOT,
    SYS_MSG_ID_SET_ROOT_RET,

    SYS_MSG_ID_UPDATE_TS_ID,
    SYS_MSG_ID_UPDATE_TS_ID_RET,
    SYS_MSG_ID_UPDATE_NETWORK_NAME,
    SYS_MSG_ID_UPDATE_NETWORK_NAME_RET,
    SYS_MSG_ID_UPDATE_NETWORK_ID,
    SYS_MSG_ID_UPDATE_NETWORK_ID_RET,
    SYS_MSG_ID_UPDATE_ORIGINAL_NETWORK_ID,
    SYS_MSG_ID_UPDATE_ORIGINAL_NETWORK_ID_RET,
    SYS_MSG_ID_UPDATE_SERVICE_LIST_DESCRIPTOR,
    SYS_MSG_ID_UPDATE_SERVICE_LIST_DESCRIPTOR_RET,
    SYS_MSG_ID_UPDATE_COUNTRY_ID,
    SYS_MSG_ID_UPDATE_COUNTRY_ID_RET,
    SYS_MSG_ID_UPDATE_LCN,
    SYS_MSG_ID_UPDATE_LCN_RET,
    SYS_MSG_ID_UPDATE_SERVICE_NAME,
    SYS_MSG_ID_UPDATE_SERVICE_NAME_RET,
    SYS_MSG_ID_GET_SERVICE_COUNT,
    SYS_MSG_ID_GET_SERVICE_COUNT_RET,
    SYS_MSG_ID_UPDATE_SERVICE_ID,
    SYS_MSG_ID_UPDATE_SERVICE_ID_RET,
    SYS_MSG_ID_UPDATE_SERVICE_PMT_PID,
    SYS_MSG_ID_UPDATE_SERVICE_PMT_PID_RET,
    SYS_MSG_ID_UPDATE_SERVICE_ES_PID,
    SYS_MSG_ID_UPDATE_SERVICE_ES_PID_RET,

    SYS_MSG_ID_GPIO_ENABLE,
    SYS_MSG_ID_GPIO_ENABLE_RET,
    SYS_MSG_ID_GPIO_DISABLE,
    SYS_MSG_ID_GPIO_DISABLE_RET,
    SYS_MSG_ID_GPIO_SET_MODE,
    SYS_MSG_ID_GPIO_SET_MODE_RET,
    SYS_MSG_ID_GPIO_SET_STATE,
    SYS_MSG_ID_GPIO_SET_STATE_RET,
    SYS_MSG_ID_GPIO_GET_STATE,
    SYS_MSG_ID_GPIO_GET_STATE_RET,
    SYS_MSG_ID_GPIO_SET_CAM_PW_ON,
    SYS_MSG_ID_GPIO_SET_CAM_PW_ON_RET,
    SYS_MSG_ID_GPIO_SET_CAM_STANDBY,
    SYS_MSG_ID_GPIO_SET_CAM_STANDBY_RET,
    SYS_MSG_ID_GPIO_SET_CAM_LED_ON,
    SYS_MSG_ID_GPIO_SET_CAM_LED_ON_RET,
    SYS_MSG_ID_GPIO_SET_CAM_LED_OFF,
    SYS_MSG_ID_GPIO_SET_CAM_LED_OFF_RET,
    SYS_MSG_ID_SET_SENSOR_ANTIFLICKER,
    SYS_MSG_ID_SET_SENSOR_ANTIFLICKER_RET,
    SYS_MSG_ID_SET_SENSOR_MIRROR,
    SYS_MSG_ID_SET_SENSOR_MIRROR_RET,
    SYS_MSG_ID_SET_SENSOR_EFFECT,
    SYS_MSG_ID_SET_SENSOR_EFFECT_RET,
    SYS_MSG_ID_JPEG_RECORD,
    SYS_MSG_ID_JPEG_RECORD_RET,
    SYS_MSG_ID_JPEG_STOP,
    SYS_MSG_ID_JPEG_STOP_RET,
    SYS_MSG_ID_ENCODER_SET_STREAM,
    SYS_MSG_ID_ENCODER_SET_STREAM_RET
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
};

typedef enum SYS_MSGQ_ID_TAG
{
    SYS_MSGQ_ID_KBD,
    SYS_MSGQ_ID_FILE,
    SYS_MSGQ_ID_CMD,
    SYS_MSGQ_ID_COUNT
} SYS_MSGQ_ID;

/**
* Queue error codes.
*/
typedef enum QUEUE_ERROR_CODE_TAG
{
    QUEUE_INVALID_INPUT = -5,
    QUEUE_IS_EMPTY      = -4,
    QUEUE_IS_FULL       = -3,
    QUEUE_EXIST         = -2,
    QUEUE_NOT_EXIST     = -1,
    QUEUE_NO_ERROR      = 0
} QUEUE_ERROR_CODE;

typedef QUEUE_ERROR_CODE SYS_MSGQ_ERROR_CODE ;

#define MAX_SYS_MSG_PARAM_COUNT (132)//(8)

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
    MMP_UINT16   msg[MAX_SYS_MSG_PARAM_COUNT];
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

