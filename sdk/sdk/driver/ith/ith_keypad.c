/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL keypad functions.
 *
 * key count algorithm:
 * 1 + 0 = 1
 * 2 + 1 + 1 = 4 
 * 3 + 2 + 2 + 2 = 9
 * 4 + 3 + 3 + 3 + 3 = 16
 * ...
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static int kpPinCount, kpPins[8];

static int KeyProbeFirst(void)
{
    int i;

    for (i = 0; i < kpPinCount; i++)
    {
        int pin = kpPins[i];
        uint32_t value = ithGpioGet(pin);
        
        if (pin >= 32)
            pin -= 32;
        
        if ((value & (0x1 << pin)) == 0)
            return i;
    }
    return -1;
}

static int KeyProbeNext(unsigned int index)
{
    int i, j, key = -1;
    int pin = kpPins[index];
        
    ithGpioSetOut(pin);
    ithGpioClear(pin);

    j = 0;
    for (i = 0; i < kpPinCount; i++)
    {
        uint32_t value;
        
        if (i == index)
            continue;

        pin = kpPins[i];
        value = ithGpioGet(pin);

        if (pin >= 32)
            pin -= 32;

        if ((value & (0x1 << pin)) == 0)
        {
            key = kpPinCount + (kpPinCount - 1) * index  + j;
            break;
        }
        j++;
    }

    // reset to input pin
    ithGpioSetIn(kpPins[index]);

    return key;
}

void ithKeypadInit(unsigned int pinCount, unsigned int* pinArray)
{
    unsigned int i;
    
    for (i = 0; i < pinCount; i++)
    {
        unsigned int pin = pinArray[i];
        kpPins[i] = pin;
    }

    kpPinCount = pinCount;
}

void ithKeypadEnable(void)
{
    int i;
    
    for (i = 0; i < kpPinCount; i++)
    {
        ithGpioSetIn(kpPins[i]);
        ithGpioEnable(kpPins[i]);
    }
}

int ithKeypadProbe(void)
{
    int i;
    
    int key = KeyProbeFirst();
    if (key != -1)
        return key;
    
    for (i = 0; i < kpPinCount; i++)
    {
        key = KeyProbeNext(i);
        if (key != -1)
            return key;
    }

    return -1;
}
