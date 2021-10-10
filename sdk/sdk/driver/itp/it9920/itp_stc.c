/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL STC functions.
 *
 * @author Evan Chang
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"


#define MACRO_RETURN_ERR                                            \
    {                                                               \
        errno = (ITP_DEVICE_STC << ITP_DEVICE_ERRNO_BIT) | EPERM;   \
        return -1;                                                  \
    }

extern STCInfo;// stc_info[];

extern void ithStcUpdateBaseClock(void);

static void StcIntrHandler(void* arg)
{
}

static int StcOpen(const char* name, int flags, int mode, void* info)
{
    int index;
    return index;
}

static int StcClose(int file, void* info)
{
    int index = file;
    return 0;
}

static int StcRead(int file, char *ptr, int len, void* info)
{
    int index = file;	
    return sizeof(uint64_t);
}

static int StcWrite(int file, char *ptr, int len, void* info)
{
    uint64_t curr, offset;
    int index = file;

    if ((index < 0) || (STC_MAX_CNT <= index))
        MACRO_RETURN_ERR;
    return sizeof(uint64_t);
}

static void StcInit(void)
{
}

static int StcIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int index = file;
    if ((index < 0) || (STC_MAX_CNT <= index))
        MACRO_RETURN_ERR;

    switch (request)
    {
        case ITP_IOCTL_INIT:
            StcInit();
            break;

        case ITP_IOCTL_PAUSE:
            {
            }

        default:
            errno = (ITP_DEVICE_STC << ITP_DEVICE_ERRNO_BIT) | 1;
            return -1;
    }
    return 0;
}

const ITPDevice itpDeviceStc =
{
    ":stc",
    StcOpen,
    StcClose,
    StcRead,
    StcWrite,
    itpLseekDefault,
    StcIoctl,
    NULL
};
