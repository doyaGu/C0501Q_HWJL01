/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file mmp_video.h
 *
 * @author
 */
 
#ifndef _MMP_ENCODER_H_
#define _MMP_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
    #if defined(ENCODER_EXPORTS)
        #define ENCODER_API __declspec(dllexport)
    #else
        #define ENCODER_API __declspec(dllimport)
    #endif
#else
    #define ENCODER_API extern
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "encoder/encoder_types.h"
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum AVC_ENCODER_QUERY_TYPE_TAG
{
    AVC_ENCODER_PICTURE_TYPE,
    AVC_ENCODER_SLICE_NUM,
    AVC_ENCODER_STREAM_BUF_OVERFLOW,
    AVC_ENCODER_ENGINE_IDLE,
    AVC_ENCODER_SEMAPHORE
} AVC_ENCODER_QUERY_TYPE;

typedef enum AVC_FRAME_RATE_TAG
{
    AVC_FRAME_RATE_UNKNOW = 0,	
    AVC_FRAME_RATE_25HZ, 
    AVC_FRAME_RATE_50HZ,
    AVC_FRAME_RATE_30HZ,
    AVC_FRAME_RATE_60HZ,
    AVC_FRAME_RATE_29_97HZ,
    AVC_FRAME_RATE_59_94HZ,
    AVC_FRAME_RATE_23_97HZ,
    AVC_FRAME_RATE_24HZ,    
    AVC_FRAME_RATE_NULL_9,
    AVC_FRAME_RATE_NULL_10,
    AVC_FRAME_RATE_NULL_11,
    AVC_FRAME_RATE_NULL_12,
    AVC_FRAME_RATE_NULL_13,
    AVC_FRAME_RATE_VESA_30HZ,
    AVC_FRAME_RATE_VESA_60HZ            
} AVC_FRAME_RATE;
   
//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct AVC_ENCODER_TAG
{
    MMP_BOOL   InstCreated;              //[OUT]
    MMP_UINT32 InstanceNum;              //[OUT]
    MMP_UINT32 framecount;               //[OUT]
    
    MMP_UINT8* pWorkBufAddr;             //[OUT]
    MMP_UINT8* pHdrBufAddr[2];           //[OUT]
    MMP_UINT8* pStreamBufAdr[8];         //[OUT]
    MMP_UINT8* pReconBufAddrY[2];        //[OUT]
    MMP_UINT8* pReconBufAddrU[2];        //[OUT]
    MMP_UINT8* pReconBufAddrV[2];        //[OUT]
    MMP_UINT8* pSourceBufAdrY[5];        //[OUT]
    MMP_UINT8* pSourceBufAdrU[5];        //[OUT]
    MMP_UINT8* pSourceBufAdrV[5];        //[OUT]
    MMP_UINT8* pTiledSrcBufAdrY[5];      //[OUT]
    MMP_UINT8* pTiledSrcBufAdrU[5];      //[OUT]
    MMP_UINT8* pTiledSrcBufAdrV[5];      //[OUT]
    MMP_UINT8* pSubImgBufAdr[2];         //[OUT]
    
    MMP_UINT32 ParaSetHdrSize[2];        //[OUT]
    MMP_UINT32 streamBufCount;           //[OUT]
    MMP_UINT32 streamBufSize;            //[OUT]
    MMP_UINT32 sourceBufCount;           //[OUT]    
    MMP_UINT32 streamBufSelect;          //[OUT]
    
    MMP_UINT32 sourceBufSelect;          //[IN]
    
    MMP_UINT32 frameWidth;               //[IN]
    MMP_UINT32 frameHeight;              //[IN]
    MMP_UINT32 framePitchY;              //[OUT]
    MMP_UINT32 frameCropTop;             //[IN]
    MMP_UINT32 frameCropBottom;          //[IN]
    MMP_UINT32 frameCropLeft;            //[IN]
    MMP_UINT32 frameCropRight;           //[IN]
    AVC_FRAME_RATE frameRate;            //[IN]
    MMP_UINT32 EnFrameRate;              //[IN}
    MMP_UINT32 gopSize;                  //[IN]
    MMP_UINT32 bitRate;                  //[IN]
    MMP_UINT32 enableAutoSkip;           //[IN]
    MMP_UINT32 initialDelay;             //[IN]
    MMP_UINT32 chromaQpOffset;           //[IN]
    MMP_UINT32 constrainedIntraPredFlag; //[IN]
    MMP_UINT32 disableDeblk;             //[IN]
    MMP_UINT32 deblkFilterOffsetAlpha;   //[IN]
    MMP_UINT32 deblkFilterOffsetBeta;    //[IN]
    MMP_UINT32 vbvBufferSize;            //[IN]
    MMP_UINT32 intraRefresh;             //[IN]
    MMP_INT32  rcIntraQp;                //[IN]
    MMP_UINT32 userQpMax;                //[IN]
    MMP_UINT32 userGamma;                //[IN]
    MMP_UINT32 RcIntervalMode;           //[IN]
    MMP_UINT32 MbInterval;               //[IN]
    MMP_UINT32 MEUseZeroPmv;             //[IN]
    MMP_UINT32 MESearchRange;            //[IN]
    MMP_UINT32 IntraCostWeight;          //[IN]
    MMP_UINT32 PicQS;                    //[IN]
    MMP_UINT32 forceIPicture;            //[IN]
    MMP_UINT32 skipEncode;               //[IN]
    MMP_BOOL   interlaced_frame;         //[IN]
    MMP_BOOL   bIFrame;                  //[OUT]
    MMP_BOOL   bMP4format;               //[IN]
    MMP_BOOL   bISPOnFly;                //[IN]
} AVC_ENCODER;

