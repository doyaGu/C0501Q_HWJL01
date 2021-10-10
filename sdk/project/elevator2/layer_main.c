#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL/SDL.h"
#include "audio_mgr.h"
#include "scene.h"
#include "elevator.h"
#include "castor3player.h"

#pragma execution_character_set("utf-8")

static ITULayer* mainLayer;
static ITUContainer* mainArrowFloor1Container;
static ITUContainer* mainArrowFloor2Container;
static ITUContainer* mainDateContainer;
static ITUContainer* mainTimeContainer;
static ITUContainer* mainTemperatureContainer;
static ITUSprite* mainArrow1UpSprite;
static ITUSprite* mainArrow1DownSprite;
static ITUAnimation* mainArrow1UpAnimation;
static ITUAnimation* mainArrow1DownAnimation;
static ITUContainer* mainFloor1Container;
static ITUSprite* mainFloor11Sprite;
static ITUSprite* mainFloor12Sprite;
static ITUSprite* mainFloor13Sprite;
static ITUSprite* mainArrow2UpSprite;
static ITUSprite* mainArrow2DownSprite;
static ITUAnimation* mainArrow2UpAnimation;
static ITUAnimation* mainArrow2DownAnimation;
static ITUContainer* mainFloor2Container;
static ITUSprite* mainFloor21Sprite;
static ITUSprite* mainFloor22Sprite;
static ITUSprite* mainFloor23Sprite;
static ITUText* mainTemperatureText;
static ITUSprite* mainDateSprite;
static ITUSprite* mainLogo1Sprite;
static ITUAnimation* mainLogo11Animation;
static ITUIcon* mainLogo111Icon;
static ITUAnimation* mainLogo12Animation;
static ITUAnimation* mainLogo13Animation;
static ITUAnimation* mainLogo14Animation;
static ITUSprite* mainLogo2Sprite;
static ITUAnimation* mainLogo21Animation;
static ITUIcon* mainLogo211Icon;
static ITUAnimation* mainLogo22Animation;
static ITUAnimation* mainLogo23Animation;
static ITUAnimation* mainLogo24Animation;
static ITUSprite* mainWeb1Sprite;
static ITUAnimation* mainWeb11Animation;
static ITUText* mainWeb111Text;
static ITUAnimation* mainWeb12Animation;
static ITUText* mainWeb122Text;
static ITUAnimation* mainWeb13Animation;
static ITUAnimation* mainWeb14Animation;
static ITUSprite* mainWeb2Sprite;
static ITUAnimation* mainWeb21Animation;
static ITUText* mainWeb211Text;
static ITUAnimation* mainWeb22Animation;
static ITUText* mainWeb222Text;
static ITUAnimation* mainWeb23Animation;
static ITUAnimation* mainWeb24Animation;
static ITUSprite* mainWeb3Sprite;
static ITUAnimation* mainWeb31Animation;
static ITUText* mainWeb311Text;
static ITUAnimation* mainWeb32Animation;
static ITUText* mainWeb322Text;
static ITUAnimation* mainWeb33Animation;
static ITUAnimation* mainWeb34Animation;
static ITUSlideshow* mainPhoto1Slideshow;
static ITUSlideshow* mainPhoto2Slideshow;
static ITUVideo* mainVideo;
static ITUMediaFileListBox* mainVideoMediaFileListBox;
static ITUMediaFileListBox* mainMusicMediaFileListBox;
static ITUBackground* mainInfo1Background;
static ITUSprite* mainInfo1Sprite;
static ITUAnimation* mainInfo11Animation;
static ITUText* mainInfo111Text;
static ITUAnimation* mainInfo12Animation;
static ITUText* mainInfo122Text;
static ITUAnimation* mainInfo13Animation;
static ITUAnimation* mainInfo14Animation;
static ITUText* mainInfo15Text;
static ITUMediaFileListBox* mainInfo1TextMediaFileListBox;
static ITUMediaFileListBox* mainInfo1FontMediaFileListBox;
static ITUBackground* mainInfo2Background;
static ITUSprite* mainInfo2Sprite;
static ITUAnimation* mainInfo21Animation;
static ITUText* mainInfo211Text;
static ITUAnimation* mainInfo22Animation;
static ITUText* mainInfo222Text;
static ITUAnimation* mainInfo23Animation;
static ITUAnimation* mainInfo24Animation;
static ITUMediaFileListBox* mainInfo2TextMediaFileListBox;
static ITUMediaFileListBox* mainInfo2FontMediaFileListBox;
static ITUBackground* warnBackground;
static ITUSprite* warn1Sprite;
static ITUText* mainWiFiSsidText;
static ITUText* mainWiFiPasswordText;

typedef enum
{
    PLAY_NONE,
    PLAY_CHINESE,
    PLAY_TAIWANESE,
    PLAY_MUSIC,
    PLAY_WARN
} PlayState;

static PlayState playState;
static int targetFloor, nextTargetFloor;
static ITUAnimation* activeMainInfo1Animation;
static ITUAnimation* activeMainInfo2Animation;
static FILE* mainInfo1File;
static char mainInfo1FilePath[PATH_MAX];
static bool mainInfo1FontLoaded;
static FILE* mainInfo2File;
static char mainInfo2FilePath[PATH_MAX];
static bool mainInfo2FontLoaded;
static char infoBuf[1024];
static bool mainPhoto1SlideshowEnabled;
static bool mainVideoEnabled;
static char mainWarnSoundPath[PATH_MAX];

#define MAX_LAYOUT_INDEX 15
static int mainLayoutIndex;
static int mainLayerWidth;
static int mainLayerHeight;

