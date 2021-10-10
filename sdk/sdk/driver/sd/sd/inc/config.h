/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as compile option header file.
 *
 * @author Irene Lin
 */
#ifndef SD_CONFIG_H
#define SD_CONFIG_H

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
#include "ite/ite_sd.h"
#include "sd_error.h"
#if defined(__OPENRTOS__)
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/semphr.h"
#endif

#if !defined(FPGA)
//#define FPGA
#endif

//=============================================================================
//                              Compile Option
//=============================================================================
#define SD_WIN32_DMA
//#define MMC_WR_TIMING
#define SD_READ_FLIP_FLOP

#if defined(__OPENRTOS__)
    #define SD_IRQ_ENABLE
//#define SD_DETECT_IRQ
    #undef SD_WIN32_DMA
#endif

#if (CFG_CHIP_FAMILY == 9910 || CFG_CHIP_FAMILY == 9850)
    #define SD_NEW_HW
#endif

//#define SD_DUMP_CSD
//#define SD_DUMP_CID
//#define SD_DUMP_SWITCH_FUN
//#define SD_DUMP_EXT_CSD

//=============================================================================
//                              Constant Definition
//=============================================================================
#if 0
    #define SD_CLK_NORMAL_INIT 4    // Default mode : 100/5 = 20 MHz
    #define SD_CLK_NORMAL      4    // Default mode : 100/5 = 20 MHz
    #define SD_CLK_HIGH_SPEED  2    // High Speed : 100/3 = 33.33 MHz
#endif

    #define SD_CLK_NORMAL_INIT 4    // Default mode : 80/5 = 16 MHz
    #define SD_CLK_NORMAL      3    // Default mode : 80/4 = 20 MHz
    #define SD_CLK_HIGH_SPEED  1    // High Speed   : 80/2 = 40 MHz


//=============================================================================
//                              LOG definition
//=============================================================================
// Log definitions
typedef enum
{
    SD_ZONE_ERROR   = (0x1 << 0),
    SD_ZONE_WARNING = (0x1 << 1),
    SD_ZONE_INFO    = (0x1 << 2),
    SD_ZONE_DEBUG   = (0x1 << 3),
    SD_ZONE_ENTER   = (0x1 << 4),
    SD_ZONE_LEAVE   = (0x1 << 5),
    SD_ZONE_ALL     = 0xFFFFFFFF
} SDLogZones;

#if 1
    #define SD_LOG_ZONES (SD_ZONE_ERROR | SD_ZONE_WARNING)
//#define SD_LOG_ZONES    (SD_BIT_ALL & ~SD_ZONE_ENTER & ~SD_ZONE_LEAVE & ~SD_ZONE_DEBUG &~SD_ZONE_INFO)
//#define SD_LOG_ZONES     0

    #define LOG_ERROR    ((void) ((SD_ZONE_ERROR & SD_LOG_ZONES) ? (printf ("[SD][ERROR]"
    #define LOG_WARNING  ((void) ((SD_ZONE_WARNING & SD_LOG_ZONES) ? (printf ("[SD][WARNING]"
    #define LOG_INFO     ((void) ((SD_ZONE_INFO & SD_LOG_ZONES) ? (printf ("[SD][INFO]"
    #define LOG_DEBUG    ((void) ((SD_ZONE_DEBUG & SD_LOG_ZONES) ? (printf ("[SD][DEBUG]"
    #define LOG_ENTER    ((void) ((SD_ZONE_ENTER & SD_LOG_ZONES) ? (printf ("[SD][ENTER]"
    #define LOG_LEAVE    ((void) ((SD_ZONE_LEAVE & SD_LOG_ZONES) ? (printf ("[SD][LEAVE]"
    #define LOG_DATA     ((void) ((true) ? (printf (
    #define LOG_INFO2    ((void) ((true) ? (printf (
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