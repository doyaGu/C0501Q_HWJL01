/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Amplifier GPIO functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

void itpAmplifierInit(void)
{
    ithGpioSetOut(CFG_GPIO_AMPLIFIER_ENABLE);
    ithGpioEnable(CFG_GPIO_AMPLIFIER_ENABLE);

    ithGpioSetOut(CFG_GPIO_AMPLIFIER_MUTE);
    ithGpioEnable(CFG_GPIO_AMPLIFIER_MUTE);
}

void itpAmplifierEnable(void)
{
    ithGpioSet(CFG_GPIO_AMPLIFIER_ENABLE);
}

void itpAmplifierDisable(void)
{
    ithGpioClear(CFG_GPIO_AMPLIFIER_ENABLE);
}

void itpAmplifierMute(void)
{
    ithGpioSet(CFG_GPIO_AMPLIFIER_MUTE);
}

void itpAmplifierUnmute(void)
{
    ithGpioClear(CFG_GPIO_AMPLIFIER_MUTE);
}
