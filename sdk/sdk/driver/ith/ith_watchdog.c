/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Watch Dog functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static uint32_t wdLoad, wdCR;

void ithWatchDogSuspend(void)
{
    wdLoad  = ithReadRegA(ITH_WD_BASE + ITH_WD_LOAD_REG);
    wdCR    = ithReadRegA(ITH_WD_BASE + ITH_WD_CR_REG);
}

void ithWatchDogResume(void)
{
    ithWriteRegA(ITH_WD_BASE + ITH_WD_LOAD_REG, wdLoad);
    ithWriteRegA(ITH_WD_BASE + ITH_WD_CR_REG, wdCR);
}
