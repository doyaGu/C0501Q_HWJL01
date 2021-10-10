/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL USB Keyboard related code.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <errno.h>
#include <string.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"
#include "ite/ite_usbkbd.h"


#define QUEUE_LEN 256

#define SCANCODE_LCTRL	224

static QueueHandle_t queue;
static uint8_t old[8];

#define TO_QUEUE(x)  \
    do { \
        if(queue) \
        { \
            if(ithGetCpuMode() == ITH_CPU_SYS) { \
                if(!(xQueueSend(queue, (x), 0)))    \
                    ithPrintf(" Kbd: queue full! \n");  \
            } else {    \
                if(!(xQueueSendFromISR(queue,(x), 0))) \
                    ithPrintf(" Kbd: queue full! \n");  \
            } \
        } \
    } while(0)


static void *memscan(void *addr, int c, uint32_t len)
{
    uint8_t *p = addr;

    while (len) {
        if (*p == c)
            return (void *)p;
        p++;
        len--;
    }
    return (void *)p;
}

static void cb_func(uint8_t *new)
{
    int i=0;
    ITPKeyboardEvent ev;

    for(i=0; i<8; i++)
    {
        if(!((old[0] >> i) & 1) && ((new[0] >> i) & 1))
        {
            ev.flags = ITP_KEYDOWN;
            ev.code = SCANCODE_LCTRL + i;
            TO_QUEUE(&ev);
        }
        if(((old[0] >> i) & 1) && !((new[0] >> i) & 1))
        {
            ev.flags = ITP_KEYUP;
            ev.code = SCANCODE_LCTRL + i;
            TO_QUEUE(&ev);
        }
    }

    for(i=2; i<8; i++)
    {
        if( (old[i] > 3) && (memscan(new+2, old[i], 6) == new+8) )
        {
            ev.flags = ITP_KEYUP;
            ev.code = old[i];
            TO_QUEUE(&ev);
        }
        if( (new[i] > 3) && (memscan(old+2, new[i], 6) == old+8) )
        {
            ev.flags = ITP_KEYDOWN;
            ev.code = new[i];
            TO_QUEUE(&ev);
        }
    }

    memcpy(old, new, 8);
}

static int UsbKbdRead(int file, char *ptr, int len, void* info)
{
    if(!queue)
    {
        printf(" Kbd: queue not created??? \n");
        errno = (ITP_DEVICE_USBKBD << ITP_DEVICE_ERRNO_BIT) | 2;
        return -1;
    }

    if (xQueueReceive(queue, ptr, 0))
        return sizeof (ITPKeyboardEvent);

    return 0;
}

static void UsbKbdInit(void)
{
    queue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(ITPKeyboardEvent));
    if(!queue)
        printf(" Kbd: create queue fail! \n");

    iteUsbKbdSetCb(cb_func);
}

static int UsbKbdIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        UsbKbdInit();
        break;

    default:
        errno = (ITP_DEVICE_USBKBD << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceUsbKbd =
{
    ":usbkbd",
    itpOpenDefault,
    itpCloseDefault,
    UsbKbdRead,
    itpWriteDefault,
    itpLseekDefault,
    UsbKbdIoctl,
    NULL
};

