/*
 * Copyright (c) 2004 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Co-processor API functoin file.
 *      Date: 2015/11/19
 *
 */
#include <pthread.h>
#include <assert.h>

#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/audio.h"
#include "ite/ite_risc.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

//=============================================================================
//                              Macro Definition
//=============================================================================
//#define AUDIO_PLUGIN_MESSAGE_QUEUE // defined in def.cmake
//#ifdef DEBUG_MODE
//#define LOG_ZONES    (ITH_BIT_ALL & ~ITH_ZONE_ENTER & ~ITH_ZONE_LEAVE & ~ITH_ZONE_DEBUG)
////#define LOG_ZONES    (ITH_BIT_ALL & ~ITH_ZONE_ENTER & ~ITH_ZONE_LEAVE)
////#define LOG_ZONES    (ITH_BIT_ALL)
//
//#define LOG_ERROR   ((void) ((ITH_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][AUD][ERROR]"
//#define LOG_WARNING ((void) ((ITH_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][AUD][WARNING]"
//#define LOG_INFO    ((void) ((ITH_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][AUD][INFO]"
//#define LOG_DEBUG   ((void) ((ITH_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][AUD][DEBUG]"
//#define LOG_ENTER   ((void) ((ITH_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][AUD][ENTER]"
//#define LOG_LEAVE   ((void) ((ITH_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][AUD][LEAVE]"
//#define LOG_DATA    ((void) ((ITH_FALSE) ? (printf(
//#define LOG_END     )), 1 : 0));
//#else
#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_END ;

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Definition
//=============================================================================
__attribute__((used)) bool
ithCodecCommand(
    int command,
    int parameter0,
    int parameter1,
    int parameter2)
{
    return true;
}

int
ithCodecWiegandReadCard(
    int index,
    unsigned long long *card_id)
{
    return 0;
}

void
ithCodecPrintfWrite(
    char *string,
    int length)
{
}

int
ithCodecUartRead(
    char *string,
    int length)
{
    return 0;
}

void
ithCodecUartWrite(
    char *string,
    int length)
{
}

__attribute__((used))  void
ithCodecUartDbgWrite(
    char *string,
    int length)
{
    return 0;
}

void
ithCodecCtrlBoardWrite(
    uint8_t *data,
    int length)
{
}

void
ithCodecCtrlBoardRead(
    uint8_t *data,
    int length)
{
}

void
ithCodecHeartBeatRead(
    uint8_t *data,
    int length)
{
}

static void
RISC2_ResetProcessor(
    void)
{
}


static void
RISC2_FireProcessor(
    void)
{
}

static int32_t
RISC2_LoadEngineImage(ITE_RISC_ENGINE engine_type)
{
    return 0;
}

int32_t
iteRiscOpenEngine(
    ITE_RISC_ENGINE engine_type, uint32_t bootmode)
{
    return 0;
}

int32_t
iteRiscTerminateEngine(
    void)
{
    return 0;
}
