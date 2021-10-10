#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "airconditioner.h"

extern ExternalMachine engineerMachine;

static ITUSprite* engineerPowerMemorySprite;

static void EngineerPowerUpdateMachineStatus(void)
{
    ituSpriteGoto(engineerPowerMemorySprite, engineerMachine.powerMemroyStatus);
}

bool EngineerPowerMemoryOnButtonOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_ENG_POWER_MEMORY;
    ev.arg1 = 1;
    ExternalSend(&ev);

    return true;
}

bool EngineerPowerMemoryOffButtonOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_ENG_POWER_MEMORY;
    ev.arg1 = 0;
    ExternalSend(&ev);

    return true;
}

bool EngineerPowerOnTimer(ITUWidget* widget, char* param)
{
    if (memcmp(&engineerMachine, &extMachine, sizeof(ExternalMachine)))
    {
        memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
        EngineerPowerUpdateMachineStatus();
        return true;
    }
    return false;
}

bool EngineerPowerOnEnter(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    if (!engineerPowerMemorySprite)
    {
        engineerPowerMemorySprite = ituSceneFindWidget(&theScene, "engineerPowerMemorySprite");
        assert(engineerPowerMemorySprite);
    }
    memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
    EngineerPowerUpdateMachineStatus();

    ev.type = EXTERNAL_ENG_POWER_MODE;
    ev.arg1 = 1;
    ExternalSend(&ev);

    return true;
}

bool EngineerPowerOnLeave(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_ENG_POWER_MODE;
    ev.arg1 = 0;
    ExternalSend(&ev);

    return true;
}

void EngineerPowerReset(void)
{
    engineerPowerMemorySprite = NULL;
}