static void MainChangeLayout(void)
{
    ExternalOrientation orient = ExternalGetOrientation();
    ExternalEvent ev;

    if (mainLayoutIndex >= MAX_LAYOUT_INDEX)
        mainLayoutIndex = 0;
    else
        mainLayoutIndex++;

    switch (mainLayoutIndex)
    {
    case 0:
        if (orient == EXTERNAL_VERTICAL)
        {
            ituWidgetSetPosition(mainDateContainer, ituWidgetGetWidth(mainArrowFloor1Container), 0);
            ituWidgetSetPosition(mainTimeContainer, ituWidgetGetWidth(mainArrowFloor1Container), ituWidgetGetHeight(mainDateContainer));
            ituWidgetSetX(mainTemperatureContainer, ituWidgetGetWidth(mainArrowFloor1Container));
            ituWidgetSetY(mainTemperatureContainer, ituWidgetGetY(mainTimeContainer) + ituWidgetGetHeight(mainTimeContainer));
            ituWidgetSetY(mainPhoto1Slideshow, ituWidgetGetHeight(mainArrowFloor1Container));
            ituWidgetSetHeight(mainPhoto1Slideshow, mainLayerHeight - ituWidgetGetHeight(mainArrowFloor1Container));

			ituWidgetSetY(mainVideo, ituWidgetGetHeight(mainArrowFloor1Container));
			ituWidgetSetHeight(mainVideo, mainLayerHeight - ituWidgetGetHeight(mainArrowFloor1Container));
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetY(mainLogo1Sprite, ituWidgetGetY(mainPhoto1Slideshow));
            ituWidgetSetY(mainWeb1Sprite, ituWidgetGetY(mainPhoto1Slideshow));
        }
        else
        {
            ituWidgetSetWidth(mainPhoto1Slideshow, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor1Container));

			ituWidgetSetWidth(mainVideo, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor1Container));
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetX(mainWeb1Sprite, mainLayerWidth - ituWidgetGetWidth(mainWeb1Sprite));
			ituWidgetSetWidth(mainInfo1Background, ituWidgetGetWidth(mainPhoto1Slideshow));
            ituWidgetSetX(mainTemperatureContainer, 0);
            ituWidgetSetY(mainTemperatureContainer, mainLayerHeight - ituWidgetGetHeight(mainTemperatureContainer));
        }
        ituWidgetSetVisible(mainArrowFloor2Container, false);
        break;

    case 1:
        if (orient == EXTERNAL_VERTICAL)
        {
            ituWidgetSetY(mainArrowFloor1Container, mainLayerHeight - ituWidgetGetHeight(mainArrowFloor1Container));
            ituWidgetSetY(mainDateContainer, ituWidgetGetY(mainArrowFloor1Container));
            ituWidgetSetY(mainTimeContainer, ituWidgetGetY(mainDateContainer) + ituWidgetGetHeight(mainDateContainer));
            ituWidgetSetY(mainTemperatureContainer, ituWidgetGetY(mainTimeContainer) + ituWidgetGetHeight(mainTimeContainer));
            ituWidgetSetY(mainPhoto1Slideshow, 0);

			ituWidgetSetY(mainVideo, 0);
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetY(mainLogo1Sprite, 0);
            ituWidgetSetY(mainWeb1Sprite, 0);
			ituWidgetSetY(mainInfo1Background, ituWidgetGetY(mainArrowFloor1Container) - ituWidgetGetHeight(mainInfo1Background));
        }
        else
        {
            ituWidgetSetX(mainArrowFloor1Container, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor1Container));
            ituWidgetSetX(mainDateContainer, mainLayerWidth - ituWidgetGetWidth(mainDateContainer));
            ituWidgetSetX(mainTimeContainer, mainLayerWidth - ituWidgetGetWidth(mainTimeContainer));
            ituWidgetSetX(mainTemperatureContainer, mainLayerWidth - ituWidgetGetWidth(mainTemperatureContainer));
            ituWidgetSetX(mainPhoto1Slideshow, 0);

			ituWidgetSetX(mainVideo, 0);
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetX(mainLogo1Sprite, ituWidgetGetX(mainPhoto1Slideshow));
            ituWidgetSetX(mainWeb1Sprite, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor1Container) - ituWidgetGetWidth(mainWeb1Sprite));
			ituWidgetSetX(mainInfo1Background, 0);
        }
        break;

    case 2:
        if (orient == EXTERNAL_VERTICAL)
        {
            ituWidgetSetY(mainArrowFloor1Container, 0);
            ituWidgetSetY(mainDateContainer, 0);
            ituWidgetSetY(mainTimeContainer, ituWidgetGetHeight(mainDateContainer));
            ituWidgetSetY(mainTemperatureContainer, ituWidgetGetY(mainTimeContainer) + ituWidgetGetHeight(mainTimeContainer));
            ituWidgetSetY(mainPhoto1Slideshow, ituWidgetGetHeight(mainArrowFloor1Container));

			ituWidgetSetY(mainVideo, ituWidgetGetHeight(mainArrowFloor1Container));
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetY(mainLogo1Sprite, ituWidgetGetHeight(mainArrowFloor1Container));
            ituWidgetSetY(mainWeb1Sprite, ituWidgetGetHeight(mainArrowFloor1Container));
			ituWidgetSetY(mainInfo1Background, mainLayerHeight - ituWidgetGetHeight(mainInfo1Background));
        }
        else
        {
            ituWidgetSetX(mainArrowFloor1Container, 0);
            ituWidgetSetX(mainDateContainer, 0);
            ituWidgetSetX(mainTimeContainer, 0);
            ituWidgetSetX(mainTemperatureContainer, 0);
            ituWidgetSetX(mainPhoto1Slideshow, mainLayerWidth - ituWidgetGetWidth(mainPhoto1Slideshow));

			ituWidgetSetX(mainVideo, mainLayerWidth - ituWidgetGetWidth(mainPhoto1Slideshow));
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetX(mainLogo1Sprite, ituWidgetGetWidth(mainArrowFloor1Container));
            ituWidgetSetX(mainWeb1Sprite, mainLayerWidth - ituWidgetGetWidth(mainWeb1Sprite));
			ituWidgetSetX(mainInfo1Background, ituWidgetGetWidth(mainArrowFloor1Container));
        }
        break;

    case 3:
        if (orient == EXTERNAL_VERTICAL)
        {
			ituWidgetSetY(mainInfo1Background, ituWidgetGetY(mainLogo1Sprite) + ituWidgetGetHeight(mainLogo1Sprite));
        }
        else
        {
			ituWidgetSetY(mainInfo1Background, ituWidgetGetY(mainLogo1Sprite) + ituWidgetGetHeight(mainLogo1Sprite));
        }
        break;

    case 4:
        if (orient == EXTERNAL_VERTICAL)
        {
			ituWidgetSetY(mainInfo1Background, mainLayerHeight - ituWidgetGetHeight(mainPhoto1Slideshow) / 2 - ituWidgetGetHeight(mainInfo1Background) / 2);
        }
        else
        {
			ituWidgetSetY(mainInfo1Background, ituWidgetGetHeight(mainPhoto1Slideshow) / 2 - ituWidgetGetHeight(mainInfo1Background) / 2);
        }
        break;

    case 5:
        if (orient == EXTERNAL_VERTICAL)
        {
			ituWidgetSetY(mainInfo1Background, mainLayerHeight - ituWidgetGetHeight(mainInfo1Background));
        }
        else
        {
			ituWidgetSetY(mainInfo1Background, mainLayerHeight - ituWidgetGetHeight(mainInfo1Background));
        }
        theConfig.info1_format = 1;
        MainUpdateByConfig();
        break;

    case 6:
        theConfig.info1_format = 0;
        MainUpdateByConfig();
        break;

    case 7:
        theConfig.info1_format = 2;
        MainUpdateByConfig();
        break;

    case 8:
        theConfig.info1_format = 3;
        MainUpdateByConfig();
        break;

    case 9:
        if (orient == EXTERNAL_VERTICAL)
        {
            ituWidgetSetX(mainLogo1Sprite, mainLayerWidth - ituWidgetGetWidth(mainPhoto1Slideshow) / 2 - ituWidgetGetWidth(mainLogo1Sprite) / 2);
            ituWidgetSetY(mainLogo1Sprite, mainLayerHeight - ituWidgetGetHeight(mainPhoto1Slideshow) / 2 - ituWidgetGetHeight(mainLogo1Sprite) / 2);
            ituWidgetSetX(mainWeb1Sprite, 0);
        }
        else
        {
            ituWidgetSetX(mainLogo1Sprite, mainLayerWidth - ituWidgetGetWidth(mainPhoto1Slideshow) / 2 - ituWidgetGetWidth(mainLogo1Sprite) / 2);
            ituWidgetSetY(mainLogo1Sprite, mainLayerHeight - ituWidgetGetHeight(mainPhoto1Slideshow) / 2 - ituWidgetGetHeight(mainLogo1Sprite) / 2);
            ituWidgetSetX(mainWeb1Sprite, ituWidgetGetWidth(mainArrowFloor1Container));
        }
		ituSpriteGoto(mainLogo1Sprite, 4);
		ituSpriteGoto(mainWeb1Sprite, 4);
		ituSpriteGoto(mainInfo1Sprite, 4);
        break;

    case 10:
        if (orient == EXTERNAL_VERTICAL)
        {
            ituWidgetSetX(mainLogo1Sprite, 0);
            ituWidgetSetY(mainLogo1Sprite, ituWidgetGetHeight(mainArrowFloor1Container));
            ituWidgetSetX(mainWeb1Sprite, mainLayerWidth - ituWidgetGetWidth(mainWeb1Sprite));
        }
        else
        {
            ituWidgetSetX(mainLogo1Sprite, ituWidgetGetWidth(mainArrowFloor1Container));
            ituWidgetSetY(mainLogo1Sprite, 0);
            ituWidgetSetX(mainWeb1Sprite, mainLayerWidth - ituWidgetGetWidth(mainWeb1Sprite));
        }
		ituSpriteGoto(mainLogo1Sprite, theConfig.logo1_format);
		ituSpriteGoto(mainWeb1Sprite, theConfig.web1_format);
		ituSpriteGoto(mainInfo1Sprite, theConfig.info1_format);
        ev.type = EXTERNAL_WARN1;
        ev.arg1 = EXTERNAL_WARN_MAINTENANCE;
        MainProcessExternalEvent(&ev);
        break;

    case 11:
        ev.type = EXTERNAL_WARN1;
        ev.arg1 = EXTERNAL_WARN_EARTHQUAKE;
        MainProcessExternalEvent(&ev);

        break;

    case 12:
        ev.type = EXTERNAL_WARN1;
        ev.arg1 = EXTERNAL_WARN_FIRE;
        MainProcessExternalEvent(&ev);

        break;

    case 13:
        ev.type = EXTERNAL_WARN1;
        ev.arg1 = EXTERNAL_WARN_OVERLOAD;
        MainProcessExternalEvent(&ev);
        break;

    case 14:
        ev.type = EXTERNAL_WARN1;
        ev.arg1 = EXTERNAL_WARN_NONE;
        MainProcessExternalEvent(&ev);
        ituWidgetSetVisible(mainDateContainer, false);
        ituWidgetSetVisible(mainTimeContainer, false);
        ituWidgetSetVisible(mainTemperatureContainer, false);
        ituWidgetSetVisible(mainPhoto2Slideshow, true);
        break;

    case 15:
        if (orient == EXTERNAL_VERTICAL)
        {
            ituWidgetSetPosition(mainDateContainer, 0, ituWidgetGetHeight(mainArrowFloor1Container));
            ituWidgetSetPosition(mainTimeContainer, 0, ituWidgetGetY(mainDateContainer) + ituWidgetGetHeight(mainDateContainer));
            ituWidgetSetX(mainTemperatureContainer, ituWidgetGetHeight(mainArrowFloor1Container));
            ituWidgetSetY(mainTemperatureContainer, ituWidgetGetY(mainDateContainer) + ituWidgetGetHeight(mainDateContainer) / 2);
            ituWidgetSetY(mainPhoto1Slideshow, ituWidgetGetY(mainTimeContainer) + ituWidgetGetHeight(mainTimeContainer));
            ituWidgetSetHeight(mainPhoto1Slideshow, mainLayerHeight - ituWidgetGetY(mainTimeContainer) - ituWidgetGetHeight(mainTimeContainer));

			ituWidgetSetY(mainVideo, ituWidgetGetY(mainTimeContainer) + ituWidgetGetHeight(mainTimeContainer));
			ituWidgetSetHeight(mainVideo, mainLayerHeight - ituWidgetGetY(mainTimeContainer) - ituWidgetGetHeight(mainTimeContainer));
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetY(mainLogo1Sprite, ituWidgetGetY(mainPhoto1Slideshow));
            ituWidgetSetY(mainWeb1Sprite, ituWidgetGetY(mainPhoto1Slideshow));
        }
        else
        {
            ituWidgetSetWidth(mainPhoto1Slideshow, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor1Container) - ituWidgetGetWidth(mainArrowFloor2Container));

			ituWidgetSetWidth(mainVideo, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor1Container) - ituWidgetGetWidth(mainArrowFloor2Container));
			ituWidgetUpdate(mainVideo, ITU_EVENT_LAYOUT, 0, 0, 0);

            ituWidgetSetX(mainWeb1Sprite, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor2Container) - ituWidgetGetWidth(mainWeb1Sprite));
			ituWidgetSetWidth(mainInfo1Background, ituWidgetGetWidth(mainPhoto1Slideshow));
            ituWidgetSetX(mainTemperatureContainer, mainLayerWidth - ituWidgetGetWidth(mainArrowFloor2Container));
            ituWidgetSetY(mainTemperatureContainer, ituWidgetGetY(mainDateContainer) + ituWidgetGetHeight(mainDateContainer) / 2);
        }
        ituWidgetSetVisible(mainDateContainer, true);
        ituWidgetSetVisible(mainTimeContainer, true);
        ituWidgetSetVisible(mainTemperatureContainer, true);
        ituWidgetSetVisible(mainPhoto2Slideshow, false);
        ituWidgetSetVisible(mainArrowFloor2Container, true);
        break;
    }
}

bool MainOnKeyDown(ITUWidget* widget, char* param)
{
    int key = atoi(param);

    switch (key)
    {
    case SDLK_RETURN:
        break;

    case SDLK_UP:
        break;

    case SDLK_DOWN:
        break;

    case SDLK_INSERT:
        break;

    case SDLK_SPACE:
        if (theConfig.demo_enable)
            MainChangeLayout();
        break;
    }
    return true;
}

static int FloorToFrame(int floor)
{
    if (floor == '-')
        return 0;
    else if (floor >= '0' && floor <= '9')
        return floor - '0' + 1;
    else
        return floor - 'A' + 1 + 10;
}

