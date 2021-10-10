/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_memory.h
 *
 * @author
 */

#ifndef _ENCODER_MEMORY_H_
#define _ENCODER_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "encoder/encoder_error.h"
#include "encoder/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
//
//          Memory space
//      -------------------           
//     |       Code        |   
//      -------------------                                                                    
//     |     Parameter     |                        Source buffer                              
//      -------------------                             2048                                   
//     |       Temp        |                     -------------------                           
//      -------------------                     |                   |                          
//     |      Work 0       |              1088  |        Y          |                          
//      -------------------                     |                   |                          
//     |      Work 1       |                     -------------------                           
//      -------------------                544  |      Cb Cr        |                          
//     |      Work 2       |                     -------------------                           
//      -------------------                                                                    
//     |                   |                 Reconstruced/Reference buffer                     
//     |      Rec 0        |                                                                   
//     |                   |            (1). the same as source buffer for single instance     
//      -------------------             (2). multiple instance  (1280x720, 720x480, 352x240)   
//     |                   |                                                                   
//     |      Rec 1        |                     -------------------                           
//     |                   |                    |                   |                          
//      -------------------                720  |        Y0         |                          
//     |                   |                    |                   |                          
//     |      Src 0        |                     -------------------                           
//     |                   |               360  |      Cb0 Cr0      |                          
//      -------------------                      -------------------                           
//     |                   |                    |                   |                          
//     |      Src 1        |               480  |        Y1         |                          
//     |                   |                    |                   |                          
//      -------------------                      -------------------                           
//     |                   |               240  |      Cb1 Cr1      |                          
//     |      Src 2        |                     -------------------                           
//     |                   |                    |                   |                          
//      -------------------                240  |        Y2         |                          
//     |                   |                    |                   |                          
//     |    SubImageA      |                     -------------------                           
//     |                   |               120  |      Cb2 Cr2      |                          
//      -------------------                      -------------------                           
//     |                   |                                                                   
//     |    SubImageB      |        
//     |                   |        
//      -------------------         
//     |     Stream 0      |        
//      -------------------         
//     |     Stream 1      |        
//      -------------------         
//     |     Stream 2      | 
//      -------------------  
//     |     PS Header     | 
//      -------------------  
//

#if defined(MULTIPLE_INSTANCES)
   #if defined (DOOR_PHONE)
      #define MAX_INSTANCE_NUM                        (2)                                              
      #define MAX_INST0_FRAME_HEIGHT                  (480)                                        
      #define MAX_INST1_FRAME_HEIGHT                  (240)
      #define MAX_INST2_FRAME_HEIGHT                  (240)
   #else
      #define MAX_INSTANCE_NUM                        (3)
      #if defined(FULL_HD)                           
         #define MAX_INST0_FRAME_HEIGHT               (1088)
      #else                                          
         #define MAX_INST0_FRAME_HEIGHT               (960)
      #endif                                         
      #define MAX_INST1_FRAME_HEIGHT                  (640)
      #define MAX_INST2_FRAME_HEIGHT                  (320)
   #endif   
#else                                            
   #define MAX_INSTANCE_NUM                        (1)
   #if defined(DOOR_PHONE)  
      #if defined(RES_SD)
        #define MAX_INST0_FRAME_HEIGHT             (512)
      #else
        #define MAX_INST0_FRAME_HEIGHT             (960)
      #endif      
   #else
      #define MAX_INST0_FRAME_HEIGHT               (1088)
   #endif
   #define MAX_INST1_FRAME_HEIGHT                  (0)
   #define MAX_INST2_FRAME_HEIGHT                  (0)
#endif                                           

#define MAX_FRAME_WIDTH                            (1920)
#define MAX_STREAM_BUF_NUM                         (4)
#define MAX_SRC_FRM_BUF_NUM                        (4)
#define ENCODER_FIRMWARE_CODE_SIZE                 (224*1024)
#define ENCODER_PARAMETER_BUF_SIZE                 (10*1024)
#define ENCODER_WORK_BUF_SIZE                      (47*1024)
#define ENCODER_TEMP_BUF_SIZE                      (200*1024)
#define SPS_PPS_HEADER_BUF_SIZE                    (10*1024)
#define EM_CAS                                     (8)
#define EM_BANK                                    (2)
#define EM_RAS                                     (4)
#define EM_WIDTH                                   (3)

