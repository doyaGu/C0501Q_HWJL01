#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "airconditioner.h"

static ITUSprite* engineerTryPowerStatusSprite;
static ITUSprite* engineerTryModeSprite;
static ITUText* engineerTryIndoorTemperatureText;
static ITUText* engineerTryOutdoorTemperatureText;
static ITUText* engineerTryExhaustTemperatureText;
static ITUText* engineerTryCompressorTargetFreqText;
static ITUText* engineerTryWiFiStrengthText;
static ITUText* engineerTryRadioID0Text;
static ITUText* engineerTryRadioID1Text;
static ITUText* engineerTryTemperatureText;
static ITUText* engineerTryEvaporatorTemperatureText;
static ITUText* engineerTryCondenserTemperatureText;
static ITUText* engineerTryReturnAirTemperatureText;
static ITUText* engineerTryCompressorActualFreqText;
static ITUText* engineerTryCurrentText;
static ITUText* engineerTryVoltageText;
static ITUText* engineerTryRadioID2Text;
static ITUTableListBox* engineerTryWarnTableListBox;

extern ExternalMachine engineerMachine;

static void EngineerTryUpdateMachineStatus(void)
{
    char buf[32];

    ituSpriteGoto(engineerTryPowerStatusSprite, engineerMachine.powerStatus);
    ituSpriteGoto(engineerTryModeSprite, engineerMachine.mode);

    sprintf(buf, "%.1f", engineerMachine.indoorTemperature);
    ituTextSetString(engineerTryIndoorTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.outdoorTemperature);
    ituTextSetString(engineerTryOutdoorTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.exhaustTemperature);
    ituTextSetString(engineerTryExhaustTemperatureText, buf);

    sprintf(buf, "%d", engineerMachine.compressorTargetFreq);
    ituTextSetString(engineerTryCompressorTargetFreqText, buf);

    sprintf(buf, "%d", engineerMachine.wifiStrength);
    ituTextSetString(engineerTryWiFiStrengthText, buf);

    sprintf(buf, "%d", engineerMachine.radioID0);
    ituTextSetString(engineerTryRadioID0Text, buf);

    sprintf(buf, "%d", engineerMachine.radioID1);
    ituTextSetString(engineerTryRadioID1Text, buf);

    sprintf(buf, "%.1f", engineerMachine.temperature);
    ituTextSetString(engineerTryTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.evaporatorTemperature);
    ituTextSetString(engineerTryEvaporatorTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.condenserTemperature);
    ituTextSetString(engineerTryCondenserTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.returnAirTemperature);
    ituTextSetString(engineerTryReturnAirTemperatureText, buf);

    sprintf(buf, "%d", engineerMachine.compressorActualFreq);
    ituTextSetString(engineerTryCompressorActualFreqText, buf);

    sprintf(buf, "%.1f", engineerMachine.current);
    ituTextSetString(engineerTryCurrentText, buf);

    sprintf(buf, "%.1f", engineerMachine.voltage);
    ituTextSetString(engineerTryVoltageText, buf);

    sprintf(buf, "%d", engineerMachine.radioID2);
    ituTextSetString(engineerTryRadioID2Text, buf);

    ituListBoxReload((ITUListBox*)engineerTryWarnTableListBox);
}

bool EngineerTryWarnTableListBoxOnLoad(ITUWidget* widget, char* param)
{
    ITCTree* node;
    ITUListBox* listbox = (ITUListBox*)widget;
    ITUTableListBox* tlistbox = (ITUTableListBox*)widget;
    int i, j, count;

    if (listbox->pageIndex == 0)
    {
        // initialize
        listbox->pageIndex = 1;
		listbox->focusIndex = -1;
        listbox->pageCount = 1;
    }
    count = EXTERNAL_WARN_COUNT;
    node = ((ITCTree*)tlistbox)->child;
    j = 0;
    for (i = 0; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;

        if (engineerMachine.warnings[i])
        {
            ituTextSetString(scrolltext, StringGetWarning(i));
            node = node->sibling;
            j++;
        }
    }

    for (i = j; i < count; i++)
    {
        ITUScrollText* scrolltext = (ITUScrollText*) node;
        ituTextSetString(scrolltext, "");
        node = node->sibling;
    }
    listbox->itemCount = j;

    return true;
}

