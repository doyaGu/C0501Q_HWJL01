#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "scene.h"
#include "ctrlboard.h"
#include "castor3player.h"
#include "ite/itv.h"

#define COUNTDOWN_TICK 5000

static ITUSprite* videoPlayerStorageSprite;
static ITUBackground* videoViewCtrlBackground;
static ITUText* videoViewTimeText;
static ITUProgressBar* videoViewProgressBar;
static ITUSprite* videoViewRepeatSprite;
static ITUCheckBox* videoViewRandomCheckBox;
static ITUCheckBox* videoViewPlayCheckBox;
static ITUSprite* videoViewVolSprite;
static ITUTrackBar* videoViewVolTrackBar;
static ITUBackground* videoViewBackground;
static ITULayer* videoPlayerLayer;
static ITUScrollMediaFileListBox* videoPlayerScrollMediaFileListBox;
static ITUCheckBox* videoPlayerPlayCheckBox;
static ITUCheckBox* videoPlayerRandomCheckBox;
static ITUAnimation* videoViewPlayAnimation;
static ITUCheckBox* videoViewPlay1CheckBox;
static ITUCheckBox* videoViewPlay2CheckBox;
static ITUSprite* videoPlayerRepeatSprite;

static uint32_t videoViewLastTick;
static int x, y, width, height = 0;
static MTAL_SPEC mtal_spec = {0};

extern bool videoPlayerIsFileEOF;
extern int LastMediaPlayerVoice;
extern bool videoPlayerIsPlaying;
extern int videoPlayerPercentage;

#ifdef CFG_VIDEO_ENABLE
extern int CalVideoPlayerProgressBarPercentage(void);
#endif

bool VideoViewSDRemovedOnCustom(ITUWidget* widget, char* param)
{
    if (videoPlayerStorageSprite->frame == STORAGE_SD)
    {
        if (videoPlayerIsPlaying)
        {
#ifdef CFG_VIDEO_ENABLE        
            mtal_pb_stop();
#endif        
            videoPlayerIsPlaying = false;
        }

        ituCheckBoxSetChecked(videoPlayerPlayCheckBox, false);
        ituCheckBoxSetChecked(videoViewPlayCheckBox, false);
        ituCheckBoxSetChecked(videoViewPlay1CheckBox, false);
        ituCheckBoxSetChecked(videoViewPlay2CheckBox, false);
        ituTextSetString(videoViewTimeText, NULL);
        videoPlayerPercentage = 0;
        ituWidgetSetVisible(videoViewProgressBar, false);
        
        ituLayerGoto(videoPlayerLayer);
    }
    return true;
}

bool VideoViewUsbRemovedOnCustom(ITUWidget* widget, char* param)
{
    if (videoPlayerStorageSprite->frame == STORAGE_USB)
    {
        if (videoPlayerIsPlaying)
        {
#ifdef CFG_VIDEO_ENABLE        
            mtal_pb_stop();
#endif        
            videoPlayerIsPlaying = false;
        }

        ituCheckBoxSetChecked(videoPlayerPlayCheckBox, false);
        ituCheckBoxSetChecked(videoViewPlayCheckBox, false);
        ituCheckBoxSetChecked(videoViewPlay1CheckBox, false);
        ituCheckBoxSetChecked(videoViewPlay2CheckBox, false);
        ituTextSetString(videoViewTimeText, NULL);
        videoPlayerPercentage = 0;
        ituWidgetSetVisible(videoViewProgressBar, false);
        
        ituLayerGoto(videoPlayerLayer);
    }
    return true;
}

bool VideoViewRepeatButtonOnPress(ITUWidget* widget, char* param)
{
    ITUMediaRepeatMode mode = videoViewRepeatSprite->frame;
    if (mode == ITU_MEDIA_REPEAT_ALL)
        mode = ITU_MEDIA_REPEAT_NONE;
    else
        mode++;

    videoPlayerScrollMediaFileListBox->mflistbox.repeatMode = mode;
    ituSpriteGoto(videoViewRepeatSprite, mode);
    ituSpriteGoto(videoPlayerRepeatSprite, mode);

    return true;
}

