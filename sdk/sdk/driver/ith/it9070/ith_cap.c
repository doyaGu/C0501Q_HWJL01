/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL CAPTURE Controllor functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../ith_cfg.h"

void ithCapEnableClock(void)
{
    // enable clock
    ithSetRegBitH(ITH_CAP_CLK_REG, ITH_EN_M17CLK_BIT);
}

void ithCapDisableClock(void)
{
    // disable clock
    ithClearRegBitH(ITH_CAP_CLK_REG, ITH_EN_M17CLK_BIT);
}

void ithCapResetReg(void)
{

}

void ithCapResetEngine(void)
{
    ithSetRegBitH(ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
    ithClearRegBitH(ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
}
