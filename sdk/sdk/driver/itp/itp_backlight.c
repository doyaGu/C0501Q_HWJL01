/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Backlight functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <unistd.h>
#include "itp_cfg.h"

#define BL_GPIO_PIN CFG_GPIO_BACKLIGHT_PWM
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
#define GPIO_MODE ITH_GPIO_MODE3
#define BL_PWM ITH_PWM1
#else
#define GPIO_MODE ITH_GPIO_MODE2
#define BL_PWM ITH_PWM5
#endif

static const unsigned int blDutyCycleTable[] = { CFG_BACKLIGHT_DUTY_CYCLES };
static unsigned int blLastValue = CFG_BACKLIGHT_DEFAULT_DUTY_CYCLE;

static int BacklightIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        ithPwmInit(BL_PWM, CFG_BACKLIGHT_FREQ, CFG_BACKLIGHT_DEFAULT_DUTY_CYCLE);
        break;

    case ITP_IOCTL_RESET:
        ithPwmReset(BL_PWM, BL_GPIO_PIN, GPIO_MODE);
        break;

    case ITP_IOCTL_ON:
        ithPwmEnable(BL_PWM, BL_GPIO_PIN, GPIO_MODE);
        break;

    case ITP_IOCTL_OFF:
        ithPwmDisable(BL_PWM, BL_GPIO_PIN);
        break;

    case ITP_IOCTL_GET_MAX_LEVEL:
        return ITH_COUNT_OF(blDutyCycleTable) - 1;
        break;

    case ITP_IOCTL_SET_BRIGHTNESS:
    {
        int value = (int) ptr;
        if (value < ITH_COUNT_OF(blDutyCycleTable))
        {
            blLastValue = blDutyCycleTable[value];
            ithPwmSetDutyCycle(BL_PWM, blLastValue);
        }
    }
        break;

    default:
        errno = (ITP_DEVICE_BACKLIGHT << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceBacklight =
{
    ":backlight",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    BacklightIoctl,
    NULL
};
