#include <sys/time.h>
#include <assert.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "scene.h"
#include "sports_equipment.h"

#define MIN_AGE     10
#define MIN_WEIGHT  10
#define MIN_PULSE   89
#define MIN_YEAR    2001

static ITUWheel* settingAgeWheel;
static ITUWheel* settingWeightWheel;
static ITUWheel* settingPulseWheel;
static ITUProgressBar* settingVolumeProgressBar;
static ITUTrackBar* settingVolumeTrackBar;
static ITUProgressBar* settingBrightnessProgressBar;
static ITUTrackBar* settingBrightnessTrackBar;
static ITUWheel* settingDayWheel;
static ITUWheel* settingMonthWheel;
static ITUWheel* settingYearWheel;	
static ITUWheel* settingHourWheel;	
static ITUWheel* settingMinuteWheel;
static ITURadioBox* settingChtRadioBox;
static ITURadioBox* settingChsRadioBox;	
static ITURadioBox* settingEngRadioBox;	
static ITURadioBox* settingKMRadioBox;
static ITURadioBox* settingMileRadioBox;
static ITURadioBox* settingKGRadioBox;
static ITURadioBox* settingLbsRadioBox;

extern time_t mainBeginTime;
static bool settingModified;

bool SettingDateTimeRadioBoxOnPress(ITUWidget* widget, char* param)
{
    struct timeval tv;
    struct tm* tm;
    int value;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    value = tm->tm_year + 1900 - MIN_YEAR;
    ituWheelGoto(settingYearWheel, value);

    value = tm->tm_mon;
    ituWheelGoto(settingMonthWheel, value);

    value = tm->tm_mday - 1;
    ituWheelGoto(settingDayWheel, value);

    value = tm->tm_hour;
    ituWheelGoto(settingHourWheel, value);

    value = tm->tm_min;
    ituWheelGoto(settingMinuteWheel, value);

    return true;
}

bool SettingAgeWheelOnChanged(ITUWidget* widget, char* param)
{
    int value = settingAgeWheel->focusIndex + MIN_AGE;
    if (value != theConfig.age)
    {
        theConfig.age = value;
        settingModified = true;
    }
    return true;
}

bool SettingWeightWheelOnChanged(ITUWidget* widget, char* param)
{
    int value = settingWeightWheel->focusIndex + MIN_WEIGHT;
    if (value != theConfig.weight)
    {
        theConfig.weight = value;
        settingModified = true;
    }
    return true;
}

bool SettingPulseWheelOnChanged(ITUWidget* widget, char* param)
{
    int value = settingPulseWheel->focusIndex + MIN_PULSE;
    if (value != theConfig.pulse)
    {
        theConfig.pulse = value;
        theProgram.pulse = theConfig.pulse;
        settingModified = true;
    }
    return true;
}

bool SettingVolumeTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = settingVolumeTrackBar->value;

    if (value != theConfig.audiolevel)
    {
        AudioSetVolume(value);
        settingModified = true;
    }
    return true;
}

bool SettingBrightnessTrackBarOnChanged(ITUWidget* widget, char* param)
{
    int value = settingBrightnessTrackBar->value;

    if (value != theConfig.brightness)
    {
        ScreenSetBrightness(value);

        theConfig.brightness = value;
        settingModified = true;
    }
    return true;
}

bool SettingDateWheelOnChanged(ITUWidget* widget, char* param)
{
    if (ituWheelCheckIdle((ITUWheel*)widget))
    {
        struct timeval tv;
        struct tm *tm, mytime, mytime2;
        time_t now;
        int seconds;

        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);

        memcpy(&mytime, tm, sizeof (struct tm));

        mytime.tm_mday = settingDayWheel->focusIndex + 1;
        mytime.tm_mon = settingMonthWheel->focusIndex;
        mytime.tm_year = settingYearWheel->focusIndex - 1900 + MIN_YEAR;
        mytime.tm_hour = settingHourWheel->focusIndex;
        mytime.tm_min = settingMinuteWheel->focusIndex;

        memcpy(&mytime2, &mytime, sizeof (struct tm));

        tv.tv_sec = mktime(&mytime);
        tv.tv_usec = 0;
        printf("y:%d m:%d d:%d s:%d\n", mytime.tm_year, mytime.tm_mon, mytime.tm_mday, tv.tv_sec);

        time(&now);
        seconds = (int)difftime(tv.tv_sec, now);

        if (seconds != 0)
        {
        #ifndef _WIN32
            mainBeginTime += seconds;
            //printf("%ld %ld %ld\n", (int)now, (int)seconds, (int)mainBeginTime);
            settimeofday(&tv, NULL);
        #endif

            if (mytime.tm_mon != mytime2.tm_mon || mytime.tm_mday != mytime2.tm_mday)
            {
                ituWheelGoto(settingMonthWheel, mytime.tm_mon);
                ituWheelGoto(settingDayWheel, mytime.tm_mday - 1);
            }
            return true;
        }
    }
    return false;
}

bool SettingLanguageRadioBoxOnPress(ITUWidget* widget, char* param)
{
    int lang = theConfig.lang;

    if (ituRadioBoxIsChecked(settingChtRadioBox))
    {
        lang = LANG_CHT;
    }
    else if (ituRadioBoxIsChecked(settingChsRadioBox))
    {
        lang = LANG_CHS;
    }
    else if (ituRadioBoxIsChecked(settingEngRadioBox))
    {
        lang = LANG_ENG;
    }

    if (theConfig.lang != lang)
    {
        theConfig.lang = lang;
        SceneChangeLanguage();
        settingModified = true;
    }
    return true;
}

