/** @file
 *  File-backed USB Storage Gadget, for USB development.
 *
 * The File-backed Storage Gadget acts as a USB Mass Storage device,
 * appearing to the host as a disk drive. 
 *
 * @author Irene Lin
 * @version 1.0
 */

#include "fsg_cfg.h"
#include "../gadget.h"
#include "ite/ite_fsg.h"
#include "storage.h"
#include "../idb/idbInjection.h"

/*-------------------------------------------------------------------------*/
/* Big enough to hold our biggest descriptor */
#define EP0_BUFSIZE			256
#define DELAYED_STATUS		(EP0_BUFSIZE + 999)	// An impossibly large value

/* Number of buffers we will use.  2 is enough for double-buffering */
#define FSG_NUM_BUFFERS	2

/* Default size of buffer length. */
#define FSG_BUFLEN		(32*1024)

enum fsg_buffer_state {
    BUF_STATE_EMPTY = 0,
    BUF_STATE_FULL,
    BUF_STATE_BUSY
};

struct fsg_buffhd
{
    void				*buf;
    enum fsg_buffer_state	state;
    struct fsg_buffhd	*next;

    struct usb_request	*inreq;
    int					inreq_busy;
    struct usb_request	*outreq;
    int					outreq_busy;
};

enum fsg_state 
{
    FSG_STATE_COMMAND_PHASE = -10,		// This one isn't used anywhere
    FSG_STATE_DATA_PHASE,
    FSG_STATE_STATUS_PHASE,

    FSG_STATE_IDLE = 0,
    FSG_STATE_ABORT_BULK_OUT,
    FSG_STATE_RESET,
    FSG_STATE_INTERFACE_CHANGE,
    FSG_STATE_CONFIG_CHANGE,
    FSG_STATE_DISCONNECT,
    FSG_STATE_EXIT,
    FSG_STATE_TERMINATED
};

/*-------------------------------------------------------------------------*/
struct lun
{
    uint32_t	num_sectors;

    uint32_t	sense_data;
    uint32_t	sense_data_info;
    uint32_t	unit_attention_data;

    uint32_t	ro : 1;
    uint32_t	info_valid : 1;
    uint32_t	prevent_medium_removal : 1;
};

enum data_direction 
{
    DATA_DIR_UNKNOWN = 0,
    DATA_DIR_FROM_HOST,
    DATA_DIR_TO_HOST,
    DATA_DIR_NONE
};

struct fsg_dev
{
    _spinlock_t			lock;
    sem_t				*sem;
    struct usb_gadget   *gadget;

    struct usb_ep       *ep0;  // handy copy of gadget->ep0
    struct usb_request  *ep0req;
    uint32_t			ep0_req_tag;
    const char			*ep0req_name;

    volatile enum fsg_state		state; // for exception handling
    uint32_t			exception_req_tag;

    uint8_t				config;
    volatile uint8_t	new_config;
    volatile uint8_t	ignore_bulk_out;
    uint8_t				interface_alternate_setting;

    uint32_t			running : 1;
    uint32_t			bulk_in_enabled : 1;
    uint32_t			bulk_out_enabled : 1;
    uint32_t			phase_error : 1;
    uint32_t			short_packet_received : 1;
    uint32_t			bad_lun_okay : 1;

    struct usb_ep		*bulk_in;
    struct usb_ep		*bulk_out;

    struct fsg_buffhd	*next_buffhd_to_fill;
    struct fsg_buffhd	*next_buffhd_to_drain;
    struct fsg_buffhd	buffhds[FSG_NUM_BUFFERS];

    uint32_t	nluns;
    struct lun	*luns;
    struct lun	*curlun;

    const struct fsg_config	*cfg;

    int			cmnd_size;
    uint8_t		cmnd[MAX_COMMAND_SIZE];
    enum data_direction	data_dir;
    uint32_t    data_size;
    uint32_t	data_size_from_cmnd;
    uint32_t	lun;
    uint32_t	tag;
    uint32_t	residue;
    uint32_t	usb_amount_left;
	IDB_DATA_INJECTION(struct idbCtx idbCtx);
};

static struct fsg_dev* the_fsg;
static sem_t *thread_notifier;


/*-------------------------------------------------------------------------*/

/* Routines for unaligned data access */

static uint16_t get_be16(uint8_t *buf)
{
    return ((uint16_t) buf[0] << 8) | ((uint16_t) buf[1]);
}

static uint32_t get_be32(uint8_t *buf)
{
    return ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) |
            ((uint32_t) buf[2] << 8) | ((uint32_t) buf[3]);
}

