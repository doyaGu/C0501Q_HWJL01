/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * USB Host Controller Driver framework.
 *
 * @author Irene Lin
 */
/*
 * USB Host Controller Driver framework
 *
 * Plugs into usbcore (usb_bus) and lets HCDs share code, minimizing
 * HCD-specific behaviors/bugs.
 *
 * This does error checks, tracks devices and urbs, and delegates to a
 * "hc_driver" only for code (and data) that really needs to know about
 * hardware differences.  That includes root hub registers, i/o queues,
 * and so on ... but as little else as possible.
 *
 * Shared code includes most of the "root hub" code (these are emulated,
 * though each HC's hardware works differently) and PCI glue, plus request
 * tracking overhead.  The HCD code should only block on spinlocks or on
 * hardware handshaking; blocking on software events (such as other kernel
 * threads releasing resources, or completing actions) is all generic.
 *
 * Happens the USB 2.0 spec says this would be invisible inside the "USBD",
 * and includes mostly a "HCDI" (HCD Interface) along with some APIs used
 * only by the hub driver ... and that neither should be seen or used by
 * usb client device drivers.
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/usb/host.h"

//=============================================================================
//                              Global Data Definition
//=============================================================================
struct usb_hcd* hcd0 = NULL;
struct usb_hcd* hcd1 = NULL;

wait_queue_head_t	usb_kill_urb_queue;


/*-------------------------------------------------------------------------*/
/**
 * usb_bus_init - shared initialization code
 * @bus: the bus structure being initialized
 *
 * This code is used to initialize a usb_bus structure, memory for which is
 * separately managed.
 */
static void usb_bus_init (struct usb_bus *bus)
{
	bus->root_device = NULL;
	bus->busnum = -1;
	bus->bandwidth_allocated = 0;
	bus->bandwidth_int_reqs  = 0;
	bus->bandwidth_isoc_reqs = 0;

	//INIT_LIST_HEAD (&bus->bus_list);
}

/**
 * usb_create_hcd - create and initialize an HCD structure
 * @driver: HC driver that will use this hcd
 * @dev: device for this HC, stored in hcd->self.controller
 * @bus_name: value to store in hcd->self.bus_name
 * Context: !in_interrupt()
 *
 * Allocate a struct usb_hcd, with extra space at the end for the
 * HC driver's private data.  Initialize the generic members of the
 * hcd structure.
 *
 * If memory is unavailable, returns NULL.
 */
struct usb_hcd *usb_create_hcd (const struct hc_driver *driver,
		uint32_t index)
{
	struct usb_hcd *hcd;

	hcd = malloc(sizeof(*hcd) + driver->hcd_priv_size);
	if (!hcd) {
		LOG_ERROR "hcd alloc failed\n" LOG_END
		return NULL;
    }
    memset((void*)hcd, 0, sizeof(*hcd) + driver->hcd_priv_size);

    hcd->bandwidth_mutex = malloc(sizeof(*hcd->bandwidth_mutex));
    if(!hcd->bandwidth_mutex)
    {
        LOG_ERROR "hcd alloc mutex failed \n" LOG_END
        free(hcd);
        return NULL;
    }
    mutex_init(hcd->bandwidth_mutex);
	usb_bus_init(&hcd->self);
    hcd->driver = driver;
    hcd->index = index;
    hcd->self.busnum = index;
	spin_lock_init(&hcd->hcd_urb_list_lock);

    if(!(index & 0x1)) 
    {   /** usb 0 */
        hcd->iobase = USB0_BASE;
        hcd0 = hcd;
    }
    else /** usb 1 */
    {
        hcd->iobase = USB1_BASE;
        hcd1 = hcd;
    }
    LOG_INFO " driver index = 0x%08X, hcd->iobase = 0x%08X \n", hcd->index, hcd->iobase LOG_END
    
	return hcd;
}

static void ehci_hcd_free(struct usb_hcd *hcd)
{
    if(hcd)
        free((void*)hcd);
}

//=============================================================================
//                              Public Function Definition
//=============================================================================