bool EngineerTryOnTimer(ITUWidget* widget, char* param)
{
    if (memcmp(&engineerMachine, &extMachine, sizeof(ExternalMachine)))
    {
        memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
        EngineerTryUpdateMachineStatus();
        return true;
    }
    return false;
}

bool EngineerTryOnEnter(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    if (!engineerTryPowerStatusSprite)
    {
        engineerTryPowerStatusSprite = ituSceneFindWidget(&theScene, "engineerTryPowerStatusSprite");
        assert(engineerTryPowerStatusSprite);
        
        engineerTryModeSprite = ituSceneFindWidget(&theScene, "engineerTryModeSprite");
        assert(engineerTryModeSprite);
        
        engineerTryIndoorTemperatureText = ituSceneFindWidget(&theScene, "engineerTryIndoorTemperatureText");
        assert(engineerTryIndoorTemperatureText);
        
        engineerTryOutdoorTemperatureText = ituSceneFindWidget(&theScene, "engineerTryOutdoorTemperatureText");
        assert(engineerTryOutdoorTemperatureText);
        
        engineerTryExhaustTemperatureText = ituSceneFindWidget(&theScene, "engineerTryExhaustTemperatureText");
        assert(engineerTryExhaustTemperatureText);
        
        engineerTryCompressorTargetFreqText = ituSceneFindWidget(&theScene, "engineerTryCompressorTargetFreqText");
        assert(engineerTryCompressorTargetFreqText);
        
        engineerTryWiFiStrengthText = ituSceneFindWidget(&theScene, "engineerTryWiFiStrengthText");
        assert(engineerTryWiFiStrengthText);
        
        engineerTryRadioID0Text = ituSceneFindWidget(&theScene, "engineerTryRadioID0Text");
        assert(engineerTryRadioID0Text);
        
        engineerTryRadioID1Text = ituSceneFindWidget(&theScene, "engineerTryRadioID1Text");
        assert(engineerTryRadioID1Text);
        
        engineerTryTemperatureText = ituSceneFindWidget(&theScene, "engineerTryTemperatureText");
        assert(engineerTryTemperatureText);
        
        engineerTryEvaporatorTemperatureText = ituSceneFindWidget(&theScene, "engineerTryEvaporatorTemperatureText");
        assert(engineerTryEvaporatorTemperatureText);
        
        engineerTryCondenserTemperatureText = ituSceneFindWidget(&theScene, "engineerTryCondenserTemperatureText");
        assert(engineerTryCondenserTemperatureText);
        
        engineerTryReturnAirTemperatureText = ituSceneFindWidget(&theScene, "engineerTryReturnAirTemperatureText");
        assert(engineerTryReturnAirTemperatureText);
        
        engineerTryCompressorActualFreqText = ituSceneFindWidget(&theScene, "engineerTryCompressorActualFreqText");
        assert(engineerTryCompressorActualFreqText);
        
        engineerTryCurrentText = ituSceneFindWidget(&theScene, "engineerTryCurrentText");
        assert(engineerTryCurrentText);
        
        engineerTryVoltageText = ituSceneFindWidget(&theScene, "engineerTryVoltageText");
        assert(engineerTryVoltageText);
        
        engineerTryRadioID2Text = ituSceneFindWidget(&theScene, "engineerTryRadioID2Text");
        assert(engineerTryRadioID2Text);
        
        engineerTryWarnTableListBox = ituSceneFindWidget(&theScene, "engineerTryWarnTableListBox");
        assert(engineerTryWarnTableListBox);
    }
    memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
    EngineerTryUpdateMachineStatus();

    ev.type = EXTERNAL_ENG_POWER_TRY_MODE;
    ev.arg1 = 1;
    ExternalSend(&ev);

    return true;
}

bool EngineerTryOnLeave(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_ENG_POWER_TRY_MODE;
    ev.arg1 = 0;
    ExternalSend(&ev);

    return true;
}

void EngineerTryReset(void)
{
    engineerTryPowerStatusSprite = NULL;
}
