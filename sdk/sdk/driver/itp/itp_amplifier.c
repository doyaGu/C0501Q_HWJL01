/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Amplifier functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

static int AmplifierIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        itpAmplifierInit();
        break;

    case ITP_IOCTL_ENABLE:
        itpAmplifierEnable();
        break;

    case ITP_IOCTL_DISABLE:
        itpAmplifierDisable();
        break;

    case ITP_IOCTL_MUTE:
        itpAmplifierMute();
        break;

    case ITP_IOCTL_UNMUTE:
        itpAmplifierUnmute();
        break;

    default:
        errno = (ITP_DEVICE_AMPLIFIER << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceAmplifier =
{
    ":amplifier",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    AmplifierIoctl,
    NULL
};
