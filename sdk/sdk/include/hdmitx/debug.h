///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <debug.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/07/19
//   @fileversion: ITE_HDMITX_SAMPLE_2.03
//******************************************/
#ifndef _DEBUG_H_
#define _DEBUG_H_

//#define DEBUG

#ifdef DEBUG
#define HDMITX_DEBUG_PRINTF(x) printf x
#define DUMPTX_DEBUG_PRINT(x) printf x
#else
#define HDMITX_DEBUG_PRINTF(x)
#define DUMPTX_DEBUG_PRINT(x)
#endif

#endif//  _DEBUG_H_
