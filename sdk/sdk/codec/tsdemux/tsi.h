/*
 * Copyright (c) 2012 ITE technology Corp. All Rights Reserved.
 */
/** @file tsi.h
 *
 *  @author I-Chun Lai
 *  @version 0.01
 */
#ifndef TSI_H
#define TSI_H

#include "def.h"
#include "share_info.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct RISC_TSI_TAG
{
    CHANNEL_INFO*   ptChanInfo;

    uint8*          pInputTsBuffer;
    uint32          tsBufferSize;
    uint32          tsBufferWriteIdx;
    uint32          tsBufferReadIdx;
} RISC_TSI;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
void
RISC_TsiInit(
    uint            channelId,
    CHANNEL_INFO*   ptChanInfo,
    uint8*          pRiscBaseAddr);

void
RISC_TsiTerminate(
    uint            channelId);

MMP_BOOL
RISC_TsiGetReady(
    uint            channelId,
    uint8**         ppOutBuffer,
    uint32*         pOutSize);

void
RISC_TsiSetFree(
    uint            channelId,
    uint32          size);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef TSI_H