int usb_hcd_probe(struct hc_driver* driver, uint32_t index)
{
    int result = 0;
    struct usb_bus* bus;
    struct usb_hcd*	hcd;

    /** alloc and init usb_hcd */
    hcd = usb_create_hcd(driver, index);
    if(!hcd)
    {
        result = ERROR_USB_ALLOC_HCD_FAIL;
        goto end;
    }

    /** ehci start */
    result = driver->reset(hcd);
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR " usb_hcd_probe() return error code 0x%08X, hcd index = 0x%08X \n", result, hcd->index LOG_END
    return result;
}

/*-------------------------------------------------------------------------*/

/**
 * usb_calc_bus_time - approximate periodic transaction time in nanoseconds
 * @speed: from dev->speed; USB_SPEED_{LOW,FULL,HIGH}
 * @is_input: true iff the transaction sends data to the host
 * @isoc: true for isochronous transactions, false for interrupt ones
 * @bytecount: how many bytes in the transaction.
 *
 * Return: Approximate bus time in nanoseconds for a periodic transaction.
 *
 * Note:
 * See USB 2.0 spec section 5.11.3; only periodic transfers need to be
 * scheduled in software, this function is only used for such scheduling.
 */
long usb_calc_bus_time (int speed, int is_input, int isoc, int bytecount)
{
	unsigned long	tmp;

	switch (speed) {
	case USB_SPEED_LOW: 	/* INTR only */
		if (is_input) {
			tmp = (67667L * (31L + 10L * BitTime (bytecount))) / 1000L;
			return 64060L + (2 * BW_HUB_LS_SETUP) + BW_HOST_DELAY + tmp;
		} else {
			tmp = (66700L * (31L + 10L * BitTime (bytecount))) / 1000L;
			return 64107L + (2 * BW_HUB_LS_SETUP) + BW_HOST_DELAY + tmp;
		}
	case USB_SPEED_FULL:	/* ISOC or INTR */
		if (isoc) {
			tmp = (8354L * (31L + 10L * BitTime (bytecount))) / 1000L;
			return ((is_input) ? 7268L : 6265L) + BW_HOST_DELAY + tmp;
		} else {
			tmp = (8354L * (31L + 10L * BitTime (bytecount))) / 1000L;
			return 9107L + BW_HOST_DELAY + tmp;
		}
	case USB_SPEED_HIGH:	/* ISOC or INTR */
		/* FIXME adjust for input vs output */
		if (isoc)
			tmp = HS_NSECS_ISO (bytecount);
		else
			tmp = HS_NSECS (bytecount);
		return tmp;
	default:
		LOG_ERROR "%s: bogus device speed!\n", __func__ LOG_END
		return -1;
    }
}


/*-------------------------------------------------------------------------*/

/*
 * Generic HC operations.
 */

/*-------------------------------------------------------------------------*/

/**
 * usb_hcd_link_urb_to_ep - add an URB to its endpoint queue
 * @hcd: host controller to which @urb was submitted
 * @urb: URB being submitted
 *
 * Host controller drivers should call this routine in their enqueue()
 * method.  The HCD's private spinlock must be held and interrupts must
 * be disabled.  The actions carried out here are required for URB
 * submission, as well as for endpoint shutdown and for usb_kill_urb.
 *
 * Returns 0 for no error, otherwise a negative error code (in which case
 * the enqueue() method must fail).  If no error occurs but enqueue() fails
 * anyway, it must call usb_hcd_unlink_urb_from_ep() before releasing
 * the private spinlock and returning.
     */
int usb_hcd_link_urb_to_ep(struct usb_hcd *hcd, struct urb *urb)
{
	int		rc = 0;

	spin_lock(&hcd->hcd_urb_list_lock);

	/* Check that the URB isn't being killed */
	if (unlikely(atomic_read(&urb->reject))) {
		rc = -EPERM;
		goto done;
    }

	if (unlikely(!urb->ep->enabled)) {
		rc = -ENOENT;
		goto done;
    }

	if (unlikely(!urb->dev->can_submit)) {
		rc = -EHOSTUNREACH;
		goto done;
    }

    /*
	 * Check the host controller's state and add the URB to the
	 * endpoint's queue.
     */
    switch (hcd->state) {
    case HC_STATE_RUNNING:
    case HC_STATE_RESUMING:
        urb->unlinked = 0;
        list_add_tail(&urb->urb_list, &urb->ep->urb_list);
        break;
    default:
        rc = -ESHUTDOWN;
        goto done;
    }
 done:
	spin_unlock(&hcd->hcd_urb_list_lock);
	return rc;
}

