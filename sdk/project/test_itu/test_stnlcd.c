#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/itu.h"

static ITUScene scene;

void* TestFunc(void* arg)
{
    itpInit();

    ituStnLcdInit();
    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/DroidSansMono.ttf", ITU_GLYPH_8BPP);
    //ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/et15.otb", ITU_GLYPH_1BPP);

    ituSceneInit(&scene, NULL);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/test.itu");

    for (;;)
    {
        if (ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0))
        {
            ioctl(ITP_DEVICE_STNLCD, ITP_IOCTL_CLEAR, NULL);
            ituSceneDraw(&scene, ituStnLcdGetDisplaySurface());
            ituFlip(ituStnLcdGetDisplaySurface());
        }
        usleep(33000);
    }

    return NULL;
}
