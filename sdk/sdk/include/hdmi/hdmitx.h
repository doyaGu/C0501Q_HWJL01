///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <hdmitx.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2012/01/13
//   @fileversion: ITE_HDMITX_SAMPLE_3.03
//******************************************/

#ifndef _HDMITX_H_
#define _HDMITX_H_

//#ifdef _MCU_8051_
//    #include "mcu.h"
//    #include "io.h"
//    #include "utility.h"
//
//#else // not MCU
//    #include <windows.h>
//    #include <winioctl.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
//    #include "usbi2c.h"
//    #include "io.h"
//#endif // MCU
#include "debug.h"
#include "config.h"
#include "typedef.h"
#include "hdmitx_drv.h"

#define HDMITX_MAX_DEV_COUNT 1


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


#define TIMER_LOOP_LEN 10
#define MS(x) (((x)+(TIMER_LOOP_LEN-1))/TIMER_LOOP_LEN); // for timer loop

// #define SUPPORT_AUDI_AudSWL 16 // Jeilin case.
#define SUPPORT_AUDI_AudSWL 24 // Jeilin case.

#if(SUPPORT_AUDI_AudSWL==16)
    #define CHTSTS_SWCODE 0x02
#elif(SUPPORT_AUDI_AudSWL==18)
    #define CHTSTS_SWCODE 0x04
#elif(SUPPORT_AUDI_AudSWL==20)
    #define CHTSTS_SWCODE 0x03
#else
    #define CHTSTS_SWCODE 0x0B
#endif

#endif // _HDMITX_H_

