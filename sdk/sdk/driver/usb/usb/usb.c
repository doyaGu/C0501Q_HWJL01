/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file
 * USB helper routines for real drivers.
 *
 * @author Irene Lin
 */
/* 
 * NOTE! This is not actually a driver at all, rather this is
 * just a collection of helper routines that implement the
 * generic USB things that the real drivers can use..
 *
 * Think of this as a "USB library" rather than anything else.
 * It should be considered a slave, with no callbacks. Callbacks
 * are evil.
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "usb/config.h"
#include "usb/usb/host.h"



/*-------------------------------------------------------------------*

/**
 * usb_find_alt_setting() - Given a configuration, find the alternate setting
 * for the given interface.
 * @config: the configuration to search (not necessarily the current config).
 * @iface_num: interface number to search in
 * @alt_num: alternate interface setting number to search for.
 *
 * Search the configuration's interface cache for the given alt setting.
 *
 * Return: The alternate setting, if found. %NULL otherwise.
 */
struct usb_host_interface *usb_find_alt_setting(
		struct usb_host_config *config,
		unsigned int iface_num,
		unsigned int alt_num)
{
	struct usb_interface_cache *intf_cache = NULL;
	int i;

	for (i = 0; i < config->desc.bNumInterfaces; i++) {
		if (config->intf_cache[i]->altsetting[0].desc.bInterfaceNumber
				== iface_num) {
			intf_cache = config->intf_cache[i];
			break;
		}
	}
	if (!intf_cache)
		return NULL;
	for (i = 0; i < intf_cache->num_altsetting; i++)
		if (intf_cache->altsetting[i].desc.bAlternateSetting == alt_num)
			return &intf_cache->altsetting[i];

	printk(KERN_DEBUG "Did not find alt setting %u for intf %u, "
			"config %u\n", alt_num, iface_num,
			config->desc.bConfigurationValue);
	return NULL;
}

/**
 * usb_ifnum_to_if - get the interface object with a given interface number
 * @dev: the device whose current configuration is considered
 * @ifnum: the desired interface
 *
 * This walks the device descriptor for the currently active configuration
 * to find the interface object with the particular interface number.
 *
 * Note that configuration descriptors are not required to assign interface
 * numbers sequentially, so that it would be incorrect to assume that
 * the first interface in that descriptor corresponds to interface zero.
 * This routine helps device drivers avoid such mistakes.
 * However, you should make sure that you do the right thing with any
 * alternate settings available for this interfaces.
 *
 * Don't call this function unless you are bound to one of the interfaces
 * on this device or you have locked the device!
 *
 * Return: A pointer to the interface that has @ifnum as interface number,
 * if found. %NULL otherwise.
 */
struct usb_interface *usb_ifnum_to_if(const struct usb_device *dev,
				      unsigned ifnum)
{
	struct usb_host_config *config = dev->actconfig;
	int i;

	if (!config)
		return NULL;
	for (i = 0; i < config->desc.bNumInterfaces; i++)
		if (config->interface[i]->altsetting[0]
				.desc.bInterfaceNumber == ifnum)
			return config->interface[i];

	return NULL;
}

/**
 * usb_altnum_to_altsetting - get the altsetting structure with a given alternate setting number.
 * @intf: the interface containing the altsetting in question
 * @altnum: the desired alternate setting number
 *
 * This searches the altsetting array of the specified interface for
 * an entry with the correct bAlternateSetting value.
 *
 * Note that altsettings need not be stored sequentially by number, so
 * it would be incorrect to assume that the first altsetting entry in
 * the array corresponds to altsetting zero.  This routine helps device
 * drivers avoid such mistakes.
 *
 * Don't call this function unless you are bound to the intf interface
 * or you have locked the device!
 *
 * Return: A pointer to the entry of the altsetting array of @intf that
 * has @altnum as the alternate setting number. %NULL if not found.
 */
struct usb_host_interface *usb_altnum_to_altsetting(
					const struct usb_interface *intf,
					unsigned int altnum)
{
	int i;

	for (i = 0; i < intf->num_altsetting; i++) {
		if (intf->altsetting[i].desc.bAlternateSetting == altnum)
			return &intf->altsetting[i];
	}
	return NULL;
}


/*-------------------------------------------------------------------*
 *      USB Device Function Definition
 *-------------------------------------------------------------------*/

struct usb_device* usb_alloc_dev(struct usb_device* parent, struct usb_bus* bus)
{
    struct usb_device* dev = NULL;

    dev = (struct usb_device*)malloc(sizeof(struct usb_device));
    if(!dev)
        goto end;

    memset(dev, 0, sizeof(struct usb_device));

    pthread_mutex_init(&dev->mutex, NULL);
    dev->state = USB_STATE_ATTACHED;
    atomic_set(&dev->urbnum, 0);

	INIT_LIST_HEAD(&dev->ep0.urb_list);
	dev->ep0.desc.bLength = USB_DT_ENDPOINT_SIZE;
	dev->ep0.desc.bDescriptorType = USB_DT_ENDPOINT;
	/* ep0 maxpacket comes later, from device descriptor */
	usb_enable_endpoint(dev, &dev->ep0, false);
	dev->can_submit = 1;

    dev->bus = bus;
    dev->parent = parent;

end:
    return dev;
}

/**
 * usb_release_dev - free a usb device structure when all users of it are finished.
 * @dev: device that's been disconnected
 *
 * Will be called only by the device core when all users of this usb device are
 * done.
 */