static void put_be16(uint8_t *buf, uint16_t val)
{
    buf[0] = val >> 8;
    buf[1] = val;
}

static void put_be32(uint8_t *buf, uint32_t val)
{
    buf[0] = val >> 24;
    buf[1] = val >> 16;
    buf[2] = val >> 8;
    buf[3] = val & 0xff;
}

/*-------------------------------------------------------------------------*/
static inline int fsg_set_halt(struct fsg_dev *fsg, struct usb_ep *ep)
{
    return usb_ep_set_halt(ep);
}

/*-------------------------------------------------------------------------*/

static void raise_exception(struct fsg_dev *fsg, enum fsg_state new_state)
{
    /* Do nothing if a higher-priority exception is already in progress.
     * If a lower-or-equal priority exception is in progress, preempt it
     * and notify the main thread by sending it a signal. */
    _spin_lock_irqsave(&fsg->lock);
    if(fsg->state <= new_state) 
    {
        fsg->exception_req_tag = fsg->ep0_req_tag;
        fsg->state = new_state;
        FSG_INFO " execption with new_state: %d \n", new_state FSG_END

        {
            int v = -1;
            sem_getvalue(fsg->sem, &v); // TODO: has isr issue!!!

            if(v==0)
            {
                if(ithGetCpuMode() == ITH_CPU_SYS)
                    sem_post(fsg->sem);
                else
                    itpSemPostFromISR(fsg->sem);
            }
        }
    }
    _spin_unlock_irqrestore(&fsg->lock);
}

static int exception_in_progress(struct fsg_dev *fsg)
{
    IDB_CODE_INJECTION(fsg->idbCtx.exception=(fsg->state>FSG_STATE_IDLE));
    return (fsg->state > FSG_STATE_IDLE);
}

#include "transport.c"

/*-------------------------------------------------------------------------*/

/* in_irq */
static int ep0_queue(struct fsg_dev *fsg)
{
    int rc=0;
    rc = usb_ep_queue(fsg->ep0, fsg->ep0req);
    if(rc != 0 && rc != -ESHUTDOWN)
        FSG_ERROR " error in submission: %s --> 0x%X \n", fsg->ep0req_name, rc FSG_END

    return rc;
}

/* in_irq */
static void ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct fsg_dev	*fsg = ep->driver_data;

    _fsg_func_enter;

    FSG_DBG " ep0_complete: %s, buf:0x%X, len:(%d/%d) \n", 
        fsg->ep0req_name, req->buf, req->actual, req->length FSG_END

    if(req->status || (req->actual != req->length))
        FSG_WARNING " ep0_complete: request status %d \n", req->status FSG_END

    // request was cancelled
    if(req->status == -ECONNRESET) 
        usb_ep_fifo_flush(ep);

    _fsg_func_leave;
}

/*-------------------------------------------------------------------------*/

/* Bulk endpoint completion handlers. These always run in_irq. */
static void bulk_in_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct fsg_dev		*fsg = ep->driver_data;
    struct fsg_buffhd	*bh = req->context;

    if(req->status || (req->actual != req->length))
        FSG_ERROR " %s => status:0x%X(%d) (%d/%d) \n", __FUNCTION__, req->status, req->status, req->actual, req->length FSG_END

    /* request was cancelled */
    if(req->status == -ECONNRESET) 
        usb_ep_fifo_flush(ep);

    /* Hold the lock while we update the request and buffer states */
    _spin_lock(&fsg->lock);
    bh->inreq_busy = 0;
    bh->state = BUF_STATE_EMPTY;
    _spin_unlock(&fsg->lock);
}

static void bulk_out_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct fsg_dev		*fsg = ep->driver_data;
    struct fsg_buffhd	*bh = req->context;

    if(req->status || (req->actual != req->length))
        FSG_ERROR " %s => status:0x%X(%d) (%d/%d) \n", __FUNCTION__, req->status, req->status, req->actual, req->length FSG_END

#if defined(DUMP_DATA)
    //if(req->actual == 0x1FF) // test
    {
        FSG_DATA " bulk-out: \n" FSG_END
        ithPrintVram8((uint32_t)req->buf, req->actual);
    }
#endif

    /* request was cancelled */
    if(req->status == -ECONNRESET) 
        usb_ep_fifo_flush(ep);

    /* Hold the lock while we update the request and buffer states */
    _spin_lock(&fsg->lock);
    bh->outreq_busy = 0;
    bh->state = BUF_STATE_FULL;
    _spin_unlock(&fsg->lock);
}

