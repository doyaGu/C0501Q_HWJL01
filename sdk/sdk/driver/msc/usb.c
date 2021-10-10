/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file usb.c
 * Driver for USB Mass Storage compliant devices.
 *
 * @author Irene Lin
 */
//=============================================================================
//                              Include Files
//=============================================================================
#include "msc/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                   Gloabl Data
//=============================================================================
/** currently support 2 context for two USB host controller each */
static struct us_data us_list[2];

static struct usb_device_id storage_usb_ids[] = 
{
    /* Control/Bulk transport for all SubClass values */
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_RBC, US_PR_CB) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_8020, US_PR_CB) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_QIC, US_PR_CB) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_UFI, US_PR_CB) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_8070, US_PR_CB) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_SCSI, US_PR_CB) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, 0xFF, US_PR_CB) },

    /* Control/Bulk/Interrupt transport for all SubClass values */
#if 0
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_RBC, US_PR_CBI) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_8020, US_PR_CBI) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_QIC, US_PR_CBI) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_UFI, US_PR_CBI) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_8070, US_PR_CBI) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_SCSI, US_PR_CBI) },
#endif

    /* Bulk-only transport for all SubClass values */
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_RBC, US_PR_BULK) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_8020, US_PR_BULK) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_QIC, US_PR_BULK) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_UFI, US_PR_BULK) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_8070, US_PR_BULK) },
    { USB_INTERFACE_INFO(USB_CLASS_MASS_STORAGE, US_SC_SCSI, US_PR_BULK) },
    { USB_INTERFACE_INFO(0, 0, 0) }  // end marker
};

//=============================================================================
//                   Private function definition
//=============================================================================

/* Set up the IRQ pipe and handler
 * Note that this function assumes that all the data in the us_data
 * strucuture is current.  This includes the ep_int field, which gives us
 * the endpoint for the interrupt.
 * Returns non-zero on failure, zero on success
 */ 
static MMP_INT usb_stor_allocate_irq(struct us_data *ss)
{
    /** TODO */
    return 0;
}