/**
 * usb_hcd_check_unlink_urb - check whether an URB may be unlinked
 * @hcd: host controller to which @urb was submitted
 * @urb: URB being checked for unlinkability
 * @status: error code to store in @urb if the unlink succeeds
 *
 * Host controller drivers should call this routine in their dequeue()
 * method.  The HCD's private spinlock must be held and interrupts must
 * be disabled.  The actions carried out here are required for making
 * sure than an unlink is valid.
 *
 * Returns 0 for no error, otherwise a negative error code (in which case
 * the dequeue() method must fail).  The possible error codes are:
 *
 *	-EIDRM: @urb was not submitted or has already completed.
 *		The completion function may not have been called yet.
 *
 *	-EBUSY: @urb has already been unlinked.
     */
int usb_hcd_check_unlink_urb(struct usb_hcd *hcd, struct urb *urb,
		int status)
{
	struct list_head	*tmp;

	/* insist the urb is still queued */
	list_for_each(tmp, &urb->ep->urb_list) {
		if (tmp == &urb->urb_list)
			break;
    }
	if (tmp != &urb->urb_list)
		return -EIDRM;

	/* Any status except -EINPROGRESS means something already started to
	 * unlink this URB from the hardware.  So there's no more work to do.
     */
	if (urb->unlinked)
		return -EBUSY;
	urb->unlinked = status;

	return 0;
}

/**
 * usb_hcd_unlink_urb_from_ep - remove an URB from its endpoint queue
 * @hcd: host controller to which @urb was submitted
 * @urb: URB being unlinked
 *
 * Host controller drivers should call this routine before calling
 * usb_hcd_giveback_urb().  The HCD's private spinlock must be held and
 * interrupts must be disabled.  The actions carried out here are required
 * for URB completion.
 */
void usb_hcd_unlink_urb_from_ep(struct usb_hcd *hcd, struct urb *urb)
{
	/* clear all state linking urb to this dev (and hcd) */
	spin_lock(&hcd->hcd_urb_list_lock);
	list_del_init(&urb->urb_list);
	spin_unlock(&hcd->hcd_urb_list_lock);
}

/*-------------------------------------------------------------------------*/

static int map_urb_for_dma(struct usb_hcd *hcd, struct urb *urb)
{
    #if defined(CFG_CPU_WB)
    if(usb_endpoint_xfer_control(&urb->ep->desc))
        ithFlushDCacheRange((void*)urb->setup_packet, sizeof(struct usb_ctrlrequest));

    //if(usb_pipeout(urb->pipe))
        ithFlushDCacheRange((void*)urb->transfer_buffer, urb->transfer_buffer_length);
    #endif

    urb->setup_dma = (dma_addr_t)urb->setup_packet;
	urb->transfer_dma = (dma_addr_t)urb->transfer_buffer;

    return 0;
}

static void unmap_urb_for_dma(struct usb_hcd *hcd, struct urb *urb)
{
    if(usb_pipein(urb->pipe))
        ithInvalidateDCacheRange(urb->transfer_buffer, urb->actual_length);
}

/*-------------------------------------------------------------------------*/

/* may be called in any context with a valid urb->dev usecount
 * caller surrenders "ownership" of urb
 * expects usb_submit_urb() to have sanity checked and conditioned all
 * inputs in the urb
     */