#define TILED_FRAME_PITCH                          (2048)

#if defined(DOOR_PHONE)
   #if defined(RES_SD)
      #define MAX_LUM_HEIGHT                          (512)
   #else 
      #if defined(MULTIPLE_INSTANCES)
         #define MAX_LUM_HEIGHT                       (480)
      #else
         #define MAX_LUM_HEIGHT                       (960)
      #endif
   #endif
#else
   #define MAX_LUM_HEIGHT                             (1280) //(1088)
#endif

#if defined(DOOR_PHONE)
   #if defined(RES_SD) || defined(MULTIPLE_INSTANCES)
      #define INST_0_LUM_HEIGHT                       (480)
   #else
      #define INST_0_LUM_HEIGHT                       (720)
   #endif
#else
   #if defined(FULL_HD)
      #define INST_0_LUM_HEIGHT                       (1088)
   #else  
      #define INST_0_LUM_HEIGHT                       (720)     
   #endif
#endif

#if defined(DOOR_PHONE)
   #define INST_1_LUM_HEIGHT                          (240)
#else
   #define INST_1_LUM_HEIGHT                          (480)
#endif

#define INST_2_LUM_HEIGHT                             (240)

#if defined(DOOR_PHONE)
   #if defined(RES_SD) || defined(MULTIPLE_INSTANCES)
      #define STREAM_I_BUF_SIZE                    (200*1024)
      #define STREAM_BUF_SIZE                      (200*1024)
   #else
      #define STREAM_I_BUF_SIZE                    (500*1024)
      #define STREAM_BUF_SIZE                      (500*1024)
   #endif
#else
   #define STREAM_I_BUF_SIZE                       (1000*1024)
   #define STREAM_BUF_SIZE                         (1000*1024)
#endif

#if defined(DOOR_PHONE)   
   #if defined(RES_SD) || defined(MULTIPLE_INSTANCES)
      #define INST_0_STREAM_BUF_SIZE               (200*1024)
   #else
      #define INST_0_STREAM_BUF_SIZE               (500*1024)
   #endif
#else
   #if defined(FULL_HD)                           
      #define INST_0_STREAM_BUF_SIZE               (750*1024)
   #else
      #define INST_0_STREAM_BUF_SIZE               (500*1024)
   #endif
#endif

#define INST_1_STREAM_BUF_SIZE                     (250*1024)
#define INST_2_STREAM_BUF_SIZE                     (120*1024)
                                       
#define DEPLOY_BUFFER_PITCH                        (2048)

#if defined(DOOR_PHONE)
    #if defined(MULTIPLE_INSTANCES)
       #define DEPLOY_BUFFER_HEIGHT                   (6100) //(8680)
    #else
       #if defined(RES_SD)
          #define DEPLOY_BUFFER_HEIGHT                (5524)
       #else
          #define DEPLOY_BUFFER_HEIGHT                (10580)
       #endif 
    #endif
#else
   #if defined(MULTIPLE_INSTANCES)
      #if defined(FULL_HD)
         #define DEPLOY_BUFFER_HEIGHT                 (22766)
      #else
         #define DEPLOY_BUFFER_HEIGHT                 (16695)
      #endif   
   #else   
      #define DEPLOY_BUFFER_HEIGHT                    (15606)   
   #endif  
#endif

