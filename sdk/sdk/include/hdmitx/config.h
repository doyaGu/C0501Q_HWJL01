///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <config.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/07/19
//   @fileversion: ITE_HDMITX_SAMPLE_2.03
//******************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_


#ifdef EXTERN_HDCPROM
#pragma message("Defined EXTERN_HDCPROM")
#endif // EXTERN_HDCPROM

#define SUPPORT_EDID
#define SUPPORT_HDCP
#define SUPPORT_SHA
// #define DISABLE_HDMITX_CSC

#define SUPPORT_INPUTRGB
#define SUPPORT_INPUTYUV444
#define SUPPORT_INPUTYUV422
//#define SUPPORT_SYNCEMBEDDED
//#if defined(HDMITX_IT6613)
//#define SUPPORT_DEGEN
//#endif
#define NON_SEQUENTIAL_YCBCR422
// #define INV_INPUT_PCLK
// #define INV_INPUT_ACLK

#ifdef SUPPORT_SYNCEMBEDDED
#pragma message("defined SUPPORT_SYNCEMBEDDED for Sync Embedded timing input or CCIR656 input.")
#endif

#if defined(SUPPORT_INPUTYUV444) || defined(SUPPORT_INPUTYUV422)
#define SUPPORT_INPUTYUV
#endif


// 2010/01/26 added a option to disable HDCP.
#define SUPPORT_OUTPUTYUV
#define SUPPORT_OUTPUTRGB

// #define HDMITX_AUTO_MONITOR_INPUT
// #define HDMITX_INPUT_INFO

#ifdef  HDMITX_AUTO_MONITOR_INPUT
#define HDMITX_INPUT_INFO
#endif
#endif
