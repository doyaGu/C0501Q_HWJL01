#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "sports_equipment.h"
#include "scene.h"

int SDL_main(int argc, char *argv[])
{
    int ret = 0;

    ret = UpgradeInit();
    if (ret)
        goto end;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());

    ConfigInit();
    ScreenInit();
    ExternalInit();
    AudioInit();
    StorageInit();

    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif

    ProgramInit();

    SceneInit();
    SceneLoad();
    ret = SceneRun();

    SceneExit();

    AudioExit();
    ExternalExit();
    ConfigExit();
    SDL_Quit();

#if 0
    {
        static signed char buf[2048];
        vTaskList(buf);
        puts(buf);

    #ifdef CFG_DBG_RMALLOC
        Rmalloc_stat(__FILE__);
    #endif
    }
#endif

end:
    ret = UpgradeProcess(ret);
    exit(ret);
    return ret;
}
