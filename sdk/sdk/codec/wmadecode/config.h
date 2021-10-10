/*
 * config.h
 * Copyright (C) 2004-2007 SMedia Tech Corp.
 *
 * Creator : Kuoping Hsu <kuoping@smediatech.com>
 * Date    : 2007/10/14.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__OR32__)
    #define __OR32__
#endif

#if defined(WIN32)
    #include <stdio.h>
    #define PRINTF // printf
    #define ASSERT // assert
#else
    #include "debug.h"
#endif

#ifndef MMP_INLINE
//#  define MMP_INLINE
#endif

//#define INPUT_MEMMODE
//#define OUTPUT_MEMMODE

/////////////////////////////////////////////////////////////////
// PC
/////////////////////////////////////////////////////////////////
#if defined(WIN32) || defined(__CYGWIN__)
    #include "win32.h"

//#    define __USE_INT64_LIB__         // Use __div64

/////////////////////////////////////////////////////////////////
// RISC
/////////////////////////////////////////////////////////////////
#elif defined(__OR32__)

    #include "mmio.h"
//#    include "engine.h"
//#    include "i2s.h"

//#    define __USE_INT64_LIB__         // Use __div64

    #ifndef __OR32_LITTLE_ENDIAN__
        #define WORDS_BIGENDIAN
    #endif

    #define USE_ENGINE

#else
    #error "No platform defined!"
#endif   // __OR32__
///////////////////////////////////////////////////////////////////////////
// End of Machine Config.
///////////////////////////////////////////////////////////////////////////

// For equalizer
#define EQUALIZER

// For Frequency Info
#define FREQINFO
#define FREQINFOCNT       20

#define WMA_LAG_OPTIMIZE  // Add by Viola Lee on 2009.02.16
//#define WMA_ADD_CONTENT_SWITCH
#define WMA_FORWARD_CODEC // Add by Viola Lee on 2009.02.26

//#define WMA_PERFORMANCE_TEST

//SMEDIA 230
//==================================================================
//#define SUPPORT_HW_FADING
//==================================================================

/////////////////////////////////////////////////////////////////
// Buffer Size
/////////////////////////////////////////////////////////////////
/******************************
   The Buffer size of input stream
******************************/
#define MAINBUF_SIZE   (1024 * 16)
#define READBUF_SIZE   (MAINBUF_SIZE * 3)
#define READBUF_BEGIN  (MAINBUF_SIZE)
#define READBUF_LEN    (READBUF_SIZE - READBUF_BEGIN)

/******************************
   The Buffer size of output PCM
******************************/
/* Output Buffer (PCM data) to I2S */
#define MAXFRAME       (MAX_CHANNELS * BLOCK_MAX_SIZE * sizeof(short))

#if defined(DUMP_PCM_DATA)      // Output to PCM stream
    #define I2SBUFSIZE (MAXFRAME * 6)
#elif defined(__FREERTOS__)     // RTOS project
    #define I2SBUFSIZE (MAXFRAME * 6)
#elif !defined(OUTPUT_MEMMODE)  // Output to I2S
    #define I2SBUFSIZE (MAXFRAME * 6)
#else                           // Output to memory buffer
    #define I2SBUFSIZE (MAXFRAME * 10)
#endif

#include "statname.h"
#include "wmadec.h"

#endif // __CONFIG_H__