#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/ite_sd.h"
#include "ite/itu.h"
#include "ite/ug.h"
#include "SDL/SDL.h"
#include "sports_equipment.h"
#include "scene.h"

#define UPGRADE_FILE_COUNT 2

static int upgradePercentage;

#ifdef CFG_UPGRADE_GUI

static const uint8_t upgradeItu[] __attribute__ ((aligned)) =
{
#include "upgrade.inc"
};

static void* UpgradeGuiTask(void* arg)
{
    ITUScene scene;
    ITUProgressBar* bar;
    SDL_Window *window = SDL_CreateWindow("Display Control Board", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ithLcdGetWidth(), ithLcdGetHeight(), 0);

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    ituSceneInit(&scene, NULL);
    ituSceneLoadArray(&scene, upgradeHItu, sizeof(upgradeItu));

    bar = (ITUProgressBar*)ituSceneFindWidget(&scene, "upgradeProgressBar");
    assert(bar);

    for (;;)
    {
        ituProgressBarSetValue(bar, upgradePercentage);

        if (ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0))
        {
            ituSceneDraw(&scene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
        }
        usleep(33000);
    }
}

#endif // CFG_UPGRADE_GUI

int UpgradeInit(void)
{
    int ret = 0;

#if !defined(CFG_LCD_INIT_ON_BOOTING) && !defined(CFG_BL_SHOW_LOGO)
    uint16_t* addr = (uint16_t*) ithLcdGetBaseAddrA();
    int size = ithLcdGetPitch() * ithLcdGetHeight();
    uint16_t* base = ithMapVram((uint32_t) addr, size, ITH_VRAM_WRITE);

    memset(base, 0, size);
    ithUnmapVram(base, size);
    
#endif // !defined(CFG_LCD_INIT_ON_BOOTING) && !defined(CFG_BL_SHOW_LOGO)

#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING

    ret = ugCheckFilesCrc(CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
    if (ret)
    {
        printf("check public file crc result: %d\n", ret);
        ret = QUIT_RESET_FACTORY;
        goto end;
    }
end:
#endif // CFG_CHECK_FILES_CRC_ON_BOOTING

    return ret;
}

static void UpgradeLcdConsoleEnable(void)
{
#ifdef CFG_UPGRADE_GUI
    pthread_t task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, UpgradeGuiTask, NULL);
#else
    // switch to lcd console
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
#endif // CFG_UPGRADE_GUI
}

static int UpgradeResetFactory(void)
{
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
    ugResetFactory();
    return 0;
}

void UpgradeSetFileCrc(char* filepath)
{
    if (strncmp(filepath, CFG_PUBLIC_DRIVE, 1) == 0)
        ugSetFileCrc(&filepath[2], CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
}

static void UpgradeFilesCrc(void)
{
    ugUpgradeFilesCrc(CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
}

static int UpgradeResourceDirectory(char* path)
{
    DIR           *dir;
    struct dirent *ent;
    int ret = 0;
    bool found = false;
    char srcPath[PATH_MAX];

    strcpy(srcPath, StorageGetDrivePath(StorageGetCurrType()));
    strcat(srcPath, CFG_RES_PATH);
    strcat(srcPath, "/");
    strcat(srcPath, path);

    dir = opendir(srcPath);
    if (dir == NULL)
    {
        printf("cannot open directory %s\n", srcPath);
        ret = __LINE__;
        goto end;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0)
            continue;

        if (strcmp(ent->d_name, "..") == 0)
            continue;

        if (ent->d_type != DT_DIR)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        char destPath[PATH_MAX];

        strcpy(destPath, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/");
        strcat(destPath, path);
        ugDeleteDirectory(destPath);

        ret = ugCopyDirectory(destPath, srcPath);
    }

end:
    if (dir)
    {
        if (closedir(dir))
            printf("cannot closedir (%s)\n", srcPath);
    }
    return ret;
}

static int UpgradeResource(void)
{
    DIR           *dir;
    struct dirent *ent;
    int ret = 0;
    char src[PATH_MAX];

    upgradePercentage = 0;

    strcpy(src, StorageGetDrivePath(StorageGetCurrType()));
    strcat(src, CFG_RES_PATH);

    dir = opendir(src);
    if (dir == NULL)
    {
        printf("cannot open directory %s\n", src);
        ret = __LINE__;
        goto end;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        int ret1;

        if (strcmp(ent->d_name, ".") == 0)
            continue;

        if (strcmp(ent->d_name, "..") == 0)
            continue;

        if (ent->d_type == DT_DIR)
        {
            ret1 = UpgradeResourceDirectory(ent->d_name);
            if (ret1)
            {
                if (ret == 0)
                    ret = ret1;
            }
        }
        else
        {
            char srcPath[PATH_MAX], destPath[PATH_MAX];

            upgradePercentage += (int)roundf(100.0f / UPGRADE_FILE_COUNT);
            if (upgradePercentage > 100)
                upgradePercentage = 100;
    
            //printf("upgradePercentage=%d\n", upgradePercentage);

            strcpy(destPath, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/");
            strcat(destPath, ent->d_name);
            strcpy(srcPath, src);
            strcat(srcPath, "/");
            strcat(srcPath, ent->d_name);

            ret1 = ugCopyFile(destPath, srcPath);
            if (ret1)
            {
                if (ret == 0)
                    ret = ret1;
            }
        }
    }

end:
    if (dir)
    {
        if (closedir(dir))
            printf("cannot closedir (%s)\n", src);
    }

    ugDeleteDirectory(src);
    return ret;
}

int UpgradeProcess(int code)
{
    int ret = 0;

    if (code == QUIT_RESET_FACTORY)
    {
        UpgradeLcdConsoleEnable();
        ret = UpgradeResetFactory();
    }
    else if (code == QUIT_UPGRADE_RESOURCE)
    {
        UpgradeLcdConsoleEnable();

        ret = UpgradeResource();
    }

    if (code == QUIT_DEFAULT || code == QUIT_RESET_FACTORY || code == QUIT_UPGRADE_RESOURCE)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
        puts("Upgrade finished.");

        sleep(5);
    }
    return ret;
}