/*-------------------------------------------------------------------------*/
#include "../usbstring.c"
#include "../config.c"
#include "../epautoconf.c"
#include "fsg_desc.c"

static void fsg_unbind(struct usb_gadget *gadget)
{
    struct fsg_dev	*fsg = get_gadget_data(gadget);
    struct usb_request *req;
    int i;

    _fsg_func_enter;

    if(fsg->state != FSG_STATE_TERMINATED)
    {
        raise_exception(fsg, FSG_STATE_EXIT);
        if(itpSemWaitTimeout(thread_notifier, 1200))
            FSG_ERROR " wait fsg thread timeout!! (1200 ms) \n" FSG_END
    }

    /* Free the data buffers */
    for(i=0; i<FSG_NUM_BUFFERS; ++i)
    {
        if(fsg->buffhds[i].buf)
            free(fsg->buffhds[i].buf);
    }

    /* Free the request and buffer for endpoint 0 */
    req = fsg->ep0req;
    if(req) 
    {
        if(req->buf)
        {
            free(req->buf);
            req->buf = NULL;
        }
        usb_ep_free_request(fsg->ep0, req);
        fsg->ep0req = NULL;
    }

    set_gadget_data(gadget, NULL);

    if(fsg->luns)
    {
        free(fsg->luns);
        fsg->luns = NULL;
    }

    _fsg_func_leave;

    return;
}

static int fsg_bind(struct usb_gadget *gadget)
{
    struct fsg_dev *fsg = the_fsg;
    int rc = 0, i;
    struct usb_ep *ep;
    struct usb_request *req;

    _fsg_func_enter;

    fsg->gadget = gadget;
    set_gadget_data(gadget, fsg);
    fsg->ep0 = gadget->ep0;
    fsg->ep0->driver_data = fsg;

    fsg->nluns = fsg->cfg->nluns;
    fsg->luns = malloc(sizeof(struct lun) * fsg->nluns);
    memset((void*)fsg->luns, 0x0, (sizeof(struct lun) * fsg->nluns));
    fsg->cfg->ops->open();

    /* Find all the endpoints we will use */
    usb_ep_autoconfig_reset(gadget);
    IDB_CODE_INJECTION(idbBindInjection(gadget, &fsg->idbCtx, &config_desc));
    ep = usb_ep_autoconfig(gadget, &fs_bulk_in_desc);
    if (!ep)
        goto autoconf_fail;
    ep->driver_data = fsg;
    fsg->bulk_in = ep;

    ep = usb_ep_autoconfig(gadget, &fs_bulk_out_desc);
    if (!ep)
        goto autoconf_fail;
    ep->driver_data = fsg;
    fsg->bulk_out = ep;

    /* Fix up the descriptors */
    device_desc.bMaxPacketSize0 = fsg->ep0->maxpacket;
    device_desc.idVendor = cpu_to_le16(fsg->cfg->vendor_id);
    device_desc.idProduct = cpu_to_le16(fsg->cfg->product_id);
    device_desc.bcdDevice = cpu_to_le16(fsg->cfg->release_num);

    if(gadget_is_dualspeed(gadget)) 
    {
        /* Assume ep0 uses the same maxpacket value for both speeds */
        dev_qualifier.bMaxPacketSize0 = fsg->ep0->maxpacket;

        /* Assume endpoint addresses are the same for both speeds */
        hs_bulk_in_desc.bEndpointAddress = fs_bulk_in_desc.bEndpointAddress;
        hs_bulk_out_desc.bEndpointAddress =	fs_bulk_out_desc.bEndpointAddress;
    }

    /* Allocate the request and buffer for endpoint 0 */
    fsg->ep0req = req = usb_ep_alloc_request(fsg->ep0);
    if(!req)
    {
        rc = ERR_FSG_ALLOC_EP0REQ_FAIL;
        goto out;
    }
    req->buf = malloc(EP0_BUFSIZE);
    if(!req->buf)
    {
        rc = ERR_FSG_ALLOC_EP0BUF_FAIL;
        goto out;
    }
    req->complete = ep0_complete;

    /* Allocate the data buffers */
    for(i=0; i<FSG_NUM_BUFFERS; ++i) 
    {
        struct fsg_buffhd *bh = &fsg->buffhds[i];

        bh->buf = malloc(FSG_BUFLEN);
        if(!bh->buf)
        {
            rc = ERR_FSG_ALLOC_BUFFHD_FAIL;
            goto out;
        }
        bh->next = bh + 1;
    }
    fsg->buffhds[FSG_NUM_BUFFERS - 1].next = &fsg->buffhds[0];

    strings[0].s = fsg->cfg->manufacturer;
    strings[1].s = fsg->cfg->product;
    strings[2].s = fsg->cfg->serial;

    /* Tell the thread to start working */
    sem_post(thread_notifier);

    _fsg_func_leave;

    return 0;

autoconf_fail:
    rc = ERR_FSG_AUTOCONFIG_FAIL;
out:
    fsg->state = FSG_STATE_TERMINATED;
    fsg_unbind(gadget);
    check_result(rc);
    return rc;
}

