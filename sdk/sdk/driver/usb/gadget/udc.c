

static void otg_init(void)
{
    // disable device's group interrupt
    mUsbIntGrpAllDis();

    // set unplug to force device to detach to PC
    mUsbUnPLGSet();

    // turn off all OTG interrutp first. It will be turned on when do role change
    //mOtg_INT_Enable_Clr(0xFFFFFFFF);

    //mOtg_INT_Enable_Set(OTGC_INT_IDCHG | OTGC_INT_AVBUSERR | OTGC_INT_OVC);
}


static void udc_dev_init(void)
{
    UDC_ENTER "%s \n", __FUNCTION__ UDC_END

    // suspend counter
    mUsbIdleCnt(7);

    // clear interrupt
    mUsbIntBusRstClr();
    mUsbIntSuspClr();
    mUsbIntResmClr();

    // disable all fifo interrupt
    mUsbIntFIFO0_3OUTDis();
    mUsbIntFIFO0_3INDis();

    // soft Reset
    mUsbSoftRstSet();
    mUsbSoftRstClr();

    if(mUsbDmaConfigRd() & 0x1)
        ithPrintf(" udc-dma: 0x%X \n", mUsbDmaConfigRd());

    // clear all fifo
    mUsbClrAllFIFOSet();	// will be cleared after one cycle.

    mUsbIntEP0EndDis();
    mUsbIntEP0InDis();		//We will use DMA-finish to instead of it
    mUsbIntEP0OutDis();		//We will use DMA-finish to instead of it

    /* group 2: not used, these two interrupts will trigger when unplug from PC */
    mUsbIntWakeupByVbusDis();
    mUsbIntDevIdleDis();

    if(mOtg_Ctrl_ID_Rd() != OTG_ID_A_TYPE)
    {
        if(mOtg_Ctrl_CROLE_Rd() == OTG_ROLE_DEVICE)
            ithPrintf(" in Device Role! \n");
        else
            ithPrintf(" in Host Role???? \n");

        if(mOtg_Ctrl_A_VBUS_VLD_Rd() == 0)
            ithPrintf(" A-Vbus not valid ?????? <==== check!! (0x80 = 0x%08X D[19])\n\n", mOtg_Ctrl_Rd());

#if 0
        {
            struct ehci_hcd* ehci;
            ehci = hcd_to_ehci(otg_hcd);
            UDC_DATA " 0x%08X = 0x%08X (BIT0: run/stop) \n 0x%08X = 0x%08X (BIT2: port enable/disable) \n", ehci->regs.command, ithReadRegA(ehci->regs.command),
                                    ehci->regs.port_status[0], ithReadRegA(ehci->regs.port_status[0]) UDC_END
            UDC_DATA " 0x%08X = 0x%08X (host interrupt enable??? 0x3F) \n", ehci->regs.intr_enable, ithReadRegA(ehci->regs.intr_enable) UDC_END
            UDC_DATA " 0x80 = 0x%08X (BIT19: V-Bus valid) \n", mOtg_Ctrl_Rd() UDC_END
        }
#endif
        // should disable and clear host interrupt here ????? TODO

        mOtg_Ctrl_A_BUS_DROP_Clr();
        mOtg_Ctrl_A_BUS_REQ_Clr();
        UDC_DATA " 0x80 = 0x%08X \n", mOtg_Ctrl_Rd() UDC_END

        mUsbIntGrpAllEn();
        // 0x13C = 0x600

        mUsbUnPLGClr();
    }
}

static void udc_enable(struct ite_udc* dev)
{
    UDC_ENTER "%s \n", __FUNCTION__ UDC_END
    
    mUsbGlobIntEnSet();
    mUsbChipEnSet();
}

static void udc_disable(struct ite_udc* dev)
{
    UDC_ENTER "%s \n", __FUNCTION__ UDC_END
    
    mUsbGlobIntDis();
    //mUsbChipDis();
    mUsbUnPLGSet();
}

static void vUsbClrFIFORegister(void)
{
    int ep;

    mUsbEPMapAllClr();
    mUsbFIFOMapAllClr();
    mUsbFIFOConfigAllClr();

    for(ep=1; ep<=MAX_EP_NUM; ep++)
    {
        mUsbEPMxPtSzClr(ep, DIRECTION_IN);
        mUsbEPMxPtSzClr(ep, DIRECTION_OUT);
    }
}

static uint8_t get_fifo_num(struct ite_ep *ep)
{
    uint8_t fifo_num;

    if(ep->num == 0)
    {
        fifo_num = FIFOCX;
        return fifo_num;
    }

    fifo_num = mUsbEPMapRd(ep->num);

    if(ep->is_in)
        fifo_num &= 0x0F;
    else
        fifo_num >>= 4;

    return fifo_num;
}

static inline uint8_t get_ep_num(uint8_t fifo_num)
{
    return (uint8_t)(mUsbFIFOMapRd(fifo_num) & 0xF);
}

/*-------------------------------------------------------------------------*/

