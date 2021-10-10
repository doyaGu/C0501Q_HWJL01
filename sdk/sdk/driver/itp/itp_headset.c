/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Headset functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

static int hsLastValue = -1;

static int HeadsetRead(int file, char *ptr, int len, void* info)
{
    int value = itpHeadsetGetValue();

    if (len > 0 && hsLastValue != value)
    {
        *ptr = (char)value;
        hsLastValue = value;
        return sizeof (char);
    }
    return 0;
}

static int HeadsetIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        itpHeadsetInit();
        break;

    default:
        errno = (ITP_DEVICE_HEADSET << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceHeadset =
{
    ":headset",
    itpOpenDefault,
    itpCloseDefault,
    HeadsetRead,
    itpWriteDefault,
    itpLseekDefault,
    HeadsetIoctl,
    NULL
};
