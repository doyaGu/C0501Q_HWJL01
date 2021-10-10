#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/itu.h"

static ITUScene scene;

static uint8_t helloworldArray[] =
{
#include "helloworld.inc"
};

void* TestFunc(void* arg)
{
    itpInit();
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);

    ituLcdInit();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/DroidSansMono.ttf", ITU_GLYPH_8BPP);

    ituSceneInit(&scene, NULL);
    ituSceneLoadArray(&scene, helloworldArray, sizeof (helloworldArray));

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
