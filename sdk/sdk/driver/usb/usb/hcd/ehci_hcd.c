#include "usb/config.h"
#include "usb/usb/host.h"


//=============================================================================
//                              Constant Definition
//=============================================================================
/* magic numbers that can affect system performance */
#define	EHCI_TUNE_CERR		3	/* 0-3 qtd retries; 0 == don't stop */
#define	EHCI_TUNE_RL_HS		4	/* nak throttle; see 4.9 */
#define	EHCI_TUNE_RL_TT		0
#define	EHCI_TUNE_MULT_HS	1	/* 1-3 transactions/uframe; 4.10.3 */
#define	EHCI_TUNE_MULT_TT	1
#define	EHCI_TUNE_FLS		1   /* (medium) 512-frame schedule */


/* Initial IRQ latency:  faster than hw default */
static int log2_irq_thresh = 0;		// 0 to 6

/* initial park setting:  slower than hw default */
static unsigned park = 0;

#define	INTR_MASK   (STS_IAA|STS_FATAL|STS_ERR|STS_INT|STS_PCD)


/*
 * handshake - spin reading hc until handshake completes or fails
 * @ptr: address of hc register to be read
 * @mask: bits to look at in result of read
 * @done: value of those bits when handshake succeeds
 * @usec: timeout in microseconds
 *
 * Returns negative errno, or zero on success
 *
 * Success happens when the "mask" bits have the specified value (hardware
 * handshake done).  There are two failure modes:  "usec" have passed (major
 * hardware flakeout), or the register reads as all-ones (hardware removed).
 *
 * That last failure should_only happen in cases like physical cardbus eject
 * before driver shutdown. But it also seems to be caused by bugs in cardbus
 * bridge shutdown:  shutting down the bridge before the devices using it.
 */
static int handshake (struct ehci_hcd *ehci, uint32_t reg,
              u32 mask, u32 done, int usec)
{
	u32	result;

    do {
        result = ithReadRegA(reg);
        if(result == ~(uint32_t)0)		/* card removed */
            return -ENODEV;
        result &= mask;
        if(result == done)
            return 0;
		udelay (1);
        usec--;
    } while (usec > 0);

    LOG_ERROR " %s: reg 0x%08X, mask 0x%08X, done 0x%08X, usec %d \n", __func__, 
                reg, mask, done, usec LOG_END

    return -ETIMEDOUT;
}
  
/* force HC to halt state from unknown (EHCI spec section 2.3) */
static int ehci_halt (struct ehci_hcd *ehci)
{ 
    int rc;
    uint32_t    temp = ithReadRegA(ehci->regs.status);

    _hcd_func_enter;

    /* disable any irqs left enabled by previous code */
    ithWriteRegA(ehci->regs.intr_enable, 0);

    if ((temp & STS_HALT) != 0)
        return 0;

    ithWriteRegMaskA(ehci->regs.command, 0, CMD_RUN);
    rc = handshake (ehci, ehci->regs.status,
              STS_HALT, STS_HALT, 16 * 125);

    check_result(rc);
    _hcd_func_leave;

    return rc;
}

static int handshake_on_error_set_halt(struct ehci_hcd *ehci, uint32_t reg,
                       u32 mask, u32 done, int usec)
{
    int error;

    error = handshake(ehci, reg, mask, done, usec);
    if (error) {
        ehci_halt(ehci);
        ehci_to_hcd(ehci)->state = HC_STATE_HALT;
        LOG_ERROR "force halt; handhake %p %08x %08x -> %d\n",
            reg, mask, done, error LOG_END
    }

    return error;
}

/* reset a non-running (STS_HALT == 1) controller */
static int ehci_reset (struct ehci_hcd *ehci)
{
    int	retval;

    _hcd_func_enter;

    ithWriteRegMaskA(ehci->regs.command, CMD_RESET, CMD_RESET);
    ehci_to_hcd(ehci)->state = HC_STATE_HALT;

    retval = handshake (ehci, ehci->regs.command,
                CMD_RESET, 0, 250 * 1000);

    if (retval)
        LOG_ERROR " %s: rc = %d, ehci index %d \n", __func__, retval, ehci_to_hcd(ehci)->index LOG_END
		
	_hcd_func_leave;

    return retval;
}
  
