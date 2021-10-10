#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"

void* TestFunc(void* arg)
{
    uint16_t* addr = (uint16_t*) ithLcdGetBaseAddrA();
    uint32_t col, row, x, y, i = 0;
    const uint16_t colors[] = { ITH_RGB565(255, 0, 0), ITH_RGB565(0, 255, 0), ITH_RGB565(0, 0, 255) };
    bool on;
    int maxLevel, brightness = 0;

    itpInit();

    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
    on = true;
    maxLevel = ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_GET_MAX_LEVEL, NULL);

    addr = (uint16_t*) ithLcdGetBaseAddrA();
    col = ithLcdGetPitch() / 2;
    row = ithLcdGetHeight();

    for (;;)
    {
        uint16_t* base = ithMapVram((uint32_t) addr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_WRITE);
				uint16_t color = colors[i++ % ITH_COUNT_OF(colors)];
        uint16_t* ptr = base;

        if (on)
        {
            ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_OFF, NULL);
            on = false;
        }
        else
        {
            ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);
            on = true;
        }

        if (++brightness > maxLevel)
            brightness = 0;

        ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_SET_BRIGHTNESS, (void*)brightness);

        for (y = 0; y < row; y++)
            for (x = 0; x < col; x++)
                *ptr++ = color;

        ithFlushDCacheRange(base, row * col * 2);
        sleep(1);
    }
    return NULL;
}
