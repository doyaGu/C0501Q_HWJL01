#include <sys/time.h>
#include <assert.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "scene.h"
#include "elevator.h"

extern bool MainOnTimer(ITUWidget* widget, char* param);

static ITURadioBox* settingLangRadioBox;
static ITURadioBox* settingDemoRadioBox;
static ITURadioBox* settingDateRadioBox;
static ITURadioBox* settingVoiceRadioBox;
static ITURadioBox* settingScreensaverRadioBox;
static ITURadioBox* settingMessageRadioBox;
static ITURadioBox* settingUpgradeRadioBox;
static ITUSprite* settingContentSprite;
static ITURadioBox* settingLangChtRadioBox;
static ITURadioBox* settingLangChsRadioBox;
static ITURadioBox* settingLangEngRadioBox;
static ITURadioBox* settingLangChtCheckRadioBox;
static ITURadioBox* settingLangChsCheckRadioBox;
static ITURadioBox* settingLangEngCheckRadioBox;
static ITURadioBox* settingDemoOnRadioBox;
static ITURadioBox* settingDemoOffRadioBox;
static ITURadioBox* settingDemoOnCheckRadioBox;
static ITURadioBox* settingDemoOffCheckRadioBox;
static ITUBackground* settingDateFormatBackground;
static ITURadioBox* settingDateFormat1RadioBox;
static ITURadioBox* settingDateFormat2RadioBox;
static ITURadioBox* settingDateFormat3RadioBox;
static ITURadioBox* settingDateFormat1CheckRadioBox;
static ITURadioBox* settingDateFormat2CheckRadioBox;
static ITURadioBox* settingDateFormat3CheckRadioBox;
static ITUBackground* settingDateDateBackground;
static ITURadioBox* settingDateYearRadioBox;
static ITURadioBox* settingDateMonthRadioBox;	
static ITURadioBox* settingDateDayRadioBox;
static ITUBackground* settingDateTimeBackground;	
static ITURadioBox* settingDateHourRadioBox;
static ITURadioBox* settingDateMinuteRadioBox;	
static ITURadioBox* settingDateSecondRadioBox;	
static ITURadioBox* settingVoiceOnRadioBox;
static ITURadioBox* settingVoiceOffRadioBox;
static ITURadioBox* settingVoiceOnCheckRadioBox;
static ITURadioBox* settingVoiceOffCheckRadioBox;
static ITURadioBox* settingScreensaver3MinRadioBox;
static ITURadioBox* settingScreensaver5MinRadioBox;
static ITURadioBox* settingScreensaver10MinRadioBox;
static ITURadioBox* settingScreensaverMiscRadioBox;
static ITURadioBox* settingScreensaver3MinCheckRadioBox;
static ITURadioBox* settingScreensaver5MinCheckRadioBox;
static ITURadioBox* settingScreensaver10MinCheckRadioBox;
static ITURadioBox* settingScreensaverMiscCheckRadioBox;
static ITURadioBox* settingMessage1RadioBox;
static ITURadioBox* settingMessage2RadioBox;
static ITURadioBox* settingMessage3RadioBox;
static ITURadioBox* settingMessage4RadioBox;
static ITURadioBox* settingMessage1CheckRadioBox;
static ITURadioBox* settingMessage2CheckRadioBox;
static ITURadioBox* settingMessage3CheckRadioBox;
static ITURadioBox* settingMessage4CheckRadioBox;
static ITURadioBox* settingUpgradeDataRadioBox;
static ITURadioBox* settingUpgradeFirmwareRadioBox;
static ITULayer* mainLayer;

typedef enum
{
    SETTING_LANG,
    SETTING_DEMO,
    SETTING_DATE,
    SETTING_VOICE,
    SETTING_SCREENSAVER,
    SETTING_MESSAGE,
    SETTING_UPGRADE
} SettingMenu;

typedef enum
{
    SETTING_EDIT_MENU,
    SETTING_EDIT_LANG,
    SETTING_EDIT_DEMO,
    SETTING_EDIT_DATE_FORMAT,
    SETTING_EDIT_DATE_DATE,
    SETTING_EDIT_DATE_DATE_YEAR,
    SETTING_EDIT_DATE_DATE_MONTH,
    SETTING_EDIT_DATE_DATE_DAY,
    SETTING_EDIT_DATE_TIME,
    SETTING_EDIT_DATE_TIME_HOUR,
    SETTING_EDIT_DATE_TIME_MINUTE,
    SETTING_EDIT_DATE_TIME_SECOND,
    SETTING_EDIT_VOICE,
    SETTING_EDIT_SCREENSAVER,
    SETTING_EDIT_SCREENSAVER_MISC,
    SETTING_EDIT_MESSAGE,
    SETTING_EDIT_UPGRADE
} SettingEditMode;

static SettingMenu settingMenu;
static SettingEditMode settingEditMode;
static int settingYear, settingMonth, settingDay, settingHour, settingMinute, settingSecond, settingScreensaverTime;
static bool settingModified;

static void SettingShowMenu(void)
{
    switch (settingMenu)
    {
    case SETTING_LANG:
        ituRadioBoxSetChecked(settingLangRadioBox, true);
        break;

    case SETTING_DEMO:
        ituRadioBoxSetChecked(settingDemoRadioBox, true);
        break;

    case SETTING_DATE:
        ituRadioBoxSetChecked(settingDateRadioBox, true);
        break;

    case SETTING_VOICE:
        ituRadioBoxSetChecked(settingVoiceRadioBox, true);
        break;

    case SETTING_SCREENSAVER:
        ituRadioBoxSetChecked(settingScreensaverRadioBox, true);
        break;

    case SETTING_MESSAGE:
        ituRadioBoxSetChecked(settingMessageRadioBox, true);
        break;

    case SETTING_UPGRADE:
        ituRadioBoxSetChecked(settingUpgradeRadioBox, true);
        break;
    }
    ituSpriteGoto(settingContentSprite, settingMenu);
}

