#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "iniparser/iniparser.h"
#include "ite/itp.h"
#include "airconditioner.h"
#include "scene.h"

#define INI_FILENAME "airconditioner.ini"

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

        dictionary_set(cfgIni, "airconditioner", NULL);
    }

    // network
    theConfig.dhcp = iniparser_getboolean(cfgIni, "tcpip:dhcp", 1);
    strncpy(theConfig.ipaddr, iniparser_getstring(cfgIni, "tcpip:ipaddr", "192.168.1.1"), sizeof (theConfig.ipaddr) - 1);
    strncpy(theConfig.netmask, iniparser_getstring(cfgIni, "tcpip:netmask", "255.255.255.0"), sizeof (theConfig.netmask) - 1);
    strncpy(theConfig.gw, iniparser_getstring(cfgIni, "tcpip:gw", "192.168.1.254"), sizeof (theConfig.gw) - 1);
    strncpy(theConfig.dns, iniparser_getstring(cfgIni, "tcpip:dns", "192.168.1.254"), sizeof (theConfig.dns) - 1);

    // display
    theConfig.brightness = iniparser_getint(cfgIni, "airconditioner:brightness", 8);
    theConfig.screensaver_time = iniparser_getint(cfgIni, "airconditioner:screensaver_time", 0);
    theConfig.lang = iniparser_getint(cfgIni, "airconditioner:lang", LANG_ENG);
 
    // sound
    theConfig.audiolevel = iniparser_getint(cfgIni, "airconditioner:audiolevel", 80);
    theConfig.sound_enable = iniparser_getboolean(cfgIni, "airconditioner:sound_enable", 0);

    // airconditioner
    theConfig.temperature = iniparser_getint(cfgIni, "airconditioner:temperature", 26);
    theConfig.mode = iniparser_getint(cfgIni, "airconditioner:mode", 4);
    theConfig.wind = iniparser_getint(cfgIni, "airconditioner:wind", 0);
    theConfig.light_enable = iniparser_getboolean(cfgIni, "airconditioner:light_enable", 0);
    theConfig.power_on_enable = iniparser_getboolean(cfgIni, "airconditioner:power_on_enable", 0);
    theConfig.power_on_hour = iniparser_getint(cfgIni, "airconditioner:power_on_hour", 0);
    theConfig.power_on_min = iniparser_getint(cfgIni, "airconditioner:power_on_min", 0);
    theConfig.power_off_enable = iniparser_getboolean(cfgIni, "airconditioner:power_off_enable", 0);
    theConfig.power_off_hour = iniparser_getint(cfgIni, "airconditioner:power_off_hour", 0);
    theConfig.power_off_min = iniparser_getint(cfgIni, "airconditioner:power_off_min", 0);
    theConfig.power_mon_enable = iniparser_getboolean(cfgIni, "airconditioner:power_mon_enable", 0);
    theConfig.power_tue_enable = iniparser_getboolean(cfgIni, "airconditioner:power_tue_enable", 0);
    theConfig.power_wed_enable = iniparser_getboolean(cfgIni, "airconditioner:power_wed_enable", 0);
    theConfig.power_thu_enable = iniparser_getboolean(cfgIni, "airconditioner:power_thu_enable", 0);
    theConfig.power_fri_enable = iniparser_getboolean(cfgIni, "airconditioner:power_fri_enable", 0);
    theConfig.power_sat_enable = iniparser_getboolean(cfgIni, "airconditioner:power_sat_enable", 0);
    theConfig.power_sun_enable = iniparser_getboolean(cfgIni, "airconditioner:power_sun_enable", 0);
    theConfig.power_price_dollar = iniparser_getint(cfgIni, "airconditioner:power_price_dollar", 0);
    theConfig.power_price_cent = iniparser_getint(cfgIni, "airconditioner:power_price_cent", 68);
    theConfig.power_reset_year = iniparser_getint(cfgIni, "airconditioner:power_reset_year", 0);
    theConfig.power_reset_month = iniparser_getint(cfgIni, "airconditioner:power_reset_month", 0);
    theConfig.power_reset_day = iniparser_getint(cfgIni, "airconditioner:power_reset_day", 0);
    theConfig.power_vol_mon = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_mon", 0);
    theConfig.power_vol_tue = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_tue", 0);
    theConfig.power_vol_wed = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_wed", 0);
    theConfig.power_vol_thu = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_thu", 0);
    theConfig.power_vol_fri = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_fri", 0);
    theConfig.power_vol_sat = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_sat", 0);
    theConfig.power_vol_sun = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_sun", 0);
    theConfig.power_vol_before = (float)iniparser_getdouble(cfgIni, "airconditioner:power_vol_before", 0);

    // login
    strncpy(theConfig.user_id, iniparser_getstring(cfgIni, "airconditioner:user_id", "admin"), sizeof (theConfig.user_id) - 1);
    strncpy(theConfig.user_password, iniparser_getstring(cfgIni, "airconditioner:user_password", "admin"), sizeof (theConfig.user_password) - 1);
}

void ConfigExit(void)
{
    iniparser_freedict(cfgIni);
    cfgIni = NULL;
}

