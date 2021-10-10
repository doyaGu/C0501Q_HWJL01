/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE USB HCD Driver API header file.
 *
 * @author Irene Lin
 */

#ifndef USB_EX_H
#define USB_EX_H

#ifdef __cplusplus
extern "C" {
#endif


#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/kref.h>
#include <linux/device.h>
#include <linux/os.h>
#include <linux/wait.h>
#include "ch9.h"


struct usb_device;
struct usb_driver;

//=============================================================================
//                              USB Constants
//=============================================================================

/*
 * HID requests
 */
#define USB_REQ_GET_REPORT		    0x01
#define USB_REQ_GET_IDLE		    0x02
#define USB_REQ_GET_PROTOCOL		0x03
#define USB_REQ_SET_REPORT		    0x09
#define USB_REQ_SET_IDLE		    0x0A
#define USB_REQ_SET_PROTOCOL		0x0B



//=============================================================================
//                              USB Descriptor
//=============================================================================


/**
 * struct usb_host_endpoint - host-side endpoint descriptor and queue
 * @desc: descriptor for this endpoint, wMaxPacketSize in native byteorder
 * @ss_ep_comp: SuperSpeed companion descriptor for this endpoint
 * @urb_list: urbs queued to this endpoint; maintained by usbcore
 * @hcpriv: for use by HCD; typically holds hardware dma queue head (QH)
 *	with one or more transfer descriptors (TDs) per urb
 * @ep_dev: ep_device for sysfs info
 * @extra: descriptors following this endpoint in the configuration
 * @extralen: how many bytes of "extra" are valid
 * @enabled: URBs may be submitted to this endpoint
 *
 * USB requests are always queued to a given endpoint, identified by a
 * descriptor within an active interface in a given USB configuration.
 */
struct usb_host_endpoint {
	struct usb_endpoint_descriptor		desc;
	struct usb_ss_ep_comp_descriptor	ss_ep_comp;
	struct list_head		urb_list;
	void				*hcpriv;

	unsigned char *extra;   /* Extra descriptors */
	int extralen;
	int enabled;
};

/* host-side wrapper for one interface setting's parsed descriptors */
struct usb_host_interface {
	struct usb_interface_descriptor	desc;

	int extralen;
	unsigned char *extra;   /* Extra descriptors */

	/* array of desc.bNumEndpoint endpoints associated with this
	 * interface setting.  these will be in no particular order.
	 */
	struct usb_host_endpoint *endpoint;

	char *string;		/* iInterface string, if present */
};

enum usb_interface_condition {
	USB_INTERFACE_UNBOUND = 0,
	USB_INTERFACE_BINDING,
	USB_INTERFACE_BOUND,
	USB_INTERFACE_UNBINDING,
};

/**
 * struct usb_interface - what usb device drivers talk to
 * @altsetting: array of interface structures, one for each alternate
 *	setting that may be selected.  Each one includes a set of
 *	endpoint configurations.  They will be in no particular order.
 * @cur_altsetting: the current altsetting.
 * @num_altsetting: number of altsettings defined.
 * @intf_assoc: interface association descriptor
 * @minor: the minor number assigned to this interface, if this
 *	interface is bound to a driver that uses the USB major number.
 *	If this interface does not use the USB major, this field should
 *	be unused.  The driver should set this value in the probe()
 *	function of the driver, after it has been assigned a minor
 *	number from the USB core by calling usb_register_dev().
 * @condition: binding state of the interface: not bound, binding
 *	(in probe()), bound to a driver, or unbinding (in disconnect())
 * @sysfs_files_created: sysfs attributes exist
 * @ep_devs_created: endpoint child pseudo-devices exist
 * @unregistering: flag set when the interface is being unregistered
 * @needs_remote_wakeup: flag set when the driver requires remote-wakeup
 *	capability during autosuspend.
 * @needs_altsetting0: flag set when a set-interface request for altsetting 0
 *	has been deferred.
 * @needs_binding: flag set when the driver should be re-probed or unbound
 *	following a reset or suspend operation it doesn't support.
 * @dev: driver model's view of this device
 * @usb_dev: if an interface is bound to the USB major, this will point
 *	to the sysfs representation for that device.
 * @pm_usage_cnt: PM usage counter for this interface
 * @reset_ws: Used for scheduling resets from atomic context.
 * @reset_running: set to 1 if the interface is currently running a
 *      queued reset so that usb_cancel_queued_reset() doesn't try to
 *      remove from the workqueue when running inside the worker
 *      thread. See __usb_queue_reset_device().
 * @resetting_device: USB core reset the device, so use alt setting 0 as
 *	current; needs bandwidth alloc after reset.
 *
 * USB device drivers attach to interfaces on a physical device.  Each
 * interface encapsulates a single high level function, such as feeding
 * an audio stream to a speaker or reporting a change in a volume control.
 * Many USB devices only have one interface.  The protocol used to talk to
 * an interface's endpoints can be defined in a usb "class" specification,
 * or by a product's vendor.  The (default) control endpoint is part of
 * every interface, but is never listed among the interface's descriptors.
 *
 * The driver that is bound to the interface can use standard driver model
 * calls such as dev_get_drvdata() on the dev member of this structure.
 *
 * Each interface may have alternate settings.  The initial configuration
 * of a device sets altsetting 0, but the device driver can change
 * that setting using usb_set_interface().  Alternate settings are often
 * used to control the use of periodic endpoints, such as by having
 * different endpoints use different amounts of reserved USB bandwidth.
 * All standards-conformant USB devices that use isochronous endpoints
 * will use them in non-default settings.
 *
 * The USB specification says that alternate setting numbers must run from
 * 0 to one less than the total number of alternate settings.  But some
 * devices manage to mess this up, and the structures aren't necessarily
 * stored in numerical order anyhow.  Use usb_altnum_to_altsetting() to
 * look up an alternate setting in the altsetting array based on its number.
 */
struct usb_interface {
	/* array of alternate settings for this interface,
	 * stored in no particular order */
	struct usb_host_interface *altsetting;

	struct usb_host_interface *cur_altsetting;	/* the currently
					 * active alternate setting */
	unsigned num_altsetting;	/* number of alternate settings */

	/* If there is an interface association descriptor then it will list
	 * the associated interfaces */
	struct usb_interface_assoc_descriptor *intf_assoc;

	int minor;			/* minor number this interface is
					 * bound to */
	enum usb_interface_condition condition;		/* state of binding */
//	unsigned sysfs_files_created:1;	/* the sysfs attributes exist */
//	unsigned ep_devs_created:1;	/* endpoint "devices" exist */
	unsigned unregistering:1;	/* unregistration is in progress */
//	unsigned needs_remote_wakeup:1;	/* driver requires remote wakeup */
//	unsigned needs_altsetting0:1;	/* switch to altsetting 0 is pending */
//	unsigned needs_binding:1;	/* needs delayed unbind/rebind */
//	unsigned reset_running:1;
	unsigned resetting_device:1;	/* true: bandwidth alloc after reset */

	struct device dev;		/* interface specific device info */
//	struct device *usb_dev;
	struct usb_device *usb_dev;
//	atomic_t pm_usage_cnt;		/* usage counter for autosuspend */
//	struct work_struct reset_ws;	/* for resets in atomic context */

    struct usb_driver *driver;
    void*  private_data;
};
#define	to_usb_interface(d) container_of(d, struct usb_interface, dev)

static inline void *usb_get_intfdata(struct usb_interface *intf)
{
	return intf->private_data;
}

static inline void usb_set_intfdata(struct usb_interface *intf, void *data)
{
	intf->private_data = data;
}

struct usb_interface *usb_get_intf(struct usb_interface *intf);
void usb_put_intf(struct usb_interface *intf);

/* this maximum is arbitrary */
#define USB_MAXINTERFACES	32
#define USB_MAXIADS		(USB_MAXINTERFACES/2)


/**
 * struct usb_interface_cache - long-term representation of a device interface
 * @num_altsetting: number of altsettings defined.
 * @ref: reference counter.
 * @altsetting: variable-length array of interface structures, one for
 *	each alternate setting that may be selected.  Each one includes a
 *	set of endpoint configurations.  They will be in no particular order.
 *
 * These structures persist for the lifetime of a usb_device, unlike
 * struct usb_interface (which persists only as long as its configuration
 * is installed).  The altsetting arrays can be accessed through these
 * structures at any time, permitting comparison of configurations and
 * providing support for the /proc/bus/usb/devices pseudo-file.
 */
struct usb_interface_cache {
	unsigned num_altsetting;	/* number of alternate settings */
	struct kref ref;		/* reference counter */

	/* variable-length array of alternate settings for this interface,
	 * stored in no particular order */
	struct usb_host_interface altsetting[0];
};
#define	ref_to_usb_interface_cache(r) \
		container_of(r, struct usb_interface_cache, ref)
#define	altsetting_to_usb_interface_cache(a) \
		container_of(a, struct usb_interface_cache, altsetting[0])


/**
 * struct usb_host_config - representation of a device's configuration
 * @desc: the device's configuration descriptor.
 * @string: pointer to the cached version of the iConfiguration string, if
 *	present for this configuration.
 * @intf_assoc: list of any interface association descriptors in this config
 * @interface: array of pointers to usb_interface structures, one for each
 *	interface in the configuration.  The number of interfaces is stored
 *	in desc.bNumInterfaces.  These pointers are valid only while the
 *	the configuration is active.
 * @intf_cache: array of pointers to usb_interface_cache structures, one
 *	for each interface in the configuration.  These structures exist
 *	for the entire life of the device.
 * @extra: pointer to buffer containing all extra descriptors associated
 *	with this configuration (those preceding the first interface
 *	descriptor).
 * @extralen: length of the extra descriptors buffer.
 *
 * USB devices may have multiple configurations, but only one can be active
 * at any time.  Each encapsulates a different operational environment;
 * for example, a dual-speed device would have separate configurations for
 * full-speed and high-speed operation.  The number of configurations
 * available is stored in the device descriptor as bNumConfigurations.
 *
 * A configuration can contain multiple interfaces.  Each corresponds to
 * a different function of the USB device, and all are available whenever
 * the configuration is active.  The USB standard says that interfaces
 * are supposed to be numbered from 0 to desc.bNumInterfaces-1, but a lot
 * of devices get this wrong.  In addition, the interface array is not
 * guaranteed to be sorted in numerical order.  Use usb_ifnum_to_if() to
 * look up an interface entry based on its number.
 *
 * Device drivers should not attempt to activate configurations.  The choice
 * of which configuration to install is a policy decision based on such
 * considerations as available power, functionality provided, and the user's
 * desires (expressed through userspace tools).  However, drivers can call
 * usb_reset_configuration() to reinitialize the current configuration and
 * all its interfaces.
 */
struct usb_host_config {
	struct usb_config_descriptor	desc;

	char *string;		/* iConfiguration string, if present */

	/* List of any Interface Association Descriptors in this
	 * configuration. */
	struct usb_interface_assoc_descriptor *intf_assoc[USB_MAXIADS];

	/* the interfaces associated with this configuration,
	 * stored in no particular order */
	struct usb_interface *interface[USB_MAXINTERFACES];

	/* Interface information available even when this is not the
	 * active configuration */
	struct usb_interface_cache *intf_cache[USB_MAXINTERFACES];

	unsigned char *extra;   /* Extra descriptors */
	int extralen;
};

/* USB2.0 and USB3.0 device BOS descriptor set */
struct usb_host_bos {
	struct usb_bos_descriptor	*desc;

	/* wireless cap descriptor is handled by wusb */
	struct usb_ext_cap_descriptor	*ext_cap;
	struct usb_ss_cap_descriptor	*ss_cap;
	struct usb_ss_container_id_descriptor	*ss_id;
};

int __usb_get_extra_descriptor(char *buffer, unsigned size,
	unsigned char type, void **ptr);
#define usb_get_extra_descriptor(ifpoint, type, ptr) \
				__usb_get_extra_descriptor((ifpoint)->extra, \
				(ifpoint)->extralen, \
				type, (void **)ptr)

/* ----------------------------------------------------------------------- */

/*
 * Allocated per bus we have
 */
struct usb_bus {
    int                 busnum;			/* Bus number (in order of reg) */

    struct usb_device*  root_device;    /* Root device */
    uint32_t          devmap;           /* Device map */

    int             bandwidth_allocated;  /* on this Host Controller; */
                                              /* applies to Int. and Isoc. pipes; */
                                              /* measured in microseconds/frame; */
                                              /* range is 0..900, where 900 = */
                                              /* 90% of a 1-millisecond frame */
    int             bandwidth_int_reqs;	  /* number of Interrupt requesters */
    int             bandwidth_isoc_reqs;  /* number of Isoc. requesters */
};


/*
 * As of USB 2.0, full/low speed devices are segregated into trees.
 * One type grows from USB 1.1 host controllers (OHCI, UHCI etc).
 * The other type grows from high speed hubs when they connect to
 * full/low speed devices using "Transaction Translators" (TTs).
 *
 * TTs should only be known to the hub driver, and high speed bus
 * drivers (only EHCI for now).  They affect periodic scheduling and
 * sometimes control/bulk error recovery.
 */
struct usb_tt {
    struct usb_device	*hub;	/* upstream highspeed hub */
    int			multi;	/* true means one TT per port */
};

enum usb_device_removable {
	USB_DEVICE_REMOVABLE_UNKNOWN = 0,
	USB_DEVICE_REMOVABLE,
	USB_DEVICE_FIXED,
};

enum usb_port_connect_type {
	USB_PORT_CONNECT_TYPE_UNKNOWN = 0,
	USB_PORT_CONNECT_TYPE_HOT_PLUG,
	USB_PORT_CONNECT_TYPE_HARD_WIRED,
	USB_PORT_NOT_USED,
};

struct usb_device {
    int         devnum;			/* Device number on USB bus */
    int         type;

	pthread_mutex_t		mutex;

    enum usb_device_state state;
    enum usb_device_speed speed;

    struct usb_tt*	tt;
    int		    ttport;

    uint32_t  toggle[2];		/* one bit for each endpoint ([0] = IN, [1] = OUT) */

    struct usb_device*  parent;
    struct usb_bus*     bus;
    struct usb_host_endpoint ep0;

    struct device   dev;

    struct usb_device_descriptor  descriptor;
    //struct usb_host_bos *bos;
    struct usb_host_config *config;

    struct usb_host_config *actconfig;
    struct usb_host_endpoint *ep_in[16];
    struct usb_host_endpoint *ep_out[16];

    uint8_t **rawdescriptors;

    uint32_t    can_submit:1;
	uint32_t    authorized:1;
    uint32_t    have_langid:1;
    int     string_langid;		/* language ID for strings */

	/* static strings from the device */
	uint8_t *product;
	uint8_t *manufacturer;
	uint8_t *serial;

    uint32_t quirks;
    atomic_t urbnum;

    enum usb_device_removable removable;

    USB_DEVICE_INFO device_info[3];
	int		drivernum;
};
#define	to_usb_device(d) container_of(d, struct usb_device, dev)

static inline struct usb_device *interface_to_usbdev(struct usb_interface *intf)
{
	return intf->usb_dev;
}

static inline struct usb_device *usb_get_dev(struct usb_device *dev) { return dev; }
static inline void usb_put_dev(struct usb_device *dev) { }

/* USB device locking */
#define usb_lock_device(udev)		pthread_mutex_lock(&(udev)->mutex)
#define usb_unlock_device(udev)		pthread_mutex_unlock(&(udev)->mutex)
#define usb_trylock_device(udev)	pthread_mutex_trylock(&(udev)->mutex)


/* USB autosuspend and autoresume */
#if 1
static inline int usb_enable_autosuspend(struct usb_device *udev)
{ return 0; }
static inline int usb_disable_autosuspend(struct usb_device *udev)
{ return 0; }

static inline int usb_autopm_get_interface(struct usb_interface *intf)
{ return 0; }
static inline int usb_autopm_get_interface_async(struct usb_interface *intf)
{ return 0; }

static inline void usb_autopm_put_interface(struct usb_interface *intf)
{ }
static inline void usb_autopm_put_interface_async(struct usb_interface *intf)
{ }
static inline void usb_autopm_get_interface_no_resume(
		struct usb_interface *intf)
{ }
static inline void usb_autopm_put_interface_no_suspend(
		struct usb_interface *intf)
{ }
static inline void usb_mark_last_busy(struct usb_device *udev)
{ }
#endif // #if 1

struct usb_device* usb_alloc_dev(struct usb_device* parent, struct usb_bus* bus);
void usb_release_dev(struct usb_device* dev);
void choose_address(struct usb_device* dev);
void usb_disconnect(struct usb_device **pdev);


/*-------------------------------------------------------------------------*/

/* for drivers using iso endpoints */
extern int usb_get_current_frame_number(struct usb_device *usb_dev);

/* Sets up a group of bulk endpoints to support multiple stream IDs. */
extern int usb_alloc_streams(struct usb_interface *interface,
		struct usb_host_endpoint **eps, unsigned int num_eps,
		unsigned int num_streams, gfp_t mem_flags);

/* Reverts a group of bulk endpoints back to not using stream IDs. */
extern int usb_free_streams(struct usb_interface *interface,
		struct usb_host_endpoint **eps, unsigned int num_eps,
		gfp_t mem_flags);

extern int usb_dev_exist(struct usb_device* usb_dev);

/* used these for multi-interface device registration */
extern int usb_driver_claim_interface(struct usb_driver *driver,
			struct usb_interface *iface, void *priv);

/**
 * usb_interface_claimed - returns true iff an interface is claimed
 * @iface: the interface being checked
 *
 * Return: %true (nonzero) iff the interface is claimed, else %false
 * (zero).
 *
 * Note:
 * Callers must own the driver model's usb bus readlock.  So driver
 * probe() entries don't need extra locking, but other call contexts
 * may need to explicitly claim that lock.
 *
 */
static inline int usb_interface_claimed(struct usb_interface *iface)
{
	return (iface->driver != NULL);
}

extern void usb_driver_release_interface(struct usb_driver *driver,
			struct usb_interface *iface);
extern const struct usb_device_id* usb_match_id( struct usb_interface *interface,
                                                   const struct usb_device_id *id);
extern int usb_match_one_id(struct usb_interface *interface,
			    const struct usb_device_id *id);
extern struct usb_interface *usb_find_interface(struct usb_driver *drv,
		int minor);
extern struct usb_interface *usb_ifnum_to_if(const struct usb_device *dev,
		unsigned ifnum);
extern struct usb_host_interface *usb_altnum_to_altsetting(
		const struct usb_interface *intf, unsigned int altnum);
extern struct usb_host_interface *usb_find_alt_setting(
		struct usb_host_config *config,
		unsigned int iface_num,
		unsigned int alt_num);


/* ----------------------------------------------------------------------- */

struct usb_device_id {
    /* This bitmask is used to determine which of the following fields
     * are to be used for matching.
     */
    uint16_t		match_flags;

    /*
     * vendor/product codes are checked, if vendor is nonzero
     * Range is for device revision (bcdDevice), inclusive;
     * zero values here mean range isn't considered
     */
    uint16_t		idVendor;
    uint16_t		idProduct;
    uint16_t		bcdDevice_lo, bcdDevice_hi;

    /*
     * if device class != 0, these can be match criteria;
     * but only if this bDeviceClass value is nonzero
     */
    uint8_t		bDeviceClass;
    uint8_t		bDeviceSubClass;
    uint8_t		bDeviceProtocol;

    /*
     * if interface class != 0, these can be match criteria;
     * but only if this bInterfaceClass value is nonzero
     */
    uint8_t		bInterfaceClass;
    uint8_t		bInterfaceSubClass;
    uint8_t		bInterfaceProtocol;

    /* Used for vendor-specific interface matches */
	uint8_t		bInterfaceNumber;

	/* not matched against */
	void*   	driver_info;
};


/* Some useful macros to use to create struct usb_device_id */
#define USB_DEVICE_ID_MATCH_VENDOR		    0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT		    0x0002
#define USB_DEVICE_ID_MATCH_DEV_LO		    0x0004
#define USB_DEVICE_ID_MATCH_DEV_HI		    0x0008
#define USB_DEVICE_ID_MATCH_DEV_CLASS		0x0010
#define USB_DEVICE_ID_MATCH_DEV_SUBCLASS	0x0020
#define USB_DEVICE_ID_MATCH_DEV_PROTOCOL	0x0040
#define USB_DEVICE_ID_MATCH_INT_CLASS		0x0080
#define USB_DEVICE_ID_MATCH_INT_SUBCLASS	0x0100
#define USB_DEVICE_ID_MATCH_INT_PROTOCOL	0x0200
#define USB_DEVICE_ID_MATCH_INT_NUMBER		0x0400


#define USB_DEVICE_ID_MATCH_DEVICE \
		(USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)
#define USB_DEVICE_ID_MATCH_DEV_RANGE \
		(USB_DEVICE_ID_MATCH_DEV_LO | USB_DEVICE_ID_MATCH_DEV_HI)
#define USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION \
		(USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_DEV_RANGE)
#define USB_DEVICE_ID_MATCH_DEV_INFO \
		(USB_DEVICE_ID_MATCH_DEV_CLASS | \
		USB_DEVICE_ID_MATCH_DEV_SUBCLASS | \
		USB_DEVICE_ID_MATCH_DEV_PROTOCOL)
#define USB_DEVICE_ID_MATCH_INT_INFO \
		(USB_DEVICE_ID_MATCH_INT_CLASS | \
		USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
		USB_DEVICE_ID_MATCH_INT_PROTOCOL)


/**
 * USB_DEVICE - macro used to describe a specific usb device
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device.
 */
#define USB_DEVICE(vend, prod) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE, \
	.idVendor = (vend), \
	.idProduct = (prod)

/**
 * USB_DEVICE_VER - describe a specific usb device with a version range
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 * @lo: the bcdDevice_lo value
 * @hi: the bcdDevice_hi value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device, with a version range.
 */
#define USB_DEVICE_VER(vend, prod, lo, hi) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bcdDevice_lo = (lo), \
	.bcdDevice_hi = (hi)

/**
 * USB_DEVICE_INTERFACE_CLASS - describe a usb device with a specific interface class
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 * @cl: bInterfaceClass value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific interface class of devices.
 */
#define USB_DEVICE_INTERFACE_CLASS(vend, prod, cl) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | \
		       USB_DEVICE_ID_MATCH_INT_CLASS, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bInterfaceClass = (cl)

/**
 * USB_DEVICE_INTERFACE_PROTOCOL - describe a usb device with a specific interface protocol
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 * @pr: bInterfaceProtocol value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific interface protocol of devices.
 */
#define USB_DEVICE_INTERFACE_PROTOCOL(vend, prod, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | \
		       USB_DEVICE_ID_MATCH_INT_PROTOCOL, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bInterfaceProtocol = (pr)

/**
 * USB_DEVICE_INTERFACE_NUMBER - describe a usb device with a specific interface number
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 * @num: bInterfaceNumber value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific interface number of devices.
 */
#define USB_DEVICE_INTERFACE_NUMBER(vend, prod, num) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | \
		       USB_DEVICE_ID_MATCH_INT_NUMBER, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bInterfaceNumber = (num)

/**
 * USB_DEVICE_INFO - macro used to describe a class of usb devices
 * @cl: bDeviceClass value
 * @sc: bDeviceSubClass value
 * @pr: bDeviceProtocol value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific class of devices.
 */
#define USB_DEVICE_INFO(cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_DEV_INFO, \
	.bDeviceClass = (cl), \
	.bDeviceSubClass = (sc), \
	.bDeviceProtocol = (pr)

/**
 * USB_INTERFACE_INFO - macro used to describe a class of usb interfaces
 * @cl: bInterfaceClass value
 * @sc: bInterfaceSubClass value
 * @pr: bInterfaceProtocol value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific class of interfaces.
 */
#define USB_INTERFACE_INFO(cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO, \
	.bInterfaceClass = (cl), \
	.bInterfaceSubClass = (sc), \
	.bInterfaceProtocol = (pr)

/**
 * USB_DEVICE_AND_INTERFACE_INFO - describe a specific usb device with a class of usb interfaces
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 * @cl: bInterfaceClass value
 * @sc: bInterfaceSubClass value
 * @pr: bInterfaceProtocol value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device with a specific class of interfaces.
 *
 * This is especially useful when explicitly matching devices that have
 * vendor specific bDeviceClass values, but standards-compliant interfaces.
 */
#define USB_DEVICE_AND_INTERFACE_INFO(vend, prod, cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO \
		| USB_DEVICE_ID_MATCH_DEVICE, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bInterfaceClass = (cl), \
	.bInterfaceSubClass = (sc), \
	.bInterfaceProtocol = (pr)

/**
 * USB_VENDOR_AND_INTERFACE_INFO - describe a specific usb vendor with a class of usb interfaces
 * @vend: the 16 bit USB Vendor ID
 * @cl: bInterfaceClass value
 * @sc: bInterfaceSubClass value
 * @pr: bInterfaceProtocol value
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific vendor with a specific class of interfaces.
 *
 * This is especially useful when explicitly matching devices that have
 * vendor specific bDeviceClass values, but standards-compliant interfaces.
 */
#define USB_VENDOR_AND_INTERFACE_INFO(vend, cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO \
		| USB_DEVICE_ID_MATCH_VENDOR, \
	.idVendor = (vend), \
	.bInterfaceClass = (cl), \
	.bInterfaceSubClass = (sc), \
	.bInterfaceProtocol = (pr)

/* ----------------------------------------------------------------------- */


struct usb_driver {
    const char *name;
    uint32_t flags;

    void *(*probe)(
        struct usb_interface *intf,		/* what interface */
        const struct usb_device_id *id	/* from id_table */
        );
    int (*open)(void *);
    void (*disconnect)(struct usb_interface *intf);

    /* ioctl -- userspace apps can talk to drivers through usbdevfs */
    int (*ioctl)(struct usb_device *dev, uint32_t code, void *buf);

    /* support for "new-style" USB hotplugging
     * binding policy can be driven from user mode too
     */
    const struct usb_device_id *id_table;

    struct list_head driver_list;
};

extern int usb_register(struct usb_driver *new_driver);
extern int usb_find_drivers(struct usb_device *dev, struct usb_interface *intf);

//=============================================================================
//                     USB Request Packet(URB) Related
//=============================================================================
/*
 * URB support, for asynchronous request completions
 */

/*
 * urb->transfer_flags:
 *
 * Note: URB_DIR_IN/OUT is automatically set in usb_submit_urb().
 */
#define URB_SHORT_NOT_OK	0x0001	/* report short reads as errors */
#define URB_ISO_ASAP		0x0002	/* iso-only, urb->start_frame ignored */

#define URB_NO_TRANSFER_DMA_MAP	0x0004	/* urb->transfer_dma valid on submit */
#define URB_NO_FSBR		0x0020	/* UHCI-specific */

#define URB_ZERO_PACKET		0x0040	/* Finish bulk OUT with short packet */
#define URB_NO_INTERRUPT	0x0080	/* HINT: no non-error interrupt needed */
#define URB_FREE_BUFFER		0x0100	/* Free transfer buffer with the URB */

/* The following flags are used internally by usbcore and HCDs */
#define URB_DIR_IN		    0x0200	/* Transfer from device to host */
#define URB_DIR_OUT		    0
#define URB_DIR_MASK		URB_DIR_IN

struct usb_iso_packet_descriptor {
	unsigned int offset;
	unsigned int length;		/* expected length */
	unsigned int actual_length;
	int status;
};


struct urb;

struct usb_anchor {
	struct list_head urb_list;
	wait_queue_head_t wait;
	spinlock_t lock;
	atomic_t suspend_wakeups;
	unsigned int poisoned:1;
};

static inline void init_usb_anchor(struct usb_anchor *anchor)
{
	memset(anchor, 0, sizeof(*anchor));
	INIT_LIST_HEAD(&anchor->urb_list);
    init_waitqueue_head(&anchor->wait);
	spin_lock_init(&anchor->lock);
}

typedef void (*usb_complete_t)(struct urb *);

typedef struct urb
{
#define URB_BUF_COOKIE	0x20140422
    uint32_t            cookies;
    struct kref         kref;		/* reference count of the URB */
    int                 type;
    spinlock_t          lock;		    // lock for the URB
    void*               hcpriv;	        // private data for host controller
    atomic_t            use_count;      /* concurrent submissions counter */
    atomic_t            reject;         /* submissions will fail */
    int                 unlinked;       /* unlink error code */

    struct list_head    urb_list;	    // list pointer to all active urbs 
    struct list_head    anchor_list;	/* the URB may be anchored */
    struct usb_anchor   *anchor;

    struct usb_device*  dev;		    // pointer to associated USB device
    struct usb_host_endpoint *ep;       /* (internal) pointer to endpoint */

    uint32_t        pipe;	   	    // pipe information
	unsigned int 	stream_id;		/* (in) stream ID */
	int             status;		    // returned status
    uint32_t        transfer_flags;	        // USB_DISABLE_SPD | USB_ISO_ASAP | etc.
    void*           transfer_buffer;		// associated data buffer
    dma_addr_t      transfer_dma;
    int             transfer_buffer_length;	// data buffer length
    int             actual_length;          // actual data buffer length	
    int             bandwidth;			    // bandwidth for this transfer request (INT or ISO)
    uint8_t*          setup_packet;	        // setup packet (control only)
    dma_addr_t      setup_dma;
    //
    int             start_frame;		// start frame (iso/irq only)
    int             number_of_packets;	// number of packets in this request (iso)
    int             interval;           // polling interval (irq only)
    int             error_count;		// number of errors in this transfer (iso only)
    int             timeout;			// timeout (in jiffies)
    //
    void*               context;			// context for completion routine
    usb_complete_t      complete;	        // pointer to completion routine
    uint32_t          reserved;
    //
    struct usb_iso_packet_descriptor iso_frame_desc[0];
} urb_t, *purb_t;

/* ----------------------------------------------------------------------- */

/**
 * usb_fill_control_urb - initializes a control urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @setup_packet: pointer to the setup_packet buffer
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete_fn: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 *
 * Initializes a control urb with the proper information needed to submit
 * it to a device.
 */
static inline void usb_fill_control_urb(struct urb *urb,
					struct usb_device *dev,
					unsigned int pipe,
					unsigned char *setup_packet,
					void *transfer_buffer,
					int buffer_length,
					usb_complete_t complete_fn,
					void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->setup_packet = setup_packet;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}

/**
 * usb_fill_bulk_urb - macro to help initialize a bulk urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete_fn: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 *
 * Initializes a bulk urb with the proper information needed to submit it
 * to a device.
 */
static inline void usb_fill_bulk_urb(struct urb *urb,
				     struct usb_device *dev,
				     unsigned int pipe,
				     void *transfer_buffer,
				     int buffer_length,
				     usb_complete_t complete_fn,
				     void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}
#define FILL_BULK_URB   usb_fill_bulk_urb
    
/**
 * usb_fill_int_urb - macro to help initialize a interrupt urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete_fn: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 * @interval: what to set the urb interval to, encoded like
 *	the endpoint descriptor's bInterval value.
 *
 * Initializes a interrupt urb with the proper information needed to submit
 * it to a device.
 *
 * Note that High Speed and SuperSpeed interrupt endpoints use a logarithmic
 * encoding of the endpoint interval, and express polling intervals in
 * microframes (eight per millisecond) rather than in frames (one per
 * millisecond).
 *
 * Wireless USB also uses the logarithmic encoding, but specifies it in units of
 * 128us instead of 125us.  For Wireless USB devices, the interval is passed
 * through to the host controller, rather than being translated into microframe
 * units.
 */
static inline void usb_fill_int_urb(struct urb *urb,
				    struct usb_device *dev,
				    unsigned int pipe,
				    void *transfer_buffer,
				    int buffer_length,
				    usb_complete_t complete_fn,
				    void *context,
				    int interval)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;

	if (dev->speed == USB_SPEED_HIGH || dev->speed == USB_SPEED_SUPER) {
		/* make sure interval is within allowed range */
		interval = clamp(interval, 1, 16);

		urb->interval = 1 << (interval - 1);
	} else {
		urb->interval = interval;
	}

	urb->start_frame = -1;
}
#define FILL_INT_URB    usb_fill_int_urb

                    
extern void usb_init_urb(struct urb *urb);
extern struct urb *usb_alloc_urb(int iso_packets);
extern void usb_free_urb(struct urb *urb);
#define usb_put_urb usb_free_urb
extern struct urb *usb_get_urb(struct urb *urb);
extern int usb_submit_urb(struct urb *urb);
extern int usb_unlink_urb(struct urb *urb);
extern void usb_kill_urb(struct urb *urb);
extern void usb_poison_urb(struct urb *urb);
extern void usb_unpoison_urb(struct urb *urb);
extern void usb_block_urb(struct urb *urb);
extern void usb_kill_anchored_urbs(struct usb_anchor *anchor);
extern void usb_poison_anchored_urbs(struct usb_anchor *anchor);
extern void usb_unpoison_anchored_urbs(struct usb_anchor *anchor);
extern void usb_unlink_anchored_urbs(struct usb_anchor *anchor);
extern void usb_anchor_suspend_wakeups(struct usb_anchor *anchor);
extern void usb_anchor_resume_wakeups(struct usb_anchor *anchor);
extern void usb_anchor_urb(struct urb *urb, struct usb_anchor *anchor);
extern void usb_unanchor_urb(struct urb *urb);
extern int usb_wait_anchor_empty_timeout(struct usb_anchor *anchor,
					 unsigned int timeout);
