/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */
#ifndef	UAS_CONFIG_H
#define	UAS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_uas.h"
#include "uas_error.h"


//=============================================================================
//                              Compile Option
//=============================================================================
//#define DUMP_CMD
//#define DUMP_STATUS
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
    UAS_ZONE_ERROR      = (0x1 << 0),
    UAS_ZONE_WARNING    = (0x1 << 1),
    UAS_ZONE_INFO       = (0x1 << 2),
    UAS_ZONE_DEBUG      = (0x1 << 3),
    UAS_ZONE_ENTER      = (0x1 << 4),
    UAS_ZONE_LEAVE      = (0x1 << 5),
    UAS_ZONE_ALL        = 0xFFFFFFFF
} UASLogZones;

#if 1
//#define UAS_LOG_ZONES    UAS_ZONE_ALL//(UAS_ZONE_ERROR | UAS_ZONE_WARNING)
#define UAS_LOG_ZONES    (UAS_ZONE_ERROR | UAS_ZONE_WARNING)

#define LOG_ERROR   ((void) ((UAS_ZONE_ERROR & UAS_LOG_ZONES) ? (ithPrintf("[UAS][ERR]"
#define LOG_WARNING ((void) ((UAS_ZONE_WARNING & UAS_LOG_ZONES) ? (ithPrintf("[UAS][WARN]"
#define LOG_INFO    ((void) ((UAS_ZONE_INFO & UAS_LOG_ZONES) ? (ithPrintf("[UAS][INFO]"
#define LOG_DEBUG   ((void) ((UAS_ZONE_DEBUG & UAS_LOG_ZONES) ? (ithPrintf("[UAS][DBG]"
#define LOG_ENTER   ((void) ((UAS_ZONE_ENTER & UAS_LOG_ZONES) ? (ithPrintf("[UAS][ENTER]"
#define LOG_LEAVE   ((void) ((UAS_ZONE_LEAVE & UAS_LOG_ZONES) ? (ithPrintf("[UAS][LEAVE]"
#define LOG_CMD     ((void) ((false) ? (ithPrintf("[UAS][CMD]"
#define LOG_DATA    ((void) ((true) ? (ithPrintf(
#define LOG_END     )), 1 : 0));
#else

#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_END         ;
#endif

#include "usb/ite_usb.h"
#include "scsi.h"
#include "uas.h"

#define _uas_func_enter			do{ LOG_ENTER "%s \n", __FUNCTION__ LOG_END } while(0)
#define _uas_func_leave			do{ LOG_LEAVE "%s \n", __FUNCTION__ LOG_END } while(0)
#define check_result(rc)		do{ if(rc) LOG_ERROR "[%s] res = 0x%08X \n", __FUNCTION__, rc LOG_END } while(0)
#define timeout_err()           do{ LOG_ERROR "[%s] cmd timeout! \n", __FUNCTION__ LOG_END } while(0)
#define uas_todo()              do{ ithPrintf("[%s] line:%d ==> TODO \n", __FUNCTION__, __LINE__); }while(0)


#ifdef __cplusplus
}
#endif

#endif