/* idle the controller (from running) */
static void ehci_quiesce (struct ehci_hcd *ehci)
{ 
	u32	temp;

    _hcd_func_enter;

    if (!HC_IS_RUNNING (ehci_to_hcd(ehci)->state))
		BUG ();


    /* wait for any schedule enables/disables to take effect */
    temp = ithReadRegA(ehci->regs.command) << 10;
    temp &= STS_ASS | STS_PSS;
    if (handshake_on_error_set_halt(ehci, ehci->regs.status,
                STS_ASS | STS_PSS, temp, 16 * 125))
        return;

    /* then disable anything that's still active */
    temp = ithReadRegA(ehci->regs.command);
    temp &= ~(CMD_ASE | CMD_IAAD | CMD_PSE);
    ithWriteRegA(ehci->regs.command, temp);

    /* hardware can take 16 microframes to turn off ... */
    handshake_on_error_set_halt(ehci, ehci->regs.status,
                STS_ASS | STS_PSS, 0, 16 * 125);

    _hcd_func_leave;
}

int ehci_port_reset(struct ehci_hcd *ehci)
{
    int rc = 0;
    uint32_t val;

    /** run first */
    ithWriteRegMaskA(ehci->regs.command, CMD_RUN, CMD_RUN);

    /** port reset */
    ithWriteRegMaskA(ehci->regs.port_status[0], PORT_RESET, PORT_RESET);
    usleep(50*1000);
    ithWriteRegMaskA(ehci->regs.port_status[0], 0x0, PORT_RESET);

    rc = handshake (ehci, ehci->regs.port_status[0],
                PORT_RESET, 0, 1000);
    /* TRSTRCY = 10 ms; plus some extra */
    usleep(20*1000);
    if (rc)
        LOG_ERROR " %s: rc = %d, ehci index %d \n", __func__, rc, ehci_to_hcd(ehci)->index LOG_END

    return rc;
}


int ehci_get_speed(struct ehci_hcd *ehci)
{
    uint32_t value = 0;
    int speed = 0;

    value = ithReadRegA(ehci->otg_regs.ctrl_status);
    speed = HOST_SPEED(value);

    if(speed == FULL_SPEED)
    {
        LOG_DATA " Host Speed: Full Speed! \n" LOG_END
        return USB_SPEED_FULL;
    }
    else if(speed == LOW_SPEED)
    {
        LOG_DATA " Host Speed: Low Speed! \n" LOG_END
        return USB_SPEED_LOW;
    }
    else if(speed == HIGH_SPEED)
    {
        LOG_DATA " Host Speed: High Speed! \n" LOG_END
        return USB_SPEED_HIGH;
    }

    return USB_SPEED_UNKNOWN;
}


static void end_unlink_async(struct ehci_hcd *ehci);

#include "ehci_mem.c"
#include "ehci_q.c"
#include "ehci_sched.c"

/*
 * Called when the ehci_hcd module is removed.
 */
static void _ehci_stop (struct usb_hcd *hcd)
{
    int rc;
    struct ehci_hcd		*ehci = hcd_to_ehci(hcd);

    if(HC_IS_RUNNING (hcd->state))
        ehci_quiesce(ehci);

    rc = ehci_halt(ehci);
    if(rc)
        LOG_ERROR " %s: rc %d \n", __func__, rc LOG_END

    ehci_reset(ehci);
    if(rc)
        LOG_ERROR " %s: rc %d \n", __func__, rc LOG_END

    spin_lock_irq (&ehci->lock);
    if(ehci->async)
    {
        ithPrintf(" _ehci_stop: ehci_work() \n");
        if (ehci->reclaim)
            end_unlink_async(ehci);

        ehci_work (ehci);
    }
    spin_unlock_irq (&ehci->lock);
}

