/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Headset GPIO functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

static int hsLastValue = 0;

static void HeadsetIntrHandler(unsigned int pin, void* arg)
{
    if (ithGpioGet(CFG_GPIO_HEADSET))
    {
    #ifdef CFG_SPEAKER_ENABLE
        ithGpioSet(CFG_GPIO_SPEAKER);
    #endif
        hsLastValue = 1;
    }
    else
    {
    #ifdef CFG_SPEAKER_ENABLE
        ithGpioClear(CFG_GPIO_SPEAKER);
    #endif
        hsLastValue = 0;
    }

    ithGpioClearIntr(pin);
}

void itpHeadsetInit(void)
{
    ithGpioRegisterIntrHandler(CFG_GPIO_HEADSET, HeadsetIntrHandler, NULL);
    ithGpioEnableIntr(CFG_GPIO_HEADSET);
    ithGpioSetIn(CFG_GPIO_HEADSET);
    ithGpioEnable(CFG_GPIO_HEADSET);

#ifdef CFG_SPEAKER_ENABLE
    ithGpioSetOut(CFG_GPIO_SPEAKER);
    ithGpioEnable(CFG_GPIO_SPEAKER);
    
#endif // CFG_SPEAKER_ENABLE

    if (ithGpioGet(CFG_GPIO_HEADSET))
    {
    #ifdef CFG_SPEAKER_ENABLE
        ithGpioSet(CFG_GPIO_SPEAKER);
    #endif
    
        hsLastValue = 1;
    }
}

int itpHeadsetGetValue(void)
{
    return hsLastValue;
}
