/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * USB Host Controller Driver (usb_hcd) framework.
 *
 * @author Irene Lin
 */
#ifndef USB_EX_HCD_H
#define USB_EX_HCD_H
/*-------------------------------------------------------------------------*/
/*
 * USB Host Controller Driver (usb_hcd) framework
 *
 * Since "struct usb_bus" is so thin, you can't share much code in it.
 * This framework is a layer over that, and should be more sharable.
 */
/*-------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              usb_hcd struct
//=============================================================================
struct usb_hcd {         /* usb_bus.hcpriv points to this */
    /*
     * housekeeping
     */
    struct usb_bus         self; /* hcd is-a bus */

    /*
     * hardware info/state
     */
    const struct hc_driver *driver; /* hw-specific hooks */
    uint32_t               iobase;

    int                    state;
#define __ACTIVE           0x01
#define __SUSPEND          0x04
#define __TRANSIENT        0x80

#define HC_STATE_HALT      0
#define HC_STATE_RUNNING   (__ACTIVE)
#define HC_STATE_QUIESCING (__SUSPEND | __TRANSIENT | __ACTIVE)
#define HC_STATE_RESUMING  (__SUSPEND | __TRANSIENT)
#define HC_STATE_SUSPENDED (__SUSPEND)

#define HC_IS_RUNNING(state)   ((state) & __ACTIVE)
#define HC_IS_SUSPENDED(state) ((state) & __SUSPEND)

    uint32_t     connect;
    spinlock_t   hcd_urb_list_lock;
    struct mutex *bandwidth_mutex;
    uint32_t     index;

    /* more shared queuing code would be good; it should support
     * smarter scheduling, handle transaction translators, etc;
     * input size of periodic table to an interrupt scheduler.
     * (ohci 32, uhci 1024, ehci 256/512/1024).
     */

    /* The HC driver's private data is stored at the end of
     * this structure.
     */
    unsigned long hcd_priv[0]
    __attribute__ ((aligned(sizeof(uint32_t))));
};

static inline struct usb_bus *hcd_to_bus(struct usb_hcd *hcd)
{
    return &hcd->self;
}

static inline struct usb_hcd *bus_to_hcd(struct usb_bus *bus)
{
    return list_entry(bus, struct usb_hcd, self);
}

//=============================================================================
//                              hc_driver struct
//=============================================================================

/* each driver provides one of these, and hardware init support */

#define USB_HAS_DEVICE_MODE 0x80000000

struct hc_driver {
    uint32_t hcd_priv_size;     /* size of private data */

    int      flags;
#define HCD_USB11 0x0010        /* USB 1.1 */
#define HCD_USB2  0x0020        /* USB 2.0 */

    /* called to init HCD and root hub */
    int (*reset)(struct usb_hcd *hcd);

    /* cleanly make HCD stop writing memory and doing I/O */
    int  (*stop) (struct usb_hcd *hcd);

    /* return current frame number */
    int  (*get_frame_number) (struct usb_hcd *hcd);

    /* manage i/o requests, device state */
    int  (*urb_enqueue) (struct usb_hcd *hcd, struct urb *urb, gfp_t mem_flags);
    int  (*urb_dequeue) (struct usb_hcd *hcd, struct urb *urb, int status);

    /* hw synch, freeing endpoint resources that urb_dequeue can't */
    void (*endpoint_disable)(struct usb_hcd           *hcd,
                             struct usb_host_endpoint *ep);

    /* (optional) reset any endpoint state such as sequence number
       and current window */
    void (*endpoint_reset)(struct usb_hcd           *hcd,
                           struct usb_host_endpoint *ep);

    /* Change a group of bulk endpoints to support multiple stream IDs */
    int (*alloc_streams)(struct usb_hcd *hcd, struct usb_device *udev,
                         struct usb_host_endpoint **eps, unsigned int num_eps,
                         unsigned int num_streams, gfp_t mem_flags);
    /* Reverts a group of bulk endpoints back to not using stream IDs.
     * Can fail if we run out of memory.
     */
    int (*free_streams)(struct usb_hcd *hcd, struct usb_device *udev,
                        struct usb_host_endpoint **eps, unsigned int num_eps,
                        gfp_t mem_flags);

