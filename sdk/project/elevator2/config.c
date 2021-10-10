#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "iniparser/iniparser.h"
#include "ite/itp.h"
#include "elevator.h"
#include "scene.h"

#define INI_FILENAME "elevator.ini"

Config theConfig;
static dictionary* cfgIni;
static bool cfgIsSaving;
static pthread_mutex_t cfgMutex  = PTHREAD_MUTEX_INITIALIZER;

void ConfigInit(void)
{
    cfgIni = iniparser_load(CFG_PUBLIC_DRIVE ":/" INI_FILENAME);
    if (!cfgIni)
    {
        cfgIni = dictionary_new(0);
        assert(cfgIni);

        dictionary_set(cfgIni, "tcpip", NULL);
        dictionary_set(cfgIni, "elevator", NULL);
    }

    // network
    theConfig.dhcp = iniparser_getboolean(cfgIni, "tcpip:dhcp", 1);
    strncpy(theConfig.ipaddr, iniparser_getstring(cfgIni, "tcpip:ipaddr", "192.168.1.1"), sizeof (theConfig.ipaddr) - 1);
    strncpy(theConfig.netmask, iniparser_getstring(cfgIni, "tcpip:netmask", "255.255.255.0"), sizeof (theConfig.netmask) - 1);
    strncpy(theConfig.gw, iniparser_getstring(cfgIni, "tcpip:gw", "192.168.1.254"), sizeof (theConfig.gw) - 1);
    strncpy(theConfig.dns, iniparser_getstring(cfgIni, "tcpip:dns", "192.168.1.254"), sizeof (theConfig.dns) - 1);

    // display
    theConfig.lang = iniparser_getint(cfgIni, "elevator:lang", LANG_ENG);
    theConfig.brightness = iniparser_getint(cfgIni, "elevator:brightness", 8);
    theConfig.screensaver_time = iniparser_getint(cfgIni, "elevator:screensaver_time", 30);
    theConfig.screensaver_type = iniparser_getint(cfgIni, "elevator:screensaver_type", SCREENSAVER_NONE);
    theConfig.date_format = iniparser_getint(cfgIni, "elevator:date_format", 0);
    theConfig.logo1_format = iniparser_getint(cfgIni, "elevator:logo1_format", 0);
    theConfig.logo1_delay = iniparser_getint(cfgIni, "elevator:logo1_delay", 0);
    theConfig.logo2_format = iniparser_getint(cfgIni, "elevator:logo2_format", 0);
    theConfig.logo2_delay = iniparser_getint(cfgIni, "elevator:logo2_delay", 0);
    theConfig.web1_format = iniparser_getint(cfgIni, "elevator:web1_format", 0);
    theConfig.web1_delay = iniparser_getint(cfgIni, "elevator:web1_delay", 0);
    theConfig.info1_format = iniparser_getint(cfgIni, "elevator:info1_format", 0);
    theConfig.info1_delay = iniparser_getint(cfgIni, "elevator:info1_delay", 0);
    theConfig.info2_format = iniparser_getint(cfgIni, "elevator:info2_format", 0);
    theConfig.info2_delay = iniparser_getint(cfgIni, "elevator:info2_delay", 0);
    theConfig.arrow_delay = iniparser_getint(cfgIni, "elevator:arrow_delay", 0);
    theConfig.demo_enable = iniparser_getint(cfgIni, "elevator:demo_enable", 1);

    // sound
    theConfig.audiolevel = iniparser_getint(cfgIni, "elevator:audiolevel", 80);
    theConfig.sound_enable = iniparser_getint(cfgIni, "elevator:sound_enable", 1);

    // photo
    theConfig.photo1_format = iniparser_getint(cfgIni, "elevator:photo1_format", 0);
    theConfig.photo1_interval = iniparser_getint(cfgIni, "elevator:photo1_interval", 10);
    theConfig.photo2_format = iniparser_getint(cfgIni, "elevator:photo2_format", 0);
    theConfig.photo2_interval = iniparser_getint(cfgIni, "elevator:photo2_interval", 10);

    // login
    strncpy(theConfig.user_id, iniparser_getstring(cfgIni, "elevator:user_id", "admin"), sizeof (theConfig.user_id) - 1);
    strncpy(theConfig.user_password, iniparser_getstring(cfgIni, "elevator:user_password", "admin"), sizeof (theConfig.user_password) - 1);

    // wifi mode
    theConfig.wifi_mode = iniparser_getint(cfgIni, "wifi:wifi_mode", 0);
    // SSID
    snprintf(theConfig.ssid, 64, iniparser_getstring(cfgIni, "wifi:ssid", "admin"));
    // Password
    snprintf(theConfig.password, 256, iniparser_getstring(cfgIni,  "wifi:password", "admin"));
    // Security mode
    snprintf(theConfig.secumode, 3, iniparser_getstring(cfgIni, "wifi:secumode", "NA"));
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
    iniparser_set(cfgIni, "elevator:lang", buf);

    sprintf(buf, "%d", theConfig.brightness);
    iniparser_set(cfgIni, "elevator:brightness", buf);

    sprintf(buf, "%d", theConfig.screensaver_time);
    iniparser_set(cfgIni, "elevator:screensaver_time", buf);

    sprintf(buf, "%d", theConfig.screensaver_type);
    iniparser_set(cfgIni, "elevator:screensaver_type", buf);

    sprintf(buf, "%d", theConfig.date_format);
    iniparser_set(cfgIni, "elevator:date_format", buf);

    sprintf(buf, "%d", theConfig.logo1_format);
    iniparser_set(cfgIni, "elevator:logo1_format", buf);

    sprintf(buf, "%d", theConfig.logo1_delay);
    iniparser_set(cfgIni, "elevator:logo1_delay", buf);

    sprintf(buf, "%d", theConfig.logo2_format);
    iniparser_set(cfgIni, "elevator:logo2_format", buf);

    sprintf(buf, "%d", theConfig.logo2_delay);
    iniparser_set(cfgIni, "elevator:logo2_delay", buf);

    sprintf(buf, "%d", theConfig.web1_format);
    iniparser_set(cfgIni, "elevator:web1_format", buf);

    sprintf(buf, "%d", theConfig.web1_delay);
    iniparser_set(cfgIni, "elevator:web1_delay", buf);

    sprintf(buf, "%d", theConfig.web2_format);
    iniparser_set(cfgIni, "elevator:web2_format", buf);

    sprintf(buf, "%d", theConfig.web2_delay);
    iniparser_set(cfgIni, "elevator:web2_delay", buf);

    sprintf(buf, "%d", theConfig.web3_format);
    iniparser_set(cfgIni, "elevator:web3_format", buf);

    sprintf(buf, "%d", theConfig.web3_delay);
    iniparser_set(cfgIni, "elevator:web3_delay", buf);

    sprintf(buf, "%d", theConfig.info1_format);
    iniparser_set(cfgIni, "elevator:info1_format", buf);

    sprintf(buf, "%d", theConfig.info1_delay);
    iniparser_set(cfgIni, "elevator:info1_delay", buf);

    sprintf(buf, "%d", theConfig.info2_format);
    iniparser_set(cfgIni, "elevator:info2_format", buf);

    sprintf(buf, "%d", theConfig.info2_delay);
    iniparser_set(cfgIni, "elevator:info2_delay", buf);

    sprintf(buf, "%d", theConfig.arrow_delay);
    iniparser_set(cfgIni, "elevator:arrow_delay", buf);

    sprintf(buf, "%d", theConfig.demo_enable);
    iniparser_set(cfgIni, "elevator:demo_enable", buf);

    // sound
    sprintf(buf, "%d", theConfig.audiolevel);
    iniparser_set(cfgIni, "elevator:audiolevel", buf);

    sprintf(buf, "%d", theConfig.sound_enable);
    iniparser_set(cfgIni, "elevator:sound_enable", buf);

    // photo
    sprintf(buf, "%d", theConfig.photo1_format);
    iniparser_set(cfgIni, "elevator:photo1_format", buf);

    sprintf(buf, "%d", theConfig.photo1_interval);
    iniparser_set(cfgIni, "elevator:photo1_interval", buf);

    sprintf(buf, "%d", theConfig.photo2_format);
    iniparser_set(cfgIni, "elevator:photo2_format", buf);

    sprintf(buf, "%d", theConfig.photo2_interval);
    iniparser_set(cfgIni, "elevator:photo2_interval", buf);

    // login
    iniparser_set(cfgIni, "elevator:user_id", theConfig.user_id);
    iniparser_set(cfgIni, "elevator:user_password", theConfig.user_password);

    //wifi
    // wifi mode
    sprintf(buf, "%d", theConfig.wifi_mode);
    iniparser_set(cfgIni, "wifi:wifi_mode", buf);
    // SSID
    iniparser_set(cfgIni, "wifi:ssid", theConfig.ssid);
    // Password    
    iniparser_set(cfgIni, "wifi:password", theConfig.password);
    // Security mode    
    iniparser_set(cfgIni, "wifi:secumode", theConfig.secumode);
    
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

    cfgIsSaving = true;

    ConfigSavePublic();

    pthread_mutex_lock(&cfgMutex);

#ifdef CFG_CHECK_FILES_CRC_ON_BOOTING
    UpgradeSetFileCrc(filepath);
#endif

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    pthread_mutex_unlock(&cfgMutex);

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

    if (cfgIsSaving)
        return;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&task, &attr, ConfigSaveTask, NULL);
}
