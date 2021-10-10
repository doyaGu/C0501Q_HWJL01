#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "ite/itp.h"
#include "airconditioner.h"

static uint32_t powerTick;
static time_t powerTime;

void PowerInit(void)
{
    time(&powerTime);
    powerTick = itpGetTickCount();
}

int PowerCheck(void)
{
    if (itpGetTickDuration(powerTick) >= 1000)
    {
        powerTick = itpGetTickCount();

        if (theConfig.power_on_enable || theConfig.power_off_enable)
        {
            struct timeval tv;
            struct tm *tm, mytime;

            gettimeofday(&tv, NULL);
            tm = localtime(&tv.tv_sec);

            memcpy(&mytime, tm, sizeof (struct tm));

            if (theConfig.power_mon_enable || 
                theConfig.power_tue_enable || 
                theConfig.power_wed_enable || 
                theConfig.power_thu_enable || 
                theConfig.power_fri_enable || 
                theConfig.power_sat_enable || 
                theConfig.power_sun_enable)
            {
                if ((theConfig.power_sun_enable && mytime.tm_wday == 0) ||
                    (theConfig.power_mon_enable && mytime.tm_wday == 1) ||
                    (theConfig.power_tue_enable && mytime.tm_wday == 2) ||
                    (theConfig.power_wed_enable && mytime.tm_wday == 3) ||
                    (theConfig.power_thu_enable && mytime.tm_wday == 4) ||
                    (theConfig.power_fri_enable && mytime.tm_wday == 5) ||
                    (theConfig.power_sun_enable && mytime.tm_wday == 6))
                {
                    if (theConfig.power_on_enable)
                    {
                        if (mytime.tm_hour == theConfig.power_on_hour &&
                            mytime.tm_min == theConfig.power_on_min)
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        if (mytime.tm_hour == theConfig.power_off_hour &&
                            mytime.tm_min == theConfig.power_off_min)
                        {
                            return -1;
                        }
                    }
                }
            }
            else
            {
                if (theConfig.power_on_enable)
                {
                    if (mytime.tm_hour == theConfig.power_on_hour &&
                        mytime.tm_min == theConfig.power_on_min)
                    {
                        return 1;
                    }
                }
                else
                {
                    if (mytime.tm_hour == theConfig.power_off_hour &&
                        mytime.tm_min == theConfig.power_off_min)
                    {
                        return -1;
                    }
                }
            }
        }
    }
    return 0;
}
