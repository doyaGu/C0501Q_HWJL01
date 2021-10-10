#ifndef _VIDEO_ENCODER_H_
#define _VIDEO_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
    #if defined(ENCODER_EXPORTS)
        #define VIDEO_ENCODER_API __declspec(dllexport)
    #else
        #define VIDEO_ENCODER_API __declspec(dllimport)
    #endif
#else
    #define VIDEO_ENCODER_API extern
#endif

#include "encoder/mmp_encoder.h"
#include "capture/mmp_capture.h"
#include "vp/mmp_vp.h"
#include "jpg/ite_jpg.h"

#define MAX_USER_NUM 16
#define MAX_VIDEO_USER_NUM 1

typedef struct VIDEO_ENCODE_PARAMETER_TAG
{
    MMP_UINT32  width;
    MMP_UINT32  height;
    MMP_UINT32  frameRate;
    MMP_UINT32  bitRate;
} VIDEO_ENCODE_PARAMETER;

typedef struct VIDEO_HEADER_INFO_TAG
{
    MMP_UINT32  streamIndex;
    MMP_UINT8*  SPSBuf;
    MMP_UINT8*  PPSBuf;
    MMP_UINT32  SPS_Size;
    MMP_UINT32  PPS_Size;
} VIDEO_HEADER_INFO;

typedef struct VIDEO_STREAM_INFO_TAG
{
    MMP_UINT32 streamIndex;
    MMP_UINT32 bufIndex;
    MMP_BOOL   bIFrame;
    MMP_UINT32 dataSize;
    MMP_UINT32 timeStamp;
} VIDEO_STREAM_INFO;

typedef struct JPEG_ENCODE_PARAMETER_TAG
{
    MMP_UINT8*  strmBuf;
    MMP_UINT32  strmBuf_size;
    MMP_UINT32  quality;
    MMP_UINT32  enSize;
} JPEG_ENCODE_PARAMETER;

typedef void (*ENCODE_PARA_CALLBACK)(VIDEO_STREAM_INFO *streamInfo);
//=============================================================================
//                              Function Declaration
//=============================================================================

VIDEO_ENCODER_API void
VideoEncoder_Init(
   void);
   //VIDEO_ENCODE_PARAMETER* enPara);

VIDEO_ENCODER_API void
VideoEncoder_SetStreamBuf(
   MMP_UINT32 stramIndex,
   MMP_UINT32 bufIndex,
   MMP_UINT8* bufAdr);
   
VIDEO_ENCODER_API void
VideoEncoder_GetHdrInfo(
   VIDEO_HEADER_INFO* headInfo);

VIDEO_ENCODER_API void
VideoEncoder_SetStreamBufCallBack(
   ENCODE_PARA_CALLBACK encodeCallback);
       
VIDEO_ENCODER_API void
VideoEncoder_Open(
   MMP_UINT32 stramIndex);
   
VIDEO_ENCODER_API void
VideoEncoder_Close(
   void);

VIDEO_ENCODER_API MMP_BOOL
VideoEncoder_GetSreamstate(
    MMP_UINT8 stream_id);

VIDEO_ENCODER_API MMP_UINT8
VideoEncoder_GetSreamUserNum(
	MMP_UINT8 stream_id);
	
VIDEO_ENCODER_API void
JPEGEncodeFrame(
   JPEG_ENCODE_PARAMETER* enPara);
      
#endif //_VIDEO_ENCODER_H_   