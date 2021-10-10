#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "curl/curl.h"
#include "ite/ite_sd.h"
#include "ite/itu.h"
#include "ite/ug.h"
#include "SDL/SDL.h"
#include "ctrlboard.h"

#define URL_LEN 256

static bool upgradeIsReady;
static char upgradeUrl[URL_LEN];
static bool upgradeIsFinished;
static int upgradeResult;
static ITCArrayStream arrayStream;
static ITCFileStream fileStream;
static ITCStream* upgradeStream;
static char pkgFilePath[PATH_MAX];

struct MemoryStruct
{
  char *memory;
  size_t size;
};

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
    ituSceneLoadArray(&scene, upgradeItu, sizeof(upgradeItu));

    bar = (ITUProgressBar*)ituSceneFindWidget(&scene, "upgradeProgressBar");
    assert(bar);

    for (;;)
    {
        ituProgressBarSetValue(bar, ugGetProrgessPercentage());

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
#ifdef CFG_LCD_ENABLE
    // switch to lcd console
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
    itpRegisterDevice(ITP_DEVICE_LCDCONSOLE, &itpDeviceLcdConsole);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_LCDCONSOLE, ITP_IOCTL_CLEAR, NULL);
#endif
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

static ITCStream* OpenUsbPackage(char* path)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus = NULL;
    int i;

    // try to find the package drive
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = ITP_MAX_DRIVE - 1; i >= 0; i--)
    {
        driveStatus = &driveStatusTable[i];
        if (driveStatus->avail && driveStatus->disk >= ITP_DISK_MSC00 && driveStatus->disk <= ITP_DISK_MSC17)
        {
            char buf[PATH_MAX], *ptr, *saveptr;

            // get file path from list
            strcpy(buf, path);
            ptr = strtok_r(buf, " ", &saveptr);
            do
            {
                strcpy(pkgFilePath, driveStatus->name);
                strcat(pkgFilePath, ptr);

                if (itcFileStreamOpen(&fileStream, pkgFilePath, false) == 0)
                {
                    printf("found package file %s\n", pkgFilePath);
                    return &fileStream.stream;
                }
                else
                {
                    printf("try to fopen(%s) fail:0x%X\n", pkgFilePath, errno);
                }
            }
            while ((ptr = strtok_r(NULL, " ", &saveptr)) != NULL);
        }
    }
    printf("cannot find package file.\n");
    return NULL;
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

static int UpgradePackage(void)
{
    int ret = 0;
    ITCStream* fwStream = NULL;

    if (upgradeUrl[0] == '\0')
    {
       // open from USB drive
       fwStream = OpenUsbPackage(CFG_UPGRADE_FILENAME_LIST);
       if (!fwStream)
       {
           ret = -1;
           printf("packages unavailable: %s\n", CFG_UPGRADE_FILENAME_LIST);
           return ret;
       }
    }
#ifdef CFG_NET_ENABLE
    else
    {
        // download from ftp server
        fwStream = OpenFtpPackage(upgradeUrl);
        if (!fwStream)
        {
            ret = -1;
            printf("remote package unavailable: %s\n", upgradeUrl);
            return ret;
        }
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
    if (upgradeUrl[0] == '\0')
    {
        remove(pkgFilePath);
    }
#endif

    printf("upgrade success!\n");
    
#if defined(CFG_UPGRADE_DELAY_AFTER_FINISH) && CFG_UPGRADE_DELAY_AFTER_FINISH > 0
    sleep(CFG_UPGRADE_DELAY_AFTER_FINISH);
#endif    
        
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

void UpgradeSetUrl(char* url)
{
    if (url)
    {
        strncpy(upgradeUrl, url, URL_LEN - 1);
        upgradeUrl[URL_LEN - 1] = '\0';
    }
    else
        upgradeUrl[0] = '\0';
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
    else if (code == QUIT_UPGRADE_FIRMWARE)
    {
        UpgradeLcdConsoleEnable();
        ret = UpgradePackage();
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

    if (code == QUIT_RESET_FACTORY || code == QUIT_UPGRADE_FIRMWARE || code == QUIT_RESET_NETWORK || code == QUIT_DEFAULT)
    {
    #if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
    #endif
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
