/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 * @version 1.0
 */
#ifndef	FSG_CONFIG_H
#define	FSG_CONFIG_H

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

#include "fsg_err.h"

//=============================================================================
//                              Compile Option
//=============================================================================
//#define DUMP_DATA

//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              LOG definition
//=============================================================================
// Log definitions
typedef enum
{
    FSG_ZONE_ERROR      = (0x1 << 0),
    FSG_ZONE_WARNING    = (0x1 << 1),
    FSG_ZONE_INFO       = (0x1 << 2),
    FSG_ZONE_DEBUG      = (0x1 << 3),
    FSG_ZONE_ENTER      = (0x1 << 4),
    FSG_ZONE_LEAVE      = (0x1 << 5),
    FSG_ZONE_ALL        = 0xFFFFFFFF
} FsgLogZones;

#if 0
#define FSG_LOG_ZONES    (FSG_ZONE_ERROR | FSG_ZONE_WARNING)
//#define FSG_LOG_ZONES    (FSG_ZONE_ERROR | FSG_ZONE_INFO)
//#define FSG_LOG_ZONES    (FSG_ZONE_ERROR | FSG_ZONE_INFO | FSG_ZONE_WARNING)
//#define FSG_LOG_ZONES    (FSG_ZONE_ERROR | FSG_ZONE_WARNING | FSG_ZONE_INFO | FSG_ZONE_DEBUG)
//#define FSG_LOG_ZONES    (FSG_ZONE_ALL)

#define FSG_ERROR   ((void) ((FSG_ZONE_ERROR & FSG_LOG_ZONES) ? (ithPrintf("[FSG][ERR]"
#define FSG_WARNING ((void) ((FSG_ZONE_WARNING & FSG_LOG_ZONES) ? (ithPrintf("[FSG][WARN]"
#define FSG_INFO    ((void) ((FSG_ZONE_INFO & FSG_LOG_ZONES) ? (ithPrintf("[FSG][INFO]"
#define FSG_DBG     ((void) ((FSG_ZONE_DEBUG & FSG_LOG_ZONES) ? (ithPrintf("[FSG][DBG]"
#define FSG_ENTER   ((void) ((FSG_ZONE_ENTER & FSG_LOG_ZONES) ? (ithPrintf("[FSG][ENTER]"
#define FSG_LEAVE   ((void) ((FSG_ZONE_LEAVE & FSG_LOG_ZONES) ? (ithPrintf("[FSG][LEAVE]"
#define FSG_DATA    ((void) ((true) ? (ithPrintf(
#define FSG_END     )), 1 : 0));
#else
#define FSG_ZONES
#define FSG_ERROR
#define FSG_WARNING
#define FSG_INFO
#define FSG_DBG
#define FSG_ENTER
#define FSG_LEAVE
#define FSG_DATA
#define FSG_END         ;
#endif


#define check_result(rc)		do{ if(rc) FSG_ERROR "[%s] res = 0x%08X(%d) \n", __FUNCTION__, rc, rc FSG_END }while(0)
#define _fsg_func_enter			do{ FSG_ENTER "%s \n", __FUNCTION__ FSG_END } while(0)
#define _fsg_func_leave			do{ FSG_LEAVE "%s \n", __FUNCTION__ FSG_END } while(0)

#define min		ITH_MIN


#ifdef __cplusplus
}
#endif

#endif
