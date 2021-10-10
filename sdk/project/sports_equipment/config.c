#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "iniparser/iniparser.h"
#include "ite/itp.h"
#include "sports_equipment.h"
#include "scene.h"

#define INI_FILENAME "sports_equipment.ini"

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

        dictionary_set(cfgIni, "sports_equipment", NULL);
    }

    // display
    theConfig.lang = iniparser_getint(cfgIni, "sports_equipment:lang", LANG_ENG);
    theConfig.brightness = iniparser_getint(cfgIni, "sports_equipment:brightness", 8);
    theConfig.demo_enable = iniparser_getboolean(cfgIni, "sports_equipment:demo_enable", 1);

    // sound
    theConfig.audiolevel = iniparser_getint(cfgIni, "sports_equipment:audiolevel", 80);

    // sports_equipment
    theConfig.distance = iniparser_getint(cfgIni, "sports_equipment:distance", 7000);
    theConfig.calorie = iniparser_getint(cfgIni, "sports_equipment:calorie", 300);
    theConfig.goal_time = iniparser_getint(cfgIni, "sports_equipment:goal_time", 2700);
	theConfig.pace = iniparser_getint(cfgIni, "sports_equipment:pace", 200);
    theConfig.slope = iniparser_getint(cfgIni, "sports_equipment:slope", 0);
    theConfig.speed = iniparser_getint(cfgIni, "sports_equipment:speed", 20000);
    theConfig.age = iniparser_getint(cfgIni, "sports_equipment:age", 45);
    theConfig.weight = iniparser_getint(cfgIni, "sports_equipment:weight", 65);
    theConfig.pulse = iniparser_getint(cfgIni, "sports_equipment:pulse", 130);
    theConfig.unit_mile = iniparser_getboolean(cfgIni, "sports_equipment:unit_mile", 0);
    theConfig.unit_lbs = iniparser_getboolean(cfgIni, "sports_equipment:unit_lbs", 0);
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

    // display
    sprintf(buf, "%d", theConfig.lang);
    iniparser_set(cfgIni, "sports_equipment:lang", buf);

    sprintf(buf, "%d", theConfig.brightness);
    iniparser_set(cfgIni, "sports_equipment:brightness", buf);

    iniparser_set(cfgIni, "sports_equipment:demo_enable", theConfig.demo_enable ? "y" : "n");

    // sound
    sprintf(buf, "%d", theConfig.audiolevel);
    iniparser_set(cfgIni, "sports_equipment:audiolevel", buf);
    
    // sports_equipment
    sprintf(buf, "%d", theConfig.distance);
    iniparser_set(cfgIni, "sports_equipment:distance", buf);

    sprintf(buf, "%d", theConfig.calorie);
    iniparser_set(cfgIni, "sports_equipment:calorie", buf);

    sprintf(buf, "%d", theConfig.goal_time);
    iniparser_set(cfgIni, "sports_equipment:goal_time", buf);

	sprintf(buf, "%d", theConfig.pace);
	iniparser_set(cfgIni, "sports_equipment:pace", buf);

    sprintf(buf, "%d", theConfig.slope);
    iniparser_set(cfgIni, "sports_equipment:slope", buf);

    sprintf(buf, "%d", theConfig.speed);
    iniparser_set(cfgIni, "sports_equipment:speed", buf);

    sprintf(buf, "%d", theConfig.age);
    iniparser_set(cfgIni, "sports_equipment:age", buf);

    sprintf(buf, "%d", theConfig.weight);
    iniparser_set(cfgIni, "sports_equipment:weight", buf);

    sprintf(buf, "%d", theConfig.pulse);
    iniparser_set(cfgIni, "sports_equipment:pulse", buf);

    iniparser_set(cfgIni, "sports_equipment:unit_mile", theConfig.unit_mile ? "y" : "n");
    iniparser_set(cfgIni, "sports_equipment:unit_lbs", theConfig.unit_lbs ? "y" : "n");

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
