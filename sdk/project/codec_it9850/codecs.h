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
#ifndef __CODECS_H__
#define __CODECS_H__

#define ENABLE_AUDIO_PROCESSOR

#if defined(__FREERTOS__)
    #include "FreeRTOS.h"
    #include "task.h"
    #include "pal/timer.h"
    #include "pal/pal.h"
    #include "pal/file.h"
#endif

#include "codecs_defs.h"

/* NOTE: To support backwards compatibility, only add new functions at
         the end of the structure.  Every time you add a new function,
         remember to increase CODEC_API_VERSION.
 */
struct _codec_api {
    /* global variable from DPF AP */
    void *eqinfo;
    void *revbinfo;
};

struct _codec_header {
    unsigned long  magic;
    unsigned short target_id;
    unsigned short api_version;
    unsigned char  *load_addr;
    unsigned char  *end_addr;
    int            (*entry_point)(struct _codec_api *);
    int            (*codec_info)();
};

#endif // __CODECS_H__