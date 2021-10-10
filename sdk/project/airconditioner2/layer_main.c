#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ite/itp.h"
#include "scene.h"
#include "airconditioner.h"

// need to install VS90SP1-KB980263-x86.exe for vs2008
#pragma execution_character_set("utf-8")

#define SHOW_TEMPERATURE_DELAY  (4 * 1000 / MS_PER_FRAME)
#define MIN_TEMPERATURE         17
#define DEFAULT_UNLOCK_VALUE    50
#define UNLOCK_VALUE            10
#define CONFIG_SAVE_DELAY       (4 * 1000 / MS_PER_FRAME)

static ITULayer* mainLayer;
static ITULayer* engineerLayer;
static ITUAnimation* mainAnimation;
static ITUBackgroundButton* mainBackgroundButton;
static ITUContainer* mainRightContainer;
static ITUSprite* mainBackgroundSprite;
static ITUContainer* barTemperatureOutdoorContainer;
static ITUText* barTemperatureOutdoorText;
static ITUContainer* barTemperatureIndoorContainer;
static ITUText* barTemperatureIndoorText;
static ITUSprite* mainCoolTemperatureSprite;
static ITUSprite* mainDryTemperatureSprite;
static ITUSprite* mainHeatTemperatureSprite;
static ITUSprite* mainAutoTemperatureSprite;
static ITUSprite* mainPureSprite;
static ITUWheel* mainTemperatureWheel;
static ITUButton* mainFoldButton;
static ITUAnimation* mainCoolAnimation;
static ITUAnimation* mainDryAnimation;
static ITUAnimation* mainHeatAnimation;
static ITUAnimation* mainFanAnimation;
static ITUAnimation* mainAutoAnimation;
static ITUButton* mainCoolSettingButton;
static ITUButton* mainCoolBackButton;
static ITUButton* mainDrySettingButton;
static ITUButton* mainDryBackButton;
static ITUButton* mainHeatSettingButton;
static ITUButton* mainHeatBackButton;
static ITUButton* mainFanSettingButton;
static ITUButton* mainFanBackButton;
static ITUButton* mainAutoSettingButton;
static ITUButton* mainAutoBackButton;
static ITUButton* mainCoolWindButton;
static ITUButton* mainDryWindButton;
static ITUButton* mainHeatWindButton;
static ITUButton* mainFanWindButton;
static ITUButton* mainAutoWindButton;
static ITUProgressBar* mainPowerProgressBar;
static ITUTrackBar* mainPowerTrackBar;
static ITUBackground* mainPowerOnBackground;
static ITUBackground* mainPowerOffBackground;
static ITUProgressBar* mainWindProgressBar;
static ITUTrackBar* mainWindTrackBar;
static ITUBackground* mainWarnBackground;
static ITUText* mainWarn1Text;
static ITUText* mainWarn2Text;
static ITUText* mainWarn3Text;
static ITUBackgroundButton* mainLockBackgroundButton;
static ITUBackground* mainLockBackground;
static ITUTrackBar* mainUnlockTrackBar;

static ITUIcon* barTimeIcon;
static ITULayer* settingLayer;
static ITULayer* settingTimeLayer;
static ITULayer* settingFuncLayer;
static ITULayer* settingSystemLayer;

bool mainPowerOn;
static int mainRightContainerX, mainShowTemperatureDelay, mainConfigSaveDelay;
static bool mainPureOn;
static uint32_t mainSettingTick, mainLockTick;
static time_t mainSettingTime;

static void MainBack(void)
{
    mainSettingTick = mainSettingTime = 0;

    ituWidgetSetVisible(mainCoolSettingButton, true);
    ituWidgetSetVisible(mainDrySettingButton, true);
    ituWidgetSetVisible(mainHeatSettingButton, true);
    ituWidgetSetVisible(mainFanSettingButton, true);
    ituWidgetSetVisible(mainAutoSettingButton, true);

    ituWidgetSetVisible(mainCoolBackButton, false);
    ituWidgetSetVisible(mainDryBackButton, false);
    ituWidgetSetVisible(mainHeatBackButton, false);
    ituWidgetSetVisible(mainFanBackButton, false);
    ituWidgetSetVisible(mainAutoBackButton, false);

    ituWidgetEnable(mainAnimation);
    ituAnimationReversePlay(mainAnimation, 1);

    SettingOnLeave(true);
}