static MainProcessExternalFloor(ExternalEvent* ev)
{
    ITUWidget *parent, *widget1, *widget2, *widget3;
    ITUContainer* mainFloorContainer;
    ITUSprite* mainFloor1Sprite;
    ITUSprite* mainFloor2Sprite;
    ITUSprite* mainFloor3Sprite;

    if (ev->type == EXTERNAL_FLOOR2)
    {
        mainFloorContainer = mainFloor2Container;
        mainFloor1Sprite = mainFloor21Sprite;
        mainFloor2Sprite = mainFloor22Sprite;
        mainFloor3Sprite = mainFloor23Sprite;
    }
    else
    {
        mainFloorContainer = mainFloor1Container;
        mainFloor1Sprite = mainFloor11Sprite;
        mainFloor2Sprite = mainFloor12Sprite;
        mainFloor3Sprite = mainFloor13Sprite;
    }

    assert(ev);

    if (ev->arg1 == EXTERNAL_FLOOR_NONE)
    {
        ituWidgetSetVisible(mainFloor1Sprite, false);
        ituWidgetSetVisible(mainFloor2Sprite, false);
        ituWidgetSetVisible(mainFloor3Sprite, false);
    }
    else
    {
        if (ev->arg2 == EXTERNAL_FLOOR_NONE)
        {
            int frame1 = FloorToFrame(ev->arg1);

            ituWidgetSetVisible(mainFloor1Sprite, true);
            ituWidgetSetVisible(mainFloor2Sprite, false);
            ituWidgetSetVisible(mainFloor3Sprite, false);

            ituSpriteGoto(mainFloor1Sprite, frame1);

            parent = (ITUWidget*)mainFloorContainer;
            widget1 = (ITUWidget*)mainFloor1Sprite;

            widget1->rect.x = parent->rect.width / 2 - widget1->rect.width / 2;
        }
        else
        {
            if (ev->arg3 == EXTERNAL_FLOOR_NONE)
            {
                int frame1 = FloorToFrame(ev->arg1);
                int frame2 = FloorToFrame(ev->arg2);

                ituWidgetSetVisible(mainFloor1Sprite, true);
                ituWidgetSetVisible(mainFloor2Sprite, true);
                ituWidgetSetVisible(mainFloor3Sprite, false);

                ituSpriteGoto(mainFloor1Sprite, frame1);
                ituSpriteGoto(mainFloor2Sprite, frame2);

                parent = (ITUWidget*)mainFloorContainer;
                widget1 = (ITUWidget*)mainFloor1Sprite;
                widget2 = (ITUWidget*)mainFloor2Sprite;

                widget1->rect.x = parent->rect.width / 2 - widget1->rect.width;
                widget2->rect.x = widget1->rect.x + widget1->rect.width;
            }
            else
            {
                int frame1 = FloorToFrame(ev->arg1);
                int frame2 = FloorToFrame(ev->arg2);
                int frame3 = FloorToFrame(ev->arg3);

                ituWidgetSetVisible(mainFloor1Sprite, true);
                ituWidgetSetVisible(mainFloor2Sprite, true);
                ituWidgetSetVisible(mainFloor3Sprite, true);

                ituSpriteGoto(mainFloor1Sprite, frame1);
                ituSpriteGoto(mainFloor2Sprite, frame2);
                ituSpriteGoto(mainFloor3Sprite, frame3);

                parent = (ITUWidget*)mainFloorContainer;
                widget1 = (ITUWidget*)mainFloor1Sprite;
                widget2 = (ITUWidget*)mainFloor2Sprite;
                widget3 = (ITUWidget*)mainFloor3Sprite;

                widget2->rect.x = parent->rect.width / 2 - widget2->rect.width / 2;
                widget1->rect.x = widget2->rect.x - widget1->rect.width;
                widget3->rect.x = widget2->rect.x + widget2->rect.width;
            }
        }
    }
}

static int MainAudioPlayCallback(int state)
{
    char path[PATH_MAX];

    switch (state)
    {
    case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
        switch (playState)
        {
        case PLAY_CHINESE:
			if (targetFloor >= 0)
				sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/t%dfar.mp3", targetFloor);
			else
				sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/tb%dfar.mp3", -targetFloor);

            if (AudioPlay(path, MainAudioPlayCallback) == 0)
            {
                playState = PLAY_TAIWANESE;
            }
            else
            {
                playState = PLAY_NONE;
                targetFloor = INT_MAX;
            }
            break;

        case PLAY_TAIWANESE:
			if (nextTargetFloor == INT_MAX)
            {
                playState = PLAY_NONE;
				targetFloor = INT_MAX;
            }
            else
            {
                targetFloor = nextTargetFloor;
				nextTargetFloor = INT_MAX;
				if (targetFloor >= 0)
					sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/c%dfar.mp3", targetFloor);
				else
					sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/cb%dfar.mp3", -targetFloor);

                if (AudioPlay(path, MainAudioPlayCallback) == 0)
                {
                    playState = PLAY_CHINESE;
                }
                else
                {
                    playState = PLAY_NONE;
					targetFloor = INT_MAX;
                }
            }
            break;

        case PLAY_MUSIC:
            if (theConfig.sound_enable)
            {
                ITUScrollText* item = ituMediaFileListNext(mainMusicMediaFileListBox);
                while (item)
                {
                    char* filepath = (char*)ituWidgetGetCustomData(item);
                    if (AudioPlay(filepath, MainAudioPlayCallback) == 0)
                        break;

                    item = ituMediaFileListNext(mainMusicMediaFileListBox);
                }
                if (!item)
                    playState = PLAY_NONE;
            }
            else
            {
                playState = PLAY_NONE;
            }
            break;

        case PLAY_WARN:
            if (theConfig.sound_enable)
            {
                if (AudioPlay(mainWarnSoundPath, MainAudioPlayCallback) < 0)
                    playState = PLAY_NONE;
            }
            else
            {
                playState = PLAY_NONE;
            }
            break;
        }
        break;
    }
    return 0;
}