extern struct urb *usb_get_from_anchor(struct usb_anchor *anchor);
extern void usb_scuttle_anchored_urbs(struct usb_anchor *anchor);
extern int usb_anchor_empty(struct usb_anchor *anchor);

#define usb_unblock_urb	usb_unpoison_urb

/**
 * usb_urb_dir_in - check if an URB describes an IN transfer
 * @urb: URB to be checked
 *
 * Returns 1 if @urb describes an IN transfer (device-to-host),
 * otherwise 0.
 */
static inline int usb_urb_dir_in(struct urb *urb)
{
	return (urb->transfer_flags & URB_DIR_MASK) == URB_DIR_IN;
}

/**
 * usb_urb_dir_out - check if an URB describes an OUT transfer
 * @urb: URB to be checked
 *
 * Returns 1 if @urb describes an OUT transfer (host-to-device),
 * otherwise 0.
 */
static inline int usb_urb_dir_out(struct urb *urb)
{
	return (urb->transfer_flags & URB_DIR_MASK) == URB_DIR_OUT;
}

void *usb_alloc_coherent(struct usb_device *dev, size_t size,
	gfp_t mem_flags, dma_addr_t *dma);
void usb_free_coherent(struct usb_device *dev, size_t size,
	void *addr, dma_addr_t dma);