static void SettingChangeLanguage(int arg)
{
    ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);
    ituSceneUpdate(&theScene, ITU_EVENT_LAYOUT, 0, 0, 0);
}

static void SettingSetDateTime(void)
{
    struct timeval tv;
    struct tm *tm, mytime;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    memcpy(&mytime, tm, sizeof (struct tm));

    mytime.tm_year = settingYear - 1900;
    mytime.tm_mon = settingMonth - 1;
    mytime.tm_mday = settingDay;
    mytime.tm_hour = settingHour;
    mytime.tm_min = settingMinute;
    mytime.tm_sec = settingSecond;

    tv.tv_sec = mktime(&mytime);
    tv.tv_usec = 0;

#ifndef _WIN32
    settimeofday(&tv, NULL);
#endif
}

static void SettingEnter(void)
{
    char buf[32];

    switch (settingEditMode)
    {
    case SETTING_EDIT_MENU:
        switch (settingMenu)
        {
        case SETTING_LANG:
            ituRadioBoxSetChecked(settingLangChtRadioBox, true);
            settingEditMode = SETTING_EDIT_LANG;
            break;

        case SETTING_DEMO:
            ituRadioBoxSetChecked(settingDemoOnRadioBox, true);
            settingEditMode = SETTING_EDIT_DEMO;
            break;

        case SETTING_DATE:
            ituRadioBoxSetChecked(settingDateFormat1RadioBox, true);
            settingEditMode = SETTING_EDIT_DATE_FORMAT;
            break;

        case SETTING_VOICE:
            ituRadioBoxSetChecked(settingVoiceOnRadioBox, true);
            settingEditMode = SETTING_EDIT_VOICE;
            break;

        case SETTING_SCREENSAVER:
            ituRadioBoxSetChecked(settingScreensaver3MinRadioBox, true);
            settingEditMode = SETTING_EDIT_SCREENSAVER;
            break;

        case SETTING_MESSAGE:
            ituRadioBoxSetChecked(settingMessage1RadioBox, true);
            settingEditMode = SETTING_EDIT_MESSAGE;
            break;

        case SETTING_UPGRADE:
            ituRadioBoxSetChecked(settingUpgradeDataRadioBox, true);
            settingEditMode = SETTING_EDIT_UPGRADE;
            break;
        }
        break;

    case SETTING_EDIT_LANG:
        if (ituRadioBoxIsChecked(settingLangChtRadioBox))
        {
            theConfig.lang = LANG_CHT;
            ituRadioBoxSetChecked(settingLangChtCheckRadioBox, true);
        }
        else if (ituRadioBoxIsChecked(settingLangChsRadioBox))
        {
            theConfig.lang = LANG_CHS;
            ituRadioBoxSetChecked(settingLangChsCheckRadioBox, true);
        }
        else
        {
            theConfig.lang = LANG_ENG;
            ituRadioBoxSetChecked(settingLangEngCheckRadioBox, true);
        }
        settingModified = true;
        ituSceneExecuteCommand(&theScene, 1, SettingChangeLanguage, 0);
        break;

    case SETTING_EDIT_DEMO:
        if (ituRadioBoxIsChecked(settingDemoOnRadioBox))
        {
            theConfig.demo_enable = true;
            ituRadioBoxSetChecked(settingDemoOnCheckRadioBox, true);
        }
        else
        {
            theConfig.demo_enable = false;
            ituRadioBoxSetChecked(settingDemoOffCheckRadioBox, true);
        }
        settingModified = true;
        break;

    case SETTING_EDIT_DATE_FORMAT:
        {
            if (ituRadioBoxIsChecked(settingDateFormat1RadioBox))
            {
                theConfig.date_format = 0;
                ituRadioBoxSetChecked(settingDateFormat1CheckRadioBox, true);
            }
            else if (ituRadioBoxIsChecked(settingDateFormat2RadioBox))
            {
                theConfig.date_format = 1;
                ituRadioBoxSetChecked(settingDateFormat2CheckRadioBox, true);
            }
            else
            {
                theConfig.date_format = 2;
                ituRadioBoxSetChecked(settingDateFormat3CheckRadioBox, true);
            }
            settingModified = true;
        }
        break;

    case SETTING_EDIT_DATE_DATE:
        if (ituRadioBoxIsChecked(settingDateYearRadioBox))
        {
            settingDateYearRadioBox->checkbox.btn.bwin.widget.color.alpha = 255;
            settingEditMode = SETTING_EDIT_DATE_DATE_YEAR;
        }
        else if (ituRadioBoxIsChecked(settingDateMonthRadioBox))
        {
            settingDateMonthRadioBox->checkbox.btn.bwin.widget.color.alpha = 255;
            settingEditMode = SETTING_EDIT_DATE_DATE_MONTH;
        }
        else if (ituRadioBoxIsChecked(settingDateDayRadioBox))
        {
            settingDateDayRadioBox->checkbox.btn.bwin.widget.color.alpha = 255;
            settingEditMode = SETTING_EDIT_DATE_DATE_DAY;
        }
        break;

    case SETTING_EDIT_DATE_DATE_YEAR:
    case SETTING_EDIT_DATE_DATE_MONTH:
    case SETTING_EDIT_DATE_DATE_DAY:
        {
            SettingSetDateTime();
            settingDateYearRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
            settingDateMonthRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
            settingDateDayRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
            settingEditMode = SETTING_EDIT_DATE_DATE;
        }
        break;

    case SETTING_EDIT_DATE_TIME:
        if (ituRadioBoxIsChecked(settingDateHourRadioBox))
        {
            settingDateHourRadioBox->checkbox.btn.bwin.widget.color.alpha = 255;
            settingEditMode = SETTING_EDIT_DATE_TIME_HOUR;
        }
        else if (ituRadioBoxIsChecked(settingDateMinuteRadioBox))
        {
            settingDateMinuteRadioBox->checkbox.btn.bwin.widget.color.alpha = 255;
            settingEditMode = SETTING_EDIT_DATE_TIME_MINUTE;
        }
        else
        {
            settingDateSecondRadioBox->checkbox.btn.bwin.widget.color.alpha = 255;
            settingEditMode = SETTING_EDIT_DATE_TIME_SECOND;
        }
        break;

    case SETTING_EDIT_DATE_TIME_HOUR:
    case SETTING_EDIT_DATE_TIME_MINUTE:
    case SETTING_EDIT_DATE_TIME_SECOND:
        {
            SettingSetDateTime();
            settingDateHourRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
            settingDateMinuteRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
            settingDateSecondRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
            settingEditMode = SETTING_EDIT_DATE_TIME;
        }
        break;

    case SETTING_EDIT_VOICE:
        if (ituRadioBoxIsChecked(settingVoiceOnRadioBox))
        {
            theConfig.sound_enable = true;
            ituRadioBoxSetChecked(settingVoiceOnCheckRadioBox, true);
        }
        else
        {
            theConfig.sound_enable = false;
            ituRadioBoxSetChecked(settingVoiceOffCheckRadioBox, true);
        }
        settingModified = true;
        break;

    case SETTING_EDIT_SCREENSAVER:
        if (ituRadioBoxIsChecked(settingScreensaver3MinRadioBox))
        {
            theConfig.screensaver_time = 3;
            ituRadioBoxSetChecked(settingScreensaver3MinCheckRadioBox, true);
            settingModified = true;
        }
        else if (ituRadioBoxIsChecked(settingScreensaver5MinRadioBox))
        {
            theConfig.screensaver_time = 5;
            ituRadioBoxSetChecked(settingScreensaver5MinCheckRadioBox, true);
            settingModified = true;
        }
        else if (ituRadioBoxIsChecked(settingScreensaver10MinRadioBox))
        {
            theConfig.screensaver_time = 10;
            ituRadioBoxSetChecked(settingScreensaver10MinCheckRadioBox, true);
            settingModified = true;
        }
        else
        {
            settingScreensaverMiscRadioBox->checkbox.btn.bwin.widget.color.alpha = 255;
            settingEditMode = SETTING_EDIT_SCREENSAVER_MISC;
        }
        settingScreensaverTime = theConfig.screensaver_time;
        sprintf(buf, "%02i", settingScreensaverTime);
        ituButtonSetString(settingScreensaverMiscRadioBox, buf);
        break;

    case SETTING_EDIT_SCREENSAVER_MISC:
        settingScreensaverMiscRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        ituRadioBoxSetChecked(settingScreensaverMiscCheckRadioBox, true);
        theConfig.screensaver_time = settingScreensaverTime;
        settingModified = true;
        settingEditMode = SETTING_EDIT_SCREENSAVER;
        break;

    case SETTING_EDIT_MESSAGE:
        if (ituRadioBoxIsChecked(settingMessage1RadioBox))
        {
            theConfig.info1_format = 0;
            ituRadioBoxSetChecked(settingMessage1CheckRadioBox, true);
        }
        else if (ituRadioBoxIsChecked(settingMessage2RadioBox))
        {
            theConfig.info1_format = 1;
            ituRadioBoxSetChecked(settingMessage2CheckRadioBox, true);
        }
        else if (ituRadioBoxIsChecked(settingMessage3RadioBox))
        {
            theConfig.info1_format = 2;
            ituRadioBoxSetChecked(settingMessage3CheckRadioBox, true);
        }
        else
        {
            theConfig.info1_format = 3;
            ituRadioBoxSetChecked(settingMessage4CheckRadioBox, true);
        }
        settingModified = true;
        break;

    case SETTING_EDIT_UPGRADE:
        if (ituRadioBoxIsChecked(settingUpgradeDataRadioBox))
        {
            SceneQuit(QUIT_UPGRADE_NET_RES);
        }
        else
        {
            SceneQuit(QUIT_UPGRADE_NET_FW);
        }
        break;
    }
}