static MainProcessExternalWarn(ExternalEvent* ev)
{
    ITUSprite* warnSprite;
    assert(ev);

    warnSprite = warn1Sprite;

    if (ev->arg1 == EXTERNAL_WARN_NONE)
    {
		ituWidgetSetVisible(warnBackground, false);
        if (playState == PLAY_WARN)
        {
            playState = PLAY_NONE;
            AudioStop();
        }
    }
    else
    {
		ituWidgetSetVisible(warnBackground, true);
        ituSpriteGoto(warnSprite, ev->arg1 - 1);

        switch (ev->arg1)
        {
        case EXTERNAL_WARN_EARTHQUAKE:
            strcpy(mainWarnSoundPath, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/earthquake.mp3");
            break;

        case EXTERNAL_WARN_FIRE:
            strcpy(mainWarnSoundPath, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/fire.mp3");
            break;

        case EXTERNAL_WARN_MAINTENANCE:
            strcpy(mainWarnSoundPath, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/maintenance.mp3");
            break;

        case EXTERNAL_WARN_OVERLOAD:
            strcpy(mainWarnSoundPath, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/overload.mp3");
            break;
        }

        if (theConfig.sound_enable)
        {
            if (AudioPlay(mainWarnSoundPath, MainAudioPlayCallback) == 0)
            {
                playState = PLAY_WARN;
				targetFloor = nextTargetFloor = INT_MAX;
            }
        }
    }
}

static void MainSlideshowOnStop(ITUSlideshow* slideshow)
{
    if (theConfig.demo_enable)
    {
        ITUScrollText* item;
        ituListBoxSelect(&mainVideoMediaFileListBox->listbox, 0);
        item = ituMediaFileListPlay(mainVideoMediaFileListBox);
        if (item)
        {
            char* filepath = (char*)ituWidgetGetCustomData(item);
            strcpy(mainVideo->filePath, filepath);
#ifdef CFG_VIDEO_ENABLE
            if(mtal_pb_get_audio_codec_id(mainVideo->filePath) != 0)
#endif
            {
                //stop background audio
				if (playState == PLAY_MUSIC)
				{
					AudioStop();
					playState = PLAY_NONE;
				}
                else if (playState == PLAY_CHINESE || playState == PLAY_TAIWANESE)
                {
                    // TODO: FIX ME
                    AudioStop();
                    playState = PLAY_NONE;
					targetFloor = nextTargetFloor = INT_MAX;
                }
            }
            printf("play video %s\n", mainVideo->filePath);
            ituVideoPlay(mainVideo, 0);

            ituWidgetSetVisible(mainPhoto1Slideshow, false);
            ituWidgetSetVisible(mainVideo, true);
        }
        else
        {
            ituSlideshowPlay(mainPhoto1Slideshow, 0);
        }
    }
}

static void MainVideoOnStop(ITUVideo* video)
{
    if (ituWidgetIsVisible(video))
    {
        ITUScrollText* item = ituMediaFileListNext(mainVideoMediaFileListBox);
        if (item)
        {
            char* filepath = (char*)ituWidgetGetCustomData(item);
            strcpy(mainVideo->filePath, filepath);
		#ifdef CFG_VIDEO_ENABLE
            if(mtal_pb_get_audio_codec_id(mainVideo->filePath) != 0)
            {
                //stop background audio
				if (playState == PLAY_MUSIC)
				{
					AudioStop();
					playState = PLAY_NONE;
				}
                else if (playState == PLAY_CHINESE || playState == PLAY_TAIWANESE)
                {
                    // TODO: FIX ME
                    AudioStop();
                    playState = PLAY_NONE;
					targetFloor = nextTargetFloor = INT_MAX;
                }
            }
			else
		#endif // CFG_VIDEO_ENABLE
			{
				//play background audio
				if (playState == PLAY_NONE)
				{
					ITUScrollText* item = ituMediaFileListNext(mainMusicMediaFileListBox);
					if (item)
					{
						char* filepath = (char*)ituWidgetGetCustomData(item);
						if (AudioPlay(filepath, MainAudioPlayCallback) == 0)
						    playState = PLAY_MUSIC;
					}
				}
			}
            printf("play video %s\n", mainVideo->filePath);
            ituVideoPlay(mainVideo, 0);
        }
        else if (theConfig.demo_enable)
        {
            ituWidgetSetVisible(mainPhoto1Slideshow, true);
            ituWidgetSetVisible(mainVideo, false);

            ituSlideshowPlay(mainPhoto1Slideshow, 0);
        }
    }
}

static MainProcessExternalFloorArrive(ExternalEvent* ev)
{
    char path[PATH_MAX];
    assert(ev);

    if (ev->arg1 == EXTERNAL_FLOOR_NONE)
    {
        // DO NOTHING
    }
    else
    {
		if (targetFloor == INT_MAX)
        {
            if (ev->arg2 == EXTERNAL_FLOOR_NONE)
            {
                if (ev->arg1 >= '0' && ev->arg1 <= '9')
                {
                    int floor = ev->arg1 - '0';

                    if (theConfig.sound_enable && playState != PLAY_WARN)
                    {
                        sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/c%dfar.mp3", floor);
                        if (AudioPlay(path, MainAudioPlayCallback) == 0)
                            playState = PLAY_CHINESE;
                    }

                    if (theConfig.photo1_format && !theConfig.demo_enable)
                        ituSlideshowGoto(mainPhoto1Slideshow, floor % itcTreeGetChildCount(mainPhoto1Slideshow));

                    if (theConfig.photo2_format)
                        ituSlideshowGoto(mainPhoto2Slideshow, floor % itcTreeGetChildCount(mainPhoto2Slideshow));

                    if (theConfig.sound_enable && playState == PLAY_CHINESE)
                        targetFloor = floor;
                }
            }
            else
            {
                if (ev->arg3 == EXTERNAL_FLOOR_NONE)
                {
                    if (ev->arg1 >= '0' && ev->arg1 <= '9' && ev->arg2 >= '0' && ev->arg2 <= '9')
                    {
                        int floor = (ev->arg1 - '0') * 10 + ev->arg2 - '0';

                        if (theConfig.sound_enable && playState != PLAY_WARN)
                        {
                            sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/c%dfar.mp3", floor);
                            if (AudioPlay(path, MainAudioPlayCallback) == 0)
                                playState = PLAY_CHINESE;
                        }

                        if (theConfig.photo1_format && !theConfig.demo_enable)
                            ituSlideshowGoto(mainPhoto1Slideshow, floor % itcTreeGetChildCount(mainPhoto1Slideshow));

                        if (theConfig.photo2_format)
                            ituSlideshowGoto(mainPhoto2Slideshow, floor % itcTreeGetChildCount(mainPhoto2Slideshow));

                        if (theConfig.sound_enable && playState == PLAY_CHINESE)
                            targetFloor = floor;
                    }
					else if (ev->arg1 == 'B' && ev->arg2 >= '0' && ev->arg2 <= '9')
					{
						int floor = -(ev->arg2 - '0');

						if (theConfig.sound_enable && playState != PLAY_WARN)
						{
							sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/cb%dfar.mp3", -floor);
							if (AudioPlay(path, MainAudioPlayCallback) == 0)
								playState = PLAY_CHINESE;
						}

						if (theConfig.photo1_format && !theConfig.demo_enable)
							ituSlideshowGoto(mainPhoto1Slideshow, -floor % itcTreeGetChildCount(mainPhoto1Slideshow));

						if (theConfig.photo2_format)
							ituSlideshowGoto(mainPhoto2Slideshow, -floor % itcTreeGetChildCount(mainPhoto2Slideshow));

						if (theConfig.sound_enable && playState == PLAY_CHINESE)
							targetFloor = floor;
					}
                }
                else
                {
                    if (ev->arg1 >= '0' && ev->arg1 <= '9' && ev->arg2 >= '0' && ev->arg2 <= '9' && ev->arg3 >= '0' && ev->arg3 <= '9')
                    {
                        int floor = (ev->arg1 - '0') * 100 + (ev->arg2 - '0') * 10 + ev->arg3 - '0';

                        if (theConfig.sound_enable && playState != PLAY_WARN)
                        {
                            sprintf(path, CFG_PUBLIC_DRIVE ":/" CFG_RES_PATH "/Voice/c%dfar.mp3", floor);
                            if (AudioPlay(path, MainAudioPlayCallback) == 0)
                                playState = PLAY_CHINESE;
                        }
                        if (theConfig.photo1_format && !theConfig.demo_enable)
                            ituSlideshowGoto(mainPhoto1Slideshow, floor % itcTreeGetChildCount(mainPhoto1Slideshow));

                        if (theConfig.photo2_format)
                            ituSlideshowGoto(mainPhoto2Slideshow, floor % itcTreeGetChildCount(mainPhoto2Slideshow));

                        if (theConfig.sound_enable && playState == PLAY_CHINESE)
                            targetFloor = floor;
                    }
                }
            }
        }
        else if (theConfig.sound_enable)
        {
            if (ev->arg2 == EXTERNAL_FLOOR_NONE)
            {
                if (ev->arg1 >= '0' && ev->arg1 <= '9')
                {
                    nextTargetFloor = ev->arg1 - '0';
                }
            }
            else
            {
                if (ev->arg3 == EXTERNAL_FLOOR_NONE)
                {
                    if (ev->arg1 >= '0' && ev->arg1 <= '9' && ev->arg2 >= '0' && ev->arg2 <= '9')
                    {
                        nextTargetFloor = (ev->arg1 - '0') * 10 + ev->arg2 - '0';
                    }
					else if (ev->arg1 == 'B' && ev->arg2 >= '0' && ev->arg2 <= '9')
					{
						nextTargetFloor = -(ev->arg2 - '0');
					}
                }
                else
                {
                    if (ev->arg1 >= '0' && ev->arg1 <= '9' && ev->arg2 >= '0' && ev->arg2 <= '9' && ev->arg3 >= '0' && ev->arg3 <= '9')
                    {
                        nextTargetFloor = (ev->arg1 - '0') * 100 + (ev->arg2 - '0') * 10 + ev->arg3 - '0';
                    }
                }
            }
        }
    }
}

void MainProcessExternalEvent(ExternalEvent* ev)
{
    assert(ev);
    switch (ev->type)
    {
    case EXTERNAL_ARROW1:
        printf("EXTERNAL_ARROW1: %d\n", ev->arg1);
        switch (ev->arg1)
        {
        case EXTERNAL_ARROW_NONE:
			if (mainArrow1UpSprite)
				ituSpriteStop(mainArrow1UpSprite);

			if (mainArrow1DownSprite)
				ituSpriteStop(mainArrow1DownSprite);

			if (mainArrow1UpAnimation)
				mainArrow1UpAnimation->repeat = 0;

			if (mainArrow1DownAnimation)
				mainArrow1DownAnimation->repeat = 0;
            break;

        case EXTERNAL_ARROW_UP:
			if (mainArrow1UpSprite)
			{
				ituSpritePlay(mainArrow1UpSprite, -1);
				ituWidgetSetVisible(mainArrow1UpSprite, true);
			}
			if (mainArrow1DownSprite)
			{
				ituSpriteStop(mainArrow1DownSprite);
				ituWidgetSetVisible(mainArrow1DownSprite, false);
			}
			if (mainArrow1UpAnimation)
			{
				mainArrow1UpAnimation->repeat = 1;
				ituAnimationPlay(mainArrow1UpAnimation, 0);
				ituWidgetSetVisible(mainArrow1UpAnimation, true);
			}
			if (mainArrow1DownAnimation)
			{
				ituAnimationStop(mainArrow1DownAnimation);
				ituWidgetSetVisible(mainArrow1DownAnimation, false);
			}
            break;

        case EXTERNAL_ARROW_DOWN:
			if (mainArrow1UpSprite)
			{
				ituSpriteStop(mainArrow1UpSprite);
				ituWidgetSetVisible(mainArrow1UpSprite, false);
			}
			if (mainArrow1DownSprite)
			{
				ituSpritePlay(mainArrow1DownSprite, -1);
				ituWidgetSetVisible(mainArrow1DownSprite, true);
			}
			if (mainArrow1UpAnimation)
			{
				ituAnimationStop(mainArrow1UpAnimation);
				ituWidgetSetVisible(mainArrow1UpAnimation, false);
			}
			if (mainArrow1DownAnimation)
			{
				mainArrow1DownAnimation->repeat = 1;
				ituAnimationPlay(mainArrow1DownAnimation, 0);
				ituWidgetSetVisible(mainArrow1DownAnimation, true);
			}
            break;
        }
        break;

    case EXTERNAL_ARROW2:
        printf("EXTERNAL_ARROW2: %d\n", ev->arg1);
        switch (ev->arg1)
        {
        case EXTERNAL_ARROW_NONE:
			if (mainArrow2UpSprite)
				ituSpriteStop(mainArrow2UpSprite);

			if (mainArrow2DownSprite)
				ituSpriteStop(mainArrow2DownSprite);

			if (mainArrow2UpAnimation)
				mainArrow2UpAnimation->repeat = 0;

			if (mainArrow2DownAnimation)
				mainArrow2DownAnimation->repeat = 0;
            break;

        case EXTERNAL_ARROW_UP:
			if (mainArrow2UpSprite)
			{
				ituSpritePlay(mainArrow2UpSprite, -1);
				ituWidgetSetVisible(mainArrow2UpSprite, true);
			}
			if (mainArrow2DownSprite)
			{
				ituSpriteStop(mainArrow2DownSprite);
				ituWidgetSetVisible(mainArrow2DownSprite, false);
			}
			if (mainArrow2UpAnimation)
			{
				mainArrow2UpAnimation->repeat = 1;
				ituAnimationPlay(mainArrow2UpAnimation, 0);
				ituWidgetSetVisible(mainArrow2UpAnimation, true);
			}
			if (mainArrow2DownAnimation)
			{
				ituAnimationStop(mainArrow2DownAnimation);
				ituWidgetSetVisible(mainArrow2DownAnimation, false);
			}
            break;

        case EXTERNAL_ARROW_DOWN:
			if (mainArrow2UpSprite)
			{
				ituSpriteStop(mainArrow2UpSprite);
				ituWidgetSetVisible(mainArrow2UpSprite, false);
			}
			if (mainArrow2DownSprite)
			{
				ituSpritePlay(mainArrow2DownSprite, -1);
				ituWidgetSetVisible(mainArrow2DownSprite, true);
			}
			if (mainArrow2UpAnimation)
			{
				ituAnimationStop(mainArrow2UpAnimation);
				ituWidgetSetVisible(mainArrow2UpAnimation, false);
			}
			if (mainArrow2DownAnimation)
			{
				mainArrow2DownAnimation->repeat = 1;
				ituAnimationPlay(mainArrow2DownAnimation, 0);
				ituWidgetSetVisible(mainArrow2DownAnimation, true);
			}
            break;
        }
        break;

    case EXTERNAL_FLOOR1:
        printf("EXTERNAL_FLOOR1: %c %c %c\n", ev->arg1 > 0 ? ev->arg1 : ' ', ev->arg2 > 0 ? ev->arg2 : ' ', ev->arg3 > 0 ? ev->arg3 : ' ');
        MainProcessExternalFloor(ev);
        break;

    case EXTERNAL_FLOOR2:
        printf("EXTERNAL_FLOOR2: %c %c %c\n", ev->arg1 > 0 ? ev->arg1 : ' ', ev->arg2 > 0 ? ev->arg2 : ' ', ev->arg3 > 0 ? ev->arg3 : ' ');
        MainProcessExternalFloor(ev);
        break;

    case EXTERNAL_TEMPERATURE:
        printf("EXTERNAL_TEMPERATURE: %d\n", ev->arg1);
        {
            char buf[32];

            ituWidgetSetVisible(mainTemperatureText, true);

            sprintf(buf, "%d°C", ev->arg1);
            ituTextSetString(mainTemperatureText, buf);
        }
        break;

    case EXTERNAL_WARN1:
        printf("EXTERNAL_WARN: %d\n", ev->arg1);
        MainProcessExternalWarn(ev);
        break;

    case EXTERNAL_FLOOR_ARRIVE1:
        printf("EXTERNAL_FLOOR_ARRIVE1: %c %c %c\n", ev->arg1 > 0 ? ev->arg1 : ' ', ev->arg2 > 0 ? ev->arg2 : ' ', ev->arg3 > 0 ? ev->arg3 : ' ');
        MainProcessExternalFloorArrive(ev);
        break;

    case EXTERNAL_FLOOR_ARRIVE2:
        printf("EXTERNAL_FLOOR_ARRIVE2: %c %c %c\n", ev->arg1 > 0 ? ev->arg1 : ' ', ev->arg2 > 0 ? ev->arg2 : ' ', ev->arg3 > 0 ? ev->arg3 : ' ');
        MainProcessExternalFloorArrive(ev);
        break;
    }
}

bool MainOnTimer(ITUWidget* widget, char* param)
{
	if (theConfig.sound_enable && playState == PLAY_NONE)
    {
        if (ituWidgetIsVisible(warnBackground))
        {
            if (AudioPlay(mainWarnSoundPath, MainAudioPlayCallback) == 0)
            {
                playState = PLAY_WARN;
                targetFloor = nextTargetFloor = INT_MAX;
            }
        }
        else if (ituWidgetIsVisible(mainMusicMediaFileListBox) && !mainVideo->playing)
        {
            if ((mainMusicMediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
                (mainMusicMediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
                (mainMusicMediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
            {
                ITUScrollText* item = ituMediaFileListPlay(mainMusicMediaFileListBox);
                while (item)
                {
                    char* filepath = (char*)ituWidgetGetCustomData(item);
                    if (AudioPlay(filepath, MainAudioPlayCallback) == 0)
                    {
                        playState = PLAY_MUSIC;
                        break;
                    }
                    item = ituMediaFileListNext(mainMusicMediaFileListBox);
                }
            }
        }
    }

    if (ituWidgetIsVisible(mainVideo) && ituWidgetIsVisible(mainVideoMediaFileListBox) && !mainVideo->playing)
    {
        if ((mainVideoMediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
            (mainVideoMediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
            (mainVideoMediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
        {
            ITUScrollText* item = ituMediaFileListPlay(mainVideoMediaFileListBox);
            if (item)
            {
                char* filepath = (char*)ituWidgetGetCustomData(item);
                strcpy(mainVideo->filePath, filepath);

            #ifdef CFG_VIDEO_ENABLE
                if(mtal_pb_get_audio_codec_id(mainVideo->filePath) != 0)
            #endif
                {
                    //stop background audio
				    if (playState == PLAY_MUSIC)
				    {
					    AudioStop();
					    playState = PLAY_NONE;
				    }
                    else if (playState == PLAY_CHINESE || playState == PLAY_TAIWANESE)
                    {
                        // TODO: FIX ME
                        AudioStop();
                        playState = PLAY_NONE;
						targetFloor = nextTargetFloor = INT_MAX;
                    }
                }
                ituVideoPlay(mainVideo, 0);
            }
        }
    }

	if (!activeMainInfo1Animation && ituWidgetIsVisible(mainInfo1Background) && ituWidgetIsVisible(mainInfo1TextMediaFileListBox) && ituWidgetIsVisible(mainInfo1FontMediaFileListBox))
    {
        if ((mainInfo1TextMediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
            (mainInfo1TextMediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
            (mainInfo1TextMediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
        {
            ITUScrollText* item = ituMediaFileListPlay(mainInfo1TextMediaFileListBox);
            if (item)
            {
                char* filepath = (char*)ituWidgetGetCustomData(item);
                FILE* file = fopen(filepath, "r");
                if (file)
                {
                    if (fgets(infoBuf, sizeof(infoBuf), file))
                    {
                        ITUText* text;
                        int len = strlen(infoBuf);

                        if (len > 1 && infoBuf[len - 1] == '\n')
                            infoBuf[len - 1] = '\0';

                        if (len > 2 && infoBuf[len - 2] == '\r')
                            infoBuf[len - 2] = '\0';

                        switch (theConfig.info1_format)
                        {
                        case 1:
                            activeMainInfo1Animation = mainInfo12Animation;
                            break;
                        case 2:
                            activeMainInfo1Animation = mainInfo13Animation;
                            break;
                        case 3:
                            activeMainInfo1Animation = mainInfo14Animation;
                            break;
                        default:
                            activeMainInfo1Animation = mainInfo11Animation;
                            break;
                        }
                        text = (ITUText*)itcTreeGetChildAt(activeMainInfo1Animation, 0);
                        ituTextSetString(text, infoBuf);
						ituTextSetString(mainInfo15Text, infoBuf);
                        mainInfo1File = file;
                        strcpy(mainInfo1FilePath, filepath);

                        ituAnimationPlay(activeMainInfo1Animation, 0);
                    }
                }
            }
        }

        if (mainInfo1FontLoaded == false &&
            (mainInfo1FontMediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
            (mainInfo1FontMediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
            (mainInfo1FontMediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
        {
            ITUScrollText* item = ituMediaFileListPlay(mainInfo1FontMediaFileListBox);
            if (item)
            {
                char* filepath = (char*)ituWidgetGetCustomData(item);
                ituFtLoadFont(1, filepath, ITU_GLYPH_8BPP);
            }
            mainInfo1FontLoaded = true;
        }
    }

	if (!activeMainInfo2Animation && ituWidgetIsVisible(mainInfo2Background) && ituWidgetIsVisible(mainInfo2TextMediaFileListBox) && ituWidgetIsVisible(mainInfo2FontMediaFileListBox))
    {
        if ((mainInfo2TextMediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
            (mainInfo2TextMediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
            (mainInfo2TextMediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
        {
            ITUScrollText* item = ituMediaFileListPlay(mainInfo2TextMediaFileListBox);
            if (item)
            {
                char* filepath = (char*)ituWidgetGetCustomData(item);
                FILE* file = fopen(filepath, "r");
                if (file)
                {
                    if (fgets(infoBuf, sizeof(infoBuf), file))
                    {
                        ITUText* text;
                        int len = strlen(infoBuf);

                        if (len > 1 && infoBuf[len - 1] == '\n')
                            infoBuf[len - 1] = '\0';

                        if (len > 2 && infoBuf[len - 2] == '\r')
                            infoBuf[len - 2] = '\0';

                        switch (theConfig.info2_format)
                        {
                        case 1:
                            activeMainInfo2Animation = mainInfo22Animation;
                            break;
                        case 2:
                            activeMainInfo2Animation = mainInfo23Animation;
                            break;
                        case 3:
                            activeMainInfo2Animation = mainInfo24Animation;
                            break;
                        default:
                            activeMainInfo2Animation = mainInfo21Animation;
                            break;
                        }
                        text = (ITUText*)itcTreeGetChildAt(activeMainInfo2Animation, 0);
                        ituTextSetString(text, infoBuf);
                        mainInfo2File = file;
                        strcpy(mainInfo2FilePath, filepath);

                        ituAnimationPlay(activeMainInfo2Animation, 0);
                    }
                }
            }
        }

        if (mainInfo2FontLoaded == false &&
            (mainInfo2FontMediaFileListBox->mflistboxFlags & ITU_FILELIST_BUSYING) == 0 &&
            (mainInfo2FontMediaFileListBox->mflistboxFlags & ITU_FILELIST_CREATED) == 0 &&
            (mainInfo2FontMediaFileListBox->mflistboxFlags & ITU_FILELIST_DESTROYING) == 0)
        {
            ITUScrollText* item = ituMediaFileListPlay(mainInfo2FontMediaFileListBox);
            if (item)
            {
                char* filepath = (char*)ituWidgetGetCustomData(item);
                ituFtLoadFont(2, filepath, ITU_GLYPH_8BPP);
            }
            mainInfo2FontLoaded = true;
        }
    }
    return false;
}

static void MainInfo1AnimationOnStop(ITUAnimation* animation)
{
    bool ok = false;

	if (mainInfo1File)
	{
		if (fgets(infoBuf, sizeof(infoBuf), mainInfo1File))
		{
			ok = true;
		}
		else
		{
			fclose(mainInfo1File);
			mainInfo1File = fopen(mainInfo1FilePath, "r");
			if (mainInfo1File)
			{
				if (fgets(infoBuf, sizeof(infoBuf), mainInfo1File))
					ok = true;
			}
		}
	}

    if (ok)
    {
        ITUText* text;
        int len = strlen(infoBuf);

        if (len > 1 && infoBuf[len - 1] == '\n')
            infoBuf[len - 1] = '\0';

        if (len > 2 && infoBuf[len - 2] == '\r')
            infoBuf[len - 2] = '\0';

        text = (ITUText*)itcTreeGetChildAt(activeMainInfo1Animation, 0);
        ituTextSetString(text, infoBuf);
    }
    ituAnimationPlay(activeMainInfo1Animation, 0);
}

static void MainInfo2AnimationOnStop(ITUAnimation* animation)
{
    bool ok = false;

	if (mainInfo2File)
	{
		if (fgets(infoBuf, sizeof(infoBuf), mainInfo2File))
		{
			ok = true;
		}
		else
		{
			fclose(mainInfo2File);
			mainInfo2File = fopen(mainInfo2FilePath, "r");
			if (mainInfo2File)
			{
				if (fgets(infoBuf, sizeof(infoBuf), mainInfo2File))
					ok = true;
			}
		}
	}

    if (ok)
    {
        ITUText* text;
        int len = strlen(infoBuf) - 1;
        if (infoBuf[len] == '\n')
            infoBuf[len] = '\0';

        text = (ITUText*)itcTreeGetChildAt(activeMainInfo2Animation, 0);
        ituTextSetString(text, infoBuf);
    }
    ituAnimationPlay(activeMainInfo2Animation, 0);
}

void MainUpdateByConfig(void)
{
    if (ituWidgetIsVisible(mainLayer))
    {
        ituSpriteGoto(mainDateSprite, theConfig.date_format);

        if (theConfig.demo_enable)
        {
            mainPhoto1Slideshow->repeat = false;
            mainVideoMediaFileListBox->repeatMode = ITU_MEDIA_REPEAT_NONE;
            ituWidgetSetVisible(mainPhoto1Slideshow, true);
            ituWidgetSetVisible(mainVideo, false);

            ituSlideshowPlay(mainPhoto1Slideshow, 0);
        }
        else
        {
            mainPhoto1Slideshow->repeat = true;
            mainVideoMediaFileListBox->repeatMode = ITU_MEDIA_REPEAT_ALL;
            ituWidgetSetVisible(mainPhoto1Slideshow, mainPhoto1SlideshowEnabled);
            ituWidgetSetVisible(mainVideo, mainVideoEnabled);

            if (theConfig.photo1_format == 0)
            {
                ituSlideshowPlay(mainPhoto1Slideshow, 0);
            }
            else
            {
                ituSlideshowStop(mainPhoto1Slideshow);
                ituSlideshowGoto(mainPhoto1Slideshow, targetFloor % itcTreeGetChildCount(mainPhoto1Slideshow));
            }
        }

        if (theConfig.photo2_format == 0)
        {
            ituSlideshowPlay(mainPhoto2Slideshow, 0);
        }
        else
        {
            ituSlideshowStop(mainPhoto2Slideshow);
            ituSlideshowGoto(mainPhoto2Slideshow, targetFloor % itcTreeGetChildCount(mainPhoto2Slideshow));
        }
        ituSpriteGoto(mainInfo1Sprite, theConfig.info1_format);
        ituSpriteGoto(mainInfo2Sprite, theConfig.info2_format);

        if (mainInfo1File)
        {
            fclose(mainInfo1File);
            mainInfo1File = NULL;
        }
        activeMainInfo1Animation = NULL;

        if (mainInfo2File)
        {
            fclose(mainInfo2File);
            mainInfo2File = NULL;
        }
        activeMainInfo2Animation = NULL;

        mainVideo->volume = theConfig.audiolevel;

        if (theConfig.sound_enable)
            AudioUnMute();
        else
            AudioMute();
    }
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
    if (!mainLayer)
    {
        ITCTree *node1, *node2;
    #ifdef CFG_NET_WIFI
        char *wifiSsid, *wifiPassword;
    #endif

        mainLayer = ituSceneFindWidget(&theScene, "mainLayer");
        assert(mainLayer);

        mainArrowFloor1Container = ituSceneFindWidget(&theScene, "mainArrowFloor1Container");
        assert(mainArrowFloor1Container);

        mainArrowFloor2Container = ituSceneFindWidget(&theScene, "mainArrowFloor2Container");
        assert(mainArrowFloor2Container);

        mainDateContainer = ituSceneFindWidget(&theScene, "mainDateContainer");
        assert(mainDateContainer);

        mainTimeContainer = ituSceneFindWidget(&theScene, "mainTimeContainer");
        assert(mainTimeContainer);

        mainTemperatureContainer = ituSceneFindWidget(&theScene, "mainTemperatureContainer");
        assert(mainTemperatureContainer);

        mainArrow1UpSprite = ituSceneFindWidget(&theScene, "mainArrow1UpSprite");
        if (mainArrow1UpSprite)
			mainArrow1UpSprite->delay = theConfig.arrow_delay;

        mainArrow1DownSprite = ituSceneFindWidget(&theScene, "mainArrow1DownSprite");
        if (mainArrow1DownSprite)
			mainArrow1DownSprite->delay = theConfig.arrow_delay;

		mainArrow1UpAnimation = ituSceneFindWidget(&theScene, "mainArrow1UpAnimation");
		if (mainArrow1UpAnimation)
			mainArrow1UpAnimation->delay = theConfig.arrow_delay;

		mainArrow1DownAnimation = ituSceneFindWidget(&theScene, "mainArrow1DownAnimation");
		if (mainArrow1DownAnimation)
			mainArrow1DownAnimation->delay = theConfig.arrow_delay;

        mainFloor1Container = ituSceneFindWidget(&theScene, "mainFloor1Container");
        assert(mainFloor1Container);

        mainFloor11Sprite = ituSceneFindWidget(&theScene, "mainFloor11Sprite");
        assert(mainFloor11Sprite);

        mainFloor12Sprite = ituSceneFindWidget(&theScene, "mainFloor12Sprite");
        assert(mainFloor12Sprite);

        mainFloor13Sprite = ituSceneFindWidget(&theScene, "mainFloor13Sprite");
        assert(mainFloor13Sprite);

        mainArrow2UpSprite = ituSceneFindWidget(&theScene, "mainArrow2UpSprite");
        if (mainArrow2UpSprite)
			mainArrow2UpSprite->delay = theConfig.arrow_delay;

        mainArrow2DownSprite = ituSceneFindWidget(&theScene, "mainArrow2DownSprite");
        if (mainArrow2DownSprite)
			mainArrow2DownSprite->delay = theConfig.arrow_delay;

		mainArrow2UpAnimation = ituSceneFindWidget(&theScene, "mainArrow2UpAnimation");
		if (mainArrow2UpAnimation)
			mainArrow2UpAnimation->delay = theConfig.arrow_delay;

		mainArrow2DownAnimation = ituSceneFindWidget(&theScene, "mainArrow2DownAnimation");
		if (mainArrow2DownAnimation)
			mainArrow2DownAnimation->delay = theConfig.arrow_delay;

        mainFloor2Container = ituSceneFindWidget(&theScene, "mainFloor2Container");
        assert(mainFloor2Container);

        mainFloor21Sprite = ituSceneFindWidget(&theScene, "mainFloor21Sprite");
        assert(mainFloor21Sprite);

        mainFloor22Sprite = ituSceneFindWidget(&theScene, "mainFloor22Sprite");
        assert(mainFloor22Sprite);

        mainFloor23Sprite = ituSceneFindWidget(&theScene, "mainFloor23Sprite");
        assert(mainFloor23Sprite);

        mainTemperatureText = ituSceneFindWidget(&theScene, "mainTemperatureText");
        assert(mainTemperatureText);

        mainDateSprite = ituSceneFindWidget(&theScene, "mainDateSprite");
        assert(mainDateSprite);

        mainLogo1Sprite = ituSceneFindWidget(&theScene, "mainLogo1Sprite");
        assert(mainLogo1Sprite);
        ituSpriteGoto(mainLogo1Sprite, theConfig.logo1_format);

        mainLogo11Animation = ituSceneFindWidget(&theScene, "mainLogo11Animation");
        assert(mainLogo11Animation);

        mainLogo111Icon = ituSceneFindWidget(&theScene, "mainLogo111Icon");
        assert(mainLogo111Icon);
        mainLogo111Icon->widget.rect.x = -mainLogo111Icon->widget.rect.width;
        mainLogo11Animation->totalframe = mainLogo11Animation->widget.rect.width + mainLogo111Icon->widget.rect.width;
        if (theConfig.logo1_delay >= 0)
        {
            mainLogo11Animation->delay = theConfig.logo1_delay;
        }
        else
        {
            mainLogo11Animation->delay = 0;
            mainLogo11Animation->totalframe /= -theConfig.logo1_delay;
        }

        mainLogo12Animation = ituSceneFindWidget(&theScene, "mainLogo12Animation");
        assert(mainLogo12Animation);
        mainLogo12Animation->totalframe = mainLogo12Animation->widget.rect.width + mainLogo111Icon->widget.rect.width;
        if (theConfig.logo1_delay >= 0)
        {
            mainLogo12Animation->delay = theConfig.logo1_delay;
        }
        else
        {
            mainLogo12Animation->delay = 0;
            mainLogo12Animation->totalframe /= -theConfig.logo1_delay;
        }

        mainLogo13Animation = ituSceneFindWidget(&theScene, "mainLogo13Animation");
        assert(mainLogo13Animation);
        mainLogo13Animation->totalframe = mainLogo13Animation->widget.rect.height + mainLogo111Icon->widget.rect.height;
        if (theConfig.logo1_delay >= 0)
        {
            mainLogo13Animation->delay = theConfig.logo1_delay;
        }
        else
        {
            mainLogo13Animation->delay = 0;
            mainLogo13Animation->totalframe /= -theConfig.logo1_delay;
        }

        mainLogo14Animation = ituSceneFindWidget(&theScene, "mainLogo14Animation");
        assert(mainLogo14Animation);
        mainLogo14Animation->totalframe = mainLogo14Animation->widget.rect.height + mainLogo111Icon->widget.rect.height;
        if (theConfig.logo1_delay >= 0)
        {
            mainLogo14Animation->delay = theConfig.logo1_delay;
        }
        else
        {
            mainLogo14Animation->delay = 0;
            mainLogo14Animation->totalframe /= -theConfig.logo1_delay;
        }

        mainLogo2Sprite = ituSceneFindWidget(&theScene, "mainLogo2Sprite");
        assert(mainLogo2Sprite);
        ituSpriteGoto(mainLogo2Sprite, theConfig.logo2_format);

        mainLogo21Animation = ituSceneFindWidget(&theScene, "mainLogo21Animation");
        assert(mainLogo21Animation);

        mainLogo211Icon = ituSceneFindWidget(&theScene, "mainLogo211Icon");
        assert(mainLogo211Icon);
        mainLogo211Icon->widget.rect.x = -mainLogo211Icon->widget.rect.width;
        mainLogo21Animation->totalframe = mainLogo21Animation->widget.rect.width + mainLogo211Icon->widget.rect.width;
        if (theConfig.logo2_delay >= 0)
        {
            mainLogo21Animation->delay = theConfig.logo2_delay;
        }
        else
        {
            mainLogo21Animation->delay = 0;
            mainLogo21Animation->totalframe /= -theConfig.logo2_delay;
        }

        mainLogo22Animation = ituSceneFindWidget(&theScene, "mainLogo22Animation");
        assert(mainLogo22Animation);
        mainLogo22Animation->totalframe = mainLogo22Animation->widget.rect.width + mainLogo211Icon->widget.rect.width;
        if (theConfig.logo2_delay >= 0)
        {
            mainLogo22Animation->delay = theConfig.logo2_delay;
        }
        else
        {
            mainLogo22Animation->delay = 0;
            mainLogo22Animation->totalframe /= -theConfig.logo2_delay;
        }

        mainLogo23Animation = ituSceneFindWidget(&theScene, "mainLogo23Animation");
        assert(mainLogo23Animation);
        mainLogo23Animation->totalframe = mainLogo23Animation->widget.rect.height + mainLogo211Icon->widget.rect.height;
        if (theConfig.logo2_delay >= 0)
        {
            mainLogo23Animation->delay = theConfig.logo2_delay;
        }
        else
        {
            mainLogo23Animation->delay = 0;
            mainLogo23Animation->totalframe /= -theConfig.logo2_delay;
        }

        mainLogo24Animation = ituSceneFindWidget(&theScene, "mainLogo24Animation");
        assert(mainLogo24Animation);
        mainLogo24Animation->totalframe = mainLogo24Animation->widget.rect.height + mainLogo211Icon->widget.rect.height;
        if (theConfig.logo2_delay >= 0)
        {
            mainLogo24Animation->delay = theConfig.logo2_delay;
        }
        else
        {
            mainLogo24Animation->delay = 0;
            mainLogo24Animation->totalframe /= -theConfig.logo2_delay;
        }

        mainWeb1Sprite = ituSceneFindWidget(&theScene, "mainWeb1Sprite");
        assert(mainWeb1Sprite);
        ituSpriteGoto(mainWeb1Sprite, theConfig.web1_format);

        mainWeb11Animation = ituSceneFindWidget(&theScene, "mainWeb11Animation");
        assert(mainWeb11Animation);

        mainWeb111Text = ituSceneFindWidget(&theScene, "mainWeb111Text");
        assert(mainWeb111Text);
        mainWeb111Text->widget.rect.x = -mainWeb111Text->widget.rect.width;
        mainWeb11Animation->totalframe = mainWeb11Animation->widget.rect.width + mainWeb111Text->widget.rect.width;
        if (theConfig.web1_delay >= 0)
        {
            mainWeb11Animation->delay = theConfig.web1_delay;
        }
        else
        {
            mainWeb11Animation->delay = 0;
            mainWeb11Animation->totalframe /= -theConfig.web1_delay;
        }

        mainWeb12Animation = ituSceneFindWidget(&theScene, "mainWeb12Animation");
        assert(mainWeb12Animation);
        mainWeb12Animation->totalframe = mainWeb12Animation->widget.rect.width + mainWeb111Text->widget.rect.width;
        if (theConfig.web1_delay >= 0)
        {
            mainWeb12Animation->delay = theConfig.web1_delay;
        }
        else
        {
            mainWeb12Animation->delay = 0;
            mainWeb12Animation->totalframe /= -theConfig.web1_delay;
        }

        mainWeb122Text = ituSceneFindWidget(&theScene, "mainWeb122Text");
        assert(mainWeb122Text);
        mainWeb122Text->widget.rect.x = -mainWeb122Text->widget.rect.width;

        mainWeb13Animation = ituSceneFindWidget(&theScene, "mainWeb13Animation");
        assert(mainWeb13Animation);
        mainWeb13Animation->totalframe = mainWeb13Animation->widget.rect.height + mainWeb111Text->widget.rect.height;
        if (theConfig.web1_delay >= 0)
        {
            mainWeb13Animation->delay = theConfig.web1_delay;
        }
        else
        {
            mainWeb13Animation->delay = 0;
            mainWeb13Animation->totalframe /= -theConfig.web1_delay;
        }

        mainWeb14Animation = ituSceneFindWidget(&theScene, "mainWeb14Animation");
        assert(mainWeb14Animation);
        mainWeb14Animation->totalframe = mainWeb14Animation->widget.rect.height + mainWeb111Text->widget.rect.height;
        if (theConfig.web1_delay >= 0)
        {
            mainWeb14Animation->delay = theConfig.web1_delay;
        }
        else
        {
            mainWeb14Animation->delay = 0;
            mainWeb14Animation->totalframe /= -theConfig.web1_delay;
        }

        mainWeb2Sprite = ituSceneFindWidget(&theScene, "mainWeb2Sprite");
        assert(mainWeb2Sprite);
        ituSpriteGoto(mainWeb2Sprite, theConfig.web2_format);

        mainWeb21Animation = ituSceneFindWidget(&theScene, "mainWeb21Animation");
        assert(mainWeb21Animation);

        mainWeb211Text = ituSceneFindWidget(&theScene, "mainWeb211Text");
        assert(mainWeb211Text);
        mainWeb211Text->widget.rect.x = -mainWeb211Text->widget.rect.width;
        mainWeb21Animation->totalframe = mainWeb21Animation->widget.rect.width + mainWeb211Text->widget.rect.width;
        if (theConfig.web2_delay >= 0)
        {
            mainWeb21Animation->delay = theConfig.web2_delay;
        }
        else
        {
            mainWeb21Animation->delay = 0;
            mainWeb21Animation->totalframe /= -theConfig.web2_delay;
        }

        mainWeb22Animation = ituSceneFindWidget(&theScene, "mainWeb22Animation");
        assert(mainWeb22Animation);
        mainWeb22Animation->totalframe = mainWeb22Animation->widget.rect.width + mainWeb211Text->widget.rect.width;
        if (theConfig.web2_delay >= 0)
        {
            mainWeb22Animation->delay = theConfig.web2_delay;
        }
        else
        {
            mainWeb22Animation->delay = 0;
            mainWeb22Animation->totalframe /= -theConfig.web2_delay;
        }

        mainWeb222Text = ituSceneFindWidget(&theScene, "mainWeb222Text");
        assert(mainWeb222Text);
        mainWeb222Text->widget.rect.x = -mainWeb222Text->widget.rect.width;

        mainWeb23Animation = ituSceneFindWidget(&theScene, "mainWeb23Animation");
        assert(mainWeb23Animation);
        mainWeb23Animation->totalframe = mainWeb23Animation->widget.rect.height + mainWeb211Text->widget.rect.height;
        if (theConfig.web2_delay >= 0)
        {
            mainWeb23Animation->delay = theConfig.web2_delay;
        }
        else
        {
            mainWeb23Animation->delay = 0;
            mainWeb23Animation->totalframe /= -theConfig.web2_delay;
        }

        mainWeb24Animation = ituSceneFindWidget(&theScene, "mainWeb24Animation");
        assert(mainWeb24Animation);
        mainWeb24Animation->totalframe = mainWeb24Animation->widget.rect.height + mainWeb211Text->widget.rect.height;
        if (theConfig.web2_delay >= 0)
        {
            mainWeb24Animation->delay = theConfig.web2_delay;
        }
        else
        {
            mainWeb24Animation->delay = 0;
            mainWeb24Animation->totalframe /= -theConfig.web2_delay;
        }

        mainWeb3Sprite = ituSceneFindWidget(&theScene, "mainWeb3Sprite");
        assert(mainWeb3Sprite);
        ituSpriteGoto(mainWeb3Sprite, theConfig.web3_format);

        mainWeb31Animation = ituSceneFindWidget(&theScene, "mainWeb31Animation");
        assert(mainWeb31Animation);

        mainWeb311Text = ituSceneFindWidget(&theScene, "mainWeb311Text");
        assert(mainWeb311Text);
        mainWeb311Text->widget.rect.x = -mainWeb311Text->widget.rect.width;
        mainWeb31Animation->totalframe = mainWeb31Animation->widget.rect.width + mainWeb311Text->widget.rect.width;
        if (theConfig.web3_delay >= 0)
        {
            mainWeb31Animation->delay = theConfig.web3_delay;
        }
        else
        {
            mainWeb31Animation->delay = 0;
            mainWeb31Animation->totalframe /= -theConfig.web3_delay;
        }

        mainWeb32Animation = ituSceneFindWidget(&theScene, "mainWeb32Animation");
        assert(mainWeb32Animation);
        mainWeb32Animation->totalframe = mainWeb32Animation->widget.rect.width + mainWeb311Text->widget.rect.width;
        if (theConfig.web3_delay >= 0)
        {
            mainWeb32Animation->delay = theConfig.web3_delay;
        }
        else
        {
            mainWeb32Animation->delay = 0;
            mainWeb32Animation->totalframe /= -theConfig.web3_delay;
        }

        mainWeb322Text = ituSceneFindWidget(&theScene, "mainWeb322Text");
        assert(mainWeb322Text);
        mainWeb322Text->widget.rect.x = -mainWeb322Text->widget.rect.width;

        mainWeb33Animation = ituSceneFindWidget(&theScene, "mainWeb33Animation");
        assert(mainWeb33Animation);
        mainWeb33Animation->totalframe = mainWeb33Animation->widget.rect.height + mainWeb311Text->widget.rect.height;
        if (theConfig.web3_delay >= 0)
        {
            mainWeb33Animation->delay = theConfig.web3_delay;
        }
        else
        {
            mainWeb33Animation->delay = 0;
            mainWeb33Animation->totalframe /= -theConfig.web3_delay;
        }

        mainWeb34Animation = ituSceneFindWidget(&theScene, "mainWeb34Animation");
        assert(mainWeb34Animation);
        mainWeb34Animation->totalframe = mainWeb34Animation->widget.rect.height + mainWeb311Text->widget.rect.height;
        if (theConfig.web3_delay >= 0)
        {
            mainWeb34Animation->delay = theConfig.web3_delay;
        }
        else
        {
            mainWeb34Animation->delay = 0;
            mainWeb34Animation->totalframe /= -theConfig.web3_delay;
        }

        mainPhoto1Slideshow = ituSceneFindWidget(&theScene, "mainPhoto1Slideshow");
        assert(mainPhoto1Slideshow);
        ituSlideshowSetOnStop(mainPhoto1Slideshow, MainSlideshowOnStop);
        if (ituWidgetIsVisible(mainPhoto1Slideshow))
        {
            mainPhoto1Slideshow->delayCount = mainPhoto1Slideshow->delay = theConfig.photo1_interval * 1000 / MS_PER_FRAME;

            node2 = NULL;
            for (node1 = ((ITCTree*)mainPhoto1Slideshow)->child; node1; node1 = node1->sibling)
            {
                ITUIcon* child = (ITUIcon*)node1;

                if (child->surf == NULL)
                {
                    if (node2)
                        node2->sibling = NULL;

                    break;
                }
                node2 = node1;
            }
        }

        mainPhoto2Slideshow = ituSceneFindWidget(&theScene, "mainPhoto2Slideshow");
        assert(mainPhoto2Slideshow);
        //if (ituWidgetIsVisible(mainPhoto2Slideshow))
        {
            mainPhoto2Slideshow->delayCount = mainPhoto2Slideshow->delay = theConfig.photo2_interval * 1000 / MS_PER_FRAME;

            node2 = NULL;
            for (node1 = ((ITCTree*)mainPhoto2Slideshow)->child; node1; node1 = node1->sibling)
            {
                ITUIcon* child = (ITUIcon*)node1;

                if (child->surf == NULL)
                {
                    if (node2)
                        node2->sibling = NULL;

                    break;
                }
                node2 = node1;
            }
        }

        mainVideo = ituSceneFindWidget(&theScene, "mainVideo");
        assert(mainVideo);
        ituVideoSetOnStop(mainVideo, MainVideoOnStop);

        mainVideoMediaFileListBox = ituSceneFindWidget(&theScene, "mainVideoMediaFileListBox");
        assert(mainVideoMediaFileListBox);
        mainVideoMediaFileListBox->repeatMode = ITU_MEDIA_REPEAT_ALL;

        mainMusicMediaFileListBox = ituSceneFindWidget(&theScene, "mainMusicMediaFileListBox");
        assert(mainMusicMediaFileListBox);
        mainMusicMediaFileListBox->repeatMode = ITU_MEDIA_REPEAT_ALL;

		mainInfo1Background = ituSceneFindWidget(&theScene, "mainInfo1Background");
		assert(mainInfo1Background);

        mainInfo1Sprite = ituSceneFindWidget(&theScene, "mainInfo1Sprite");
        assert(mainInfo1Sprite);

        mainInfo11Animation = ituSceneFindWidget(&theScene, "mainInfo11Animation");
        assert(mainInfo11Animation);
        ituAnimationSetOnStop(mainInfo11Animation, MainInfo1AnimationOnStop);

        mainInfo111Text = ituSceneFindWidget(&theScene, "mainInfo111Text");
        assert(mainInfo111Text);
        mainInfo111Text->widget.rect.x = -mainInfo111Text->widget.rect.width;
        mainInfo11Animation->totalframe = mainInfo11Animation->widget.rect.width + mainInfo111Text->widget.rect.width;
        if (theConfig.info1_delay >= 0)
        {
            mainInfo11Animation->delay = theConfig.info1_delay;
        }
        else
        {
            mainInfo11Animation->delay = 0;
            mainInfo11Animation->totalframe /= -theConfig.info1_delay;
        }

        mainInfo12Animation = ituSceneFindWidget(&theScene, "mainInfo12Animation");
        assert(mainInfo12Animation);
        ituAnimationSetOnStop(mainInfo12Animation, MainInfo1AnimationOnStop);
        mainInfo12Animation->totalframe = mainInfo12Animation->widget.rect.width + mainInfo111Text->widget.rect.width;
        if (theConfig.info1_delay >= 0)
        {
            mainInfo12Animation->delay = theConfig.info1_delay;
        }
        else
        {
            mainInfo12Animation->delay = 0;
            mainInfo12Animation->totalframe /= -theConfig.info1_delay;
        }

        mainInfo122Text = ituSceneFindWidget(&theScene, "mainInfo122Text");
        assert(mainInfo122Text);
        mainInfo122Text->widget.rect.x = -mainInfo122Text->widget.rect.width;

        mainInfo13Animation = ituSceneFindWidget(&theScene, "mainInfo13Animation");
        assert(mainInfo13Animation);
        ituAnimationSetOnStop(mainInfo13Animation, MainInfo1AnimationOnStop);
        mainInfo13Animation->totalframe = mainInfo13Animation->widget.rect.height + mainInfo111Text->widget.rect.height;
        if (theConfig.info1_delay >= 0)
        {
            mainInfo13Animation->delay = theConfig.info1_delay;
        }
        else
        {
            mainInfo13Animation->delay = 0;
            mainInfo13Animation->totalframe /= -theConfig.info1_delay;
        }

        mainInfo14Animation = ituSceneFindWidget(&theScene, "mainInfo14Animation");
        assert(mainInfo14Animation);
        ituAnimationSetOnStop(mainInfo14Animation, MainInfo1AnimationOnStop);
        mainInfo14Animation->totalframe = mainInfo14Animation->widget.rect.height + mainInfo111Text->widget.rect.height;
        if (theConfig.info1_delay >= 0)
        {
            mainInfo14Animation->delay = theConfig.info1_delay;
        }
        else
        {
            mainInfo14Animation->delay = 0;
            mainInfo14Animation->totalframe /= -theConfig.info1_delay;
        }

		mainInfo15Text = ituSceneFindWidget(&theScene, "mainInfo15Text");
		assert(mainInfo15Text);

        mainInfo1TextMediaFileListBox = ituSceneFindWidget(&theScene, "mainInfo1TextMediaFileListBox");
        assert(mainInfo1TextMediaFileListBox);

        mainInfo1FontMediaFileListBox = ituSceneFindWidget(&theScene, "mainInfo1FontMediaFileListBox");
        assert(mainInfo1FontMediaFileListBox);

		mainInfo2Background = ituSceneFindWidget(&theScene, "mainInfo2Background");
		assert(mainInfo2Background);

        mainInfo2Sprite = ituSceneFindWidget(&theScene, "mainInfo2Sprite");
        assert(mainInfo2Sprite);

        mainInfo21Animation = ituSceneFindWidget(&theScene, "mainInfo21Animation");
        assert(mainInfo21Animation);
        ituAnimationSetOnStop(mainInfo21Animation, MainInfo2AnimationOnStop);

        mainInfo211Text = ituSceneFindWidget(&theScene, "mainInfo211Text");
        assert(mainInfo211Text);
        mainInfo211Text->widget.rect.x = -mainInfo211Text->widget.rect.width;
        mainInfo21Animation->totalframe = mainInfo21Animation->widget.rect.width + mainInfo211Text->widget.rect.width;
        if (theConfig.info2_delay >= 0)
        {
            mainInfo21Animation->delay = theConfig.info2_delay;
        }
        else
        {
            mainInfo21Animation->delay = 0;
            mainInfo21Animation->totalframe /= -theConfig.info2_delay;
        }

        mainInfo22Animation = ituSceneFindWidget(&theScene, "mainInfo22Animation");
        assert(mainInfo22Animation);
        ituAnimationSetOnStop(mainInfo22Animation, MainInfo2AnimationOnStop);
        mainInfo22Animation->totalframe = mainInfo22Animation->widget.rect.width + mainInfo211Text->widget.rect.width;
        if (theConfig.info2_delay >= 0)
        {
            mainInfo22Animation->delay = theConfig.info2_delay;
        }
        else
        {
            mainInfo22Animation->delay = 0;
            mainInfo22Animation->totalframe /= -theConfig.info2_delay;
        }

        mainInfo222Text = ituSceneFindWidget(&theScene, "mainInfo222Text");
        assert(mainInfo222Text);
        mainInfo222Text->widget.rect.x = -mainInfo222Text->widget.rect.width;

        mainInfo23Animation = ituSceneFindWidget(&theScene, "mainInfo23Animation");
        assert(mainInfo23Animation);
        ituAnimationSetOnStop(mainInfo23Animation, MainInfo2AnimationOnStop);
        mainInfo23Animation->totalframe = mainInfo23Animation->widget.rect.height + mainInfo211Text->widget.rect.height;
        if (theConfig.info2_delay >= 0)
        {
            mainInfo23Animation->delay = theConfig.info2_delay;
        }
        else
        {
            mainInfo23Animation->delay = 0;
            mainInfo23Animation->totalframe /= -theConfig.info2_delay;
        }

        mainInfo24Animation = ituSceneFindWidget(&theScene, "mainInfo24Animation");
        assert(mainInfo24Animation);
        ituAnimationSetOnStop(mainInfo24Animation, MainInfo2AnimationOnStop);
        mainInfo24Animation->totalframe = mainInfo24Animation->widget.rect.height + mainInfo211Text->widget.rect.height;
        if (theConfig.info2_delay >= 0)
        {
            mainInfo24Animation->delay = theConfig.info2_delay;
        }
        else
        {
            mainInfo24Animation->delay = 0;
            mainInfo24Animation->totalframe /= -theConfig.info2_delay;
        }

        mainInfo2TextMediaFileListBox = ituSceneFindWidget(&theScene, "mainInfo2TextMediaFileListBox");
        assert(mainInfo2TextMediaFileListBox);

        mainInfo2FontMediaFileListBox = ituSceneFindWidget(&theScene, "mainInfo2FontMediaFileListBox");
        assert(mainInfo2FontMediaFileListBox);

		warnBackground = ituSceneFindWidget(&theScene, "warnBackground");
		assert(warnBackground);
		ituWidgetSetVisible(warnBackground, false);

		warn1Sprite = ituSceneFindWidget(&theScene, "warn1Sprite");
		assert(warn1Sprite);

    #ifdef CFG_NET_WIFI
        mainWiFiSsidText = ituSceneFindWidget(&theScene, "mainWiFiSsidText");
        assert(mainWiFiSsidText);
        wifiSsid = ituTextGetString(mainWiFiSsidText);

        mainWiFiPasswordText = ituSceneFindWidget(&theScene, "mainWiFiPasswordText");
        assert(mainWiFiPasswordText);
        wifiPassword = ituTextGetString(mainWiFiPasswordText);

        if (wifiSsid && wifiPassword && strcmp(wifiSsid, theConfig.ssid) && strcmp(wifiPassword, theConfig.password))
        {
            strcpy(theConfig.ssid, wifiSsid);
            strcpy(theConfig.password, wifiPassword);

            ConfigSave();
            SceneQuit(QUIT_DEFAULT);
        }
    #endif // CFG_NET_WIFI

		if (mainArrow1UpSprite)
			ituWidgetSetVisible(mainArrow1UpSprite, false);

		if (mainArrow1DownSprite)
			ituWidgetSetVisible(mainArrow1DownSprite, false);

		if (mainArrow1UpAnimation)
			ituWidgetSetVisible(mainArrow1UpAnimation, false);

		if (mainArrow1DownAnimation)
			ituWidgetSetVisible(mainArrow1DownAnimation, false);

        ituWidgetSetVisible(mainFloor11Sprite, false);
        ituWidgetSetVisible(mainFloor12Sprite, false);
        ituWidgetSetVisible(mainFloor13Sprite, false);

		if (mainArrow2UpSprite)
			ituWidgetSetVisible(mainArrow2UpSprite, false);

		if (mainArrow2DownSprite)
			ituWidgetSetVisible(mainArrow2DownSprite, false);

		if (mainArrow2UpAnimation)
			ituWidgetSetVisible(mainArrow2UpAnimation, false);

		if (mainArrow2DownAnimation)
			ituWidgetSetVisible(mainArrow2DownAnimation, false);

        ituWidgetSetVisible(mainFloor21Sprite, false);
        ituWidgetSetVisible(mainFloor22Sprite, false);
        ituWidgetSetVisible(mainFloor23Sprite, false);
        ituWidgetSetVisible(mainTemperatureText, false);
        ituAnimationPlay(mainLogo11Animation, 0);
        ituAnimationPlay(mainLogo12Animation, 0);
        ituAnimationPlay(mainLogo13Animation, 0);
        ituAnimationPlay(mainLogo14Animation, 0);
        ituAnimationPlay(mainLogo21Animation, 0);
        ituAnimationPlay(mainLogo22Animation, 0);
        ituAnimationPlay(mainLogo23Animation, 0);
        ituAnimationPlay(mainLogo24Animation, 0);
        ituAnimationPlay(mainWeb11Animation, 0);
        ituAnimationPlay(mainWeb12Animation, 0);
        ituAnimationPlay(mainWeb13Animation, 0);
        ituAnimationPlay(mainWeb14Animation, 0);
        ituAnimationPlay(mainWeb21Animation, 0);
        ituAnimationPlay(mainWeb22Animation, 0);
        ituAnimationPlay(mainWeb23Animation, 0);
        ituAnimationPlay(mainWeb24Animation, 0);
        ituAnimationPlay(mainWeb31Animation, 0);
        ituAnimationPlay(mainWeb32Animation, 0);
        ituAnimationPlay(mainWeb33Animation, 0);
        ituAnimationPlay(mainWeb34Animation, 0);

        playState = PLAY_NONE;
		targetFloor = nextTargetFloor = INT_MAX;
        activeMainInfo1Animation = activeMainInfo2Animation = NULL;
        mainInfo1File = mainInfo2File = NULL;
        mainInfo1FontLoaded = mainInfo2FontLoaded = false;

        mainPhoto1SlideshowEnabled = ituWidgetIsVisible(mainPhoto1Slideshow);
        mainVideoEnabled = ituWidgetIsVisible(mainVideo);

        mainLayoutIndex = 0;
        mainLayerWidth = ituWidgetGetWidth(mainLayer);
        mainLayerHeight = ituWidgetGetHeight(mainLayer);

        if (theConfig.demo_enable)
        {
            ExternalEvent ev;

			mainLayoutIndex = -1;
			MainChangeLayout();

			ev.type = EXTERNAL_FLOOR1;
			ev.arg1 = 'B';
			ev.arg2 = '3';
			ev.arg3 = EXTERNAL_FLOOR_NONE;
			MainProcessExternalEvent(&ev);

            ev.type = EXTERNAL_TEMPERATURE;
            ev.arg1 = 17;
            MainProcessExternalEvent(&ev);
        }

        SceneSetReady(true);
    }

    MainUpdateByConfig();

    return true;
}

bool MainOnLeave(ITUWidget* widget, char* param)
{
    if (mainInfo1File)
    {
        fclose(mainInfo1File);
        mainInfo1File = NULL;
    }
    activeMainInfo1Animation = NULL;

    if (mainInfo2File)
    {
        fclose(mainInfo2File);
        mainInfo2File = NULL;
    }
    activeMainInfo2Animation = NULL;

    return true;
}

void MainReset(void)
{
    if (mainInfo1File)
    {
        fclose(mainInfo1File);
        mainInfo1File = NULL;
    }
    activeMainInfo1Animation = NULL;

    if (mainInfo2File)
    {
        fclose(mainInfo2File);
        mainInfo2File = NULL;
    }
    activeMainInfo2Animation = NULL;

    mainLayer = NULL;
    SceneSetReady(false);
}
