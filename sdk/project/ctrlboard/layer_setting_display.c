#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUTrackBar* settingDisplayBrightnessTrackBar;
static ITUProgressBar* settingDisplayBrightnessProgressBar;
static ITUSprite* settingDisplayScreensaverTimeSprite;
static ITUSprite* settingDisplayScreensaverTypeSprite;
static ITUSprite* settingDisplayMainMenuTypeSprite;
static ITUBackground* settingDisplayGrayBackground;
static ITURadioBox* settingDisplayScreensaverTime1MinRadioBox;
static ITURadioBox* settingDisplayScreensaverTime2MinRadioBox;
static ITURadioBox* settingDisplayScreensaverTime5MinRadioBox;
static ITURadioBox* settingDisplayScreensaverTime10MinRadioBox;
static ITURadioBox* settingDisplayScreensaverTypeClockRadioBox;
static ITURadioBox* settingDisplayScreensaverTypeBlankRadioBox;
static ITURadioBox* settingDisplayMainMenuNoneRadioBox;
static ITURadioBox* settingDisplayMainMenuReflectionRadioBox;
static ITURadioBox* settingDisplayMainMenuTurnPageRadioBox;
static ITURadioBox* settingDisplayMainMenuTurnPage2RadioBox;
static ITURadioBox* settingDisplayMainMenuFoldingRadioBox;
static ITURadioBox* settingDisplayMainMenuRippleRadioBox;

// status
static int settingDisplayScreenMaxBrightness;
static int settingDisplayBrightnessOld;
static int settingDisplayScreensaverTimeOld;
static ScreensaverType settingDisplayScreensaverTypeOld;
static MainMenuType settingDisplayMainMenuTypeOld;

bool SettingDisplayBrightnessTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = atoi(param);

    if (value < 0)
        value = 0;
    else if (value > settingDisplayScreenMaxBrightness)
        value = settingDisplayScreenMaxBrightness;

    theConfig.brightness = value;
    ScreenSetBrightness(theConfig.brightness);

    return true;
}

bool SettingDisplayScreensaverTimeRadioBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituRadioBoxIsChecked(settingDisplayScreensaverTime1MinRadioBox))
    {
        theConfig.screensaver_time = 1;
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 0);
    }
    else if (ituRadioBoxIsChecked(settingDisplayScreensaverTime2MinRadioBox))
    {
        theConfig.screensaver_time = 2;
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 1);
    }
    else if (ituRadioBoxIsChecked(settingDisplayScreensaverTime5MinRadioBox))
    {
        theConfig.screensaver_time = 5;
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 2);
    }
    else if (ituRadioBoxIsChecked(settingDisplayScreensaverTime10MinRadioBox))
    {
        theConfig.screensaver_time = 10;
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 3);
    }
    return true;
}

bool SettingDisplayScreensaverTypeRadioBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituRadioBoxIsChecked(settingDisplayScreensaverTypeClockRadioBox))
    {
        theConfig.screensaver_type = SCREENSAVER_CLOCK;
        ituSpriteGoto(settingDisplayScreensaverTypeSprite, 0);
    }
    else if (ituRadioBoxIsChecked(settingDisplayScreensaverTypeBlankRadioBox))
    {
        theConfig.screensaver_type = SCREENSAVER_BLANK;
        ituSpriteGoto(settingDisplayScreensaverTypeSprite, 1);
    }
    return true;
}

bool SettingDisplayMainMenuTypeRadioBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituRadioBoxIsChecked(settingDisplayMainMenuNoneRadioBox))
    {
        theConfig.mainmenu_type = MAINMENU_COVERFLOW;
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 0);
    }
    else if (ituRadioBoxIsChecked(settingDisplayMainMenuReflectionRadioBox))
    {
        theConfig.mainmenu_type = MAINMENU_COVERFLOW_REFLECTION;
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 1);
    }
    else if (ituRadioBoxIsChecked(settingDisplayMainMenuTurnPageRadioBox))
    {
        theConfig.mainmenu_type = MAINMENU_PAGEFLOW_FLIP;
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 2);
    }
    else if (ituRadioBoxIsChecked(settingDisplayMainMenuTurnPage2RadioBox))
    {
        theConfig.mainmenu_type = MAINMENU_PAGEFLOW_FLIP2;
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 3);
    }
    else if (ituRadioBoxIsChecked(settingDisplayMainMenuFoldingRadioBox))
    {
        theConfig.mainmenu_type = MAINMENU_PAGEFLOW_FOLD;
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 4);
    }
    else if (ituRadioBoxIsChecked(settingDisplayMainMenuRippleRadioBox))
    {
        theConfig.mainmenu_type = MAINMENU_COVERFLOW_RIPPLE;
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 5);
    }
    return true;
}

