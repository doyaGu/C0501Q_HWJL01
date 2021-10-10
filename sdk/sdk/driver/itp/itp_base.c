/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL base functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <stddef.h>
#include <errno.h>
#include "itp_cfg.h"

const ITPDevice* itpDeviceTable[ITP_DEVICE_MAX];
static ITPIdleHandler itpIdleTable[ITP_MAX_IDLE_HANDLER];

int itpOpenDefault(const char* name, int flags, int mode, void* info)
{
    errno = ENOENT;
    return -1;
}

int itpCloseDefault(int file, void* info)
{
    errno = ENOENT;
    return -1;
}

int itpReadDefault(int file, char *ptr, int len, void* info)
{
    return 0;
}

int itpWriteDefault(int file, char *ptr, int len, void* info)
{
    return len;
}

int itpLseekDefault(int file, int ptr, int dir, void* info)
{
    return 0;
}

int itpIoctlDefault(int file, unsigned long request, void* ptr, void* info)
{
    return 0;
}

void itpRegisterDevice(ITPDeviceType type, const ITPDevice* device)
{
    int index = (type >> ITP_DEVICE_BIT);
    if (index < ITP_DEVICE_MAX)
        itpDeviceTable[index] = device;
}

void itpRegisterIdleHandler(ITPIdleHandler handler)
{
    int i;
    
    for (i = 0; i < ITP_MAX_IDLE_HANDLER; i++)
    {
        if (itpIdleTable[i] == NULL)
        {
            itpIdleTable[i] = handler;
            break;
        }
    }
}

void vApplicationIdleHook( void )
{
    int i;
    
    for (i = 0; i < ITP_MAX_IDLE_HANDLER; i++)
    {
        if (itpIdleTable[i] == NULL)
            break;
            
        itpIdleTable[i]();
    }

#ifdef CFG_POWER_DOZE
    ithCpuDoze();
#endif
}
