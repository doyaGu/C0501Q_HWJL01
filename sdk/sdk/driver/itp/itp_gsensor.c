/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL G-Sensor functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

static int gsLastValue = -1;

static int GSensorRead(int file, char *ptr, int len, void* info)
{
    int value = itpGSensorGetValue();

    if (len > 0 && gsLastValue != value)
    {
        *ptr = (char)value;
        gsLastValue = value;
        return sizeof (char);
    }
    return 0;
}

static int GSensorIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        itpGSensorInit();
        break;

    default:
        errno = (ITP_DEVICE_GSENSOR << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceGSensor =
{
    ":gsensor",
    itpOpenDefault,
    itpCloseDefault,
    GSensorRead,
    itpWriteDefault,
    itpLseekDefault,
    GSensorIoctl,
    NULL
};
