/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL timer functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static uint32_t timerRegs[(ITH_TIMER_INTRMASK_REG - ITH_TIMER1_CNT_REG + 4) / 4];

void ithTimerReset(ITHTimer timer)
{
    ithWriteRegMaskA(ITH_TIMER_BASE + ITH_TIMER_INTRSTATE_REG, 0x7 << (timer * 4), 0x7 << (timer * 4));
	ithWriteRegMaskA(ITH_TIMER_BASE + ITH_TIMER_INTRMASK_REG, 0, 0x7 << (timer * 4));
    
	ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_CNT_REG, 0);
	ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_LOAD_REG, 0);
	ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH1_REG, 0);
	ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH2_REG, 0);
    ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, 0);
}

void ithTimerCtrlEnable(ITHTimer timer, ITHTimerCtrl ctrl)
{
    switch (ctrl)
    {
    case ITH_TIMER_EN:
        ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_EN_BIT);
        break;

    case ITH_TIMER_EXTCLK:
        ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_CLK_BIT);
        break;

    case ITH_TIMER_UPCOUNT:
        ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_UPDOWN_BIT);
        break;
        
    case ITH_TIMER_ONESHOT:
        ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_ONESHOT_BIT);
        break;

    case ITH_TIMER_PERIODIC:
        ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_MODE_BIT);
        break;

    case ITH_TIMER_PWM:
        ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_PWMEN_BIT);
        break;
    
    case ITH_TIMER_EN64:
        switch (timer)
        {
        case ITH_TIMER4:
        case ITH_TIMER6:
            ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_EN64_BIT);
            break;
            
        default:
            LOG_ERR "Unsupport 64-bit timer: %d\r\n", timer LOG_END
            break;
        }
        break;
    }
}

void ithTimerCtrlDisable(ITHTimer timer, ITHTimerCtrl ctrl)
{
    switch (ctrl)
    {
    case ITH_TIMER_EN:
        ithClearRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_EN_BIT);
        break;

    case ITH_TIMER_EXTCLK:
        ithClearRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_CLK_BIT);
        break;

    case ITH_TIMER_UPCOUNT:
        ithClearRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_UPDOWN_BIT);
        break;
        
    case ITH_TIMER_ONESHOT:
        ithClearRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_ONESHOT_BIT);
        break;

    case ITH_TIMER_PERIODIC:
        ithClearRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_MODE_BIT);
        break;

    case ITH_TIMER_PWM:
        ithClearRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_PWMEN_BIT);
        break;
    
    case ITH_TIMER_EN64:
        switch (timer)
        {
        case ITH_TIMER4:
        case ITH_TIMER6:
            ithClearRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, ITH_TIMER_EN64_BIT);
            break;

        default:
            LOG_ERR "Unsupport 64-bit timer: %d\r\n", timer LOG_END
            break;
        }
        break;
    }
}

void ithTimerSuspend(void)
{
    int i;
    
    for (i = 0; i < ITH_COUNT_OF(timerRegs); i++)
    {
        switch (i)
        {
        case ITH_TIMER_INTRRAWSTATE_REG:
        case ITH_TIMER_INTRSTATE_REG:
            // don't need to backup
            break;
            
        default:
            timerRegs[i] = ithReadRegA(ITH_TIMER_BASE + ITH_GPIO1_PINDIR_REG + i * 4);
        }
    }
}

void ithTimerResume(void)
{
    int i;
    
    for (i = 0; i < ITH_COUNT_OF(timerRegs); i++)
    {
        switch (i)
        {
        case ITH_TIMER_INTRRAWSTATE_REG:
        case ITH_TIMER_INTRSTATE_REG:
            // don't need to restore
            break;
            
        default:
            ithWriteRegA(ITH_TIMER_BASE + ITH_GPIO1_PINDIR_REG + i * 4, timerRegs[i]);
        }
    }
}
