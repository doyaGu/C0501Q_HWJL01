/**************************************************************************************
 * File name:   config.h
 *
 * Description: Header file 
 *
 *
 *
 *
 *
 *
 **************************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__


#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__OR32__)
#  define __OR32__
#endif

#include "debug.h"

#if defined(WIN32) || defined(__CYGWIN__)
#  include "win32.h"
#elif defined(__OR32__)
#  include "mmio.h"
//#  include "sys.h"
//#  include "or32.h"
//#  include "engine.h"
#  include "i2s.h"
#endif // defined(__OR32__)

#ifndef MMP_INLINE
//#define MMP_INLINE
#endif

// Enable SBC Encoding
#if defined(USE_SBC)
#define ENABLE_SBC
#endif


// Force find sync work when it fill-in the read buffer (default is undef).
#define FORCE_FINDSYNCWORD_ON_FILLBUFFER

// Define the new extension header to add PTS information.
// It's not a standard, adds by kuoping to support the streaming of Mobile TV.
//#define ADTS_EXTENSION_FOR_PTS

// Name mangling macros for static linking
//#define USE_STATNAME

#define AAC_INTERNAL_SRAM_FLOW

// Use raw block
//#define RAW_BLOCK

  //#define AAC_FLOW_CONTROL

// aac pts for tv
#define AAC_INVALID_PTS_OVER_FLOW_VALUE     (0xFFFFFFFF)
// Use MMIO to sync the PTS information
#define USE_PTS_EXTENSION
#define MMIO_PTS_WRIDX  (0x16ae)
#define MMIO_PTS_HI     (0x16b2)
#define MMIO_PTS_LO     (0x16b0)
#define AAC_HI_BIT_OVER_FLOW_THRESHOLD      (0x3E8)
#define AAC_INVALID_PTS_OVER_FLOW_VALUE     (0xFFFFFFFF)
#define AAC_WRAP_AROUND_THRESHOLD           (0x3E80000) // 65536 seconds
#define AAC_JUDGE_MAXIMUM_GAP               (0x1F40000) // 36728 seconds

///////////////////////////////////////////////////////////////////////////
// Sound Effect Configuration
//
// Notice: To compare the data between RISC and PC verison, it should be turn
//         off the REVERBERATION and DRCTL.
//
///////////////////////////////////////////////////////////////////////////
//Equalizer related macro
//#define EQUALIZER

// Reverberation related macro
//#define REVERBERATION

// DRC related macro
//#define DRCTL

// Frequency info related macro
//#define FREQINFO
#define FREQINFOCNT   20

// Voice OFF, Two mode of voice off, one is simple voice off,
//  one is filtered voice off
//#define VOICEOFF
//#define SIMPLE_VOICEOFF
#define FILTER_VOICEOFF

//#define AAC_SUPPORT_MULTICHANNELS

// AAC_SBR Configuration, will cause aacdec.h to define AAC_ENABLE_SBR
//#define HELIX_FEATURE_AUDIO_CODEC_AAC_SBR     // Helix AAC Plus Decoder

//#define LATM_TO_ADTS

//#define SUPPORT_MORE_THAN_2_CHANNELS
#ifndef SUPPORT_MORE_THAN_2_CHANNELS
  #define ENABLE_DOWNMIX_CHANNELS
#endif
///////////////////////////////////////////////////////////////////////////
// WIN32
///////////////////////////////////////////////////////////////////////////
#if defined(WIN32) || defined(__CYGWIN__) // For PC

//#define INPUT_MEMMODE
//#define OUTPUT_MEMMODE

//#define ORIG
//#define OR32_EXPAND

///////////////////////////////////////////////////////////////////////////
// OpenRISC
///////////////////////////////////////////////////////////////////////////
#else // For OpenRISC

//#define INPUT_MEMMODE
//#define OUTPUT_MEMMODE

// OR32_ASM & OR32_EXPAND is exclusive defination
#define OR32_ASM

//#define REVERB_DYNAMIC_MALLOC

 //#define AAC_DUMP_PCM

//#define AAC_PERFORMANCE_TEST

//#define AAC_PERFORMANCE_TEST_BY_TICK

#define AAC_ENABLE_INTERNAL_SD 
#if defined (CFG_CHIP_REV_A0)
#define INTERNAL_SD 0xC0200000
#else
    #define INTERNAL_SD 0xC0000000
#endif

//Recompense audio time
//#define AAC_RECOMPENSE_TIME
  // #define AAC_RESET_DECODED_BYTE
#endif
///////////////////////////////////////////////////////////////////////////
// Buffer size
///////////////////////////////////////////////////////////////////////////

// it should include aacdec.h to define AAC_ENABLE_SBR before use it.
#include "aacdec.h"

/*
 The output buffer size.
 */
#if defined(HELIX_FEATURE_AUDIO_CODEC_AAC_SBR)
#define SBR_MUL     2
#else
#define SBR_MUL     1
#endif // AAC_ENABLE_SBR

/******************************
 The Buffer size of output PCM
 ******************************/
/* Output Buffer (PCM data) to I2S */
#define MAXFRAME  (AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL * 2)

#if defined(DUMP_PCM_DATA)      // Output to PCM stream
#  define I2SBUFSIZE (MAXFRAME * 6)
#elif defined(__FREERTOS__)
  #ifdef AAC_SUPPORT_MULTICHANNELS
    #define I2SBUFSIZE (MAXFRAME * 3)
   #else
    #define I2SBUFSIZE (MAXFRAME * 4)
   #endif
#elif !defined(OUTPUT_MEMMODE)  // Output to I2S
#  define I2SBUFSIZE (MAXFRAME * 4 *2)
#else                           // Output to memory buffer
#  define I2SBUFSIZE (MAXFRAME * 10)
#endif

/******************************
 The Buffer size of input stream
 ******************************/
#if defined(__FREERTOS__)
    #ifdef AAC_SUPPORT_MULTICHANNELS
        #define READBUF_SIZE      (14 * AAC_MAINBUF_SIZE )
    #else
        #define READBUF_SIZE      (32 * AAC_MAINBUF_SIZE )
    #endif
#else
#define READBUF_SIZE      (14 * 2 *AAC_MAINBUF_SIZE )
#endif
#define READBUF_GUARDSIZE AAC_MAINBUF_SIZE
#define READBUF_BEGIN     READBUF_GUARDSIZE
#define READBUF_GUARD     (READBUF_SIZE - READBUF_GUARDSIZE)
///////////////////////////////////////////////////////////////////////////
// HE-AAC V2
///////////////////////////////////////////////////////////////////////////
//#ifdef HAVE_HEAACV2
#define PARSING_HE_AAC_V2
//endif

///////////////////////////////////////////////////////////////////////////
// Type defined
///////////////////////////////////////////////////////////////////////////

typedef unsigned long DWORD;
typedef unsigned char BOOL;
typedef long          INT32;

#if defined(REVERBERATION) || defined(DRCTL)
typedef int   PCM_WORD;
#else
typedef short PCM_WORD;
#endif

#endif /* __CONFIG_H__ */

