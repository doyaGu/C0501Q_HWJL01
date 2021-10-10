#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "iniparser/iniparser.h"
#include "ite/itp.h"
#include "ctrlboard.h"

#define INI_FILENAME "test_ftp_client.ini"

Config theConfig;
static dictionary* cfgIni;
static bool cfgIsSaving;
static bool cfgSavingCount;
static pthread_mutex_t cfgMutex  = PTHREAD_MUTEX_INITIALIZER;

void ConfigInit(void)
{
    // network
    theConfig.dhcp = 1;
    strncpy(theConfig.ipaddr, "192.168.1.1", sizeof (theConfig.ipaddr) - 1);
    strncpy(theConfig.netmask, "255.255.255.0", sizeof (theConfig.netmask) - 1);
    strncpy(theConfig.gw, "192.168.1.254", sizeof (theConfig.gw) - 1);
    strncpy(theConfig.dns, "192.168.1.1", sizeof (theConfig.dns) - 1);

    // wifi mode
    theConfig.wifi_mode = 1;
	// wifi switch status
	theConfig.wifi_status= 1;
    // SSID
    snprintf(theConfig.ssid, 64, "TP-LINK_C217");
    // Password
    snprintf(theConfig.password, 256, "1234567890");
    // Security mode
    snprintf(theConfig.secumode, 3, "6");        

    cfgSavingCount = 0;
}

void ConfigExit(void)
{
    iniparser_freedict(cfgIni);
    cfgIni = NULL;
}

static void ConfigSavePublic(void)
{
    FILE* f;
    char buf[8];

    // network
    iniparser_set(cfgIni, "tcpip:dhcp", theConfig.dhcp ? "y" : "n");
    iniparser_set(cfgIni, "tcpip:ipaddr", theConfig.ipaddr);
    iniparser_set(cfgIni, "tcpip:netmask", theConfig.netmask);
    iniparser_set(cfgIni, "tcpip:gw", theConfig.gw);
    iniparser_set(cfgIni, "tcpip:dns", theConfig.dns);

    // display
    sprintf(buf, "%d", theConfig.lang);
    iniparser_set(cfgIni, "ctrlboard:lang", buf);

    sprintf(buf, "%d", theConfig.brightness);
    iniparser_set(cfgIni, "ctrlboard:brightness", buf);
    sprintf(buf, "%d", theConfig.screensaver_time);
    iniparser_set(cfgIni, "ctrlboard:screensaver_time", buf);
    sprintf(buf, "%d", theConfig.screensaver_type);
    iniparser_set(cfgIni, "ctrlboard:screensaver_type", buf);

    sprintf(buf, "%d", theConfig.mainmenu_type);
    iniparser_set(cfgIni, "ctrlboard:mainmenu_type", buf);

    // sound
    iniparser_set(cfgIni, "ctrlboard:keysound", theConfig.keysound);

    sprintf(buf, "%d", theConfig.keylevel);
    iniparser_set(cfgIni, "ctrlboard:keylevel", buf);

    sprintf(buf, "%d", theConfig.audiolevel);
    iniparser_set(cfgIni, "ctrlboard:audiolevel", buf);

    //wifi
    // wifi mode
    sprintf(buf, "%d", theConfig.wifi_mode);
    iniparser_set(cfgIni, "wifi:wifi_mode", buf);
	// wifi switch status
    sprintf(buf, "%d", theConfig.wifi_status);
    iniparser_set(cfgIni, "wifi:wifi_status", buf);
    // SSID
    iniparser_set(cfgIni, "wifi:ssid", theConfig.ssid);
    // Password    
    iniparser_set(cfgIni, "wifi:password", theConfig.password);
    // Security mode    
    iniparser_set(cfgIni, "wifi:secumode", theConfig.secumode);
 
    // photo
    sprintf(buf, "%d", theConfig.photo_interval);
    iniparser_set(cfgIni, "ctrlboard:photo_interval", buf);
    
    // misc
    sprintf(buf, "%d", theConfig.touch_calibration);
    iniparser_set(cfgIni, "ctrlboard:touch_calibration", buf);

    // save to file
    f = fopen(CFG_PUBLIC_DRIVE ":/" INI_FILENAME, "wb");
	if (!f)
    {
	    printf("cannot open ini file: %s\n", CFG_PUBLIC_DRIVE ":/" INI_FILENAME);
        return;
    }

    iniparser_dump_ini(cfgIni, f);
    fclose(f);
}

static void* ConfigSaveTask(void* arg)
{
    char* filepath = CFG_PUBLIC_DRIVE ":/" INI_FILENAME;
    int savingCount;

    cfgIsSaving = true;

    do
    {
        savingCount = cfgSavingCount;

        ConfigSavePublic();

        pthread_mutex_lock(&cfgMutex);

#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
        UpgradeSetFileCrc(filepath);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
        BackupSave();
    #else
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
#endif

        pthread_mutex_unlock(&cfgMutex);

    } while (savingCount != cfgSavingCount);

    cfgIsSaving = false;

    return NULL;
}

void ConfigUpdateCrc(char* filepath)
{
#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
    pthread_mutex_lock(&cfgMutex);

    if (filepath)
        UpgradeSetFileCrc(filepath);
    else
        UpgradeSetFileCrc(CFG_PUBLIC_DRIVE ":/" INI_FILENAME);

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    pthread_mutex_unlock(&cfgMutex);
#endif // CFG_CHECK_FILES_CRC_ON_BOOTING
}

void ConfigSave(void)
{
    pthread_t task;
    pthread_attr_t attr;

    cfgSavingCount++;

    if (cfgIsSaving)
        return;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, ConfigSaveTask, NULL);
}
