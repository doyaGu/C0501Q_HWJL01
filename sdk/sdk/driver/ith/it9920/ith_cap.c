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
    ithSetRegBitA(ITH_CAP_CLK_REG, ITH_EN_M12CLK_BIT);
	ithSetRegBitA(ITH_CAP_CLK_REG, ITH_EN_W18CLK_BIT);
	ithSetRegBitA(ITH_CAP_CLK_REG, ITH_EN_DIV_CAPCLK_BIT);
}

void ithCapDisableClock(void)
{
    // disable clock
    ithClearRegBitA(ITH_CAP_CLK_REG, ITH_EN_M12CLK_BIT);
	ithClearRegBitA(ITH_CAP_CLK_REG, ITH_EN_W18CLK_BIT);
	ithClearRegBitA(ITH_CAP_CLK_REG, ITH_EN_DIV_CAPCLK_BIT);
}

void ithCapResetReg(void)
{
	ithSetRegBitA(ITH_CAP_CLK_REG, ITH_EN_CAP_REG_RST_BIT);
	ithClearRegBitA(ITH_CAP_CLK_REG, ITH_EN_CAP_REG_RST_BIT);
}

void ithCapResetEngine(void)
{
    ithSetRegBitH(ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
    ithClearRegBitH(ITH_CAP_CLK_REG, ITH_EN_CAPC_RST_BIT);
}
