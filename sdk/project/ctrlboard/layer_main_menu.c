#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"

#define RIPPLE_MAX_COUNT 2
#define RIPPLE_2ND_DELAY 10

static ITUBackground* mainMenuBackground;
static ITUBackground* mainMenuRippleBackground;
static ITUCoverFlow* mainMenuCoverFlow;
static ITUShadow* mainMenuWeatherShadow;
static ITUShadow* mainMenuClockShadow;
static ITUShadow* mainMenuAirConditionerShadow;
static ITUShadow* mainMenuKeyboardShadow;
static ITUShadow* mainMenuMediaShadow;
static ITUShadow* mainMenuCalendarShadow;
static ITUShadow* mainMenuButtonShadow;
static ITUShadow* mainMenuMeterShadow;
static ITUShadow* mainMenuSettingShadow;
static ITUShadow* mainMenuCheckShadow;
static ITUShadow* mainMenuCircleCtrlShadow;
static ITUShadow* mainMenuListShadow;
static ITUPageFlow* mainMenuPageFlow;
static ITUAnimation* mainMenu1Animation0;
static ITUAnimation* mainMenu2Animation0;
static ITUAnimation* mainMenu1Animation1;
static ITUAnimation* mainMenu2Animation1;

static ITUAnimation* mainMenu1Animations[RIPPLE_MAX_COUNT];
static ITUAnimation* mainMenu2Animations[RIPPLE_MAX_COUNT];

static MainMenuPlay2ndRipple0(int arg)
{
    ITUAnimation* mainMenu2Animation = mainMenu2Animations[0];

    ituWidgetSetVisible(mainMenu2Animation, true);
    ituAnimationPlay(mainMenu2Animation, 0);
}

static MainMenuPlay2ndRipple1(int arg)
{
    ITUAnimation* mainMenu2Animation = mainMenu2Animations[1];

    ituWidgetSetVisible(mainMenu2Animation, true);
    ituAnimationPlay(mainMenu2Animation, 0);
}

static bool MainMenuBackgroundUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_MOUSEDOWN)
    {
        if (theConfig.mainmenu_type == MAINMENU_COVERFLOW_RIPPLE && ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                int xx, yy, i;

                for (i = 0; i < RIPPLE_MAX_COUNT; i++)
                {
                    ITUAnimation *mainMenu1Animation, *mainMenu2Animation;
                        
                    mainMenu1Animation = mainMenu1Animations[i];
                    if (mainMenu1Animation->playing)
                        continue;

                    mainMenu2Animation = mainMenu2Animations[i];

                    xx = x - ituWidgetGetWidth(mainMenu1Animation) / 2;
                    yy = y - ituWidgetGetHeight(mainMenu1Animation) / 2;
                    ituWidgetSetVisible(mainMenu1Animation, true);
                    ituWidgetSetPosition(mainMenu1Animation, xx, yy);
                    ituAnimationPlay(mainMenu1Animation, 0);

                    if (i == 0)
                        ituSceneExecuteCommand(&theScene, RIPPLE_2ND_DELAY, MainMenuPlay2ndRipple0, 0);
                    else if (i == 1)
                        ituSceneExecuteCommand(&theScene, RIPPLE_2ND_DELAY, MainMenuPlay2ndRipple1, 0);

                    xx = x - ituWidgetGetWidth(mainMenu2Animation) / 2;
                    yy = y - ituWidgetGetHeight(mainMenu2Animation) / 2;
                    ituWidgetSetPosition(mainMenu2Animation, xx, yy);

                    result = true;
                    break;
                }
            }
        }
    }
    return result;
}

static void MainMenuAnimationOnStop(ITUAnimation* animation)
{
    ituWidgetSetVisible(animation, false);
}

