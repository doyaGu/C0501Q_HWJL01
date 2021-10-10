#include <sys/time.h>
#include <assert.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUAnalogClock* clockAnalogClock;
static ITUSprite* clockNoonSprite;
static ITUBackground* clockGrayBackground;
static ITUWheel* clockHourWheel;
static ITUWheel* clockMinuteWheel;

bool ClockWheelOnChanged(ITUWidget* widget, char* param)
{
    //AudioPlayKeySound();
    return true;
}

bool ClockOnTimer(ITUWidget* widget, char* param)
{
    if (clockNoonSprite->frame == 0)
    {
        if (clockAnalogClock->hour >= 12)
            ituSpriteGoto(clockNoonSprite, 1);
    }
    else
    {
        if (clockAnalogClock->hour < 12)
            ituSpriteGoto(clockNoonSprite, 0);
    }
    return true;
}

bool ClockConfirmButtonOnPress(ITUWidget* widget, char* param)
{
    struct timeval tv;
    struct tm *tm, mytime;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    memcpy(&mytime, tm, sizeof (struct tm));

    mytime.tm_hour = clockHourWheel->focusIndex;
    mytime.tm_min = clockMinuteWheel->focusIndex;
    mytime.tm_sec = 0;

    tv.tv_sec = mktime(&mytime);
    tv.tv_usec = 0;

    settimeofday(&tv, NULL);
    return true;
}

bool ClockOnEnter(ITUWidget* widget, char* param)
{
    if (!clockAnalogClock)
    {
        clockAnalogClock = ituSceneFindWidget(&theScene, "clockAnalogClock");
        assert(clockAnalogClock);

        clockNoonSprite = ituSceneFindWidget(&theScene, "clockNoonSprite");
        assert(clockNoonSprite);

        clockGrayBackground = ituSceneFindWidget(&theScene, "clockGrayBackground");
        assert(clockGrayBackground);

        clockHourWheel = ituSceneFindWidget(&theScene, "clockHourWheel");
        assert(clockHourWheel);

        clockMinuteWheel = ituSceneFindWidget(&theScene, "clockMinuteWheel");
        assert(clockMinuteWheel);
    }
    ituAnalogClockUpdate((ITUWidget*)clockAnalogClock, ITU_EVENT_TIMER, 0, 0, 0);
    ituWheelGoto(clockHourWheel, clockAnalogClock->hour);
    ituWheelGoto(clockMinuteWheel, clockAnalogClock->minute);

    return true;
}

void ClockReset(void)
{
    clockAnalogClock = NULL;
}