int usb_hcd_submit_urb(struct urb* urb, gfp_t mem_flags)
{
    int status;
    struct usb_hcd *hcd = bus_to_hcd(urb->dev->bus);
     
    /* increment urb's reference count as part of giving it to the HCD
	 * (which will control it).  HCD guarantees that it either returns
	 * an error or calls giveback(), but not both.
     */
    usb_get_urb(urb);
    atomic_inc(&urb->use_count);
    atomic_inc(&urb->dev->urbnum);

    status = map_urb_for_dma(hcd, urb);
    if(unlikely(status))
        goto error;

    status = hcd->driver->urb_enqueue(hcd, urb, mem_flags);
    if(unlikely(status))
    {
        unmap_urb_for_dma(hcd, urb);
error:
        urb->hcpriv = NULL;
        INIT_LIST_HEAD(&urb->urb_list);
        atomic_dec(&urb->use_count);
        atomic_dec(&urb->dev->urbnum);
	    if (atomic_read(&urb->reject))
            wake_up(&usb_kill_urb_queue);
        usb_put_urb(urb);
    }

    if(status)
        LOG_ERROR " %s: status = %d (0x%X) \n", __func__, status, status LOG_END
	
    return status;
}

	
/*-------------------------------------------------------------------------*/

int usb_hcd_dev_exist(struct usb_device *udev)
{
	struct usb_hcd	*hcd = bus_to_hcd(udev->bus);

	if (!HC_IS_RUNNING (hcd->state))
		return -ESHUTDOWN;
	return hcd->driver->dev_exist (hcd);
}

/*-------------------------------------------------------------------------*/

/* called in any context */
int usb_hcd_get_frame_number (struct usb_device *udev)
{
	struct usb_hcd	*hcd = bus_to_hcd(udev->bus);

	if (!HC_IS_RUNNING (hcd->state))
		return -ESHUTDOWN;
	return hcd->driver->get_frame_number (hcd);
}

/*-------------------------------------------------------------------------*/

/* this makes the hcd giveback() the urb more quickly, by kicking it
 * off hardware queues (which may take a while) and returning it as
 * soon as practical.  we've already set up the urb's return status,
 * but we can't know if the callback completed already.
 */
static int unlink1(struct usb_hcd *hcd, struct urb *urb, int status)
{
	int		value;

	/* The only reason an HCD might fail this call is if
	 * it has not yet fully queued the urb to begin with.
	 * Such failures should be harmless. */
	value = hcd->driver->urb_dequeue(hcd, urb, status);

    return value;
}

/*
 * called in any context
 *
 * caller guarantees urb won't be recycled till both unlink()
 * and the urb's completion function return
 */
int usb_hcd_unlink_urb (struct urb *urb, int status)
{
	struct usb_hcd		*hcd;
	int			retval = -EIDRM;

	/* Prevent the device and bus from going away while
	 * the unlink is carried out.  If they are already gone
	 * then urb->use_count must be 0, since disconnected
	 * devices can't have any active URBs.
     */
	ithEnterCritical();
	if (atomic_read(&urb->use_count) > 0) {
		retval = 0;
		usb_get_dev(urb->dev);
    }
	ithExitCritical();
	if (retval == 0) {
        hcd = bus_to_hcd(urb->dev->bus);
		retval = unlink1(hcd, urb, status);
		usb_put_dev(urb->dev);
        }

	if (retval == 0)
		retval = -EINPROGRESS;
	else if (retval != -EIDRM && retval != -EBUSY)
		LOG_ERROR "hcd_unlink_urb %p fail %d\n", urb, retval LOG_END
	return retval;
}

/*-------------------------------------------------------------------------*/
/**
 * usb_hcd_giveback_urb - return URB from HCD to device driver
 * @hcd: host controller returning the URB
 * @urb: urb being returned to the USB device driver.
 * @status: completion status code for the URB.
 * Context: in_interrupt()
 *
 * This hands the URB from HCD to its USB device driver, using its
 * completion function.  The HCD has freed all per-urb resources
 * (and is done using urb->hcpriv).  It also released all HCD locks;
 * the device driver won't cause problems if it frees, modifies,
 * or resubmits this URB.
 *
 * If @urb was unlinked, the value of @status will be overridden by
 * @urb->unlinked.  Erroneous short transfers are detected in case
 * the HCD hasn't checked for them.
 */