bool VideoViewRandomCheckBoxOnPress(ITUWidget* widget, char* param)
{
    videoViewLastTick = itpGetTickCount();

    videoPlayerScrollMediaFileListBox->mflistbox.randomPlay = ituCheckBoxIsChecked(videoViewRandomCheckBox);

    if(ituCheckBoxIsChecked(videoViewRandomCheckBox))
    {
        ituCheckBoxSetChecked(videoPlayerRandomCheckBox, true);
    }
    else
    {
        ituCheckBoxSetChecked(videoPlayerRandomCheckBox, false);
    }
    return true;
}

bool VideoViewLastButtonOnPress(ITUWidget* widget, char* param)
{
	ITUScrollText* item = ituMediaFileListPrev((ITUMediaFileListBox*)videoPlayerScrollMediaFileListBox);
    videoViewLastTick = itpGetTickCount();
    
    if (item)
    {
        char* filepath = (char*)ituWidgetGetCustomData(item);

        //AudioPauseKeySound();
        
        strcpy(mtal_spec.srcname, filepath);
        mtal_spec.vol_level = LastMediaPlayerVoice;
#ifdef CFG_VIDEO_ENABLE
        mtal_pb_stop();
        mtal_pb_select_file(&mtal_spec);
        mtal_pb_play();
#endif
        videoPlayerIsPlaying = true;
        videoPlayerPercentage = 0;
        ituWidgetSetVisible(videoViewProgressBar, true);
    }
    
    return true;
}

bool VideoViewPlayCheckBoxOnPress(ITUWidget* widget, char* param)
{
    videoViewLastTick = itpGetTickCount();

    if (ituCheckBoxIsChecked((ITUCheckBox*)widget))
    {
        ITUScrollText* item = ituMediaFileListPlay((ITUMediaFileListBox*)videoPlayerScrollMediaFileListBox);
        if (item)
        {
            char* filepath = (char*)ituWidgetGetCustomData(item);
            strcpy(mtal_spec.srcname, filepath);
            mtal_spec.vol_level = LastMediaPlayerVoice;
#ifdef CFG_VIDEO_ENABLE
            mtal_pb_stop();
            mtal_pb_select_file(&mtal_spec);
            mtal_pb_play();
#endif
            ituCheckBoxSetChecked(videoPlayerPlayCheckBox, true);
            ituCheckBoxSetChecked(videoViewPlayCheckBox, true);
            ituCheckBoxSetChecked(videoViewPlay1CheckBox, true);
            ituCheckBoxSetChecked(videoViewPlay2CheckBox, true);

            videoPlayerIsPlaying = true;
            videoPlayerPercentage = 0;
            ituWidgetSetVisible(videoViewProgressBar, true);
        }
    }
    else
    {
#ifdef CFG_VIDEO_ENABLE        
        mtal_pb_stop();
#endif
        ituCheckBoxSetChecked(videoPlayerPlayCheckBox, false);
        ituCheckBoxSetChecked(videoViewPlayCheckBox, false);
        ituCheckBoxSetChecked(videoViewPlay1CheckBox, false);
        ituCheckBoxSetChecked(videoViewPlay2CheckBox, false);
        videoPlayerIsPlaying = false;
        videoPlayerPercentage = 0;
        ituWidgetSetVisible(videoViewProgressBar, false);
    }
    ituTextSetString(videoViewTimeText, NULL);
    return true;
}

bool VideoViewNextButtonOnPress(ITUWidget* widget, char* param)
{
    ITUScrollText* item = ituMediaFileListNext((ITUMediaFileListBox*)videoPlayerScrollMediaFileListBox);
    videoViewLastTick = itpGetTickCount();

    if (item)
    {
        char* filepath = (char*)ituWidgetGetCustomData(item);

        //AudioPauseKeySound();
        
        strcpy(mtal_spec.srcname, filepath);
        mtal_spec.vol_level = LastMediaPlayerVoice;
#ifdef CFG_VIDEO_ENABLE
        mtal_pb_stop();
        mtal_pb_select_file(&mtal_spec);
        mtal_pb_play();
#endif
        videoPlayerIsPlaying = true;
        videoPlayerPercentage = 0;
        ituWidgetSetVisible(videoViewProgressBar, true);
    }
    
    return true;
}

bool VideoViewVolTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int vol;

    if (!videoViewVolSprite)
        return false;

    videoViewLastTick = itpGetTickCount();

    vol = atoi(param);
    LastMediaPlayerVoice = vol;

    if (vol > 0)
    {
        ituSpriteGoto(videoViewVolSprite, 1);
    }
    else
    {
        ituSpriteGoto(videoViewVolSprite, 0);
    }

    AudioSetVolume(vol);
    return true;
}

