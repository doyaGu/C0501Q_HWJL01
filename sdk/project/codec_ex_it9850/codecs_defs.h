/***************************************************************************
* Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
*
* @file
* Codecs Code
*
* @author Kuoping Hsu
* @version 1.0
*
***************************************************************************/
#ifndef __CODECS_DEFS_H__
#define __CODECS_DEFS_H__

/* Additional region for nand boot */
#if defined(HAVE_NANDBOOT)
    #if defined(DEBUG)
        #define BOOTSTRAP_SIZE 0x8000
    #else
        #define BOOTSTRAP_SIZE 0x2000
    #endif // defined(DEBUG)
#else
    #define BOOTSTRAP_SIZE     0x0
#endif

/* puts the plugin to array */
#define CODEC_ARRAY_SIZE       (180 * 1024)      // the maximun size of each plug-ins

/* the start address of CODEC plugin */
#define CODEC_START_ADDR       (0x1000 + BOOTSTRAP_SIZE)

/* magic for normal codecs */
#define CODEC_MAGIC            0x534D3020       // "SM0 "

/* increase this every time the api struct changes */
#define CODEC_API_VERSION      0x00000002

/* machine target, no used currently */
#define TARGET_ID              0x00000220

/* the maximun size of codec plug-ins */
#if defined(DEBUG)
    #define RISC1_SIZE         (900 * 1024)
#else
    #ifdef ENABLE_XCPU_MSGQ
        #define RISC1_SIZE     (900 * 1024)
    #else
        #if defined(HAVE_HEAACV2)
            #define RISC1_SIZE (900 * 1024)  // aac plugin takes >350KB.
        #elif defined(HAVE_WMA)
            #define RISC1_SIZE (900 * 1024)  // eac3 plugin takes ~350KB.
        #else
            #define RISC1_SIZE (1000 * 1024) // Others plugin take ~198KB.
        #endif
    #endif
#endif
#define RISC2_SIZE          (64 * 1024)
#endif // __CODECS_DEFS_H__