int ehci_start(struct usb_hcd *hcd, struct usb_device** usb_device)
{
    int rc = 0, c;
    struct ehci_hcd* ehci = hcd_to_ehci(hcd);
    struct usb_device* udev = NULL;
    uint32_t temp = 0;

    _hcd_func_enter;

    rc = ehci_halt(ehci);
    if(rc)
        goto end;

    //------- step 1: ehci_init (.reset) -------

    //ehci->i_thresh = 8;  // in ehci_start()
    ehci->reclaim = 0;
    ehci->next_uframe = -1;
    ehci_periodic_init(ehci);

    /*
     * dedicate a qh for the async ring head, since we couldn't unlink
     * a 'real' qh without stopping the async schedule [4.8].  use it
     * as the 'reclamation list head' too.
     * its dummy is used in hw_alt_next of many tds, to prevent the qh
     * from automatically advancing to the next td after short reads.
     */
    ehci->async->qh_next.qh = NULL;
    ehci->async->hw_next = QH_NEXT(ehci, ehci->async->qh_dma);
	ehci->async->hw_info1 = cpu_to_hc32(ehci, QH_HEAD);
	ehci->async->hw_token = cpu_to_hc32(ehci, QTD_STS_HALT);
    ehci->async->hw_qtd_next = EHCI_LIST_END(ehci);
    ehci->async->qh_state = QH_STATE_LINKED;
	ehci->async->hw_alt_next = QTD_NEXT(ehci, ehci->async->dummy->qtd_dma);

    /* clear interrupt enables, set irq latency */
    if (log2_irq_thresh < 0 || log2_irq_thresh > 6)
        log2_irq_thresh = 0;
    temp = 1 << (16 + log2_irq_thresh);
    if (HCC_CANPARK(ehci->hcc_params)) {
		/* HW default park == 3, on hardware that supports it (like
		 * NVidia and ALI silicon), maximizes throughput on the async
		 * schedule by avoiding QH fetches between transfers.
		 *
		 * With fast usb storage devices and NForce2, "park" seems to
		 * make problems:  throughput reduction (!), data errors...
		 */
        if (park) {
            park = min(park, (unsigned) 3);
            temp |= CMD_PARK;
            temp |= park << 8;
        }
		ehci_dbg(ehci, "park %d\n", park);
    }
    if (HCC_PGM_FRAMELISTLEN(ehci->hcc_params)) {
        /* periodic schedule size can be smaller than default */
        temp &= ~(3 << 2);
        temp |= (EHCI_TUNE_FLS << 2);
        switch (EHCI_TUNE_FLS) {
        case 0: ehci->periodic_size = 1024; break;
        case 1: ehci->periodic_size = 512; break;
        case 2: ehci->periodic_size = 256; break;
		default:	BUG();
        }
    }
    ehci->command = temp;


    //------- step 2: ehci_run (.start) -------

    /* EHCI spec section 4.1 */
    rc = ehci_reset(ehci);
    if(rc)
        goto end;

    ithWriteRegA(ehci->regs.frame_list, ehci->periodic_addr);
    ithWriteRegA(ehci->regs.async_next, (uint32_t)ehci->async->qh_dma);
    ithWriteRegA(ehci->regs.intr_enable, INTR_MASK);
    ithWriteRegA(ehci->regs.command, ehci->command);
	hcd->state = HC_STATE_RUNNING;


    //------- step 3: usb_new_device -------

    hcd->connect = 1;

    /** wire up the device */
    hcd->self.root_device = udev = usb_alloc_dev(NULL, &hcd->self);
    if(!udev)
    {
        rc = ERROR_USB_ALLOC_DEVICE_FAIL;
        goto end;
    }
    usb_set_device_state(udev, USB_STATE_POWERED);
    choose_devnum(udev);
    if(!udev->devnum)
    {
        rc = ERROR_USB_NOT_FIND_DEVCE_ID;
        goto end;
    }
    rc = port_init(ehci, udev);
    if(rc)
        goto end;

    usb_detect_quirks(udev);
	if (udev->quirks & USB_QUIRK_DELAY_INIT)
		msleep(1000);
    
    rc = usb_new_device(udev); /* enumeration */
    if(rc)
        goto end;

    udev->authorized = 1;
    c = usb_choose_configuration(udev);
    LOG_INFO " choose config:%d \n", c LOG_END

    rc = usb_set_configuration(udev, c);
    if(rc)
    {
        #if 1
        ehci_quiesce(ehci);
        usb_disconnect(&hcd->self.root_device);
		#endif
        rc = -ENODEV;
        goto end;
    }
    (*usb_device) = udev;

end:
    check_result(rc);
    _hcd_func_leave;
    return rc;
}