static void MainProcessExternalWarn(ExternalEvent* ev)
{
    assert(ev);

    if (ev->arg1 == EXTERNAL_WARN_NONE)
    {
        ituWidgetSetVisible(mainWarnBackground, false);
    }
    else
    {
        ituWidgetSetVisible(mainWarnBackground, true);
        ituTextSetString(mainWarn1Text, StringGetWarning(ev->arg1));
        ituTextResize(mainWarn1Text);

        if (ev->arg2 == EXTERNAL_WARN_NONE)
        {
            ituTextSetString(mainWarn2Text, "");
            ituTextSetString(mainWarn3Text, "");
        }
        else
        {
            ituTextSetString(mainWarn2Text, StringGetWarning(ev->arg2));
            ituTextResize(mainWarn2Text);

            if (ev->arg3 == EXTERNAL_WARN_NONE)
            {
                ituTextSetString(mainWarn3Text, "");
            }
            else
            {
                ituTextSetString(mainWarn3Text, StringGetWarning(ev->arg3));
                ituTextResize(mainWarn3Text);
            }
        }
    }
}

static void MainProcessExternalTemperatureIndoor(ExternalEvent* ev)
{
    char buf[32];
    assert(ev);

    sprintf(buf, "%2.1f°C", *(float*)&ev->arg1);
    ituTextSetString(barTemperatureIndoorText, buf);
    ituTextResize(barTemperatureIndoorText);
}

static void MainProcessExternalTemperatureOutdoor(ExternalEvent* ev)
{
    char buf[32];
    assert(ev);

    sprintf(buf, "%2.1f°C", *(float*)&ev->arg1);
    ituTextSetString(barTemperatureOutdoorText, buf);
    ituTextResize(barTemperatureOutdoorText);
}

static void MainProcessExternalPM25(ExternalEvent* ev)
{
    if (ev->arg1)
    {
        ituSpriteGoto(mainPureSprite, 1);

        AudioPlaySound(SOUND_PURE_ON);

        mainPureOn = true;
    }
    else
    {
        ituSpriteGoto(mainPureSprite, 0);

        AudioPlaySound(SOUND_PURE_OFF);

        mainPureOn = false;
    }
    if (mainSettingTime != 0)
        MainBack();
}

static void MainSetModeChecked(ITUAnimation* parent, bool checked)
{
    ITUWidget* widget = (ITUWidget*)parent;
    ITCTree* node;

    for (node = widget->tree.child; node; node = node->sibling)
    {
        ITUCheckBox* checkbox = (ITUCheckBox*)node;
        ituCheckBoxSetChecked(checkbox, checked);
    }
}

