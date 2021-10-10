/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL interrupt functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static uint32_t intr1Regs[(ITH_INTR_FIQ1_SWINTR_REG - ITH_INTR_IRQ1_EN_REG + 4) / 4];
static uint32_t intr2Regs[(ITH_INTR_FIQ2_SWINTR_REG - ITH_INTR_IRQ2_EN_REG + 4) / 4];

void ithIntrReset(void)
{
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, 0xFFFFFFFF);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGMODE_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGLEVEL_REG, 0);

	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_CLR_REG, 0xFFFFFFFF);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGMODE_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGLEVEL_REG, 0);

	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, 0xFFFFFFFF);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGMODE_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGLEVEL_REG, 0);

	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_CLR_REG, 0xFFFFFFFF);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGMODE_REG, 0);
	ithWriteRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGLEVEL_REG, 0);
}

void ithIntrSetTriggerModeIrq(ITHIntr intr, ITHIntrTriggerMode mode)
{
    ithEnterCritical();

    if (mode)
    {
        if (intr < 32)
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGMODE_REG, intr);
        else
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGMODE_REG, intr - 32);
    }
    else
    {
        if (intr < 32)
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGMODE_REG, intr);
        else
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGMODE_REG, intr - 32);
    }
    
    ithExitCritical();
}

void ithIntrSetTriggerLevelIrq(ITHIntr intr, ITHIntrTriggerLevel level)
{
    ithEnterCritical();

    if (level)
    {
        if (intr < 32)
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGLEVEL_REG, intr);
        else
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGLEVEL_REG, intr - 32);
    }
    else
    {
        if (intr < 32)
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_TRIGLEVEL_REG, intr);
        else
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_TRIGLEVEL_REG, intr - 32);
    }
    
    ithExitCritical();
}

void ithIntrSetTriggerModeFiq(ITHIntr intr, ITHIntrTriggerMode mode)
{
    ithEnterCritical();

    if (mode)
    {
        if (intr < 32)
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGMODE_REG, intr);
        else
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGMODE_REG, intr - 32);
    }
    else
    {
        if (intr < 32)
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGMODE_REG, intr);
        else
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGMODE_REG, intr - 32);
    }
    
    ithExitCritical();
}

void ithIntrSetTriggerLevelFiq(ITHIntr intr, ITHIntrTriggerLevel level)
{
    ithEnterCritical();

    if (level)
    {
        if (intr < 32)
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGLEVEL_REG, intr);
        else
            ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGLEVEL_REG, intr - 32);
    }
    else
    {
        if (intr < 32)
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_TRIGLEVEL_REG, intr);
        else
            ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_TRIGLEVEL_REG, intr - 32);
    }
    
    ithExitCritical();
}

void ithIntrSuspend(void)
{
    int i;
    
    for (i = 0; i < ITH_COUNT_OF(intr1Regs); i++)
    {
        switch (i)
        {
        case ITH_INTR_IRQ1_CLR_REG:
        case ITH_INTR_IRQ1_STATUS_REG:
        case ITH_INTR_FIQ1_SRC_REG:
        case ITH_INTR_FIQ1_CLR_REG:
        case ITH_INTR_FIQ1_STATUS_REG:
            // don't need to backup
            break;
            
        default:
            intr1Regs[i] = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG + i * 4);
        }
    }

    for (i = 0; i < ITH_COUNT_OF(intr2Regs); i++)
    {
        switch (i)
        {
        case ITH_INTR_IRQ2_CLR_REG:
        case ITH_INTR_IRQ2_STATUS_REG:
        case ITH_INTR_FIQ2_SRC_REG:
        case ITH_INTR_FIQ2_CLR_REG:
        case ITH_INTR_FIQ2_STATUS_REG:
            // don't need to backup
            break;
            
        default:
            intr2Regs[i] = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG + i * 4);
        }
    }
}

void ithIntrResume(void)
{
    int i;
    
    for (i = 0; i < ITH_COUNT_OF(intr1Regs); i++)
    {
        switch (i)
        {
        case ITH_INTR_IRQ1_CLR_REG:
        case ITH_INTR_IRQ1_STATUS_REG:
        case ITH_INTR_FIQ1_SRC_REG:
        case ITH_INTR_FIQ1_CLR_REG:
        case ITH_INTR_FIQ1_STATUS_REG:
            // don't need to restore
            break;
            
        default:
            ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG + i * 4, intr1Regs[i]);
        }
    }

    for (i = 0; i < ITH_COUNT_OF(intr2Regs); i++)
    {
        switch (i)
        {
        case ITH_INTR_IRQ2_CLR_REG:
        case ITH_INTR_IRQ2_STATUS_REG:
        case ITH_INTR_FIQ2_SRC_REG:
        case ITH_INTR_FIQ2_CLR_REG:
        case ITH_INTR_FIQ2_STATUS_REG:
            // don't need to restore
            break;
            
        default:
            ithWriteRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG + i * 4, intr2Regs[i]);
        }
    }
}

void ithIntrStats(void)
{
    PRINTF("Interrupt:\r\n");
    ithPrintRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_SRC_REG, ITH_INTR_FIQ2_SWINTR_REG - ITH_INTR_IRQ1_SRC_REG + sizeof (uint32_t));
}
