/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Tuner functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

static void TunerInit(void)
{
    ithGpioSetOut(CFG_GPIO_TUNER_ENABLE);
    ithGpioSet(CFG_GPIO_TUNER_ENABLE);
    ithGpioEnable(CFG_GPIO_TUNER_ENABLE);
}

static void TunerOn(void)
{
    ithGpioSet(CFG_GPIO_TUNER_ENABLE);
}

static void TunerOff(void)
{
    ithGpioClear(CFG_GPIO_TUNER_ENABLE);
}

static int TunerIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        TunerInit();
        break;

    case ITP_IOCTL_ON:
        TunerOn();
        break;

    case ITP_IOCTL_OFF:
        TunerOn();
        break;

    default:
        errno = (ITP_DEVICE_TUNER << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceTuner =
{
    ":tuner",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    TunerIoctl,
    NULL
};
