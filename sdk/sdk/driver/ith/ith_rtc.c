/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL RTC functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static unsigned long divCycle;
static unsigned long lastTime;

static void Reset(void)
{
    ITHPackageId pkg;

    ithRtcCtrlEnable(ITH_RTC_RESET);
    ithRtcCtrlDisable(ITH_RTC_RESET);
    ithWriteRegA(ITH_RTC_BASE + ITH_RTC_DIV_REG, ITH_RTC_DIV_EN_MASK | divCycle);
    ithRtcCtrlEnable(ITH_RTC_EN);

#ifdef CFG_CHIP_REV_A0
    {
        unsigned long sec, lastsec = ithReadRegA(ITH_RTC_BASE + ITH_RTC_SEC_REG);
        for (;;)
        {
            sec = ithReadRegA(ITH_RTC_BASE + ITH_RTC_SEC_REG);
            if (sec != lastsec)
                break;

            lastsec = sec;
        }
    }
#endif // CFG_CHIP_REV_A0

    pkg = ithGetPackageId();
    if (pkg == ITH_IT9078)
	    ithSetRegBitA(ITH_RTC_BASE + ITH_RTC_DIV_REG, ITH_RTC_DIV_SRC_BIT);
	else
	    ithClearRegBitA(ITH_RTC_BASE + ITH_RTC_DIV_REG, ITH_RTC_DIV_SRC_BIT);

#ifdef CFG_CHIP_REV_A0
	if ( ithGetDeviceId() == 0x9070 )
    	ithSetRegBitA(ITH_RTC_BASE + ITH_RTC_DIV_REG, ITH_RTC_DIV_SRC_BIT); // workaround package id incorrect bug
#endif
}

void ithRtcSetDivSrc(ITHRtcClockSource clkSrc)
{
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_DIV_REG, clkSrc, 1<<ITH_RTC_DIV_SRC_BIT);
}

void ithRtcInit(unsigned long extClk)
{
    divCycle = extClk;
    ithRtcCtrlDisable(ITH_RTC_SW_POWEROFF);
}

bool ithRtcEnable(void)
{
    bool firstBoot;
    uint32_t value;

    ithSetRegBitH(ITH_APB_CLK3_REG, ITH_EN_W11CLK_BIT);

    value = ithReadRegA(ITH_RTC_BASE + ITH_RTC_DIV_REG);
    if ((value & ITH_RTC_DIV_EN_MASK) == 0 ||
        (value & ITH_RTC_DIV_CYCLE_MASK) != divCycle)
    {
        Reset();
        firstBoot = true;
    }
    else
    {
        ithRtcCtrlEnable(ITH_RTC_EN);
        firstBoot = false;
    }
    return firstBoot;
}

unsigned long ithRtcGetTime(void)
{
    unsigned long day, hour, min, sec, sec2;

    if (ithReadRegA(ITH_RTC_BASE + ITH_RTC_CR_REG) & (0x1 << ITH_RTC_COUNTER_LOAD))
        return lastTime;

    do
    {
        sec     = ithReadRegA(ITH_RTC_BASE + ITH_RTC_SEC_REG) & ITH_RTC_SEC_MASK;
        day     = ithReadRegA(ITH_RTC_BASE + ITH_RTC_DAY_REG) & ITH_RTC_DAY_MASK;
        hour    = ithReadRegA(ITH_RTC_BASE + ITH_RTC_HOUR_REG) & ITH_RTC_HOUR_MASK;
        min     = ithReadRegA(ITH_RTC_BASE + ITH_RTC_MIN_REG) & ITH_RTC_MIN_MASK;
        sec2    = ithReadRegA(ITH_RTC_BASE + ITH_RTC_SEC_REG) & ITH_RTC_SEC_MASK;
    } while (sec != sec2);

    return day * 24 * 60 * 60 + hour * 60 * 60 + min * 60 + sec;
}

void ithRtcSetTime(unsigned long t)
{
    unsigned long hour, min, sec;

    lastTime = t;

    sec = t % 60;
    t /= 60;

    min = t % 60;
    t /= 60;

    hour = t % 24;
    t /= 24;

    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_WDAY_REG, t, ITH_RTC_WDAY_MASK);
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_WHOUR_REG, hour, ITH_RTC_WHOUR_MASK);
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_WMIN_REG, min, ITH_RTC_WMIN_MASK);
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_WSEC_REG, sec, ITH_RTC_WSEC_MASK);
    ithRtcCtrlEnable(ITH_RTC_COUNTER_LOAD);
}