static void MainSetMode(ExternalMode mode)
{
    switch (mode)
    {
    case EXTERNAL_MODE_COOL:
        MainSetModeChecked(mainCoolAnimation, true);
        MainSetModeChecked(mainDryAnimation, false);
        MainSetModeChecked(mainHeatAnimation, false);
        MainSetModeChecked(mainFanAnimation, false);
        MainSetModeChecked(mainAutoAnimation, false);
        break;

    case EXTERNAL_MODE_DRY:
        MainSetModeChecked(mainCoolAnimation, false);
        MainSetModeChecked(mainDryAnimation, true);
        MainSetModeChecked(mainHeatAnimation, false);
        MainSetModeChecked(mainFanAnimation, false);
        MainSetModeChecked(mainAutoAnimation, false);
        break;

    case EXTERNAL_MODE_HEAT:
        MainSetModeChecked(mainCoolAnimation, false);
        MainSetModeChecked(mainDryAnimation, false);
        MainSetModeChecked(mainHeatAnimation, true);
        MainSetModeChecked(mainFanAnimation, false);
        MainSetModeChecked(mainAutoAnimation, false);
        break;

    case EXTERNAL_MODE_FAN:
        MainSetModeChecked(mainCoolAnimation, false);
        MainSetModeChecked(mainDryAnimation, false);
        MainSetModeChecked(mainHeatAnimation, false);
        MainSetModeChecked(mainFanAnimation, true);
        MainSetModeChecked(mainAutoAnimation, false);
        break;

    case EXTERNAL_MODE_AUTO:
        MainSetModeChecked(mainCoolAnimation, false);
        MainSetModeChecked(mainDryAnimation, false);
        MainSetModeChecked(mainHeatAnimation, false);
        MainSetModeChecked(mainFanAnimation, false);
        MainSetModeChecked(mainAutoAnimation, true);
        break;
    }

    if (mode != theConfig.mode)
    {
        ExternalEvent ev;

        ev.type = EXTERNAL_MODE;
        ev.arg1 = mode;
        ExternalSend(&ev);

        theConfig.mode = mode;
        mainConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
}

static void MainProcessExternalMode(ExternalEvent* ev)
{
    ExternalMode mode = ev->arg1;

    ituSpriteGoto(mainBackgroundSprite, mode);
    MainSetMode(mode);

    if (mode == EXTERNAL_MODE_COOL)
    {
        AudioPlaySound(SOUND_MODE_COOL);
    }
    else if (mode == EXTERNAL_MODE_DRY)
    {
        AudioPlaySound(SOUND_MODE_DRY);
    }
    else if (mode == EXTERNAL_MODE_HEAT)
    {
        AudioPlaySound(SOUND_MODE_HEAT);
    }
    else if (mode == EXTERNAL_MODE_FAN)
    {
        AudioPlaySound(SOUND_MODE_FAN);
    }
    else if (mode == EXTERNAL_MODE_AUTO)
    {
        AudioPlaySound(SOUND_MODE_AUTO);
    }
    if (mainSettingTime != 0)
        MainBack();
}

static void MainProcessExternalTemperature(ExternalEvent* ev)
{
    int temperature = (int)*(float*)&ev->arg1;

    if (temperature != theConfig.temperature)
    {
        theConfig.temperature = temperature;
        mainConfigSaveDelay = CONFIG_SAVE_DELAY;

        AudioPlaySound(SOUND_TEMPERATURE_17 + theConfig.temperature - MIN_TEMPERATURE);

        ituSpriteGoto(mainCoolTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);
        ituSpriteGoto(mainDryTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);
        ituSpriteGoto(mainHeatTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);
        ituSpriteGoto(mainAutoTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);

        ituWidgetEnable(mainTemperatureWheel);
        ituWheelGoto(mainTemperatureWheel, theConfig.temperature - MIN_TEMPERATURE);
    }
    if (mainSettingTime != 0)
        MainBack();
}

static void MainProcessExternalPower(ExternalEvent* ev)
{
    bool powerOn = ev->arg1 ? true : false;

    if (powerOn)
    {
        AudioPlaySound(SOUND_POWER_ON);

        if (theConfig.power_on_enable || theConfig.power_off_enable)
        {
            if (!theConfig.power_mon_enable && 
                !theConfig.power_tue_enable && 
                !theConfig.power_wed_enable && 
                !theConfig.power_thu_enable && 
                !theConfig.power_fri_enable && 
                !theConfig.power_sat_enable && 
                !theConfig.power_sun_enable)
            {
                ituWidgetSetVisible(barTimeIcon, false);
                theConfig.power_on_enable = theConfig.power_off_enable = false;
                mainConfigSaveDelay = CONFIG_SAVE_DELAY;
            }
        }

        ituWidgetSetVisible(mainCoolWindButton, true);
        ituWidgetSetVisible(mainDryWindButton, true);
        ituWidgetSetVisible(mainHeatWindButton, true);
        ituWidgetSetVisible(mainFanWindButton, true);
        ituWidgetSetVisible(mainAutoWindButton, true);
        ituWidgetSetVisible(mainPowerOnBackground, true);

        ituProgressBarSetValue(mainPowerProgressBar, 100);
        ituTrackBarSetValue(mainPowerTrackBar, 100);

        mainPowerOn = true;
    }
    else
    {
        AudioPlaySound(SOUND_POWER_OFF);

        if (theConfig.power_on_enable || theConfig.power_off_enable)
        {
            if (!theConfig.power_mon_enable && 
                !theConfig.power_tue_enable && 
                !theConfig.power_wed_enable && 
                !theConfig.power_thu_enable && 
                !theConfig.power_fri_enable && 
                !theConfig.power_sat_enable && 
                !theConfig.power_sun_enable)
            {
                ituWidgetSetVisible(barTimeIcon, false);
                theConfig.power_on_enable = theConfig.power_off_enable = false;
                mainConfigSaveDelay = CONFIG_SAVE_DELAY;
            }
        }

        ituWidgetSetVisible(mainCoolWindButton, false);
        ituWidgetSetVisible(mainDryWindButton, false);
        ituWidgetSetVisible(mainHeatWindButton, false);
        ituWidgetSetVisible(mainFanWindButton, false);
        ituWidgetSetVisible(mainAutoWindButton, false);
        ituWidgetSetVisible(mainPowerOffBackground, true);

        ituProgressBarSetValue(mainPowerProgressBar, 0);
        ituTrackBarSetValue(mainPowerTrackBar, 0);

        mainPowerOn = false;
    }
    if (mainSettingTime != 0)
        MainBack();
}

static void MainProcessExternalWind(ExternalEvent* ev)
{
    int wind = ev->arg1;

    if (wind != theConfig.wind)
    {
        theConfig.wind = wind;
        mainConfigSaveDelay = CONFIG_SAVE_DELAY;

        ituProgressBarSetValue(mainWindProgressBar, theConfig.wind);
        ituTrackBarSetValue(mainWindTrackBar, theConfig.wind);

        if (wind == 0)
            AudioPlaySound(SOUND_WIND_AUTO);
        else if (wind == 100)
            AudioPlaySound(SOUND_WIND_MAX);
        else
            AudioPlaySound(SOUND_WIND);
    }
    if (mainSettingTime != 0)
        MainBack();
}

static void MainProcessExternalEngineerMode(ExternalEvent* ev)
{
    if (!mainLayer)
        return;

    if (ev->arg1)
        ituLayerGoto(engineerLayer);
    else
        ituLayerGoto(mainLayer);
}

void MainProcessExternalEvent(ExternalEvent* ev)
{
    assert(ev);
    switch (ev->type)
    {
    case EXTERNAL_WARN:
        printf("EXTERNAL_WARN: %d %d %d\n", ev->arg1, ev->arg2, ev->arg3);
        MainProcessExternalWarn(ev);
        break;

    case EXTERNAL_TEMPERATURE_INDOOR:
        printf("EXTERNAL_TEMPERATURE_INDOOR: %d\n", ev->arg1);
        MainProcessExternalTemperatureIndoor(ev);
        break;

    case EXTERNAL_TEMPERATURE_OUTDOOR:
        printf("EXTERNAL_TEMPERATURE_OUTDOOR: %d\n", ev->arg1);
        MainProcessExternalTemperatureOutdoor(ev);
        break;

    case EXTERNAL_PM25:
        printf("EXTERNAL_PM25: %d\n", ev->arg1);
        MainProcessExternalPM25(ev);
        break;

    case EXTERNAL_MODE:
        printf("EXTERNAL_MODE: %d\n", ev->arg1);
        MainProcessExternalMode(ev);
        break;

    case EXTERNAL_TEMPERATURE:
        printf("EXTERNAL_TEMPERATURE: %d\n", ev->arg1);
        MainProcessExternalTemperature(ev);
        break;

    case EXTERNAL_POWER:
        printf("EXTERNAL_POWER: %d\n", ev->arg1);
        MainProcessExternalPower(ev);
        break;

    case EXTERNAL_WIND:
        printf("EXTERNAL_WIND: %d\n", ev->arg1);
        MainProcessExternalWind(ev);
        break;

    case EXTERNAL_ENG_MODE:
        printf("EXTERNAL_ENG_MODE: %d\n", ev->arg1);
        MainProcessExternalEngineerMode(ev);
        break;
    }
}

void MainRefreshTime(void)
{
    if (mainSettingTime != 0)
    {
        time(&mainSettingTime);
        mainSettingTick = itpGetTickCount();
    }
}

void MainPowerOn(void)
{
    if (!mainPowerOn)
    {
        ituProgressBarSetValue(mainPowerProgressBar, 100);
        ituTrackBarSetValue(mainPowerTrackBar, 100);
    }
    SettingPowerOn();
}

void MainPowerOff(void)
{
    if (mainPowerOn)
    {
        ituProgressBarSetValue(mainPowerProgressBar, 0);
        ituTrackBarSetValue(mainPowerTrackBar, 0);
    }
    SettingPowerOff();
}

bool MainPureOffButtonOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_PM25;
    ev.arg1 = 1;

    ExternalSend(&ev);

    AudioPlaySound(SOUND_PURE_ON);

    return true;
}

