/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL RTC software functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <time.h>
#include "../itp_cfg.h"

static unsigned long rtcSec, rtcMsec, rtcLastMS;

static void RtcHandler(timer_t timerid, int arg)
{
    uint32_t ms, diff;
    uint64_t last, now;

    ithEnterCritical();

    ms = itpGetTickCount();
    if (ms >= rtcLastMS)
        diff = ms - rtcLastMS;
    else
        diff = ms + (0xFFFFFFFF - rtcLastMS); // overflow
    
    last = (uint64_t)rtcSec * 1000 + rtcMsec;
    now = last + diff;
    
    rtcLastMS   = ms;
    rtcSec      = now / 1000;
    rtcMsec     = now % 1000;
    
    ithExitCritical();
}

void itpRtcInit(void)
{
    timer_t timerId;
    struct itimerspec value;

    value.it_value.tv_sec       = 60;
    value.it_value.tv_nsec      = 0;
    value.it_interval.tv_sec    = value.it_value.tv_sec;
    value.it_interval.tv_nsec   = value.it_value.tv_nsec;

    timer_create(CLOCK_REALTIME, NULL, &timerId);
    timer_connect(timerId, RtcHandler, 0);
    
    rtcLastMS = itpGetTickCount();
    rtcMsec = (rtcLastMS % 1000) * 1000;
    rtcSec  = CFG_RTC_DEFAULT_TIMESTAMP + rtcLastMS / 1000;

    timer_settime(timerId, 0, &value, NULL);
}

long itpRtcGetTime(long* usec)
{
    uint32_t ms, diff;
    uint64_t last, now;

    ithEnterCritical();

    ms = itpGetTickCount();
    if (ms >= rtcLastMS)
        diff = ms - rtcLastMS;
    else
        diff = ms + (0xFFFFFFFF - rtcLastMS); // overflow

    last = (uint64_t)rtcSec * 1000 + rtcMsec;
    now = last + diff;
    
    rtcLastMS   = ms;
    rtcSec      = now / 1000;
    rtcMsec     = now % 1000;
    
    if (usec)
        *usec = rtcMsec * 1000;

    ithExitCritical();

    return rtcSec;
}

void itpRtcSetTime(long sec, long usec)
{
    ithEnterCritical();
    
    rtcLastMS   = itpGetTickCount();
    rtcSec      = (unsigned long)sec;
    rtcMsec     = usec / 1000;

    ithExitCritical();
}
