/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Keypad functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"

#define QUEUE_LEN 256

static QueueHandle_t kpQueue;
static ITPKeypadEvent kpLastEvent;

static void KeypadProbe(void)
{
    ITPKeypadEvent ev;
    
    ev.code = itpKeypadProbe();
    gettimeofday(&ev.time, NULL);
    
    if (ev.code == -1)
    {
        if (kpLastEvent.code == -1)
        {
            return;
        }
        else
        {
            ev.code = kpLastEvent.code;
            ev.flags = ITP_KEYPAD_UP;
            kpLastEvent.code = -1;
            kpLastEvent.time = ev.time;
        }
    }
    else
    {
        long msec = itpTimevalDiff(&kpLastEvent.time, &ev.time);

        if ((kpLastEvent.code == ev.code) && (kpLastEvent.flags & ITP_KEYPAD_DOWN))
        {
            if (msec < CFG_KEYPAD_PRESS_INTERVAL)
                return;

        #ifndef CFG_KEYPAD_REPEAT
            kpLastEvent.time = ev.time;
            return;
        #endif // !CFG_KEYPAD_REPEAT

            ev.flags = ITP_KEYPAD_DOWN | ITP_KEYPAD_REPEAT;
        }
        else
        {
            ev.flags = ITP_KEYPAD_DOWN;
            kpLastEvent.code = ev.code;
        }
    }
    kpLastEvent.flags = ev.flags;
    kpLastEvent.time = ev.time;

    xQueueSend(kpQueue, &ev, 0);
}

#if CFG_KEYPAD_PROBE_INTERVAL > 0

static void KeypadProbeHandler(timer_t timerid, int arg)
{
    KeypadProbe();
}
#endif // CFG_KEYPAD_PROBE_INTERVAL > 0

static void KeypadInit(void)
{
    int i;

    kpLastEvent.code = -1;
    kpQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(ITPKeypadEvent));

    itpKeypadInit();

#if CFG_KEYPAD_PROBE_INTERVAL > 0
    {
        timer_t timer;
        struct itimerspec value;
        timer_create(CLOCK_REALTIME, NULL, &timer);
        timer_connect(timer, (VOIDFUNCPTR)KeypadProbeHandler, 0);
        value.it_value.tv_sec = value.it_interval.tv_sec = 0;
        value.it_value.tv_nsec = value.it_interval.tv_nsec = CFG_KEYPAD_PROBE_INTERVAL * 1000000;
        timer_settime(timer, 0, &value, NULL);
    }
#endif // CFG_KEYPAD_PROBE_INTERVAL > 0
}

static int KeypadRead(int file, char *ptr, int len, void* info)
{
    if (xQueueReceive(kpQueue, ptr, 0))
        return sizeof (ITPKeypadEvent);

    return 0;
}

static int KeypadIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_PROBE:
        KeypadProbe();
        break;
        
    case ITP_IOCTL_INIT:
        KeypadInit();
        break;

    case ITP_IOCTL_GET_MAX_LEVEL:
        return itpKeypadGetMaxLevel();
        break;

    default:
        errno = (ITP_DEVICE_KEYPAD << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceKeypad =
{
    ":keypad",
    itpOpenDefault,
    itpCloseDefault,
    KeypadRead,
    itpWriteDefault,
    itpLseekDefault,
    KeypadIoctl,
    NULL
};
