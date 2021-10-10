/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor platform abstraction layer configurations.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef ITP_CFG_H
#define ITP_CFG_H

// include paths
#include "ite/ith.h"
#include "ite/itp.h"
#include <stdio.h>
#define PRINTF printf

// Debug definition
#if !defined(NDEBUG) && !defined(DEBUG)
    #define DEBUG
#endif

#ifdef DEBUG
    #define ASSERT(e) ((e) ? (void) 0 : ithAssertFail(#e, __FILE__, __LINE__, __FUNCTION__))
#else
    #define ASSERT(e) ((void) 0)
#endif

/* Log fucntions definition */
#define STRINGIFY(x)    #x
#define TOSTRING(x)     STRINGIFY(x)

#define LOG_PREFIX   __FILE__ ":" TOSTRING(__LINE__) ": "

#ifdef CFG_ITP_ERR
    #define LOG_ERR     PRINTF("ERR:" LOG_PREFIX
#else
    #define LOG_ERR     (void)(1 ? 0 :
#endif

#ifdef CFG_ITP_WARN
    #define LOG_WARN    PRINTF("WARN:" LOG_PREFIX
#else
    #define LOG_WARN    (void)(1 ? 0 :
#endif

#ifdef CFG_ITP_INFO
    #define LOG_INFO    PRINTF("INFO:"
#else
    #define LOG_INFO    (void)(1 ? 0 :
#endif

#ifdef CFG_ITP_DBG
    #define LOG_DBG     PRINTF("DBG:"
#else
    #define LOG_DBG     (void)(1 ? 0 :
#endif

#define LOG_END         );

// UART
//#define ITP_UART_DMA
//#define ITP_RS485_DMA

// FAT
#define ITP_FAT_UTF8

// OSD CONSOLE
#define ITP_OSD_TIMEOUT_COUNT 10000

#endif // ITP_CFG_H