//=============================================================================
//                              Public Function Definition
//=============================================================================

static int ehci_init(struct usb_hcd *hcd)
{
    int result = 0;
    uint32_t i = 0, value;
    struct ehci_hcd* ehci = hcd_to_ehci(hcd);
    uint32_t* reg = NULL;

    /** generate register offset */
    reg = (uint32_t*)&ehci->caps;
    for(i=0; i<(sizeof(struct ehci_caps)/4); i++, reg++)
        (*reg) = hcd->iobase + (i*4);

    ehci->hc_caps = ithReadRegA(ehci->caps.hcc_reg);
    ehci->hcs_params = ithReadRegA(ehci->caps.hcs_params);
    ehci->hcc_params = ithReadRegA(ehci->caps.hcc_params);
    LOG_INFO " Reg 0x%08X = 0x%08X \n", ehci->caps.hcc_reg, ehci->hc_caps LOG_END
    LOG_INFO " Reg 0x%08X = 0x%08X \n", ehci->caps.hcs_params, ehci->hcs_params LOG_END
    LOG_INFO " Reg 0x%08X = 0x%08X \n", ehci->caps.hcc_params, ehci->hcc_params LOG_END
    reg = (uint32_t*)&ehci->regs;
    for(i=0; i<(sizeof(struct ehci_regs)/4); i++, reg++)
        (*reg) = hcd->iobase + (ehci->hc_caps & 0xFF) + (i*4);
    reg = (uint32_t*)&ehci->otg_regs;
    for(i=0; i<(sizeof(struct otg_regs)/4); i++, reg++)
        (*reg) = hcd->iobase + 0x80 + (i*4);
    ehci->otg_regs.curr_role = 0;
    reg = (uint32_t*)&ehci->global_regs;
    for(i=0; i<(sizeof(struct global_regs)/4); i++, reg++)
        (*reg) = hcd->iobase + 0xC0 + (i*4);

	spin_lock_init(&ehci->lock);

    result = ehci_halt(ehci);
    if(result)
        goto end;

    /* for host mode setting */
    value = ithReadRegA(ehci->otg_regs.ctrl_status);
    value &= ~A_DEVICE_BUS_DROP;
    value |= A_DEVICE_BUS_REQUEST;
    ithWriteRegA(ehci->otg_regs.ctrl_status, value);

    /** memory init */
    /**
     * hw default: 1K periodic list heads, one per frame.
     * periodic_size can shrink by USBCMD update if hcc_params allows.
     */
    ehci->periodic_size = DEFAULT_I_TDPS;
    result = ehci_mem_init(ehci);
    if(result)
        goto end;

    /* controllers may cache some of the periodic schedule ... */
    if (HCC_ISOC_CACHE(ehci->hcc_params))		// full frame cache
        ehci->i_thresh = 8;
    else					// N microframes cached
        ehci->i_thresh = 2 + HCC_ISOC_THRES(ehci->hcc_params);

end:
    check_result(result);

    return result;
}