//=============================================================================
//                              Function Declaration
//=============================================================================

ENCODER_API MMP_RESULT
mmpAVCEncodeInit(void);

ENCODER_API MMP_RESULT
mmpAVCEncodeOpen(
    AVC_ENCODER* ptEncoder);
    
ENCODER_API MMP_RESULT
mmpAVCEncodeCreateHdr(
    AVC_ENCODER* ptEncoder);
    
ENCODER_API MMP_RESULT
mmpAVCEncodeSetFrameRate(
    AVC_ENCODER* ptEncoder);
    
ENCODER_API MMP_RESULT
mmpAVCEncodeFire(
    AVC_ENCODER* ptEncoder);
    
ENCODER_API MMP_RESULT
mmpAVCEncodeWait(
    MMP_UINT32 timeout);
    
ENCODER_API MMP_RESULT
mmpAVCEncodeClose(
    AVC_ENCODER* ptEncoder);
    
ENCODER_API MMP_RESULT
mmpAVCEncodeGetStream(
    AVC_ENCODER* ptEncoder, 
    MMP_UINT32*  StreamLen,
    MMP_BOOL*    frameEnd
    );
    
ENCODER_API MMP_RESULT
mmpAVCEncodeReset(
    AVC_ENCODER* ptEncoder);
    
ENCODER_API MMP_RESULT
mmpAVCEncodeQuery(
    AVC_ENCODER_QUERY_TYPE queryType,
    MMP_UINT32* pValue);

ENCODER_API MMP_BOOL
mmpAVCEncodeIsIdle(
    void);
        
ENCODER_API MMP_RESULT
mmpAVCEncodeEnableInterrupt(
    ITHIntrHandler  handler);

ENCODER_API MMP_RESULT
mmpAVCEncodeDisableInterrupt(
    void);

ENCODER_API void
mmpAVCEncodeClearInterrupt(
    void);

ENCODER_API void
mmpAVCEncodeSWRest(
    void);
//ENCODER_API MMP_UINT32
//mmpAVCEncodeGetIntStatus(
//    void);

ENCODER_API void
mmpAVCEncodePowerUp(
    void);

ENCODER_API void
mmpAVCEncodePowerDown(
    void);
#ifdef __cplusplus
}
#endif

#endif //_MMP_ENCODER_H_