static void SettingLeave(void)
{
    char buf[32];

    switch (settingEditMode)
    {
    case SETTING_EDIT_MENU:
        ituLayerGoto(mainLayer);
        break;

    case SETTING_EDIT_LANG:
        ituRadioBoxSetChecked(settingLangChtRadioBox, false);
        ituRadioBoxSetChecked(settingLangChsRadioBox, false);
        ituRadioBoxSetChecked(settingLangEngRadioBox, false);
        settingEditMode = SETTING_EDIT_MENU;
        break;

    case SETTING_EDIT_DEMO:
        ituRadioBoxSetChecked(settingDemoOnRadioBox, false);
        ituRadioBoxSetChecked(settingDemoOffRadioBox, false);
        settingEditMode = SETTING_EDIT_MENU;
        break;

    case SETTING_EDIT_VOICE:
        ituRadioBoxSetChecked(settingVoiceOnRadioBox, false);
        ituRadioBoxSetChecked(settingVoiceOffRadioBox, false);
        settingEditMode = SETTING_EDIT_MENU;
        break;

    case SETTING_EDIT_SCREENSAVER:
        ituRadioBoxSetChecked(settingScreensaver3MinRadioBox, false);
        ituRadioBoxSetChecked(settingScreensaver5MinRadioBox, false);
        ituRadioBoxSetChecked(settingScreensaver10MinRadioBox, false);
        ituRadioBoxSetChecked(settingScreensaverMiscRadioBox, false);
        settingEditMode = SETTING_EDIT_MENU;
        break;

    case SETTING_EDIT_MESSAGE:
        ituRadioBoxSetChecked(settingMessage1RadioBox, false);
        ituRadioBoxSetChecked(settingMessage2RadioBox, false);
        ituRadioBoxSetChecked(settingMessage3RadioBox, false);
        ituRadioBoxSetChecked(settingMessage4RadioBox, false);
        settingEditMode = SETTING_EDIT_MENU;
        break;

    case SETTING_EDIT_UPGRADE:
        ituRadioBoxSetChecked(settingUpgradeDataRadioBox, false);
        ituRadioBoxSetChecked(settingUpgradeFirmwareRadioBox, false);
        settingEditMode = SETTING_EDIT_MENU;
        break;

    case SETTING_EDIT_DATE_FORMAT:
    case SETTING_EDIT_DATE_DATE:
    case SETTING_EDIT_DATE_TIME:
        settingDateYearRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateMonthRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateDayRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateHourRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateMinuteRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateSecondRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        ituRadioBoxSetChecked(settingDateFormat1RadioBox, false);
        ituRadioBoxSetChecked(settingDateFormat2RadioBox, false);
        ituRadioBoxSetChecked(settingDateFormat3RadioBox, false);
        ituRadioBoxSetChecked(settingDateYearRadioBox, false);
        ituRadioBoxSetChecked(settingDateMonthRadioBox, false);
        ituRadioBoxSetChecked(settingDateDayRadioBox, false);
        ituRadioBoxSetChecked(settingDateHourRadioBox, false);
        ituRadioBoxSetChecked(settingDateMinuteRadioBox, false);
        ituRadioBoxSetChecked(settingDateSecondRadioBox, false);
        ituWidgetSetVisible(settingDateFormatBackground, true);
        ituWidgetSetVisible(settingDateDateBackground, false);
        ituWidgetSetVisible(settingDateTimeBackground, false);
        settingEditMode = SETTING_EDIT_MENU;
        break;

    case SETTING_EDIT_DATE_DATE_YEAR:
    case SETTING_EDIT_DATE_DATE_MONTH:
    case SETTING_EDIT_DATE_DATE_DAY:
        settingDateYearRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateMonthRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateDayRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingEditMode = SETTING_EDIT_DATE_DATE;
        break;

    case SETTING_EDIT_DATE_TIME_HOUR:
    case SETTING_EDIT_DATE_TIME_MINUTE:
    case SETTING_EDIT_DATE_TIME_SECOND:
        settingDateHourRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateMinuteRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingDateSecondRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingEditMode = SETTING_EDIT_DATE_TIME;
        break;

    case SETTING_EDIT_SCREENSAVER_MISC:
        settingScreensaverTime = theConfig.screensaver_time;
        sprintf(buf, "%02i", settingScreensaverTime);
        ituButtonSetString(settingScreensaverMiscRadioBox, buf);
        settingScreensaverMiscRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
        settingEditMode = SETTING_EDIT_SCREENSAVER;
        break;
    }
}

