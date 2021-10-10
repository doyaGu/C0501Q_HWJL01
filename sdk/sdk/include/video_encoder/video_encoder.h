#ifndef _VIDEO_ENCODER_H_
#define _VIDEO_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
    #if defined(VIDEO_ENCODER_EXPORTS)
        #define VIDEO_ENCODER_API __declspec(dllexport)
    #else
        #define VIDEO_ENCODER_API __declspec(dllimport)
    #endif
#else
    #define VIDEO_ENCODER_API extern
#endif

#include <malloc.h>
#include "ite/itp.h"

#define MAX_USER_NUM 16
#define MAX_VIDEO_USER_NUM 4

typedef struct VIDEO_SAMPLE_TAG
{
    uint8_t*       addr;
    uint32_t       size;
    uint32_t       timestamp;
    uint8_t        streamId;
    uint8_t        reused_cnt;
	uint8_t        send_flag[MAX_USER_NUM];   
    struct VIDEO_SAMPLE *  next;
} VIDEO_SAMPLE;

typedef struct VIDEO_ENCODE_PARAMETER_TAG
{
    uint32_t  width;
    uint32_t  height;
    uint32_t  frameRate;
    uint32_t  bitRate;
} VIDEO_ENCODE_PARAMETER;

typedef struct JPEG_ENCODE_PARAMETER_TAG
{
    uint8_t*  strmBuf;
    uint32_t  strmBuf_size;
    uint32_t  quality;
    uint32_t  enSize;
} JPEG_ENCODE_PARAMETER;

//=============================================================================
//                              Function Declaration
//=============================================================================

VIDEO_ENCODER_API void
VideoEncoder_Init(
    void);
    //VIDEO_ENCODE_PARAMETER* enPara);
       
VIDEO_ENCODER_API void
VideoEncoder_Open(
    uint8_t streamId);
   
VIDEO_ENCODER_API void
VideoEncoder_Close(
    void);

VIDEO_ENCODER_API void
VideoEncoder_GetSample(
    VIDEO_SAMPLE **pVidSample,
    uint8_t      get_id);

VIDEO_ENCODER_API void
VideoEncoder_SetSreamstate(
    uint8_t stream_id,
    bool state);

VIDEO_ENCODER_API bool
VideoEncoder_GetSreamstate(
    uint8_t stream_id);

VIDEO_ENCODER_API void
VideoEncoder_SetSreamUserNum(
	uint8_t stream_id, 
	bool state);
	
VIDEO_ENCODER_API uint8_t
VideoEncoder_GetSreamUserNum(
	uint8_t stream_id);
	
VIDEO_ENCODER_API void
JPEGEncodeFrame(
    JPEG_ENCODE_PARAMETER* enPara);
      
#endif //_VIDEO_ENCODER_H_   