static void ConfigSavePublic(void)
{
    FILE* f;
    char buf[32];

    // network
    iniparser_set(cfgIni, "tcpip:dhcp", theConfig.dhcp ? "y" : "n");
    iniparser_set(cfgIni, "tcpip:ipaddr", theConfig.ipaddr);
    iniparser_set(cfgIni, "tcpip:netmask", theConfig.netmask);
    iniparser_set(cfgIni, "tcpip:gw", theConfig.gw);
    iniparser_set(cfgIni, "tcpip:dns", theConfig.dns);

    // display
    sprintf(buf, "%d", theConfig.brightness);
    iniparser_set(cfgIni, "airconditioner:brightness", buf);

    sprintf(buf, "%d", theConfig.screensaver_time);
    iniparser_set(cfgIni, "airconditioner:screensaver_time", buf);

    sprintf(buf, "%d", theConfig.lang);
    iniparser_set(cfgIni, "airconditioner:lang", buf);

    // sound
    sprintf(buf, "%d", theConfig.audiolevel);
    iniparser_set(cfgIni, "airconditioner:audiolevel", buf);
    
    iniparser_set(cfgIni, "airconditioner:sound_enable", theConfig.sound_enable ? "y" : "n");

    // airconditioner
    sprintf(buf, "%d", theConfig.temperature);
    iniparser_set(cfgIni, "airconditioner:temperature", buf);

    sprintf(buf, "%d", theConfig.mode);
    iniparser_set(cfgIni, "airconditioner:mode", buf);

    sprintf(buf, "%d", theConfig.wind);
    iniparser_set(cfgIni, "airconditioner:wind", buf);

    iniparser_set(cfgIni, "airconditioner:light_enable", theConfig.light_enable ? "y" : "n");

    iniparser_set(cfgIni, "airconditioner:power_on_enable", theConfig.power_on_enable ? "y" : "n");

    sprintf(buf, "%d", theConfig.power_on_hour);
    iniparser_set(cfgIni, "airconditioner:power_on_hour", buf);

    sprintf(buf, "%d", theConfig.power_on_min);
    iniparser_set(cfgIni, "airconditioner:power_on_min", buf);

    iniparser_set(cfgIni, "airconditioner:power_off_enable", theConfig.power_off_enable ? "y" : "n");

    sprintf(buf, "%d", theConfig.power_off_hour);
    iniparser_set(cfgIni, "airconditioner:power_off_hour", buf);

    sprintf(buf, "%d", theConfig.power_off_min);
    iniparser_set(cfgIni, "airconditioner:power_off_min", buf);

    iniparser_set(cfgIni, "airconditioner:power_mon_enable", theConfig.power_mon_enable ? "y" : "n");
    iniparser_set(cfgIni, "airconditioner:power_tue_enable", theConfig.power_tue_enable ? "y" : "n");
    iniparser_set(cfgIni, "airconditioner:power_wed_enable", theConfig.power_wed_enable ? "y" : "n");
    iniparser_set(cfgIni, "airconditioner:power_thu_enable", theConfig.power_thu_enable ? "y" : "n");
    iniparser_set(cfgIni, "airconditioner:power_fri_enable", theConfig.power_fri_enable ? "y" : "n");
    iniparser_set(cfgIni, "airconditioner:power_sat_enable", theConfig.power_sat_enable ? "y" : "n");
    iniparser_set(cfgIni, "airconditioner:power_sun_enable", theConfig.power_sun_enable ? "y" : "n");

    sprintf(buf, "%d", theConfig.power_price_dollar);
    iniparser_set(cfgIni, "airconditioner:power_price_dollar", buf);

    sprintf(buf, "%d", theConfig.power_price_cent);
    iniparser_set(cfgIni, "airconditioner:power_price_cent", buf);

    sprintf(buf, "%d", theConfig.power_reset_year);
    iniparser_set(cfgIni, "airconditioner:power_reset_year", buf);

    sprintf(buf, "%d", theConfig.power_reset_month);
    iniparser_set(cfgIni, "airconditioner:power_reset_month", buf);

    sprintf(buf, "%d", theConfig.power_reset_day);
    iniparser_set(cfgIni, "airconditioner:power_reset_day", buf);

    sprintf(buf, "%f", theConfig.power_vol_mon);
    iniparser_set(cfgIni, "airconditioner:power_vol_mon", buf);

    sprintf(buf, "%f", theConfig.power_vol_tue);
    iniparser_set(cfgIni, "airconditioner:power_vol_tue", buf);

    sprintf(buf, "%f", theConfig.power_vol_wed);
    iniparser_set(cfgIni, "airconditioner:power_vol_wed", buf);

    sprintf(buf, "%f", theConfig.power_vol_thu);
    iniparser_set(cfgIni, "airconditioner:power_vol_thu", buf);

    sprintf(buf, "%f", theConfig.power_vol_fri);
    iniparser_set(cfgIni, "airconditioner:power_vol_fri", buf);

    sprintf(buf, "%f", theConfig.power_vol_sat);
    iniparser_set(cfgIni, "airconditioner:power_vol_sat", buf);

    sprintf(buf, "%f", theConfig.power_vol_sun);
    iniparser_set(cfgIni, "airconditioner:power_vol_sun", buf);

    sprintf(buf, "%f", theConfig.power_vol_before);
    iniparser_set(cfgIni, "airconditioner:power_vol_before", buf);

    // login
    iniparser_set(cfgIni, "airconditioner:user_id", theConfig.user_id);
    iniparser_set(cfgIni, "airconditioner:user_password", theConfig.user_password);

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
