/*
 * Copyright (c) 2012 ITE technology Corp. All Rights Reserved.
 */
/** @file tsi.c
 *
 *  @author I-Chun Lai
 *  @version 0.01
 */

#include "tsi.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

#if 0
typedef struct RISC_TSI_TAG
{
    CHANNEL_INFO*   ptChanInfo;

    uint8*          pInputTsBuffer;
    uint32          tsBufferSize;
    uint32          tsBufferWriteIdx;
    uint32          tsBufferReadIdx;
} RISC_TSI;
#endif

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//static RISC_TSI             gtRiscTsi[MAX_CHANNEL_COUNT];
RISC_TSI             gtRiscTsi[MAX_CHANNEL_COUNT];

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
RISC_TsiInit(
    uint            channelId,
    CHANNEL_INFO*   ptChanInfo,
    uint8*          pRiscBaseAddr)
{
    if (channelId <  MAX_CHANNEL_COUNT
     && MMP_NULL != ptChanInfo)
    {
        RISC_TSI* ptTsi = &gtRiscTsi[channelId];

        PalMemset(ptTsi, 0, sizeof(RISC_TSI));
        // do error check?
        ptTsi->ptChanInfo       = ptChanInfo;
        ptTsi->pInputTsBuffer   = (uint8*)(GET32((uint)ptChanInfo->pInputTsBuffer)
                                - (uint)pRiscBaseAddr);
        ptTsi->tsBufferSize     = GET32(ptChanInfo->tsBufferSize);
        ptTsi->tsBufferWriteIdx = GET32(ptChanInfo->tsBufferWriteIdx);
        ptTsi->tsBufferReadIdx  = GET32(ptChanInfo->tsBufferReadIdx);
    }
}

void
RISC_TsiTerminate(
    uint            channelId)
{
}

MMP_BOOL
RISC_TsiGetReady(
    uint            channelId,
    uint8**         ppOutBuffer,
    uint32*         pOutSize)
{
    MMP_BOOL result = MMP_FALSE;

    if (channelId < MAX_CHANNEL_COUNT
     && ppOutBuffer
     && pOutSize)
    {
        RISC_TSI* ptTsi = &gtRiscTsi[channelId];

        *ppOutBuffer    = MMP_NULL;
        *pOutSize       = 0;

        dc_invalidate();

        ptTsi->tsBufferWriteIdx = GET32(ptTsi->ptChanInfo->tsBufferWriteIdx);
        if (ptTsi->tsBufferWriteIdx != ptTsi->tsBufferReadIdx)
        {
            *ppOutBuffer = ptTsi->pInputTsBuffer + ptTsi->tsBufferReadIdx;
            *pOutSize    = (ptTsi->tsBufferWriteIdx > ptTsi->tsBufferReadIdx)
                         ? (ptTsi->tsBufferWriteIdx - ptTsi->tsBufferReadIdx)
                         : (ptTsi->tsBufferSize     - ptTsi->tsBufferReadIdx);
            result = MMP_TRUE;
        }
    }
    return result;
}

void
RISC_TsiSetFree(
    uint            channelId,
    uint32          size)
{
    if (channelId < MAX_CHANNEL_COUNT
     && 0 < size)
    {
        RISC_TSI* ptTsi = &gtRiscTsi[channelId];
        // do error check?
        ptTsi->tsBufferReadIdx += size;
        if (ptTsi->tsBufferReadIdx >= ptTsi->tsBufferSize)
            ptTsi->tsBufferReadIdx -= ptTsi->tsBufferSize;
        SET32(ptTsi->ptChanInfo->tsBufferReadIdx, ptTsi->tsBufferReadIdx);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