bool MainPureOnButtonOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_PM25;
    ev.arg1 = 0;

    ExternalSend(&ev);

    AudioPlaySound(SOUND_PURE_OFF);

    return true;
}

bool MainTemperatureWheelOnChanged(ITUWidget* widget, char* param)
{
    ITUWheel* wheel = (ITUWheel*) widget;
    
    if (ituWheelCheckIdle(wheel))
    {
        int temperature = wheel->focusIndex + MIN_TEMPERATURE;
        //printf("temperature:%d\n", temperature);
        if (temperature != theConfig.temperature)
        {
            ExternalEvent ev;

            ev.type = EXTERNAL_TEMPERATURE;
            ev.arg1 = temperature;
            ExternalSend(&ev);

            AudioPlaySound(SOUND_TEMPERATURE_17 + wheel->focusIndex);

            theConfig.temperature = temperature;
            mainConfigSaveDelay = CONFIG_SAVE_DELAY;

            return true;
        }
    }
    return false;
}

bool MainModePopupRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ITUAnimation* parent = (ITUAnimation*)widget->tree.parent;

    if (parent == mainCoolAnimation)
    {
        MainSetMode(EXTERNAL_MODE_COOL);
        AudioPlaySound(SOUND_MODE_COOL);
    }
    else if (parent == mainDryAnimation)
    {
        MainSetMode(EXTERNAL_MODE_DRY);
        AudioPlaySound(SOUND_MODE_DRY);
    }
    else if (parent == mainHeatAnimation)
    {
        MainSetMode(EXTERNAL_MODE_HEAT);
        AudioPlaySound(SOUND_MODE_HEAT);
    }
    else if (parent == mainFanAnimation)
    {
        MainSetMode(EXTERNAL_MODE_FAN);
        AudioPlaySound(SOUND_MODE_FAN);
    }
    else if (parent == mainAutoAnimation)
    {
        MainSetMode(EXTERNAL_MODE_AUTO);
        AudioPlaySound(SOUND_MODE_AUTO);
    }
    return true;
}