void usb_hcd_giveback_urb(struct usb_hcd *hcd, struct urb *urb, int status)
{
    struct usb_anchor *anchor = urb->anchor;
    urb->hcpriv = NULL;

    LOG_DEBUG " %s: urb %p, status %d \n", __func__, urb, status LOG_END
	//LOG_DEBUG " %s: urb %p \n", __func__, urb LOG_END

    if(unlikely(urb->unlinked))
        status = urb->unlinked;
    else if(unlikely((urb->transfer_flags & URB_SHORT_NOT_OK) &&
                     urb->actual_length < urb->transfer_buffer_length &&
                     !status))
        status = -EREMOTEIO;

    unmap_urb_for_dma(hcd, urb);
	usb_anchor_suspend_wakeups(anchor);
    usb_unanchor_urb(urb);

    urb->status = status;
    urb->complete(urb);

    usb_anchor_resume_wakeups(anchor);
	atomic_dec (&urb->use_count);
	if (unlikely (atomic_read(&urb->reject)))
        wake_up(&usb_kill_urb_queue);
	usb_put_urb (urb);
}

/*-------------------------------------------------------------------------*/

/* Cancel all URBs pending on this endpoint and wait for the endpoint's
 * queue to drain completely.  The caller must first insure that no more
 * URBs can be submitted for this endpoint.
 */
void usb_hcd_flush_endpoint(struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	struct usb_hcd		*hcd;
	struct urb		*urb;

	if (!ep)
		return;
	might_sleep();
	hcd = bus_to_hcd(udev->bus);

	/* No more submits can occur */
	spin_lock_irq(&hcd->hcd_urb_list_lock);
rescan:
	list_for_each_entry (urb, &ep->urb_list, urb_list) {
		int	is_in;

		if (urb->unlinked)
			continue;
		usb_get_urb (urb);
		is_in = usb_urb_dir_in(urb);
		spin_unlock(&hcd->hcd_urb_list_lock);

		/* kick hcd */
		unlink1(hcd, urb, -ESHUTDOWN);
		LOG_DEBUG "shutdown urb %p ep%d%s%s\n",
			urb, usb_endpoint_num(&ep->desc),
			is_in ? "in" : "out",
			({	char *s;

				 switch (usb_endpoint_type(&ep->desc)) {
				 case USB_ENDPOINT_XFER_CONTROL:
					s = ""; break;
				 case USB_ENDPOINT_XFER_BULK:
					s = "-bulk"; break;
				 case USB_ENDPOINT_XFER_INT:
					s = "-intr"; break;
				 default:
					s = "-iso"; break;
				};
				s;
			}) LOG_END
		usb_put_urb (urb);

		/* list contents may have changed */
		spin_lock(&hcd->hcd_urb_list_lock);
		goto rescan;
	}
	spin_unlock_irq(&hcd->hcd_urb_list_lock);

	/* Wait until the endpoint queue is completely empty */
	while (!list_empty (&ep->urb_list)) {
		spin_lock_irq(&hcd->hcd_urb_list_lock);

		/* The list may have changed while we acquired the spinlock */
		urb = NULL;
		if (!list_empty (&ep->urb_list)) {
			urb = list_entry (ep->urb_list.prev, struct urb,
					urb_list);
			usb_get_urb (urb);
		}
		spin_unlock_irq(&hcd->hcd_urb_list_lock);

		if (urb) {
			usb_kill_urb (urb);
			usb_put_urb (urb);
		}
	}
}
#if 0
/**
 * usb_hcd_alloc_bandwidth - check whether a new bandwidth setting exceeds
 *				the bus bandwidth
 * @udev: target &usb_device
 * @new_config: new configuration to install
 * @cur_alt: the current alternate interface setting
 * @new_alt: alternate interface setting that is being installed
 *
 * To change configurations, pass in the new configuration in new_config,
 * and pass NULL for cur_alt and new_alt.
 *
 * To reset a device's configuration (put the device in the ADDRESSED state),
 * pass in NULL for new_config, cur_alt, and new_alt.
 *
 * To change alternate interface settings, pass in NULL for new_config,
 * pass in the current alternate interface setting in cur_alt,
 * and pass in the new alternate interface setting in new_alt.
 *
 * Return: An error if the requested bandwidth change exceeds the
 * bus bandwidth or host controller internal resources.
 */
