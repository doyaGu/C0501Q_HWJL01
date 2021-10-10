///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <hdmitx_sys.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/07/19
//   @fileversion: ITE_HDMITX_SAMPLE_2.03
//******************************************/

#ifndef _HDMITX_SYS_H_
#define _HDMITX_SYS_H_
#include "hdmitx/typedef.h"

////////////////////////////////////////////////////////////////////////////////
// Internal Data Type
////////////////////////////////////////////////////////////////////////////////

typedef enum tagHDMI_Video_Type {
    HDMI_Unkown = 0 ,
    HDMI_640x480p60 = 1 ,
    HDMI_480p60,
    HDMI_480p60_16x9,
    HDMI_720p60,
    HDMI_1080i60,
    HDMI_480i60,
    HDMI_480i60_16x9,
    HDMI_1080p60 = 16,
    HDMI_576p50,
    HDMI_576p50_16x9,
    HDMI_720p50,
    HDMI_1080i50,
    HDMI_576i50,
    HDMI_576i50_16x9,
    HDMI_1080p50 = 31,
    HDMI_1080p24,
    HDMI_1080p25,
    HDMI_1080p30,
    HDMI_720p30 = 61,
    HDMI_VESA = 0xFF,
} HDMI_Video_Type ;

typedef enum tagHDMI_Aspec {
    HDMI_4x3 ,
    HDMI_16x9
} HDMI_Aspec;

typedef enum tagHDMI_OutputColorMode {
    HDMI_RGB444,
    HDMI_YUV444,
    HDMI_YUV422
} HDMI_OutputColorMode ;

typedef enum tagHDMI_Colorimetry {
    HDMI_ITU601,
    HDMI_ITU709
} HDMI_Colorimetry ;

///////////////////////////////////////////////////////////////////////
// Output Mode Type
///////////////////////////////////////////////////////////////////////

#define RES_ASPEC_4x3 0
#define RES_ASPEC_16x9 1
#define F_MODE_REPT_NO 0
#define F_MODE_REPT_TWICE 1
#define F_MODE_REPT_QUATRO 3
#define F_MODE_CSC_ITU601 0
#define F_MODE_CSC_ITU709 1

void InitHDMITX_Instance_6613(TXInput_Device inputDevice);
void InitHDMITX_Instance_66121(TXInput_Device inputDevice);
void HDMITX_ChangeDisplayOption_6613(HDMI_Video_Type OutputVideoTiming, MODE_ID VesaTiming, unsigned int EnableHDCP, BOOL IsYUVInput);
void HDMITX_ChangeDisplayOption_66121(HDMI_Video_Type OutputVideoTiming, MODE_ID VesaTiming, unsigned int EnableHDCP, BOOL IsYUVInput);
void HDMITX_SetOutput_6613(unsigned int AudioSampleRate, unsigned int AudioSampleNum, TXInput_Device inputDevice);
void HDMITX_SetOutput_66121(unsigned int AudioSampleRate, unsigned int AudioSampleNum, TXInput_Device inputDevice);
void HDMITX_DevLoopProc_6613(unsigned char bHDMIRxModeChange, unsigned int AudioSampleRate, unsigned int AudioChannelNum, TXInput_Device inputDevice);
void HDMITX_DevLoopProc_66121(unsigned char bHDMIRxModeChange, unsigned int AudioSampleRate, unsigned int AudioChannelNum, TXInput_Device inputDevice);
void HDMITXDisable_6613();
void HDMITXDisable_66121();

#endif // _HDMITX_SYS_H_
