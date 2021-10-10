#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char dclkName[] = "ITUDigitalClock";

bool ituDigitalClockUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result;
    ITUDigitalClock* dclk = (ITUDigitalClock*) widget;
    assert(dclk);

    result = ituIconUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_TIMER && !(dclk->digitalClockFlags & ITU_DIGITALCLOCK_STOP))
    {
        char buf[8];
        struct timeval tv;
        struct tm* tm;

        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);

        if (tm->tm_sec != dclk->second)
        {
            if (dclk->secondText)
            {
                dclk->second = tm->tm_sec;

                sprintf(buf, "%02i", dclk->second);
                ituTextSetString(dclk->secondText, buf);

                result = widget->dirty = true;
            }
            if (dclk->colonText)
            {
                if (tm->tm_sec % 2)
                    ituTextSetString(dclk->colonText, " ");
                else
                    ituTextSetString(dclk->colonText, ":");

                result = widget->dirty = true;
            }
        }
        if (dclk->minuteText && tm->tm_min != dclk->minute)
        {
            dclk->minute = tm->tm_min;

            sprintf(buf, "%02i", dclk->minute);
            ituTextSetString(dclk->minuteText, buf);

            result = widget->dirty = true;
        }
        if (dclk->hourText && tm->tm_hour != dclk->hour)
        {
            int hour;

            dclk->hour = tm->tm_hour;

            if ((dclk->digitalClockFlags & ITU_DIGITALCLOCK_12H) && tm->tm_hour > 12)
                hour = tm->tm_hour - 12;
            else
                hour = tm->tm_hour;

            sprintf(buf, "%02i", hour);
            ituTextSetString(dclk->hourText, buf);

            result = widget->dirty = true;
        }
        if (dclk->weekSprite && tm->tm_wday != dclk->week)
        {
            dclk->week = tm->tm_wday;

            ituSpriteGoto(dclk->weekSprite, dclk->week);

            result = widget->dirty = true;
        }
        if (dclk->dayText && tm->tm_mday != dclk->day)
        {
            dclk->day = tm->tm_mday;

            sprintf(buf, "%02i", dclk->day);
            ituTextSetString(dclk->dayText, buf);

            result = widget->dirty = true;
        }
        if (dclk->monthText && tm->tm_mon != dclk->month)
        {
            int month;

            dclk->month = tm->tm_mon;

            month = tm->tm_mon + 1;

            sprintf(buf, "%02i", month);
            ituTextSetString(dclk->monthText, buf);

            result = widget->dirty = true;
        }
        if (dclk->yearText && tm->tm_year != dclk->year)
        {
            int year;

            dclk->year = tm->tm_year;

            year = tm->tm_year + 1900;

            sprintf(buf, "%04i", year);
            ituTextSetString(dclk->yearText, buf);

            result = widget->dirty = true;
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if (!dclk->yearText && (dclk->yearName[0] != '\0'))
        {
            dclk->yearText = (ITUText*) ituSceneFindWidget(ituScene, dclk->yearName);
            dclk->year = -1;
        }
        if (!dclk->monthText && (dclk->monthName[0] != '\0'))
        {
            dclk->monthText = (ITUText*) ituSceneFindWidget(ituScene, dclk->monthName);
            dclk->month = -1;
        }
        if (!dclk->dayText && (dclk->dayName[0] != '\0'))
        {
            dclk->dayText = (ITUText*) ituSceneFindWidget(ituScene, dclk->dayName);
            dclk->day = -1;
        }
        if (!dclk->hourText && (dclk->hourName[0] != '\0'))
        {
            dclk->hourText = (ITUText*) ituSceneFindWidget(ituScene, dclk->hourName);
            dclk->hour = -1;
        }
        if (!dclk->minuteText && (dclk->minuteName[0] != '\0'))
        {
            dclk->minuteText = (ITUText*) ituSceneFindWidget(ituScene, dclk->minuteName);
            dclk->minute = -1;
        }
        if (!dclk->secondText && (dclk->secondName[0] != '\0'))
        {
            dclk->secondText = (ITUText*) ituSceneFindWidget(ituScene, dclk->secondName);
            dclk->second = -1;
        }
        if (!dclk->weekSprite && (dclk->weekName[0] != '\0'))
        {
            dclk->weekSprite = (ITUSprite*) ituSceneFindWidget(ituScene, dclk->weekName);
            dclk->week = -1;
        }
        if (!dclk->colonText && (dclk->colonName[0] != '\0'))
        {
            dclk->colonText = (ITUText*)ituSceneFindWidget(ituScene, dclk->colonName);
        }
        result = widget->dirty = true;
    }
    return result;
}

void ituDigitalClockInit(ITUDigitalClock* dclk)
{
    assert(dclk);
    ITU_ASSERT_THREAD();

    memset(dclk, 0, sizeof (ITUDigitalClock));

    ituBackgroundInit(&dclk->bg);
    
    ituWidgetSetType(dclk, ITU_DIGITALCLOCK);
    ituWidgetSetName(dclk, dclkName);
    ituWidgetSetUpdate(dclk, ituDigitalClockUpdate);
}

void ituDigitalClockLoad(ITUDigitalClock* dclk, uint32_t base)
{
    assert(dclk);

    ituBackgroundLoad(&dclk->bg, base);

    if (dclk->yearText)
    {
        dclk->yearText = (ITUText*)((uint32_t)dclk->yearText + base);
        dclk->year = -1;
    }
    if (dclk->monthText)
    {
        dclk->monthText = (ITUText*)((uint32_t)dclk->monthText + base);
        dclk->month = -1;
    }
    if (dclk->dayText)
    {
        dclk->dayText = (ITUText*)((uint32_t)dclk->dayText + base);
        dclk->day = -1;
    }
    if (dclk->hourText)
    {
        dclk->hourText = (ITUText*)((uint32_t)dclk->hourText + base);
        dclk->hour = -1;
    }
    if (dclk->minuteText)
    {
        dclk->minuteText = (ITUText*)((uint32_t)dclk->minuteText + base);
        dclk->minute = -1;
    }
    if (dclk->secondText)
    {
        dclk->secondText = (ITUText*)((uint32_t)dclk->secondText + base);
        dclk->second = -1;
    }
    if (dclk->weekSprite)
    {
        dclk->weekSprite = (ITUSprite*)((uint32_t)dclk->weekSprite + base);
        dclk->week = -1;
    }
    if (dclk->colonText)
    {
        dclk->colonText = (ITUText*)((uint32_t)dclk->colonText + base);
    }
    ituWidgetSetUpdate(dclk, ituDigitalClockUpdate);
}
