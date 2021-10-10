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
#include "libuvc/libuvc.h"
#include "libuvc/libuvc_internal.h"
//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                   Gloabl Data
//=============================================================================
uvc_device_t ud = {0};
	
static struct usb_device_id uvc_usb_ids[] = 
{
    /* Control/Bulk transport for all SubClass values */
    { USB_INTERFACE_INFO(USB_CLASS_VIDEO, 1, 0) },
    { USB_INTERFACE_INFO(USB_CLASS_VIDEO, 2, 0) },	
    { USB_INTERFACE_INFO(0, 0, 0) }  // end marker
};

//=============================================================================
//                   Private function definition
//=============================================================================

//=============================================================================
//                   Public function definition
//=============================================================================
uvc_device_t *uvc_get_device(void)
{
	return &ud;
}

static void* uvc_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    const int id_index = id - uvc_usb_ids;
	struct uvc_context *ctx;
    struct usb_host_interface *altsetting = intf->cur_altsetting;
    uint8_t i = 0;

    if(id_index >= sizeof(uvc_usb_ids)/sizeof(uvc_usb_ids[0]))
        goto end;

	if (id->idVendor && id->idProduct)
		UVC_DEBUG( "Probing known UVC device (%04x:%04x)\n",
				id->idVendor, id->idProduct );
	else
		UVC_DEBUG( "Probing generic UVC device.\n");

				
	
    ud.ref = 0;
    ud.usb_dev = intf->usb_dev;
    ud.usb_dev->type = USB_DEVICE_TYPE_UVC;

end:
    return (void*)&ud;
}

static void uvc_disconnect(struct usb_interface *intf)
{
	if (ud.ctx)
		uvc_exit(ud.ctx);
	
	ud.ctx = NULL;
	ud.ref = 0;
	ud.usb_dev = NULL;
}

struct usb_driver uvc_driver = 
{
    "uvc_driver",       /** driver name */
    0,                  /** flag */
    uvc_probe,          /** probe function */
    MMP_NULL,           /** open function */
    uvc_disconnect,     /** disconnect function */
    MMP_NULL,           /** ioctl */
    uvc_usb_ids,        /** id table */
    MMP_NULL,MMP_NULL   /** driver_list */
};

MMP_INT iteUvcDriverRegister(void)
{
    return usb_register(&uvc_driver);
}
