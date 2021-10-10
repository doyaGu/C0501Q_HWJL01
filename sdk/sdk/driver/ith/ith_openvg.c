/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL OpenVG functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

void ithOvgEnableClock(void)
{
    // enable clock
    ithSetRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_N9CLK_BIT);
    ithSetRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_M16CLK_BIT);
    ithSetRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_VCLK_BIT);
    ithSetRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_TCLK_BIT);
}

void ithOvgDisableClock(void)
{
    // disable clock
    ithClearRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_N9CLK_BIT);
    ithClearRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_M16CLK_BIT);
    ithClearRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_VCLK_BIT);
    ithClearRegBitH(ITH_OPVG_CLK1_REG, ITH_EN_TCLK_BIT);
}