bool SettingUnitDistanceRadioBoxOnPress(ITUWidget* widget, char* param)
{
    int mile = theConfig.unit_mile;

    if (ituRadioBoxIsChecked(settingKMRadioBox))
    {
        mile = 0;
    }
    else if (ituRadioBoxIsChecked(settingMileRadioBox))
    {
        mile = 1;
    }

    if (theConfig.unit_mile != mile)
    {
        theConfig.unit_mile = mile;
        settingModified = true;
    }
    return true;
}

bool SettingUnitWeightRadioBoxOnPress(ITUWidget* widget, char* param)
{
    int lbs = theConfig.unit_mile;

    if (ituRadioBoxIsChecked(settingKGRadioBox))
    {
        lbs = 0;
    }
    else if (ituRadioBoxIsChecked(settingLbsRadioBox))
    {
        lbs = 1;
    }

    if (theConfig.unit_lbs != lbs)
    {
        theConfig.unit_lbs = lbs;
        settingModified = true;
    }
    return true;
}

bool SettingOnEnter(ITUWidget* widget, char* param)
{
    if (!settingAgeWheel)
    {
        settingAgeWheel = ituSceneFindWidget(&theScene, "settingAgeWheel");
        assert(settingAgeWheel);

        settingWeightWheel = ituSceneFindWidget(&theScene, "settingWeightWheel");
        assert(settingWeightWheel);

        settingPulseWheel = ituSceneFindWidget(&theScene, "settingPulseWheel");
        assert(settingPulseWheel);

        settingVolumeProgressBar = ituSceneFindWidget(&theScene, "settingVolumeProgressBar");
        assert(settingVolumeProgressBar);

        settingVolumeTrackBar = ituSceneFindWidget(&theScene, "settingVolumeTrackBar");
        assert(settingVolumeTrackBar);

        settingBrightnessProgressBar = ituSceneFindWidget(&theScene, "settingBrightnessProgressBar");
        assert(settingBrightnessProgressBar);

        settingBrightnessTrackBar = ituSceneFindWidget(&theScene, "settingBrightnessTrackBar");
        assert(settingBrightnessTrackBar);

        settingDayWheel = ituSceneFindWidget(&theScene, "settingDayWheel");
        assert(settingDayWheel);

        settingMonthWheel = ituSceneFindWidget(&theScene, "settingMonthWheel");
        assert(settingMonthWheel);

        settingYearWheel = ituSceneFindWidget(&theScene, "settingYearWheel");
        assert(settingYearWheel);

        settingHourWheel = ituSceneFindWidget(&theScene, "settingHourWheel");
        assert(settingHourWheel);

        settingMinuteWheel = ituSceneFindWidget(&theScene, "settingMinuteWheel");
        assert(settingMinuteWheel);

        settingChtRadioBox = ituSceneFindWidget(&theScene, "settingChtRadioBox");
        assert(settingChtRadioBox);

        settingChsRadioBox = ituSceneFindWidget(&theScene, "settingChsRadioBox");
        assert(settingChsRadioBox);

        settingEngRadioBox = ituSceneFindWidget(&theScene, "settingEngRadioBox");
        assert(settingEngRadioBox);

        settingKMRadioBox = ituSceneFindWidget(&theScene, "settingKMRadioBox");
        assert(settingKMRadioBox);

        settingMileRadioBox = ituSceneFindWidget(&theScene, "settingMileRadioBox");
        assert(settingMileRadioBox);

        settingKGRadioBox = ituSceneFindWidget(&theScene, "settingKGRadioBox");
        assert(settingKGRadioBox);

        settingLbsRadioBox = ituSceneFindWidget(&theScene, "settingLbsRadioBox");
        assert(settingLbsRadioBox);
    }

    ituWheelGoto(settingAgeWheel, theConfig.age - MIN_AGE);
    ituWheelGoto(settingWeightWheel, theConfig.weight - MIN_WEIGHT);
    ituWheelGoto(settingPulseWheel, theConfig.pulse - MIN_PULSE);

    ituProgressBarSetValue(settingVolumeProgressBar, theConfig.audiolevel);
    ituTrackBarSetValue(settingVolumeTrackBar, theConfig.audiolevel);

    ituProgressBarSetValue(settingBrightnessProgressBar, theConfig.brightness);
    ituTrackBarSetValue(settingBrightnessTrackBar, theConfig.brightness);

    switch (theConfig.lang)
    {
    case LANG_ENG:
        ituRadioBoxSetChecked(settingEngRadioBox, true);
        break;

    case LANG_CHT:
        ituRadioBoxSetChecked(settingChtRadioBox, true);
        break;

    case LANG_CHS:
        ituRadioBoxSetChecked(settingChsRadioBox, true);
        break;
    }

    if (theConfig.unit_mile)
        ituRadioBoxSetChecked(settingMileRadioBox, true);
    else
        ituRadioBoxSetChecked(settingKMRadioBox, true);

    if (theConfig.unit_lbs)
        ituRadioBoxSetChecked(settingLbsRadioBox, true);
    else
        ituRadioBoxSetChecked(settingKGRadioBox, true);

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
    settingAgeWheel = NULL;
}
