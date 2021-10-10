﻿/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor main processor message queue.
 *
 * @author Nick Han
 * @version 1.0
 */
#ifndef MAIN_PROCESSOR_MESSAGE_QUEUE_H
#define MAIN_PROCESSOR_MESSAGE_QUEUE_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// cpu id / audio plugin cmd
//    2 bits                14 bits
#define SMTK_MAIN_PROCESSOR_ID   1
#define SMTK_AUDIO_PROCESSOR_ID 2

// define file info
#define SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH 4
#define SMTK_AUDIO_PLUGIN_CMD_FILE_WRITE_LENGTH 4
#define SMTK_AUDIO_PLUGIN_CMD_FILE_OPEN_TYPE 1

#define AUDIO_PLUGIN_MESSAGE_REGISTER    0x16e8

#define getAudioPluginMessageStatus() ithReadRegH(AUDIO_PLUGIN_MESSAGE_REGISTER) 
#define setAudioPluginMessageStatus(value) ithWriteRegH(AUDIO_PLUGIN_MESSAGE_REGISTER,value) 

#define SMTK_AUDIO_PLUGIN_CMD_I2S_BUFFER_ADDRESS_LENGTH 4
#define SMTK_AUDIO_PLUGIN_CMD_I2S_CHANNELS_LENGTH 4
#define SMTK_AUDIO_PLUGIN_CMD_I2S_SAMPLERATE_LENGTH 4
#define SMTK_AUDIO_PLUGIN_CMD_I2S_BUFFER_LENGTH 4
// for Audio_Plugin_MSG_TYPE_CMD
enum
{
    SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN = 0x1,
    SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE,    // 2
    SMTK_AUDIO_PLUGIN_CMD_ID_FILE_READ,   // 3
    SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE,  // 4
    /* IIS */
    SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC,  // 5
    SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_ADC, // 6
    SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_CODEC, // 7
    SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC, // 8
    SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_ADC, // 9
    SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC, // 10
    SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_ADC, // 11
    /* kernel*/
    SMTK_AUDIO_PLUGIN_CMD_ID_SLEEP, // 12
    SMTK_AUDIO_PLUGIN_CMD_ID_GET_CLOCK,   // 13
    SMTK_AUDIO_PLUGIN_CMD_ID_GET_DURATION,   // 14
    SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF,  // 15
};

enum 
{
    SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NO_ERROR = 0,
    SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NOT_SUPPORT_CPU_ID,
    SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_NOT_SUPPORT_CMD_ID,
    SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_FILE_OPEN_FAIL,
    SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_FILE_WRITE_FAIL,    
    SMTK_MAIN_PROCESSOR_MESSAGE_QUEUE_UNKNOW_FAIL
};

/**
 * main processor implement command
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 */
int
smtkMainProcessorExecuteAudioPluginCmd(
    unsigned short nRegisterStatus);

#ifdef __cplusplus
}
#endif

#endif // MAIN_PROCESSOR_MESSAGE_QUEUE_H



