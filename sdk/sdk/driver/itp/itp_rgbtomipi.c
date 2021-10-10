/*
 * Copyright (c) 2016 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL RGB To MIPI functions.
 *
 * @author Benson Lin
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

static int RGBtoMIPIIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
		//printf("RGBtoMIPIIoctl ITP_IOCTL_INIT\n");
        itpRGBToMIPIInit();
        break;

    default:
        errno = (ITP_DEVICE_RTC << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceRGBtoMIPI =
{
    ":RGBTOMIPI",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    RGBtoMIPIIoctl,
    NULL
};