/*-------------------------------------------------------------------*
 *                         SYNCHRONOUS CALL SUPPORT                  *
 *-------------------------------------------------------------------*/

 extern int usb_control_msg(struct usb_device *dev, unsigned int pipe,
	 __u8 request, __u8 requesttype, __u16 value, __u16 index,
	 void *data, __u16 size, int timeout);
 extern int usb_interrupt_msg(struct usb_device *usb_dev, unsigned int pipe,
	 void *data, int len, int *actual_length, int timeout);
 extern int usb_bulk_msg(struct usb_device *usb_dev, unsigned int pipe,
	 void *data, int len, int *actual_length,
	 int timeout);

/* wrappers around usb_control_msg() for the most common standard requests */
extern int usb_get_descriptor(struct usb_device *dev, uint8_t desctype,
	uint8_t descindex, void *buf, int size);
extern int usb_get_status(struct usb_device *dev,
	int type, int target, void *data);
extern int usb_string(struct usb_device *dev, int index, char *buf, size_t size);

/* wrappers that also update important state inside usbcore */
extern int usb_clear_halt(struct usb_device *dev, int pipe);
extern int usb_reset_configuration(struct usb_device *dev);
extern int usb_set_interface(struct usb_device *dev, int ifnum, int alternate);
extern void usb_reset_endpoint(struct usb_device *dev, unsigned int epaddr);