static int ehci_stop(struct usb_hcd *hcd)
{
    int type = 0;

    _hcd_func_enter;

    if(HC_IS_RUNNING (hcd->state))
        _ehci_stop(hcd);

    if(hcd->self.root_device)
    {
        type = hcd->self.root_device->device_info[1].type;
        usb_disconnect(&hcd->self.root_device);
    }

    hcd->connect = 0;
    _hcd_func_leave;

    return type;
}

/*-------------------------------------------------------------------------*/

/*
 * non-error returns are a promise to giveback() the urb later
 * we drop ownership so next owner (or urb unlink) can get it
 *
 * urb + dev is in hcd.self.controller.urb_list
 * we're queueing TDs onto software and hardware lists
 *
 * hcd-specific init for hcpriv hasn't been done yet
 *
 * NOTE:  control, bulk, and interrupt share the same code to append TDs
 * to a (possibly active) QH, and the same QH scanning code.
 */
static int ehci_urb_enqueue (
	struct usb_hcd	*hcd,
	struct urb	*urb,
	gfp_t		mem_flags
) {
    int status=0;
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
    struct list_head	qtd_list;

    _hcd_func_enter;

	INIT_LIST_HEAD (&qtd_list);

    switch (usb_pipetype (urb->pipe)) {
    case PIPE_CONTROL:
        /* qh_completions() code doesn't handle all the fault cases
         * in multi-TD control transfers.  Even 1KB is rare anyway.
         */
        if (urb->transfer_buffer_length > (16 * 1024))
        {
            status = -EMSGSIZE;
            goto end;
        }
        /* FALLTHROUGH */
    //case PIPE_BULK:
    default:
        if(!qh_urb_transaction(ehci, urb, &qtd_list, mem_flags))
        {
            status = -ENOMEM;
            goto end;
        }
        status = submit_async(ehci, urb, &qtd_list, mem_flags);
        break;
    case PIPE_INTERRUPT:
        if(!qh_urb_transaction(ehci, urb, &qtd_list, mem_flags))
        {
            status = -ENOMEM;
            goto end;
        }
        status = intr_submit(ehci, urb, &qtd_list, mem_flags);
        break;
    case PIPE_ISOCHRONOUS:
		if (urb->dev->speed == USB_SPEED_HIGH)
		{
			status = itd_submit (ehci, urb, mem_flags);
		}
		else
		{
			status = sitd_submit (ehci, urb, mem_flags);
		}
        break;
    }

end:
    check_result(status);
    _hcd_func_leave;
    return status;
}

static void unlink_async (struct ehci_hcd *ehci, struct ehci_qh *qh)
{
    _hcd_func_enter;

    /* failfast */
    if (!HC_IS_RUNNING(ehci_to_hcd(ehci)->state) && ehci->reclaim)
        end_unlink_async(ehci);

    /* if it's not linked then there's nothing to do */
    if (qh->qh_state != QH_STATE_LINKED)
        ;

    /* defer till later if busy */
	else if (ehci->reclaim) {
        struct ehci_qh		*last;

        for (last = ehci->reclaim;
                last->reclaim;
                last = last->reclaim)
            continue;
        qh->qh_state = QH_STATE_UNLINK_WAIT;
        last->reclaim = qh;

    /* start IAA cycle */
	} else
        start_unlink_async (ehci, qh);

    _hcd_func_leave;
}

/* remove from hardware lists
 * completions normally happen asynchronously
 */

