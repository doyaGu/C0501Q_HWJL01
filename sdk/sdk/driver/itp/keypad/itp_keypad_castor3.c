/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor3 keypad module.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

static const unsigned int kpGpioTable[] = { CFG_GPIO_KEYPAD };

int itpKeypadProbe(void)
{
    return ithKeypadProbe();
}

void itpKeypadInit(void)
{
    ithKeypadInit(ITH_COUNT_OF(kpGpioTable), (unsigned int*)kpGpioTable);
    ithKeypadEnable();
}

int itpKeypadGetMaxLevel(void)
{
    int i, value = ITH_COUNT_OF(kpGpioTable);
    
    for (i = 0; i < ITH_COUNT_OF(kpGpioTable); i++)
        value += ITH_COUNT_OF(kpGpioTable) - 1;

    return value - 1;
}
