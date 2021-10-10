#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "airconditioner.h"

static ITULayer* logoVersionText;

bool LogoOnEnter(ITUWidget* widget, char* param)
{
    if (!logoVersionText)
    {
        logoVersionText = ituSceneFindWidget(&theScene, "logoVersionText");
        assert(logoVersionText);
    }
    ituTextSetString(logoVersionText, CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR);

    return true;
}

void LogoReset(void)
{
    logoVersionText = NULL;
}