/* Ep0 standard request handlers.  These always run in_irq. */
static int standard_setup_req(struct fsg_dev *fsg, const struct usb_ctrlrequest *ctrl)
{
    struct usb_request *req = fsg->ep0req;
    uint16_t w_index = (ctrl->wIndex);
    uint16_t w_value = (ctrl->wValue);
    int value = -ERR_FSG_REQ_NOT_SUPPORT;

    _fsg_func_enter;

    switch(ctrl->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
        if(ctrl->bRequestType != (USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_DEVICE))
            break;
        switch(w_value >> 8)
        {
        case USB_DT_DEVICE:
            FSG_INFO " get device descriptor \n" FSG_END
            value = sizeof(device_desc);
            memcpy(req->buf, &device_desc, value);
            break;
        case USB_DT_DEVICE_QUALIFIER:
            FSG_INFO " get device qualifier \n" FSG_END
            if(!gadget_is_dualspeed(fsg->gadget))
                break;
            value = sizeof(dev_qualifier);
            memcpy(req->buf, &dev_qualifier, value);
            break;
        case USB_DT_OTHER_SPEED_CONFIG:
            FSG_INFO " get other-speed config descriptor \n" FSG_END
            if(!gadget_is_dualspeed(fsg->gadget))
                break;
            goto get_config;
            break;
        case USB_DT_CONFIG:
            FSG_INFO " get configuration descriptor \n" FSG_END
get_config:
            value = populate_config_buf(fsg->gadget, req->buf, (w_value>>8), (w_value&0xFF));
            break;
        case USB_DT_STRING:
            FSG_INFO " get string descriptor: %d \n", (w_value & 0xFF) FSG_END
            /* wIndex == language code */
            value = usb_gadget_get_string(&stringtab, (w_value & 0xFF), req->buf);
            break;
        }
        #if defined(DUMP_DATA)
        if(value > 0)
            ithPrintVram8((uint32_t)req->buf, value);
        #endif
        break;

    case USB_REQ_SET_CONFIGURATION:
        if(ctrl->bRequestType != (USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_DEVICE))
            break;
        FSG_INFO " set configuration: %d \n", w_value FSG_END
        if((w_value == CONFIG_VALUE) || (w_value == 0))
        {
            fsg->new_config = w_value;

            /* Raise an exception to wipe out previous transaction
             * state (queued bufs, etc) and set the new config. */
            raise_exception(fsg, FSG_STATE_CONFIG_CHANGE);
            value = DELAYED_STATUS;
        }
        break;

    case USB_REQ_GET_CONFIGURATION:
        if(ctrl->bRequestType != (USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_DEVICE))
            break;
        FSG_INFO " get configuration \n" FSG_END
        *(uint8_t *)req->buf = fsg->config;
        value = 1;
        break;

    case USB_REQ_SET_INTERFACE:
        if(ctrl->bRequestType != (USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_INTERFACE))
            break;
        FSG_INFO " set interface: w_value %d, w_index %d \n", w_value, w_index FSG_END
        if(fsg->config && (w_index==0))
        {
            fsg->interface_alternate_setting = w_value;
            if(w_value)
                ithPrintf(" set interface alternate setting: %d => non-zero??? \n", w_value);

            /* Raise an exception to wipe out previous transaction
             * state (queued bufs, etc) and install the new
             * interface altsetting. */
            raise_exception(fsg, FSG_STATE_INTERFACE_CHANGE);
            value = DELAYED_STATUS;
        }
        break;

    case USB_REQ_GET_INTERFACE:
        if(ctrl->bRequestType != (USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE))
            break;
        FSG_INFO " get interface \n" FSG_END
        *(uint8_t *)req->buf = fsg->interface_alternate_setting;
        value = 1;
        break;

    default:
        break;
    }

end:
    if(value<0)
        check_result(value);

    _fsg_func_leave;
    return value;
}

