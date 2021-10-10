/*
 * Copyright (c) 2012 ITE technology Corp. All Rights Reserved.
 */
/** @file share_info.h
 *
 *  @author I-Chun Lai
 *  @version 0.01
 */
#ifndef SHARE_INFO_H
#define SHARE_INFO_H

#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_CHANNEL_COUNT           (16)
#define MAX_PID_COUNT_PER_CHANNEL   (8)
#define MAX_PES_SAMPLE_COUNT        (32)

#define CPU_ID_ARM                  (1)
#define CPU_ID_RISC                 (2)

#define XCPU_CMD_REG                (0x16A2)

typedef enum XCPU_COMMAND_TAG
{
    XCPU_COMMAND_NULL       = 0,
    XCPU_COMMAND_CFG_CHANGE,
    //XCPU_COMMAND_RUN,
    //XCPU_COMMAND_STOP,
    XCPU_COMMAND_MAX
} XCPU_COMMAND;

typedef enum XCPU_RETURN_CODE_TAG
{
    XCPU_RETURN_CODE_NULL   = 0,
    XCPU_RETURN_CODE_DONE,
    XCPU_RETURN_CODE_MAX
} XCPU_RETURN_CODE;

//=============================================================================
//                              Macro Definition
//=============================================================================
#define ENDIAN_LITTLE       (1)
#define ENDIAN_BIG          (2)
#define ENDIAN              ENDIAN_BIG  // 1: little, 2: big

#if (ENDIAN == ENDIAN_BIG)
#define SET32(var,val)  {(var) = (((val) & 0x000000FFL) << 24)  \
                               + (((val) & 0x0000FF00L) <<  8)  \
                               + (((val) & 0x00FF0000L) >>  8)  \
                               + (((val) & 0xFF000000L) >> 24);}
#define GET32(v)         ((((v) & 0x000000FFL) << 24)  \
                        + (((v) & 0x0000FF00L) <<  8)  \
                        + (((v) & 0x00FF0000L) >>  8)  \
                        + (((v) & 0xFF000000L) >> 24))
#else
#define SET32(var,val)  {(var) = (val);}
#define GET32(v)        (v)
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct PID_INFO_TAG
{
    uint32          pid;
    uint8*          pOutPesBuffer;
    uint32          pOutPesSampleSize;
    uint32          validPesSampleCount;
    uint32          pOutputPesValidSize[MAX_PES_SAMPLE_COUNT];
    uint32          pesBufferWriteIdx;
    uint32          pesBufferReadIdx;
} PID_INFO;

typedef struct CHANNEL_INFO_TAG
{
    uint32          bBlockMode;
    uint32          bValidChannel;
    uint8*          pInputTsBuffer;
    uint32          tsBufferSize;
    uint32          tsBufferWriteIdx;
    uint32          tsBufferReadIdx;

    uint32          validPidCount;
    PID_INFO        tPidInfo[MAX_PID_COUNT_PER_CHANNEL];   // max 4 video, 4 audio
} CHANNEL_INFO;

typedef struct CHANNLE_INFO_ARRAY_TAG
{
    uint8*          pRiscBaseAddr;
    CHANNEL_INFO    tChannelArray[MAX_CHANNEL_COUNT];
} CHANNEL_INFO_ARRAY;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef SHARE_INFO_H
