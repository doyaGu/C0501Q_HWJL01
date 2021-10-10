/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL RTC internal functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

static void RtcSecIntrHandler(void* arg)
{
    ithTimerSetCounter(ITH_TIMER4, 0);  // reset counter
    ithRtcClearIntr(ITH_RTC_SEC);
}

void itpRtcInit(void)
{
    ithRtcInit(CFG_RTC_EXTCLK);
    if (ithRtcEnable())
    {
        LOG_INFO "First time boot\n" LOG_END
        ithRtcSetTime(CFG_RTC_DEFAULT_TIMESTAMP);
    }

    // init timer4 to calc usec of gettimeofday()
    ithTimerReset(ITH_TIMER4);
    ithTimerCtrlEnable(ITH_TIMER4, ITH_TIMER_UPCOUNT);
    ithTimerSetCounter(ITH_TIMER4, 0);
    ithTimerEnable(ITH_TIMER4);

    // init rtc sec interrupt
    ithRtcCtrlEnable(ITH_RTC_INTR_SEC);
    ithIntrRegisterHandlerIrq(ITH_INTR_RTCSEC, RtcSecIntrHandler, NULL);
    ithIntrSetTriggerModeIrq(ITH_INTR_RTCSEC, ITH_INTR_EDGE);
    ithIntrEnableIrq(ITH_INTR_RTCSEC);
}

long itpRtcGetTime(long* usec)
{
    long sec1, sec2;
    do 
    {
        sec1 = ithRtcGetTime();
        if (usec)
            *usec = ithTimerGetTime(ITH_TIMER4);

        sec2 = ithRtcGetTime();
        
    } while (sec1 != sec2);

    return sec1;
}

void itpRtcSetTime(long sec, long usec)
{
    ithRtcSetTime(sec + (usec / 1000000));
}