int usb_hcd_alloc_bandwidth(struct usb_device *udev,
		struct usb_host_config *new_config,
		struct usb_host_interface *cur_alt,
		struct usb_host_interface *new_alt)
{
	int num_intfs, i, j;
	struct usb_host_interface *alt = NULL;
	int ret = 0;
	struct usb_hcd *hcd;
	struct usb_host_endpoint *ep;

	hcd = bus_to_hcd(udev->bus);
	if (!hcd->driver->check_bandwidth)
		return 0;

	/* Configuration is being removed - set configuration 0 */
	if (!new_config && !cur_alt) {
		for (i = 1; i < 16; ++i) {
			ep = udev->ep_out[i];
			if (ep)
				hcd->driver->drop_endpoint(hcd, udev, ep);
			ep = udev->ep_in[i];
			if (ep)
				hcd->driver->drop_endpoint(hcd, udev, ep);
		}
		hcd->driver->check_bandwidth(hcd, udev);
		return 0;
	}
	/* Check if the HCD says there's enough bandwidth.  Enable all endpoints
	 * each interface's alt setting 0 and ask the HCD to check the bandwidth
	 * of the bus.  There will always be bandwidth for endpoint 0, so it's
	 * ok to exclude it.
	 */
	if (new_config) {
		num_intfs = new_config->desc.bNumInterfaces;
		/* Remove endpoints (except endpoint 0, which is always on the
		 * schedule) from the old config from the schedule
		 */
		for (i = 1; i < 16; ++i) {
			ep = udev->ep_out[i];
			if (ep) {
				ret = hcd->driver->drop_endpoint(hcd, udev, ep);
				if (ret < 0)
					goto reset;
			}
			ep = udev->ep_in[i];
			if (ep) {
				ret = hcd->driver->drop_endpoint(hcd, udev, ep);
				if (ret < 0)
					goto reset;
			}
		}
		for (i = 0; i < num_intfs; ++i) {
			struct usb_host_interface *first_alt;
			int iface_num;

			first_alt = &new_config->intf_cache[i]->altsetting[0];
			iface_num = first_alt->desc.bInterfaceNumber;
			/* Set up endpoints for alternate interface setting 0 */
			alt = usb_find_alt_setting(new_config, iface_num, 0);
			if (!alt)
				/* No alt setting 0? Pick the first setting. */
				alt = first_alt;

			for (j = 0; j < alt->desc.bNumEndpoints; j++) {
				ret = hcd->driver->add_endpoint(hcd, udev, &alt->endpoint[j]);
				if (ret < 0)
					goto reset;
			}
		}
	}
	if (cur_alt && new_alt) {
		struct usb_interface *iface = usb_ifnum_to_if(udev,
				cur_alt->desc.bInterfaceNumber);

		if (!iface)
			return -EINVAL;
		if (iface->resetting_device) {
			/*
			 * The USB core just reset the device, so the xHCI host
			 * and the device will think alt setting 0 is installed.
			 * However, the USB core will pass in the alternate
			 * setting installed before the reset as cur_alt.  Dig
			 * out the alternate setting 0 structure, or the first
			 * alternate setting if a broken device doesn't have alt
			 * setting 0.
			 */
			cur_alt = usb_altnum_to_altsetting(iface, 0);
			if (!cur_alt)
				cur_alt = &iface->altsetting[0];
		}

		/* Drop all the endpoints in the current alt setting */
		for (i = 0; i < cur_alt->desc.bNumEndpoints; i++) {
			ret = hcd->driver->drop_endpoint(hcd, udev,
					&cur_alt->endpoint[i]);
			if (ret < 0)
				goto reset;
		}
		/* Add all the endpoints in the new alt setting */
		for (i = 0; i < new_alt->desc.bNumEndpoints; i++) {
			ret = hcd->driver->add_endpoint(hcd, udev,
					&new_alt->endpoint[i]);
			if (ret < 0)
				goto reset;
		}
	}
	ret = hcd->driver->check_bandwidth(hcd, udev);
reset:
	if (ret < 0)
		hcd->driver->reset_bandwidth(hcd, udev);
	return ret;
}
#endif

