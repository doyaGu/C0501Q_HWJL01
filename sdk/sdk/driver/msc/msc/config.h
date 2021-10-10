/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */
#ifndef MSC_CONFIG_H
#define MSC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_msc.h"
#include "msc_error.h"

//=============================================================================
//                              Compile Option
//=============================================================================
//#define DUMP_CMD
//#define US_CBW_BUSY_WAIT_EN

//=============================================================================
//                              Constant Definition
//=============================================================================
#define US_BULK_TIMEOUT 10000
#define US_ADSC_TIMEOUT 2000

//=============================================================================
//                              LOG definition
//=============================================================================
// Log definitions
typedef enum
{
    MSC_ZONE_ERROR   = (0x1 << 0),
    MSC_ZONE_WARNING = (0x1 << 1),
    MSC_ZONE_INFO    = (0x1 << 2),
    MSC_ZONE_DEBUG   = (0x1 << 3),
    MSC_ZONE_ENTER   = (0x1 << 4),
    MSC_ZONE_LEAVE   = (0x1 << 5),
    MSC_ZONE_ALL     = 0xFFFFFFFF
} MSCLogZones;

#if 1
    #define MSC_LOG_ZONES 0//(MSC_ZONE_ERROR | MSC_ZONE_WARNING)

    #define LOG_ERROR     ((void) ((MSC_ZONE_ERROR & MSC_LOG_ZONES) ? (printf ("[MSC][ERROR]"
    #define LOG_WARNING   ((void) ((MSC_ZONE_WARNING & MSC_LOG_ZONES) ? (printf ("[MSC][WARNING]"
    #define LOG_INFO      ((void) ((MSC_ZONE_INFO & MSC_LOG_ZONES) ? (printf ("[MSC][INFO]"
    #define LOG_DEBUG     ((void) ((MSC_ZONE_DEBUG & MSC_LOG_ZONES) ? (printf ("[MSC][DEBUG]"
    #define LOG_ENTER     ((void) ((MSC_ZONE_ENTER & MSC_LOG_ZONES) ? (printf ("[MSC][ENTER]"
    #define LOG_LEAVE     ((void) ((MSC_ZONE_LEAVE & MSC_LOG_ZONES) ? (printf ("[MSC][LEAVE]"
    #define LOG_CMD       ((void) ((false) ? (printf ("[MSC][CMD]"
    #define LOG_DATA      ((void) ((true) ? (printf (
    #define LOG_END       )), 1 : 0));
#else

    #define LOG_ZONES
    #define LOG_ERROR
    #define LOG_WARNING
    #define LOG_INFO
    #define LOG_DEBUG
    #define LOG_ENTER
    #define LOG_LEAVE
    #define LOG_DATA
    #define LOG_END ;
#endif

#include "usb/ite_usb.h"

#include "msc/scsi.h"
#include "msc/usb.h"
#include "msc/protocol.h"
#include "msc/transport.h"

#define mmpMscDriverRegister      iteMscDriverRegister
#define mmpMscInitialize          iteMscInitialize
#define mmpMscTerminate           iteMscTerminate
#define mmpMscReadMultipleSector  iteMscReadMultipleSector
#define mmpMscWriteMultipleSector iteMscWriteMultipleSector
#define mmpMscGetCapacity         iteMscGetCapacity
#define mmpMscGetStatus           iteMscGetStatus
#define mmpMscGetStatus2          iteMscGetStatus2
#define mmpMscIsLock              iteMscIsLock
#define mmpMscInDataAccess        iteMscInDataAccess

#define sdk_msg_ex(a, b) printf(b)

#ifdef __cplusplus
}
#endif

#endif