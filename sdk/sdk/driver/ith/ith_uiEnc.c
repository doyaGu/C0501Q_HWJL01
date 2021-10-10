/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL UIEnc functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

void ithUiEncEnableClock(void)
{
    // enable clock
    ithSetRegBitH(ITH_UIENC_CLK_REG, ITH_EN_M14CLK_BIT);
}

void ithUiEncDisableClock(void)
{
    // disable clock
    ithClearRegBitH(ITH_UIENC_CLK_REG, ITH_EN_M14CLK_BIT);
}

void ithUiEncResetReg(void)
{

}

void ithUiEncResetEngine(void)
{
    ithSetRegBitH(ITH_UIENC_CLK_REG, ITH_EN_UIENC_RST_BIT);
    ithClearRegBitH(ITH_UIENC_CLK_REG, ITH_EN_UIENC_RST_BIT);
}
