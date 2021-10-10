#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITURadioBox* airConditionerWindSlowRadioBox;
static ITUAnimation* airConditionerWindSlowAnimation;
static ITURadioBox* airConditionerWindNormalRadioBox;
static ITUAnimation* airConditionerWindNormalAnimation;
static ITURadioBox* airConditionerWindFastRadioBox;
static ITUAnimation* airConditionerWindFastAnimation;
static ITURadioBox* airConditionerWindAutoRadioBox;
static ITUAnimation* airConditionerWindAutoAnimation;
static ITUCheckBox* airConditionerPowerCheckBox;

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

bool AirConditionerOnTimer(ITUWidget* widget, char* param)
{
    return false;
}

bool AirConditionerOnEnter(ITUWidget* widget, char* param)
{
    if (!airConditionerWindSlowRadioBox)
    {
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

        airConditionerPowerCheckBox = ituSceneFindWidget(&theScene, "airConditionerPowerCheckBox");
        assert(airConditionerPowerCheckBox);
    }
    ituWidgetSetVisible(airConditionerWindSlowRadioBox, true);
    ituWidgetSetVisible(airConditionerWindNormalRadioBox, true);
    ituWidgetSetVisible(airConditionerWindFastRadioBox, true);
    ituWidgetSetVisible(airConditionerWindAutoRadioBox, true);
    return true;
}

void AirConditionerReset(void)
{
    airConditionerWindSlowRadioBox = NULL;
}
