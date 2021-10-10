#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "iniparser/iniparser.h"
#include "ite/itp.h"
#include "dynamic_ui.h"

#define INI_FILENAME "dynamic_ui.ini"

Config theConfig;
static dictionary* cfgIni;
static bool cfgIsSaving;

void ConfigLoad(void)
{
    int i;
    char buf[64];

    cfgIni = iniparser_load(CFG_PRIVATE_DRIVE ":/" INI_FILENAME);
    if (!cfgIni)
    {
        cfgIni = dictionary_new(0);
        assert(cfgIni);

        dictionary_set(cfgIni, "dynamic_ui", NULL);
    }

    for (i = 0; i < MAX_BUTTON_COUNT; i++)
    {
        sprintf(buf, "dynamic_ui:btn%d_visible", i);
        theConfig.btn_visible[i] = iniparser_getboolean(cfgIni, buf, 1);

        sprintf(buf, "dynamic_ui:btn%d_tabindex", i);
        theConfig.btn_tabindex[i] = iniparser_getint(cfgIni, buf, 0);
    }
}

static void ConfigSavePrivate(void)
{
    FILE* f;
    int i;
    char buf[64], buf2[32];

    for (i = 0; i < MAX_BUTTON_COUNT; i++)
    {
        sprintf(buf, "dynamic_ui:btn%d_visible", i);
        iniparser_set(cfgIni, buf, theConfig.btn_visible[i] ? "y" : "n");

        sprintf(buf, "dynamic_ui:btn%d_tabindex", i);
        sprintf(buf2, "%d", theConfig.btn_tabindex[i]);
        iniparser_set(cfgIni, buf, buf2);
    }

    // save to file
    f = fopen(CFG_PRIVATE_DRIVE ":/" INI_FILENAME, "wb");
	if (!f)
    {
	    printf("cannot open ini file: %s\n", CFG_PRIVATE_DRIVE ":/" INI_FILENAME);
        return;
    }

    iniparser_dump_ini(cfgIni, f);
    fclose(f);
}

static void* ConfigSaveTask(void* arg)
{
    cfgIsSaving = true;

    ConfigSavePrivate();

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    cfgIsSaving = false;

    return NULL;
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
