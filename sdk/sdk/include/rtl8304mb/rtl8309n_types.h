/*
* Copyright (C) 2012 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: v1.0.1 $
* $Date: 2012-10-23 11:18:41 +0800 $
*
* Purpose : RTL8309N switch API varable type declaration
* Feature : 
*
*/
 
 
#ifndef _RTL8309N_TYPES_H_
#define _RTL8309N_TYPES_H_

#ifndef _RTL_TYPES_H
typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uint8;
typedef char int8;
#endif

typedef int32                   rtk_api_ret_t;
typedef int32                   ret_t;
typedef uint64                  rtk_u_long_t;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif


#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#define rtlglue_printf ithPrintf

#ifdef rtlglue_printf
#include <stdio.h>
#endif

#endif
