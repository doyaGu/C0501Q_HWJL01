#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "scene.h"
#include "airconditioner.h"

#define HIDE_CTRL_PANEL_DELAY (3 * 1000 / MS_PER_FRAME)

static ITUVideo* videoPlayerVideo;
static ITUContainer* videoPlayerCtrlContainer;
static ITUCheckBox* videoPlayerPlay1CheckBox;
static ITUCheckBox* videoPlayerPlay2CheckBox;
static ITUProgressBar* videoPlayerProgressBar;
static ITUTrackBar* videoPlayerTrackBar;
static ITULayer* mainLayer;

static int videoPlayerHideCtrlPanelDelay;
static int videoPlayerPercentage, old_videoPlayerPercentage = 0;
static bool bVideoStop = false;

static void VideoPlayerVideoOnStop(ITUVideo* video)
{
    //printf("video stopped\n");
    //ituLayerGoto(mainLayer);
    bVideoStop = true;
}

bool VideoPlayerBackgroundButtonOnPress(ITUWidget* widget, char* param)
{
    ituWidgetSetVisible(videoPlayerCtrlContainer, true);

    videoPlayerHideCtrlPanelDelay = HIDE_CTRL_PANEL_DELAY;
    return true;
}

bool VideoPlayerPlayCheckBoxOnPress(ITUWidget* widget, char* param)
{
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;
    bool checked = ituCheckBoxIsChecked(checkbox);

    if (checked)
        ituVideoPause(videoPlayerVideo);
    else
        ituVideoPlay(videoPlayerVideo, -1);        

    ituCheckBoxSetChecked(videoPlayerPlay1CheckBox, checked);
    ituCheckBoxSetChecked(videoPlayerPlay2CheckBox, checked);

    videoPlayerHideCtrlPanelDelay = HIDE_CTRL_PANEL_DELAY;
    return true;
}

bool VideoPlayerTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = videoPlayerTrackBar->value;

    //printf("video goto: %d\n", value);
    ituVideoGoto(videoPlayerVideo, value);

    videoPlayerHideCtrlPanelDelay = HIDE_CTRL_PANEL_DELAY;
    return true;
}

bool VideoPlayerOnTimer(ITUWidget* widget, char* param)
{
    if(bVideoStop)
    {
        ituLayerGoto(mainLayer);
        bVideoStop = false;
    }
    
    if (videoPlayerHideCtrlPanelDelay > 0)
    {
        if (--videoPlayerHideCtrlPanelDelay == 0)
        {
            ituWidgetSetVisible(videoPlayerCtrlContainer, false);
        }
    }

    videoPlayerPercentage = ituVideoGetPlayingPercentage(videoPlayerVideo);
    //printf("videoPlayerPercentage = %d\n", videoPlayerPercentage);
    if(videoPlayerPercentage >= 0 && videoPlayerPercentage != old_videoPlayerPercentage)
    {
            ituProgressBarSetValue(videoPlayerProgressBar, videoPlayerPercentage);
            ituTrackBarSetValue(videoPlayerTrackBar, videoPlayerPercentage);
            old_videoPlayerPercentage = videoPlayerPercentage;
    }       
    return true;
}

bool VideoPlayerOnEnter(ITUWidget* widget, char* param)
{
    if (!videoPlayerVideo)
    {
        videoPlayerVideo = ituSceneFindWidget(&theScene, "videoPlayerVideo");
        assert(videoPlayerVideo);
        ituVideoSetOnStop(videoPlayerVideo, VideoPlayerVideoOnStop);
        videoPlayerVideo->volume = theConfig.audiolevel;

        videoPlayerCtrlContainer = ituSceneFindWidget(&theScene, "videoPlayerCtrlContainer");
        assert(videoPlayerCtrlContainer);

        videoPlayerPlay1CheckBox = ituSceneFindWidget(&theScene, "videoPlayerPlay1CheckBox");
        assert(videoPlayerPlay1CheckBox);

        videoPlayerPlay2CheckBox = ituSceneFindWidget(&theScene, "videoPlayerPlay2CheckBox");
        assert(videoPlayerPlay2CheckBox);

        videoPlayerProgressBar = ituSceneFindWidget(&theScene, "videoPlayerProgressBar");
        assert(videoPlayerProgressBar);

        videoPlayerTrackBar = ituSceneFindWidget(&theScene, "videoPlayerTrackBar");
        assert(videoPlayerTrackBar);

        mainLayer = ituSceneFindWidget(&theScene, "mainLayer");
        assert(mainLayer);
    }
    ScreenSaverPause();
    ituWidgetSetVisible(videoPlayerCtrlContainer, true);
    videoPlayerHideCtrlPanelDelay = HIDE_CTRL_PANEL_DELAY;
    
    ituCheckBoxSetChecked(videoPlayerPlay1CheckBox, true);
    ituCheckBoxSetChecked(videoPlayerPlay2CheckBox, true);
    ituProgressBarSetValue(videoPlayerProgressBar, 0);
    ituTrackBarSetValue(videoPlayerTrackBar, 0);

    AudioPauseKeySound();
    ituVideoPlay(videoPlayerVideo, -1);

    return true;
}

bool VideoPlayerOnLeave(ITUWidget* widget, char* param)
{
    ituVideoStop(videoPlayerVideo);
    videoPlayerHideCtrlPanelDelay = 0;
    AudioResumeKeySound();
    ScreenSaverContinue();
    return true;
}

void VideoPlayerReset(void)
{
    videoPlayerVideo = NULL;
}
