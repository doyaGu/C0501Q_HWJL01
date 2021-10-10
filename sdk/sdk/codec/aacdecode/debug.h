/*
 * Copyright (c) 2006 ITE Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for debugging
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __DEBUG_H
#define __DEBUG_H

#if defined(__OR32__)
//#  include "spr_defs.h"
#endif

///////////////////////////////////////////////////////////////////////////
// PRINTF
///////////////////////////////////////////////////////////////////////////
#if !defined(__GNUC__)
    #if defined(__DEBUG__)
        #define PRINTF printf
    #else
        #define PRINTF
    #endif // defined(__DEBUG__)
#else
    #if defined(__DEBUG__)
        #include <stdio.h>
        #define PRINTF(...) printf(__VA_ARGS__)
    #else
        #define PRINTF(...)
    #endif // defined(__DEBUG__)
#endif     // !defined(__GNUC__)

///////////////////////////////////////////////////////////////////////////
// ASSERTION
///////////////////////////////////////////////////////////////////////////
#if defined(NOASSERT)
    #if defined(__DEBUG__)
        #define ASSERT(k) if (!(k)) printf("##ASSERT## %s: #%d: %s\n", __FILE__, __LINE__, #k);
    #else
        #define ASSERT(k)
    #endif // __DEBUG__
#else
    #if defined(__CYGWIN__)
        #include <assert.h>
        #define ASSERT(k)           assert(k)
    #elif defined(WIN32)
        #define ASSERT(x)           if (!(x)) __asm int 3;
    #elif defined(__OR32__)
        #if defined(__FREERTOS__)
            #include "FreeRTOS.h"
            #include "task.h"
            #define ASSERT(k)       //if(!(k)) taskSOFTWARE_BREAKPOINT();
        #else // !__FREERTOS__
            #define _int3()         ({ asm volatile ("l.trap 15"); 0; })
            #if defined(__DEBUG__)
                #define __ASSERT(k) printf("##ASSERT## %s: #%d: %s\n", __FILE__, __LINE__, #k);
            #else
                #define __ASSERT(k)
            #endif // __DEBUG__
            #define ASSERT(k)       if (!(k)) { __ASSERT(k); _int3(); }
        #endif
    #else
        #error "no compiler defined"
    #endif
#endif // defined(NOASSERT)

#endif // __DEBUG_H