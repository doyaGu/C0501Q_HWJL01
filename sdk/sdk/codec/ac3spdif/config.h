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
//#  define MAXFRAME (OUTPUT_CHANNELS * OUTPUT_SAMPLES * sizeof(short))
#define MAXFRAME (6144)

#  if defined(DUMP_PCM_DATA)    // Output to PCM stream
#    define I2SBUFSIZE (MAXFRAME * 6)
#  elif !defined(OUTPUT_MEMMODE)// Output to I2S
#    define I2SBUFSIZE (MAXFRAME * 6)
#  else                         // Output to memory buffer
#    define I2SBUFSIZE (MAXFRAME * 10)
#  endif

#endif /* __CONFIG_H__ */

