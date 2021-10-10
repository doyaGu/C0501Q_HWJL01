/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file usb.c
 * Driver for USB Tuner compliant devices.
 *
 * @author Barry Wu
 */
//=============================================================================
//                              Include Files
//=============================================================================

#include "usb_demod.h"
#include "usb_demod_transport.h"
#include "usb/ite_usb.h"
//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                   Gloabl Data
//=============================================================================
/** currently support 2 context for two USB host controller each */
static struct usbtuner_data ut_list[2];

static struct usb_device_id tuner_usb_ids[] = 
{
    { USB_INTERFACE_INFO(USB_CLASS_VENDOR_SPEC, 0, 0) }
};


//=============================================================================
//                   Private function definition
//=============================================================================

//=============================================================================
//                   Public function definition
//=============================================================================
static void* usb_tuner_probe(struct usb_device *dev, uint32_t intf, const struct usb_device_id *id)
{
    const int id_index = id - tuner_usb_ids;
    struct usbtuner_data* ss = 0;
    struct usb_interface *usb_interface = dev->actconfig->usb_interface + intf;
    struct usb_interface_descriptor *altsetting = usb_interface->altsetting + usb_interface->act_altsetting;
    uint32_t i = 0;

    if(id_index >= sizeof(tuner_usb_ids)/sizeof(tuner_usb_ids[0]))
        goto end;

    dem_msg(1, "......... USB Tuner device detected!\n");

    /** decide the context */
    if(!ut_list[0].pusb_dev)
        ss = &ut_list[0];
    else if(!ut_list[1].pusb_dev)
        ss = &ut_list[1];
    else
    {
        dem_msg(DEM_MSG_TYPE_ERR, " No context available??? \n");
        goto end;
    }
    
    dev->type = USB_DEVICE_TYPE_DEMOD;
    ss->pusb_dev = dev;
    
    /** Find the endpoints we need */
    for(i=0; i<altsetting->bNumEndpoints; i++) 
    {
        /* is it an BULK endpoint? */
        if((altsetting->endpoint[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) 
        {
            /* BULK in or out? */
            if(altsetting->endpoint[i].bEndpointAddress & USB_DIR_IN)
            {
                if(ss->ep_in == 0)
                    ss->ep_in = altsetting->endpoint[i].bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            }
            else
                ss->ep_out = altsetting->endpoint[i].bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
        }

        /* is it an interrupt endpoint? */
        if((altsetting->endpoint[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) 
            ss->ep_int = &altsetting->endpoint[i];
    }
    dem_msg(1, "Endpoints: Bulk-in: %d, Bulk-out: %d, Interrupt: 0x%p (Period %d)\n",
          ss->ep_in, ss->ep_out, ss->ep_int, (ss->ep_int ? ss->ep_int->bInterval : 0));

    ss->subclass = id->bInterfaceSubClass;
    ss->protocol = id->bInterfaceProtocol;
    ss->ifnum = intf;
    ss->protocol_name = "0x00";

     /* 
     * Set the handler pointers based on the protocol
     */
    switch(ss->protocol)
    {
    case UT_PR_CB:
        ss->transport_name = "Control/Bulk";
        ss->max_lun = 0;
        break;
    case UT_PR_CBI:
        ss->transport_name = "Control/Bulk/Interrupt";
        ss->transport = usb_tuner_CBI_transport;
        ss->transport_reset = usb_tuner_CB_reset;
        ss->max_lun = 0;
        break;
    case UT_PR_BULK:
        ss->transport_name = "Bulk";
        break;
    default:
        goto error;
    }
    dem_msg(1, "\n Transport: %s \n", ss->transport_name);
    dem_msg(1, " Protocol : %s \n", ss->protocol_name);
    dem_msg(1, " Max Lun  : %d \n", ss->max_lun);

    ss->semaphore = SYS_CreateSemaphore(1, 0);
    if(!ss->semaphore)
    {
        dem_msg(DEM_MSG_TYPE_ERR, " usb_tuner_probe() has error code %X \n", ERROR_USB_TUNER_CREATE_SEMAPHORE_FAIL);
        goto error;
    }

    goto end;

error:
    memset((void*)ss, 0, sizeof(struct usbtuner_data));
    ss = 0;

end: 
    return (void*)ss;

}

static void usb_tuner_disconnect(struct usb_device* dev, void* ptr)
{
    struct usbtuner_data* ss = 0;

    if(ut_list[0].pusb_dev == dev)
        ss = &ut_list[0];
    else if(ut_list[1].pusb_dev == dev)
        ss = &ut_list[1];
    else
        dem_msg(DEM_MSG_TYPE_ERR, " msc_disconnect() no available context?? \n");

    /** Do something.... */
    if(ss->semaphore)
    {
        /** wait ap finish the command */
        SYS_WaitSemaphore(ss->semaphore);
        SYS_ReleaseSemaphore(ss->semaphore);

        SYS_DeleteSemaphore(ss->semaphore);;
    }

    memset((void*)ss, 0, sizeof(struct usbtuner_data));
}

struct usb_driver usb_tuner_driver = 
{
    "usb_tuner_driver",   /** driver name */
    0,                    /** flag */
    0,                    /** mutex : serialize */
    usb_tuner_probe,      /** probe function */
    0,                    /** open function */
    usb_tuner_disconnect, /** disconnect function */
    0,                    /** ioctl */
    tuner_usb_ids,        /** id table */
    0,0                  /** driver_list */
};

int iteUsbDemodDriverRegister(void)
{
    int rst = 0;
    
    INIT_LIST_HEAD(&usb_tuner_driver.driver_list);
    usb_tuner_driver.serialize = SYS_CreateSemaphore(1, 0);

    rst = usb_register(&usb_tuner_driver);
    return rst;
}