typedef enum ENCODER_MEM_DEPLOY_TAG
{    
    ENCODER_MEM_DEPLOY_CODE = 0,       
    ENCODER_MEM_DEPLOY_PARA,       
    ENCODER_MEM_DEPLOY_TEMP,     
    ENCODER_MEM_DEPLOY_WORK_0,
#if defined(MULTIPLE_INSTANCES)
    ENCODER_MEM_DEPLOY_WORK_1,
    ENCODER_MEM_DEPLOY_WORK_2,
#endif    
    ENCODER_MEM_DEPLOY_REC_INST0_Y0,
    ENCODER_MEM_DEPLOY_REC_INST0_U0,
    ENCODER_MEM_DEPLOY_REC_INST0_V0,
    ENCODER_MEM_DEPLOY_REC_INST0_Y1,
    ENCODER_MEM_DEPLOY_REC_INST0_U1,
    ENCODER_MEM_DEPLOY_REC_INST0_V1,
#if defined(MULTIPLE_INSTANCES)    
    ENCODER_MEM_DEPLOY_REC_INST1_Y0,
    ENCODER_MEM_DEPLOY_REC_INST1_U0,
    ENCODER_MEM_DEPLOY_REC_INST1_V0,
    ENCODER_MEM_DEPLOY_REC_INST1_Y1,
    ENCODER_MEM_DEPLOY_REC_INST1_U1,
    ENCODER_MEM_DEPLOY_REC_INST1_V1,
#if !defined (DOOR_PHONE)
    ENCODER_MEM_DEPLOY_REC_INST2_Y0,
    ENCODER_MEM_DEPLOY_REC_INST2_U0,
    ENCODER_MEM_DEPLOY_REC_INST2_V0,
    ENCODER_MEM_DEPLOY_REC_INST2_Y1,
    ENCODER_MEM_DEPLOY_REC_INST2_U1,
    ENCODER_MEM_DEPLOY_REC_INST2_V1,
#endif    
#endif    
    ENCODER_MEM_DEPLOY_SRC_Y0,     
    ENCODER_MEM_DEPLOY_SRC_U0,     
    ENCODER_MEM_DEPLOY_SRC_V0,     
    ENCODER_MEM_DEPLOY_SRC_Y1,     
    ENCODER_MEM_DEPLOY_SRC_U1,     
    ENCODER_MEM_DEPLOY_SRC_V1,     
    ENCODER_MEM_DEPLOY_SRC_Y2,     
    ENCODER_MEM_DEPLOY_SRC_U2,     
    ENCODER_MEM_DEPLOY_SRC_V2,
#if defined(FULL_HD)
    ENCODER_MEM_DEPLOY_SRC_Y3,     
    ENCODER_MEM_DEPLOY_SRC_U3,     
    ENCODER_MEM_DEPLOY_SRC_V3,     
    ENCODER_MEM_DEPLOY_SRC_Y4,     
    ENCODER_MEM_DEPLOY_SRC_U4,     
    ENCODER_MEM_DEPLOY_SRC_V4,     
#endif    
    ENCODER_MEM_DEPLOY_SUB_INST0_0,
    ENCODER_MEM_DEPLOY_SUB_INST0_1,
#if defined(MULTIPLE_INSTANCES)
    ENCODER_MEM_DEPLOY_SUB_INST1_0,
    ENCODER_MEM_DEPLOY_SUB_INST1_1,
#if !defined (DOOR_PHONE)    
    ENCODER_MEM_DEPLOY_SUB_INST2_0,
    ENCODER_MEM_DEPLOY_SUB_INST2_1,
#endif    
#endif    

#if defined(DOOR_PHONE) && defined(MULTIPLE_INSTANCES)
    ENCODER_MEM_DEPLOY_STREAM_INST0_0,  
    ENCODER_MEM_DEPLOY_STREAM_INST0_1,
    ENCODER_MEM_DEPLOY_STREAM_INST0_2,
    ENCODER_MEM_DEPLOY_STREAM_INST0_3,
    ENCODER_MEM_DEPLOY_STREAM_INST0_4,
    ENCODER_MEM_DEPLOY_STREAM_INST0_5,
    ENCODER_MEM_DEPLOY_STREAM_INST0_6,
    ENCODER_MEM_DEPLOY_STREAM_INST1_0,  
    ENCODER_MEM_DEPLOY_STREAM_INST1_1,
    ENCODER_MEM_DEPLOY_STREAM_INST1_2,
    ENCODER_MEM_DEPLOY_STREAM_INST1_3,
    ENCODER_MEM_DEPLOY_STREAM_INST1_4,
    ENCODER_MEM_DEPLOY_STREAM_INST1_5,
    ENCODER_MEM_DEPLOY_STREAM_INST1_6,
#else
    ENCODER_MEM_DEPLOY_STREAM_INST0_0,  
    ENCODER_MEM_DEPLOY_STREAM_INST0_1,
    ENCODER_MEM_DEPLOY_STREAM_INST0_2,    
#if defined(MULTIPLE_INSTANCES) 
    ENCODER_MEM_DEPLOY_STREAM_INST1_0,  
    ENCODER_MEM_DEPLOY_STREAM_INST1_1,
    ENCODER_MEM_DEPLOY_STREAM_INST1_2,    
    ENCODER_MEM_DEPLOY_STREAM_INST2_0,  
    ENCODER_MEM_DEPLOY_STREAM_INST2_1,
    ENCODER_MEM_DEPLOY_STREAM_INST2_2,   
#else
    ENCODER_MEM_DEPLOY_STREAM_INST0_3,
    ENCODER_MEM_DEPLOY_STREAM_INST0_4,
    ENCODER_MEM_DEPLOY_STREAM_INST0_5,
    ENCODER_MEM_DEPLOY_STREAM_INST0_6,
#endif

#endif  
    ENCODER_MEM_DEPLOY_SPS_PPS_0,
#if defined(MULTIPLE_INSTANCES)
    ENCODER_MEM_DEPLOY_SPS_PPS_1,
#if !defined (DOOR_PHONE)    
    ENCODER_MEM_DEPLOY_SPS_PPS_2,
#endif    
#endif    
    ENCODER_MEM_DEPLOY_COUNT
} ENCODER_MEM_DEPLOY;

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct VIDEO_FRAME_BUFFER_TAG
{
    MMP_UINT8* pAddress_Y;
    MMP_UINT8* pAddress_U;
    MMP_UINT8* pAddress_V;
} VIDEO_FRAME_BUFFER;

