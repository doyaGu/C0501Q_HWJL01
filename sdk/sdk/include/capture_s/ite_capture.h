/*
 * Copyright (c) 2015 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Capture Driver API header file.
 *
 * @author Benson Lin
 * @version 1.0
 */
#ifndef ITE_CAPTURE_H
#define ITE_CAPTURE_H

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"
#include "capture_s/capture_types.h"
#include "capture_module/capture_module.h"

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
//typedef struct CAP_CONTEXT_TAG *ITE_CAP_HANDLE;          
#if defined (CFG_CAPTURE_MODULE_RESOLUTION_1280x720) || defined (CFG_SECOND_CAPTURE_MODULE_RESOLUTION_1280x720)
	#define CAP_MEM_BUF_PITCH    1280
	#define CAP_INPUT_MAX_HEIGHT 720

#elif defined(CFG_CAPTURE_MODULE_RESOLUTION_800x600) || defined (CFG_SECOND_CAPTURE_MODULE_RESOLUTION_800x600)
	#define CAP_MEM_BUF_PITCH    800
	#define CAP_INPUT_MAX_HEIGHT 600

#elif defined(CFG_CAPTURE_MODULE_RESOLUTION_640x480) || defined (CFG_SECOND_CAPTURE_MODULE_RESOLUTION_640x480)    
	#define CAP_MEM_BUF_PITCH    640
	#define CAP_INPUT_MAX_HEIGHT 480
#else
	#define CAP_MEM_BUF_PITCH	 CFG_CAPTURE_WIDTH
	#define CAP_INPUT_MAX_HEIGHT CFG_CAPTURE_HEIGHT 
#endif	
	
#ifdef TRIPLE_BUFFER
    #define CAP_MEM_BUF_HEIGHT   (CAP_MEM_BUF_PITCH * 3 + (CAP_MEM_BUF_PITCH >> 1) * 6 ) //  3Y
#else
    #define CAP_MEM_BUF_HEIGHT   (CAP_MEM_BUF_PITCH * 2 + (CAP_MEM_BUF_PITCH >> 1) * 4)  // 2Y
#endif

typedef enum MMP_CAP_ERROR_CODE_TAG
{
    MMP_HSYNC_ERR = 0x1,        //0x01
    MMP_VSYNC_ERR,              //0x02
    MMP_SIZE_ERROR_ERR,         //0x03
    MMP_OVERFLOW_ERR,           //0x04
    MMP_ABNORMAL_TERMINATE_ERR, //0x05
    MMP_TIMEOUT_ERR,            //0x06
    MMP_SCDTO_ERR,              //0x07
    MMP_BT656_SYNC_ERR,         //0x08
    MMP_BT656_PREAMBLE_ERR,     //0x09
    MMP_MEM_LAST_ERR,           //0x10
    MMP_FRAME_SYNC_POINT_ERR,   //0x11
    MMP_FRAME_SYNC_ACTIVE_ERR,  //0x12
    MMP_FIRE_ISP_ERR,           //0x13
    MMP_INTERNAL_DATAPTH_ERR    //0x14
} MMP_CAP_ERROR_CODE;

typedef enum MMP_CAP_FRAMERATE_TAG
{
    MMP_CAP_FRAMERATE_UNKNOW = 0,
    MMP_CAP_FRAMERATE_25HZ,
    MMP_CAP_FRAMERATE_50HZ,
    MMP_CAP_FRAMERATE_30HZ,
    MMP_CAP_FRAMERATE_60HZ,
    MMP_CAP_FRAMERATE_29_97HZ,
    MMP_CAP_FRAMERATE_59_94HZ,
    MMP_CAP_FRAMERATE_23_97HZ,
    MMP_CAP_FRAMERATE_24HZ,
    MMP_CAP_FRAMERATE_56HZ,
    MMP_CAP_FRAMERATE_70HZ,
    MMP_CAP_FRAMERATE_72HZ,
    MMP_CAP_FRAMERATE_75HZ,
    MMP_CAP_FRAMERATE_85HZ,
    MMP_CAP_FRAMERATE_VESA_30HZ,
    MMP_CAP_FRAMERATE_VESA_60HZ
} MMP_CAP_FRAMERATE;


typedef struct ITE_CAP_VIDEO_INFO_TAG
{
    uint16_t          OutWidth;
    uint16_t          OutHeight;
    uint8_t           IsInterlaced;
    MMP_CAP_FRAMERATE FrameRate;
    uint8_t           *DisplayAddrY;
    uint8_t           *DisplayAddrU;
    uint8_t           *DisplayAddrV;
    uint16_t          PitchY;
    uint16_t          PitchUV;
} ITE_CAP_VIDEO_INFO; //ITE_CAP_INFO;

//=============================================================================
//                  Global Data Definition
//=============================================================================
enum {MAX_CAPTURE_MODULE = 2};

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
bool ithCapDeviceIsSignalStable();
int ithCapInitialize();
int ithCapTerminate();
int ithCapStop();
int ithCapFire();
int ithCaptureGetNewFrame(ITE_CAP_VIDEO_INFO *Outdata);
int ithCaptureSetModule(CaptureModuleDriver);
bool ithCaptureGetDetectedStatus();
int ithCapControllerSetBT601(uint16_t capwidth,uint16_t capheight,CAP_INPUT_DATA_FORMAT input_dFmt);
int ithCapControllerSetBT601Href(uint16_t capwidth,uint16_t capheight,CAP_INPUT_DATA_FORMAT input_dFmt);
int ithCapControllerSetBT601WithoutDE(uint16_t capwidth,uint16_t capheight,CAP_INPUT_DATA_FORMAT input_dFmt,uint16_t HS_AreaStart,uint16_t HS_AreaEnd,uint16_t VS_TopAreaStart,uint16_t VS_TopAreaEnd,uint16_t VS_BtmAreaStart,uint16_t VS_BtmAreaEnd);
int ithCapControllerSetBT656(uint16_t capwidth,uint16_t capheight,CAP_INPUT_DATA_FORMAT input_dFmt);


#ifdef __cplusplus
}
#endif

#endif
