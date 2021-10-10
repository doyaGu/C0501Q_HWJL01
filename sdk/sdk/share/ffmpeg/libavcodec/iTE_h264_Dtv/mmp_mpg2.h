/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file mmp_mpg2.h
 *
 * @author Vincent Lee
 */

#ifndef _MMP_MPG2_H_
#define _MMP_MPG2_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
    #if defined(MPG2_EXPORTS)
        #define MPG2_API extern
    #else
        #define MPG2_API extern
    #endif
#else
    #define MPG2_API extern
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "ite/mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef enum MPG2_QUERY_TYPE_TAG
{
    MPG2_STATUS_FRAME_BUF,
    MPG2_STATUS_T_FIELD_BUF,
    MPG2_STATUS_B_FIELD_BUF,
    MPG2_STATUS_FRAME_BUF_ERROR,
    MPG2_STATUS_T_FIELD_BUF_ERROR,
    MPG2_STATUS_B_FIELD_BUF_ERROR,
    MPG2_ADDRESS_FRAME_BUF_Y,
    MPG2_ADDRESS_FRAME_BUF_U,
    MPG2_ADDRESS_FRAME_BUF_V,
    MPG2_SEMAPHORE
} MPG2_QUERY_TYPE;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct MPG2_DECODER_TAG
{
    void*       ppCmdDataBufAddr[2];
    MMP_UINT32  cmdDataBufSize;
    MMP_UINT32  cmdDataBufSelect;
    MMP_UINT32  decodeBufSelect;

    // picture_display_extension
    MMP_UINT32  picture_structure;

    MMP_UINT32  framePitchY;
    MMP_UINT32  framePitchUV;
    MMP_UINT32  frameWidth;
    MMP_UINT32  frameHeight;
    MMP_UINT32  clipStartX;
    MMP_UINT32  clipStartY;
    MMP_UINT32  clipWidth;
    MMP_UINT32  clipHeight;
    MMP_UINT32  frameBufCount;
    MMP_UINT32  switchHardwareScan; // 0: disable; !0: enable

    MMP_UINT32  currDisplayFrameBufIndex;
    MMP_UINT32  prevDisplayFrameBufIndex;
    MMP_UINT16  enableKeepLastField;
    MMP_UINT16  isProgressive;

    // only used in A0
    MMP_UINT32  top_field_first;
} MPG2_DECODER;

//=============================================================================
//                              Function Declaration
//=============================================================================

MPG2_API MMP_RESULT
mmpMpg2DecodeInit(
    MPG2_DECODER* ptDecoder);

MPG2_API MMP_RESULT
mmpMpg2DecodeFire(
    MPG2_DECODER* ptDecoder);

MPG2_API MMP_RESULT
mmpMpg2DecodeWait(
    MMP_UINT32 timeout);

MPG2_API MMP_RESULT
mmpMpg2DecodeEnd(
    MPG2_DECODER* ptDecoder);

MPG2_API MMP_RESULT
mmpMpg2DecodeDisplay(
    MPG2_DECODER* ptDecoder);

MPG2_API MMP_RESULT
mmpMpg2DecodeReset(
    MPG2_DECODER* ptDecoder);

MPG2_API MMP_RESULT
mmpMpg2DecodeQuery(
    MPG2_QUERY_TYPE queryType,
    MMP_UINT32 value,
    MMP_UINT32* pValue);

//MPG2_API MMP_RESULT
//dbgMpg2_QueryRegisterValue(
//    MMP_UINT32 registerAddr,
//    MMP_UINT32* pValue);

//MPG2_API MMP_RESULT
//dbgMpg2_QueryRegisterAddress(
//    MMP_UINT32 regAddrLo,
//    MMP_UINT32 regAddrHi,
//    MMP_UINT32* pValue);

//MPG2_API MMP_RESULT
//dbgMpg2_QueryFrameBufferAddress(
//    MMP_UINT32 plane,
//    MMP_UINT32 bufferIndex,
//    MMP_UINT32* pValue);

#ifdef __cplusplus
}
#endif

#endif