bool MainPowerTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = mainPowerTrackBar->value;

    //printf("power value: %d\n", value);

    if (value <= 50)
    {
        if (mainPowerOn)
        {
            ExternalEvent ev;

            ev.type = EXTERNAL_POWER;
            ev.arg1 = 0;
            ExternalSend(&ev);

            AudioPlaySound(SOUND_POWER_OFF);

            if (theConfig.power_on_enable || theConfig.power_off_enable)
            {
                if (!theConfig.power_mon_enable && 
                    !theConfig.power_tue_enable && 
                    !theConfig.power_wed_enable && 
                    !theConfig.power_thu_enable && 
                    !theConfig.power_fri_enable && 
                    !theConfig.power_sat_enable && 
                    !theConfig.power_sun_enable)
                {
                    ituWidgetSetVisible(barTimeIcon, false);
                    theConfig.power_on_enable = theConfig.power_off_enable = false;
                    mainConfigSaveDelay = CONFIG_SAVE_DELAY;
                }
            }

            ituWidgetSetVisible(mainCoolWindButton, false);
            ituWidgetSetVisible(mainDryWindButton, false);
            ituWidgetSetVisible(mainHeatWindButton, false);
            ituWidgetSetVisible(mainFanWindButton, false);
            ituWidgetSetVisible(mainAutoWindButton, false);
            ituWidgetSetVisible(mainPowerOffBackground, true);
            mainPowerOn = false;

            SettingPowerOff();
        }
    }
    else if (value > 50)
    {
        if (!mainPowerOn)
        {
            ExternalEvent ev;

            ev.type = EXTERNAL_POWER;
            ev.arg1 = 1;
            ExternalSend(&ev);

            AudioPlaySound(SOUND_POWER_ON);

            if (theConfig.power_on_enable || theConfig.power_off_enable)
            {
                if (!theConfig.power_mon_enable && 
                    !theConfig.power_tue_enable && 
                    !theConfig.power_wed_enable && 
                    !theConfig.power_thu_enable && 
                    !theConfig.power_fri_enable && 
                    !theConfig.power_sat_enable && 
                    !theConfig.power_sun_enable)
                {
                    ituWidgetSetVisible(barTimeIcon, false);
                    theConfig.power_on_enable = theConfig.power_off_enable = false;
                    mainConfigSaveDelay = CONFIG_SAVE_DELAY;
                }
            }

            ituWidgetSetVisible(mainCoolWindButton, true);
            ituWidgetSetVisible(mainDryWindButton, true);
            ituWidgetSetVisible(mainHeatWindButton, true);
            ituWidgetSetVisible(mainFanWindButton, true);
            ituWidgetSetVisible(mainAutoWindButton, true);
            ituWidgetSetVisible(mainPowerOnBackground, true);
            mainPowerOn = true;

            SettingPowerOn();
        }
    }
    return true;
}

static bool MainPowerTrackBarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    if (ev == ITU_EVENT_MOUSEUP)
    {
        if (mainPowerOn)
        {
            ituProgressBarSetValue(mainPowerProgressBar, 100);
            ituTrackBarSetValue(mainPowerTrackBar, 100);
        }
        else
        {
            ituProgressBarSetValue(mainPowerProgressBar, 0);
            ituTrackBarSetValue(mainPowerTrackBar, 0);
        }
    }
    return ituTrackBarUpdate(widget, ev, arg1, arg2, arg3);
}

