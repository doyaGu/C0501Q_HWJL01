/** @file
 * usb device controller header file.
 *
 * @author Irene Lin
 */

#ifndef GADGET_H
#define GADGET_H


#include <stdlib.h>
#include "../inc/usb/ite_usb.h"


#ifdef __cplusplus
extern "C" {
#endif


struct usb_ep;

/**
 * struct usb_request - describes one i/o request
 */
struct usb_request {
    void				*buf;
    uint32_t			length;

    uint32_t			zero:1;
    uint32_t			index:3; // just for test

    void				(*complete)(struct usb_ep *ep, struct usb_request *req);
    
    void				*context;
    struct list_head	list;
    int					status;
    uint32_t			actual;
};


/*-------------------------------------------------------------------------*/

/**
 * endpoint-specific parts of the api to the usb controller hardware.
 */
struct usb_ep_ops {
    int (*enable)(struct usb_ep *ep, const struct usb_endpoint_descriptor *desc);
    int (*disable)(struct usb_ep *ep);

    struct usb_request *(*alloc_request)(struct usb_ep *ep);
    void (*free_request)(struct usb_ep *ep, struct usb_request *req);
    
    int (*queue)(struct usb_ep *ep, struct usb_request *req);
    int (*dequeue)(struct usb_ep *ep, struct usb_request *req);

    int (*set_halt)(struct usb_ep *ep, int value);
    int (*set_wedge)(struct usb_ep *ep);

    int (*fifo_status)(struct usb_ep *ep);
    void (*fifo_flush)(struct usb_ep *ep);
};


/**
 * struct usb_ep - device side representation of USB endpoint
 */
struct usb_ep {
    void					*driver_data;

    const char				*name;
    const struct usb_ep_ops	*ops;
    struct list_head		ep_list;
    uint32_t				maxpacket;
};

/*-------------------------------------------------------------------------*/

/**
 * usb_ep_enable - configure endpoint, making it usable
 * returns zero, or a negative error code.
 */
static inline int usb_ep_enable(struct usb_ep *ep, const struct usb_endpoint_descriptor *desc)
{
    return ep->ops->enable(ep, desc);
}

/**
 * usb_ep_disable - endpoint is no longer usable
 * returns zero, or a negative error code.
 */
static inline int usb_ep_disable(struct usb_ep *ep)
{
    return ep->ops->disable(ep);
}

/**
 * usb_ep_alloc_request - allocate a request object to use with this endpoint
 * Returns the request, or null if one could not be allocated.
 */
static inline struct usb_request *usb_ep_alloc_request(struct usb_ep *ep)
{
    return ep->ops->alloc_request(ep);
}

/**
 * usb_ep_free_request - frees a request object
 */
static inline void usb_ep_free_request(struct usb_ep *ep, struct usb_request *req)
{
    ep->ops->free_request(ep, req);
}

/**
 * usb_ep_queue - queues (submits) an I/O request to an endpoint.
 *
 * Returns zero, or a negative error code.  
 * Endpoints that are not enabled report errors; errors will also be
 * reported when the usb peripheral is disconnected.
 */
static inline int usb_ep_queue(struct usb_ep *ep, struct usb_request *req)
{
    return ep->ops->queue(ep, req);
}

/**
 * usb_ep_dequeue - dequeues (cancels, unlinks) an I/O request from an endpoint
 */
static inline int usb_ep_dequeue(struct usb_ep *ep, struct usb_request *req)
{
    return ep->ops->dequeue(ep, req);
}

/**
 * usb_ep_set_halt - sets the endpoint halt feature.
 * @ep: the non-isochronous endpoint being stalled
 *
 * Use this to stall an endpoint, perhaps as an error report.
 * Except for control endpoints,
 * the endpoint stays halted (will not stream any data) until the host
 * clears this feature; drivers may need to empty the endpoint's request
 * queue first, to make sure no inappropriate transfers happen.
 *
 * Note that while an endpoint CLEAR_FEATURE will be invisible to the
 * gadget driver, a SET_INTERFACE will not be.  To reset endpoints for the
 * current altsetting, see usb_ep_clear_halt().  When switching altsettings,
 * it's simplest to use usb_ep_enable() or usb_ep_disable() for the endpoints.
 *
 * Returns zero, or a negative error code.  On success, this call sets
 * underlying hardware state that blocks data transfers.
 * Attempts to halt IN endpoints will fail (returning -EAGAIN) if any
 * transfer requests are still queued, or if the controller hardware
 * (usually a FIFO) still holds bytes that the host hasn't collected.
 */
static inline int usb_ep_set_halt(struct usb_ep *ep)
{
    return ep->ops->set_halt(ep, 1);
}

/**
 * usb_ep_clear_halt - clears endpoint halt, and resets toggle
 * @ep:the bulk or interrupt endpoint being reset
 *
 * Use this when responding to the standard usb "set interface" request,
 * for endpoints that aren't reconfigured, after clearing any other state
 * in the endpoint's i/o queue.
 *
 * Returns zero, or a negative error code.  On success, this call clears
 * the underlying hardware state reflecting endpoint halt and data toggle.
 */
static inline int usb_ep_clear_halt(struct usb_ep *ep)
{
    return ep->ops->set_halt(ep, 0);
}

/**
 * usb_ep_set_wedge - sets the halt feature and ignores clear requests
 * @ep: the endpoint being wedged
 *
 * Use this to stall an endpoint and ignore CLEAR_FEATURE(HALT_ENDPOINT)
 * requests. If the gadget driver clears the halt status, it will
 * automatically unwedge the endpoint.
 *
 * Returns zero on success, else negative errno.
 */
static inline int usb_ep_set_wedge(struct usb_ep *ep)
{
    if (ep->ops->set_wedge)
        return ep->ops->set_wedge(ep);
    else
        return ep->ops->set_halt(ep, 1);
}

/**
 * usb_ep_fifo_status - returns number of bytes in fifo, or error
 * @ep: the endpoint whose fifo status is being checked.
 *
 * FIFO endpoints may have "unclaimed data" in them in certain cases,
 * such as after aborted transfers.  Hosts may not have collected all
 * the IN data written by the gadget driver (and reported by a request
 * completion).  The gadget driver may not have collected all the data
 * written OUT to it by the host.  Drivers that need precise handling for
 * fault reporting or recovery may need to use this call.
 *
 * This returns the number of such bytes in the fifo, or a negative
 * errno if the endpoint doesn't use a FIFO or doesn't support such
 * precise handling.
 */
static inline int usb_ep_fifo_status(struct usb_ep *ep)
{
    return ep->ops->fifo_status(ep);
}

/**
 * usb_ep_fifo_flush - flushes contents of a fifo
 * @ep: the endpoint whose fifo is being flushed.
 *
 * This call may be used to flush the "unclaimed data" that may exist in
 * an endpoint fifo after abnormal transaction terminations.  The call
 * must never be used except when endpoint is not being used for any
 * protocol translation.
 */
static inline void usb_ep_fifo_flush(struct usb_ep *ep)
{
    if (ep->ops->fifo_flush)
        ep->ops->fifo_flush(ep);
}

/*-------------------------------------------------------------------------*/

struct usb_gadget;

struct usb_gadget_ops {
    int (*get_frame)(struct usb_gadget*);
};

/**
 * struct usb_gadget - represents a usb slave device
 * @ops: Function pointers used to access hardware-specific operations.
 * @ep0: Endpoint zero, used when reading or writing responses to
 *	driver setup() requests
 * @ep_list: List of other endpoints supported by the device.
 * @speed: Speed of current connection to USB host.
 *
 * Gadgets have a mostly-portable "gadget driver" implementing device
 * functions, handling all usb configurations and interfaces.  Gadget
 * drivers talk to hardware-specific code indirectly, through ops vectors.
 * That insulates the gadget driver from hardware details, and packages
 * the hardware endpoints through generic i/o queues.  The "usb_gadget"
 * and "usb_ep" interfaces provide that insulation from the hardware.
 */
struct usb_gadget {
    const struct usb_gadget_ops *ops;
    struct usb_ep				*ep0;
    struct list_head			ep_list;
    enum usb_device_speed		speed;
    void						*driver_data;
};

static inline void set_gadget_data(struct usb_gadget *gadget, void *data)
    { gadget->driver_data = data; }
static inline void *get_gadget_data(struct usb_gadget *gadget)
    { return gadget->driver_data; }

/* iterates the non-control endpoints; 'tmp' is a struct usb_ep pointer */
#define gadget_for_each_ep(tmp,gadget) \
    list_for_each_entry(tmp, &(gadget)->ep_list, ep_list)


/**
 * gadget_is_dualspeed - return true iff the hardware handles high speed
 * @g: controller that might support both high and full speeds
 */
static inline int gadget_is_dualspeed(struct usb_gadget *g)
{
    return 1;
}

/**
 * gadget_is_otg - return true iff the hardware is OTG-ready
 * @g: controller that might have a Mini-AB connector
 */
static inline int gadget_is_otg(struct usb_gadget *g)
{
    return 0;
}

/**
 * usb_gadget_frame_number - returns the current frame number
 * @gadget: controller that reports the frame number
 *
 * Returns the usb frame number, normally eleven bits from a SOF packet,
 * or negative errno if this device doesn't support this capability.
 */
static inline int usb_gadget_frame_number(struct usb_gadget *gadget)
{
    return gadget->ops->get_frame(gadget);
}

/*-------------------------------------------------------------------------*/

/**
 * struct usb_gadget_driver - driver for usb 'slave' devices
 * @function: String describing the gadget's function
 * @speed: Highest speed the driver handles.
 * @bind: Invoked when the driver is bound to a gadget, usually
 *	after registering the driver.
 *	At that point, ep0 is fully initialized, and ep_list holds
 *	the currently-available endpoints.
 *	Called in a context that permits sleeping.
 * @setup: Invoked for ep0 control requests that aren't handled by
 *	the hardware level driver. Most calls must be handled by
 *	the gadget driver, including descriptor and configuration
 *	management.  The 16 bit members of the setup data are in
 *	USB byte order. Called in_interrupt; this may not sleep.  Driver
 *	queues a response to ep0, or returns negative to stall.
 * @disconnect: Invoked after all transfers have been stopped,
 *	when the host is disconnected.  May be called in_interrupt; this
 *	may not sleep.  Some devices can't detect disconnect, so this might
 *	not be called except as part of controller shutdown.
 * @unbind: Invoked when the driver is unbound from a gadget,
 *	usually from rmmod (after a disconnect is reported).
 *	Called in a context that permits sleeping.
 *
 * Devices are disabled till a gadget driver successfully bind()s, which
 * means the driver will handle setup() requests needed to enumerate (and
 * meet "chapter 9" requirements) then do some useful work.
 *
 * The usb controller driver handles a few standard usb requests.  Those
 * include set_address, and feature flags for devices, interfaces, and
 * endpoints (the get_status, set_feature, and clear_feature requests).
 *
 * Accordingly, the driver's setup() callback must always implement all
 * get_descriptor requests, returning at least a device descriptor and
 * a configuration descriptor.  
 *
 * The driver's setup() callback must also implement set_configuration,
 * and should also implement set_interface, get_configuration, and
 * get_interface.  Setting a configuration (or interface) is where
 * endpoints should be activated or (config 0) shut down.
 *
 * (Note that only the default control endpoint is supported.  Neither
 * hosts nor devices generally support control traffic except to ep0.)
 */
struct usb_gadget_driver {
    char			*function;
    enum usb_device_speed	speed;
    int				(*bind)(struct usb_gadget *);
    void			(*unbind)(struct usb_gadget *);
    int				(*setup)(struct usb_gadget *, const struct usb_ctrlrequest *);
    void			(*disconnect)(struct usb_gadget *);
    int				(*getselfpower)(void);
};



/*-------------------------------------------------------------------------*/

/* driver modules register and unregister, as usual.
 * these calls must be made in a context that can sleep.
 *
 * these will usually be implemented directly by the hardware-dependent
 * usb bus interface driver, which will only support a single driver.
 */

/**
 * usb_gadget_register_driver - register a gadget driver
 * @driver:the driver being registered
 *
 * Call this in your gadget driver's module initialization function,
 * to tell the underlying usb controller driver about your driver.
 * The driver's bind() function will be called to bind it to a
 * gadget before this registration call returns. 
 */
int usb_gadget_register_driver(struct usb_gadget_driver *driver);

/**
 * usb_gadget_unregister_driver - unregister a gadget driver
 * @driver:the driver being unregistered
 *
 * Call this in your gadget driver's module cleanup function,
 * to tell the underlying usb controller that your driver is
 * going away.  If the controller is connected to a USB host,
 * it will first disconnect().  The driver is also requested
 * to unbind() and clean up any device state, before this procedure
 * finally returns. 
 */
int usb_gadget_unregister_driver(struct usb_gadget_driver *driver);

/*-------------------------------------------------------------------------*/

/* utility to simplify dealing with string descriptors */

/**
 * struct usb_string - wraps a C string and its USB id
 * @id:the (nonzero) ID for this string
 * @s:the string, in UTF-8 encoding
 *
 * If you're using usb_gadget_get_string(), use this to wrap a string
 * together with its ID.
 */
struct usb_string {
    uint8_t			id;
    const char		*s;
};

/**
 * struct usb_gadget_strings - a set of USB strings in a given language
 * @language:identifies the strings' language (0x0409 for en-us)
 * @strings:array of strings with their ids
 *
 * If you're using usb_gadget_get_string(), use this to wrap all the
 * strings for a given language.
 */
struct usb_gadget_strings {
    uint16_t			language;	/* 0x0409 for en-us */
    struct usb_string	*strings;
};

/* put descriptor for string with that id into buf (buflen >= 256) */
int usb_gadget_get_string(struct usb_gadget_strings *table, int id, uint8_t *buf);

/*-------------------------------------------------------------------------*/

/* utility to simplify managing config descriptors */

/* write vector of descriptors into buffer */
int usb_descriptor_fillbuf(void *, unsigned,
        const struct usb_descriptor_header **);

/* build config descriptor from single descriptor vector */
int usb_gadget_config_buf(const struct usb_config_descriptor *config,
    void *buf, unsigned buflen, const struct usb_descriptor_header **desc);

/* copy a NULL-terminated vector of descriptors */
struct usb_descriptor_header **usb_copy_descriptors(
        struct usb_descriptor_header **);

/* return copy of endpoint descriptor given original descriptor set */
struct usb_endpoint_descriptor *usb_find_endpoint(
    struct usb_descriptor_header **src,
    struct usb_descriptor_header **copy,
    struct usb_endpoint_descriptor *match);

/**
 * usb_free_descriptors - free descriptors returned by usb_copy_descriptors()
 * @v: vector of descriptors
 */
static inline void usb_free_descriptors(struct usb_descriptor_header **v)
{
    free(v);
}


/*-------------------------------------------------------------------------*/

/* utility wrapping a simple endpoint selection policy */

extern struct usb_ep *usb_ep_autoconfig(struct usb_gadget *,
            struct usb_endpoint_descriptor *);

extern void usb_ep_autoconfig_reset(struct usb_gadget *);



#ifdef __cplusplus
}
#endif



#endif // GADGET_H
