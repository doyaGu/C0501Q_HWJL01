#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "curl/curl.h"
#include "ite/ite_sd.h"
#include "ite/itu.h"
#include "ite/ug.h"
#include "SDL/SDL.h"
#include "elevator.h"
#include "scene.h"

#define UPGRADE_FILE_COUNT 2

static bool upgradeIsReady;
static bool upgradeIsFinished;
static int upgradeResult;
static ITCArrayStream arrayStream;
static ITCStream* upgradeStream;
static int upgradePercentage;

struct MemoryStruct
{
  char *memory;
  size_t size;
};

#ifdef CFG_UPGRADE_GUI

static const uint8_t upgradeHItu[] __attribute__ ((aligned)) =
{
#include "upgrade_h.inc"
};

static const uint8_t upgradeVItu[] __attribute__ ((aligned)) =
{
#include "upgrade_v.inc"
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

    if (SceneIsVertical() == EXTERNAL_VERTICAL)
    {
        ituSceneLoadArray(&scene, upgradeVItu, sizeof(upgradeVItu));
        ituSceneSetRotation(&scene, ITU_ROT_90, ithLcdGetWidth(), ithLcdGetHeight());
    }
    else
    {
        ituSceneLoadArray(&scene, upgradeHItu, sizeof(upgradeHItu));
        ituSceneSetRotation(&scene, ITU_ROT_0, ithLcdGetWidth(), ithLcdGetHeight());
    }
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

    upgradeStream = NULL;
    upgradeIsReady = false;
    upgradeIsFinished = false;
    upgradeResult = 0;

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

#ifdef CFG_NET_ENABLE

static size_t throw_away(void *ptr, size_t size, size_t nmemb, void *data)
{
    (void)ptr;
    (void)data;
    /* we are not interested in the headers itself,
     so we only return the size we would have saved ... */
    return (size_t)(size * nmemb);
}

static int GetPackageSize(char* ftpurl)
{
    CURL* curl = NULL;
    CURLcode res = CURLE_OK;
    double filesize = 0.0;

    curl = curl_easy_init();
    if (!curl)
    {
        printf("curl_easy_init() fail.\n");
        goto end;
    }

    curl_easy_setopt(curl, CURLOPT_URL, ftpurl);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, throw_away);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15L);

#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    res = curl_easy_perform(curl);
    if (CURLE_OK != res)
    {
        printf("curl_easy_perform() fail: %d\n", res);
        goto end;
    }

    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
    if ((CURLE_OK == res) && (filesize > 0.0))
    {
        printf("filesize: %0.0f bytes\n", filesize);
    }
    else
    {
        printf("curl_easy_getinfo(CURLINFO_CONTENT_LENGTH_DOWNLOAD) fail: %d, filesize: %0.0f bytes\n", res, filesize);
        filesize = 0.0;
        goto end;
    }

end:
    if (curl)
        curl_easy_cleanup(curl);

    return (int)filesize;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    assert(mem->memory);

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;

    return realsize;
}

static ITCStream* DownloadPackage(char* ftpurl, int filesize)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(filesize);    /* will be grown as needed by the realloc above */ 
    chunk.size = 0;                     /* no data at this point */ 

    /* init the curl session */ 
    curl = curl_easy_init();
    if (!curl)
    {
        printf("curl_easy_init() fail.\n");
        goto error;
    }

    /* specify URL to get */ 
    curl_easy_setopt(curl, CURLOPT_URL, ftpurl);

    /* send all data to this function  */ 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 15L);

#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    /* get it! */
    res = curl_easy_perform(curl);
    if (CURLE_OK != res)
    {
        printf("curl_easy_perform() fail: %d\n", res);
        goto error;
    }
    else
    {
        printf("%lu bytes retrieved\n", (long)chunk.size);
    }

    curl_easy_cleanup(curl);

    itcArrayStreamOpen(&arrayStream, chunk.memory, chunk.size);

    return &arrayStream.stream;

error:
    if (curl)
        curl_easy_cleanup(curl);

    free(chunk.memory);

    return NULL;
}

static ITCStream* OpenFtpPackage(char* path)
{
    int filesize;
    ITCStream* fwStream = NULL;
    int retry = 10;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    printf("ftp url: %s\n", path);

    // get file size first
    while (retry-- >= 0)
    {
        filesize = GetPackageSize(path);
        if (filesize > 0)
            break;
    }

    // download firmware
    while (retry-- >= 0)
    {
        fwStream = DownloadPackage(path, filesize);
        if (fwStream)
            break;
    };

    curl_global_cleanup();
    return fwStream;
}
#endif // CFG_NET_ENABLE

static int UpgradeNetworkResource(void)
{
    // TODO: IMPLEMENT
    return 0;
}

static int UpgradeNetworkFirmware(void)
{
    int ret = 0;
    ITCStream* fwStream = NULL;

#ifdef CFG_NET_ENABLE

    // download from ftp server
    fwStream = OpenFtpPackage(CFG_UPGRADE_FW_URL);
    if (!fwStream)
    {
        ret = -1;
        printf("remote package unavailable: %s\n", CFG_UPGRADE_FW_URL);
        return ret;
    }
#endif // CFG_NET_ENABLE

    ret = ugCheckCrc(fwStream, NULL);
    if (ret)
    {
        printf("check crc fail: %d.\n", ret);
        return ret;
    }

    ret = ugUpgradePackage(fwStream);
    if (ret)
    {
        printf("upgrade fail: %d.\n", ret);
        return ret;
    }

#ifdef CFG_UPGRADE_DELETE_PKGFILE_AFTER_FINISH
    remove(CFG_UPGRADE_FW_URL);
#endif

    printf("upgrade success!\n");
    
#if defined(CFG_UPGRADE_DELAY_AFTER_FINISH) && CFG_UPGRADE_DELAY_AFTER_FINISH > 0
    sleep(CFG_UPGRADE_DELAY_AFTER_FINISH);
#endif    
        
    return 0;
}

void UpgradeSetStream(void* stream)
{
    upgradeIsFinished = false;
    upgradeResult = 0;
    upgradeStream = stream;
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
    else if (code == QUIT_UPGRADE_WEB)
    {
        upgradeStream = NULL;
        upgradeIsReady = true;

        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_MSC00);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_SD0);

        while (!upgradeStream)
            sleep(1);

        UpgradeLcdConsoleEnable();

    #ifdef CFG_WATCHDOG_ENABLE
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
    #endif
        upgradeResult = ugUpgradePackage(upgradeStream);
        upgradeIsFinished = true;

    #ifdef CFG_WATCHDOG_ENABLE
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
    #endif

        for (;;)
            sleep(UINT_MAX);
    }
    else if (code == QUIT_UPGRADE_NET_RES)
    {
        UpgradeLcdConsoleEnable();

        ret = UpgradeNetworkResource();
    }
    else if (code == QUIT_UPGRADE_NET_FW)
    {
        UpgradeLcdConsoleEnable();

        ret = UpgradeNetworkFirmware();
    }

    if (code == QUIT_DEFAULT || code == QUIT_RESET_FACTORY || code == QUIT_UPGRADE_RESOURCE || code == QUIT_UPGRADE_NET_RES || code == QUIT_UPGRADE_NET_FW)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
        printf("Upgrade finished: %d\n", ret);

        sleep(5);
    }
    return ret;
}

bool UpgradeIsReady(void)
{
    return upgradeIsReady;
}

bool UpgradeIsFinished(void)
{
    return upgradeIsFinished;
}

int UpgradeGetResult(void)
{
    return upgradeResult;
}
