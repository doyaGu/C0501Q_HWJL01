/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL G-Sensor GPIO functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

static const unsigned int gsGpioTable[] = { CFG_GPIO_GSENSOR };

static int gsLastValue = 0;

static void GSensorIntrHandler(unsigned int pin, void* arg)
{
    int i;
    
    for (i = 0; i < ITH_COUNT_OF(gsGpioTable); i++)
    {
        if (gsGpioTable[i] == pin)
        {
            if (ithGpioGet(pin))
                gsLastValue |= 0x1 << i;
            else
                gsLastValue &= ~(0x1 << i);
            break;
        }
    }
    //ithPrintf("ithGpioGet(%d)=0x%X\n", pin, ithGpioGet(pin));
    ithGpioClearIntr(pin);
}

void itpGSensorInit(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(gsGpioTable); i++)
    {
        unsigned int pin = gsGpioTable[i];
        ithGpioRegisterIntrHandler(pin, GSensorIntrHandler, NULL);
        ithGpioCtrlEnable(pin, ITH_GPIO_INTR_BOTHEDGE);
        ithGpioEnableIntr(pin);
        ithGpioSetIn(pin);
        ithGpioEnable(pin);
    }

    gsLastValue = 0;
    for (i = 0; i < ITH_COUNT_OF(gsGpioTable); i++)
    {
        if (ithGpioGet(gsGpioTable[i]))
            gsLastValue |= 0x1 << i;
    }
}

int itpGSensorGetValue(void)
{
    return gsLastValue;
}
