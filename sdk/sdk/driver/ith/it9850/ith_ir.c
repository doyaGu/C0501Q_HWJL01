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
    printf("	### xy=%x ###\n",(int)xext);
    return (int)xext;
}

void ithIrInit(unsigned int pin, unsigned long extClk, unsigned long sample, unsigned int precision)
{
    unsigned int busClk, prescale;
    unsigned long sampDuration = sample * (1 << precision) / 1000; // sample duration in milliseconds
        
    if (extClk == 0)
    {
        busClk = ithGetBusClock();
        printf("busclk=%d\n",busClk);
    }
    else
    {
        unsigned int outdiv = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) + 4;
        busClk = (extClk / outdiv) << 2;
    }
    printf(" ### IR RX Pin=%d #### \n",pin);
    ithGpioSetMode(pin, ITH_GPIO_MODE0);
    ithIrSetGpio(pin);
    ithIrSetCaptureMode(ITH_IR_RISING);
    ithIrCtrlEnable(ITH_IR_DEBOUNCE);
    ithIrCtrlEnable(ITH_IR_SIGINVESE);
    ithIrCtrlEnable(ITH_IR_TMRST);
    ithIrCtrlEnable(ITH_IR_WARP);
    ithIrClear();

    prescale = MULSHIFT(sampDuration, busClk, precision) / 1000 - 1;
    printf("set IR prescale=%d,sampDuration=%d,busClk=%d,precision=%d\n",prescale,sampDuration,busClk,precision);
    
    ithWriteRegA(ITH_IR_BASE + ITH_IR_CAP_PRESCALE_REG, prescale);
    //while(1);
    irWidth = (ithReadRegA(ITH_IR_BASE + ITH_IR_HWCFG_REG) & ITH_IR_WIDTH_MASK) >> ITH_IR_WIDTH_BIT;
    printf("irWidth=%d\n",irWidth);
}

void ithIrTxInit(unsigned int pin, unsigned long extClk, unsigned long sample, unsigned int precision)
{
    unsigned int busClk, prescale;
    unsigned long sampDuration = sample * (1 << precision) / 1000; // sample duration in milliseconds
        
    if (extClk == 0)
    {
        busClk = ithGetBusClock();
        printf("busclk=%d\n",busClk);
    }
    else
    {
        unsigned int outdiv = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) + 4;
        busClk = (extClk / outdiv) << 2;
    }

    printf(" ### IR Tx Pin=%d #### \n",pin);
    ithGpioSetMode(pin, ITH_GPIO_MODE1);
    //ithIrTxSetCaptureMode(ITH_IRTX_CFG_LENGTH);
    //ithIrTxCtrlEnable(ITH_IR_TX_SIGINVESE);
    ithIrTxClear();
    
    //ithWriteRegMaskA(ITH_IR_BASE + ITH_IRTX_CAP_CTRL_REG, 1<<1, 1<<1 );	//enable TX loop-back RX mode
		
    //while(1);
    //set IR-TX clock
    prescale = MULSHIFT(sampDuration, busClk, precision) / 1000 - 1;
    printf("set IR prescale=%x,sampDuration=%x,busClk=%x,precision=%x\n",prescale,sampDuration,busClk,precision);
    
    ithWriteRegA(ITH_IR_BASE + ITH_IRTX_CAP_PRESCALE_REG, prescale);
    
    irWidth = (ithReadRegA(ITH_IR_BASE + ITH_IR_HWCFG_REG) & ITH_IR_WIDTH_MASK) >> ITH_IR_WIDTH_BIT;
    printf("irTxWidth=%d\n",irWidth);
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