//=============================================================================
//                              Function Declaration
//=============================================================================

VIDEO_ERROR_CODE
encoder_memory_Initialize(
    void);
    
VIDEO_ERROR_CODE
encoder_memory_GetCodeBuffer(
    MMP_UINT8** ppBuf);
    
VIDEO_ERROR_CODE
encoder_memory_GetParaBuffer(
    MMP_UINT8** ppBuf);
    
VIDEO_ERROR_CODE
encoder_memory_GetTempBuffer(
    MMP_UINT8** ppBuf);
    
VIDEO_ERROR_CODE
encoder_memory_GetWorkBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 instNum);
    
VIDEO_ERROR_CODE
encoder_memory_GetStreamBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 index,
    MMP_UINT32 instNum);    
	    
VIDEO_ERROR_CODE
encoder_memory_GetReconBuffer(
    VIDEO_FRAME_BUFFER* ptFrameBuf,
    MMP_UINT32 index,
    MMP_UINT32 instNum);
    
VIDEO_ERROR_CODE
encoder_memory_GetSourceBuffer(
    VIDEO_FRAME_BUFFER* ptFrameBuf,
    VIDEO_FRAME_BUFFER* ptTiledFrmBuf,
    MMP_UINT32 index);
    
VIDEO_ERROR_CODE
encoder_memory_GetSubImgBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 index,
    MMP_UINT32 instNum);
    
VIDEO_ERROR_CODE
encoder_memory_GetPSBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 instNum);
                                        
VIDEO_ERROR_CODE
encoder_memory_CreateBuffer(
    MMP_UINT8** ppBuf, 
    MMP_UINT32 bufSize);

VIDEO_ERROR_CODE
encoder_memory_FreeBuffer(
    MMP_UINT8* pBuf);

MMP_UINT32
encoder_memory_GetTiledBaseAddr(void);

#ifdef __cplusplus
}
#endif

#endif //_ENCODER_MEMORY_H_