static int ehci_urb_dequeue(struct usb_hcd *hcd, struct urb *urb, int status)
{
    struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
            struct ehci_qh	*qh;
	unsigned long		flags;
    int			rc;

	spin_lock_irqsave (&ehci->lock, flags);
    LOG_DEBUG " %s: urb %p, status %d \n", __func__, urb, status LOG_END

    rc = usb_hcd_check_unlink_urb(hcd, urb, status);
    if (rc)
        goto done;

	switch (usb_pipetype (urb->pipe)) {
    // case PIPE_CONTROL:
    // case PIPE_BULK:
    default:
        qh = (struct ehci_qh *) urb->hcpriv;
        if (!qh)
            break;
		LOG_DEBUG " %s: qh %p, state %d \n", __func__, qh, qh->qh_state LOG_END
        switch (qh->qh_state) {
        case QH_STATE_LINKED:
        case QH_STATE_COMPLETING:
            unlink_async(ehci, qh);
            break;
        case QH_STATE_UNLINK:
        case QH_STATE_UNLINK_WAIT:
            /* already started */
            break;
        case QH_STATE_IDLE:
            LOG_WARNING " %s: unlink idle qh?? urb %p \n", __func__, urb LOG_END
            break;
            }
        break;
 
    case PIPE_INTERRUPT:
        qh = (struct ehci_qh *) urb->hcpriv;
        if (!qh)
            break;
        switch (qh->qh_state) {
        case QH_STATE_LINKED:
			intr_deschedule (ehci, qh);
            /* FALL THROUGH */
        case QH_STATE_IDLE:
            qh_completions (ehci, qh);
            break;
        default:
			ehci_dbg (ehci, "bogus qh %p state %d\n",
					qh, qh->qh_state);
            goto done;
        }

        /* reschedule QH iff another request is queued */
        if (!list_empty (&qh->qtd_list)
                && HC_IS_RUNNING (hcd->state)) {
            rc = qh_schedule(ehci, qh);

            /* An error here likely indicates handshake failure
             * or no space left in the schedule.  Neither fault
             * should happen often ...
             *
             * FIXME kill the now-dysfunctional queued urbs
             */
            if (rc != 0)
                ehci_err(ehci,
                    "can't reschedule qh %p, err %d",
                    qh, rc);
            }
        break;

    case PIPE_ISOCHRONOUS:
        // itd or sitd ...

        // wait till next completion, do it then.
        // completion irqs can wait up to 1024 msec,
        break;
    }
done:
	spin_unlock_irqrestore (&ehci->lock, flags);
    return rc;
}

/*-------------------------------------------------------------------------*/

// bulk qh holds the data toggle

static void
ehci_endpoint_disable (struct usb_hcd *hcd, struct usb_host_endpoint *ep)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	unsigned long		flags;
	struct ehci_qh		*qh, *tmp;

    _hcd_func_enter;
	/* ASSERT:  any requests/urbs are being unlinked */
	/* ASSERT:  nobody can be submitting urbs for this any more */

rescan:
	spin_lock_irqsave (&ehci->lock, flags);
	qh = ep->hcpriv;
	if (!qh)
		goto done;

    ehci_invalid_qh(qh);
	/* endpoints can be iso streams.  for now, we don't
	 * accelerate iso completions ... so spin a while.
	 */
	if (qh->hw_info1 == 0) {
		ehci_vdbg (ehci, "iso delay\n");
		goto idle_timeout;
	}

	if (!HC_IS_RUNNING (hcd->state)) 
		qh->qh_state = QH_STATE_IDLE;
	
	switch (qh->qh_state) {
	case QH_STATE_LINKED:
		for (tmp = ehci->async->qh_next.qh;
				tmp && tmp != qh;
				tmp = tmp->qh_next.qh)
			continue;
		/* periodic qh self-unlinks on empty */
		if (!tmp)
			goto nogood;
		unlink_async (ehci, qh);
		/* FALL THROUGH */
	case QH_STATE_UNLINK:		/* wait for hw to finish? */
	case QH_STATE_UNLINK_WAIT:
idle_timeout:
		spin_unlock_irqrestore (&ehci->lock, flags);
		schedule_timeout_uninterruptible(1);
		goto rescan;
	case QH_STATE_IDLE:		/* fully unlinked */
		if (list_empty (&qh->qtd_list)) {
			qh_put (qh);
			break;
		}
		/* else FALL THROUGH */
	default:
nogood:
		/* caller was supposed to have unlinked any requests;
		 * that's not our job.  just leak this memory.
		 */
		ehci_err (ehci, "qh %p (#%02x) state %d%s\n",
			qh, ep->desc.bEndpointAddress, qh->qh_state,
			list_empty (&qh->qtd_list) ? "" : "(has tds)");
		break;
	}
	ep->hcpriv = NULL;
