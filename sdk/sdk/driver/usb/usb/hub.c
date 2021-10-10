#include "usb/config.h"
#include "usb/usb/host.h"

#define SET_ADDRESS_TRIES	    2
#define GET_DESCRIPTOR_TRIES	2


spinlock_t device_state_lock;

int find_next_zero_bit(uint32_t* map, uint8_t startBit, uint8_t endBit)
{
    int i = 0;
    for(i=startBit; i<=endBit; i++)
    {
        if((((*map)>>i) & 1) == 0)
        {
            (*map) |= (1<<i);
            return i;
        }
    }
    ithPrintf(" find_next_zero_bit(0x%08X, %d, %d) not find! \n", (*map), startBit, endBit);
    return -1;
}

void clear_bit(uint32_t* map, uint8_t i)
{
    (*map) &= ~(1<<i);
}



static int usb_enumerate_device_otg(struct usb_device *udev)
{
	return 0;
}

/**
 * usb_enumerate_device - Read device configs/intfs/otg (usbcore-internal)
 * @udev: newly addressed device (in ADDRESS state)
 *
 * This is only called by usb_new_device() and usb_authorize_device()
 * and FIXME -- all comments that apply to them apply here wrt to
 * environment.
 *
 * If the device is WUSB and not authorized, we don't attempt to read
 * the string descriptors, as they will be errored out by the device
 * until it has been authorized.
 *
 * Return: 0 if successful. A negative error code otherwise.
 */
static int usb_enumerate_device(struct usb_device *udev)
{
	int err;

	if (udev->config == NULL) {
		err = usb_get_configuration(udev);
		if (err < 0) {
			if (err != -ENODEV)
				dev_err(&udev->dev, "can't read configurations, error %d\n",
						err);
			return err;
		}
	}

	/* read the standard strings and cache them if present */
	udev->product = usb_cache_string(udev, udev->descriptor.iProduct);
	udev->manufacturer = usb_cache_string(udev,
					      udev->descriptor.iManufacturer);
	udev->serial = usb_cache_string(udev, udev->descriptor.iSerialNumber);

	err = usb_enumerate_device_otg(udev);
	if (err < 0)
		return err;

	usb_detect_interface_quirks(udev);

	return 0;
}


/**
 * usb_new_device - perform initial device setup (usbcore-internal)
 * @udev: newly addressed device (in ADDRESS state)
 *
 * This is called with devices which have been detected but not fully
 * enumerated.  The device descriptor is available, but not descriptors
 * for any device configuration.  The caller must have locked either
 * the parent hub (if udev is a normal device) or else the
 * usb_bus_list_lock (if udev is a root hub).  The parent's pointer to
 * udev has already been installed, but udev is not yet visible through
 * sysfs or other filesystem code.
 *
 * This call is synchronous, and may not be used in an interrupt context.
 *
 * Only the hub driver or root-hub registrar should ever call this.
 *
 * Return: Whether the device is configured properly or not. Zero if the
 * interface was registered with the driver core; else a negative errno
 * value.
 *
 */
int usb_new_device(struct usb_device *udev)
{
	int err;

	err = usb_enumerate_device(udev);	/* Read descriptors */
	if (err < 0)
		goto fail;
	dev_dbg(&udev->dev, "udev %d, busnum %d, minor = %d\n",
			udev->devnum, udev->bus->busnum,
			(((udev->bus->busnum-1) * 128) + (udev->devnum-1)));

	return err;

fail:
	usb_set_device_state(udev, USB_STATE_NOTATTACHED);
	return err;
}

void usb_ep0_reinit(struct usb_device *udev)
{
	usb_disable_endpoint(udev, 0 + USB_DIR_IN, true);
	usb_disable_endpoint(udev, 0 + USB_DIR_OUT, true);
	usb_enable_endpoint(udev, &udev->ep0, true);
}

/**
 * usb_set_device_state - change a device's current state (usbcore, hcds)
 * @udev: pointer to device whose state should be changed
 * @new_state: new state value to be stored
 *
 * udev->state is _not_ fully protected by the device lock.  Although
 * most transitions are made only while holding the lock, the state can
 * can change to USB_STATE_NOTATTACHED at almost any time.  This
 * is so that devices can be marked as disconnected as soon as possible,
 * without having to wait for any semaphores to be released.  As a result,
 * all changes to any device's state must be protected by the
 * device_state_lock spinlock.
 *
 * Once a device has been added to the device tree, all changes to its state
 * should be made using this routine.  The state should _not_ be set directly.
 *
 * If udev->state is already USB_STATE_NOTATTACHED then no change is made.
 * Otherwise udev->state is set to new_state, and if new_state is
 * USB_STATE_NOTATTACHED then all of udev's descendants' states are also set
 * to USB_STATE_NOTATTACHED.
 */
void usb_set_device_state(struct usb_device *udev,
		enum usb_device_state new_state)
{
    unsigned long flags;
	
    spin_lock_irqsave(&device_state_lock, flags);

	if (udev->state == USB_STATE_NOTATTACHED)
		;	/* do nothing */
	else if (new_state != USB_STATE_NOTATTACHED) {
		udev->state = new_state;
	} else
        udev->state = USB_STATE_NOTATTACHED;	

