#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"

static ITULayer* mainMenuLayer;
static ITUBackground* mainMenuBackground;
static ITUCoverFlow* mainMenuCoverFlow;
static ITULayer* airConditionerLayer;
static ITULayer* videoViewLayer;

static void MainMenuInit(void)
{
    if (!mainMenuLayer)
    {
        mainMenuLayer = ituSceneFindWidget(&theScene, "mainMenuLayer");
        assert(mainMenuLayer);

        mainMenuBackground = ituSceneFindWidget(&theScene, "mainMenuBackground");
        assert(mainMenuBackground);

        mainMenuCoverFlow = ituSceneFindWidget(&theScene, "mainMenuCoverFlow");
        assert(mainMenuCoverFlow);

        airConditionerLayer = ituSceneFindWidget(&theScene, "airConditionerLayer");
        assert(airConditionerLayer);

        videoViewLayer = ituSceneFindWidget(&theScene, "videoViewLayer");
        assert(videoViewLayer);

    #if !defined(CFG_PLAY_VIDEO_ON_BOOTING) && !defined(CFG_PLAY_MJPEG_ON_BOOTING)        
        ituSceneExecuteCommand(&theScene, 3, ScenePredraw, 0);
    #endif
    }
}

bool MainMenuCoverFlowOnChanged(ITUWidget* widget, char* param)
{
    ExternalEvent ev;
    int index = atoi(param);

    if (index == 0)
        ev.type = EXTERNAL_TAP_HOME_PAGE_UP;
    else
        ev.type = EXTERNAL_TAP_HOME_PAGE_DOWN;

    ev.arg1 = index;
    ExternalSend(&ev);

    return true;
}

bool MainMenuAirConditionerPopupButtonOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_TAP_AIRCONDITIONER;
    ev.arg1 = 1;
    ExternalSend(&ev);

    return true;
}

bool MainMenuMediaPopupButtonOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_TAP_MULTIMEDIA;
    ev.arg1 = 1;
    ExternalSend(&ev);

    return true;
}

void MainMenuProcessEvent(ExternalEvent* ev)
{
    MainMenuInit();

    switch (ev->type)
    {
    case EXTERNAL_TAP_HOME_PAGE_UP:
        ituLayerGoto(mainMenuLayer);
        ituCoverFlowGoto(mainMenuCoverFlow, 1);
        ituCoverFlowPrev(mainMenuCoverFlow);
        break;

    case EXTERNAL_TAP_HOME_PAGE_DOWN:
        ituLayerGoto(mainMenuLayer);
        ituCoverFlowGoto(mainMenuCoverFlow, 0);
        ituCoverFlowNext(mainMenuCoverFlow);
        break;

    case EXTERNAL_TAP_AIRCONDITIONER:
        ituLayerGoto(airConditionerLayer);
        break;

    case EXTERNAL_TAP_MULTIMEDIA:
        ituLayerGoto(videoViewLayer);
        break;
    }
}

bool MainMenuOnEnter(ITUWidget* widget, char* param)
{
    MainMenuInit();
    return true;
}

bool MainMenuOnLeave(ITUWidget* widget, char* param)
{
    return true;
}

void MainMenuReset(void)
{
    mainMenuLayer = NULL;
}