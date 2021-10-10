/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL IR functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static unsigned int irWidth;

static inline int MULSHIFT(int x, int y, int shift)
{
    int64_t xext, yext;
    xext = (int64_t)x;
    yext = (int64_t)y;
    xext = ((xext * yext) >> shift);
    return (int)xext;
}

void ithIrInit(unsigned int pin, unsigned long extClk, unsigned long sample, unsigned int precision)
{
    unsigned int busClk, prescale;
    unsigned long sampDuration = sample * (1 << precision) / 1000; // sample duration in milliseconds
        
    if (extClk == 0)
    {
        busClk = ithGetBusClock();
    }
    else
    {
        unsigned int outdiv = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) + 4;
        busClk = (extClk / outdiv) << 2;
    }

    ithGpioSetMode(pin, ITH_GPIO_MODE1);
    ithIrSetCaptureMode(ITH_IR_RISING);
    ithIrCtrlEnable(ITH_IR_DEBOUNCE);
    ithIrCtrlEnable(ITH_IR_SIGINVESE);
    ithIrCtrlEnable(ITH_IR_TMRST);
    ithIrCtrlEnable(ITH_IR_WARP);
    ithIrClear();

    prescale = MULSHIFT(sampDuration, busClk, precision) / 1000 - 1;
    ithWriteRegA(ITH_IR_BASE + ITH_IR_CAP_PRESCALE_REG, prescale);
    
    irWidth = (ithReadRegA(ITH_IR_BASE + ITH_IR_HWCFG_REG) & ITH_IR_WIDTH_MASK) >> ITH_IR_WIDTH_BIT;
}

int ithIrProbe(void)
{
    uint32_t status = ithReadRegA(ITH_IR_BASE + ITH_IR_CAP_STATUS_REG);
    if (status & (0x1 << ITH_IR_DATAREADY_BIT))
        return ithReadRegA(ITH_IR_BASE + ITH_IR_CAP_DATA_REG) & ((0x1 << irWidth) - 1);
    else
    {     
        if(status & (0x1 << ITH_IR_OE_BIT))
            LOG_WARN "IR overrun error: 0x%X\n", status LOG_END

        return -1;
    }
}