    int (*dev_exist) (struct usb_hcd *hcd);
};

/*-------------------------------------------------------------------------*/

/*
 * Generic bandwidth allocation constants/support
 */
#define FRAME_TIME_USECS 1000L
#define BitTime(bytecount) (7 * 8 * bytecount / 6)   /* with integer truncation */
/* Trying not to use worst-case bit-stuffing
 * of (7/6 * 8 * bytecount) = 9.33 * bytecount */
/* bytecount = data payload byte count */

#define NS_TO_US(ns)       ((ns + 500L) / 1000L)
/* convert & round nanoseconds to microseconds */

/*
 * Full/low speed bandwidth allocation constants/support.
 */
#define BW_HOST_DELAY              1000L /* nanoseconds */
#define BW_HUB_LS_SETUP            333L  /* nanoseconds */
/* 4 full-speed bit times (est.) */

#define FRAME_TIME_BITS            12000L   /* frame = 1 millisecond */
#define FRAME_TIME_MAX_BITS_ALLOC  (90L * FRAME_TIME_BITS / 100L)
#define FRAME_TIME_MAX_USECS_ALLOC (90L * FRAME_TIME_USECS / 100L)

/*
 * Ceiling [nano/micro]seconds (typical) for that many bytes at high speed
 * ISO is a bit less, no ACK ... from USB 2.0 spec, 5.11.3 (and needed
 * to preallocate bandwidth)
 */
#define USB2_HOST_DELAY            5 /* nsec, guess */
#define HS_NSECS(bytes)     (((55 * 8 * 2083) \
                             + (2083UL * (3 + BitTime(bytes)))) / 1000 \
                             + USB2_HOST_DELAY)
#define HS_NSECS_ISO(bytes) (((38 * 8 * 2083) \
                             + (2083UL * (3 + BitTime(bytes)))) / 1000 \
                             + USB2_HOST_DELAY)
#define HS_USECS(bytes)     NS_TO_US(HS_NSECS(bytes))
#define HS_USECS_ISO(bytes) NS_TO_US(HS_NSECS_ISO(bytes))

extern long usb_calc_bus_time(int speed, int is_input,
                              int isoc, int bytecount);

/*-------------------------------------------------------------------------*/

extern void usb_set_device_state(struct usb_device     *udev,
                                 enum usb_device_state new_state);

/*-------------------------------------------------------------------------*/

extern struct usb_hcd    *hcd0;
extern struct usb_hcd    *hcd1;
extern wait_queue_head_t usb_kill_urb_queue;

//=============================================================================
//                              Public functions
//=============================================================================
struct ehci_hcd;

int ehci_hcd_init(uint32_t index);
void usb_hcd_giveback_urb(struct usb_hcd *hcd, struct urb *urb, int status);

#define usb_get_hcd(x)                 (x)
#define usb_put_hcd(x)
#define usb_endpoint_out(ep_dir)       (!((ep_dir) & USB_DIR_IN))

/* The D0/D1 toggle bits ... USE WITH CAUTION (they're almost hcd-internal) */
#define usb_gettoggle(dev, ep, out)    (((dev)->toggle[out] >> (ep)) & 1)
#define usb_dotoggle(dev, ep, out)     ((dev)->toggle[out] ^= (1 << (ep)))
#define usb_settoggle(dev, ep, out, bit) \
    ((dev)->toggle[out] = ((dev)->toggle[out] & ~(1 << (ep))) | \
                          ((bit) << (ep)))

#define usb_hcd_synchronize_unlinks(x) do { } while (0)
#define usb_remove_ep_devs(x)          do {} while (0)

#ifdef __cplusplus
}
#endif

#endif // ifndef USB_EX_HCD_H