/* Disables the endpoint: synchronizes with the hcd to make sure all
 * endpoint state is gone from hardware.  usb_hcd_flush_endpoint() must
 * have been called previously.  Use for set_configuration, set_interface,
 * driver removal, physical disconnect.
 *
 * example:  a qh stored in ep->hcpriv, holding state related to endpoint
 * type, maxpacket size, toggle, halt status, and scheduling.
 */
void usb_hcd_disable_endpoint(struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	struct usb_hcd		*hcd;

	might_sleep();
	hcd = bus_to_hcd(udev->bus);
	if (hcd->driver->endpoint_disable)
		hcd->driver->endpoint_disable(hcd, ep);
}

/**
 * usb_hcd_reset_endpoint - reset host endpoint state
 * @udev: USB device.
 * @ep:   the endpoint to reset.
 *
 * Resets any host endpoint state such as the toggle bit, sequence
 * number and current window.
 */
void usb_hcd_reset_endpoint(struct usb_device *udev,
			    struct usb_host_endpoint *ep)
{
	struct usb_hcd *hcd = bus_to_hcd(udev->bus);

	if (hcd->driver->endpoint_reset)
		hcd->driver->endpoint_reset(hcd, ep);
	else {
		int epnum = usb_endpoint_num(&ep->desc);
		int is_out = usb_endpoint_dir_out(&ep->desc);
		int is_control = usb_endpoint_xfer_control(&ep->desc);

		usb_settoggle(udev, epnum, is_out, 0);
		if (is_control)
			usb_settoggle(udev, epnum, !is_out, 0);
	}
}

/**
 * usb_alloc_streams - allocate bulk endpoint stream IDs.
 * @interface:		alternate setting that includes all endpoints.
 * @eps:		array of endpoints that need streams.
 * @num_eps:		number of endpoints in the array.
 * @num_streams:	number of streams to allocate.
 * @mem_flags:		flags hcd should use to allocate memory.
 *
 * Sets up a group of bulk endpoints to have @num_streams stream IDs available.
 * Drivers may queue multiple transfers to different stream IDs, which may
 * complete in a different order than they were queued.
 *
 * Return: On success, the number of allocated streams. On failure, a negative
 * error code.
 */
int usb_alloc_streams(struct usb_interface *interface,
		struct usb_host_endpoint **eps, unsigned int num_eps,
		unsigned int num_streams, gfp_t mem_flags)
{
	struct usb_hcd *hcd;
	struct usb_device *dev;
	int i;

	dev = interface_to_usbdev(interface);
	hcd = bus_to_hcd(dev->bus);
	if (!hcd->driver->alloc_streams || !hcd->driver->free_streams)
		return -EINVAL;
	if (dev->speed != USB_SPEED_SUPER)
		return -EINVAL;

	/* Streams only apply to bulk endpoints. */
	for (i = 0; i < num_eps; i++)
		if (!usb_endpoint_xfer_bulk(&eps[i]->desc))
			return -EINVAL;

	return hcd->driver->alloc_streams(hcd, dev, eps, num_eps,
			num_streams, mem_flags);
}

/**
 * usb_free_streams - free bulk endpoint stream IDs.
 * @interface:	alternate setting that includes all endpoints.
 * @eps:	array of endpoints to remove streams from.
 * @num_eps:	number of endpoints in the array.
 * @mem_flags:	flags hcd should use to allocate memory.
 *
 * Reverts a group of bulk endpoints back to not using stream IDs.
 * Can fail if we are given bad arguments, or HCD is broken.
 *
 * Return: On success, the number of allocated streams. On failure, a negative
 * error code.
 */
int usb_free_streams(struct usb_interface *interface,
		struct usb_host_endpoint **eps, unsigned int num_eps,
		gfp_t mem_flags)
{
	struct usb_hcd *hcd;
	struct usb_device *dev;
	int i;

	dev = interface_to_usbdev(interface);
	hcd = bus_to_hcd(dev->bus);
	if (dev->speed != USB_SPEED_SUPER)
		return -EINVAL;

	/* Streams only apply to bulk endpoints. */
	for (i = 0; i < num_eps; i++)
		if (!eps[i] || !usb_endpoint_xfer_bulk(&eps[i]->desc))
			return -EINVAL;

	return hcd->driver->free_streams(hcd, dev, eps, num_eps, mem_flags);
}



