/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * USB HID Boot Protocol Keyboard driver.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <stdio.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_usbkbd.h"
#include "usb/ite_usb.h"


#define KEYCODE_NUML        0x53
#define KEYCODE_CAPSL       0x39
#define KEYCODE_SCROLLL     0x47

#define LED_NUML		    0x01
#define LED_CAPSL		    0x02
#define LED_SCROLLL		    0x04


struct usb_kbd
{
    struct usb_device* usbdev;
	//unsigned char old[8];
	struct urb *irq, *led;
	unsigned char newleds;

	unsigned char *new;
	struct usb_ctrlrequest *cr;
	unsigned char *leds;
	dma_addr_t new_dma;
	dma_addr_t leds_dma;

	//spinlock_t leds_lock;
	//bool led_urb_submitted;
};

static usb_kbd_cb	kbd_cb;

static struct usb_device_id usb_kbd_id_table[] = 
{
    { USB_INTERFACE_INFO(3, 1, 1) },
    { USB_INTERFACE_INFO(0, 0, 0) }  // end marker
};


static void usb_kbd_irq(struct urb *urb)
{
    struct usb_kbd *kbd = urb->context;
    int j, do_led=1;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
        do_led = 0;
		goto resubmit;
	}

    #if 1
    {
        uint32_t i;
        ithPrintf("\n");
        for(i=0; i<8; i++)
            ithPrintf(" %02d", kbd->new[i]);
        ithPrintf("\n");
    }
    #endif
    if(kbd_cb)
        kbd_cb(kbd->new);

    /** process LEDs */
    for(j=2; j<8; j++)
    {
        if(kbd->new[j] == KEYCODE_NUML)
            kbd->newleds ^= LED_NUML;
        else if(kbd->new[j] == KEYCODE_CAPSL)
            kbd->newleds ^= LED_CAPSL;
        else if(kbd->new[j] == KEYCODE_SCROLLL)
            kbd->newleds ^= LED_SCROLLL;
    }

    if((*kbd->leds) == kbd->newleds)
        do_led = 0;

    if(kbd->led->status == -EINPROGRESS)
        do_led = 0;

resubmit:
    j = usb_submit_urb(urb);
    if(j)
        ithPrintf("[KBD][ERR]: can't resubmit kbd intr, status %d \n", j);

    if(do_led)
    {
        *(kbd->leds) = kbd->newleds;
    kbd->led->dev = kbd->usbdev;
	    if (usb_submit_urb(kbd->led))
		    ithPrintf("[KBD][ERR]: usb_submit_urb(leds) failed\n");
    }

    return;
}

static void usb_kbd_led(struct urb *urb)
{
    struct usb_kbd *kbd = urb->context;

    if(urb->status)
        ithPrintf("[KBD][ERR]: led urb status 0x%x received \n", urb->status);
    
    if((*kbd->leds) == kbd->newleds)
        return;

    *(kbd->leds) = kbd->newleds;
    kbd->led->dev = kbd->usbdev;
    if(usb_submit_urb(kbd->led))
        ithPrintf("[KBD][ERR]: usb_submit_urb(leds) failed 2 \r\n");
}

static int usb_kbd_alloc_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	if (!(kbd->irq = usb_alloc_urb(0)))
        return -1;
	if (!(kbd->led = usb_alloc_urb(0)))
        return -1;
	if (!(kbd->new = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &kbd->new_dma)))
        return -1;
	if (!(kbd->cr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL)))
        return -1;
	if (!(kbd->leds = usb_alloc_coherent(dev, 1, GFP_ATOMIC, &kbd->leds_dma)))
        return -1;

    return 0;
}

static void usb_kbd_free_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	usb_free_urb(kbd->irq);
	usb_free_urb(kbd->led);
	usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
	kfree(kbd->cr);
	usb_free_coherent(dev, 1, kbd->leds, kbd->leds_dma);
}

static void* usb_kbd_probe(struct usb_interface *iface,
			 const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(iface);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_kbd *kbd = NULL;
	struct input_dev *input_dev;
	int i, pipe, maxp;
	int error = -ENOMEM;

	interface = iface->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1) {
		error = -ENODEV;
        goto end;
    }

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint)) {
		error = -ENODEV;
        goto end;
    }

    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
    maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));

	kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
	if (!kbd)
		goto fail1;

	if (usb_kbd_alloc_mem(dev, kbd))
		goto fail2;

    kbd->usbdev = dev;
    dev->type = USB_DEVICE_TYPE_KBD;

	usb_fill_int_urb(kbd->irq, dev, pipe,
			 kbd->new, (maxp > 8 ? 8 : maxp),
        usb_kbd_irq, kbd, endpoint->bInterval);
	kbd->irq->transfer_dma = kbd->new_dma;
	kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    /** fill Set_Report request urb for LED control */
    kbd->cr->bRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
	kbd->cr->bRequest = 0x09;
    kbd->cr->wValue = cpu_to_le16(0x200);
	kbd->cr->wIndex = cpu_to_le16(interface->desc.bInterfaceNumber);
    kbd->cr->wLength = cpu_to_le16(1);

    usb_fill_control_urb(kbd->led, dev, usb_sndctrlpipe(dev, 0),
			     (void *) kbd->cr, kbd->leds, 1,
			     usb_kbd_led, kbd);
	kbd->led->transfer_dma = kbd->leds_dma;
	kbd->led->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    usb_set_intfdata(iface, kbd);

    /** default disable all LEDs */
    usb_submit_urb(kbd->led); 

    /** submit interrupt in urb */
    usb_submit_urb(kbd->irq);

    return kbd;

fail2:	
	usb_kbd_free_mem(dev, kbd);
fail1:
   	kfree(kbd);
    kbd = NULL;
end:
    if(error)
        ithPrintf(" %s: error %d \n", __func__, error);
    return kbd;
}

static void usb_kbd_disconnect(struct usb_interface *intf)
{
	struct usb_kbd *kbd = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);
	if (kbd) {
		usb_kill_urb(kbd->irq);
		usb_kill_urb(kbd->led);
		usb_kbd_free_mem(interface_to_usbdev(intf), kbd);
		kfree(kbd);
	}
}

struct usb_driver usb_kbd_driver = 
{
    "usbkbd_driver",		/** driver name */
    0,						/** flag */
    usb_kbd_probe,			/** probe function */
    NULL,					/** open function */
    usb_kbd_disconnect,		/** disconnect function */
    NULL,					/** ioctl */
    usb_kbd_id_table,		/** id table */
    NULL,NULL				/** driver_list */
};

int iteUsbKbdRegister(void)
{
    INIT_LIST_HEAD(&usb_kbd_driver.driver_list);

    return usb_register(&usb_kbd_driver);
}

int iteUsbKbdSetCb(usb_kbd_cb cb)
{
    if(cb)
        kbd_cb = cb;
}
