/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Battery GPIO functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"

static const unsigned int batGpioTable[] = { CFG_GPIO_BATTERY };
static ITPBatteryState batState = ITP_BATTERY_UNKNOWN;
static int batMaxValue = 0, batLastValue = 0, batStep = 0;

static void BatteryIntrHandler(unsigned int pin, void* arg)
{
#ifdef CFG_BATTERY_CHARGE_DETECT
    if (pin == CFG_GPIO_BATTERY_CHARGE_DETECT)
    {
        if (ithGpioGet(CFG_GPIO_BATTERY_CHARGE_DETECT))
        {
            batState = ITP_BATTERY_ON_BATTERY;
        }
        else
        {
            batState = (batMaxValue == batLastValue) ? ITP_BATTERY_CHARGED : ITP_BATTERY_CHARGING;
        }
    }
    else
#endif // CFG_BATTERY_CHARGE_DETECT
    {
        int i;
        
        for (i = 0; i < ITH_COUNT_OF(batGpioTable); i++)
        {
            if (batGpioTable[i] == pin)
            {
                if (ithGpioGet(pin))
                    batLastValue |= 0x1 << i;
                else
                    batLastValue &= ~(0x1 << i);
                break;
            }
        }
    }
    //ithPrintf("ithGpioGet(%d)=0x%X, batLastValue=0x%X\n", pin, ithGpioGet(pin), batLastValue);
    ithGpioClearIntr(pin);
}

void itpBatteryInit(void)
{
    int i;

#ifdef CFG_BATTERY_CHARGE_DETECT
    ithGpioRegisterIntrHandler(CFG_GPIO_BATTERY_CHARGE_DETECT, BatteryIntrHandler, NULL);
    ithGpioCtrlEnable(CFG_GPIO_BATTERY_CHARGE_DETECT, ITH_GPIO_INTR_BOTHEDGE);
    ithGpioEnableIntr(CFG_GPIO_BATTERY_CHARGE_DETECT);
    ithGpioSetIn(CFG_GPIO_BATTERY_CHARGE_DETECT);
    ithGpioEnable(CFG_GPIO_BATTERY_CHARGE_DETECT);
        
#endif // CFG_BATTERY_CHARGE_DETECT

    //ithGpioSetDebounceClock(32983);

    for (i = 0; i < ITH_COUNT_OF(batGpioTable); i++)
    {
        unsigned int pin = batGpioTable[i];
        ithGpioRegisterIntrHandler(pin, BatteryIntrHandler, NULL);
        //ithGpioEnableBounce(pin);
        ithGpioEnableIntr(pin);
        ithGpioSetIn(pin);
        ithGpioEnable(pin);
    }

    // get init value
    batMaxValue = batLastValue = 0;
    for (i = 0; i < ITH_COUNT_OF(batGpioTable); i++)
    {
        batMaxValue |= 0x1 << i;
        
        if (ithGpioGet(batGpioTable[i]))
            batLastValue |= 0x1 << i;
    }
    
    batStep = 1;
    for (i = 0; i < ITH_COUNT_OF(batGpioTable); i++)
        batStep *= 2;

    batStep = 100 / batStep;

#ifdef CFG_BATTERY_CHARGE_DETECT
    if (ithGpioGet(CFG_GPIO_BATTERY_CHARGE_DETECT))
    {
        batState = ITP_BATTERY_ON_BATTERY;
    }
    else
    {
        batState = (batMaxValue == batLastValue) ? ITP_BATTERY_CHARGED : ITP_BATTERY_CHARGING;
    }
#endif // CFG_BATTERY_CHARGE_DETECT
}

ITPBatteryState itpBatteryGetState(void)
{
    return batState;
}

int itpBatteryGetPercent(void)
{
    //ithPrintf("ithGpioGet(%d)=0x%X, ithGpioGet(%d)=0x%X, batLastValue=%d\n", batGpioTable[0], ithGpioGet(batGpioTable[0]), batGpioTable[1], ithGpioGet(batGpioTable[1]), batLastValue);

    if (batMaxValue == batLastValue)
    {
        return 100;
    }
    else
    {
        return batStep * (batLastValue + 1);
    }
}
