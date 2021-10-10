#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "airconditioner.h"

static bool screenOff;

static float screenSaverCountDown;
static uint32_t screenSaverLastTick;
static bool screenSaverPause;

void ScreenInit(void)
{
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_SET_BRIGHTNESS, (void*)theConfig.brightness);
    screenSaverLastTick = SDL_GetTicks();
    screenSaverCountDown = theConfig.screensaver_time;
    screenOff = false;
    screenSaverPause = false;
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
    screenSaverCountDown = theConfig.screensaver_time;

    if (screenOff)
    {
        ScreenOn();
    }
}

int ScreenSaverCheck(void)
{
    uint32_t diff, tick = SDL_GetTicks();

    if (screenSaverPause)
    {
        screenSaverLastTick = tick;
        return 0;
    }

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

void ScreenSaverPause(void)
{
    screenSaverPause = true;
}

void ScreenSaverContinue(void)
{
    screenSaverPause = false;
}

bool ScreenSaverIsScreenSaving(void)
{
    return screenOff || (screenSaverCountDown <= 0.0f);
}
