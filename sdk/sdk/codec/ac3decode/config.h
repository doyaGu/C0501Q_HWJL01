/*
 * config.h
 * Copyright (C) 2004-2007 Kuoping Hsu <kuoping@smediatech.com>
 *
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "debug.h"            // it defines the PRINT and ASSERT macro.

#ifndef MMP_INLINE
//#  define MMP_INLINE
#endif

#define LIBA52_FIXED
//#define LIBA52_DOUBLE
//#define LIBA52_FLOAT

// Use MMIO to sync the PTS information
#define USE_PTS_EXTENSION
#define MMIO_PTS_WRIDX  (0x16ae)
#define MMIO_PTS_HI     (0x16b2)
#define MMIO_PTS_LO     (0x16b0)

#define AC3_RESET_DECODED_BYTE
// Defined it if it uses the malloc instead of static memory.
//#define USE_MALLOC

//#define INPUT_MEMMODE
//#define OUTPUT_MEMMODE

/////////////////////////////////////////////////////////////////
// PC
/////////////////////////////////////////////////////////////////
#  if defined(WIN32) || defined(__CYGWIN__)
#    include "win32.h"

/////////////////////////////////////////////////////////////////
// OpenRISC
/////////////////////////////////////////////////////////////////
#  elif defined(__OR32__)

#    include "mmio.h"
#    include "engine.h"
#    include "i2s.h"

//#  define __USE_INT64_LIB__   // Use div64 library
#    define USE_ENGINE
#  endif                        //__OR32__

/////////////////////////////////////////////////////////////////
// Buffer Size
/////////////////////////////////////////////////////////////////
/******************************
 The Buffer size of input stream
 ******************************/
#  include "common.h"
#  define READBUF_SIZE      (MAINBUF_SIZE * 6)
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
#    define I2SBUFSIZE (MAXFRAME * 6)
#  else                         // Output to memory buffer
#    define I2SBUFSIZE (MAXFRAME * 10)
#  endif

#endif /* __CONFIG_H__ */

