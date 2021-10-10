/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */
#ifndef MS_CONFIG_H
#define MS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_mspro.h"

#include "mspro/ms_port.h"
#include "mspro/ms_error.h"

#if !defined(FPGA)
//#define FPGA
#endif

//=============================================================================
//                              Compile Option
//=============================================================================
#define MS_WIN32_DMA
//#define MS_WORKAROUND
//#define WR_TMP_STATUS
//#define MS_SERIAL_ONLY_DISABLE

//#define MS_SONY_EMULATOR_TEST

#if defined(MS_SONY_EMULATOR_TEST)
    #define MS_SONY_FILE_SYSTEM
    #define MS_SONY_2_STATE_ERROR
#endif

#if defined(__OPENRTOS__)
//#define MS_IRQ_ENABLE
//#define MS_DMA_IRQ_ENABLE
    #undef MS_WIN32_DMA
#endif

#define MS_IO_MODE ITH_STOR_MS_0      // mode 3
//#define MS_IO_MODE      ITH_STOR_MS_1 // mode 4

//=============================================================================
//                              Constant Definition
//=============================================================================
/** WCLK = 133Mhz */
#if 0
    #define MS_SERIAL_CLK_DIV   2  // 133/((2+1)*2) = 22.16Mhz
    #define MS_PARALLEL_CLK_DIV 1  // 133/((1+1)*2) = 33Mhz
#endif

#if 1
    #define MS_SERIAL_CLK_DIV   1  // 80/((1+1)*2) = 20Mhz
    #define MS_PARALLEL_CLK_DIV 0  // 80/((0+1)*2) = 40Mhz
#endif

//=============================================================================
//                              LOG definition
//=============================================================================
// Log definitions
typedef enum
{
    MS_ZONE_ERROR   = (0x1 << 0),
    MS_ZONE_WARNING = (0x1 << 1),
    MS_ZONE_INFO    = (0x1 << 2),
    MS_ZONE_DEBUG   = (0x1 << 3),
    MS_ZONE_ENTER   = (0x1 << 4),
    MS_ZONE_LEAVE   = (0x1 << 5),
    MS_ZONE_ALL     = 0xFFFFFFFF
} MSLogZones;

#if 1
    #define MS_LOG_ZONES (MS_ZONE_ERROR /*| MS_ZONE_WARNING | MS_ZONE_INFO | MS_ZONE_DEBUG*/)

    #define LOG_ERROR    ((void) ((MS_ZONE_ERROR & MS_LOG_ZONES) ? (printf ("[MS][ERROR]"
    #define LOG_WARNING  ((void) ((MS_ZONE_WARNING & MS_LOG_ZONES) ? (printf ("[MS][WARNING]"
    #define LOG_INFO     ((void) ((MS_ZONE_INFO & MS_LOG_ZONES) ? (printf ("[MS][INFO]"
    #define LOG_DEBUG    ((void) ((MS_ZONE_DEBUG & MS_LOG_ZONES) ? (printf ("[MS][DEBUG]"
    #define LOG_ENTER    ((void) ((MS_ZONE_ENTER & MS_LOG_ZONES) ? (printf ("[MS][ENTER]"
    #define LOG_LEAVE    ((void) ((MS_ZONE_LEAVE & MS_LOG_ZONES) ? (printf ("[MS][LEAVE]"
    #define LOG_DATA     ((void) ((false) ? (printf (
    #define LOG_INFO2    ((void) ((false) ? (printf (
    #define LOG_END      )), 1 : 0));
#else
    #define LOG_ZONES
    #define LOG_ERROR
    #define LOG_WARNING
    #define LOG_INFO
    #define LOG_DEBUG
    #define LOG_ENTER
    #define LOG_LEAVE
    #define LOG_DATA
    #define LOG_INFO2
    #define LOG_END ;
#endif

#ifdef __cplusplus
}
#endif

#endif