/* Ep0 class request handlers.  These always run in_irq. */
static int class_setup_req(struct fsg_dev *fsg, const struct usb_ctrlrequest *ctrl)
{
    struct usb_request *req = fsg->ep0req;
    uint16_t w_index = (ctrl->wIndex);
    uint16_t w_value = (ctrl->wValue);
    uint16_t w_length = (ctrl->wLength);
    int value = -ERR_FSG_REQ_NOT_SUPPORT;

    _fsg_func_enter;

    if(!fsg->config)
        goto end;

    /* Handle Bulk-only class-specific requests */
    switch(ctrl->bRequest)
    {
    case USB_BULK_RESET_REQUEST:
        if(ctrl->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if(w_index != 0 || w_value != 0 || w_length != 0)
        {
            value = -ERR_FSG_REQ_INVALID_PARAM;
            break;
        }
        FSG_INFO " bulk reset request \n" FSG_END
        raise_exception(fsg, FSG_STATE_RESET);
        value = DELAYED_STATUS;
        break;

    case USB_BULK_GET_MAX_LUN_REQUEST:
        if(ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if(w_index != 0 || w_value != 0)
        {
            value = -ERR_FSG_REQ_INVALID_PARAM;
            break;
        }
        *(uint8_t*)req->buf = fsg->nluns - 1;
        FSG_INFO " get max LUN: %d \n", (fsg->nluns - 1) FSG_END
        value = 1;
        break;
    }

end:
    if(value<0)
    {
        //if(value == -ERR_FSG_REQ_NOT_SUPPORT)
            FSG_ERROR " unknown class-specific control req "
                "%02x.%02x v:%04x i:%04x l:%u\n",
                ctrl->bRequestType, ctrl->bRequest,	w_value, w_index, w_length FSG_END

        check_result(value);
    }

    _fsg_func_leave;
    return value;
}

/* in irq */
static int fsg_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
    int rc = 0;
    struct fsg_dev *fsg = get_gadget_data(gadget);
    int w_length = (ctrl->wLength);

    _fsg_func_enter;

    ++fsg->ep0_req_tag;
    fsg->ep0req->context = NULL;
    fsg->ep0req->length = 0;

    if((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS)
        rc = class_setup_req(fsg, ctrl);
    else if((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD)
        rc = standard_setup_req(fsg, ctrl);
    else
    {
        rc = -ERR_FSG_REQ_NOT_SUPPORT;
        goto end;
    }

    if((rc >= 0) && (rc != DELAYED_STATUS))
    {
        rc = min(rc, w_length);
        fsg->ep0req->length = rc;
        fsg->ep0req->zero = rc < w_length;
        fsg->ep0req_name = ((ctrl->bRequestType & USB_DIR_IN) ? "ep0-in" : "ep0-out");
        rc = ep0_queue(fsg);
    }

end:
    if(rc < 0)
        check_result(rc);

    _fsg_func_leave;
    /* device either stalls (rc < 0) or reports sucess */
    return rc;
}

static void fsg_disconnect(struct usb_gadget *gadget)
{
    struct fsg_dev *fsg = get_gadget_data(gadget);

    _fsg_func_enter;
    raise_exception(fsg, FSG_STATE_DISCONNECT);
    _fsg_func_leave;

    return;
}


static char function_name[] = "file storage gadget";

static struct usb_gadget_driver	fsg_driver = 
{
    .speed		= USB_SPEED_HIGH,
    .function   = function_name,
    .bind		= fsg_bind,
    .unbind		= fsg_unbind,
    .setup		= fsg_setup,
    .disconnect = fsg_disconnect,
    .getselfpower = fsg_selfpower,
};


/*-------------------------------------------------------------------------*/


int iteFsgInitialize(const struct fsg_config *cfg)
{
    int rc = 0;
    _fsg_func_enter;

    if(the_fsg)
    {
        ithPrintf(" fsg: double initial?? \n");
        goto end;
    }

    the_fsg = (struct fsg_dev*) malloc(sizeof(struct fsg_dev));
    if(!the_fsg)
    {
        rc = ERR_FSG_ALLOC_CTXT_FAIL;
        goto end;
    }

#if defined(__SM32__) // for _SM32_ build fail
    device_desc.bcdUSB = cpu_to_le16(0x0200);
    dev_qualifier.bcdUSB = cpu_to_le16(0x0200);
    hs_bulk_in_desc.wMaxPacketSize = cpu_to_le16(512);
    hs_bulk_out_desc.wMaxPacketSize = cpu_to_le16(512);
#endif

    memset((void*)the_fsg, 0x0, sizeof(struct fsg_dev));
    _spin_lock_init(&the_fsg->lock);
    the_fsg->sem = (sem_t *)malloc(sizeof(sem_t));
    sem_init(the_fsg->sem, 0, 0); 
    the_fsg->cfg = cfg;

    if(!thread_notifier)
    {
        thread_notifier = (sem_t *)malloc(sizeof(sem_t)); 
        sem_init(thread_notifier, 0, 0); 
    }

    usb_gadget_register_driver(&fsg_driver);

    _fsg_func_leave;

end:
    check_result(rc);
    return rc;
}


int iteFsgTerminate(void)
{
    int rc = 0;
    _fsg_func_enter;

    usb_gadget_unregister_driver(&fsg_driver);

    if(the_fsg)
    {
        sem_destroy(the_fsg->sem);
        if(the_fsg->sem)
            free(the_fsg->sem);

        free((void*)the_fsg);
        the_fsg = 0;
    }

    _fsg_func_leave;

    return rc;
}

/*-------------------------------------------------------------------------*/
static int enable_endpoint(struct fsg_dev *fsg, struct usb_ep *ep,
        const struct usb_endpoint_descriptor *d)
{
    int	rc;

    ep->driver_data = fsg;
    rc = usb_ep_enable(ep, d);
    if (rc)
        FSG_ERROR "can't enable %s, result %d\n", ep->name, rc FSG_END
    return rc;
}

static int alloc_request(struct usb_ep *ep,	struct usb_request **preq)
{
    *preq = usb_ep_alloc_request(ep);
    if (*preq)
        return 0;
    FSG_ERROR " can't allocate request for %s\n", ep->name FSG_END
    return -ERR_FSG_ALLOC_EPREQ_FAIL;
}

/*
 * Reset interface setting and re-init endpoint state (toggle etc).
 * Call with altsetting < 0 to disable the interface.  The only other
 * available altsetting is 0, which enables the interface.
 */
static int do_set_interface(struct fsg_dev *fsg, int altsetting)
{
    int rc = 0;
    int i;
    const struct usb_endpoint_descriptor *d;

    _fsg_func_enter;

    if(fsg->running)
        FSG_INFO " reset interface \n" FSG_END

reset:
    IDB_CODE_INJECTION(idbDoSetInterfaceInjection(&fsg->idbCtx, altsetting));
    /* deallocate the requests */
    for(i=0; i<FSG_NUM_BUFFERS; i++)
    {
        struct fsg_buffhd *bh = &fsg->buffhds[i];

        if(bh->inreq)
        {
            usb_ep_free_request(fsg->bulk_in, bh->inreq);
            bh->inreq = NULL;
        }
        if(bh->outreq)
        {
            usb_ep_free_request(fsg->bulk_out, bh->outreq);
            bh->outreq = NULL;
        }
    }

    /* disable the endpoints */
    if(fsg->bulk_in_enabled)
    {
        usb_ep_disable(fsg->bulk_in);
        fsg->bulk_in_enabled = 0;
    }
    if(fsg->bulk_out_enabled)
    {
        usb_ep_disable(fsg->bulk_out);
        fsg->bulk_out_enabled = 0;
    }
    fsg->running = 0;
    if((altsetting < 0) || (rc != 0))
    {
        FSG_DBG " already disable interface! rc=0x%X \n", rc FSG_END
        return rc;
    }

    FSG_DBG " set interface: %d \n", altsetting FSG_END

    /* enable the endpoints */
    d = ep_desc(fsg->gadget, &fs_bulk_in_desc, &hs_bulk_in_desc);
    if((rc = enable_endpoint(fsg, fsg->bulk_in, d)) != 0)
        goto reset;
    fsg->bulk_in_enabled = 1;

    d = ep_desc(fsg->gadget, &fs_bulk_out_desc, &hs_bulk_out_desc);
    if((rc = enable_endpoint(fsg, fsg->bulk_out, d)) != 0)
        goto reset;
    fsg->bulk_out_enabled = 1;
    fsg->ignore_bulk_out = 0;

    /* allocate the requests */
    for(i=0; i<FSG_NUM_BUFFERS; i++)
    {
        struct fsg_buffhd *bh = &fsg->buffhds[i];

        if((rc = alloc_request(fsg->bulk_in, &bh->inreq)) != 0)
            goto reset;
        if((rc = alloc_request(fsg->bulk_out, &bh->outreq)) != 0)
            goto reset;
        bh->inreq->buf = bh->outreq->buf = bh->buf;
        bh->inreq->context = bh->outreq->context = bh;
        bh->inreq->complete = bulk_in_complete;
        bh->outreq->complete = bulk_out_complete;
        //<< for debug // test
        if(1)
        {
            if(i==0)
            {
                bh->inreq->index = 1;
                bh->outreq->index = 2;
            }
            if(i==1)
            {
                bh->inreq->index = 3;
                bh->outreq->index = 4;
            }
        }
        //>>
    }

    fsg->running = 1;
    for(i=0; i<fsg->nluns; i++)
    {
        fsg->luns[i].unit_attention_data = SS_NO_SENSE;
        //fsg->luns[i].unit_attention_data = SS_RESET_OCCURRED;
    }

    _fsg_func_leave;
    return rc;
}

static int do_set_config(struct fsg_dev *fsg, uint8_t new_config)
{
    int rc = 0;
    _fsg_func_enter;

    /* disable the single interface */
    if(fsg->config != 0)
    {
        FSG_DBG " reset config \n" FSG_END
        fsg->config = 0;
        rc = do_set_interface(fsg, -1);
    }

    /* enable the interface */
    if(new_config != 0)
    {
        fsg->config = new_config;

        if((rc = do_set_interface(fsg, 0)) != 0)
            fsg->config = 0;
        else
        {
            char *speed;
            switch(fsg->gadget->speed)
            {
                case USB_SPEED_LOW:		speed = "low"; break;
                case USB_SPEED_FULL:	speed = "full"; break;
                case USB_SPEED_HIGH:	speed = "high"; break;
                default:				speed = "?"; break;
            }
            FSG_INFO " %s speed config #%d \n", speed, fsg->config FSG_END
        }
    }

    check_result(rc);
    _fsg_func_leave;
    return rc;
}

static void handle_exception(struct fsg_dev *fsg)
{
    int				rc = 0;
    uint8_t			new_config;
    enum fsg_state	old_state;
    uint32_t		exception_req_tag;
    int				i;
    struct fsg_buffhd	*bh;
    struct lun		*curlun;
    int				num_active;

    _fsg_func_enter;

    FSG_DBG " %s: %d \n", __FUNCTION__, fsg->state FSG_END

    /* Cancel all the pending transfers */
    for(i=0; i<FSG_NUM_BUFFERS; ++i) 
    {
        bh = &fsg->buffhds[i];
        if(bh->inreq_busy)
            usb_ep_dequeue(fsg->bulk_in, bh->inreq);
        if(bh->outreq_busy)
            usb_ep_dequeue(fsg->bulk_out, bh->outreq);
    }

    /* Wait until everything is idle */
    for(;;) 
    {
        for(i=0; i<FSG_NUM_BUFFERS; ++i) 
        {
            bh = &fsg->buffhds[i];
            num_active = bh->inreq_busy + bh->outreq_busy;
        }
        if(num_active == 0)
            break;
        ithPrintf("&\n");
    }

    /* Clear out the controller's fifos */
    if(fsg->bulk_in_enabled)
        usb_ep_fifo_flush(fsg->bulk_in);
    if(fsg->bulk_out_enabled)
        usb_ep_fifo_flush(fsg->bulk_out);

    /* Reset the I/O buffer states and pointers, the SCSI
     * state, and the exception.  Then invoke the handler. */
    _spin_lock_irq(&fsg->lock);

    for(i=0; i<FSG_NUM_BUFFERS; ++i) 
    {
        bh = &fsg->buffhds[i];
        bh->state = BUF_STATE_EMPTY;
    }
    fsg->next_buffhd_to_fill = fsg->next_buffhd_to_drain = &fsg->buffhds[0];

    exception_req_tag = fsg->exception_req_tag;
    new_config = fsg->new_config;
    old_state = fsg->state;

    if(old_state == FSG_STATE_ABORT_BULK_OUT)
        fsg->state = FSG_STATE_STATUS_PHASE;
    else 
    {
        for(i=0; i<fsg->nluns; ++i) 
        {
            curlun = &fsg->luns[i];
            curlun->prevent_medium_removal = 0;
            curlun->sense_data = curlun->unit_attention_data = SS_NO_SENSE;
            curlun->sense_data_info = 0;
            curlun->info_valid = 0;
        }
        fsg->state = FSG_STATE_IDLE;
    }
    _spin_unlock_irq(&fsg->lock);

    /* Carry out any extra actions required for the exception */
    switch (old_state) 
    {
    default:
        break;

    case FSG_STATE_ABORT_BULK_OUT:
        send_status(fsg);
        _spin_lock_irq(&fsg->lock);
        if (fsg->state == FSG_STATE_STATUS_PHASE)
            fsg->state = FSG_STATE_IDLE;
        _spin_unlock_irq(&fsg->lock);
        break;

    case FSG_STATE_RESET:
        /* In case we were forced against our will to halt a
         * bulk endpoint, clear the halt now.  (The SuperH UDC
         * requires this.) */
        if(fsg->ignore_bulk_out)
        {
            fsg->ignore_bulk_out = 0;
            usb_ep_clear_halt(fsg->bulk_in);
        }

        if(fsg->ep0_req_tag == exception_req_tag)
            ep0_queue(fsg);	// Complete the status stage
        else
        {
            ithPrintf(" %d: fsg->ep0_req_tag != exception_req_tag (%d != %d) \n",
                __LINE__, fsg->ep0_req_tag, exception_req_tag);
        }

        /* Technically this should go here, but it would only be
         * a waste of time.  Ditto for the INTERFACE_CHANGE and
         * CONFIG_CHANGE cases. */
        // for (i = 0; i < fsg->nluns; ++i)
        //	fsg->luns[i].unit_attention_data = SS_RESET_OCCURRED;
        break;

    case FSG_STATE_INTERFACE_CHANGE:
        rc = do_set_interface(fsg, 0);
        if (fsg->ep0_req_tag != exception_req_tag)
        {
            ithPrintf(" %d: fsg->ep0_req_tag != exception_req_tag (%d != %d) \n",
                __LINE__, fsg->ep0_req_tag, exception_req_tag);
            break;
        }
        if (rc != 0)			// STALL on errors
            fsg_set_halt(fsg, fsg->ep0);
        else				// Complete the status stage
            ep0_queue(fsg);
        break;

    case FSG_STATE_CONFIG_CHANGE:
        rc = do_set_config(fsg, new_config);
        if (fsg->ep0_req_tag != exception_req_tag)
        {
            ithPrintf(" %d: fsg->ep0_req_tag != exception_req_tag (%d != %d) \n",
                __LINE__, fsg->ep0_req_tag, exception_req_tag);
            break;
        }
        if (rc != 0)		// STALL on errors
            fsg_set_halt(fsg, fsg->ep0);
        else				// Complete the status stage
            ep0_queue(fsg);
        break;

    case FSG_STATE_DISCONNECT:
        do_set_config(fsg, 0);		// Unconfigured state
        break;

    case FSG_STATE_EXIT:
    case FSG_STATE_TERMINATED:
        do_set_config(fsg, 0);			// Free resources
        _spin_lock_irq(&fsg->lock);
        fsg->state = FSG_STATE_TERMINATED;	// Stop the thread
        _spin_unlock_irq(&fsg->lock);
        break;
    }

    _fsg_func_leave;
}

/*-------------------------------------------------------------------------*/

void* fsg_main_thread(void *arg)
{
    struct fsg_dev *fsg;

    if(!thread_notifier)
    {
        thread_notifier = (sem_t *)malloc(sizeof(sem_t)); 
        sem_init(thread_notifier, 0, 0); 
    }

wait:
    sem_wait(thread_notifier);
    FSG_DBG " fsg_main_thread: wake up \n" FSG_END

    fsg = the_fsg;
    sem_wait(fsg->sem);
    IDB_CODE_INJECTION(fsg->idbCtx.running=1);
    IDB_CODE_INJECTION(idbThreadInjection(&fsg->idbCtx));
    while(fsg->state != FSG_STATE_TERMINATED)
    {
        if(exception_in_progress(fsg))
        {
            handle_exception(fsg);
            continue;
        }

        if(!fsg->running) // wait set configuration/interface ready
        {
            //usleep(1000);
            continue;
        }

        if(get_next_command(fsg))
            continue;

        _spin_lock_irq(&fsg->lock);
        if(!exception_in_progress(fsg))
            fsg->state = FSG_STATE_DATA_PHASE;
        _spin_unlock_irq(&fsg->lock);

        if(do_scsi_command(fsg) || finish_reply(fsg))
            continue;

        _spin_lock_irq(&fsg->lock);
        if(!exception_in_progress(fsg))
            fsg->state = FSG_STATE_STATUS_PHASE;
        _spin_unlock_irq(&fsg->lock);

        if(send_status(fsg))
            continue;

        _spin_lock_irq(&fsg->lock);
        if(!exception_in_progress(fsg))
            fsg->state = FSG_STATE_IDLE;
        _spin_unlock_irq(&fsg->lock);
    }
    IDB_CODE_INJECTION(fsg->idbCtx.running=0);
    sem_post(thread_notifier);
    usleep(500*1000);
    goto wait;
}

