#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "airconditioner.h"

static ITUSprite* engineerPowerStatusSprite;
static ITUSprite* engineerModeSprite;
static ITUText* engineerIndoorTemperatureText;
static ITUText* engineerOutdoorTemperatureText;
static ITUText* engineerExhaustTemperatureText;
static ITUText* engineerCompressorTargetFreqText;
static ITUText* engineerWiFiStrengthText;
static ITUText* engineerRadioID0Text;
static ITUText* engineerRadioID1Text;
static ITUText* engineerTemperatureText;
static ITUText* engineerEvaporatorTemperatureText;
static ITUText* engineerCondenserTemperatureText;
static ITUText* engineerReturnAirTemperatureText;
static ITUText* engineerCompressorActualFreqText;
static ITUText* engineerCurrentText;
static ITUText* engineerVoltageText;
static ITUText* engineerRadioID2Text;
static ITUTableListBox* engineerWarnTableListBox;

ExternalMachine engineerMachine;

static void EngineerUpdateMachineStatus(void)
{
    char buf[32];

    ituSpriteGoto(engineerPowerStatusSprite, engineerMachine.powerStatus);
    ituSpriteGoto(engineerModeSprite, engineerMachine.mode);

    sprintf(buf, "%.1f", engineerMachine.indoorTemperature);
    ituTextSetString(engineerIndoorTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.outdoorTemperature);
    ituTextSetString(engineerOutdoorTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.exhaustTemperature);
    ituTextSetString(engineerExhaustTemperatureText, buf);

    sprintf(buf, "%d", engineerMachine.compressorTargetFreq);
    ituTextSetString(engineerCompressorTargetFreqText, buf);

    sprintf(buf, "%d", engineerMachine.wifiStrength);
    ituTextSetString(engineerWiFiStrengthText, buf);

    sprintf(buf, "%d", engineerMachine.radioID0);
    ituTextSetString(engineerRadioID0Text, buf);

    sprintf(buf, "%d", engineerMachine.radioID1);
    ituTextSetString(engineerRadioID1Text, buf);

    sprintf(buf, "%.1f", engineerMachine.temperature);
    ituTextSetString(engineerTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.evaporatorTemperature);
    ituTextSetString(engineerEvaporatorTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.condenserTemperature);
    ituTextSetString(engineerCondenserTemperatureText, buf);

    sprintf(buf, "%.1f", engineerMachine.returnAirTemperature);
    ituTextSetString(engineerReturnAirTemperatureText, buf);

    sprintf(buf, "%d", engineerMachine.compressorActualFreq);
    ituTextSetString(engineerCompressorActualFreqText, buf);

    sprintf(buf, "%.1f", engineerMachine.current);
    ituTextSetString(engineerCurrentText, buf);

    sprintf(buf, "%.1f", engineerMachine.voltage);
    ituTextSetString(engineerVoltageText, buf);

    sprintf(buf, "%d", engineerMachine.radioID2);
    ituTextSetString(engineerRadioID2Text, buf);

    ituListBoxReload((ITUListBox*)engineerWarnTableListBox);
}

bool EngineerWarnTableListBoxOnLoad(ITUWidget* widget, char* param)
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

bool EngineerOnTimer(ITUWidget* widget, char* param)
{
    if (memcmp(&engineerMachine, &extMachine, sizeof(ExternalMachine)))
    {
        memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
        EngineerUpdateMachineStatus();
        return true;
    }
    return false;
}

bool EngineerOnEnter(ITUWidget* widget, char* param)
{
    if (!engineerPowerStatusSprite)
    {
        engineerPowerStatusSprite = ituSceneFindWidget(&theScene, "engineerPowerStatusSprite");
        assert(engineerPowerStatusSprite);
        
        engineerModeSprite = ituSceneFindWidget(&theScene, "engineerModeSprite");
        assert(engineerModeSprite);
        
        engineerIndoorTemperatureText = ituSceneFindWidget(&theScene, "engineerIndoorTemperatureText");
        assert(engineerIndoorTemperatureText);
        
        engineerOutdoorTemperatureText = ituSceneFindWidget(&theScene, "engineerOutdoorTemperatureText");
        assert(engineerOutdoorTemperatureText);
        
        engineerExhaustTemperatureText = ituSceneFindWidget(&theScene, "engineerExhaustTemperatureText");
        assert(engineerExhaustTemperatureText);
        
        engineerCompressorTargetFreqText = ituSceneFindWidget(&theScene, "engineerCompressorTargetFreqText");
        assert(engineerCompressorTargetFreqText);
        
        engineerWiFiStrengthText = ituSceneFindWidget(&theScene, "engineerWiFiStrengthText");
        assert(engineerWiFiStrengthText);
        
        engineerRadioID0Text = ituSceneFindWidget(&theScene, "engineerRadioID0Text");
        assert(engineerRadioID0Text);
        
        engineerRadioID1Text = ituSceneFindWidget(&theScene, "engineerRadioID1Text");
        assert(engineerRadioID1Text);
        
        engineerTemperatureText = ituSceneFindWidget(&theScene, "engineerTemperatureText");
        assert(engineerTemperatureText);
        
        engineerEvaporatorTemperatureText = ituSceneFindWidget(&theScene, "engineerEvaporatorTemperatureText");
        assert(engineerEvaporatorTemperatureText);
        
        engineerCondenserTemperatureText = ituSceneFindWidget(&theScene, "engineerCondenserTemperatureText");
        assert(engineerCondenserTemperatureText);
        
        engineerReturnAirTemperatureText = ituSceneFindWidget(&theScene, "engineerReturnAirTemperatureText");
        assert(engineerReturnAirTemperatureText);
        
        engineerCompressorActualFreqText = ituSceneFindWidget(&theScene, "engineerCompressorActualFreqText");
        assert(engineerCompressorActualFreqText);
        
        engineerCurrentText = ituSceneFindWidget(&theScene, "engineerCurrentText");
        assert(engineerCurrentText);
        
        engineerVoltageText = ituSceneFindWidget(&theScene, "engineerVoltageText");
        assert(engineerVoltageText);
        
        engineerRadioID2Text = ituSceneFindWidget(&theScene, "engineerRadioID2Text");
        assert(engineerRadioID2Text);
        
        engineerWarnTableListBox = ituSceneFindWidget(&theScene, "engineerWarnTableListBox");
        assert(engineerWarnTableListBox);
    }
    memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
    EngineerUpdateMachineStatus();
    return true;
}

bool EngineerOnLeave(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_ENG_MODE;
    ev.arg1 = 0;
    ExternalSend(&ev);

    return true;
}

void EngineerReset(void)
{
    engineerPowerStatusSprite = NULL;
}