bool MainMenuOnEnter(ITUWidget* widget, char* param)
{
    int i;

    if (!mainMenuBackground)
    {
        mainMenuBackground = ituSceneFindWidget(&theScene, "mainMenuBackground");
        assert(mainMenuBackground);
        ituWidgetSetUpdate(mainMenuBackground, MainMenuBackgroundUpdate);

        mainMenuRippleBackground = ituSceneFindWidget(&theScene, "mainMenuRippleBackground");
        assert(mainMenuRippleBackground);

        mainMenuCoverFlow = ituSceneFindWidget(&theScene, "mainMenuCoverFlow");
        assert(mainMenuCoverFlow);

        mainMenuWeatherShadow = ituSceneFindWidget(&theScene, "mainMenuWeatherShadow");
        assert(mainMenuWeatherShadow);

        mainMenuClockShadow = ituSceneFindWidget(&theScene, "mainMenuClockShadow");
        assert(mainMenuClockShadow);

        mainMenuAirConditionerShadow = ituSceneFindWidget(&theScene, "mainMenuAirConditionerShadow");
        assert(mainMenuAirConditionerShadow);

        mainMenuKeyboardShadow = ituSceneFindWidget(&theScene, "mainMenuKeyboardShadow");
        assert(mainMenuKeyboardShadow);

        mainMenuMediaShadow = ituSceneFindWidget(&theScene, "mainMenuMediaShadow");
        assert(mainMenuMediaShadow);

        mainMenuCalendarShadow = ituSceneFindWidget(&theScene, "mainMenuCalendarShadow");
        assert(mainMenuCalendarShadow);

        mainMenuButtonShadow = ituSceneFindWidget(&theScene, "mainMenuButtonShadow");
        assert(mainMenuButtonShadow);

        mainMenuMeterShadow = ituSceneFindWidget(&theScene, "mainMenuMeterShadow");
        assert(mainMenuMeterShadow);

        mainMenuSettingShadow = ituSceneFindWidget(&theScene, "mainMenuSettingShadow");
        assert(mainMenuSettingShadow);

        mainMenuCheckShadow = ituSceneFindWidget(&theScene, "mainMenuCheckShadow");
        assert(mainMenuCheckShadow);

        mainMenuCircleCtrlShadow = ituSceneFindWidget(&theScene, "mainMenuCircleCtrlShadow");
        assert(mainMenuCircleCtrlShadow);

        mainMenuListShadow = ituSceneFindWidget(&theScene, "mainMenuListShadow");
        assert(mainMenuListShadow);

        mainMenuPageFlow = ituSceneFindWidget(&theScene, "mainMenuPageFlow");
        assert(mainMenuPageFlow);

        i = 0;
        mainMenu1Animation0 = ituSceneFindWidget(&theScene, "mainMenu1Animation0");
        ituAnimationSetOnStop(mainMenu1Animation0, MainMenuAnimationOnStop);
        mainMenu1Animations[i] = mainMenu1Animation0;

        mainMenu2Animation0 = ituSceneFindWidget(&theScene, "mainMenu2Animation0");
        assert(mainMenu2Animation0);
        ituAnimationSetOnStop(mainMenu2Animation0, MainMenuAnimationOnStop);
        mainMenu2Animations[i] = mainMenu2Animation0;
        i++;

        mainMenu1Animation1 = ituSceneFindWidget(&theScene, "mainMenu1Animation1");
        assert(mainMenu1Animation1);
        ituAnimationSetOnStop(mainMenu1Animation1, MainMenuAnimationOnStop);
        mainMenu1Animations[i] = mainMenu1Animation1;

        mainMenu2Animation1 = ituSceneFindWidget(&theScene, "mainMenu2Animation1");
        assert(mainMenu2Animation1);
        ituAnimationSetOnStop(mainMenu2Animation1, MainMenuAnimationOnStop);
        mainMenu2Animations[i] = mainMenu2Animation1;
        i++;

    #if !defined(CFG_PLAY_VIDEO_ON_BOOTING) && !defined(CFG_PLAY_MJPEG_ON_BOOTING)        
        ituSceneExecuteCommand(&theScene, 3, ScenePredraw, 0);
    #endif
    }

    ituWidgetSetVisible(mainMenuWeatherShadow, false);
    ituWidgetSetVisible(mainMenuClockShadow, false);
    ituWidgetSetVisible(mainMenuAirConditionerShadow, false);
    ituWidgetSetVisible(mainMenuKeyboardShadow, false);
    ituWidgetSetVisible(mainMenuMediaShadow, false);
    ituWidgetSetVisible(mainMenuCalendarShadow, false);
    ituWidgetSetVisible(mainMenuButtonShadow, false);
    ituWidgetSetVisible(mainMenuMeterShadow, false);
    ituWidgetSetVisible(mainMenuSettingShadow, false);
    ituWidgetSetVisible(mainMenuCheckShadow, false);
    ituWidgetSetVisible(mainMenuCircleCtrlShadow, false);
    ituWidgetSetVisible(mainMenuListShadow, false);

    switch (theConfig.mainmenu_type)
    {
    case MAINMENU_COVERFLOW:
        ituWidgetSetVisible(mainMenuRippleBackground, false);
        ituWidgetSetVisible(mainMenuCoverFlow, true);
        ituWidgetSetVisible(mainMenuPageFlow, false);
        break;

    case MAINMENU_COVERFLOW_REFLECTION:
        ituWidgetSetVisible(mainMenuRippleBackground, false);
        ituWidgetSetVisible(mainMenuCoverFlow, true);
        ituWidgetSetVisible(mainMenuPageFlow, false);
        ituWidgetSetVisible(mainMenuWeatherShadow, true);
        ituWidgetSetVisible(mainMenuClockShadow, true);
        ituWidgetSetVisible(mainMenuAirConditionerShadow, true);
        ituWidgetSetVisible(mainMenuKeyboardShadow, true);
        ituWidgetSetVisible(mainMenuMediaShadow, true);
        ituWidgetSetVisible(mainMenuCalendarShadow, true);
        ituWidgetSetVisible(mainMenuButtonShadow, true);
        ituWidgetSetVisible(mainMenuMeterShadow, true);
        ituWidgetSetVisible(mainMenuSettingShadow, true);
        ituWidgetSetVisible(mainMenuCheckShadow, true);
        ituWidgetSetVisible(mainMenuCircleCtrlShadow, true);
        ituWidgetSetVisible(mainMenuListShadow, true);
        break;

    case MAINMENU_PAGEFLOW_FLIP:
        ituWidgetSetVisible(mainMenuRippleBackground, false);
        ituWidgetSetVisible(mainMenuCoverFlow, false);
        ituWidgetSetVisible(mainMenuPageFlow, true);
        mainMenuPageFlow->type = ITU_PAGEFLOW_FLIP;
        break;

    case MAINMENU_PAGEFLOW_FLIP2:
        ituWidgetSetVisible(mainMenuRippleBackground, false);
        ituWidgetSetVisible(mainMenuCoverFlow, false);
        ituWidgetSetVisible(mainMenuPageFlow, true);
        mainMenuPageFlow->type = ITU_PAGEFLOW_FLIP2;
        break;

    case MAINMENU_PAGEFLOW_FOLD:
        ituWidgetSetVisible(mainMenuRippleBackground, false);
        ituWidgetSetVisible(mainMenuCoverFlow, false);
        ituWidgetSetVisible(mainMenuPageFlow, true);
        mainMenuPageFlow->type = ITU_PAGEFLOW_FOLD;
        break;

    case MAINMENU_COVERFLOW_RIPPLE:
        ituWidgetSetVisible(mainMenuRippleBackground, true);
        ituWidgetSetVisible(mainMenuCoverFlow, true);
        ituWidgetSetVisible(mainMenuPageFlow, false);
        break;
    }

    for (i = 0; i < RIPPLE_MAX_COUNT; i++)
    {
        ituWidgetSetVisible(mainMenu1Animations[i], false);
        ituWidgetSetVisible(mainMenu2Animations[i], false);
    }
    return true;
}

bool MainMenuOnLeave(ITUWidget* widget, char* param)
{
    int i;

    for (i = 0; i < RIPPLE_MAX_COUNT; i++)
    {
        ituAnimationStop(mainMenu1Animations[i]);
        ituAnimationStop(mainMenu2Animations[i]);
    }
    return true;
}

void MainMenuReset(void)
{
    mainMenuBackground = NULL;
}