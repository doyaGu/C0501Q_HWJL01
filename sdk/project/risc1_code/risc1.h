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
#ifndef __RISC1_H__
#define __RISC1_H__

#if defined(__FREERTOS__)
    #include "FreeRTOS.h"
    #include "task.h"
    #include "pal/timer.h"
    #include "pal/pal.h"
    #include "pal/file.h"
#endif

#include "risc1_defs.h"

/* NOTE: To support backwards compatibility, only add new functions at
         the end of the structure.  Every time you add a new function,
         remember to increase CODEC_API_VERSION.
 */
struct _risc_api {
    void* pData;
};

struct _risc1_header {
    unsigned long  magic;
    unsigned short target_id;
    unsigned short api_version;
    unsigned char  *load_addr;
    unsigned char  *end_addr;
    int            (*entry_point)(struct _risc_api *);
    int            (*risc_info)();
};

#endif // __RISC1_H__