#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITULayer* airConditionerLayer;
static ITURadioBox* airConditionerWindSlowRadioBox;
static ITUAnimation* airConditionerWindSlowAnimation;
static ITURadioBox* airConditionerWindNormalRadioBox;
static ITUAnimation* airConditionerWindNormalAnimation;
static ITURadioBox* airConditionerWindFastRadioBox;
static ITUAnimation* airConditionerWindFastAnimation;
static ITURadioBox* airConditionerWindAutoRadioBox;
static ITUAnimation* airConditionerWindAutoAnimation;
static ITURadioBox* airConditionerPowerRadioBox;
static ITURadioBox* airConditionerAutoRadioBox;
static ITURadioBox* airConditionerCoolRadioBox;
static ITURadioBox* airConditionerDryRadioBox;
static ITURadioBox* airConditionerFanRadioBox;
static ITURadioBox* airConditionerHeatRadioBox;
static ITUTrackBar* airConditionerTemperatureTrackBar;
static ITUProgressBar* airConditionerTemperatureProgressBar;

static int temperatureValue;
static bool powerOn;

static void AirConditionerWindAnimationOnStop(ITUAnimation* animation)
{
    if (animation == airConditionerWindSlowAnimation)
        ituWidgetSetVisible(airConditionerWindSlowRadioBox, true);
    else if (animation == airConditionerWindNormalAnimation)
        ituWidgetSetVisible(airConditionerWindNormalRadioBox, true);
    else if (animation == airConditionerWindFastAnimation)
        ituWidgetSetVisible(airConditionerWindFastRadioBox, true);
    else if (animation == airConditionerWindAutoAnimation)
        ituWidgetSetVisible(airConditionerWindAutoRadioBox, true);
}

static void AirConditionerInit(void)
{
    if (!airConditionerLayer)
    {
        airConditionerLayer = ituSceneFindWidget(&theScene, "airConditionerLayer");
        assert(airConditionerLayer);

        airConditionerWindSlowRadioBox = ituSceneFindWidget(&theScene, "airConditionerWindSlowRadioBox");
        assert(airConditionerWindSlowRadioBox);

        airConditionerWindSlowAnimation = ituSceneFindWidget(&theScene, "airConditionerWindSlowAnimation");
        assert(airConditionerWindSlowAnimation);
        ituAnimationSetOnStop(airConditionerWindSlowAnimation, AirConditionerWindAnimationOnStop);

        airConditionerWindNormalRadioBox = ituSceneFindWidget(&theScene, "airConditionerWindNormalRadioBox");
        assert(airConditionerWindNormalRadioBox);

        airConditionerWindNormalAnimation = ituSceneFindWidget(&theScene, "airConditionerWindNormalAnimation");
        assert(airConditionerWindNormalAnimation);
        ituAnimationSetOnStop(airConditionerWindNormalAnimation, AirConditionerWindAnimationOnStop);

        airConditionerWindFastRadioBox = ituSceneFindWidget(&theScene, "airConditionerWindFastRadioBox");
        assert(airConditionerWindFastRadioBox);

        airConditionerWindFastAnimation = ituSceneFindWidget(&theScene, "airConditionerWindFastAnimation");
        assert(airConditionerWindFastAnimation);
        ituAnimationSetOnStop(airConditionerWindFastAnimation, AirConditionerWindAnimationOnStop);

        airConditionerWindAutoRadioBox = ituSceneFindWidget(&theScene, "airConditionerWindAutoRadioBox");
        assert(airConditionerWindAutoRadioBox);

        airConditionerWindAutoAnimation = ituSceneFindWidget(&theScene, "airConditionerWindAutoAnimation");
        assert(airConditionerWindAutoAnimation);
        ituAnimationSetOnStop(airConditionerWindAutoAnimation, AirConditionerWindAnimationOnStop);

        airConditionerPowerRadioBox = ituSceneFindWidget(&theScene, "airConditionerPowerRadioBox");
        assert(airConditionerPowerRadioBox);

        airConditionerAutoRadioBox = ituSceneFindWidget(&theScene, "airConditionerAutoRadioBox");
        assert(airConditionerAutoRadioBox);

        airConditionerCoolRadioBox = ituSceneFindWidget(&theScene, "airConditionerCoolRadioBox");
        assert(airConditionerCoolRadioBox);

        airConditionerDryRadioBox = ituSceneFindWidget(&theScene, "airConditionerDryRadioBox");
        assert(airConditionerDryRadioBox);

        airConditionerFanRadioBox = ituSceneFindWidget(&theScene, "airConditionerFanRadioBox");
        assert(airConditionerFanRadioBox);

        airConditionerHeatRadioBox = ituSceneFindWidget(&theScene, "airConditionerHeatRadioBox");
        assert(airConditionerHeatRadioBox);

        airConditionerTemperatureTrackBar = ituSceneFindWidget(&theScene, "airConditionerTemperatureTrackBar");
        assert(airConditionerTemperatureTrackBar);

        airConditionerTemperatureProgressBar = ituSceneFindWidget(&theScene, "airConditionerTemperatureProgressBar");
        assert(airConditionerTemperatureProgressBar);
    }
}

