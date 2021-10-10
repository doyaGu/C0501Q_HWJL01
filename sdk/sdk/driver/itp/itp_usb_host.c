/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL USB Host related code.
 *
 * @author Irene Lin
 * @version 1.0
 */

typedef struct
{
    int     index;
    bool    connected;
    int     type;
    void*   ctxt;  /* usb device's context that currently connected */
} ITPUsb;


#if defined(CFG_MSC_ENABLE)
    #include "itp_usb_msc.c"
#endif

#if defined(CFG_NET_WIFI)
    #include "ite/ite_wifi.h"
#endif


static ITPUsb itpUsb[2] = {{USB0, false, 0, NULL}, {USB1, false, 0, NULL}};

#if defined(CFG_USB_MOUSE)
extern bool usb_mouse_insert;
#endif

static int _CheckUsbState(ITPUsb* itpusb)
{
    static USB_DEVICE_INFO device_info = {0};
    static int usb_state = 0;
    int res;

    res = mmpUsbExCheckDeviceState(itpusb->index, &usb_state, &device_info);
    if(!res)
    {
        if(USB_DEVICE_CONNECT(usb_state))
        {
            itpusb->type = device_info.type;
            itpusb->ctxt = device_info.ctxt;
            itpusb->connected = true;

            #if defined(CFG_UAS_ENABLE)
            UasConnect(itpusb);
            #endif

            #if defined(CFG_MSC_ENABLE)
            MscConnect(itpusb);
            #endif

            #if defined(CFG_NET_WIFI)
            WifiConnect(itpusb->type);
            #endif

            #if defined(CFG_DEMOD_USB_INDEX)
            if(USB_DEVICE_DEMOD(itpusb->type))
            {
                printf(" USB : Demodulator device is interted!! \n");
            }
            #endif

            #if defined(CFG_USB_MOUSE)
            if(USB_DEVICE_MOUSE(itpusb->type))
        	{
                LOG_INFO " USB%d: usb mouse is interted!! \n", itpusb->index LOG_END
				usb_mouse_insert = true;
        	}
            #endif 

            #if defined(CFG_USB_KBD)
            if(USB_DEVICE_KBD(itpusb->type))
                LOG_INFO " USB%d: usb keyboard is interted!! \n", itpusb->index LOG_END
            #endif 
        }
        if(USB_DEVICE_DISCONNECT(usb_state))
        {
            #if defined(CFG_UAS_ENABLE)
            UasDisconnect(itpusb);
            #endif

            #if defined(CFG_MSC_ENABLE)
            MscDisconnect(itpusb);
            #endif

            #if defined(CFG_NET_WIFI)
            WifiDisconnect(itpusb->type);
            #endif

            #if defined(CFG_DEMOD_USB_INDEX)
            if(USB_DEVICE_DEMOD(itpusb->type))
            {
                printf(" USB : Demodulator device is removed!! \n");
            }
            #endif

            #if defined(CFG_USB_MOUSE)
            if(USB_DEVICE_MOUSE(itpusb->type))
        	{
                LOG_INFO " USB%d: usb mouse is disconnected!! \n", itpusb->index LOG_END
				usb_mouse_insert = false;
        	}
            #endif 

            #if defined(CFG_USB_KBD)
            if(USB_DEVICE_KBD(itpusb->type))
                LOG_INFO " USB%d: usb keyboard is disconnected!! \n", itpusb->index LOG_END
            #endif 

            itpusb->connected = false;
            itpusb->ctxt = NULL;
            itpusb->type = 0;
        }

    }

    return res;
}

static void* UsbHostDetectHandler(void* arg)
{
    while(1)
    {
    #if defined(CFG_USB0_ENABLE)
        if(_CheckUsbState(&itpUsb[USB0]))
            LOG_ERR " itp usb0 check state fail! \n" LOG_END
    #endif

    #if defined(CFG_USB1_ENABLE)
        if(_CheckUsbState(&itpUsb[USB1]))
            LOG_ERR " itp usb1 check state fail! \n" LOG_END
    #endif

        usleep(30*1000);
    }
}

static void UsbHostGetInfo(ITPUsbInfo* usbInfo)
{
    usbInfo->ctxt = itpUsb[usbInfo->usbIndex].ctxt;
    usbInfo->type = itpUsb[usbInfo->usbIndex].type;
}
