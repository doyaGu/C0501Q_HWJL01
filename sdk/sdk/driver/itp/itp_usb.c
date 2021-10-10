/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL USB functions.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <pthread.h>
#include <errno.h>
#include "itp_cfg.h"
#include "ite/ite_usbex.h"

#include "itp_usb_host.c"

#if defined(CFG_USB_DEVICE)
#include "itp_usb_device.c"
#endif

#if defined(CFG_UAS_ENABLE)
#include "ite/ite_uas.h"
#endif

#if defined(CFG_NET_WIFI)
#include "ite/ite_wifi.h"
#endif

#if defined(CFG_USB_MOUSE)
#include "ite/ite_usbmouse.h"
#endif

#if defined(CFG_USB_KBD)
#include "ite/ite_usbkbd.h"
#endif

#if defined(__OPENRTOS__)
#include "openrtos/FreeRTOSConfig.h"
#else
#define configMAX_PRIORITIES       50
#endif

#define USBH_TASK_PRIORITY              5
#define USBH_STACK_SIZE                 10*1024

static int usb_init_ok = 0;
static pthread_t usb_init_task;

static void* UsbInitTask(void* arg)
{
    for (;;)
    {
        if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_INIT, NULL) == 0) /* success */
        {
            #ifdef CFG_USB_MOUSE
            itpRegisterDevice(ITP_DEVICE_USBMOUSE, &itpDeviceUsbMouse);
            ioctl(ITP_DEVICE_USBMOUSE, ITP_IOCTL_INIT, NULL);
            #endif
            #ifdef CFG_USB_KBD
            itpRegisterDevice(ITP_DEVICE_USBKBD, &itpDeviceUsbKbd);
            ioctl(ITP_DEVICE_USBKBD, ITP_IOCTL_INIT, NULL);
            #endif
            #ifdef CFG_NET_WIFI
            itpRegisterDevice(ITP_DEVICE_WIFI, &itpDeviceWifi);
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
            #endif
            return NULL;
        }
        usleep(1 * 1000 * 1000);
    }

    return NULL;
}


