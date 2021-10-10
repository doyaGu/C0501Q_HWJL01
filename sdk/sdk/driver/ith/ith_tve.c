/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL TV Encoder functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

void ithTveReset(void)
{
    ithSetRegBitH(ITH_TVE_CLK2_REG, ITH_TVE_ERST_BIT);
    ithClearRegBitH(ITH_TVE_CLK2_REG, ITH_TVE_ERST_BIT);
}

void ithTveEnableClock(void)
{
    // enable clock
    ithSetRegBitH(ITH_TVE_CLK2_REG, ITH_EN_ECLK_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_TVE_MMIO_BIT);
}

void ithTveDisableClock(void)
{
    // disable clock
    ithClearRegBitH(ITH_TVE_CLK2_REG, ITH_EN_ECLK_BIT);
    ithClearRegBitH(ITH_EN_MMIO_REG, ITH_EN_TVE_MMIO_BIT);
}

void ithTveEnablePower(void)
{
    ithWriteRegH(ITH_TVE_DAC_OUT_REG, 0x0);
    while (ithReadRegH(ITH_TVE_DAC_OUT_REG) != 0x0);
}

void ithTveDisablePower(void)
{
    ithWriteRegH(ITH_TVE_DAC_OUT_REG, 0xFFFF);
    while (ithReadRegH(ITH_TVE_DAC_OUT_REG) != 0xFFFF);
}