    spin_unlock_irqrestore(&device_state_lock, flags);
}

/*
 * Connect a new USB device. This basically just initializes
 * the USB device information and sets up the topology - it's
 * up to the low-level driver to reset the port and actually
 * do the setup (the upper levels don't know how to do that).
 *
 * Actually it should call by HUB thread.
 */
void choose_devnum(struct usb_device* dev)
{
    //dev->devnum = find_next_zero_bit(&dev->bus->devmap, 1, 31);
    dev->devnum = 2;
}

static void release_devnum(struct usb_device *udev)
{
    //clear_bit(&udev->bus->devmap, udev->devnum);
    udev->devnum = 0;
}

static void update_devnum(struct usb_device *udev, int devnum)
{
	udev->devnum = devnum;
}


/**
 * usb_disconnect - disconnect a device (usbcore-internal)
 * @pdev: pointer to device being disconnected
 * Context: !in_interrupt ()
 *
 * Something got disconnected. Get rid of it and all of its children.
 *
 * If *pdev is a normal device then the parent hub must already be locked.
 * If *pdev is a root hub then the caller must hold the usb_bus_list_lock,
 * which protects the set of root hubs as well as the list of buses.
 *
 * Only hub drivers (including virtual root hub drivers for host
 * controllers) should ever call this.
 *
 * This call is synchronous, and may not be used in an interrupt context.
 */
void usb_disconnect(struct usb_device **pdev)
{
	struct usb_device	*udev = *pdev;
	int			i;

	/* mark the device as inactive, so any further urb submissions for
	 * this device (and any of its children) will fail immediately.
	 * this quiesces everything except pending urbs.
	 */
	usb_set_device_state(udev, USB_STATE_NOTATTACHED);
	dev_info(&udev->dev, "USB disconnect, device number %d\n",
			udev->devnum);

	usb_lock_device(udev);


	/* deallocate hcd/hardware state ... nuking all pending urbs and
	 * cleaning up all state associated with the current configuration
	 * so that the hardware is now fully quiesced.
	 */
	dev_dbg (&udev->dev, "unregistering device\n");
	usb_disable_device(udev, 0);
	usb_hcd_synchronize_unlinks(udev);


	usb_remove_ep_devs(&udev->ep0);
	usb_unlock_device(udev);

	/* Unregister the device.  The device driver is responsible
	 * for de-configuring the device and invoking the remove-device
	 * notifier chain (used by usbfs and possibly others).
	 */
	device_del(&udev->dev);

	/* Free the device number and delete the parent's children[]
	 * (or root_hub) pointer.
	 */
	release_devnum(udev);

	/* Avoid races with recursively_mark_NOTATTACHED() */
	spin_lock_irq(&device_state_lock);
	*pdev = NULL;
	spin_unlock_irq(&device_state_lock);

	usb_release_dev(udev);

	put_device(&udev->dev);
}


#define usb_sndaddr0pipe()	(PIPE_CONTROL << 30)
#define usb_rcvaddr0pipe()	((PIPE_CONTROL << 30) | USB_DIR_IN)

static int hub_set_address(struct usb_device *udev, int devnum)
{
	int retval;
#if 0
	struct usb_hcd *hcd = bus_to_hcd(udev->bus);

	/*
	 * The host controller will choose the device address,
	 * instead of the core having chosen it earlier
	 */
	if (!hcd->driver->address_device && devnum <= 1)
		return -EINVAL;
	if (udev->state == USB_STATE_ADDRESS)
		return 0;
	if (udev->state != USB_STATE_DEFAULT)
		return -EINVAL;
	if (hcd->driver->address_device)
		retval = hcd->driver->address_device(hcd, udev);
	else
#else
	if (udev->state == USB_STATE_ADDRESS)
		return 0;
	if (udev->state != USB_STATE_DEFAULT)
		return -EINVAL;
    else
#endif
		retval = usb_control_msg(udev, usb_sndaddr0pipe(),
				USB_REQ_SET_ADDRESS, 0, devnum, 0,
				NULL, 0, USB_CTRL_SET_TIMEOUT);
	if (retval == 0) {
		update_devnum(udev, devnum);
		/* Device now using proper address. */
		usb_set_device_state(udev, USB_STATE_ADDRESS);
		usb_ep0_reinit(udev);
	}
	return retval;
}


