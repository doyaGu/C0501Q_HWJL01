#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "ctrlboard.h"
#include "scene.h"

int SDL_main(int argc, char *argv[])
{
    int ret = 0;
    int restryCount = 0;
    
#ifdef CFG_LCD_MULTIPLE
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, (void*)0);
#endif

#if !defined(CFG_LCD_INIT_ON_BOOTING) && !defined(CFG_BL_SHOW_LOGO)
    ScreenClear();
#endif

#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
    BackupInit();
retry_backup:
    ret = UpgradeInit();
    if (ret)
    {
        if (++restryCount > 2)
        {
            printf("cannot recover from backup....\n");
            goto end;
        }
        BackupRestore();
        goto retry_backup;
    }
    BackupSyncFile();
#else
    ret = UpgradeInit();
    if (ret)
        goto end;
#endif

#ifdef CFG_LCD_MULTIPLE
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    usleep(100000);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);
#endif // CFG_LCD_MULTIPLE

#ifdef	CFG_DYNAMIC_LOAD_TP_MODULE
	//This function must be in front of SDL_Init().
	DynamicLoadTpModule();
#endif

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());

    ConfigInit();

#ifdef CFG_NET_ENABLE
    NetworkInit();
    #ifdef CFG_NET_WIFI

    #else 
        WebServerInit();
    #endif
#endif // CFG_NET_ENABLE

    ScreenInit();
    ExternalInit();
    StorageInit();
    AudioInit();
    PhotoInit();

    SceneInit();
    SceneLoad();
    ret = SceneRun();

    SceneExit();

    PhotoExit();
    AudioExit();
    ExternalExit();

#ifdef CFG_NET_ENABLE
    if (ret != QUIT_UPGRADE_WEB)
        WebServerExit();
#endif // CFG_NET_ENABLE

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
    itp_codec_standby();
    exit(ret);
    return ret;
}