/* this request isn't really synchronous, but it belongs with the others */
extern int usb_driver_set_configuration(struct usb_device *udev, int config);

/*
 * timeouts, in milliseconds, used for sending/receiving control messages
 * they typically complete within a few frames (msec) after they're issued
 * USB identifies 5 second timeouts, maybe more in a few cases, and a few
 * slow devices (like some MGE Ellipse UPSes) actually push that limit.
 */
#define USB_CTRL_GET_TIMEOUT	5000
#define USB_CTRL_SET_TIMEOUT	5000



//=============================================================================
//                     Pipe Related
//=============================================================================
/*
 * For various legacy reasons, Linux has a small cookie that's paired with
 * a struct usb_device to identify an endpoint queue.  Queue characteristics
 * are defined by the endpoint's descriptor.  This cookie is called a "pipe",
 * an unsigned int encoded as:
 *
 *  - direction:	bit 7		(0 = Host-to-Device [Out],
 *					 1 = Device-to-Host [In] ...
 *					like endpoint bEndpointAddress)
 *  - device address:	bits 8-14       ... bit positions known to uhci-hcd
 *  - endpoint:		bits 15-18      ... bit positions known to uhci-hcd
 *  - pipe type:	bits 30-31	(00 = isochronous, 01 = interrupt,
 *					 10 = control, 11 = bulk)
 *
 * Given the device address and endpoint descriptor, pipes are redundant.
 */

