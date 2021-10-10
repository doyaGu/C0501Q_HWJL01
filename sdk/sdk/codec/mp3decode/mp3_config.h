#ifndef __CONFIG_H__
#define __CONFIG_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__OR32__)
#  define __OR32__
#endif

#if defined(WIN32) || defined(__CYGWIN__)
#  include "win32.h"
#elif defined(__OR32__)
//#  include "or32.h"
#endif

#include "debug.h" // It defines the PRINTF and ASSERT macro.


//#define LAYERII_DEBUG
// Enable MIXER
#if defined(USE_MIXER)
#  define ENABLE_MIXER
#endif

// Enable SBC Encoding
#if defined(USE_SBC)
#  if defined(USE_MIXER)
#    error "Can not enable Mixer when use SBC encoding."
#  endif
#  define ENABLE_SBC
#  include "sbc.h"
#endif

// Use MMIO to sync the PTS information
#define USE_PTS_EXTENSION
#define MMIO_PTS_WRIDX  (0x16ae)
#define MMIO_PTS_HI     (0x16b2)
#define MMIO_PTS_LO     (0x16b0)

// Force find sync work when it fill-in the read buffer (default is undef).
// undef this symbol will
// 1. For I2S mode, it will NOT find the sync word when it fills the read buffer.
// 2. For PCM mode, it will find the sync word when it fills the read buffer.
// Notes: it should undef it if the ID3 header is bigger than the size of
// stream buffer, the decoder will decode the wrong data.
#define FORCE_FINDSYNCWORD_ON_FILLBUFFER

// Mode Select, exclusive option
// 1. MANUFACTURE_MEMMODE -- Input mp3 stream from array and output to memory for manufature test.
// 1. MANUFACTURE_I2SMODE -- Input mp3 stream from array and output to I2S for manufature test.
// 2. BOOTUP_MODE      -- Input mp3 stream from end of code, it used to play the MP3 on mobile handset booting.
// 3. NORMAL_MODE      -- Normal mode

//#define MANUFACTURE_MEMMODE // For manufacture testing mode
//#define MANUFACTURE_I2SMODE // For manufacture testing mode
//#define BOOTUP_MODE
  #define NORMAL_MODE

  //#define MP3_FLOW_CONTROL
  //#define MP3_PERFORMANCE_TEST

  //#define MP3_PERFORMANCE_TEST_BY_TICK
  // no output to test decoding performace
  //#define MP3_NO_OUTPUT_TEST_PERFORMANCE

//  #define MP3_RESET_DECODED_BYTE  
///////////////////////////////////////////////////////////////////////////
// For Win32
///////////////////////////////////////////////////////////////////////////
#if defined(WIN32) || defined(__CYGWIN__)

#if defined(NORMAL_MODE) // Normal Configuration

  #define INPUT_FILEMODE
//#define INPUT_MEMMODE
//#define OUTPUT_MEMMODE

#elif defined(MANUFACTURE_MEMMODE) || defined(MANUFACTURE_I2SMODE) // For manufacture

#if defined(MANUFACTURE_I2SMODE)
#error "No support manufacture i2s mode on Win32"
#else
//#define INPUT_FILEMODE
  #define INPUT_MEMMODE
  #define OUTPUT_MEMMODE
#endif // defined(MANUFACTURE_I2SMODE)

#elif defined(BOOTUP_MODE)
#error "No support bootup mode on win32"
#else
#error "Mode select not correct."
#endif // MANUFACTURE_MODE

///////////////////////////////////////////////////////////////////////////
// For OpenRISC
///////////////////////////////////////////////////////////////////////////
#elif defined(__OR32__)

// Set ENABLE_SET_FRAME to report frame number instead of frame time.
//#if defined(__FREERTOS__) // || defined(API_V2)
//  #define ENABLE_SET_FRAME
//#endif

#if defined(NORMAL_MODE) // Normal Configuration

  #define INPUT_FILEMODE
//#define INPUT_MEMMODE
  #define OUTPUT_I2SMODE
