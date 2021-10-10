/** @file
 *  USB Device Controller extern API implementation.
 *
 * @author Irene Lin
 * @version 1.0
 */

#include "udc_cfg.h"
#include "gadget.h"
#include "udc.h"
#include "utility.h"


static char* names[] = {"ep0","ep1-bulkin","ep2-bulkout","ep3-bulkin","ep4-bulkout","ep5","ep6","ep7","ep8","ep9","ep10"};

static struct ite_udc* g_udc = NULL;


/*-------------------------------------------------------------------------*/
static void ep_reset(struct ite_ep *ep)
{
    _udc_func_enter;

    ep->ep.maxpacket = MAX_FIFO_SIZE;
    ep->desc = 0;
    ep->stopped = 1;
    ep->stall = 0;
    ep->wedged = 0;
    ep->irqs = 0;
}

/*-------------------------------------------------------------------------*/
static void ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
    UDC_DBG " ep0_complete: buf:0x%X, len:(%d/%d) \n", req->buf, req->actual, req->length UDC_END

    if(req->length == 53) // Test_Packet
    {
        req->buf = 0;

        mUsbEP0DoneSet();

        // Turn on "r_test_packet_done" bit(flag) (Bit 5)
        mUsbTsPkDoneSet();
        ithPrintf(" test packet done! \n");
    }
}

// finish or abort one request
static void done(struct ite_ep* ep, struct ite_request* req, int status)
{
    struct ite_udc* dev = ep->dev;

    UDC_DBG " done: ep%d, req:0x%X, status:%d \n", ep->num, req, status UDC_END

    list_del_init(&req->queue);

    if(dev->gadget.speed == USB_SPEED_UNKNOWN)
        req->req.status = -ESHUTDOWN;
    else
        req->req.status = status;

    UDC_DBG " complete: ep%d, req 0x%X, status:%d, len(%d/%d) \n",
        ep->num, &req->req, status, req->req.actual, req->req.length UDC_END

    if(status == -ESHUTDOWN) 
        UDC_ERROR " %s req %x(%x) done by ESHUTDOWN \n", ep->ep.name, req, &req->req UDC_END


    _spin_unlock(&dev->lock);

    if(req->req.complete)
        req->req.complete(&ep->ep, &req->req);
    else
        ithPrintf(" %s req %x(%x) no complete functin!! \n", ep->ep.name, req, &req->req);

    _spin_lock(&dev->lock);

    // TODO: should re-enable fifo interrupt for non-ep0 ???

    if(ep->num == 0)
        mUsbEP0DoneSet();
}

/* dequeue ALL requests */
static void nuke(struct ite_ep *ep, int status)
{
    struct ite_request	*req;

    _udc_func_enter;
    
    ep->stopped = 1;
    if(list_empty(&ep->queue))
        return;

    UDC_DBG " nuke: ep%d with status: %d \n", ep->num, status UDC_END

    while(!list_empty(&ep->queue)) 
    {
        req = list_entry(ep->queue.next, struct ite_request, queue);
        UDC_DBG "release req = %x\n", (uint32_t)req UDC_END
        done(ep, req, status);
    }

    _udc_func_leave;
}

/*-------------------------------------------------------------------------*/