static void MainWindTrackBarOnValueChanged(ITUTrackBar* tbar, int value, bool confirm)
{
    if (!confirm)
        return;

    if (value < 0 || value > 100)
        return;

    if (value != theConfig.wind)
    {
        ExternalEvent ev;

        ev.type = EXTERNAL_WIND;
        ev.arg1 = value;
        ExternalSend(&ev);

        if (value == 0)
            AudioPlaySound(SOUND_WIND_AUTO);
        else if (value == 100)
            AudioPlaySound(SOUND_WIND_MAX);
        else
            AudioPlaySound(SOUND_WIND);

        theConfig.wind = value;
        mainConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
}

bool MainAutoWindButtonOnPress(ITUWidget* widget, char* param)
{
    int value = 0;

    if (value != theConfig.wind)
    {
        ExternalEvent ev;

        ev.type = EXTERNAL_WIND;
        ev.arg1 = value;
        ExternalSend(&ev);

        theConfig.wind = value;
        mainConfigSaveDelay = CONFIG_SAVE_DELAY;
    }
    return true;
}

bool MainAnimationOnStop(ITUWidget* widget, char* param)
{
    bool ret;

    itcTreeRotateFront((ITCTree*)mainAnimation);

    if (mainAnimation->animationFlags & ITU_ANIM_REVERSE)
    {
        ituWidgetSetX(mainBackgroundButton, 0);
        ituWidgetSetX(mainRightContainer, mainRightContainerX);
        ituWidgetSetVisible(mainFoldButton, true);
        ituWidgetSetVisible(mainPureSprite, true);
        mainSettingTick = mainSettingTime = 0;

        SettingOnLeave(false);

        ret = true;
    }
    else
    {
        ituWidgetSetX(mainBackgroundButton, mainRightContainerX);
        ituWidgetSetX(mainRightContainer, 0);
        time(&mainSettingTime);
        mainSettingTick = itpGetTickCount();
        ret = false;
    }
    mainAnimation->child = (ITUWidget*)mainBackgroundButton;
    return ret;
}

bool MainBackgroundButtonOnLongPress(ITUWidget* widget, char* param)
{
    if (mainSettingTime != 0)
        return false;

    mainLockTick = itpGetTickCount();

    return true;
}

bool MainLockBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
    mainLockTick = 0;
    ituTrackBarSetValue(mainUnlockTrackBar, DEFAULT_UNLOCK_VALUE);
    return true;
}

static bool MainUnlockTrackBarUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    if (ev == ITU_EVENT_MOUSEUP && ituWidgetIsVisible(mainLockBackground))
    {
        if (mainUnlockTrackBar->value <= UNLOCK_VALUE)
        {
            ituWidgetSetVisible(mainLockBackground, false);
            ituWidgetEnable(mainBackgroundButton);
        }
        else
        {
            ituTrackBarSetValue(mainUnlockTrackBar, DEFAULT_UNLOCK_VALUE);
        }
    }
    return ituTrackBarUpdate(widget, ev, arg1, arg2, arg3);
}

