/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL USB Mouse related code.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <errno.h>
#include <string.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"
#include "ite/ite_usbmouse.h"


#define QUEUE_LEN 256

static QueueHandle_t queue;
static ITPMouseEvent ev;
static ITPMouseEvent last_ev;

bool usb_mouse_insert = false;



static void cb_func(int8_t *data)
{
	int i=0;
    memcpy((void*)&ev, data, sizeof(ITPMouseEvent));

	if(last_ev.flags & 0x0F)
	{
		for(i=0; i<4; i++)
		{
			if((last_ev.flags & (1<<i)) && !(ev.flags & (1<<i)))
				ev.flags |= (1<<(i+4));
		}
	}

    if(queue)
    {
        if(ithGetCpuMode() == ITH_CPU_SYS)
        {
            if(!(xQueueSend(queue, &ev, 0)))
                ithPrintf(" Mouse: queue full! \n");
        }
        else
        {
            if(!(xQueueSendFromISR(queue, &ev, 0)))
                ithPrintf(" Mouse: queue full! \n");
        }
    }
    memcpy((void*)&last_ev, (void*)&ev, sizeof(ITPMouseEvent));
}

static int UsbMouseRead(int file, char *ptr, int len, void* info)
{
    if(!queue)
    {
        printf(" Mouse: queue not created??? \n");
        errno = (ITP_DEVICE_USBMOUSE << ITP_DEVICE_ERRNO_BIT) | 2;
        return -1;
    }

    if (xQueueReceive(queue, ptr, 0))
        return sizeof (ITPMouseEvent);

    return 0;
}

static void UsbMouseInit(void)
{
    queue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(ITPMouseEvent));
    if(!queue)
        printf(" Mouse: create queue fail! \n");

    iteUsbMouseSetCb(cb_func);
}

static int UsbMouseIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        UsbMouseInit();
        break;

    case ITP_IOCTL_IS_AVAIL:
        return usb_mouse_insert;

    default:
        errno = (ITP_DEVICE_USBMOUSE << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceUsbMouse =
{
    ":usbmouse",
    itpOpenDefault,
    itpCloseDefault,
    UsbMouseRead,
    itpWriteDefault,
    itpLseekDefault,
    UsbMouseIoctl,
    NULL
};

