#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "sports_equipment.h"

static bool screenOff;

void ScreenInit(void)
{
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_SET_BRIGHTNESS, (void*)theConfig.brightness);
    screenOff = false;
}

bool ScreenIsOff(void)
{
    return screenOff;
}

void ScreenSetBrightness(int value)
{
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_SET_BRIGHTNESS, (void*)value);
}

int ScreenGetMaxBrightness(void)
{
    return ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_GET_MAX_LEVEL, NULL);
}

void ScreenOff(void)
{
    puts("Screen Off!");
#ifdef CFG_POWER_STANDBY
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_STANDBY, NULL);
#else
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
#endif
    screenOff = true;
}

void ScreenOn(void)
{
#ifdef CFG_POWER_STANDBY
    ioctl(ITP_DEVICE_POWER, ITP_IOCTL_RESUME, NULL);
#else
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
#endif
    screenOff = false;
}