bool SettingDisplayOnEnter(ITUWidget* widget, char* param)
{
    if (!settingDisplayBrightnessTrackBar)
    {
        settingDisplayBrightnessTrackBar = ituSceneFindWidget(&theScene, "settingDisplayBrightnessTrackBar");
        assert(settingDisplayBrightnessTrackBar);

        settingDisplayBrightnessProgressBar = ituSceneFindWidget(&theScene, "settingDisplayBrightnessProgressBar");
        assert(settingDisplayBrightnessProgressBar);

        settingDisplayScreensaverTimeSprite = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTimeSprite");
        assert(settingDisplayScreensaverTimeSprite);

        settingDisplayScreensaverTypeSprite = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTypeSprite");
        assert(settingDisplayScreensaverTypeSprite);

        settingDisplayMainMenuTypeSprite = ituSceneFindWidget(&theScene, "settingDisplayMainMenuTypeSprite");
        assert(settingDisplayMainMenuTypeSprite);

        settingDisplayGrayBackground = ituSceneFindWidget(&theScene, "settingDisplayGrayBackground");
        assert(settingDisplayGrayBackground);

        settingDisplayScreensaverTime1MinRadioBox = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTime1MinRadioBox");
        assert(settingDisplayScreensaverTime1MinRadioBox);

        settingDisplayScreensaverTime2MinRadioBox = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTime2MinRadioBox");
        assert(settingDisplayScreensaverTime2MinRadioBox);

        settingDisplayScreensaverTime5MinRadioBox = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTime5MinRadioBox");
        assert(settingDisplayScreensaverTime5MinRadioBox);

        settingDisplayScreensaverTime10MinRadioBox = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTime10MinRadioBox");
        assert(settingDisplayScreensaverTime10MinRadioBox);

        settingDisplayScreensaverTypeClockRadioBox = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTypeClockRadioBox");
        assert(settingDisplayScreensaverTypeClockRadioBox);

        settingDisplayScreensaverTypeBlankRadioBox = ituSceneFindWidget(&theScene, "settingDisplayScreensaverTypeBlankRadioBox");
        assert(settingDisplayScreensaverTypeBlankRadioBox);

        settingDisplayMainMenuNoneRadioBox = ituSceneFindWidget(&theScene, "settingDisplayMainMenuNoneRadioBox");
        assert(settingDisplayMainMenuNoneRadioBox);

        settingDisplayMainMenuReflectionRadioBox = ituSceneFindWidget(&theScene, "settingDisplayMainMenuReflectionRadioBox");
        assert(settingDisplayMainMenuReflectionRadioBox);

        settingDisplayMainMenuTurnPageRadioBox = ituSceneFindWidget(&theScene, "settingDisplayMainMenuTurnPageRadioBox");
        assert(settingDisplayMainMenuTurnPageRadioBox);

        settingDisplayMainMenuTurnPage2RadioBox = ituSceneFindWidget(&theScene, "settingDisplayMainMenuTurnPage2RadioBox");
        assert(settingDisplayMainMenuTurnPage2RadioBox);

        settingDisplayMainMenuFoldingRadioBox = ituSceneFindWidget(&theScene, "settingDisplayMainMenuFoldingRadioBox");
        assert(settingDisplayMainMenuFoldingRadioBox);

        settingDisplayMainMenuRippleRadioBox = ituSceneFindWidget(&theScene, "settingDisplayMainMenuRippleRadioBox");
        assert(settingDisplayMainMenuRippleRadioBox);

        // status
        settingDisplayScreenMaxBrightness = ScreenGetMaxBrightness();
    }

    // current settings
    settingDisplayBrightnessOld = theConfig.brightness;
    ituTrackBarSetValue(settingDisplayBrightnessTrackBar, theConfig.brightness);
    ituProgressBarSetValue(settingDisplayBrightnessProgressBar, theConfig.brightness);

    switch (theConfig.screensaver_time)
    {
    case 1:
        ituRadioBoxSetChecked(settingDisplayScreensaverTime1MinRadioBox, true);
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 0);
        break;

    case 2:
        ituRadioBoxSetChecked(settingDisplayScreensaverTime2MinRadioBox, true);
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 1);
        break;

    case 5:
        ituRadioBoxSetChecked(settingDisplayScreensaverTime5MinRadioBox, true);
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 2);
        break;

    case 10:
        ituRadioBoxSetChecked(settingDisplayScreensaverTime10MinRadioBox, true);
        ituSpriteGoto(settingDisplayScreensaverTimeSprite, 3);
        break;
	}
    settingDisplayScreensaverTimeOld = theConfig.screensaver_time;

    switch (theConfig.screensaver_type)
    {
    case SCREENSAVER_CLOCK:
        ituRadioBoxSetChecked(settingDisplayScreensaverTypeClockRadioBox, true);
        ituSpriteGoto(settingDisplayScreensaverTypeSprite, 0);
        break;

    case SCREENSAVER_BLANK:
        ituRadioBoxSetChecked(settingDisplayScreensaverTypeBlankRadioBox, true);
        ituSpriteGoto(settingDisplayScreensaverTypeSprite, 1);
        break;
	}
    settingDisplayScreensaverTypeOld = theConfig.screensaver_type;

    switch (theConfig.mainmenu_type)
    {
    case MAINMENU_COVERFLOW:
        ituRadioBoxSetChecked(settingDisplayMainMenuNoneRadioBox, true);
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 0);
        break;

    case MAINMENU_COVERFLOW_REFLECTION:
        ituRadioBoxSetChecked(settingDisplayMainMenuReflectionRadioBox, true);
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 1);
        break;

    case MAINMENU_PAGEFLOW_FLIP:
        ituRadioBoxSetChecked(settingDisplayMainMenuTurnPageRadioBox, true);
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 2);
        break;

    case MAINMENU_PAGEFLOW_FLIP2:
        ituRadioBoxSetChecked(settingDisplayMainMenuTurnPage2RadioBox, true);
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 3);
        break;

    case MAINMENU_PAGEFLOW_FOLD:
        ituRadioBoxSetChecked(settingDisplayMainMenuFoldingRadioBox, true);
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 4);
        break;

    case MAINMENU_COVERFLOW_RIPPLE:
        ituRadioBoxSetChecked(settingDisplayMainMenuRippleRadioBox, true);
        ituSpriteGoto(settingDisplayMainMenuTypeSprite, 5);
        break;
	}
    settingDisplayMainMenuTypeOld = theConfig.mainmenu_type;

    return true;
}

bool SettingDisplayOnLeave(ITUWidget* widget, char* param)
{
    if (settingDisplayBrightnessOld != theConfig.brightness ||
        settingDisplayScreensaverTimeOld != theConfig.screensaver_time ||
        settingDisplayScreensaverTypeOld != theConfig.screensaver_type ||
        settingDisplayMainMenuTypeOld != theConfig.mainmenu_type)
    {
        ConfigSave();
    }
    return true;
}

void SettingDisplayReset(void)
{
    settingDisplayBrightnessTrackBar = NULL;
}