done:
	spin_unlock_irqrestore (&ehci->lock, flags);
    _hcd_func_leave;
	return;
}

static int ehci_get_frame (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
    uint32_t reg;

    reg = ithReadRegA(ehci->regs.frame_index);
    return (reg >> 3) % ehci->periodic_size;
}

static int ehci_dev_exist(struct usb_hcd *hcd)
{
    return _ehci_dev_exist(hcd_to_ehci(hcd));
}

static struct hc_driver ehci_driver = 
{   
    sizeof(struct ehci_hcd),
    HCD_USB2,
    ehci_init,
    ehci_stop,
    ehci_get_frame,
    ehci_urb_enqueue,
    ehci_urb_dequeue,
    ehci_endpoint_disable,
    NULL,  // ehci_endpoint_reset
    NULL,  // alloc_streams
    NULL,  // free_streams
    ehci_dev_exist,
};

extern int usb_hcd_probe(struct hc_driver* driver, uint32_t index);

int ehci_hcd_init(uint32_t index)
{
    return usb_hcd_probe(&ehci_driver, index);
}

/*-------------------------------------------------------------------------*/

/*
 * ehci_work is called from some interrupts, timers, and so on.
 * it calls driver completion functions, after dropping ehci->lock.
 */
void ehci_work (struct ehci_hcd *ehci)
{
    #if defined(ISR_TO_TASK)
    ehci->tasklet--;
    #endif

    /* another CPU may drop ehci->lock during a schedule scan while
     * it reports urb completions.  this flag guards against bogus
     * attempts at re-entrant schedule scanning.
     */
    if (ehci->scanning)
    {
        ithPrintf(" ehci_work() return \n");
        return;
    }
    ehci->scanning = 1;
    scan_async (ehci);
    if (ehci->next_uframe != -1) {	
        scan_periodic (ehci);
    }
    ehci->scanning = 0;
}

/*-------------------------------------------------------------------------*/

extern sem_t   isr_event;

void ehci_irq(struct usb_hcd* hcd)
{
    struct ehci_hcd* ehci = hcd_to_ehci(hcd);
    uint32_t status = 0, cmd, masked_status;
    int   bh = 0;

    status = ithReadRegA(ehci->regs.status);

    /* e.g. cardbus physical eject */
    if (status == ~(uint32_t) 0) 
    {
        LOG_ERROR " %s: device removed\n", __func__ LOG_END
                hcd->state = HC_STATE_HALT;
        goto dead;
    }

    masked_status = status & INTR_MASK;
    if(!masked_status)
        return;

    ithWriteRegA(ehci->regs.status, status);

    /* normal [4.15.1.2] or error [4.15.1.1] completion */
    if(likely((status & (STS_INT|STS_ERR)) != 0))
        bh = 1;

    /* complete the unlinking of some qh [4.15.2.3] */
    if(status & STS_IAA) 
    {
        if(ehci->reclaim)
            end_unlink_async(ehci);
        else
            LOG_ERROR " IAA with nothing to reclaim? \n" LOG_END
    }

    /* Bus errors [4.15.2.4] */
    if(unlikely((status & STS_FATAL) != 0)) 
    {
        ithPrintf("[USB] hcd index %08X: fatal error, state %x  \n", hcd->index, hcd->state);
        ehci_halt(ehci);
dead:
        ehci_reset(ehci);
        // Irene TODO !!!!! ??
        // generic layer kills/unlinks all urbs
        // then tasklet cleans up the rest
        bh = 1;
    }

    if(likely(bh == 1))
    {
#if (CFG_CHIP_FAMILY == 9070)
        ithFlushAhbWrap();
#endif
        #if defined(ISR_TO_TASK)
        to_ehci_work(ehci);
        itpSemPostFromISR(&isr_event);
        #else
        ehci_work(ehci);
        #endif
    }

    return;
}


