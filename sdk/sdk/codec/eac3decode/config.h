/*
 * config.h
 * Copyright (C) 2004-2007 Kuoping Hsu <kuoping@smediatech.com>
 *
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__OR32__)
#  define __OR32__
#endif

#include "debug.h"            // it defines the PRINT and ASSERT macro.

#ifndef MMP_INLINE
//#  define MMP_INLINE
#endif

// Use MMIO to sync the PTS information
#define USE_PTS_EXTENSION
#define MMIO_PTS_WRIDX  (0x16ae)
#define MMIO_PTS_HI     (0x16b2)
#define MMIO_PTS_LO     (0x16b0)

//#define AC3_RESET_DECODED_BYTE
//#define EAC3_PERFORMANCE_TEST_BY_TICK

//#define AC3_DUMP_PCM

#define EAC3_ACCEPT_PARAMETER
#define EAC3_MEMORY_COPY_TO_I2S
/////////////////////////////////////////////////////////////////
// PC
/////////////////////////////////////////////////////////////////
#  if defined(WIN32) || defined(__CYGWIN__)
#    include "win32.h"

/////////////////////////////////////////////////////////////////
// OpenRISC
/////////////////////////////////////////////////////////////////
#  elif defined(__OR32__)

#include "mmio.h"
#include "i2s.h"

//#  define __USE_INT64_LIB__   // Use div64 library
#    define USE_ENGINE
#  endif                        //__OR32__

/////////////////////////////////////////////////////////////////
// Buffer Size
/////////////////////////////////////////////////////////////////
#  define MAINBUF_SIZE    (640*8)  // 640 is the maximun bitrate.
#  define OUTPUT_CHANNELS 2  //6
#  define OUTPUT_SAMPLES  (256*6)

/******************************
 The Buffer size of input stream
 ******************************/
#  define READBUF_SIZE      (MAINBUF_SIZE * 8)//(MAINBUF_SIZE * 12)
#  define READBUF_BEGIN     (MAINBUF_SIZE)
#  define READBUF_LEN       (READBUF_SIZE - READBUF_BEGIN)

/******************************
 The Buffer size of output PCM
 ******************************/
/* Output Buffer (PCM data) to I2S */
#  define MAXFRAME (OUTPUT_CHANNELS * OUTPUT_SAMPLES * sizeof(short))

#  if defined(DUMP_PCM_DATA)    // Output to PCM stream
#    define I2SBUFSIZE (MAXFRAME * 6)
#  elif !defined(OUTPUT_MEMMODE)// Output to I2S
#    define I2SBUFSIZE (MAXFRAME * 8) 
#  else                         // Output to memory buffer
#    define I2SBUFSIZE (MAXFRAME * 10)
#  endif

#endif /* __CONFIG_H__ */