//=============================================================================
//                   Public function definition
//=============================================================================
static void* msc_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    const int id_index = id - storage_usb_ids;
    struct us_data* ss = NULL;
    struct usb_host_interface *altsetting = intf->cur_altsetting;
    uint8_t i = 0;

    if(id_index >= sizeof(storage_usb_ids)/sizeof(storage_usb_ids[0]))
        goto end;

    LOG_INFO " USB Mass Storage device detected!\n" LOG_END

    /** decide the context */
    if(!us_list[0].pusb_dev)
        ss = &us_list[0];
    else if(!us_list[1].pusb_dev)
        ss = &us_list[1];
    else
    {
        LOG_ERROR " No context available??? \n" LOG_END
        goto end;
    }
    ss->pusb_dev = intf->usb_dev;
    ss->pusb_dev->type = USB_DEVICE_TYPE_MSC;

    /** Find the endpoints we need */
    for(i=0; i<altsetting->desc.bNumEndpoints; i++) 
    {
        /* is it an BULK endpoint? */
        if((altsetting->endpoint[i].desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) 
        {
            /* BULK in or out? */
            if(altsetting->endpoint[i].desc.bEndpointAddress & USB_DIR_IN)
                ss->ep_in = altsetting->endpoint[i].desc.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            else
                ss->ep_out = altsetting->endpoint[i].desc.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
        }

        /* is it an interrupt endpoint? */
        if((altsetting->endpoint[i].desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) 
            ss->ep_int = &altsetting->endpoint[i];
    }
    LOG_INFO "Endpoints: Bulk-in: %d, Bulk-out: %d, Interrupt: 0x%p (Period %d)\n",
          ss->ep_in, ss->ep_out, ss->ep_int, (ss->ep_int ? ss->ep_int->desc.bInterval : 0) LOG_END

    if(!ss->ep_in || !ss->ep_out || (id->bInterfaceProtocol == US_PR_CBI && !ss->ep_int))
    {
        LOG_ERROR " Endpoint sanity check failed! Rejecting devcie~~ \n" LOG_END
        LOG_ERROR " Endpoints: Bulk-in: %d, Bulk-out: %d, Interrupt: 0x%p (Period %d)\n",
              ss->ep_in, ss->ep_out, ss->ep_int, (ss->ep_int ? ss->ep_int->desc.bInterval : 0) LOG_END
        goto error;
    }
#if 0
    /** fetch the strings */
    if(dev->descriptor.iManufacturer)
        usb_string(dev, dev->descriptor.iManufacturer, ss->vendor, sizeof(ss->vendor));
    if(dev->descriptor.iProduct)
        usb_string(dev, dev->descriptor.iProduct, ss->product, sizeof(ss->product));
    if(dev->descriptor.iSerialNumber)
        usb_string(dev, dev->descriptor.iSerialNumber, ss->serial, sizeof(ss->serial));
#endif
    ss->subclass = id->bInterfaceSubClass;
    ss->protocol = id->bInterfaceProtocol;
    ss->ifnum = altsetting->desc.bInterfaceNumber;

    /* 
     * Set the handler pointers based on the protocol
     */
    switch(ss->protocol)
    {
    case US_PR_CB:
        ss->transport_name = "Control/Bulk";
        ss->transport = usb_stor_CB_transport;
        ss->transport_reset = usb_stor_CB_reset;
        ss->max_lun = 0;
        break;
    case US_PR_CBI:
        ss->transport_name = "Control/Bulk/Interrupt";
        ss->transport = usb_stor_CBI_transport;
        ss->transport_reset = usb_stor_CB_reset;
        ss->max_lun = 0;
        break;
    case US_PR_BULK:
        ss->transport_name = "Bulk";
        ss->transport = usb_stor_Bulk_transport;
        ss->transport_reset = usb_stor_Bulk_reset;
        ss->max_lun = usb_stor_Bulk_max_lun(ss);
        break;
    default:
        goto error;
    }
    LOG_DATA "\n Transport: %s \n", ss->transport_name LOG_END

    switch(ss->subclass)
    {
    case US_SC_RBC:
        ss->protocol_name = "Reduced Block Commands (RBC)";
        break;
    case US_SC_8020:
        ss->protocol_name = "8020i";
        ss->max_lun = 0;
        break;
    case US_SC_QIC:
        ss->protocol_name = "QIC-157";
        ss->max_lun = 0;
        break;
    case US_SC_8070:
        ss->protocol_name = "8070i";
        ss->max_lun = 0;
        break;
    case US_SC_SCSI:
        ss->protocol_name = "Transparent SCSI";
        ss->use_mode_sense6 = 1;
        break;
    case US_SC_UFI:
        ss->protocol_name = "Uniform Floppy Interface (UFI)";
        break;
    case 0xFF:
        ss->protocol_name = "0xFF";
        break;
    default:
        goto error;
    }
    LOG_DATA " Protocol : %s \n", ss->protocol_name LOG_END
    LOG_DATA " Max Lun  : %d \n", ss->max_lun LOG_END

    /* allocate an IRQ callback if one is needed */
    if((ss->protocol == US_PR_CBI) && usb_stor_allocate_irq(ss)) 
        goto error;

    ss->semaphore = SYS_CreateSemaphore(1, MMP_NULL);
    if(!ss->semaphore)
    {
        LOG_ERROR " msc_probe() has error code %X \n", ERROR_MSC_CREATE_SEMAPHORE_FAIL LOG_END
        goto error;
    }

    goto end;

error:
    memset((void*)ss, 0, sizeof(struct us_data));
    ss = NULL;

end:
    return (void*)ss;
}

static void msc_disconnect(struct usb_interface *intf)
{
    struct us_data* ss = (struct us_data*)usb_get_intfdata(intf);

    if(!ss)
		return;

    /** Do something.... */
    if(ss->semaphore)
    {
        /** wait ap finish the command */
        SYS_WaitSemaphore(ss->semaphore);
        SYS_ReleaseSemaphore(ss->semaphore);

        SYS_DeleteSemaphore(ss->semaphore);;
    }

    memset((void*)ss, 0, sizeof(struct us_data));
}

struct usb_driver msc_driver = 
{
    "msc_driver",       /** driver name */
    0,                  /** flag */
    msc_probe,          /** probe function */
    MMP_NULL,           /** open function */
    msc_disconnect,     /** disconnect function */
    MMP_NULL,           /** ioctl */
    storage_usb_ids,    /** id table */
    MMP_NULL,MMP_NULL   /** driver_list */
};

MMP_INT mmpMscDriverRegister(void)
{
    return usb_register(&msc_driver);
}

