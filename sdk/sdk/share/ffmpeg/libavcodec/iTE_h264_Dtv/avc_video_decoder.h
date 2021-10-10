/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file avc_video_decoder.h
 *
 * @author
 */

#ifndef _AVC_VIDEO_DECODER_H_
#define _AVC_VIDEO_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "core_config.h"
#include "info_mgr.h"
#include "ite/itp.h"
#include "libavcodec/avcodec.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct AVC_VIDEO_DECODER_TAG
{
    MMP_UINT8*  pPacketStartAddr;
    MMP_UINT8*  pPacketEndAddr;
    MMP_UINT8*  pBufferStartAddr;
    MMP_UINT8*  pBufferEndAddr;

    MMP_UINT32  timeStamp;
    //MMP_UINT32  mpeg2_flag;

    // sequence_header
    MMP_UINT32  horizontal_size;
    MMP_UINT32  vertical_size;
    MMP_UINT32  aspect_ratio_information;
    MMP_UINT32  frame_rate_code;
    // sequence_extension
    //MMP_UINT32  progressive_sequence;
    // picture_header
    //MMP_UINT32  prev_temporal_reference;
    //MMP_UINT32  curr_temporal_reference;
    //MMP_UINT32  picture_coding_type;
    // picture_display_extension
    //MMP_UINT32  picture_structure;
    //MMP_UINT32  repeat_first_field;
    //MMP_UINT32  top_field_first;
    // picture_coding_extension
    //MMP_UINT32  progressive_frame;
    // user_data - afd_data
    MMP_UINT32  active_format;

    INFO_VIDEO_RATIO currVideoRatio;
    MMP_UINT32  currAfd;
    MMP_BOOL    bUpdateAspectRatio;
    //MMP_BOOL    bFindSequenceHeader;
    MMP_BOOL    bAVsyncReady;
    MMP_BOOL    bHDVideoFormat;
    MMP_BOOL    bReSync;
    //MMP_UINT32  second_field_flag;
    
    // for video segment decode
    MMP_UINT8*  pSegStartAddr;
    MMP_UINT8*  pSegEndAddr;
    MMP_UINT8*  pPESStartAddr;
    MMP_BOOL    bStartSegment;
    MMP_BOOL    bEndSegment;
    MMP_INT32   remainByte;

    MMP_BOOL    bIsNalFormat;
    MMP_UINT32  NALUnitLength;

    // STC,audio engine time offset
    MMP_INT nOffset;

    MMP_UINT32  gPreTimeStamp;    
} AVC_VIDEO_DECODER;

//=============================================================================
//                              Function Declaration
//=============================================================================

void
AVC_Video_Decoder_Release(
    void);

void
AVC_Video_Decoder_Init(
    AVCodecContext *avctx);

MMP_UINT32
AVC_Video_Decoder_Next(
    void* ptVideoSample);

MMP_UINT32
AVC_Video_Decoder_Reload(
    AVCodecContext* avctx);

void
AVC_Video_Decoder_Display(
    AVCodecContext *avctx,
    AVFrame *data,
    int *data_size);

void
AVC_Video_Decoder_DisplayJumpFrame(
    void);

void
AVC_Video_Decoder_Flip(
    VIDEO_FLIP_TYPE videoFlipType);

void
AVC_Video_Decoder_Wait(
    void);

void
AVC_Video_Decoder_Fire(
    void);

void
AVC_Video_Decoder_End(
    void);

void
AVC_Video_Decoder_ReSync(
    MMP_BOOL bEnable);

#ifdef __cplusplus
}
#endif

#endif
