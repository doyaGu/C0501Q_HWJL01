/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Switch button functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

static const unsigned int switchGpioTable[] = { CFG_GPIO_SWITCH };
typedef struct
{
    bool value, lastValue;

} SwitchData;

static SwitchData switchData[ITH_COUNT_OF(switchGpioTable)];

static void SwitchIntrHandler(unsigned int pin, void* arg)
{
    int i;
    
    for (i = 0; i < ITH_COUNT_OF(switchGpioTable); i++)
    {
        if (switchGpioTable[i] == pin)
        {
            SwitchData* data = &switchData[i];

            if (ithGpioGet(pin))
                data->value = true;
            else
                data->value = false;
            break;
        }
    }
    //ithPrintf("ithGpioGet(%d)=0x%X\n", pin, ithGpioGet(pin));
    ithGpioClearIntr(pin);
}

static void SwitchInit(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(switchGpioTable); i++)
    {
        unsigned int pin = switchGpioTable[i];
        SwitchData* data = &switchData[i];

        ithGpioRegisterIntrHandler(pin, SwitchIntrHandler, NULL);
        ithGpioCtrlEnable(pin, ITH_GPIO_INTR_BOTHEDGE);
        ithGpioEnableIntr(pin);
        ithGpioSetIn(pin);
        ithGpioClear(pin);
        ithGpioEnable(pin);

        if (ithGpioGet(pin))
        {
            data->value     = true;
            data->lastValue = false;
        }
        else
        {
            data->value     = false;
            data->lastValue = true;
        }
    }
}

static int SwitchOpen(const char* name, int flags, int mode, void* info)
{
    return atoi(name);
}

static int SwitchClose(int file, void* info)
{
    return 0;
}

static int SwitchRead(int file, char *ptr, int len, void* info)
{
    SwitchData* data = &switchData[file];

    if ((len > 0) && (data->lastValue != data->value))
    {
        *ptr = (char) data->value;
        data->lastValue = data->value;
        return sizeof (char);
    }
    return 0;
}

static int SwitchIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        SwitchInit();
        break;

    default:
        errno = (ITP_DEVICE_SWITCH << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceSwitch =
{
    ":switch",
    SwitchOpen,
    SwitchClose,
    SwitchRead,
    itpWriteDefault,
    itpLseekDefault,
    SwitchIoctl,
    NULL
};
