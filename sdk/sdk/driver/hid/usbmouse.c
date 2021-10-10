/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * USB HID Boot Protocol Mouse driver.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_usbmouse.h"
#include "usb/ite_usb.h"

struct usb_mouse
{
	char name[128];
    struct usb_device* usbdev;
    struct urb* irq;

	signed char *data;
	dma_addr_t data_dma;
};

static struct usb_mouse ctxt;
static usb_mouse_cb	mouse_cb;

static struct usb_device_id usb_mouse_id_table[] = 
{
    { USB_INTERFACE_INFO(3, 1, 2) },
    { USB_INTERFACE_INFO(0, 0, 0) }  // end marker
};


static void usb_mouse_irq(struct urb *urb)
{
    struct usb_mouse *mouse = urb->context;
    int status;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		goto resubmit;
	}

    #if defined(DUMP_CODE)
    {
        uint32_t i;
        ithPrintf("\n");
        for(i=0; i<4; i++)
            ithPrintf(" %02d", mouse->data[i]);
        ithPrintf("\n");
    }
    #endif

    if(mouse_cb)
        mouse_cb(mouse->data);

resubmit:
	status = usb_submit_urb (urb);
	if (status)
		ithPrintf("[MOUSE] can't resubmit intr, status %d", status);

    return;
}

static void* usb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_mouse *mouse = NULL;
    int pipe, maxp;
	int error = -ENOMEM;

	interface = intf->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1)
    {
		error = -ENODEV;
        goto end;
    }

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
    {
		error = -ENODEV;
        goto end;
    }

    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
    maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));

    mouse = kzalloc(sizeof(struct usb_mouse), GFP_KERNEL);
	if (!mouse)
		goto fail1;

	mouse->data = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &mouse->data_dma);
	if (!mouse->data)
		goto fail1;

    mouse->irq = usb_alloc_urb(0);
	if (!mouse->irq)
		goto fail2;

    mouse->usbdev = dev;
    dev->type = USB_DEVICE_TYPE_MOUSE;

	if (dev->manufacturer)
		strlcpy(mouse->name, dev->manufacturer, sizeof(mouse->name));

	if (dev->product) {
		if (dev->manufacturer)
			strlcat(mouse->name, " ", sizeof(mouse->name));
		strlcat(mouse->name, dev->product, sizeof(mouse->name));
	}

	if (!strlen(mouse->name))
    {
		ithPrintf(" %s \n", mouse->name);
        ithPrintf(" USB HIDBP Mouse %04x:%04x \n",
			 le16_to_cpu(dev->descriptor.idVendor),
			 le16_to_cpu(dev->descriptor.idProduct));
    }

	usb_fill_int_urb(mouse->irq, dev, pipe, mouse->data,
			 (maxp > 8 ? 8 : maxp),
        usb_mouse_irq, mouse, endpoint->bInterval);
	mouse->irq->transfer_dma = mouse->data_dma;
	mouse->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	usb_set_intfdata(intf, mouse);

    /** submit interrupt in urb */
    error = usb_submit_urb(mouse->irq);
    if(error)
        goto fail2;

    return mouse;

fail2:	
	usb_free_coherent(dev, 8, mouse->data, mouse->data_dma);
fail1:	
	mouse = NULL;
end:
    if(error)
        ithPrintf(" %s: error %d \n", __func__, error);
    return mouse;
}

static void usb_mouse_disconnect(struct usb_interface *intf)
{
    struct usb_mouse *mouse = usb_get_intfdata (intf);

    usb_set_intfdata(intf, NULL);
	if (mouse) {
		usb_kill_urb(mouse->irq);
        usb_free_urb(mouse->irq);
		usb_free_coherent(interface_to_usbdev(intf), 8, mouse->data, mouse->data_dma);
		kfree(mouse);
	}
}

struct usb_driver usb_mouse_driver = 
{
    "usbmouse_driver",		/** driver name */
    0,						/** flag */
    usb_mouse_probe,		/** probe function */
    NULL,					/** open function */
    usb_mouse_disconnect,	/** disconnect function */
    NULL,					/** ioctl */
    usb_mouse_id_table,		/** id table */
    NULL,NULL				/** driver_list */
};


int iteUsbMouseRegister(void)
{
    INIT_LIST_HEAD(&usb_mouse_driver.driver_list);

    return usb_register(&usb_mouse_driver);
}

int iteUsbMouseSetCb(usb_mouse_cb cb)
{
    if(cb)
        mouse_cb = cb;
}
