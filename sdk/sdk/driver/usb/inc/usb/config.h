/*
 * Copyright (c) 2008 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */

#ifndef USB_EX_CONFIG_H
#define USB_EX_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define USB0_BASE ITH_USB0_BASE
#define USB1_BASE ITH_USB1_BASE

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_usbex.h"

//#define RUN_FPGA
#define POLLING_REMOVE
#define ISR_TO_TASK

//=============================================================================
//                              Constant Definition
//=============================================================================
// Log definitions
typedef enum
{
    USB_ZONE_ERROR   = (0x1 << 0),
    USB_ZONE_WARNING = (0x1 << 1),
    USB_ZONE_INFO    = (0x1 << 2),
    USB_ZONE_DEBUG   = (0x1 << 3),
    USB_ZONE_ENTER   = (0x1 << 4),
    USB_ZONE_LEAVE   = (0x1 << 5),
    USB_ZONE_ALL     = 0xFFFFFFFF
} USBLogZones;

#if 1
    #define USB_LOG_ZONES (USB_ZONE_ERROR /*|USB_ZONE_DEBUG| USB_ZONE_WARNING*/)

    #define LOG_ERROR     ((void) ((USB_ZONE_ERROR & USB_LOG_ZONES) ? (ithPrintf ("[USB][ERR]"
    #define LOG_WARNING   ((void) ((USB_ZONE_WARNING & USB_LOG_ZONES) ? (ithPrintf ("[USB][WARN]"
    #define LOG_INFO      ((void) ((USB_ZONE_INFO & USB_LOG_ZONES) ? (ithPrintf ("[USB][INFO]"
    #define LOG_DEBUG     ((void) ((USB_ZONE_DEBUG & USB_LOG_ZONES) ? (ithPrintf ("[USB][DBG]"
    #define LOG_ENTER     ((void) ((USB_ZONE_ENTER & USB_LOG_ZONES) ? (ithPrintf ("[USB][ENTER]"
    #define LOG_LEAVE     ((void) ((USB_ZONE_LEAVE & USB_LOG_ZONES) ? (ithPrintf ("[USB][LEAVE]"
    #define LOG_DATA      ((void) ((true) ? (ithPrintf (
    #define LOG_CMD       ((void) ((false) ? (ithPrintf ("[USB][CMD]"
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
    #define LOG_CMD
    #define LOG_END     ;
#endif

#define check_result(rc) do { if (rc) LOG_ERROR "[%s] res = %d(0x%08X) \n", __FUNCTION__, rc, rc LOG_END } while (0)
#define _hcd_func_enter do { LOG_ENTER "%s \n", __FUNCTION__ LOG_END } while (0)
#define _hcd_func_leave do { LOG_LEAVE "%s \n", __FUNCTION__ LOG_END } while (0)

#define _usb_func_enter do { LOG_ENTER "%s \n", __FUNCTION__ LOG_END } while (0)
#define _usb_func_leave do { LOG_LEAVE "%s \n", __FUNCTION__ LOG_END } while (0)

#define DEV_WARN        0
#define DEV_DBG         0
#define DEV_INFO        0

#define dev_WARN        dev_warn

#if DEV_WARN
    #define dev_warn(ddev, string, args ...) do { ithPrintf("[USB][WARN] "); \
                                                  ithPrintf(string, ## args); \
                                             } while (0)
#else
    #define dev_warn(ddev, string, args ...)
#endif

#if DEV_DBG
    #define dev_dbg(ddev, string, args ...)  do { ithPrintf("[USB][DBG] ");  \
                                                  ithPrintf(string, ## args); \
                                             } while (0)
#else
    #define dev_dbg(ddev, string, args ...)
#endif

#if DEV_INFO
    #define dev_info(ddev, string, args ...) do { ithPrintf("[USB][INFO] ");  \
                                                  ithPrintf(string, ## args); \
                                             } while (0)
#else
    #define dev_info(ddev, string, args ...)
#endif

#define dev_err(ddev, string, args ...)      do { ithPrintf("[USB][ERR] "); \
                                                  ithPrintf(string, ## args); \
                                                  ithPrintf("	%s:%d\n", __FILE__, __LINE__); \
                                             } while (0)
#define dbg ithPrintf

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef USB_EX_CONFIG_H