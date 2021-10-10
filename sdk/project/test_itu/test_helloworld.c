#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/itu.h"

static ITUScene scene;

void* TestFunc(void* arg)
{
    itpInit();

    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif

    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);

    ituLcdInit();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/DroidSansMono.ttf", ITU_GLYPH_8BPP);
    ituSceneInit(&scene, NULL);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/helloworld.itu");

    for (;;)
    {
        if (ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0))
        {
            ituSceneDraw(&scene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
        }
        usleep(33000);
    }

    return NULL;
}
