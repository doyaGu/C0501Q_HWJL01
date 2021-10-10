///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <config.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2012/01/13
//   @fileversion: ITE_HDMITX_SAMPLE_3.03
//******************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_


#ifdef EXTERN_HDCPROM
#pragma message("Defined EXTERN_HDCPROM")
#endif // EXTERN_HDCPROM

#define SUPPORT_EDID
//#define SUPPORT_HDCP
#define SUPPORT_SHA
//#define SUPPORT_AUDIO_MONITOR
#define AudioOutDelayCnt 250
// #define SUPPORT_HBR_AUDIO
#define USE_SPDIF_CHSTAT


// 2010/01/26 added a option to disable HDCP.
#define SUPPORT_OUTPUTYUV
#define SUPPORT_OUTPUTRGB
// #define DISABLE_HDMITX_CSC

#define SUPPORT_INPUTRGB
#define SUPPORT_INPUTYUV444
#define SUPPORT_INPUTYUV422
// #define SUPPORT_SYNCEMBEDDED
// #define SUPPORT_DEGEN
#define NON_SEQUENTIAL_YCBCR422


#define INPUT_SAMPLE_FREQ AUDFS_48KHz
#define OUTPUT_CHANNEL 2

//#define INPUT_COLOR_MODE F_MODE_RGB444
//#define INPUT_COLOR_MODE F_MODE_YUV422
#define INPUT_COLOR_MODE F_MODE_YUV444

#define INPUT_COLOR_DEPTH 24
//#define OUTPUT_COLOR_MODE F_MODE_YUV422
//#define OUTPUT_COLOR_MODE F_MODE_YUV444
#define OUTPUT_COLOR_MODE F_MODE_RGB444


//#define OUTPUT_3D_MODE Frame_Pcaking
//#define OUTPUT_3D_MODE Top_and_Botton
//#define OUTPUT_3D_MODE Side_by_Side

// #define INV_INPUT_ACLK
// #define INV_INPUT_PCLK

#ifdef SUPPORT_SYNCEMBEDDED
    // #define INPUT_SIGNAL_TYPE (T_MODE_SYNCEMB)                 // 16 bit sync embedded
    // #define INPUT_SIGNAL_TYPE (T_MODE_SYNCEMB | T_MODE_CCIR656) // 8 bit sync embedded
    #define INPUT_SIGNAL_TYPE (T_MODE_SYNCEMB|T_MODE_INDDR|T_MODE_PCLKDIV2) // 16 bit sync embedded DDR
    // #define INPUT_SIGNAL_TYPE (T_MODE_SYNCEMB|T_MODE_INDDR)      // 8  bit sync embedded DDR

    #define SUPPORT_INPUTYUV422
    #ifdef INPUT_COLOR_MODE
    #undef INPUT_COLOR_MODE
    #endif // INPUT_COLOR_MODE
    #define INPUT_COLOR_MODE F_MODE_YUV422
#else
    #define INPUT_SIGNAL_TYPE 0 // 24 bit sync seperate
    //#define INPUT_SIGNAL_TYPE ( T_MODE_DEGEN )
    //#define INPUT_SIGNAL_TYPE ( T_MODE_INDDR)
    //#define INPUT_SIGNAL_TYPE ( T_MODE_SYNCEMB)
    //#define INPUT_SIGNAL_TYPE ( T_MODE_CCIR656 | T_MODE_SYNCEMB )
#endif


#if defined(SUPPORT_INPUTYUV444) || defined(SUPPORT_INPUTYUV422)
#define SUPPORT_INPUTYUV
#endif

#ifdef SUPPORT_SYNCEMBEDDED
#pragma message("defined SUPPORT_SYNCEMBEDDED for Sync Embedded timing input or CCIR656 input.")
#endif

#endif