/* NOTE:  these are not the standard USB_ENDPOINT_XFER_* values!! */
/* (yet ... they're the values used by usbfs) */
#define PIPE_ISOCHRONOUS		0
#define PIPE_INTERRUPT			1
#define PIPE_CONTROL			2
#define PIPE_BULK			3

#define usb_pipein(pipe)	((pipe) & USB_DIR_IN)
#define usb_pipeout(pipe)	(!usb_pipein(pipe))

#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)

#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	(usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	(usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	(usb_pipetype((pipe)) == PIPE_BULK)

static inline unsigned int __create_pipe(struct usb_device *dev,
		unsigned int endpoint)
{
	return (dev->devnum << 8) | (endpoint << 15);
}

/* Create various pipes... */
#define usb_sndctrlpipe(dev, endpoint)	\
	((PIPE_CONTROL << 30) | __create_pipe(dev, endpoint))
#define usb_rcvctrlpipe(dev, endpoint)	\
	((PIPE_CONTROL << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndisocpipe(dev, endpoint)	\
	((PIPE_ISOCHRONOUS << 30) | __create_pipe(dev, endpoint))
#define usb_rcvisocpipe(dev, endpoint)	\
	((PIPE_ISOCHRONOUS << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndbulkpipe(dev, endpoint)	\
	((PIPE_BULK << 30) | __create_pipe(dev, endpoint))
#define usb_rcvbulkpipe(dev, endpoint)	\
	((PIPE_BULK << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndintpipe(dev, endpoint)	\
	((PIPE_INTERRUPT << 30) | __create_pipe(dev, endpoint))
#define usb_rcvintpipe(dev, endpoint)	\
	((PIPE_INTERRUPT << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)

static inline struct usb_host_endpoint *
usb_pipe_endpoint(struct usb_device *dev, unsigned int pipe)
{
	struct usb_host_endpoint **eps;
	eps = usb_pipein(pipe) ? dev->ep_in : dev->ep_out;
	return eps[usb_pipeendpoint(pipe)];
}

/*-------------------------------------------------------------------------*/

static inline __u16
usb_maxpacket(struct usb_device *udev, int pipe, int is_out)
{
	struct usb_host_endpoint	*ep;
	unsigned			epnum = usb_pipeendpoint(pipe);

	if (is_out) {
		WARN_ON(usb_pipein(pipe));
		ep = udev->ep_out[epnum];
	} else {
		WARN_ON(usb_pipeout(pipe));
		ep = udev->ep_in[epnum];
	}
	if (!ep)
		return 0;

	/* NOTE:  only 0x07ff bits are for packet size... */
	return usb_endpoint_maxp(&ep->desc);
}

/*-------------------------------------------------------------------------*/

/*
 * Send and receive control messages..
 */
int usb_new_device(struct usb_device *dev);


/*-------------------------------------------------------------------------*/

extern void usb_detect_quirks(struct usb_device *udev);
extern void usb_detect_interface_quirks(struct usb_device *udev);
#define usb_get_dev(x)      (x)
#define usb_put_dev(x)
#define usb_get_intf(x)     (x)
#define usb_put_intf(x)
#define usb_autosuspend_device(x)
#define usb_unlocked_enable_lpm(x)
#define usb_enable_ltm(x)
#define usb_enable_lpm(x)
#define usb_disable_lpm(x)  0

extern void usb_release_interface_cache(struct kref *ref);
extern char *usb_cache_string(struct usb_device *udev, int index);
static inline int usb_choose_configuration(struct usb_device *udev)
{
    return udev->config[0].desc.bConfigurationValue;
}

extern struct usb_host_interface *usb_find_alt_setting(
		struct usb_host_config *config,
		unsigned int iface_num,
		unsigned int alt_num);
extern struct usb_interface *usb_ifnum_to_if(const struct usb_device *dev,
				      unsigned ifnum);
extern struct usb_host_interface *usb_altnum_to_altsetting(
					const struct usb_interface *intf,
					unsigned int altnum);





#ifdef __cplusplus
}
#endif

#endif 