bool MainOnTimer(ITUWidget* widget, char* param)
{
    bool result = false;

    if (--mainShowTemperatureDelay <= 0)
    {
        if (ituWidgetIsVisible(barTemperatureOutdoorContainer))
        {
            ituWidgetSetVisible(barTemperatureOutdoorContainer, false);
            ituWidgetSetVisible(barTemperatureIndoorContainer, true);
        }
        else
        {
            ituWidgetSetVisible(barTemperatureOutdoorContainer, true);
            ituWidgetSetVisible(barTemperatureIndoorContainer, false);
        }
        mainShowTemperatureDelay = SHOW_TEMPERATURE_DELAY;

        result = true;
    }

    if (mainConfigSaveDelay > 0)
    {
        if (--mainConfigSaveDelay <= 0)
        {
            mainConfigSaveDelay = 0;

            ConfigSave();
        }
    }

    if (itpGetTickDuration(mainSettingTick) >= 1000)
    {
        mainSettingTick = itpGetTickCount();

        if (mainSettingTime != 0)
        {
            int seconds;
            time_t now;

            time(&now);

            seconds = (int)difftime(now, mainSettingTime);
            if (seconds >= 60)
            {
                MainBack();
                result = true;
            }
        }
        SettingOnSecond();
    }

    if (mainLockTick > 0)
    {
        if (itpGetTickDuration(mainLockTick) >= 2000)
        {
            ituWidgetSetVisible(mainLockBackgroundButton, false);
            ituWidgetEnable(mainBackgroundButton);

            mainLockTick = 0;
            result = true;
        }
    }
    return result;
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
    if (!mainLayer)
    {
        mainLayer = ituSceneFindWidget(&theScene, "mainLayer");
        assert(mainLayer);

        engineerLayer = ituSceneFindWidget(&theScene, "engineerLayer");
        assert(engineerLayer);

        mainAnimation = ituSceneFindWidget(&theScene, "mainAnimation");
        assert(mainAnimation);

        mainBackgroundButton = ituSceneFindWidget(&theScene, "mainBackgroundButton");
        assert(mainBackgroundButton);

        mainRightContainer = ituSceneFindWidget(&theScene, "mainRightContainer");
        assert(mainRightContainer);

        mainBackgroundSprite = ituSceneFindWidget(&theScene, "mainBackgroundSprite");
        assert(mainBackgroundSprite);

        barTemperatureOutdoorContainer = ituSceneFindWidget(&theScene, "barTemperatureOutdoorContainer");
        assert(barTemperatureOutdoorContainer);

        barTemperatureOutdoorText = ituSceneFindWidget(&theScene, "barTemperatureOutdoorText");
        assert(barTemperatureOutdoorText);

        barTemperatureIndoorContainer = ituSceneFindWidget(&theScene, "barTemperatureIndoorContainer");
        assert(barTemperatureIndoorContainer);

        barTemperatureIndoorText = ituSceneFindWidget(&theScene, "barTemperatureIndoorText");
        assert(barTemperatureIndoorText);

        mainCoolTemperatureSprite = ituSceneFindWidget(&theScene, "mainCoolTemperatureSprite");
        assert(mainCoolTemperatureSprite);

        mainDryTemperatureSprite = ituSceneFindWidget(&theScene, "mainDryTemperatureSprite");
        assert(mainDryTemperatureSprite);

        mainHeatTemperatureSprite = ituSceneFindWidget(&theScene, "mainHeatTemperatureSprite");
        assert(mainHeatTemperatureSprite);

        mainAutoTemperatureSprite = ituSceneFindWidget(&theScene, "mainAutoTemperatureSprite");
        assert(mainAutoTemperatureSprite);

        mainPureSprite = ituSceneFindWidget(&theScene, "mainPureSprite");
        assert(mainPureSprite);

        mainTemperatureWheel = ituSceneFindWidget(&theScene, "mainTemperatureWheel");
        assert(mainTemperatureWheel);

        mainFoldButton = ituSceneFindWidget(&theScene, "mainFoldButton");
        assert(mainFoldButton);

        mainCoolAnimation = ituSceneFindWidget(&theScene, "mainCoolAnimation");
        assert(mainCoolAnimation);

        mainDryAnimation = ituSceneFindWidget(&theScene, "mainDryAnimation");
        assert(mainDryAnimation);

        mainHeatAnimation = ituSceneFindWidget(&theScene, "mainHeatAnimation");
        assert(mainHeatAnimation);

        mainFanAnimation = ituSceneFindWidget(&theScene, "mainFanAnimation");
        assert(mainFanAnimation);

        mainAutoAnimation = ituSceneFindWidget(&theScene, "mainAutoAnimation");
        assert(mainAutoAnimation);

        mainCoolWindButton = ituSceneFindWidget(&theScene, "mainCoolWindButton");
        assert(mainCoolWindButton);

        mainCoolSettingButton = ituSceneFindWidget(&theScene, "mainCoolSettingButton");
        assert(mainCoolSettingButton);

        mainCoolBackButton = ituSceneFindWidget(&theScene, "mainCoolBackButton");
        assert(mainCoolBackButton);

        mainDrySettingButton = ituSceneFindWidget(&theScene, "mainDrySettingButton");
        assert(mainDrySettingButton);

        mainDryBackButton = ituSceneFindWidget(&theScene, "mainDryBackButton");
        assert(mainDryBackButton);

        mainHeatSettingButton = ituSceneFindWidget(&theScene, "mainHeatSettingButton");
        assert(mainHeatSettingButton);

        mainHeatBackButton = ituSceneFindWidget(&theScene, "mainHeatBackButton");
        assert(mainHeatBackButton);

        mainFanSettingButton = ituSceneFindWidget(&theScene, "mainFanSettingButton");
        assert(mainFanSettingButton);

        mainFanBackButton = ituSceneFindWidget(&theScene, "mainFanBackButton");
        assert(mainFanBackButton);

        mainAutoSettingButton = ituSceneFindWidget(&theScene, "mainAutoSettingButton");
        assert(mainAutoSettingButton);

        mainAutoBackButton = ituSceneFindWidget(&theScene, "mainAutoBackButton");
        assert(mainAutoBackButton);

        mainDryWindButton = ituSceneFindWidget(&theScene, "mainDryWindButton");
        assert(mainDryWindButton);

        mainHeatWindButton = ituSceneFindWidget(&theScene, "mainHeatWindButton");
        assert(mainHeatWindButton);

        mainFanWindButton = ituSceneFindWidget(&theScene, "mainFanWindButton");
        assert(mainFanWindButton);

        mainAutoWindButton = ituSceneFindWidget(&theScene, "mainAutoWindButton");
        assert(mainAutoWindButton);

        mainPowerProgressBar = ituSceneFindWidget(&theScene, "mainPowerProgressBar");
        assert(mainPowerProgressBar);

        mainPowerTrackBar = ituSceneFindWidget(&theScene, "mainPowerTrackBar");
        assert(mainPowerTrackBar);
        ituWidgetSetUpdate(mainPowerTrackBar, MainPowerTrackBarUpdate);

        mainPowerOnBackground = ituSceneFindWidget(&theScene, "mainPowerOnBackground");
        assert(mainPowerOnBackground);

        mainPowerOffBackground = ituSceneFindWidget(&theScene, "mainPowerOffBackground");
        assert(mainPowerOffBackground);

        mainWindProgressBar = ituSceneFindWidget(&theScene, "mainWindProgressBar");
        assert(mainWindProgressBar);

        mainWindTrackBar = ituSceneFindWidget(&theScene, "mainWindTrackBar");
        assert(mainWindTrackBar);
        ituTrackBarSetValueChanged(mainWindTrackBar, MainWindTrackBarOnValueChanged);

        mainWarnBackground = ituSceneFindWidget(&theScene, "mainWarnBackground");
        assert(mainWarnBackground);

        mainWarn1Text = ituSceneFindWidget(&theScene, "mainWarn1Text");
        assert(mainWarn1Text);

        mainWarn2Text = ituSceneFindWidget(&theScene, "mainWarn2Text");
        assert(mainWarn2Text);

        mainWarn3Text = ituSceneFindWidget(&theScene, "mainWarn3Text");
        assert(mainWarn3Text);

        mainLockBackgroundButton = ituSceneFindWidget(&theScene, "mainLockBackgroundButton");
        assert(mainLockBackgroundButton);

        mainLockBackground = ituSceneFindWidget(&theScene, "mainLockBackground");
        assert(mainLockBackground);

        mainUnlockTrackBar = ituSceneFindWidget(&theScene, "mainUnlockTrackBar");
        assert(mainUnlockTrackBar);
        ituWidgetSetUpdate(mainUnlockTrackBar, MainUnlockTrackBarUpdate);

        barTimeIcon = ituSceneFindWidget(&theScene, "barTimeIcon");
        assert(barTimeIcon);

        settingLayer = ituSceneFindWidget(&theScene, "settingLayer");
        assert(settingLayer);

        settingTimeLayer = ituSceneFindWidget(&theScene, "settingTimeLayer");
        assert(settingTimeLayer);

        settingFuncLayer = ituSceneFindWidget(&theScene, "settingFuncLayer");
        assert(settingFuncLayer);

        settingSystemLayer = ituSceneFindWidget(&theScene, "settingSystemLayer");
        assert(settingSystemLayer);

        if (!theConfig.sound_enable)
            AudioMute();

        if (theConfig.power_on_enable || theConfig.power_off_enable)
            ituWidgetSetVisible(barTimeIcon, true);

        mainPureOn = mainPowerOn = false;

        SceneSetReady(true);
    }

    if (mainSettingTime != 0)
    {
        //printf("back from video player\n");

        ituWidgetSetVisible(settingLayer, true);
        ituWidgetSetVisible(settingTimeLayer, true);
        ituWidgetSetVisible(settingFuncLayer, true);
        ituWidgetSetVisible(settingSystemLayer, true);

        time(&mainSettingTime);
        mainSettingTick = itpGetTickCount();
    }
    else
    {
        mainShowTemperatureDelay = SHOW_TEMPERATURE_DELAY;
        mainSettingTick = mainSettingTime = mainLockTick = 0;

        ituSpriteGoto(mainCoolTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);
        ituSpriteGoto(mainDryTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);
        ituSpriteGoto(mainHeatTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);
        ituSpriteGoto(mainAutoTemperatureSprite, theConfig.temperature - MIN_TEMPERATURE);

        ituSpriteGoto(mainPureSprite, mainPureOn ? 1 : 0);

        ituWidgetEnable(mainTemperatureWheel);
        ituWheelGoto(mainTemperatureWheel, theConfig.temperature - MIN_TEMPERATURE);

        ituSpriteGoto(mainBackgroundSprite, theConfig.mode);
        MainSetMode(theConfig.mode);

        ituProgressBarSetValue(mainPowerProgressBar, 0);
        ituTrackBarSetValue(mainPowerTrackBar, 0);

        ituProgressBarSetValue(mainWindProgressBar, theConfig.wind);
        ituTrackBarSetValue(mainWindTrackBar, theConfig.wind);

        mainRightContainerX = ituWidgetGetX(mainRightContainer);
    }


    mainConfigSaveDelay = 0;

    return true;
}

bool MainOnLeave(ITUWidget* widget, char* param)
{
    if (mainConfigSaveDelay > 0)
    {
        mainConfigSaveDelay = 0;

        ConfigSave();
    }
    return true;
}

void MainReset(void)
{
    mainLayer = NULL;
    SceneSetReady(false);
}
