/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 * @version 1.0
 */
#ifndef	UDC_CONFIG_H
#define	UDC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"

#include "udc_err.h"

//=============================================================================
//                              Compile Option
//=============================================================================
//#define SUSPEND_ENABLE

//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              LOG definition
//=============================================================================
// Log definitions
typedef enum
{
    UDC_ZONE_ERROR      = (0x1 << 0),
    UDC_ZONE_WARNING    = (0x1 << 1),
    UDC_ZONE_INFO       = (0x1 << 2),
    UDC_ZONE_DEBUG      = (0x1 << 3),
    UDC_ZONE_ENTER      = (0x1 << 4),
    UDC_ZONE_LEAVE      = (0x1 << 5),
    UDC_ZONE_CTRL       = (0x1 << 6),
    UDC_ZONE_REQ        = (0x1 << 7),
    UDC_ZONE_ALL        = 0xFFFFFFFF
} UdcLogZones;

#if 0
//#define UDC_LOG_ZONES    (UDC_ZONE_ERROR)
//#define UDC_LOG_ZONES    (UDC_ZONE_ERROR | UDC_ZONE_CTRL | UDC_ZONE_INFO)
#define UDC_LOG_ZONES    (UDC_ZONE_ERROR | UDC_ZONE_CTRL | UDC_ZONE_INFO | UDC_ZONE_DEBUG)
//#define UDC_LOG_ZONES    (UDC_ZONE_ALL)

#define UDC_ERROR   ((void) ((UDC_ZONE_ERROR & UDC_LOG_ZONES) ? (ithPrintf("[UDC][ERR]"
#define UDC_WARNING ((void) ((UDC_ZONE_WARNING & UDC_LOG_ZONES) ? (ithPrintf("[UDC][WARN]"
#define UDC_INFO    ((void) ((UDC_ZONE_INFO & UDC_LOG_ZONES) ? (ithPrintf("[UDC][INFO]"
#define UDC_CTRL    ((void) ((UDC_ZONE_CTRL & UDC_LOG_ZONES) ? (ithPrintf("[UDC][CTRL]"
#define UDC_REQ     ((void) ((UDC_ZONE_REQ & UDC_LOG_ZONES) ? (ithPrintf("[UDC][REQ]"
#define UDC_DBG     ((void) ((UDC_ZONE_DEBUG & UDC_LOG_ZONES) ? (ithPrintf("[UDC][DBG]"
#define UDC_ENTER   ((void) ((UDC_ZONE_ENTER & UDC_LOG_ZONES) ? (ithPrintf("[UDC][ENTER]"
#define UDC_LEAVE   ((void) ((UDC_ZONE_LEAVE & UDC_LOG_ZONES) ? (ithPrintf("[UDC][LEAVE]"
#define UDC_DATA    ((void) ((false) ? (ithPrintf(
#define UDC_DLEN    ((void) ((false) ? (ithPrintf(
#define UDC_END     )), 1 : 0));
#else
#define UDC_ZONES
#define UDC_ERROR
#define UDC_WARNING
#define UDC_INFO
#define UDC_CTRL
#define UDC_REQ
#define UDC_DBG
#define UDC_ENTER
#define UDC_LEAVE
#define UDC_DATA
#define UDC_DLEN
#define UDC_END         ;
#endif


#define check_result(rc)		do{ if(rc) UDC_ERROR "[%s] res = 0x%08X(%d) \n", __FUNCTION__, rc, rc UDC_END } while(0)
#define _udc_func_enter			do{ UDC_ENTER "%s \n", __FUNCTION__ UDC_END } while(0)
#define _udc_func_leave			do{ UDC_LEAVE "%s \n", __FUNCTION__ UDC_END } while(0)


#ifdef __cplusplus
}
#endif

#endif