static void SettingKeyUp(void)
{
    char buf[32];

    switch (settingEditMode)
    {
    case SETTING_EDIT_MENU:
        if ((int)(--settingMenu) < SETTING_LANG)
            settingMenu = SETTING_UPGRADE;

        SettingShowMenu();
        break;

    case SETTING_EDIT_LANG:
        if (ituRadioBoxIsChecked(settingLangChtRadioBox))
            ituRadioBoxSetChecked(settingLangEngRadioBox, true);
        else if (ituRadioBoxIsChecked(settingLangChsRadioBox))
            ituRadioBoxSetChecked(settingLangChtRadioBox, true);
        else
            ituRadioBoxSetChecked(settingLangChsRadioBox, true);
        break;

    case SETTING_EDIT_DEMO:
        if (ituRadioBoxIsChecked(settingDemoOnRadioBox))
            ituRadioBoxSetChecked(settingDemoOffRadioBox, true);
        else
            ituRadioBoxSetChecked(settingDemoOnRadioBox, true);
        break;

    case SETTING_EDIT_DATE_FORMAT:
        if (ituRadioBoxIsChecked(settingDateFormat1RadioBox))
            ;
        else if (ituRadioBoxIsChecked(settingDateFormat2RadioBox))
            ituRadioBoxSetChecked(settingDateFormat1RadioBox, true);
        else
            ituRadioBoxSetChecked(settingDateFormat2RadioBox, true);
        break;

    case SETTING_EDIT_DATE_DATE:
        if (ituRadioBoxIsChecked(settingDateYearRadioBox))
        {
            ituRadioBoxSetChecked(settingDateFormat2RadioBox, true);
            ituWidgetSetVisible(settingDateFormatBackground, true);
            ituWidgetSetVisible(settingDateDateBackground, false);
            settingEditMode = SETTING_EDIT_DATE_FORMAT;
        }
        else if (ituRadioBoxIsChecked(settingDateMonthRadioBox))
        {
            ituRadioBoxSetChecked(settingDateYearRadioBox, true);
        }
        else if (ituRadioBoxIsChecked(settingDateDayRadioBox))
        {
            ituRadioBoxSetChecked(settingDateMonthRadioBox, true);
        }
        break;

    case SETTING_EDIT_DATE_DATE_YEAR:
        if (settingYear < 2106)
            settingYear++;

        sprintf(buf, "%04i", settingYear);
        ituButtonSetString(settingDateYearRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_DATE_MONTH:
        if (settingMonth < 12)
            settingMonth++;

        sprintf(buf, "%02i", settingMonth);
        ituButtonSetString(settingDateMonthRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_DATE_DAY:
        if (settingDay < 31)
            settingDay++;

        sprintf(buf, "%02i", settingDay);
        ituButtonSetString(settingDateDayRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_TIME:
        if (ituRadioBoxIsChecked(settingDateHourRadioBox))
        {
            ituRadioBoxSetChecked(settingDateDayRadioBox, true);
            ituWidgetSetVisible(settingDateDateBackground, true);
            ituWidgetSetVisible(settingDateTimeBackground, false);
            settingEditMode = SETTING_EDIT_DATE_DATE;
        }
        else if (ituRadioBoxIsChecked(settingDateMinuteRadioBox))
            ituRadioBoxSetChecked(settingDateHourRadioBox, true);
        else
            ituRadioBoxSetChecked(settingDateMinuteRadioBox, true);
        break;

    case SETTING_EDIT_DATE_TIME_HOUR:
        if (settingHour < 23)
            settingHour++;

        sprintf(buf, "%02i", settingHour);
        ituButtonSetString(settingDateHourRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_TIME_MINUTE:
        if (settingMinute < 59)
            settingMinute++;

        sprintf(buf, "%02i", settingMinute);
        ituButtonSetString(settingDateMinuteRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_TIME_SECOND:
        if (settingSecond < 59)
            settingSecond++;

        sprintf(buf, "%02i", settingSecond);
        ituButtonSetString(settingDateSecondRadioBox, buf);
        break;

    case SETTING_EDIT_VOICE:
        if (ituRadioBoxIsChecked(settingVoiceOnRadioBox))
            ituRadioBoxSetChecked(settingVoiceOffRadioBox, true);
        else
            ituRadioBoxSetChecked(settingVoiceOnRadioBox, true);
        break;

    case SETTING_EDIT_SCREENSAVER:
        if (ituRadioBoxIsChecked(settingScreensaver3MinRadioBox))
            ituRadioBoxSetChecked(settingScreensaverMiscRadioBox, true);
        else if (ituRadioBoxIsChecked(settingScreensaver5MinRadioBox))
            ituRadioBoxSetChecked(settingScreensaver3MinRadioBox, true);
        else if (ituRadioBoxIsChecked(settingScreensaver10MinRadioBox))
            ituRadioBoxSetChecked(settingScreensaver5MinRadioBox, true);
        else
            ituRadioBoxSetChecked(settingScreensaver10MinRadioBox, true);
        break;

    case SETTING_EDIT_SCREENSAVER_MISC:
        if (settingScreensaverTime < 99)
            settingScreensaverTime++;

        sprintf(buf, "%02i", settingScreensaverTime);
        ituButtonSetString(settingScreensaverMiscRadioBox, buf);
        break;

    case SETTING_EDIT_MESSAGE:
        if (ituRadioBoxIsChecked(settingMessage1RadioBox))
            ituRadioBoxSetChecked(settingMessage4RadioBox, true);
        else if (ituRadioBoxIsChecked(settingMessage2RadioBox))
            ituRadioBoxSetChecked(settingMessage1RadioBox, true);
        else if (ituRadioBoxIsChecked(settingMessage3RadioBox))
            ituRadioBoxSetChecked(settingMessage2RadioBox, true);
        else
            ituRadioBoxSetChecked(settingMessage3RadioBox, true);
        break;

    case SETTING_EDIT_UPGRADE:
        if (ituRadioBoxIsChecked(settingUpgradeDataRadioBox))
            ituRadioBoxSetChecked(settingUpgradeFirmwareRadioBox, true);
        else
            ituRadioBoxSetChecked(settingUpgradeDataRadioBox, true);
        break;
    }
}

static void SettingDown(void)
{
    char buf[32];

    switch (settingEditMode)
    {
    case SETTING_EDIT_MENU:
        if (++settingMenu > SETTING_UPGRADE)
            settingMenu = SETTING_LANG;

        SettingShowMenu();
        break;

    case SETTING_EDIT_LANG:
        if (ituRadioBoxIsChecked(settingLangChtRadioBox))
            ituRadioBoxSetChecked(settingLangChsRadioBox, true);
        else if (ituRadioBoxIsChecked(settingLangChsRadioBox))
            ituRadioBoxSetChecked(settingLangEngRadioBox, true);
        else
            ituRadioBoxSetChecked(settingLangChtRadioBox, true);
        break;

    case SETTING_EDIT_DEMO:
        if (ituRadioBoxIsChecked(settingDemoOnRadioBox))
            ituRadioBoxSetChecked(settingDemoOffRadioBox, true);
        else
            ituRadioBoxSetChecked(settingDemoOnRadioBox, true);
        break;

    case SETTING_EDIT_DATE_FORMAT:
        if (ituRadioBoxIsChecked(settingDateFormat1RadioBox))
            ituRadioBoxSetChecked(settingDateFormat2RadioBox, true);
        else if (ituRadioBoxIsChecked(settingDateFormat2RadioBox))
            ituRadioBoxSetChecked(settingDateFormat3RadioBox, true);
        else
        {
            ituRadioBoxSetChecked(settingDateYearRadioBox, true);
            ituWidgetSetVisible(settingDateFormatBackground, false);
            ituWidgetSetVisible(settingDateDateBackground, true);
            settingEditMode = SETTING_EDIT_DATE_DATE;
        }
        break;

    case SETTING_EDIT_DATE_DATE:
        if (ituRadioBoxIsChecked(settingDateYearRadioBox))
        {
            ituRadioBoxSetChecked(settingDateMonthRadioBox, true);
        }
        else if (ituRadioBoxIsChecked(settingDateMonthRadioBox))
        {
            ituRadioBoxSetChecked(settingDateDayRadioBox, true);
        }
        else if (ituRadioBoxIsChecked(settingDateDayRadioBox))
        {
            ituRadioBoxSetChecked(settingDateHourRadioBox, true);
            ituWidgetSetVisible(settingDateDateBackground, false);
            ituWidgetSetVisible(settingDateTimeBackground, true);
            settingEditMode = SETTING_EDIT_DATE_TIME;
        }
        break;

    case SETTING_EDIT_DATE_DATE_YEAR:
        if (settingYear > 2001)
            settingYear--;

        sprintf(buf, "%04i", settingYear);
        ituButtonSetString(settingDateYearRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_DATE_MONTH:
        if (settingMonth > 1)
            settingMonth--;

        sprintf(buf, "%02i", settingMonth);
        ituButtonSetString(settingDateMonthRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_DATE_DAY:
        if (settingDay > 1)
            settingDay--;

        sprintf(buf, "%02i", settingDay);
        ituButtonSetString(settingDateDayRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_TIME:
        if (ituRadioBoxIsChecked(settingDateHourRadioBox))
            ituRadioBoxSetChecked(settingDateMinuteRadioBox, true);
        else if (ituRadioBoxIsChecked(settingDateMinuteRadioBox))
            ituRadioBoxSetChecked(settingDateSecondRadioBox, true);
        else
            ;
        break;

    case SETTING_EDIT_DATE_TIME_HOUR:
        if (settingHour > 0)
            settingHour--;

        sprintf(buf, "%02i", settingHour);
        ituButtonSetString(settingDateHourRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_TIME_MINUTE:
        if (settingMinute > 0)
            settingMinute--;

        sprintf(buf, "%02i", settingMinute);
        ituButtonSetString(settingDateMinuteRadioBox, buf);
        break;

    case SETTING_EDIT_DATE_TIME_SECOND:
        if (settingSecond > 0)
            settingSecond--;

        sprintf(buf, "%02i", settingSecond);
        ituButtonSetString(settingDateSecondRadioBox, buf);
        break;

    case SETTING_EDIT_VOICE:
        if (ituRadioBoxIsChecked(settingVoiceOnRadioBox))
            ituRadioBoxSetChecked(settingVoiceOffRadioBox, true);
        else
            ituRadioBoxSetChecked(settingVoiceOnRadioBox, true);
        break;

    case SETTING_EDIT_SCREENSAVER:
        if (ituRadioBoxIsChecked(settingScreensaver3MinRadioBox))
            ituRadioBoxSetChecked(settingScreensaver5MinRadioBox, true);
        else if (ituRadioBoxIsChecked(settingScreensaver5MinRadioBox))
            ituRadioBoxSetChecked(settingScreensaver10MinRadioBox, true);
        else if (ituRadioBoxIsChecked(settingScreensaver10MinRadioBox))
            ituRadioBoxSetChecked(settingScreensaverMiscRadioBox, true);
        else
            ituRadioBoxSetChecked(settingScreensaver3MinRadioBox, true);
        break;

    case SETTING_EDIT_SCREENSAVER_MISC:
        if (settingScreensaverTime > 0)
            settingScreensaverTime--;

        sprintf(buf, "%02i", settingScreensaverTime);
        ituButtonSetString(settingScreensaverMiscRadioBox, buf);
        break;

    case SETTING_EDIT_MESSAGE:
        if (ituRadioBoxIsChecked(settingMessage1RadioBox))
            ituRadioBoxSetChecked(settingMessage2RadioBox, true);
        else if (ituRadioBoxIsChecked(settingMessage2RadioBox))
            ituRadioBoxSetChecked(settingMessage3RadioBox, true);
        else if (ituRadioBoxIsChecked(settingMessage3RadioBox))
            ituRadioBoxSetChecked(settingMessage4RadioBox, true);
        else
            ituRadioBoxSetChecked(settingMessage1RadioBox, true);
        break;

    case SETTING_EDIT_UPGRADE:
        if (ituRadioBoxIsChecked(settingUpgradeDataRadioBox))
            ituRadioBoxSetChecked(settingUpgradeFirmwareRadioBox, true);
        else
            ituRadioBoxSetChecked(settingUpgradeDataRadioBox, true);
        break;
    }
}

bool SettingOnKeyDown(ITUWidget* widget, char* param)
{
    int key = atoi(param);

    switch (key)
    {
    case SDLK_RETURN:
        SettingEnter();
        break;

    case SDLK_UP:
        SettingKeyUp();
        break;

    case SDLK_DOWN:
        SettingDown();
        break;

    case SDLK_INSERT:
        SettingLeave();
        break;

    case SDLK_SPACE:
        break;
    }
    return true;
}

bool SettingOnTimer(ITUWidget* widget, char* param)
{
    char buf[32];
    struct timeval tv;
    struct tm* tm;

	MainOnTimer(NULL, NULL);

    switch (settingEditMode)
    {
    case SETTING_EDIT_DATE_DATE_YEAR:
    case SETTING_EDIT_DATE_DATE_MONTH:
    case SETTING_EDIT_DATE_DATE_DAY:
    case SETTING_EDIT_DATE_TIME_HOUR:
    case SETTING_EDIT_DATE_TIME_MINUTE:
    case SETTING_EDIT_DATE_TIME_SECOND:
        return false;
    }

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    settingYear = tm->tm_year + 1900;
    sprintf(buf, "%04i", settingYear);
    ituButtonSetString(settingDateYearRadioBox, buf);

    settingMonth = tm->tm_mon + 1;
    sprintf(buf, "%02i", settingMonth);
    ituButtonSetString(settingDateMonthRadioBox, buf);
    
    settingDay = tm->tm_mday;
    sprintf(buf, "%02i", settingDay);
    ituButtonSetString(settingDateDayRadioBox, buf);

    settingHour = tm->tm_hour;
    sprintf(buf, "%02i", settingHour);
    ituButtonSetString(settingDateHourRadioBox, buf);

    settingMinute = tm->tm_min;
    sprintf(buf, "%02i", settingMinute);
    ituButtonSetString(settingDateMinuteRadioBox, buf);

    settingSecond = tm->tm_sec;
    sprintf(buf, "%02i", settingSecond);
    ituButtonSetString(settingDateSecondRadioBox, buf);

    return true;
}

bool SettingOnEnter(ITUWidget* widget, char* param)
{
    char buf[32];

    if (!settingLangRadioBox)
    {
        settingLangRadioBox = ituSceneFindWidget(&theScene, "settingLangRadioBox");
        assert(settingLangRadioBox);

        settingDemoRadioBox = ituSceneFindWidget(&theScene, "settingDemoRadioBox");
        assert(settingDemoRadioBox);

        settingDateRadioBox = ituSceneFindWidget(&theScene, "settingDateRadioBox");
        assert(settingDateRadioBox);

        settingVoiceRadioBox = ituSceneFindWidget(&theScene, "settingVoiceRadioBox");
        assert(settingVoiceRadioBox);

        settingScreensaverRadioBox = ituSceneFindWidget(&theScene, "settingScreensaverRadioBox");
        assert(settingScreensaverRadioBox);

        settingMessageRadioBox = ituSceneFindWidget(&theScene, "settingMessageRadioBox");
        assert(settingMessageRadioBox);

        settingUpgradeRadioBox = ituSceneFindWidget(&theScene, "settingUpgradeRadioBox");
        assert(settingUpgradeRadioBox);

        settingContentSprite = ituSceneFindWidget(&theScene, "settingContentSprite");
        assert(settingContentSprite);

        settingLangChtRadioBox = ituSceneFindWidget(&theScene, "settingLangChtRadioBox");
        assert(settingLangChtRadioBox);

        settingLangChsRadioBox = ituSceneFindWidget(&theScene, "settingLangChsRadioBox");
        assert(settingLangChsRadioBox);

        settingLangEngRadioBox = ituSceneFindWidget(&theScene, "settingLangEngRadioBox");
        assert(settingLangEngRadioBox);

        settingLangChtCheckRadioBox = ituSceneFindWidget(&theScene, "settingLangChtCheckRadioBox");
        assert(settingLangChtCheckRadioBox);

        settingLangChsCheckRadioBox = ituSceneFindWidget(&theScene, "settingLangChsCheckRadioBox");
        assert(settingLangChsCheckRadioBox);

        settingLangEngCheckRadioBox = ituSceneFindWidget(&theScene, "settingLangEngCheckRadioBox");
        assert(settingLangEngCheckRadioBox);

        settingDemoOnRadioBox = ituSceneFindWidget(&theScene, "settingDemoOnRadioBox");
        assert(settingDemoOnRadioBox);

        settingDemoOffRadioBox = ituSceneFindWidget(&theScene, "settingDemoOffRadioBox");
        assert(settingDemoOffRadioBox);

        settingDemoOnCheckRadioBox = ituSceneFindWidget(&theScene, "settingDemoOnCheckRadioBox");
        assert(settingDemoOnCheckRadioBox);

        settingDemoOffCheckRadioBox = ituSceneFindWidget(&theScene, "settingDemoOffCheckRadioBox");
        assert(settingDemoOffCheckRadioBox);

        settingDateFormatBackground = ituSceneFindWidget(&theScene, "settingDateFormatBackground");
        assert(settingDateFormatBackground);

        settingDateFormat1RadioBox = ituSceneFindWidget(&theScene, "settingDateFormat1RadioBox");
        assert(settingDateFormat1RadioBox);

        settingDateFormat2RadioBox = ituSceneFindWidget(&theScene, "settingDateFormat2RadioBox");
        assert(settingDateFormat2RadioBox);

        settingDateFormat3RadioBox = ituSceneFindWidget(&theScene, "settingDateFormat3RadioBox");
        assert(settingDateFormat3RadioBox);

        settingDateFormat1CheckRadioBox = ituSceneFindWidget(&theScene, "settingDateFormat1CheckRadioBox");
        assert(settingDateFormat1CheckRadioBox);

        settingDateFormat2CheckRadioBox = ituSceneFindWidget(&theScene, "settingDateFormat2CheckRadioBox");
        assert(settingDateFormat2CheckRadioBox);

        settingDateFormat3CheckRadioBox = ituSceneFindWidget(&theScene, "settingDateFormat3CheckRadioBox");
        assert(settingDateFormat3CheckRadioBox);

        settingDateDateBackground = ituSceneFindWidget(&theScene, "settingDateDateBackground");
        assert(settingDateDateBackground);

        settingDateYearRadioBox = ituSceneFindWidget(&theScene, "settingDateYearRadioBox");
        assert(settingDateYearRadioBox);

        settingDateMonthRadioBox = ituSceneFindWidget(&theScene, "settingDateMonthRadioBox");
        assert(settingDateMonthRadioBox);

        settingDateDayRadioBox = ituSceneFindWidget(&theScene, "settingDateDayRadioBox");
        assert(settingDateDayRadioBox);

        settingDateTimeBackground = ituSceneFindWidget(&theScene, "settingDateTimeBackground");
        assert(settingDateTimeBackground);

        settingDateHourRadioBox = ituSceneFindWidget(&theScene, "settingDateHourRadioBox");
        assert(settingDateHourRadioBox);

        settingDateMinuteRadioBox = ituSceneFindWidget(&theScene, "settingDateMinuteRadioBox");
        assert(settingDateMinuteRadioBox);

        settingDateSecondRadioBox = ituSceneFindWidget(&theScene, "settingDateSecondRadioBox");
        assert(settingDateSecondRadioBox);

        settingVoiceOnRadioBox = ituSceneFindWidget(&theScene, "settingVoiceOnRadioBox");
        assert(settingVoiceOnRadioBox);

        settingVoiceOffRadioBox = ituSceneFindWidget(&theScene, "settingVoiceOffRadioBox");
        assert(settingVoiceOffRadioBox);

        settingVoiceOnCheckRadioBox = ituSceneFindWidget(&theScene, "settingVoiceOnCheckRadioBox");
        assert(settingVoiceOnCheckRadioBox);

        settingVoiceOffCheckRadioBox = ituSceneFindWidget(&theScene, "settingVoiceOffCheckRadioBox");
        assert(settingVoiceOffCheckRadioBox);

        settingScreensaver3MinRadioBox = ituSceneFindWidget(&theScene, "settingScreensaver3MinRadioBox");
        assert(settingScreensaver3MinRadioBox);

        settingScreensaver5MinRadioBox = ituSceneFindWidget(&theScene, "settingScreensaver5MinRadioBox");
        assert(settingScreensaver5MinRadioBox);

        settingScreensaver10MinRadioBox = ituSceneFindWidget(&theScene, "settingScreensaver10MinRadioBox");
        assert(settingScreensaver10MinRadioBox);

        settingScreensaverMiscRadioBox = ituSceneFindWidget(&theScene, "settingScreensaverMiscRadioBox");
        assert(settingScreensaverMiscRadioBox);

        settingScreensaver3MinCheckRadioBox = ituSceneFindWidget(&theScene, "settingScreensaver3MinCheckRadioBox");
        assert(settingScreensaver3MinCheckRadioBox);

        settingScreensaver5MinCheckRadioBox = ituSceneFindWidget(&theScene, "settingScreensaver5MinCheckRadioBox");
        assert(settingScreensaver5MinCheckRadioBox);

        settingScreensaver10MinCheckRadioBox = ituSceneFindWidget(&theScene, "settingScreensaver10MinCheckRadioBox");
        assert(settingScreensaver10MinCheckRadioBox);

        settingScreensaverMiscCheckRadioBox = ituSceneFindWidget(&theScene, "settingScreensaverMiscCheckRadioBox");
        assert(settingScreensaverMiscCheckRadioBox);

        settingMessage1RadioBox = ituSceneFindWidget(&theScene, "settingMessage1RadioBox");
        assert(settingMessage1RadioBox);

        settingMessage2RadioBox = ituSceneFindWidget(&theScene, "settingMessage2RadioBox");
        assert(settingMessage2RadioBox);

        settingMessage3RadioBox = ituSceneFindWidget(&theScene, "settingMessage3RadioBox");
        assert(settingMessage3RadioBox);

        settingMessage4RadioBox = ituSceneFindWidget(&theScene, "settingMessage4RadioBox");
        assert(settingMessage4RadioBox);

        settingMessage1CheckRadioBox = ituSceneFindWidget(&theScene, "settingMessage1CheckRadioBox");
        assert(settingMessage1CheckRadioBox);

        settingMessage2CheckRadioBox = ituSceneFindWidget(&theScene, "settingMessage2CheckRadioBox");
        assert(settingMessage2CheckRadioBox);

        settingMessage3CheckRadioBox = ituSceneFindWidget(&theScene, "settingMessage3CheckRadioBox");
        assert(settingMessage3CheckRadioBox);

        settingMessage4CheckRadioBox = ituSceneFindWidget(&theScene, "settingMessage4CheckRadioBox");
        assert(settingMessage4CheckRadioBox);

        settingUpgradeDataRadioBox = ituSceneFindWidget(&theScene, "settingUpgradeDataRadioBox");
        assert(settingUpgradeDataRadioBox);

        settingUpgradeFirmwareRadioBox = ituSceneFindWidget(&theScene, "settingUpgradeFirmwareRadioBox");
        assert(settingUpgradeFirmwareRadioBox);

        mainLayer = ituSceneFindWidget(&theScene, "mainLayer");
        assert(mainLayer);
    }
    settingEditMode = SETTING_EDIT_MENU;
    settingMenu = SETTING_LANG;
    SettingShowMenu();

    switch (theConfig.lang)
    {
    case LANG_ENG:
        ituRadioBoxSetChecked(settingLangEngCheckRadioBox, true);
        break;

    case LANG_CHT:
        ituRadioBoxSetChecked(settingLangChtCheckRadioBox, true);
        break;

    case LANG_CHS:
        ituRadioBoxSetChecked(settingLangChsCheckRadioBox, true);
        break;
    }

    if (theConfig.demo_enable)
        ituRadioBoxSetChecked(settingDemoOnCheckRadioBox, true);
    else
        ituRadioBoxSetChecked(settingDemoOffCheckRadioBox, true);

    ituWidgetSetVisible(settingDateFormatBackground, true);
    ituWidgetSetVisible(settingDateDateBackground, false);
    ituWidgetSetVisible(settingDateTimeBackground, false);

    settingDateYearRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
    settingDateMonthRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
    settingDateDayRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
    settingDateHourRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
    settingDateMinuteRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;
    settingDateSecondRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;

    switch (theConfig.date_format)
    {
    case 0:
        ituRadioBoxSetChecked(settingDateFormat1CheckRadioBox, true);
        break;

    case 1:
        ituRadioBoxSetChecked(settingDateFormat2CheckRadioBox, true);
        break;

    case 2:
        ituRadioBoxSetChecked(settingDateFormat3CheckRadioBox, true);
        break;
    }

    if (theConfig.sound_enable)
        ituRadioBoxSetChecked(settingVoiceOnCheckRadioBox, true);
    else
        ituRadioBoxSetChecked(settingVoiceOffCheckRadioBox, true);

    settingScreensaverTime = theConfig.screensaver_time;
    sprintf(buf, "%02i", settingScreensaverTime);
    ituButtonSetString(settingScreensaverMiscRadioBox, buf);
    settingScreensaverMiscRadioBox->checkbox.btn.bwin.widget.color.alpha = 0;

    switch (settingScreensaverTime)
    {
    case 3:
        ituRadioBoxSetChecked(settingScreensaver3MinCheckRadioBox, true);
        break;

    case 5:
        ituRadioBoxSetChecked(settingScreensaver5MinCheckRadioBox, true);
        break;

    case 10:
        ituRadioBoxSetChecked(settingScreensaver10MinCheckRadioBox, true);
        break;

    default:
        ituRadioBoxSetChecked(settingScreensaverMiscCheckRadioBox, true);
    }

    switch (theConfig.info1_format)
    {
    case 0:
        ituRadioBoxSetChecked(settingMessage1CheckRadioBox, true);
        break;

    case 1:
        ituRadioBoxSetChecked(settingMessage2CheckRadioBox, true);
        break;

    case 2:
        ituRadioBoxSetChecked(settingMessage3CheckRadioBox, true);
        break;

    case 3:
        ituRadioBoxSetChecked(settingMessage4CheckRadioBox, true);
    }

    ituRadioBoxSetChecked(settingUpgradeDataRadioBox, false);
    ituRadioBoxSetChecked(settingUpgradeFirmwareRadioBox, false);

    settingModified = false;

    return true;
}

bool SettingOnLeave(ITUWidget* widget, char* param)
{
    if (settingModified)
        ConfigSave();

    return true;
}

void SettingReset(void)
{
    if (settingModified)
        ConfigSave();

    settingLangRadioBox = NULL;
}
