/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor hardware abstraction layer configurations.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef ITH_CFG_H
#define ITH_CFG_H

// include paths
#if defined(ANDROID)    // Android
    #include "ith.h"
    #include <stdio.h>
    #define PRINTF printf
    
#elif defined(__LINUX_ARM_ARCH__)   // Linux kernel
    #include "mach/ith.h"
    #include <linux/kernel.h>
    #define PRINTF printk
    
#elif defined(CONFIG_ARM) // U-Boot
    #include "asm/arch/ith.h"
    #include "exports.h"
    #define PRINTF printf

#elif defined(_WIN32)    // Win32
    #include "ite/ith.h"
    #include <stdio.h>
    #define PRINTF printf

#else
    #include "ite/ith.h"
    #define PRINTF ithPrintf

#endif // defined(__LINUX_ARM_ARCH__)

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

#ifdef CFG_ITH_ERR
    #define LOG_ERR     PRINTF("ERR:" LOG_PREFIX
#else
    #define LOG_ERR     (void)(1 ? 0 :
#endif

#ifdef CFG_ITH_WARN
    #define LOG_WARN    PRINTF("WARN:" LOG_PREFIX
#else
    #define LOG_WARN    (void)(1 ? 0 :
#endif

#ifdef CFG_ITH_INFO
    #define LOG_INFO    PRINTF("INFO:"
#else
    #define LOG_INFO    (void)(1 ? 0 :
#endif

#ifdef CFG_ITH_DBG
    #define LOG_DBG     PRINTF("DBG:"
#else
    #define LOG_DBG     (void)(1 ? 0 :
#endif

#define LOG_END         );

// Command queue definitions
#define ITH_CMDQ_LOOP_TIMEOUT       30000

// Video memory definitions
#define ITH_VMEM_BESTFIT

#endif // ITH_CFG_H