void AirConditionerProcessEvent(ExternalEvent* ev)
{
    AirConditionerInit();
    ituLayerGoto(airConditionerLayer);

    switch (ev->type)
    {
    case EXTERNAL_TAP_POWER:
        ituRadioBoxSetChecked(airConditionerPowerRadioBox, ev->arg1 ? true : false);
        break;

    case EXTERNAL_TAP_AUTO:
        ituRadioBoxSetChecked(airConditionerAutoRadioBox, true);
        break;

    case EXTERNAL_TAP_COOL:
        ituRadioBoxSetChecked(airConditionerCoolRadioBox, true);
        break;

    case EXTERNAL_TAP_DRY:
        ituRadioBoxSetChecked(airConditionerDryRadioBox, true);
        break;

    case EXTERNAL_TAP_FAN:
        ituRadioBoxSetChecked(airConditionerFanRadioBox, true);
        break;

    case EXTERNAL_TAP_HEAT:
        ituRadioBoxSetChecked(airConditionerHeatRadioBox, true);
        break;

    case EXTERNAL_DRAG_TEMP_UP:
        ituTrackBarSetValue(airConditionerTemperatureTrackBar, airConditionerTemperatureTrackBar->value + 1);
        ituProgressBarSetValue(airConditionerTemperatureProgressBar, airConditionerTemperatureProgressBar->value + 1);
        temperatureValue = airConditionerTemperatureTrackBar->value;
        break;

    case EXTERNAL_DRAG_TEMP_DOWN:
        ituTrackBarSetValue(airConditionerTemperatureTrackBar, airConditionerTemperatureTrackBar->value - 1);
        ituProgressBarSetValue(airConditionerTemperatureProgressBar, airConditionerTemperatureProgressBar->value - 1);
        temperatureValue = airConditionerTemperatureTrackBar->value;
        break;

    case EXTERNAL_TAP_WIND_HIGH:
        ituRadioBoxSetChecked(airConditionerWindFastRadioBox, true);
        break;

    case EXTERNAL_TAP_WIND_MID:
        ituRadioBoxSetChecked(airConditionerWindNormalRadioBox, true);
        break;

    case EXTERNAL_TAP_WIND_LOW:
        ituRadioBoxSetChecked(airConditionerWindSlowRadioBox, true);
        break;

    case EXTERNAL_TAP_WIND_AUTO:
        ituRadioBoxSetChecked(airConditionerWindAutoRadioBox, true);
        break;
    }
}

bool AirConditionerOnTimer(ITUWidget* widget, char* param)
{
    return false;
}

bool AirConditionerPowerRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    powerOn = powerOn ? false : true;
    ituRadioBoxSetChecked(airConditionerPowerRadioBox, powerOn);

    ev.type = EXTERNAL_TAP_POWER;
    ev.arg1 = powerOn ? 1 : 0;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerAutoRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_AUTO;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerCoolRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_COOL;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerDryRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_DRY;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerFanRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_FAN;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerHeatRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_HEAT;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerWindSlowRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_WIND_LOW;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerWindNormalRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_WIND_MID;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerWindFastRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_WIND_HIGH;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerWindAutoRadioBoxOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    ev.type = EXTERNAL_TAP_WIND_AUTO;
    ev.arg1 = 1;
    ExternalSend(&ev);
    return true;
}

bool AirConditionerTemperatureTrackBarOnChanged(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    int value = atoi(param);

    if (value > temperatureValue)
    {
        ev.type = EXTERNAL_DRAG_TEMP_UP;
        ev.arg1 = value;
        ExternalSend(&ev);
    }
    else if (value < temperatureValue)
    {
        ev.type = EXTERNAL_DRAG_TEMP_DOWN;
        ev.arg1 = value;
        ExternalSend(&ev);
    }
    temperatureValue = value;
    return true;
}

bool AirConditionerOnEnter(ITUWidget* widget, char* param)
{
    AirConditionerInit();

    ituWidgetSetVisible(airConditionerWindSlowRadioBox, true);
    ituWidgetSetVisible(airConditionerWindNormalRadioBox, true);
    ituWidgetSetVisible(airConditionerWindFastRadioBox, true);
    ituWidgetSetVisible(airConditionerWindAutoRadioBox, true);

    temperatureValue = airConditionerTemperatureTrackBar->value;
    powerOn = ituRadioBoxIsChecked(airConditionerPowerRadioBox);

    return true;
}

void AirConditionerReset(void)
{
    airConditionerLayer = NULL;
}
