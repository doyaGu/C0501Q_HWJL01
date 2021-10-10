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
    unsigned int i;
    
    for (i = 0; i < ITH_COUNT_OF(kpGpioTable); i++)
    {
        int pin = kpGpioTable[i];
        uint32_t value = ithGpioGet(pin);
        
        if (pin >= 32)
            pin -= 32;
        
        if ( (value & (0x1 << pin)) == 0 )
            return i;
    }
    return -1;
}

void itpKeypadInit(void)
{
    unsigned int i;
    
    for (i = 0; i < ITH_COUNT_OF(kpGpioTable); i++)
    {
		ithGpioCtrlEnable(kpGpioTable[i], ITH_GPIO_PULL_ENABLE);	//set GPIO pull enable & set pull-down
		
        ithGpioSetIn(kpGpioTable[i]);
        ithGpioEnable(kpGpioTable[i]);
    }
}

int itpKeypadGetMaxLevel(void)
{
    return ITH_COUNT_OF(kpGpioTable);
}