void usb_release_dev(struct usb_device* udev)
{
	struct usb_hcd *hcd = bus_to_hcd(udev->bus);

	hcd = bus_to_hcd(udev->bus);

	usb_destroy_configuration(udev);
	pthread_mutex_destroy(&udev->mutex);

	usb_put_hcd(hcd);
	free(udev->product);
	free(udev->manufacturer);
	free(udev->serial);
	free(udev);
}

/*			USB device locking
 *
 * USB devices and interfaces are locked using the semaphore in their
 * embedded struct device.  The hub driver guarantees that whenever a
 * device is connected or disconnected, drivers are called with the
 * USB device locked as well as their particular interface.
 *
 * Complications arise when several devices are to be locked at the same
 * time.  Only hub-aware drivers that are part of usbcore ever have to
 * do this; nobody else needs to worry about it.  The rule for locking
 * is simple:
 *
 *	When locking both a device and its parent, always lock the
 *	the parent first.
 */

/**
 * usb_lock_device_for_reset - cautiously acquire the lock for a usb device structure
 * @udev: device that's being locked
 * @iface: interface bound to the driver making the request (optional)
 *
 * Attempts to acquire the device lock, but fails if the device is
 * NOTATTACHED or SUSPENDED, or if iface is specified and the interface
 * is neither BINDING nor BOUND.  Rather than sleeping to wait for the
 * lock, the routine polls repeatedly.  This is to prevent deadlock with
 * disconnect; in some drivers (such as usb-storage) the disconnect()
 * or suspend() method will block waiting for a device reset to complete.
 *
 * Return: A negative error code for failure, otherwise 0.
 */
int usb_lock_device_for_reset(struct usb_device *udev,
			      const struct usb_interface *iface)
{
	unsigned long jiffies_expire = jiffies + HZ;

	if (udev->state == USB_STATE_NOTATTACHED)
		return -ENODEV;
	if (udev->state == USB_STATE_SUSPENDED)
		return -EHOSTUNREACH;
	if (iface && (iface->condition == USB_INTERFACE_UNBINDING ||
			iface->condition == USB_INTERFACE_UNBOUND))
		return -EINTR;

	while (!usb_trylock_device(udev)) {

		/* If we can't acquire the lock after waiting one second,
		 * we're probably deadlocked */
		if (time_after(jiffies, jiffies_expire))
			return -EBUSY;

		msleep(15);
		if (udev->state == USB_STATE_NOTATTACHED)
			return -ENODEV;
		if (udev->state == USB_STATE_SUSPENDED)
			return -EHOSTUNREACH;
		if (iface && (iface->condition == USB_INTERFACE_UNBINDING ||
				iface->condition == USB_INTERFACE_UNBOUND))
			return -EINTR;
	}
	return 0;
}

/**
 * usb_get_current_frame_number - return current bus frame number
 * @dev: the device whose bus is being queried
 *
 * Return: The current frame number for the USB host controller used
 * with the given USB device. This can be used when scheduling
 * isochronous requests.
 *
 * Note: Different kinds of host controller have different "scheduling
 * horizons". While one type might support scheduling only 32 frames
 * into the future, others could support scheduling up to 1024 frames
 * into the future.
 *
 */
int usb_get_current_frame_number(struct usb_device *dev)
{
	return usb_hcd_get_frame_number(dev);
}

int usb_dev_exist(struct usb_device* usb_dev)
{
    return usb_hcd_dev_exist(usb_dev);
}

/**
 * usb_alloc_coherent - allocate dma-consistent buffer for URB_NO_xxx_DMA_MAP
 * @dev: device the buffer will be used with
 * @size: requested buffer size
 * @mem_flags: affect whether allocation may block
 * @dma: used to return DMA address of buffer
 *
 * Return value is either null (indicating no buffer could be allocated), or
 * the cpu-space pointer to a buffer that may be used to perform DMA to the
 * specified device.  Such cpu-space buffers are returned along with the DMA
 * address (through the pointer provided).
 *
 * These buffers are used with URB_NO_xxx_DMA_MAP set in urb->transfer_flags
 * to avoid behaviors like using "DMA bounce buffers", or thrashing IOMMU
 * hardware during URB completion/resubmit.  The implementation varies between
 * platforms, depending on details of how DMA will work to this device.
 * Using these buffers also eliminates cacheline sharing problems on
 * architectures where CPU caches are not DMA-coherent.  On systems without
 * bus-snooping caches, these buffers are uncached.
 *
 * When the buffer is no longer used, free it with usb_free_coherent().
 */
void *usb_alloc_coherent(struct usb_device *dev, size_t size, gfp_t mem_flags,
			 dma_addr_t *dma)
{
    uint32_t addr;

	if (!dev || !dev->bus)
		return NULL;

    addr = itpVmemAlloc(size);
    if(addr) {
        if(dma)
            (*dma) = (dma_addr_t)addr;
    } else
        LOG_ERROR " %s: failed (size: %d) \n", __func__, size LOG_END

	return (void *)addr;
}

/**
 * usb_free_coherent - free memory allocated with usb_alloc_coherent()
 * @dev: device the buffer was used with
 * @size: requested buffer size
 * @addr: CPU address of buffer
 * @dma: DMA address of buffer
 *
 * This reclaims an I/O buffer, letting it be reused.  The memory must have
 * been allocated using usb_alloc_coherent(), and the parameters must match
 * those provided in that allocation request.
 */
void usb_free_coherent(struct usb_device *dev, size_t size, void *addr,
		       dma_addr_t dma)
{
	if (!dev || !dev->bus)
		return;
	if (!addr)
		return;

    itpVmemFree((uint32_t)addr);
}


