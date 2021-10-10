///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <hdmitx.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/07/19
//   @fileversion: ITE_HDMITX_SAMPLE_2.03
//******************************************/

#ifndef _HDMITX_H_
#define _HDMITX_H_

#ifdef _MCU_8051_
    #include "mcu.h"
    #include "io.h"
    #include "utility.h"

#else // not MCU
   // #include <windows.h>
   // #include <winioctl.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
    //#include "ioaccess.h"
    //#include "install.h"
    //#include "pc.h"
#endif // MCU

#include "hdmitx/config.h"
#include "hdmitx/typedef.h"
// #include "edid.h"
#include "hdmitx/hdmitx_drv.h"
#include "hdmitx/debug.h"


#define HDMITX_INSTANCE_MAX 1

#define SIZEOF_CSCMTX 18
#define SIZEOF_CSCGAIN 6
#define SIZEOF_CSCOFFSET 3

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

///////////////////////////////////////////////////////////////////////
// ROM OFFSET
///////////////////////////////////////////////////////////////////////
#define ROMOFF_INT_TYPE 0
#define ROMOFF_INPUT_VIDEO_TYPE 1
#define ROMOFF_OUTPUT_AUDIO_MODE 8
#define ROMOFF_AUDIO_CH_SWAP 9



#define TIMER_LOOP_LEN 10
#define MS(x) (((x)+(TIMER_LOOP_LEN-1))/TIMER_LOOP_LEN); // for timer loop


#ifdef _MCU_8051_ // DSSSHA need large computation data rather than 8051 supported.
#ifndef DelayMS
#define DelayMS(x) delay1ms(x)
#endif
#endif

//#define HDMITX_DEBUG_PRINTF(x) printf x

#define SUPPORT_AUDI_AudSWL 16
#endif // _HDMITX_H_

