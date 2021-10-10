/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_bitstream.h
 *
 * @author
 */

#ifndef _ENCODER_BITSTREAM_H_
#define _ENCODER_BITSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "mmp_encoder.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
	MMP_UINT32 bufa;
	MMP_UINT32 bufb;
	MMP_UINT32 buf;
	MMP_UINT32 pos;
	MMP_UINT32 *tail;
	MMP_UINT32 *start;
	MMP_UINT32 length;
} BIT_STREAM;

//=============================================================================
//                              Function Declaration
//=============================================================================

void encoder_CreateIFrameHdr(AVC_ENCODER* ptEncoder,
                           MMP_UINT32 *bitstream,
                           MMP_UINT32 dwStreamSize);
                           
                             
void encoder_CreatePFrameHdr(AVC_ENCODER* ptEncoder,
                             MMP_UINT32 *bitstream,
                             MMP_UINT32 dwStreamSize);
                             
void encoder_CreateMP4Config(AVC_ENCODER* ptEncoder,
                             MMP_UINT32 *bitstream,
                             MMP_UINT32 dwStreamSize);

void encoder_CreateMP4SEI(AVC_ENCODER* ptEncoder,
                          MMP_UINT32 *bitstream,
                          MMP_UINT32 dwStreamSize);
                          
void encoder_CreateSPS(AVC_ENCODER* ptEncoder,
                       MMP_UINT32 *bitstream,
                       MMP_UINT32 dwStreamSize);

void encoder_CreatePPS(AVC_ENCODER* ptEncoder,
                       MMP_UINT32 *bitstream,
                       MMP_UINT32 dwStreamSize);                                              
                                                                                    
#ifdef __cplusplus
}
#endif

#endif //_ENCODER_BITSTREAM_H_