bool VideoViewOnTimer(ITUWidget* widget, char* param)
{
    if (ituWidgetIsVisible(videoViewCtrlBackground) && itpGetTickDuration(videoViewLastTick) >= COUNTDOWN_TICK)
    {
        ituWidgetHide(videoViewCtrlBackground, ITU_EFFECT_SCROLL_DOWN, 10);
        videoViewPlayAnimation->animationFlags &= ~ITU_ANIM_CYCLE;
        ituAnimationPlay(videoViewPlayAnimation, 0);
        videoViewLastTick = itpGetTickCount();
    }

    if(videoPlayerIsFileEOF)
    {
        ITUScrollText* item = ituMediaFileListNext((ITUMediaFileListBox*)videoPlayerScrollMediaFileListBox);
        videoPlayerIsFileEOF = false;
#ifdef CFG_VIDEO_ENABLE        
        mtal_pb_stop();
#endif
        if(item)
        {
            char* filepath = (char*)ituWidgetGetCustomData(item);
            strcpy(mtal_spec.srcname, filepath);
            mtal_spec.vol_level = LastMediaPlayerVoice;
#ifdef CFG_VIDEO_ENABLE            
            mtal_pb_select_file(&mtal_spec);
            mtal_pb_play();
#endif            
        }
        else
        {
            ituWidgetSetVisible(videoViewProgressBar, false);
            ituCheckBoxSetChecked(videoViewPlayCheckBox, false);
            ituCheckBoxSetChecked(videoPlayerPlayCheckBox, false);
            ituCheckBoxSetChecked(videoViewPlay1CheckBox, false);
            ituCheckBoxSetChecked(videoViewPlay2CheckBox, false);
            videoPlayerIsPlaying = false;
        }    
    }

    if (videoPlayerIsPlaying)
    {
        ScreenSaverRefresh();
#ifdef CFG_VIDEO_ENABLE        
        videoPlayerPercentage = CalVideoPlayerProgressBarPercentage();
#endif
        ituProgressBarSetValue(videoViewProgressBar, videoPlayerPercentage);
        if (videoPlayerPercentage > 100)
        {
            videoPlayerIsPlaying = false;
            videoPlayerPercentage = 0;
            ituWidgetSetVisible(videoViewProgressBar, false);
        }
    }

    if (videoPlayerIsPlaying)
    {
        int h, m, s = 0;
        char buf[32];
#ifdef CFG_VIDEO_ENABLE        
        mtal_pb_get_current_time(&s);
#endif        
        m = s / 60;
        s %= 60;
        h = m / 60;
        m %= 60;

        if (h > 0)
            sprintf(buf, "%02d:%02d:%02d", h, m, s);
        else
            sprintf(buf, "%02d:%02d", m, s);

        ituTextSetString(videoViewTimeText, buf);
    }
    return true;
}

bool VideoViewViewButtonOnPress(ITUWidget* widget, char* param)
{
    if (videoViewPlayAnimation->keyframe == 1)
    {
        videoViewPlayAnimation->animationFlags |= ITU_ANIM_CYCLE;
        ituAnimationPlay(videoViewPlayAnimation, 1);
    }
    else if (ituWidgetIsVisible(videoViewCtrlBackground))
    {
        videoViewPlayAnimation->animationFlags &= ~ITU_ANIM_CYCLE;
        ituAnimationPlay(videoViewPlayAnimation, 0);
        ituWidgetHide(videoViewCtrlBackground, ITU_EFFECT_SCROLL_DOWN, 10);
    }
    else
    {
        videoViewLastTick = itpGetTickCount();
        ituWidgetShow(videoViewCtrlBackground, ITU_EFFECT_SCROLL_UP, 10);
    }
    return true;
}

static bool VideoViewBackgroundUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    if (ev == ITU_EVENT_TOUCHPINCH)
    {
        int x, y;

        ituWidgetGetGlobalPosition(widget, &x, &y);
        x += arg2;
        y += arg3;

        // TODO: IMPLEMENT
        // arg1: distance
    }
    return ituIconUpdate(widget, ev, arg1, arg2, arg3);
}

#if (CFG_CHIP_FAMILY == 9850) && (CFG_VIDEO_ENABLE)
static void VideoViewBackgroundDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty;
    ITURectangle* rect = (ITURectangle*) &widget->rect;

    ituDrawVideoSurface(dest, destx, desty, rect->width, rect->height);
    ituWidgetDrawImpl(widget, dest, x, y, alpha);    
}
#endif