static int udc_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
    int rc = 0;
    struct ite_udc *dev;
    struct ite_ep *ep;
    uint16_t max;

    _udc_func_enter;

    ep = container_of(_ep, struct ite_ep, ep);
    UDC_DBG " ep_enable: _ep=0x%X, desc=0x%X, ep->desc=0x%X \n", (uint32_t)_ep, (uint32_t)desc, (uint32_t)ep->desc UDC_END

    if(!_ep || !desc || ep->desc || (desc->bDescriptorType != USB_DT_ENDPOINT))
    {
        rc = ERR_UDC_EP_ENABLE_INVALID1;
        goto end;
    }

    // if this is used to enable ep0, return false
    dev = ep->dev;
    if(ep == &dev->ep[0])
    {
        rc = ERR_UDC_EP_ENABLE_EP0;
        goto end;
    }

    // if upper level driver not ready or device speed unknown, return false
    if(!dev->driver || (dev->gadget.speed == USB_SPEED_UNKNOWN))
    {
        rc = ERR_UDC_EP_ENABLE_NOT_READY;
        goto end;
    }

    if(ep->num != (desc->bEndpointAddress & 0x0F))
    {
        rc = ERR_UDC_EP_ENABLE_INVALID2;
        goto end;
    }

    // EP should be Bulk
    switch(desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
    {
    case USB_ENDPOINT_XFER_BULK:
        break;
    case USB_ENDPOINT_XFER_INT:
    default:
        rc = ERR_UDC_EP_ENABLE_INVALID3;
        break;
    }

    max = le16_to_cpu(desc->wMaxPacketSize);

    _spin_lock_irqsave(&dev->lock);
    ep->is_in = ((USB_DIR_IN & desc->bEndpointAddress) != 0);
    ep->ep.maxpacket = max;
    ep->stopped = 0;
    ep->stall = 0;
    ep->wedged = 0;
    ep->desc = desc;
    _spin_unlock_irqrestore(&dev->lock);

    // reset data toggle 
    if(ep->is_in)
    {
        mUsbEPinRsTgSet(ep->num);
        mUsbEPinRsTgClr(ep->num);
        mUsbEPinStallClr(ep->num);
    }
    else
    {
        mUsbEPoutRsTgSet(ep->num);
        mUsbEPoutRsTgClr(ep->num);
        mUsbEPoutStallClr(ep->num);
    }

    UDC_INFO " enable %s %s macpacket %d \n", 
        ep->ep.name, (ep->is_in ? "in" : "out"), max UDC_END

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

static int udc_ep_disable(struct usb_ep *_ep)
{
    int rc = 0;
    struct ite_udc *dev;
    struct ite_ep *ep;

    _udc_func_enter;

    ep = container_of(_ep, struct ite_ep, ep);
    if(!_ep || !ep->desc)
    {
        rc = ERR_UDC_EP_DISABLE_NO_DEV;
        goto end;
    }

    dev = ep->dev;
    if(ep == &dev->ep[0])
    {
        rc = ERR_UDC_EP_DISABLE_EP0;
        goto end;
    }

    UDC_DBG " disable %s \n", _ep->name UDC_END

    _spin_lock_irqsave(&dev->lock);
    nuke(ep, -ESHUTDOWN);
    ep_reset(ep);
    _spin_unlock_irqrestore(&dev->lock);

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

static struct usb_request* udc_alloc_request(struct usb_ep *ep)
{
    int rc = 0;
    struct ite_request *req;

    _udc_func_enter;
    UDC_DBG "[%s] %s \n", __FUNCTION__, ep->name UDC_END

    req = malloc(sizeof(struct ite_request));
    if(!req)
    {
        rc = ERR_UDC_ALLOC_REQ_FAIL;
        goto end;
    }
    memset((void*)req, 0x0, sizeof(struct ite_request));
    INIT_LIST_HEAD(&req->queue);

end:
    if(rc) 
        UDC_ERROR "[%s] res = 0x%08X (%s) \n", __FUNCTION__, rc, ep->name UDC_END
    _udc_func_leave;
    return &req->req;
}

static void udc_free_request(struct usb_ep *ep, struct usb_request *_req)
{
    struct ite_request *req;

    _udc_func_enter;
    UDC_DBG "[%s] %s \n", __FUNCTION__, ep->name UDC_END

    if(!ep || !_req)
        return;

    req = container_of(_req, struct ite_request, req);
    if(!list_empty(&req->queue))
        UDC_WARNING " request queue not empty!!! \n" UDC_END

    free(req);

    _udc_func_leave;
    return;
}

static int udc_queue(struct usb_ep *ep, struct usb_request *req);

static int udc_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
    int rc = 0;
    struct ite_udc *dev;
    struct ite_ep *ep;
    struct ite_request *req;

    _udc_func_enter;

    ep = container_of(_ep, struct ite_ep, ep);
    if(!_ep || !_req || (!ep->desc && ep->num != 0))
    {
        rc = ERR_UDC_DEQUE_NO_DEV;
        goto end;
    }

    dev = ep->dev;
    if(!dev->driver)
    {
        rc = ERR_UDC_DEQUE_NO_DRIVER;
        goto end;
    }

    UDC_DBG " %s: %s \n", __FUNCTION__, _ep->name UDC_END

    _spin_lock_irqsave(&dev->lock);

    /* make sure it's actually queued on this endpoint */
    list_for_each_entry(req, &ep->queue, queue)
    {
        if(&req->req == _req)
            break;
    }
    if(&req->req != _req)
    {
        rc = ERR_UDC_DEQUE_INVALID;
        goto end0;
    }

    if(!list_empty(&req->queue))
        done(ep, req, -ECONNRESET);

end0:
    _spin_unlock_irqrestore(&dev->lock);

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

static int enable_fifo_int(struct ite_ep *ep);

static void udc_clear_halt(struct ite_ep *ep)
{
    UDC_INFO " %s => %s \n", __FUNCTION__, ep->ep.name UDC_END

    if(ep->num == 0)
    {
        ep->dev->ep0state = EP0_IDLE;	
    }
    else
    {
        if(ep->is_in)
        {
            mUsbEPinRsTgSet(ep->num);
            mUsbEPinRsTgClr(ep->num);
            mUsbEPinStallClr(ep->num);
        }
        else
        {
            mUsbEPoutRsTgSet(ep->num);
            mUsbEPoutRsTgClr(ep->num);
            mUsbEPoutStallClr(ep->num);
        }
    }

    ep->stopped = 0;
    ep->stall = 0;
    ep->wedged = 0;
    UDC_DBG " %s stall clear \n", ep->ep.name UDC_END

    if(!list_empty(&ep->queue))
        enable_fifo_int(ep);
}

static int udc_set_halt_and_wedge(struct usb_ep *_ep, int value, int wedge)
{
    int rc = 0;
    struct ite_ep *ep;
    _udc_func_enter;

    if(!_ep)
    {
        rc = ERR_UDC_EP_DISABLE_NO_DEV;
        goto end;
    }

    ep = container_of(_ep, struct ite_ep, ep);
    UDC_INFO " %s => ep %d (value=%d) \n", __FUNCTION__, ep->num, value UDC_END

    /* Process the EP 0 */
    if(ep->num == 0)
    {
        mUsbEP0StallSet();
        return rc;
    }
    else if(!ep->desc)
    {
        rc = ERR_UDC_EP_NOT_ENABLE;
        goto end;
    }

    /* Process the EP X */
    _spin_lock_irqsave(&ep->dev->lock);

    // something in queue 
    if(!list_empty(&ep->queue)) 
        rc = -EAGAIN;
    else if(!value)
        udc_clear_halt(ep);
    else
    {
        ep->stall = ep->stopped = 1;
        if(wedge)
            ep->wedged = 1;

        UDC_DBG " %s stall set \n", ep->ep.name UDC_END

        if(ep->is_in)
            mUsbEPinStallSet(ep->num);
        else
            mUsbEPoutStallSet(ep->num);
    }

    _spin_unlock_irqrestore(&ep->dev->lock);

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

static int udc_set_halt(struct usb_ep *_ep, int value)
{
    udc_set_halt_and_wedge(_ep, value, 0);
}

static int udc_set_wedge(struct usb_ep *_ep)
{
    udc_set_halt_and_wedge(_ep, 1, 1);
}

static uint8_t get_fifo_num(struct ite_ep *ep);

static int udc_fifo_status(struct usb_ep *_ep)
{
    int rc = 0;
    struct ite_ep *ep;
    int size = 0;
    uint32_t fifo_num;

    _udc_func_enter;

    if(!_ep)
    {
        rc = ERR_UDC_FIFO_STATUS_NO_DEV;
        goto end;
    }
    ep = container_of(_ep, struct ite_ep, ep);

    if(ep->is_in)
    {
        rc = ERR_UDC_FIFO_STATUS_NO_SUPP;
        goto end;
    }

    if(ep->num == 0)
    {
        /* EP0, only know empty or not */
        size = !mUsbCxFEmpty();
    }
    else
    {
        fifo_num = get_fifo_num(ep);
        if((mUsbFIFOConfigRd(fifo_num) & FIFOEnBit) == 0)
        {
            rc = ERR_UDC_FIFO_STATUS_NOT_ENABLE;
            goto end;
        }
        size = mUsbFIFOOutByteCount(fifo_num);
    }

end:
    check_result(rc);
    _udc_func_leave;
    return (rc ? rc : size);
}

static void udc_fifo_flush(struct usb_ep *_ep)
{
    int rc = 0;
    struct ite_ep *ep;
    uint32_t fifo_num;

    _udc_func_enter;

    if(!_ep)
    {
        rc = ERR_UDC_FIFO_FLUSH_NO_DEV;
        goto end;
    }
    ep = container_of(_ep, struct ite_ep, ep);
    UDC_DBG " %s: %s \n", __FUNCTION__, ep->ep.name UDC_END

    if(!ep->desc && ep->num != 0)
    {
        rc = ERR_UDC_FIFO_FLUSH_INACTIVE;
        goto end;
    }

    if(ep->num == 0)
    {
        mUsbCxFClr();
    }
    else
    {
        fifo_num = get_fifo_num(ep);
        if((mUsbFIFOConfigRd(fifo_num) & FIFOEnBit) == 0)
        {
            rc = ERR_UDC_FIFO_FLUSH_NOT_ENABLE;
            goto end;
        }
        mUsbFIFOReset(fifo_num); 
        ithDelay(10);
        mUsbFIFOResetOK(fifo_num); 
    }

end:
    check_result(rc);
    _udc_func_leave;
    return;
}

static struct usb_ep_ops UDC_ep_ops = 
{
    .enable = udc_ep_enable,
    .disable = udc_ep_disable,

    .alloc_request = udc_alloc_request,
    .free_request = udc_free_request,

    .queue = udc_queue,
    .dequeue = udc_dequeue,

    .set_halt = udc_set_halt,
    .set_wedge = udc_set_wedge,

    .fifo_status = udc_fifo_status,
    .fifo_flush = udc_fifo_flush,
};

/*-------------------------------------------------------------------------*/

static int udc_get_frame(struct usb_gadget* gadget)
{
    int rc = 0;
    _udc_func_enter;
    ithPrintf(" udc_get_frame => TODO! \n");

    _udc_func_leave;
    return rc;
}

static struct usb_gadget_ops UDC_ops = 
{
    udc_get_frame
};


/*-------------------------------------------------------------------------*/

#include "udc.c"

/*-------------------------------------------------------------------------*/

static void udc_reset(struct ite_udc* dev)
{
    _udc_func_enter;

    // init variables
    dev->ep_use_dma = DMA_CHANEL_FREE;

    // init hardware
    udc_dev_init();

    _udc_func_leave;
}

static void udc_reinit(struct ite_udc* dev)
{
    int i;
    struct ite_ep* ep;

    _udc_func_enter;

    INIT_LIST_HEAD (&dev->gadget.ep_list);
    dev->gadget.ep0 = &dev->ep[0].ep;
    dev->gadget.speed = USB_SPEED_UNKNOWN;
    dev->ep0state = EP0_DISCONNECT;
    dev->irqs = 0;

    for(i=0; i<ITE_CURRENT_SUPPORT_EP; i++)
    {
        ep = &dev->ep[i];
        ep->num = i;
        ep->ep.name = names[i];
        UDC_INFO " ep%d name = %s \n", i, ep->ep.name UDC_END

        ep->ep.ops = &UDC_ep_ops;
        list_add_tail(&ep->ep.ep_list, &dev->gadget.ep_list);
        ep->dev = dev;
        INIT_LIST_HEAD(&ep->queue);
        ep_reset(ep);
    }

    dev->ep[0].ep.maxpacket = MAX_EP0_SIZE;
    list_del_init(&dev->ep[0].ep.ep_list);

    _udc_func_leave;
}

static int iteUdcInitialize(void)
{
    int rc = 0;
    _udc_func_enter;

    g_udc = (struct ite_udc*)malloc(sizeof(struct ite_udc));
    if(!g_udc)
    {
        rc = ERR_UDC_ALLOC_CTXT_FAIL;
        goto end;
    }
    memset((void*)g_udc, 0x0, sizeof(struct ite_udc));

    otg_init();

    _spin_lock_init(&g_udc->lock);
    g_udc->gadget.ops = &UDC_ops;
    udc_reset(g_udc);
    udc_reinit(g_udc);

    g_udc->ep0_req = usb_ep_alloc_request(g_udc->gadget.ep0);
    if(g_udc->ep0_req == NULL)
    {
        rc = ERR_UDC_ALLOC_EP0REQ_FAIL;
        goto end;
    }
    g_udc->ep0_req->buf = g_udc->ep0_data;
    g_udc->ep0_req->complete = ep0_complete;

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

static void udc_isr(void)
{
    struct ite_udc* dev = g_udc;

    dev->intr_level1_save = (uint8_t)mUsbIntGroupRegRd();
    dev->intr_level1_mask = (uint8_t)mUsbIntGroupMaskRd();
    dev->intr_level1 = dev->intr_level1_save & ~dev->intr_level1_mask;

    if(dev->intr_level1 /*&& mOtg_Ctrl_CROLE_Rd() != OTG_ROLE_HOST*/) /* note: it will lost last interrupt when unplug from PC */
    {
        dev->irqs++;
        vUsbHandler(dev);
        dev->intr_level1 = 0;
    }
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
    struct ite_udc* dev = g_udc;
    int rc = 0;

    _udc_func_enter;

    if(!dev)
    {
        rc = ERR_UDC_NOT_INIT;
        goto end;
    }

    if(driver->speed != USB_SPEED_HIGH ||
      !driver->bind ||
      !driver->unbind ||
      !driver->setup ||
      !driver->disconnect)
    {
        UDC_ERROR " speed:%d(%d) bind:%p unbind:%p dis:%p setup:%p \n", driver->speed,USB_SPEED_HIGH,
            driver->bind, driver->unbind, driver->disconnect, driver->setup UDC_END
        rc = ERR_UDC_INVAL_PARAM;
        goto end;
    }

    if(dev->driver)
    {
        rc = ERR_UDC_DOUBLE_REGISTER;
        goto end;
    }
    mOtgC_A_Bus_Drop();
    udc_reset(dev);

    dev->driver = driver;
    rc = driver->bind(&dev->gadget);
    if(rc)
    {
        UDC_ERROR " bind to driver %s ==> error 0x%x \n", driver->function, rc UDC_END
        dev->driver = 0;
        rc = ERR_UDC_BIND_FAIL;
        goto end;
    }

    udc_enable(dev);

    UDC_INFO " registered gadget driver '%s' \n", driver->function UDC_END

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

static void stop_activity(struct ite_udc *dev, struct usb_gadget_driver *driver)
{
    int i;
    _udc_func_enter;

    /* don't disconnect drivers more than once */
    if(dev->gadget.speed == USB_SPEED_UNKNOWN)
        driver = NULL;

    udc_reset(dev);

    for(i=0; i<ITE_CURRENT_SUPPORT_EP; i++)
        nuke(&dev->ep[i], -ESHUTDOWN);

    /* report disconnect; the driver is already quiesced */
    if(driver)
        driver->disconnect(&dev->gadget);
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
    struct ite_udc *dev = g_udc;
    int rc=0;

    _udc_func_enter;

    if(!dev)
    {
        rc = ERR_UDC_UNREG_NULL_DEV;
        goto end;
    }
    if(!driver || driver != dev->driver)
    {
        rc = ERR_UDC_UNREG_INVAILD_DRIVER;
        goto end;
    }
    dev->driver = NULL;

    stop_activity(dev, driver);
    driver->unbind(&dev->gadget);

    udc_disable(dev);
    
    /* re-init driver-visible data structures */
    udc_reinit(dev);
    mOtgC_A_Bus_Drive();

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

