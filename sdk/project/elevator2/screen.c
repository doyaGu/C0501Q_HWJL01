#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "elevator.h"

static bool screenOff;

static float screenSaverCountDown;
static uint32_t screenSaverLastTick;

void ScreenInit(void)
{
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_SET_BRIGHTNESS, (void*)theConfig.brightness);
    screenSaverLastTick = SDL_GetTicks();
    screenSaverCountDown = theConfig.screensaver_time * 60.0f;
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

void ScreenSaverRefresh(void)
{
    screenSaverLastTick = SDL_GetTicks();
    screenSaverCountDown = theConfig.screensaver_time * 60.0f;

    if (screenOff && theConfig.screensaver_type == SCREENSAVER_BLANK)
    {
        ScreenOn();
    }
}

int ScreenSaverCheck(void)
{
    uint32_t diff, tick = SDL_GetTicks();

    if (tick >= screenSaverLastTick)
    {
        diff = tick - screenSaverLastTick;
    }
    else
    {
        diff = 0xFFFFFFFF - screenSaverLastTick + tick;
    }

    //printf("ScreenSaverCheck: tick: %d diff: %d countdown: %d\n", tick, diff, (int)screenSaverCountDown);

    if (diff >= 1000)
    {
        screenSaverCountDown -= (float)diff / 1000.0f;
        screenSaverLastTick = tick;

        if (screenSaverCountDown <= 0.0f)
            return -1;
    }
    return 0;
}
bool ScreenSaverIsScreenSaving(void)
{
    return screenOff || (screenSaverCountDown <= 0.0f);
}