//#define OUTPUT_MEMMODE

#elif defined(MANUFACTURE_MEMMODE) || defined(MANUFACTURE_I2SMODE) // For manufacture

  #define MANUFACTURE

#if defined(MANUFACTURE_I2SMODE)
//#define INPUT_FILEMODE
  #define INPUT_MEMMODE
  #define OUTPUT_I2SMODE
//#define OUTPUT_MEMMODE
#else
//#define INPUT_FILEMODE
  #define INPUT_MEMMODE
//#define OUTPUT_I2SMODE
  #define OUTPUT_MEMMODE
#endif // defined(MANUFACTURE_I2SMODE)

#elif defined(BOOTUP_MODE)

//#define INPUT_FILEMODE
  #define INPUT_MEMMODE
  #define OUTPUT_I2SMODE
//#define OUTPUT_MEMMODE

#else
#error "Mode select not correct."
#endif // MANUFACTURE_MODE

// Set the repeat play on the INPUT_MEMMODE
//#if defined(INPUT_MEMMODE) && !defined(MANUFACTURE) && !defined(BOOTUP_MODE)
//#define REPEAT_PLAY
//#endif // INPUT_MEMMODE

#define SAT_MODE    0
//#define ENABLE_PERFORMANCE_MEASURE

#endif
///////////////////////////////////////////////////////////////////////////
// End of Machine Config.
///////////////////////////////////////////////////////////////////////////

//#define EQUALIZER
//#define DRC

 //#define MP3_DUMP_PCM

// For Frequency Info
#define FREQINFO
#define FREQINFOCNT     20
#define FREQINFOARRAY 5

// For Voice removal
// Two mode of voice off, one is simple voice off, one is filtered voice off
//#define VOICEOFF
//#define SIMPLE_VOICEOFF
#define FILTER_VOICEOFF

// For reverberation
//#define REVERBERATION
#define REVERB_DYNAMIC_MALLOC
#define MAX_FRAMEDELAY 4        // Max Frame delay

///////////////////////////////////////////////////////////////////////////
// Buffer Size
///////////////////////////////////////////////////////////////////////////
/* determining MAINBUF_SIZE:
 *   max mainDataBegin = (2^9 - 1) bytes (since 9-bit offset) = 511
 *   max nSlots (concatenated with mainDataBegin bytes from before) = 1440 - 9 - 4 + 1 = 1428
 *   511 + 1428 = 1939, round up to 1940 (4-byte align)
 */
#define MAINBUF_SIZE    1940

#define MAX_NGRAN       2       /* max granules */
#define MAX_NCHAN       2       /* max channels */
#define MAX_NSAMP       576     /* max samples per channel, per granule */
#define MAX_FRAMESIZE   MAX_NGRAN * MAX_NSAMP   // max samples per channel //

/******************************
 The Buffer size of output PCM
 ******************************/
#define MAX_FRAMEBYTES  (sizeof(short) * MAX_NCHAN * MAX_FRAMESIZE)
#if defined(DUMP_PCM_DATA)      // Output to PCM stream
#  define I2SBUFSIZE (MAX_FRAMEBYTES * 6)
#elif defined(ENABLE_MIXER)     // Output to MIXER buffer
#  define I2SBUFSIZE (MAX_FRAMEBYTES * 3)
#elif !defined(OUTPUT_MEMMODE)  // Output to I2S
#  define I2SBUFSIZE (MAX_FRAMEBYTES * 6)
#else                           // Output to memory buffer
#  define I2SBUFSIZE (MAX_FRAMEBYTES * 14)
#endif

/******************************
 The Buffer size of input stream
 ******************************/
#if defined(__FREERTOS__)
#ifdef ENABLE_XCPU_MSGQ
#define READBUF_SIZE        (1024*56)
#else
#define READBUF_SIZE        (1024*16)
#endif
#else
#define READBUF_SIZE        (1024*16)
#endif

#include "coder.h"

#endif  /* __CONFIG_H__ */