bool VideoViewOnEnter(ITUWidget* widget, char* param)
{
    int vol;

    if (!videoPlayerStorageSprite)
    {
        videoPlayerStorageSprite = ituSceneFindWidget(&theScene, "videoPlayerStorageSprite");
        assert(videoPlayerStorageSprite);

        videoViewCtrlBackground = ituSceneFindWidget(&theScene, "videoViewCtrlBackground");
        assert(videoViewCtrlBackground);

        videoViewTimeText = ituSceneFindWidget(&theScene, "videoViewTimeText");
        assert(videoViewTimeText);

        videoViewProgressBar = ituSceneFindWidget(&theScene, "videoViewProgressBar");
        assert(videoViewProgressBar);

        videoViewRepeatSprite = ituSceneFindWidget(&theScene, "videoViewRepeatSprite");
        assert(videoViewRepeatSprite);

        videoViewRandomCheckBox = ituSceneFindWidget(&theScene, "videoViewRandomCheckBox");
        assert(videoViewRandomCheckBox);

        videoViewPlayCheckBox = ituSceneFindWidget(&theScene, "videoViewPlayCheckBox");
        assert(videoViewPlayCheckBox);

        videoViewVolSprite = ituSceneFindWidget(&theScene, "videoViewVolSprite");
        assert(videoViewVolSprite);

        videoViewVolTrackBar = ituSceneFindWidget(&theScene, "videoViewVolTrackBar");
        assert(videoViewVolTrackBar);

        videoViewBackground = ituSceneFindWidget(&theScene, "videoViewBackground");
        assert(videoViewBackground);
#if (CFG_CHIP_FAMILY == 9850) && (CFG_VIDEO_ENABLE)        
        ituWidgetSetDraw(videoViewBackground, VideoViewBackgroundDraw);
#endif
        ituWidgetSetUpdate(videoViewBackground, VideoViewBackgroundUpdate);

        videoPlayerLayer = ituSceneFindWidget(&theScene, "videoPlayerLayer");
        assert(videoPlayerLayer);

        videoPlayerScrollMediaFileListBox = ituSceneFindWidget(&theScene, "videoPlayerScrollMediaFileListBox");
        assert(videoPlayerScrollMediaFileListBox);

        videoPlayerPlayCheckBox = ituSceneFindWidget(&theScene, "videoPlayerPlayCheckBox");
        assert(videoPlayerPlayCheckBox);

        videoPlayerRandomCheckBox = ituSceneFindWidget(&theScene, "videoPlayerRandomCheckBox");
        assert(videoPlayerRandomCheckBox);

        videoViewPlayAnimation = ituSceneFindWidget(&theScene, "videoViewPlayAnimation");
        assert(videoViewPlayAnimation);

        videoViewPlay1CheckBox = ituSceneFindWidget(&theScene, "videoViewPlay1CheckBox");
        assert(videoViewPlay1CheckBox);

        videoViewPlay2CheckBox = ituSceneFindWidget(&theScene, "videoViewPlay2CheckBox");
        assert(videoViewPlay2CheckBox);

        videoPlayerRepeatSprite = ituSceneFindWidget(&theScene, "videoPlayerRepeatSprite");
        assert(videoPlayerRepeatSprite);
    }

    ituWidgetGetGlobalPosition(videoViewBackground, &x, &y);
    width = ituWidgetGetWidth(videoViewBackground);
    height = ituWidgetGetHeight(videoViewBackground);
#ifdef CFG_VIDEO_ENABLE        
    itv_set_video_window(x, y, width, height);
#endif        
    vol = AudioGetVolume();
    ituSpriteGoto(videoViewVolSprite, vol > 0 ? 1 : 0);
    ituTrackBarSetValue(videoViewVolTrackBar, vol);
    ituTextSetString(videoViewTimeText, NULL);
    ituWidgetSetVisible(videoViewCtrlBackground, true);
    ituWidgetSetVisible(videoViewProgressBar, true);
    videoViewLastTick = itpGetTickCount();

    return true;
}

bool VideoViewOnLeave(ITUWidget* widget, char* param)
{
    ituAnimationStop(videoViewPlayAnimation);
    return true;
}

void VideoViewReset(void)
{
    videoPlayerStorageSprite = NULL;
}
