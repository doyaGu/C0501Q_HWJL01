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
#ifndef __RISC1_DEFS_H__
#define __RISC1_DEFS_H__

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
#define RISC1_ARRAY_SIZE       (180 * 1024)      // the maximun size of each plug-ins

/* the start address of CODEC plugin */
#define RISC1_START_ADDR       (0x1000 + BOOTSTRAP_SIZE)

/* magic for normal codecs */
#define RISC1_MAGIC            0x534D3020       // "SM0 "

/* increase this every time the api struct changes */
#define RISC1_API_VERSION      0x00000002

/* machine target, no used currently */
#define TARGET_ID              0x00000220

/* the maximun size of codec plug-ins */
#define RISC1_SIZE         (1000 * 1024)

#define RISC2_SIZE         (120 * 1024)
#endif // __RISC1_DEFS_H__