int port_init(struct ehci_hcd* ehci, struct usb_device *udev)
{
    int retval, i, j;
    enum usb_device_speed	oldspeed;
    int			devnum = udev->devnum;

    retval = ehci_port_reset(ehci);
    if(retval)
        goto fail;

	usb_set_device_state(udev, USB_STATE_DEFAULT);
    /* success, speed is known */
    oldspeed = udev->speed = ehci_get_speed(ehci);

    retval = -ENODEV;

	/* USB 2.0 section 5.5.3 talks about ep0 maxpacket ...
	 * it's fixed size except for full speed devices.
	 * For Wireless USB devices, ep0 max packet is always 512 (tho
	 * reported as 0xff in the device descriptor). WUSB1.0[4.8.1].
	 */
	switch (udev->speed) {
	case USB_SPEED_SUPER:
	case USB_SPEED_WIRELESS:	/* fixed at 512 */
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(512);
		break;
	case USB_SPEED_HIGH:		/* fixed at 64 */
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(64);
		break;
	case USB_SPEED_FULL:		/* 8, 16, 32, or 64 */
		/* to determine the ep0 maxpacket size, try to read
		 * the device descriptor to get bMaxPacketSize0 and
		 * then correct our initial guess.
		 */
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(64);
		break;
	case USB_SPEED_LOW:		/* fixed at 8 */
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(8);
		break;
	default:
		goto fail;
	}

    /* Set up TT records, if needed  */
    // We need??

    /* we start with SET_ADDRESS and then try to read the
	 * first 8 bytes of the device descriptor to get the ep0 maxpacket
	 * value.
	 */
    for (i = 0; i < GET_DESCRIPTOR_TRIES; (++i, msleep(100))) {
		for (j = 0; j < SET_ADDRESS_TRIES; ++j) {
			retval = hub_set_address(udev, devnum);
			if (retval >= 0)
				break;
			msleep(200);
		}
		if (retval < 0) {
			if (retval != -ENODEV)
                LOG_ERROR "%s:device not accepting address %d, error %d\n", __FUNCTION__,
						devnum, retval LOG_END
			goto fail;
		}
#if defined(SPEED_SUPER)
		if (udev->speed == USB_SPEED_SUPER) {
			devnum = udev->devnum;
            LOG_INFO "%s: %s SuperSpeed USB device number %d using %s\n", __FUNCTION__,
					(udev->config) ? "reset" : "new",
					devnum, udev->bus->controller->driver->name);
		}
#endif
		/* cope with hardware quirkiness:
		 *  - let SET_ADDRESS settle, some device hardware wants it
		 *  - read ep0 maxpacket even for high and low speed,
		 */
		msleep(10);
		/* use_new_scheme() checks the speed which may have
		 * changed since the initial look so we cache the result
		 * in did_new_scheme
		 */

		retval = usb_get_device_descriptor(udev, 8);
		if (retval < 8) {
			if (retval != -ENODEV)
                LOG_ERROR "%s: device descriptor read/8, error %d\n", __FUNCTION__,	retval LOG_END
			if (retval >= 0)
				retval = -EMSGSIZE;
		} else {
			retval = 0;
			break;
		}
    }
	if (retval)
		goto fail;

#if defined(SPEED_SUPER)
	/*
	 * Some superspeed devices have finished the link training process
	 * and attached to a superspeed hub port, but the device descriptor
	 * got from those devices show they aren't superspeed devices. Warm
	 * reset the port attached by the devices can fix them.
	 */
	if ((udev->speed == USB_SPEED_SUPER) &&
			(le16_to_cpu(udev->descriptor.bcdUSB) < 0x0300)) {
                LOG_ERROR "%s: got a wrong device descriptor, "
				"warm reset device\n", __FUNCTION__ LOG_END
		hub_port_reset(hub, port1, udev,
				HUB_BH_RESET_TIME, true);
		retval = -EINVAL;
		goto fail;
	}
#endif
	if (udev->descriptor.bMaxPacketSize0 == 0xff ||
			udev->speed == USB_SPEED_SUPER)
		i = 512;
	else
		i = udev->descriptor.bMaxPacketSize0;
	if (usb_endpoint_maxp(&udev->ep0.desc) != i) {
		if (udev->speed == USB_SPEED_LOW ||
				!(i == 8 || i == 16 || i == 32 || i == 64)) {
            LOG_ERROR "%s: Invalid ep0 maxpacket: %d\n", __FUNCTION__, i LOG_END
			retval = -EMSGSIZE;
			goto fail;
		}
		if (udev->speed == USB_SPEED_FULL)
			LOG_DEBUG "%s: ep0 maxpacket = %d\n", __FUNCTION__, i LOG_END
		else
			LOG_WARNING "%s: Using ep0 maxpacket: %d\n", __FUNCTION__, i LOG_END
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(i);
		usb_ep0_reinit(udev);
	}

	retval = usb_get_device_descriptor(udev, USB_DT_DEVICE_SIZE);
	if (retval < (int)sizeof(udev->descriptor)) {
		if (retval != -ENODEV)
            LOG_ERROR "%s: device descriptor read/all, error %d\n", __FUNCTION__,
					retval LOG_END
		if (retval >= 0)
			retval = -ENOMSG;
		goto fail;
	}
#if defined(SPEED_SUPER)
	if (udev->wusb == 0 && le16_to_cpu(udev->descriptor.bcdUSB) >= 0x0201) {
		retval = usb_get_bos_descriptor(udev);
		if (!retval) {
			udev->lpm_capable = usb_device_supports_lpm(udev);
			usb_set_lpm_parameters(udev);
		}
	}
#endif

	retval = 0;

fail:
    if(retval)
        LOG_ERROR " %s: rc %d \n", __FUNCTION__, retval LOG_END

    return retval;
}