static int UsbInit(void)
{
    int res;
	int usb_en = 0;

    if (usb_init_ok)
        return 0;

#if defined(CFG_USB_DEVICE)
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    ithGpioSetMode(CFG_GPIO_USB_ID_PIN, ITH_GPIO_MODE1);
    #if defined(CFG_USB_DEVICE_USB0)
	ithWriteRegMaskH(0x900, 0x1, 0x1);
    #endif // #if defined(CFG_USB_DEVICE_USB0)
#else // 9850
	{
	    uint16_t val = CFG_GPIO_USB_ID_PIN;
		uint16_t mask = 0x7F;

		ithGpioSetMode(CFG_GPIO_USB_ID_PIN, ITH_GPIO_MODE0);
		ithGpioSetIn(CFG_GPIO_USB_ID_PIN);
	    #if defined(CFG_USB_DEVICE_USB1)
		mask = mask << 8;
		val = val << 8;
	    #endif // #if defined(CFG_USB_DEVICE_USB0)
		ithWriteRegMaskH(0x902, val, mask);
	}
#endif // #if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
#endif // #if defined(CFG_USB_DEVICE)
	

#if defined(CFG_USB0_ENABLE)
	usb_en |= (1 << 0);
#endif
#if defined(CFG_USB1_ENABLE)
	usb_en |= (1 << 1);
#endif
    res = mmpUsbExInitialize(usb_en);
    if (res)
    {
        if (!usb_init_task)
        {
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            pthread_create(&usb_init_task, &attr, UsbInitTask, NULL);
        }
        return res;
    }
    usb_init_ok = 1;

#if defined(CFG_UAS_ENABLE)
    res = iteUasDriverRegister();
    if(res)
        LOG_ERR " iteUasDriverRegister() res 0x%08X \n", res LOG_END
#endif

#if defined(CFG_MSC_ENABLE)
    res = iteMscDriverRegister();
    if(res)
        LOG_ERR " iteMscDriverRegister() res 0x%08X \n", res LOG_END
#endif

#if defined(CFG_USB_MOUSE)
    res = iteUsbMouseRegister();
    if(res)
        LOG_ERR " iteUsbMouseRegister() res 0x%08X \n", res LOG_END
#endif

#if defined(CFG_USB_KBD)
    res = iteUsbKbdRegister();
    if(res)
        LOG_ERR " iteUsbKbdRegister() res 0x%08X \n", res LOG_END
#endif

#if defined(CFG_DEMOD_ENABLE)
    {
        extern int iteUsbDemodDriverRegister(void);
        res = iteUsbDemodDriverRegister();
        if(res)
            return res;
    }
#endif

#if defined(CFG_NET_WIFI)

	//================= Should add this in Initialize() ======================
	printf("mmpRtlWifiDriverRegister\n");
	res = mmpRtlWifiDriverRegister();
	if(res)
	{
		printf( " mmpRtWifiDriverRegister() return 0x%08X \n", res );
	}
#endif

#if defined(CFG_UVC_ENABLE)
    res = iteUvcDriverRegister();
    if(res)
    {
        LOG_ERR " iteMscDriverRegister() res 0x%08X \n", res LOG_END
    }
#endif

    /* create usb host tasklet thread */
    {
        pthread_t task;
        pthread_attr_t attr;
        struct sched_param param;
        pthread_attr_init(&attr);
        //pthread_attr_setstacksize(&attr, USBH_STACK_SIZE);
        param.sched_priority = USBH_TASK_PRIORITY;
        pthread_attr_setschedparam(&attr, &param);
        res = pthread_create(&task, &attr, USBEX_ThreadFunc, NULL);
        if(res)
            LOG_ERR " create usb host thread fail! 0x%08X \n", res LOG_END
    }

    /* detect usb device insert into our usb host or not */
    {
        pthread_t task;
        pthread_attr_t attr;
        struct sched_param param;

        pthread_attr_init(&attr);
        //pthread_attr_setstacksize(&attr, USBH_STACK_SIZE*10);
        param.sched_priority = USBH_TASK_PRIORITY-1;
        pthread_attr_setschedparam(&attr, &param);
        res = pthread_create(&task, &attr, UsbHostDetectHandler, NULL);
        if(res)
            LOG_ERR " create usb host detect thread fail! 0x%08X \n", res LOG_END
    }
#if defined(CFG_USB_DEVICE)
    {
        pthread_t task;
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        res = pthread_create(&task, &attr, UsbDeviceDetectHandler, NULL);
        if (res)
            LOG_ERR " create usb device detect thread fail! 0x%08X \n", res LOG_END
    }
#endif

    return res;
}

static void UsbGetInfo(ITPUsbInfo* usbInfo)
{
    if(usbInfo->host)
        UsbHostGetInfo(usbInfo);
#if defined(CFG_USB_DEVICE)
	else
		usbInfo->b_device = iteOtgIsDeviceMode();
#endif
}

static int UsbIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int res;

    switch (request)
    {
    case ITP_IOCTL_IS_AVAIL:
        return usb_init_ok;

    case ITP_IOCTL_INIT:
        res = UsbInit();
        if(res)
        {
            errno = (ITP_DEVICE_USB << ITP_DEVICE_ERRNO_BIT) | res;
            return -1;
        }
        break;

    case ITP_IOCTL_GET_INFO:
        //printf(" usb ioctl get info \n");
        UsbGetInfo((ITPUsbInfo*)ptr);
        break;

    case ITP_IOCTL_IS_CONNECTED:
        return iteUsbExIsPortConnect((int)ptr);

    default:
        errno = (ITP_DEVICE_USB << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceUsb =
{
    ":usb",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    UsbIoctl,
    NULL
};