static void vUsbFIFO_EPxCfg_HS(void)
{
    int i;
    _udc_func_enter;

    vUsbClrFIFORegister();

    // EP4
    mUsbEPMap(EP4, HIGH_EP4_Map);
    mUsbFIFOMap(HIGH_EP4_FIFO_START, HIGH_EP4_FIFO_Map);
    mUsbFIFOConfig(HIGH_EP4_FIFO_START, HIGH_EP4_FIFO_Config);

    for(i=(HIGH_EP4_FIFO_START+1); i<(HIGH_EP4_FIFO_START+(HIGH_EP4_bBLKNO*HIGH_EP4_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (HIGH_EP4_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP4, HIGH_EP4_bDIRECTION, HIGH_EP4_MAXPACKET);
    mUsbEPinHighBandSet(EP4, HIGH_EP4_bDIRECTION, HIGH_EP4_MAXPACKET);

    // EP3
    mUsbEPMap(EP3, HIGH_EP3_Map);
    mUsbFIFOMap(HIGH_EP3_FIFO_START, HIGH_EP3_FIFO_Map);
    mUsbFIFOConfig(HIGH_EP3_FIFO_START, HIGH_EP3_FIFO_Config);

    for(i=(HIGH_EP3_FIFO_START+1); i<(HIGH_EP3_FIFO_START+(HIGH_EP3_bBLKNO*HIGH_EP3_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (HIGH_EP3_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP3, HIGH_EP3_bDIRECTION, HIGH_EP3_MAXPACKET);
    mUsbEPinHighBandSet(EP3, HIGH_EP3_bDIRECTION, HIGH_EP3_MAXPACKET);

    // EP2
    mUsbEPMap(EP2, HIGH_EP2_Map);
    mUsbFIFOMap(HIGH_EP2_FIFO_START, HIGH_EP2_FIFO_Map);
    mUsbFIFOConfig(HIGH_EP2_FIFO_START, HIGH_EP2_FIFO_Config);

    for(i=(HIGH_EP2_FIFO_START+1); i<(HIGH_EP2_FIFO_START+(HIGH_EP2_bBLKNO*HIGH_EP2_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (HIGH_EP2_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP2, HIGH_EP2_bDIRECTION, HIGH_EP2_MAXPACKET);
    mUsbEPinHighBandSet(EP2, HIGH_EP2_bDIRECTION, HIGH_EP2_MAXPACKET);

    // EP1
    mUsbEPMap(EP1, HIGH_EP1_Map);
    mUsbFIFOMap(HIGH_EP1_FIFO_START, HIGH_EP1_FIFO_Map);
    mUsbFIFOConfig(HIGH_EP1_FIFO_START, HIGH_EP1_FIFO_Config);

    for(i=(HIGH_EP1_FIFO_START+1); i<(HIGH_EP1_FIFO_START+(HIGH_EP1_bBLKNO*HIGH_EP1_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (HIGH_EP1_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP1, HIGH_EP1_bDIRECTION, HIGH_EP1_MAXPACKET);
    mUsbEPinHighBandSet(EP1, HIGH_EP1_bDIRECTION, HIGH_EP1_MAXPACKET);

    // SOF mask timer
    mUsbSofMskTimer(0x44C);

//	ithPrintRegA(udc_reg(0x160), (0x1B0-0x160));
}

static void vUsbFIFO_EPxCfg_FS(void)
{
    int i;
    _udc_func_enter;

    vUsbClrFIFORegister();

    // EP4
    mUsbEPMap(EP4, FULL_EP4_Map);
    mUsbFIFOMap(FULL_EP4_FIFO_START, FULL_EP4_FIFO_Map);
    mUsbFIFOConfig(FULL_EP4_FIFO_START, FULL_EP4_FIFO_Config);

    for(i=(FULL_EP4_FIFO_START+1); i<(FULL_EP4_FIFO_START+(FULL_EP4_bBLKNO*FULL_EP4_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (FULL_EP4_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP4, FULL_EP4_bDIRECTION, FULL_EP4_MAXPACKET);
    mUsbEPinHighBandSet(EP4, FULL_EP4_bDIRECTION, FULL_EP4_MAXPACKET);

    // EP3
    mUsbEPMap(EP3, FULL_EP3_Map);
    mUsbFIFOMap(FULL_EP3_FIFO_START, FULL_EP3_FIFO_Map);
    mUsbFIFOConfig(FULL_EP3_FIFO_START, FULL_EP3_FIFO_Config);

    for(i=(FULL_EP3_FIFO_START+1); i<(FULL_EP3_FIFO_START+(FULL_EP3_bBLKNO*FULL_EP3_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (FULL_EP3_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP3, FULL_EP3_bDIRECTION, FULL_EP3_MAXPACKET);
    mUsbEPinHighBandSet(EP3, FULL_EP3_bDIRECTION, FULL_EP3_MAXPACKET);

    // EP2
    mUsbEPMap(EP2, FULL_EP2_Map);
    mUsbFIFOMap(FULL_EP2_FIFO_START, FULL_EP2_FIFO_Map);
    mUsbFIFOConfig(FULL_EP2_FIFO_START, FULL_EP2_FIFO_Config);

    for(i=(FULL_EP2_FIFO_START+1); i<(FULL_EP2_FIFO_START+(FULL_EP2_bBLKNO*FULL_EP2_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (FULL_EP2_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP2, FULL_EP2_bDIRECTION, FULL_EP2_MAXPACKET);
    mUsbEPinHighBandSet(EP2, FULL_EP2_bDIRECTION, FULL_EP2_MAXPACKET);

    // EP1
    mUsbEPMap(EP1, FULL_EP1_Map);
    mUsbFIFOMap(FULL_EP1_FIFO_START, FULL_EP1_FIFO_Map);
    mUsbFIFOConfig(FULL_EP1_FIFO_START, FULL_EP1_FIFO_Config);

    for(i=(FULL_EP1_FIFO_START+1); i<(FULL_EP1_FIFO_START+(FULL_EP1_bBLKNO*FULL_EP1_bBLKSIZE)); i++)
        mUsbFIFOConfig(i, (FULL_EP1_FIFO_Config & (~BIT7)));

    mUsbEPMxPtSz(EP1, FULL_EP1_bDIRECTION, FULL_EP1_MAXPACKET);
    mUsbEPinHighBandSet(EP1, FULL_EP1_bDIRECTION, FULL_EP1_MAXPACKET);

    // SOF mask timer
    mUsbSofMskTimer(0x2710);
}

/*-------------------------------------------------------------------------*/
static int enable_fifo_int(struct ite_ep *ep)
{
    if(ep->is_in)
        mUsbIntFXINEn(get_fifo_num(ep));
    else
    {
        mUsbIntFXOUTEn(get_fifo_num(ep));
    }
}

static int start_dma(struct ite_ep *ep, struct ite_request *req)
{
    struct ite_udc *dev = ep->dev;
    uint32_t start = (uint32_t)req->req.buf;
    uint32_t fifo_num, fifo_sel;

    UDC_DBG " start_dma: ep%d-%s, addr=0x%X, len=%d \n", ep->num, (ep->is_in?"in":"out"), start, req->req.length UDC_END

    if(dev->ep0state == EP0_SUSPEND)
    {
        done(ep, req, -ESHUTDOWN);
        return 0;
    }

    // get the fifo select
    fifo_num = get_fifo_num(ep);
    if(fifo_num == FIFOCX)
        fifo_sel = UDC_DMA2CxFIFO;
    else
        fifo_sel = (1 << fifo_num);

    // program dma register
    if(ep->is_in)
    {
        if(req->req.length == 0)
        {
            if(ep->num==0)
            {
                mUsbEP0DoneSet();
                done(ep, req, 0);
            }
            else
            {
                mUsbEPinZeroSet(ep->num);
                dev->ep_use_dma = ep->num;
            }
            return 0;
        }
        else
        {
            ep->dma_set_len = req->req.length;
            mUsbDmaConfig(req->req.length, DIRECTION_IN);
        }
    }
    else
    {
        if(ep->num == 0)
        {
            mUsbDmaConfig(req->req.length, DIRECTION_OUT);
        }
        else
        {
            uint32_t fifo_byte_count = mUsbFIFOOutByteCount(fifo_num);
            uint32_t maxPacketSize = mUsbEPMxPtSzRd(ep->num, DIRECTION_OUT);
            if(fifo_byte_count == 0)
            {
                UDC_ERROR " start_dma: fifo #%d 0 byte??? \n", fifo_num UDC_END
                return 0;
            }

			if (req->req.length == 0) {
				req->req.length = ep->dma_set_len = fifo_byte_count;
			} else
            if(fifo_byte_count < maxPacketSize)
                ep->dma_set_len = fifo_byte_count;
            else
                ep->dma_set_len = req->req.length;

            mUsbDmaConfig(ep->dma_set_len, DIRECTION_OUT);

            UDC_DBG " start_dma: fifo out byte count 0x%X, ep%d (MaxPackSize:0x%X), dma length:0x%X \n", 
                fifo_byte_count, ep->num, maxPacketSize, ep->dma_set_len UDC_END
        }
    }
#if defined(CFG_CPU_WB)
    ithFlushDCacheRange((void*)start, ep->dma_set_len);
    ithFlushMemBuffer();
#endif

    mUsbDMA2FIFOSel(fifo_sel);
    mUsbDmaAddr(start);
    UDC_DBG " mUsbDmaConfigRd()=0x%X (request len=0x%x) \n", mUsbDmaConfigRd(), req->req.length UDC_END
    UDC_DBG " mUsbDMA2FIFOSel()=0x%X \n", mUsbDMA2FIFORd() UDC_END
    UDC_DBG " mUsbDmaAddr()=0x%X \n", mUsbDmaAddrRd() UDC_END

    // record who use the DMA channel
    dev->ep_use_dma = ep->num;

    // enable dma interrupt
    mUsbIntDmaErrEn();
    mUsbIntDmaFinishEn();

    // fire dma
    mUsbDmaStart();
    return 0;
}

static void dma_advance(struct ite_udc* dev, struct ite_ep* ep)
{
    struct ite_request *req;

    _udc_func_enter;

    if(list_empty(&ep->queue))
    {
stop:
        mUsbDmaStop();
        mUsbIntDmaErrDis();
        mUsbIntDmaFinishDis();
        mUsbDMA2FIFOSel(UDC_DMA2FIFO_Non);
        dev->ep_use_dma = DMA_CHANEL_FREE;

        if(ep->num)
        {
            if(ep->is_in)
                mUsbIntFXINDis(get_fifo_num(ep));
            else
                mUsbIntFXOUTDis(get_fifo_num(ep));
        }
        return;
    }

    req = list_entry(ep->queue.next, struct ite_request, queue);

    // get length
    if(mUsbIntDmaErrRd() == 0)
        req->req.actual = ep->dma_set_len;
    else
        req->req.actual = 0;

    //UDC_DBG " dma done: ep%d-%s, %d/%d bytes, req:0x%X \n",
    UDC_CTRL " dma done: ep%d-%s, %d/%d bytes, req:0x%X \n",
        ep->num, ep->is_in?"in":"out", req->req.actual, req->req.length, req UDC_END

    UDC_DLEN "%d\n", req->req.actual UDC_END

    if(!ep->is_in && req->req.actual)
        ithInvalidateDCacheRange((void*)req->req.buf, req->req.actual);

    // one the request
    done(ep, req, 0);

    if(list_empty(&ep->queue))
        goto stop;

    // start another request dma
    if(ep->num == 0)
    {
        req = list_entry(ep->queue.next, struct ite_request, queue);
        start_dma(ep, req);
    }
    else
    {
        // free the DMA resource => Waiting for next DMA-Start
        mUsbDmaStop();
        mUsbIntDmaErrDis();
        mUsbIntDmaFinishDis();
        mUsbDMA2FIFOSel(UDC_DMA2FIFO_Non);
        dev->ep_use_dma = DMA_CHANEL_FREE;

        enable_fifo_int(ep);
    }
}

static int udc_queue(struct usb_ep *_ep, struct usb_request *_req)
{
    int rc = 0;
    struct ite_request *req;
    struct ite_ep *ep;
    struct ite_udc *dev;
    int request = 0;

    _udc_func_enter;

    // <1> check request & ep & dev
    req = container_of(_req, struct ite_request, req);
    if(!_req || !_req->complete || !_req->buf || !list_empty(&req->queue))
    {
        rc = ERR_UDC_REQ_PARAM_INVALID;
        goto end;
    }

    ep = container_of(_ep, struct ite_ep, ep);
    if(!_ep || (ep->num != 0 && !ep->desc))
    {
        rc = ERR_UDC_EP_PARAM_INVALID;
        goto end;
    }

    // check Cx 0 byte
    if(req->req.length == 0)
    {
        // request control transfer 0 byte
        if(ep->num==0)
        {
            mUsbEP0DoneSet();
            mUsbCfgSet();
            UDC_INFO " ep0 (0 byte) => return (set configuration) \n" UDC_END
            return 0;
        }
        else
            UDC_WARNING " udc_queue: %s length 0 \n", _ep->name UDC_END
    }

    dev = ep->dev;
    if(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
    {
        rc = -ESHUTDOWN;
        goto end;
    }

    UDC_DBG " %s queue req 0x%x, len %d, buf 0x%X \n", _ep->name, _req, _req->length, _req->buf UDC_END

    // <2> set the req's status ...
    _spin_lock_irqsave(&dev->lock);

    /* can't touch registers when suspended */
    if(dev->ep0state == EP0_SUSPEND)
    {
        rc = -ESHUTDOWN;
        goto leave;
    }

    if(list_empty(&ep->queue))
        request = 1;

    _req->status = -EINPROGRESS;
    _req->actual = 0;

    // <3> for control-in => force short packet
    if(ep->num == 0 && ep->is_in)
        _req->zero = 1;

    if((ep->num == 0) && list_empty(&ep->queue) && !ep->stopped)
    {
        if(dev->ep_use_dma != DMA_CHANEL_FREE)
            UDC_ERROR " ep0 queue => dma not free ??? \n" UDC_END
        rc = start_dma(ep, req);
        if(rc != 0)	{ /* ?? */ }
    }

    // add request to queue
    if(req != 0)
    {
        UDC_REQ " add req:0x%x to ep %d queue \n", req, ep->num UDC_END
        list_add_tail(&req->queue, &ep->queue);
    }

    // enable the FIFO interrupt
    if((ep->num > 0) && request && !ep->stall)
    {
        enable_fifo_int(ep);

        UDC_DBG " enable ep%d (FIFO:%d) interrupt. reg=0x%X, length=%d \n",
            ep->num, get_fifo_num(ep), mUsbIntSrc1MaskRd(), req->req.length UDC_END
    }
leave:
    _spin_unlock_irqrestore(&dev->lock);

end:
    check_result(rc);
    _udc_func_leave;
    return rc;
}

static void ep_isr(struct ite_udc *dev, uint8_t ep_num)
{
    struct ite_ep *ep;
    struct ite_request *req;

    if(dev->ep_use_dma != DMA_CHANEL_FREE)
    {
        UDC_WARNING " dma in used!(ep%d) \n", dev->ep_use_dma UDC_END
        return;
    }

    ep = &dev->ep[ep_num];

    if(list_empty(&ep->queue))
    {
        if(ep->is_in)
            mUsbIntFXINDis(get_fifo_num(ep));
        else
            mUsbIntFXOUTDis(get_fifo_num(ep));

        UDC_DBG " ep_isr(ep_num:%d) => list empty \n", ep->num UDC_END
    }
    else
    {
        req = list_entry(ep->queue.next, struct ite_request, queue);
        start_dma(ep, req);
    }
}

/*-------------------------------------------------------------------------*/
static void request_error(struct ite_udc* dev)
{
    mUsbEP0StallSet();
    dev->ep[0].stopped = 1;
    dev->ep[0].stall = 1;
    dev->ep0state = EP0_STALL;
}

static void clear_feature(struct ite_udc* dev, struct usb_ctrlrequest* ctrl)
{
    uint8_t ep_num = 0;
    uint8_t fifo_num = 0;
    uint8_t dir_in = 0;
    struct ite_ep *ep;

    switch(ctrl->wValue)
    {
    case 0: // ENDPOINT_HALE
        UDC_CTRL " clear feature: ENDPOINT_HALE \n" UDC_END
        ep_num = ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK;
        dir_in = ctrl->wIndex >> 7;
        ep = &dev->ep[ep_num];
        if(ep_num)
        {
            if(ep->wedged)
                break;

            if(ep->stall)
            {
                fifo_num = get_fifo_num(ep);
                if(fifo_num > MAX_FIFO_NUM)
                {
                    UDC_ERROR " clear_feature: fifo num invalid?? (ep%d: fifo %d) \n", ep->num, fifo_num UDC_END
                    goto stallcx;
                }
                if((mUsbFIFOConfigRd(fifo_num) & FIFOEnBit) == 0)
                {
                    UDC_ERROR " clear_feature: fifo not enabled?? (ep%d: fifo %d) \n", ep->num, fifo_num UDC_END
                    goto stallcx;
                }
            }
        }
        udc_clear_halt(ep);
        break;

    case 1: // Device Remote Wakeup
        UDC_CTRL " clear feature: Device Remote Wakeup \n" UDC_END
        mUsbRmWkupClr();
        break;

    case 2: // Test Mode
        UDC_CTRL " clear feature: Test Mode \n" UDC_END
        goto stallcx;
        break;

    default:
        UDC_ERROR " clear_feature: Unknown?? \n" UDC_END
        goto stallcx;
        break;
    }

    mUsbEP0DoneSet();
    return;

stallcx:
    request_error(dev);
}

static void get_status(struct ite_udc* dev, struct usb_ctrlrequest* ctrl)
{
    uint8_t ep_num, dir_in;

    dev->ep0_data[0] = 0;
    dev->ep0_data[1] = 0;

    if((ctrl->bRequestType & USB_DIR_IN) != USB_DIR_IN)
    {
        UDC_ERROR " get status: invalid request direction !!\n" UDC_END 
        return;
    }

    switch(ctrl->bRequestType & USB_RECIP_MASK)
    {
    case USB_RECIP_DEVICE:
        UDC_CTRL " get status: USB_RECIP_DEVICE \n" UDC_END
        // TODO: remote wakeup??
        if(dev->driver->getselfpower())
            dev->ep0_data[0] = 1 << USB_DEVICE_SELF_POWERED;
        break;

    case USB_RECIP_INTERFACE:
        UDC_CTRL " get status: USB_RECIP_INTERFACE \n" UDC_END
        ithPrintf(" get status: USB_RECIP_INTERFACE \n"); // TODO
        /* Return 2-byte ZEROs Interface status to Host */
        break;

    case USB_RECIP_ENDPOINT:
        UDC_CTRL " get status: USB_RECIP_ENDPOINT, wIndex:0x%X \n", ctrl->wIndex UDC_END
        ep_num = ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK;
        dir_in = (ctrl->wIndex & USB_ENDPOINT_DIR_MASK) ? 1 : 0;
        if(ep_num == 0)
        {
            if(dev->ep0state == EP0_STALL)
                dev->ep0_data[0] = 0;
        }
        else
        {
            if(dir_in)
                dev->ep0_data[0] = mUsbEPinStallST(ep_num);
            else
                dev->ep0_data[0] = mUsbEPoutStallST(ep_num);
        }
        break;

    default:
        request_error(dev);
        return;
    }

    dev->ep0_req->length = 2;
    dev->ep0_req->buf = dev->ep0_data;
    udc_queue(dev->gadget.ep0, dev->ep0_req);
}

static void set_address(struct ite_udc* dev, struct usb_ctrlrequest* ctrl)
{
    ithPrintf(" addr: %d \n", ctrl->wValue);

    if(ctrl->wValue > 0x0100)
        request_error(dev);
    else
    {
        mUsbDevAddrSet(ctrl->wValue);
        mUsbEP0DoneSet();
    }
}

static uint8_t test_packet[60] = {0};

static void set_feature(struct ite_udc* dev, struct usb_ctrlrequest* ctrl)
{
    uint8_t ep_num = 0;
    uint8_t fifo_num = 0;
    struct ite_ep *ep;

    switch(ctrl->wValue)
    {
    case 0: // ENDPOINT_HALE
        ep_num = ctrl->wIndex & USB_ENDPOINT_NUMBER_MASK;
        UDC_CTRL " set_feature: ENDPOINT_HALE ep:%d\n", ep_num UDC_END

        ep = &dev->ep[ep_num];
        if(ep_num==0)
        {
            udc_set_halt(&ep->ep, 1);
        }
        else if(ep_num)
        {
            fifo_num = get_fifo_num(ep);
            if(fifo_num > MAX_FIFO_NUM)
            {
                UDC_ERROR " set_feature: fifo num invalid?? (ep%d: fifo %d) \n", ep->num, fifo_num UDC_END
                goto stallcx;
            }
            if((mUsbFIFOConfigRd(fifo_num) & FIFOEnBit) == 0)
            {
                UDC_ERROR " set_feature: fifo not enabled?? (ep%d: fifo %d) \n", ep->num, fifo_num UDC_END
                goto stallcx;
            }
            if(ep->is_in)
                mUsbEPinStallSet(ep->num);
            else
                mUsbEPoutStallSet(ep->num);
        }
        //udc_set_halt(&ep->ep, 1);  // note: non-ep0 will block by non-empty queue
        break;

    case 1: // Device Remote Wakeup
        UDC_CTRL " set_feature: Device Remote Wakeup \n" UDC_END
        mUsbRmWkupSet();
        break;

    case 2: // Test Mode
        UDC_CTRL " set_feature: Test Mode \n" UDC_END
        switch((ctrl->wIndex >> 8))  // Test Selector
        {
        case 0x1:   // Test_J
            mUsbTsMdWr(TEST_J);
            break;
        case 0x2:   // Test_K
            mUsbTsMdWr(TEST_K);
            break;
        case 0x3:   // Test_SE0_NAK
            mUsbTsMdWr(TEST_SE0_NAK);
            break;
        case 0x4:   // Test_Packet
            {
                uint8_t *tp, i;

                mUsbTsMdWr(TEST_PKY);
                mUsbEP0DoneSet();    // special case: follow the test sequence

                tp = test_packet;
                for (i = 0; i < 9; i++)/*JKJKJKJK x 9*/
                  *tp++ = 0x00;
                     
                 for (i = 0; i < 8; i++) /* 8*AA */
                  *tp++ = 0xAA;
                     
                 for (i = 0; i < 8; i++) /* 8*EE */  
                  *tp++ = 0xEE;
                
                 *tp++ = 0xFE;
                     
                 for (i = 0; i < 11; i++) /* 11*FF */
                  *tp++ = 0xFF;
                     
                *tp++ = 0x7F;
                *tp++ = 0xBF;
                *tp++ = 0xDF;
                *tp++ = 0xEF;
                *tp++ = 0xF7;
                *tp++ = 0xFB;
                *tp++ = 0xFD;
                *tp++ = 0xFC;
                *tp++ = 0x7E;
                *tp++ = 0xBF;
                *tp++ = 0xDF;
                *tp++ = 0xEF;
                *tp++ = 0xF7;
                *tp++ = 0xFB;
                *tp++ = 0xFD;
                *tp++ = 0x7E;

                dev->ep0_req->length = 53;
                dev->ep0_req->buf = test_packet;
                dev->ep[0].is_in = 1;
                udc_queue(dev->gadget.ep0, dev->ep0_req);
            }
            break;
        case 0x5:   // Test_Force_Enable
        default:
            break;
        }
        break;

    default:
        UDC_ERROR " set_feature: Unknown?? \n" UDC_END
        goto stallcx;
        break;
    }

    mUsbEP0DoneSet();
    return;

stallcx:
    request_error(dev);
}

static void synch_frame(struct ite_udc* dev, struct usb_ctrlrequest* ctrl)
{
    UDC_CTRL " synch frame! \n" UDC_END
    request_error(dev);
    // TODO
}


/*-------------------------------------------------------------------------*/
static void dma_reset(struct ite_udc* dev)
{
    struct ite_ep* ep;
    uint32_t fifo_num;

    if(dev->ep_use_dma == DMA_CHANEL_FREE)
        return;

    // do nothing
    UDC_DATA " dma:0x%X, src2:0x%X \n", mUsbDmaConfigRd(), mUsbIntSrc2Rd() UDC_END
    return;
}

static void vUsb_reset(struct ite_udc* dev)
{
    ithPrintf(" Bus Reset \n");

    mUsbIntBusRstClr();
    dev->gadget.speed = USB_SPEED_UNKNOWN;
    dma_reset(dev);

    #if defined(SUSPEND_ENABLE)
    mUsbGoSuspendClr();
    #endif
    dev->ep0state = EP0_IDLE;
}

static void vUsb_suspend(struct ite_udc* dev)
{
    ithPrintf(" Suspend \n");

    mUsbIntSuspClr();
    #if defined(SUSPEND_ENABLE)
    mUsbGoSuspend();
    #endif

    dma_reset(dev);

    /* clear FIFO for suspend to avoid extra interrupt */
    mUsbFIFODone(0);
    mUsbFIFODone(1);
    mUsbFIFODone(2);
    mUsbFIFODone(3);

    #if defined(SUSPEND_ENABLE)
    /* workaround: 0x80 read in iteOtgIsDeviceMode() will be 0x0 issue */ 
    ithReadRegA(udc_reg(0x80));
    #endif

    dev->ep0state = EP0_SUSPEND;
}

static void vUsb_resume(struct ite_udc* dev)
{
    ithPrintf(" Resume \n");

    mUsbIntResmClr();
    #if defined(SUSPEND_ENABLE)
    mUsbGoSuspendClr();
    #endif

    dma_reset(dev);
    dev->ep0state = EP0_IDLE;
}

// Read 8-byte setup packet from FIFO
static void setup_packet(struct usb_ctrlrequest* ctrl)
{
    uint32_t cmd[2];
    uint8_t  *u8cmd;
    
    mUsbDMA2FIFOSel(UDC_DMA2CxFIFO);
    cmd[0] = mUsbEP0CmdDataRdDWord();
    cmd[1] = mUsbEP0CmdDataRdDWord();
    mUsbDMA2FIFOSel(UDC_DMA2FIFO_Non);
    cmd[0] = le32_to_cpu(cmd[0]);
    cmd[1] = le32_to_cpu(cmd[1]);
    u8cmd = (uint8_t*)cmd;

    UDC_DATA " setup: %02X %02X %02X %02X %02X %02X %02X %02X \n", 
        u8cmd[0], u8cmd[1], u8cmd[2], u8cmd[3], u8cmd[4], u8cmd[5], u8cmd[6], u8cmd[7] UDC_END

    ctrl->bRequestType = u8cmd[0];
    ctrl->bRequest = u8cmd[1];
    ctrl->wValue = (u8cmd[3] << 8) | u8cmd[2];
    ctrl->wIndex= (u8cmd[5] << 8) | u8cmd[4];
    ctrl->wLength= (u8cmd[7] << 8) | u8cmd[6];
}

static void vUsb_ep0setup(struct ite_udc* dev)
{
    struct usb_ctrlrequest	ctrl={0};
    int to_gadget=0, rc=0;
    UDC_CTRL " ep0 Setup \n" UDC_END

    if(dev->gadget.speed == USB_SPEED_UNKNOWN)
    {
        if(mUsbHighSpeedST())
        {
            UDC_INFO " High Speed! \n" UDC_END
            dev->gadget.speed = USB_SPEED_HIGH;
            vUsbFIFO_EPxCfg_HS(); // set the FIFO information
        }
        else
        {
            UDC_INFO " Full Speed! \n" UDC_END
            dev->gadget.speed = USB_SPEED_FULL;
            vUsbFIFO_EPxCfg_FS(); // set the FIFO information
        }
    }

    // dequeue all request
    nuke(&dev->ep[0], 0);
    dev->ep[0].stopped = 0;

    setup_packet(&ctrl);

    if(ctrl.bRequestType & USB_DIR_IN)
    {
        dev->ep[0].is_in = 1;
        dev->ep0state = EP0_IN;
    }
    else
    {
        dev->ep[0].is_in = 0;
        dev->ep0state = EP0_OUT;
    }

    if((ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD)
    {
        switch(ctrl.bRequest)
        {
        case USB_REQ_CLEAR_FEATURE:
            clear_feature(dev, &ctrl);
            break;
        case USB_REQ_GET_STATUS:
            get_status(dev, &ctrl);
            break;
        case USB_REQ_SET_ADDRESS:
            set_address(dev, &ctrl);
            break;
        case USB_REQ_SET_FEATURE:
            set_feature(dev, &ctrl);
            break;
        case USB_REQ_SYNCH_FRAME:
            synch_frame(dev, &ctrl);
            break;
        default:
            to_gadget = 1;
            break;
        }
    }
    else
    {
        to_gadget = 1;
    }

    if(to_gadget)
    {
        rc = dev->driver->setup(&dev->gadget, &ctrl);
        if(rc < 0)
        {
            UDC_ERROR " gadget driver setup() return 0x%X(%d) \n", rc, rc UDC_END
            request_error(dev);
        }
    }
}

static void vUsb_ep0end(struct ite_udc* dev)
{
    UDC_CTRL " ep0 End \n" UDC_END

    dev->eUsbCxCommand = CMD_VOID;
}

static void vUsb_ep0fail(struct ite_udc* dev)
{
    UDC_CTRL " ep0 Fail \n" UDC_END

    udc_set_halt(dev->gadget.ep0, 1);
}

static void vUsbHandler(struct ite_udc* dev)
{
    uint32_t intr_level2_ori;
    uint32_t intr_level2_mask;
    uint32_t intr_level2;

    UDC_CTRL " intr_level1: 0x%X \n", dev->intr_level1 UDC_END

    if(dev->intr_level1 & BIT2)
    {
        intr_level2_ori = mUsbIntSrc2Rd();
        intr_level2_mask = mUsbIntSrc2MaskRd();
        intr_level2 = intr_level2_ori & ~intr_level2_mask;
        UDC_CTRL " IntrSrc2: 0x%X & 0x%X = 0x%X \n", intr_level2_ori, ~intr_level2_mask, intr_level2 UDC_END

        if(intr_level2 & BIT0)
            vUsb_reset(dev);
        if(intr_level2 & BIT1)
            vUsb_suspend(dev);
        if(intr_level2 & BIT2)
            vUsb_resume(dev);
        if(intr_level2 & BIT3)
        {
            mUsbIntIsoSeqErrClr();
            UDC_ERROR " ISO sequence error??? \n" UDC_END
        }
        if(intr_level2 & BIT4)
        {
            mUsbIntIsoSeqAbortClr();
            UDC_CTRL " ISO sequence abort??? \n" UDC_END
        }
        if(intr_level2 & BIT5)
        {
            UDC_CTRL " Tx 0 byte (ep: 0x%X) \n", mUsbIntTX0ByteRd() UDC_END
            mUsbIntTX0ByteSetClr(mUsbIntTX0ByteRd());
            mUsbIntTX0ByteClr();
            // stall should active after Tx 0 byte complete!
            if(dev->ep_use_dma != DMA_CHANEL_FREE)
                dma_advance(dev, &(dev->ep[dev->ep_use_dma]));
        }
        if(intr_level2 & BIT6)
        {
            mUsbIntRX0ByteClr();
            UDC_CTRL " Rx 0 byte \n" UDC_END
        }
        if(intr_level2 & BIT7)
        {
            mUsbIntDmaFinishClr();
            UDC_CTRL " DMA finish %d\n", dev->ep_use_dma UDC_END
            if(dev->ep_use_dma != DMA_CHANEL_FREE)
                dma_advance(dev, &(dev->ep[dev->ep_use_dma]));
        }
        if(intr_level2 & BIT8)
        {
            mUsbIntDmaErrClr();
            UDC_ERROR " DMA error \n" UDC_END
        }
    }

    if(dev->intr_level1 & BIT0)
    {
        intr_level2_ori = mUsbIntSrc0Rd();
        intr_level2_mask = mUsbIntSrc0MaskRd();
        intr_level2 = intr_level2_ori & ~intr_level2_mask;
        UDC_CTRL " IntrSrc0: 0x%X & 0x%X = 0x%X \n", intr_level2_ori, ~intr_level2_mask, intr_level2 UDC_END

        dev->ep[0].irqs++;
        if(intr_level2 & BIT5) // gets the highest priority
        {
            mUsbIntEP0AbortClr();
            UDC_ERROR " ep0 Abort??? \n" UDC_END
        }
        if(intr_level2 & BIT0)
            vUsb_ep0setup(dev);
        if(intr_level2 & BIT1)
            UDC_CTRL " ep0 In \n" UDC_END
        if(intr_level2 & BIT2)
            UDC_CTRL " ep0 Out \n" UDC_END
        if(intr_level2 & BIT3)
            vUsb_ep0end(dev);
        if(intr_level2 & BIT4)
            vUsb_ep0fail(dev);
    }

    if(dev->intr_level1 & BIT1)
    {
        intr_level2_ori = mUsbIntSrc1Rd();
        intr_level2_mask = mUsbIntSrc1MaskRd();
        intr_level2 = intr_level2_ori & ~intr_level2_mask;
        UDC_CTRL " IntrSrc1: 0x%X & 0x%X = 0x%X \n", intr_level2_ori, ~intr_level2_mask, intr_level2 UDC_END

        if(intr_level2 & BIT1)
            UDC_CTRL " FIFO0 Short \n" UDC_END
        else if(intr_level2 & BIT0)
            UDC_CTRL " FIFO0 Out \n" UDC_END

        if(intr_level2 & BIT3)
        {
            UDC_CTRL " FIFO1 Short \n" UDC_END
            ep_isr(dev, get_ep_num(1));
        }
        else if(intr_level2 & BIT2)
        {
            UDC_CTRL " FIFO1 Out \n" UDC_END
            ep_isr(dev, get_ep_num(1));
        }

        if(intr_level2 & BIT5)
        {
            UDC_CTRL " FIFO2 Short \n" UDC_END
        }
        else if(intr_level2 & BIT4)
        {
            UDC_CTRL " FIFO2 Out \n" UDC_END
        }

        if(intr_level2 & BIT7) {
            UDC_CTRL " FIFO3 Short \n" UDC_END
            ep_isr(dev, get_ep_num(3));
		}
        else if(intr_level2 & BIT6) {
            UDC_CTRL " FIFO3 Out \n" UDC_END
            ep_isr(dev, get_ep_num(3));
		}
        if(intr_level2 & BIT16)
        {
            UDC_CTRL " FIFO0 In \n" UDC_END
            ep_isr(dev, get_ep_num(0));
        }
        if(intr_level2 & BIT17)
            UDC_CTRL " FIFO1 In \n" UDC_END
        if(intr_level2 & BIT18) {
            UDC_CTRL " FIFO2 In \n" UDC_END
            ep_isr(dev, get_ep_num(2));
		}
        if(intr_level2 & BIT19)
            UDC_CTRL " FIFO3 In \n" UDC_END
    }
}
