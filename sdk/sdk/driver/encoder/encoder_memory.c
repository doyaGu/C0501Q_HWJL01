/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_memory.c
 *
 * @author
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdlib.h>
#include "encoder/encoder_types.h"
#include "encoder/config.h"
#include "encoder/encoder_memory.h" 

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static MMP_UINT32 EndeployBuffer[ENCODER_MEM_DEPLOY_COUNT] = { 0 };
static MMP_UINT16 EndeployTable[ENCODER_MEM_DEPLOY_COUNT][2] = {
    // x, y
#if !defined(MULTIPLE_INSTANCES)
#if defined (DOOR_PHONE)
  #if defined (RES_SD)
      {    0,                  0}, // ENCODER_MEM_DEPLOY_CODE
      {    0,                224}, // ENCODER_MEM_DEPLOY_PARA
      {    0,                240}, // ENCODER_MEM_DEPLOY_TEMP
      {    0,                440}, // ENCODER_MEM_DEPLOY_WORK_0
      {    0,                640}, // ENCODER_MEM_DEPLOY_REC_INST0_Y0
      {    0,          512 + 640}, // ENCODER_MEM_DEPLOY_REC_INST0_U0
      {    0,          768 + 640}, // ENCODER_MEM_DEPLOY_REC_INST0_V0
      {    0,               1408}, // ENCODER_MEM_DEPLOY_REC_INST0_Y1
      {    0,         512 + 1408}, // ENCODER_MEM_DEPLOY_REC_INST0_U1
      {    0,         768 + 1408}, // ENCODER_MEM_DEPLOY_REC_INST0_V1
      {    0,               2176}, // ENCODER_MEM_DEPLOY_SRC_Y0
      {    0,         512 + 2176}, // ENCODER_MEM_DEPLOY_SRC_U0
      {    0,         768 + 2176}, // ENCODER_MEM_DEPLOY_SRC_V0
      {    0,               2944}, // ENCODER_MEM_DEPLOY_SRC_Y1
      {    0,         512 + 2944}, // ENCODER_MEM_DEPLOY_SRC_U1
      {    0,         768 + 2944}, // ENCODER_MEM_DEPLOY_SRC_V1
      {    0,               3712}, // ENCODER_MEM_DEPLOY_SRC_Y2
      {    0,         512 + 3712}, // ENCODER_MEM_DEPLOY_SRC_U2
      {    0,         768 + 3712}, // ENCODER_MEM_DEPLOY_SRC_V2
      {    0,               4480}, // ENCODER_MEM_DEPLOY_SUB_INST0_0
      {    0,         512 + 4480}, // ENCODER_MEM_DEPLOY_SUB_INST0_1
      {    0,               5504}, // ENCODER_MEM_DEPLOY_STREAM_0
      {    0,               5504}, // ENCODER_MEM_DEPLOY_STREAM_1
      {    0,               5504}, // ENCODER_MEM_DEPLOY_STREAM_2  
      {    0,               5504}, // ENCODER_MEM_DEPLOY_STREAM_3
      {    0,               5504}, // ENCODER_MEM_DEPLOY_STREAM_4
      {    0,               5504}, // ENCODER_MEM_DEPLOY_STREAM_5
      {    0,               5504}, // ENCODER_MEM_DEPLOY_STREAM_6
      {    0,               5504}, // ENCODER_MEM_DEPLOY_SPS_PPS_0   // Height : 5524  
  #else
      {    0,                  0}, // ENCODER_MEM_DEPLOY_CODE
      {    0,                224}, // ENCODER_MEM_DEPLOY_PARA
      {    0,                240}, // ENCODER_MEM_DEPLOY_TEMP
      {    0,                440}, // ENCODER_MEM_DEPLOY_WORK_0
      {    0,                640}, // ENCODER_MEM_DEPLOY_REC_INST0_Y0
      {    0,          960 + 640}, // ENCODER_MEM_DEPLOY_REC_INST0_U0
      {    0,         1600 + 640}, // ENCODER_MEM_DEPLOY_REC_INST0_V0
      {    0,               2240}, // ENCODER_MEM_DEPLOY_REC_INST0_Y1
      {    0,         960 + 2240}, // ENCODER_MEM_DEPLOY_REC_INST0_U1
      {    0,        1600 + 2240}, // ENCODER_MEM_DEPLOY_REC_INST0_V1
      {    0,               3840}, // ENCODER_MEM_DEPLOY_SRC_Y0
      {    0,         960 + 3840}, // ENCODER_MEM_DEPLOY_SRC_U0
      {    0,        1600 + 3840}, // ENCODER_MEM_DEPLOY_SRC_V0
      {    0,               5440}, // ENCODER_MEM_DEPLOY_SRC_Y1
      {    0,         960 + 5440}, // ENCODER_MEM_DEPLOY_SRC_U1
      {    0,        1600 + 5440}, // ENCODER_MEM_DEPLOY_SRC_V1
      {    0,               7040}, // ENCODER_MEM_DEPLOY_SRC_Y2
      {    0,         960 + 7040}, // ENCODER_MEM_DEPLOY_SRC_U2
      {    0,        1600 + 7040}, // ENCODER_MEM_DEPLOY_SRC_V2
      {    0,               8640}, // ENCODER_MEM_DEPLOY_SUB_INST0_0
      {    0,         960 + 8640}, // ENCODER_MEM_DEPLOY_SUB_INST0_1
      {    0,              10560}, // ENCODER_MEM_DEPLOY_STREAM_0
      {    0,              10560}, // ENCODER_MEM_DEPLOY_STREAM_1
      {    0,              10560}, // ENCODER_MEM_DEPLOY_STREAM_2  
      {    0,              10560}, // ENCODER_MEM_DEPLOY_STREAM_3
      {    0,              10560}, // ENCODER_MEM_DEPLOY_STREAM_4
      {    0,              10560}, // ENCODER_MEM_DEPLOY_STREAM_5
      {    0,              10560}, // ENCODER_MEM_DEPLOY_STREAM_6
      {    0,              10560}, // ENCODER_MEM_DEPLOY_SPS_PPS_0   // Height : 10580  
  #endif          
#else    
    {    0,                  0}, // ENCODER_MEM_DEPLOY_CODE
    {    0,                112}, // ENCODER_MEM_DEPLOY_PARA
    {    0,                120}, // ENCODER_MEM_DEPLOY_TEMP
    {    0,                220}, // ENCODER_MEM_DEPLOY_WORK_0
    {    0,                320}, // ENCODER_MEM_DEPLOY_REC_INST0_Y0
    {    0,         1280 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_U0
    {    0,         1920 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_V0
    {    0,               2240}, // ENCODER_MEM_DEPLOY_REC_INST0_Y1
    {    0,        1280 + 2240}, // ENCODER_MEM_DEPLOY_REC_INST0_U1
    {    0,        1920 + 2240}, // ENCODER_MEM_DEPLOY_REC_INST0_V1
    {    0,               4160}, // ENCODER_MEM_DEPLOY_SRC_Y0
    {    0,        1280 + 4160}, // ENCODER_MEM_DEPLOY_SRC_U0
    {    0,        1920 + 4160}, // ENCODER_MEM_DEPLOY_SRC_V0
    {    0,               6080}, // ENCODER_MEM_DEPLOY_SRC_Y1
    {    0,        1280 + 6080}, // ENCODER_MEM_DEPLOY_SRC_U1
    {    0,        1920 + 6080}, // ENCODER_MEM_DEPLOY_SRC_V1
    {    0,               8000}, // ENCODER_MEM_DEPLOY_SRC_Y2
    {    0,        1280 + 8000}, // ENCODER_MEM_DEPLOY_SRC_U2
    {    0,        1920 + 8000}, // ENCODER_MEM_DEPLOY_SRC_V2
    {    0,               9920}, // ENCODER_MEM_DEPLOY_SUB_INST0_0
    {    0,        1088 + 9920}, // ENCODER_MEM_DEPLOY_SUB_INST0_1
    {    0,              12096}, // ENCODER_MEM_DEPLOY_STREAM_0
    {    0,        500 + 12096}, // ENCODER_MEM_DEPLOY_STREAM_1
    {    0,       1000 + 12096}, // ENCODER_MEM_DEPLOY_STREAM_2  
    {    0,       1500 + 12096}, // ENCODER_MEM_DEPLOY_STREAM_3
    {    0,       2000 + 12096}, // ENCODER_MEM_DEPLOY_STREAM_4
    {    0,       2500 + 12096}, // ENCODER_MEM_DEPLOY_STREAM_5
    {    0,       3000 + 12096}, // ENCODER_MEM_DEPLOY_STREAM_6
    {    0,              15596}, // ENCODER_MEM_DEPLOY_SPS_PPS_0   // Height : 15606
#endif    
#else
#if defined (DOOR_PHONE)    
    {    0,                  0}, // ENCODER_MEM_DEPLOY_CODE
    {    0,                112}, // ENCODER_MEM_DEPLOY_PARA
    {    0,                120}, // ENCODER_MEM_DEPLOY_TEMP
    {    0,                220}, // ENCODER_MEM_DEPLOY_WORK_0
    {    0,                244}, // ENCODER_MEM_DEPLOY_WORK_1
    {    0,                268}, // ENCODER_MEM_DEPLOY_WORK_2
    {    0,                320}, // ENCODER_MEM_DEPLOY_REC_INST0_Y0
    {    0,          480 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_U0
    {    0,          720 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_V0
    {    0,               1040}, // ENCODER_MEM_DEPLOY_REC_INST0_Y1
    {    0,         480 + 1040}, // ENCODER_MEM_DEPLOY_REC_INST0_U1
    {    0,         720 + 1040}, // ENCODER_MEM_DEPLOY_REC_INST0_V1
    {    0,               1760}, // ENCODER_MEM_DEPLOY_REC_INST1_Y0
    {    0,         240 + 1760}, // ENCODER_MEM_DEPLOY_REC_INST1_U0
    {    0,         360 + 1760}, // ENCODER_MEM_DEPLOY_REC_INST1_V0
    {    0,               2120}, // ENCODER_MEM_DEPLOY_REC_INST1_Y1
    {    0,         240 + 2120}, // ENCODER_MEM_DEPLOY_REC_INST1_U1
    {    0,         360 + 2120}, // ENCODER_MEM_DEPLOY_REC_INST1_V1
    {    0,               2480}, // ENCODER_MEM_DEPLOY_SRC_Y0
    {    0,         480 + 2480}, // ENCODER_MEM_DEPLOY_SRC_U0
    {    0,         720 + 2480}, // ENCODER_MEM_DEPLOY_SRC_V0
    {    0,               3200}, // ENCODER_MEM_DEPLOY_SRC_Y1
    {    0,         480 + 3200}, // ENCODER_MEM_DEPLOY_SRC_U1
    {    0,         720 + 3200}, // ENCODER_MEM_DEPLOY_SRC_V1    
    {    0,               3920}, // ENCODER_MEM_DEPLOY_SRC_Y2
    {    0,         480 + 3920}, // ENCODER_MEM_DEPLOY_SRC_U2
    {    0,         720 + 3920}, // ENCODER_MEM_DEPLOY_SRC_V2    
    {    0,               4640}, // ENCODER_MEM_DEPLOY_SUB_INST0_0
    {    0,         480 + 4640}, // ENCODER_MEM_DEPLOY_SUB_INST0_1
    {    0,               5600}, // ENCODER_MEM_DEPLOY_SUB_INST1_0
    {    0,         240 + 5600}, // ENCODER_MEM_DEPLOY_SUB_INST1_1
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST0_0
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST0_1
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST0_2    
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST0_3
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST0_4
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST0_5
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST0_6    
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST1_0
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST1_1
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST1_2
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST1_3
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST1_4
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST1_5
    {    0,               6080}, // ENCODER_MEM_DEPLOY_STREAM_INST1_6
    {    0,               6080}, // ENCODER_MEM_DEPLOY_SPS_PPS_0
    {    0,          10 + 6080}, // ENCODER_MEM_DEPLOY_SPS_PPS_1   // Height : 6100
#else
#if !defined(FULL_HD)
    {    0,                  0}, // ENCODER_MEM_DEPLOY_CODE
    {    0,                112}, // ENCODER_MEM_DEPLOY_PARA
    {    0,                120}, // ENCODER_MEM_DEPLOY_TEMP
    {    0,                220}, // ENCODER_MEM_DEPLOY_WORK_0
    {    0,                244}, // ENCODER_MEM_DEPLOY_WORK_1
    {    0,                268}, // ENCODER_MEM_DEPLOY_WORK_2
    {    0,                320}, // ENCODER_MEM_DEPLOY_REC_INST0_Y0
    {    0,          960 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_U0
    {    0,         1600 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_V0
    {    0,               1920}, // ENCODER_MEM_DEPLOY_REC_INST0_Y1
    {    0,         960 + 1920}, // ENCODER_MEM_DEPLOY_REC_INST0_U1
    {    0,        1600 + 1920}, // ENCODER_MEM_DEPLOY_REC_INST0_V1
    {    0,               3520}, // ENCODER_MEM_DEPLOY_REC_INST1_Y0
    {    0,         640 + 3520}, // ENCODER_MEM_DEPLOY_REC_INST1_U0
    {    0,         960 + 3520}, // ENCODER_MEM_DEPLOY_REC_INST1_V0
    {    0,               4480}, // ENCODER_MEM_DEPLOY_REC_INST1_Y1
    {    0,         640 + 4480}, // ENCODER_MEM_DEPLOY_REC_INST1_U1
    {    0,         960 + 4480}, // ENCODER_MEM_DEPLOY_REC_INST1_V1
    {    0,               5440}, // ENCODER_MEM_DEPLOY_REC_INST2_Y0
    {    0,         320 + 5440}, // ENCODER_MEM_DEPLOY_REC_INST2_U0
    {    0,         640 + 5440}, // ENCODER_MEM_DEPLOY_REC_INST2_V0
    {    0,               6080}, // ENCODER_MEM_DEPLOY_REC_INST2_Y1
    {    0,         320 + 6080}, // ENCODER_MEM_DEPLOY_REC_INST2_U1
    {    0,         640 + 6080}, // ENCODER_MEM_DEPLOY_REC_INST2_V1
    {    0,               6720}, // ENCODER_MEM_DEPLOY_SRC_Y0
    {    0,         960 + 6720}, // ENCODER_MEM_DEPLOY_SRC_U0
    {    0,        1600 + 6720}, // ENCODER_MEM_DEPLOY_SRC_V0
    {    0,               8320}, // ENCODER_MEM_DEPLOY_SRC_Y1
    {    0,         960 + 8320}, // ENCODER_MEM_DEPLOY_SRC_U1
    {    0,        1600 + 8320}, // ENCODER_MEM_DEPLOY_SRC_V1
    {    0,               9920}, // ENCODER_MEM_DEPLOY_SRC_Y2
    {    0,         960 + 9920}, // ENCODER_MEM_DEPLOY_SRC_U2
    {    0,        1600 + 9920}, // ENCODER_MEM_DEPLOY_SRC_V2
    {    0,              11520}, // ENCODER_MEM_DEPLOY_SUB_INST0_0
    {    0,        960 + 11520}, // ENCODER_MEM_DEPLOY_SUB_INST0_1
    {    0,              13440}, // ENCODER_MEM_DEPLOY_SUB_INST1_0
    {    0,        640 + 13440}, // ENCODER_MEM_DEPLOY_SUB_INST1_1
    {    0,              14720}, // ENCODER_MEM_DEPLOY_SUB_INST2_0
    {    0,        320 + 14720}, // ENCODER_MEM_DEPLOY_SUB_INST2_1
    {    0,              15360}, // ENCODER_MEM_DEPLOY_STREAM_INST0_0
    {    0,        250 + 15360}, // ENCODER_MEM_DEPLOY_STREAM_INST0_1
    {    0,        500 + 15360}, // ENCODER_MEM_DEPLOY_STREAM_INST0_2
    {    0,              16110}, // ENCODER_MEM_DEPLOY_STREAM_INST1_0
    {    0,        125 + 16110}, // ENCODER_MEM_DEPLOY_STREAM_INST1_1
    {    0,        250 + 16110}, // ENCODER_MEM_DEPLOY_STREAM_INST1_2
    {    0,              16485}, // ENCODER_MEM_DEPLOY_STREAM_INST2_0
    {    0,         60 + 16485}, // ENCODER_MEM_DEPLOY_STREAM_INST2_1
    {    0,        120 + 16485}, // ENCODER_MEM_DEPLOY_STREAM_INST2_2
    {    0,              16665}, // ENCODER_MEM_DEPLOY_SPS_PPS_0
    {    0,         10 + 16665}, // ENCODER_MEM_DEPLOY_SPS_PPS_1
    {    0,         20 + 16665}, // ENCODER_MEM_DEPLOY_SPS_PPS_2   // Height : 16695  
#else
    {    0,                  0}, // ENCODER_MEM_DEPLOY_CODE
    {    0,                112}, // ENCODER_MEM_DEPLOY_PARA
    {    0,                120}, // ENCODER_MEM_DEPLOY_TEMP
    {    0,                220}, // ENCODER_MEM_DEPLOY_WORK_0
    {    0,                244}, // ENCODER_MEM_DEPLOY_WORK_1
    {    0,                268}, // ENCODER_MEM_DEPLOY_WORK_2
    {    0,                320}, // ENCODER_MEM_DEPLOY_REC_INST0_Y0
    {    0,         1280 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_U0
    {    0,         1920 + 320}, // ENCODER_MEM_DEPLOY_REC_INST0_V0
    {    0,               2240}, // ENCODER_MEM_DEPLOY_REC_INST0_Y1
    {    0,        1280 + 2240}, // ENCODER_MEM_DEPLOY_REC_INST0_U1
    {    0,        1920 + 2240}, // ENCODER_MEM_DEPLOY_REC_INST0_V1
    {    0,               4160}, // ENCODER_MEM_DEPLOY_REC_INST1_Y0
    {    0,         640 + 4160}, // ENCODER_MEM_DEPLOY_REC_INST1_U0
    {    0,         960 + 4160}, // ENCODER_MEM_DEPLOY_REC_INST1_V0
    {    0,               5120}, // ENCODER_MEM_DEPLOY_REC_INST1_Y1
    {    0,         640 + 5120}, // ENCODER_MEM_DEPLOY_REC_INST1_U1
    {    0,         960 + 5120}, // ENCODER_MEM_DEPLOY_REC_INST1_V1
    {    0,               6080}, // ENCODER_MEM_DEPLOY_REC_INST2_Y0
    {    0,         320 + 6080}, // ENCODER_MEM_DEPLOY_REC_INST2_U0
    {    0,         640 + 6080}, // ENCODER_MEM_DEPLOY_REC_INST2_V0
    {    0,               6720}, // ENCODER_MEM_DEPLOY_REC_INST2_Y1
    {    0,         320 + 6720}, // ENCODER_MEM_DEPLOY_REC_INST2_U1
    {    0,         640 + 6720}, // ENCODER_MEM_DEPLOY_REC_INST2_V1
    {    0,               7360}, // ENCODER_MEM_DEPLOY_SRC_Y0
    {    0,        1280 + 7360}, // ENCODER_MEM_DEPLOY_SRC_U0
    {    0,        1920 + 7360}, // ENCODER_MEM_DEPLOY_SRC_V0
    {    0,               9280}, // ENCODER_MEM_DEPLOY_SRC_Y1
    {    0,        1280 + 9280}, // ENCODER_MEM_DEPLOY_SRC_U1
    {    0,        1920 + 9280}, // ENCODER_MEM_DEPLOY_SRC_V1
    {    0,              11200}, // ENCODER_MEM_DEPLOY_SRC_Y2
    {    0,       1280 + 11200}, // ENCODER_MEM_DEPLOY_SRC_U2
    {    0,       1920 + 11200}, // ENCODER_MEM_DEPLOY_SRC_V2
    {    0,              13120}, // ENCODER_MEM_DEPLOY_SRC_Y3
    {    0,       1280 + 13120}, // ENCODER_MEM_DEPLOY_SRC_U3
    {    0,       1920 + 13120}, // ENCODER_MEM_DEPLOY_SRC_V3        
    {    0,              15040}, // ENCODER_MEM_DEPLOY_SRC_Y4
    {    0,       1280 + 15040}, // ENCODER_MEM_DEPLOY_SRC_U4
    {    0,       1920 + 15040}, // ENCODER_MEM_DEPLOY_SRC_V4     
    {    0,              16960}, // ENCODER_MEM_DEPLOY_SUB_INST0_0
    {    0,       1088 + 16960}, // ENCODER_MEM_DEPLOY_SUB_INST0_1
    {    0,              19136}, // ENCODER_MEM_DEPLOY_SUB_INST1_0
    {    0,        640 + 19136}, // ENCODER_MEM_DEPLOY_SUB_INST1_1
    {    0,              20416}, // ENCODER_MEM_DEPLOY_SUB_INST2_0
    {    0,        320 + 20416}, // ENCODER_MEM_DEPLOY_SUB_INST2_1
    {    0,              21056}, // ENCODER_MEM_DEPLOY_STREAM_INST0_0
    {    0,        375 + 21056}, // ENCODER_MEM_DEPLOY_STREAM_INST0_1
    {    0,        750 + 21056}, // ENCODER_MEM_DEPLOY_STREAM_INST0_2
    {    0,              22181}, // ENCODER_MEM_DEPLOY_STREAM_INST1_0
    {    0,        125 + 22181}, // ENCODER_MEM_DEPLOY_STREAM_INST1_1
    {    0,        250 + 22181}, // ENCODER_MEM_DEPLOY_STREAM_INST1_2
    {    0,              22556}, // ENCODER_MEM_DEPLOY_STREAM_INST2_0
    {    0,         60 + 22556}, // ENCODER_MEM_DEPLOY_STREAM_INST2_1
    {    0,        120 + 22556}, // ENCODER_MEM_DEPLOY_STREAM_INST2_2
    {    0,              22736}, // ENCODER_MEM_DEPLOY_SPS_PPS_0
    {    0,         10 + 22736}, // ENCODER_MEM_DEPLOY_SPS_PPS_1
    {    0,         20 + 22736}, // ENCODER_MEM_DEPLOY_SPS_PPS_2   // Height : 22766
#endif   
#endif 
#endif    
};
//=============================================================================
//                              Private Function Declaration
//=============================================================================
static MMP_UINT32 _TiledMapping(MMP_UINT32 bufAdr, MMP_UINT32 baseAdr, MMP_UINT32 offset);
static void _WriteMemDW(MMP_UINT32 dest, MMP_UINT32 data);

//=============================================================================
//                              Public Function Definition
//=============================================================================

VIDEO_ERROR_CODE
encoder_memory_Initialize(
    void)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
MMP_UINT32 size = DEPLOY_BUFFER_PITCH * DEPLOY_BUFFER_HEIGHT;
MMP_UINT32 i;
MMP_UINT32 offset = 0;
MMP_UINT32 tmpBuf;

    size = (DEPLOY_BUFFER_PITCH * DEPLOY_BUFFER_HEIGHT) + ((1<<(EM_CAS+EM_BANK+EM_WIDTH+EM_RAS)) - 1);
    tmpBuf = (MMP_UINT32) MEM_Allocate(size, MEM_USER_VIDEO);   
    
#ifndef NULL_CMD
    if (tmpBuf == 0)
        error = ERROR_ENCODER_MEM_REQUEST;
#endif
    
    tmpBuf -= (MMP_UINT32)HOST_GetVramBaseAddress();
    
    EndeployBuffer[0] = (((tmpBuf + ((1<<(EM_CAS+EM_BANK+EM_WIDTH+EM_RAS)) - 1))>> (EM_CAS+EM_BANK+EM_WIDTH+EM_RAS)) << (EM_CAS+EM_BANK+EM_WIDTH+EM_RAS));
        
    for (i=1; i<ENCODER_MEM_DEPLOY_COUNT; i++)
    {
        offset = EndeployTable[i][0] + EndeployTable[i][1]*DEPLOY_BUFFER_PITCH;
        EndeployBuffer[i] = EndeployBuffer[0] + offset;
    }
    
#if 0
    for (i=0; i<ENCODER_MEM_DEPLOY_COUNT; i++)
    {
        LOG_INFO " buf[%d] 0x%08X\n", i, EndeployBuffer[i] LOG_END          
    }
#endif    
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetCodeBuffer(
    MMP_UINT8** ppBuf)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    *ppBuf = (MMP_UINT8*) EndeployBuffer[ENCODER_MEM_DEPLOY_CODE];
 
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetParaBuffer(
    MMP_UINT8** ppBuf)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    *ppBuf = (MMP_UINT8*) EndeployBuffer[ENCODER_MEM_DEPLOY_PARA];
 
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetTempBuffer(
    MMP_UINT8** ppBuf)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    *ppBuf = (MMP_UINT8*) EndeployBuffer[ENCODER_MEM_DEPLOY_TEMP];
 
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetWorkBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 instNum)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
   
    *ppBuf = (MMP_UINT8*) EndeployBuffer[ENCODER_MEM_DEPLOY_WORK_0+instNum];
 
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetStreamBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 index,
    MMP_UINT32 instNum)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
   
    *ppBuf = (MMP_UINT8*) EndeployBuffer[ENCODER_MEM_DEPLOY_STREAM_INST0_0+index+instNum*3];
 
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetReconBuffer(
    VIDEO_FRAME_BUFFER* ptFrameBuf,
    MMP_UINT32 index,
    MMP_UINT32 instNum)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
MMP_UINT32 LumFieldHeight;
MMP_UINT32 bufbase;
MMP_UINT32 framebuf;
MMP_UINT32 offset;
MMP_UINT32 destAdr;
    
#if defined(MULTIPLE_INSTANCES)
    if (instNum == 0)
        LumFieldHeight = INST_0_LUM_HEIGHT >> 1;
    else if (instNum == 1)
        LumFieldHeight = INST_1_LUM_HEIGHT >> 1;
    else
        LumFieldHeight = INST_2_LUM_HEIGHT >> 1;
#else
    LumFieldHeight = MAX_LUM_HEIGHT >> 1;
#endif    
    
    bufbase = EndeployBuffer[ENCODER_MEM_DEPLOY_REC_INST0_Y0];
    
    framebuf = EndeployBuffer[ENCODER_MEM_DEPLOY_REC_INST0_Y0+index*3+instNum*6];
    offset = LumFieldHeight * TILED_FRAME_PITCH;
    framebuf  = _TiledMapping(framebuf, bufbase, offset);
    ptFrameBuf->pAddress_Y = (MMP_UINT8*)framebuf;
    destAdr = EndeployBuffer[ENCODER_MEM_DEPLOY_PARA] + index * 12;
    _WriteMemDW(destAdr, framebuf);
        
    framebuf = EndeployBuffer[ENCODER_MEM_DEPLOY_REC_INST0_U0+index*3+instNum*6];
    offset = (LumFieldHeight >> 1) * TILED_FRAME_PITCH;
    framebuf  = _TiledMapping(framebuf, bufbase, offset);
    ptFrameBuf->pAddress_U = (MMP_UINT8*)framebuf;
    destAdr = EndeployBuffer[ENCODER_MEM_DEPLOY_PARA] + index * 12 + 4;
    _WriteMemDW(destAdr, framebuf);    
    
    framebuf = EndeployBuffer[ENCODER_MEM_DEPLOY_REC_INST0_U0+index*3+instNum*6];
    offset = 8 * (1 << (EM_CAS+EM_BANK+EM_WIDTH));
    framebuf  = _TiledMapping(framebuf, bufbase, offset);
    ptFrameBuf->pAddress_V = (MMP_UINT8*)framebuf;
    destAdr = EndeployBuffer[ENCODER_MEM_DEPLOY_PARA] + index * 12 + 8;
   _WriteMemDW(destAdr, framebuf);
               
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetSourceBuffer(
    VIDEO_FRAME_BUFFER* ptFrameBuf,
    VIDEO_FRAME_BUFFER* ptTiledFrmBuf,
    MMP_UINT32 index)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
MMP_UINT32 LumFieldHeight;
MMP_UINT32 bufbase;
MMP_UINT32 framebuf;
MMP_UINT32 offset;

#if defined(MULTIPLE_INSTANCES)    
    LumFieldHeight = INST_0_LUM_HEIGHT >> 1;    
#else
    LumFieldHeight = MAX_LUM_HEIGHT >> 1;
#endif 
    
    bufbase = EndeployBuffer[ENCODER_MEM_DEPLOY_REC_INST0_Y0];
    
    framebuf = EndeployBuffer[ENCODER_MEM_DEPLOY_SRC_Y0+index*3];
    ptFrameBuf->pAddress_Y = (MMP_UINT8*)framebuf;
    offset = LumFieldHeight * TILED_FRAME_PITCH;    
    framebuf = _TiledMapping(framebuf, bufbase, offset);    
    ptTiledFrmBuf->pAddress_Y = (MMP_UINT8*)framebuf;
    
    framebuf = EndeployBuffer[ENCODER_MEM_DEPLOY_SRC_U0+index*3];
    ptFrameBuf->pAddress_U = (MMP_UINT8*)framebuf;
    offset = (LumFieldHeight >> 1) * TILED_FRAME_PITCH;   
    framebuf = _TiledMapping(framebuf, bufbase, offset);    
    ptTiledFrmBuf->pAddress_U = (MMP_UINT8*)framebuf;
    
    framebuf = EndeployBuffer[ENCODER_MEM_DEPLOY_SRC_U0+index*3];
    ptFrameBuf->pAddress_V = (MMP_UINT8*)framebuf;
    offset = 8 * (1 << (EM_CAS+EM_BANK+EM_WIDTH));
    framebuf = _TiledMapping(framebuf, bufbase, offset);    
    ptTiledFrmBuf->pAddress_V = (MMP_UINT8*)framebuf;
  
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetSubImgBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 index,
    MMP_UINT32 instNum)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
   
    *ppBuf = (MMP_UINT8*) EndeployBuffer[ENCODER_MEM_DEPLOY_SUB_INST0_0+index+instNum*2];
 
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_GetPSBuffer(
    MMP_UINT8** ppBuf,
    MMP_UINT32 instNum)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
   
    *ppBuf = (MMP_UINT8*) EndeployBuffer[ENCODER_MEM_DEPLOY_SPS_PPS_0+instNum];
 
    return error;
}

VIDEO_ERROR_CODE
encoder_memory_CreateBuffer(
    MMP_UINT8** ppBuf, 
    MMP_UINT32 bufSize)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

    *ppBuf = (MMP_UINT8*) MEM_Allocate(bufSize, MEM_USER_VIDEO);
    if (*ppBuf == 0)
        error = ERROR_ENCODER_MEM_REQUEST;

    return error;
}

VIDEO_ERROR_CODE
encoder_memory_FreeBuffer(
    MMP_UINT8* pBuf)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;
    
    if (pBuf)
    {
        if (MEM_STATUS_SUCCESS != MEM_Release(pBuf))
            error = ERROR_ENCODER_MEM_REQUEST;
    }
    
    return error;
}

MMP_UINT32
encoder_memory_GetTiledBaseAddr(void)
{
    return EndeployBuffer[ENCODER_MEM_DEPLOY_REC_INST0_Y0];
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_UINT32
_TiledMapping(
    MMP_UINT32 bufAdr,
    MMP_UINT32 baseAdr,
    MMP_UINT32 offset)
{
#if defined(TILED_MODE_MAPPING)
MMP_UINT32 topbufAdr;
MMP_UINT32 botbufAdr;
    
    topbufAdr = (bufAdr - baseAdr);
    botbufAdr = topbufAdr + offset;
    
    return (botbufAdr >> (EM_CAS+EM_BANK+EM_WIDTH)) << 16 | (topbufAdr >> (EM_CAS+EM_BANK+EM_WIDTH));
#else
    return bufAdr;
#endif    
}

static void
_WriteMemDW(
    MMP_UINT32 dest,
    MMP_UINT32 data)
{
MMP_UINT32 WrData;
   
#if defined(__OPENRTOS__)
   WrData = data;
#else   
   WrData = ((data >> 24) & 0xFF) |((data >> 16) & 0xFF) << 8 | ((data >> 8) & 0xFF) << 16 | (data & 0xFF) << 24;
#endif
   HOST_WriteBlockMemory(dest, (MMP_UINT32) &WrData, 4);
}
