
/*-------------------------------------------------------------------------*/

static int halt_bulk_in_endpoint(struct fsg_dev *fsg)
{
    int rc=0;

    FSG_DBG " bulk-in set halt \n" FSG_END
    rc = fsg_set_halt(fsg, fsg->bulk_in);
    if(rc == -EAGAIN)
        FSG_DBG " delayed bulk-in endpoint halt \n" FSG_END

    while(rc != 0)
    {
        if(rc != -EAGAIN)
        {
            FSG_WARNING " bulk-in set halt => 0x%X \n", rc FSG_END
            rc = 0;
            break;
        }
        /* Wait for a short time and then try again */
        usleep(200*1000);
        rc = fsg_set_halt(fsg, fsg->bulk_in);
    }

    return rc;
}

static int wedge_bulk_in_endpoint(struct fsg_dev *fsg)
{
    int rc=0;

    FSG_DBG " bulk-in set wedge \n" FSG_END
    rc = usb_ep_set_wedge(fsg->bulk_in);
    if(rc == -EAGAIN)
        FSG_DBG " delayed bulk-in endpoint wedge \n" FSG_END

    while(rc != 0)
    {
        if(rc != -EAGAIN)
        {
            FSG_WARNING " bulk-in set wedge => 0x%X \n", rc FSG_END
            rc = 0;
            break;
        }
        /* Wait for a short time and then try again */
        usleep(200*1000);
        rc = usb_ep_set_wedge(fsg->bulk_in);
    }

    return rc;
}

/*-------------------------------------------------------------------------*/

/* Use this for bulk or interrupt transfers, not ep0 */
static void start_transfer(struct fsg_dev *fsg, 
                        struct usb_ep *ep, 
                        struct usb_request *req,
                        int *pbusy,
                        enum fsg_buffer_state *state)
{
    int rc=0;

#if defined(DUMP_DATA)
    if(ep == fsg->bulk_in)
    //if((ep == fsg->bulk_in) && (req->length == 13)) // test
    {
        FSG_DATA " bulk-in: \n" FSG_END
        ithPrintVram8((uint32_t)req->buf, req->length);
    }
#endif

    _spin_lock_irq(&fsg->lock);
    *pbusy = 1;
    *state = BUF_STATE_BUSY;
    _spin_unlock_irq(&fsg->lock);

    rc = usb_ep_queue(ep, req);
    if(rc != 0)
    {
        *pbusy = 0;
        *state = BUF_STATE_EMPTY;

        if(rc != -ESHUTDOWN)
            FSG_ERROR " error in submission: %s req %X => rc=0x%X \n", ep->name, req, rc FSG_END
    }
}

#include "scsi.c"

/*-------------------------------------------------------------------------*/

static int send_status(struct fsg_dev *fsg)
{
    struct lun *curlun = fsg->curlun;
    struct fsg_buffhd *bh;
    int rc=0;
    uint32_t sd=0, sdinfo=0;
    uint8_t status = USB_STATUS_PASS;
    struct bulk_cs_wrap *csw;
    int count=2000;

    /* Wait for the next buffer to become available */
    bh = fsg->next_buffhd_to_fill;
    while(bh->state != BUF_STATE_EMPTY)
    {
        FSG_DBG " wait buffer for csw...\n" FSG_END
        if(exception_in_progress(fsg))
        {
            rc = ERR_FSG_EXCEPTION;
            goto end;
        }
        usleep(1000);
        if(--count == 0)
        {
            rc = ERR_FSG_CSW_NO_BUF;
            goto end;
        }
    }

    if(curlun)
    {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
    }
    else if(fsg->bad_lun_okay)
        sd = SS_NO_SENSE;
    else
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;

    if(fsg->phase_error)
    {
        FSG_INFO " sending phase-error status \n" FSG_END
        status = USB_STATUS_PHASE_ERROR;
        sd = SS_INVALID_COMMAND;
    }
    else if(sd != SS_NO_SENSE)
    {
        FSG_INFO " sending command-failure status \n" FSG_END
        status = USB_STATUS_FAIL;
        FSG_INFO " sense data: SK x%02x, ASC x%02x, ASCQ x%02x; info x%x\n", 
            SK(sd), ASC(sd), ASCQ(sd), sdinfo FSG_END
    }

    csw = (struct bulk_cs_wrap*)bh->buf;
    csw->Signature = cpu_to_le32(USB_BULK_CS_SIG);
    csw->Tag = fsg->tag;
    csw->Residue = cpu_to_le32(fsg->residue);
    csw->Status = status;

    bh->inreq->length = USB_BULK_CS_WRAP_LEN;
    bh->inreq->zero = 0;
    start_transfer(fsg, fsg->bulk_in, bh->inreq, &bh->inreq_busy, &bh->state);
    fsg->next_buffhd_to_fill = bh->next;

    return 0;

end:
    check_result(rc);
    return rc;
}
                        
/*-------------------------------------------------------------------------*/
static int throw_away_data(struct fsg_dev *fsg)
{
    struct fsg_buffhd *bh;
    uint32_t amount;

    while(((bh = fsg->next_buffhd_to_drain)->state != BUF_STATE_EMPTY) ||
            (fsg->usb_amount_left > 0))
    {
        /* Throw away the data in a filled buffer */
        if(bh->state == BUF_STATE_FULL)
        {
            FSG_WARNING "throw_away_data: Throw away the data in a filled buffer!!!! \n" FSG_END
            bh->state = BUF_STATE_EMPTY;
            fsg->next_buffhd_to_drain = bh->next;

            /* A short packet or an error ends everything */
            if((bh->outreq->actual != bh->outreq->length) || (bh->outreq->status != 0)) 
            {
                FSG_ERROR " throw_away_data: %d != %d, status(%d) \n", bh->outreq->actual, bh->outreq->length, bh->outreq->status FSG_END
                raise_exception(fsg, FSG_STATE_ABORT_BULK_OUT);
                return -EINTR;
            }
        }

        /* Try to submit another request if we need one */
        bh = fsg->next_buffhd_to_fill;
        if((bh->state == BUF_STATE_EMPTY) && (fsg->usb_amount_left > 0))
        {
            FSG_WARNING " throw_away_data: fsg->usb_amount_left = %d \n", fsg->usb_amount_left FSG_END
            amount = min(fsg->usb_amount_left, FSG_BUFLEN);
            bh->outreq->length = amount;
            start_transfer(fsg, fsg->bulk_out, bh->outreq, &bh->outreq_busy, &bh->state);
            fsg->next_buffhd_to_fill = bh->next;
            fsg->usb_amount_left -= amount;
        }
        usleep(1000);
    }
    
    return 0;
}

static int finish_reply(struct fsg_dev *fsg)
{
    struct fsg_buffhd *bh = fsg->next_buffhd_to_fill;
    int rc=0;

    switch(fsg->data_dir)
    {
    case DATA_DIR_NONE:
        /* Nothing to send */
        break; 

    case DATA_DIR_UNKNOWN:
        // TODO
        FSG_ERROR " finish_reply: DATA_DIR_UNKNOWN => TODO !!! \n" FSG_END
        break;

    case DATA_DIR_TO_HOST:
        if(fsg->data_size == 0)
        { 
            /* Nothing to send */
        }
        /* If there's no residue, simply send the last buffer */
        else if(fsg->residue == 0)
        {
            bh->inreq->zero = 0;
            start_transfer(fsg, fsg->bulk_in, bh->inreq, &bh->inreq_busy, &bh->state);
            fsg->next_buffhd_to_fill = bh->next;
        }
        /* For Bulk-only, if we're allowed to stall then send the
         * short packet and halt the bulk-in endpoint.  If we can't
         * stall, pad out the remaining data with 0's. */
        else
        {
            FSG_WARNING " finish_reply: DATA_IN fsg->residue=0x%X, send short packet and halt bulk-in !!! \n", fsg->residue FSG_END
            bh->inreq->zero = 1;
            start_transfer(fsg, fsg->bulk_in, bh->inreq, &bh->inreq_busy, &bh->state);
            fsg->next_buffhd_to_fill = bh->next;
            rc = halt_bulk_in_endpoint(fsg);
        }
        break;

    case DATA_DIR_FROM_HOST:
        if(fsg->residue == 0)
        {
            /* Nothing to receive */
        }
        /* Did the host stop sending unexpectedly early? */
        else if(fsg->short_packet_received)
        {
            FSG_INFO " finish_reply: short packet! \n" FSG_END
            raise_exception(fsg, FSG_STATE_ABORT_BULK_OUT);
            rc = -EINTR;
        }
        /* We can't stall.  Read in the excess data and throw it all away. */
        else
        {
            FSG_INFO " finish_reply: throw away data! \n" FSG_END
            rc = throw_away_data(fsg);
        }
        break;
    }

    if(rc) 
        FSG_ERROR "[%s] res = 0x%08X(%d) lun:%d, cmd:0x%X \n", __FUNCTION__, rc, rc, fsg->lun, fsg->cmnd[0] FSG_END

    return rc;
}


/*-------------------------------------------------------------------------*/

/* Check whether the command is properly formed and whether its data size
 * and direction agree with the values we already have. */
static int check_command(struct fsg_dev *fsg,
                         int cmnd_size,
                         enum data_direction data_dir,
                         uint32_t mask,
                         int needs_medium,
                         const char *name)
{
    int lun = fsg->cmnd[1] >> 5;
    struct lun *curlun;
    int i, rc = 0;

    FSG_INFO "(%d) SCSI cmd: %s (cmd_size:%d), data size(cmd):%d, cmd size(CBW):%d, data size(CBW):%d \n", fsg->lun,
        name, cmnd_size, fsg->data_size_from_cmnd, fsg->cmnd_size, fsg->data_size FSG_END

    /* We can't reply at all until we know the correct data direction
     * and size. */
    if(fsg->data_size_from_cmnd == 0)
        data_dir = DATA_DIR_NONE;

    /* Host data size < Device data size is a phase error.
     * Carry out the command, but only transfer as much
     * as we are allowed. */
    // see case (2)(3)(7)(8)(10)(13)
    if(fsg->data_size < fsg->data_size_from_cmnd)
    {
        fsg->data_size_from_cmnd = fsg->data_size;
        fsg->phase_error = 1;
    }
    fsg->residue = fsg->usb_amount_left = fsg->data_size;

    /* Conflicting data directions is a phase error */
    if((fsg->data_dir != data_dir) && (fsg->data_size_from_cmnd > 0))
    {
        fsg->phase_error = 1;
        rc = -ERR_FSG_DIR_CONFLICT;
        goto end;
    }

    /* Verify the length of the command itself */
    if(cmnd_size != fsg->cmnd_size)
    {
        /* Special case workaround: There are plenty of buggy SCSI
         * implementations. Many have issues with cbw->Length
         * field passing a wrong command size. For those cases we
         * always try to work around the problem by using the length
         * sent by the host side provided it is at least as large
         * as the correct command length.
         */
        if(cmnd_size <= fsg->cmnd_size)
        {
            FSG_WARNING " %s: expected cmd length %d, but we got %d \n", name, cmnd_size, fsg->cmnd_size FSG_END
            cmnd_size = fsg->cmnd_size;
        }
        else
        {
            fsg->phase_error = 1;
            rc = -ERR_FSG_CMD_SIZE_CONFLICT;
            goto end;
        }
    }

#if 0
    /* Check that the LUN values are consistent */
    if(fsg->lun != lun)
        FSG_WARNING " using LUN %d from CBW, not LUN %d from CDB \n", fsg->lun, lun FSG_END
#endif

    /* Check the LUN */
    if((fsg->lun >=0) && (fsg->lun < fsg->cfg->nluns))
    {
        fsg->curlun = curlun = &fsg->luns[fsg->lun];
        if(fsg->cmnd[0] != SC_REQUEST_SENSE)
        {
            curlun->sense_data = SS_NO_SENSE;
            curlun->sense_data_info = 0;
            curlun->info_valid = 0;
        }
    }
    else
    {
        fsg->curlun = curlun = NULL;
        fsg->bad_lun_okay = 0;

        /* INQUIRY and REQUEST SENSE commands are explicitly allowed
         * to use unsupported LUNs; all others may not. */
        if((fsg->cmnd[0] != SC_INQUIRY) && (fsg->cmnd[0] != SC_REQUEST_SENSE))
        {
            FSG_ERROR " unsupported LUN %d\n", fsg->lun FSG_END
            rc = -ERR_FSG_LUN_INVALID;
            goto end;
        }
    }

    /* If a unit attention condition exists, only INQUIRY and
     * REQUEST SENSE commands are allowed; anything else must fail. */
    if(curlun && (curlun->unit_attention_data != SS_NO_SENSE) &&
        (fsg->cmnd[0] != SC_INQUIRY) && (fsg->cmnd[0] != SC_REQUEST_SENSE))
    {
        curlun->sense_data = curlun->unit_attention_data;
        curlun->unit_attention_data = SS_NO_SENSE;
        rc = -ERR_FSG_UNIT_ATTENTION_DATA;
        goto end;
    }

    /* Check that only command bytes listed in the mask are non-zero */
    fsg->cmnd[1] &= 0x1f;  // Mask away the LUN
    for(i=1; i<cmnd_size; i++)
    {
        if(fsg->cmnd[i] && !(mask & (1 << i)))
        {
            if(curlun)
                curlun->sense_data = SS_INVALID_FIELD_IN_CDB;

            rc = -ERR_FSG_RESERVED_BITS_INVALID;
            #if 0
            ithPrintVram8((uint32_t)fsg->cmnd, cmnd_size);
            ithPrintf(" fail index: %d (lun:%d) \n", i, fsg->lun);
            #endif
            goto end;
        }
    }

    /* If the medium isn't mounted and the command needs to access
     * it, return an error. */
    if(curlun && needs_medium)
    {
        rc = fsg->cfg->ops->response((uint8_t)fsg->lun);
        if(rc != FSG_MEDIA_READY)
        {
            if(rc == FSG_MEDIA_NOT_PRESENT)
                curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
            else if(rc == FSG_MEDIA_CHANGE)
                curlun->sense_data = SS_NOT_READY_TO_READY_TRANSITION;

            rc = -ERR_FSG_MEDIA_NOT_READY;
            goto end;
        }
        rc = 0;
    }

end:
    if(rc) 
        FSG_ERROR "[%s] res = 0x%08X(%d) lun:%d, cmd:0x%X \n", __FUNCTION__, rc, rc, fsg->lun, fsg->cmnd[0] FSG_END

    if(rc!=0)
        rc = -EINVAL;

    return rc;
}

static int do_scsi_command(struct fsg_dev *fsg)
{
#define NEED_MEDIUM		1
#define NO_NEED_MEDIUM	0

    struct fsg_buffhd	*bh;
    int		reply = -EINVAL;
    int		count = 2000;
    int		rc = 0, i;

    bh = fsg->next_buffhd_to_drain = fsg->next_buffhd_to_fill;
    while(bh->state != BUF_STATE_EMPTY)
    {
        FSG_DBG " wait buffer for scsi command...\n" FSG_END
        if(exception_in_progress(fsg))
        {
            rc = ERR_FSG_EXCEPTION;
            goto end;
        }
        usleep(1000);
        if(--count == 0)
        {
            rc = ERR_FSG_SCSI_NO_BUF;
            goto end;
        }
    }
    fsg->phase_error = 0;
    fsg->short_packet_received = 0;
    
    switch(fsg->cmnd[0])
    {
    case SC_INQUIRY:
        fsg->data_size_from_cmnd = fsg->cmnd[4];
        if((reply = check_command(fsg, 6, DATA_DIR_TO_HOST, (1<<4), NO_NEED_MEDIUM, "INQUIRY")) == 0)
            reply = do_inquiry(fsg, bh);
        break;

    case SC_MODE_SELECT_6:
        fsg->data_size_from_cmnd = fsg->cmnd[4];
        if((reply = check_command(fsg, 6, DATA_DIR_FROM_HOST, (1<<1) | (1<<4), NO_NEED_MEDIUM, "MODE SELECT(6)")) == 0)
            reply = do_mode_select(fsg, bh);
        break;

    case SC_MODE_SELECT_10:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if((reply = check_command(fsg, 10, DATA_DIR_FROM_HOST, (1<<1) | (3<<7), NO_NEED_MEDIUM, "MODE SELECT(10)")) == 0)
            reply = do_mode_select(fsg, bh);
        break;

    case SC_MODE_SENSE_6:
        fsg->data_size_from_cmnd = fsg->cmnd[4];
        if((reply = check_command(fsg, 6, DATA_DIR_TO_HOST, (1<<1) | (1<<2) | (1<<4), NEED_MEDIUM, "MODE SENSE(6)")) == 0)
            reply = do_mode_sense(fsg, bh);
        break;

    case SC_MODE_SENSE_10:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if((reply = check_command(fsg, 6, DATA_DIR_TO_HOST, (1<<1) | (1<<2) | (3<<7), NEED_MEDIUM, "MODE SENSE(10)")) == 0)
            reply = do_mode_sense(fsg, bh);
        break;

    case SC_PREVENT_ALLOW_MEDIUM_REMOVAL:
        fsg->data_size_from_cmnd = 0;
        if((reply = check_command(fsg, 6, DATA_DIR_NONE, (1<<4), NO_NEED_MEDIUM, "PREVENT-ALLOW MEDIUM REMOVAL")) == 0)
            reply = do_prevent_allow(fsg);
        break;

    case SC_READ_6:
        i = fsg->cmnd[4];
        fsg->data_size_from_cmnd = (i == 0 ? 256 : i) << 9;
        if((reply = check_command(fsg, 6, DATA_DIR_TO_HOST, (0x7<<1) | (1<<4), NEED_MEDIUM, "READ(6)")) == 0)
            reply = do_read(fsg);
        break;

    case SC_READ_10:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]) << 9;
        if((reply = check_command(fsg, 10, DATA_DIR_TO_HOST, (0x1<<1) | (0xF<<2) | (0x3<<7), NEED_MEDIUM, "READ(10)")) == 0)
            reply = do_read(fsg);
        break;

    case SC_READ_12:
        fsg->data_size_from_cmnd = get_be32(&fsg->cmnd[6]) << 9;
        if((reply = check_command(fsg, 12, DATA_DIR_TO_HOST, (0x1<<1) | (0xF<<2) | (0xF<<6), NEED_MEDIUM, "READ(12)")) == 0)
            reply = do_read(fsg);
        break;

    case SC_READ_CAPACITY:
        fsg->data_size_from_cmnd = 8;
        if((reply = check_command(fsg, 10, DATA_DIR_TO_HOST, (0xF<<2) | (1<<8), NEED_MEDIUM, "READ CAPACITY")) == 0)
            reply = do_read_capacities(fsg, bh);
        break;

    case SC_READ_FORMAT_CAPACITIES:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if((reply = check_command(fsg, 10, DATA_DIR_TO_HOST, (3<<7), NEED_MEDIUM, "READ FORMAT CAPACITIES")) == 0)
            reply = do_read_format_capacities(fsg, bh);
        break;

    case SC_REQUEST_SENSE:
        fsg->data_size_from_cmnd = fsg->cmnd[4];
        if((reply = check_command(fsg, 6, DATA_DIR_TO_HOST, (1<<4), NO_NEED_MEDIUM, "REQUEST SENSE")) == 0)
            reply = do_request_sense(fsg, bh);
        break;

    case SC_START_STOP_UNIT:
        fsg->data_size_from_cmnd = 0;
        if((reply = check_command(fsg, 6, DATA_DIR_NONE, (1<<1) | (1<<4), NO_NEED_MEDIUM, "START-STOP UNIT")) == 0)
            reply = do_start_stop(fsg);
        break;

    case SC_TEST_UNIT_READY:
        fsg->data_size_from_cmnd = 0;
        reply = check_command(fsg, 6, DATA_DIR_NONE, 0, NEED_MEDIUM, "TEST UNIT READY");
        break;

    /*
     * Although optional, this command is used by MS-Windows.  We
     * support a minimal version: BytChk must be 0.
     */
    case SC_VERIFY:
        fsg->data_size_from_cmnd = 0;
        if((reply = check_command(fsg, 10, DATA_DIR_NONE, (0x1<<1) | (0xF<<2) | (3<<7), NEED_MEDIUM, "VERIFY")) == 0)
            reply = do_verify(fsg);
        break;

    case SC_WRITE_6:
        i = fsg->cmnd[4];
        fsg->data_size_from_cmnd = (i == 0 ? 256 : i) << 9;
        if((reply = check_command(fsg, 6, DATA_DIR_FROM_HOST, (0x7<<1) | (1<<4), NEED_MEDIUM, "WRITE(6)")) == 0)
            reply = do_write(fsg);
        break;

    case SC_WRITE_10:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]) << 9;
        if((reply = check_command(fsg, 10, DATA_DIR_FROM_HOST, (0x1<<1) | (0xF<<2) | (0x3<<7), NEED_MEDIUM, "WRITE(10)")) == 0)
            reply = do_write(fsg);
        break;

    case SC_WRITE_12:
        fsg->data_size_from_cmnd = get_be32(&fsg->cmnd[6]) << 9;
        if((reply = check_command(fsg, 12, DATA_DIR_FROM_HOST, (0x1<<1) | (0xF<<2) | (0xF<<6), NEED_MEDIUM, "WRITE(12)")) == 0)
            reply = do_write(fsg);
        break;


    case SC_SYNCHRONIZE_CACHE:
    default:
        fsg->data_size_from_cmnd = 0;
        FSG_ERROR " Unknown SCSI cmd: x%02X \n", fsg->cmnd[0] FSG_END
        if((reply = check_command(fsg, fsg->cmnd_size, DATA_DIR_UNKNOWN, 0xFF, NO_NEED_MEDIUM, "Unknown")) == 0)
        {
            fsg->curlun->sense_data = SS_INVALID_COMMAND;
            reply = -EINVAL;
        }
        break;
    }

    /* Set up the single reply buffer for finish_reply() */
    if(reply == -EINVAL)
        reply = 0;		/* Error reply length */

    if((reply >= 0) && (fsg->data_dir == DATA_DIR_TO_HOST))
    {
        reply = min((uint32_t)reply, fsg->data_size_from_cmnd);
        bh->inreq->length = reply;
        bh->state = BUF_STATE_FULL;
        fsg->residue -= reply;
    } /* Otherwise it's already set */

end:
    check_result(rc);
    return rc;
}

/*-------------------------------------------------------------------------*/

static int received_cbw(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
    int rc = 0;
    struct usb_request *req = bh->outreq;
    struct bulk_cb_wrap *cbw = (struct bulk_cb_wrap *)req->buf;

    /* Was this a real packet?  Should it be ignored? */
    if(req->status || fsg->ignore_bulk_out)
    {
        FSG_ERROR " received_cbw: request status 0x%X(%d), ignore:%d \n", req->status, req->status, fsg->ignore_bulk_out FSG_END
        rc = ERR_FSG_CBW_INVALID;
        goto end;
    }

    /* Is the CBW valid? */
    if((req->actual != USB_BULK_CB_WRAP_LEN) || 
       (cbw->Signature != cpu_to_le32(USB_BULK_CB_SIG)))
    {
        FSG_ERROR " received_cbw => invalid CBW: len %d, sig: 0x%X \n", req->actual, le32_to_cpu(cbw->Signature) FSG_END

        /*
         * The Bulk-only spec says we MUST stall the IN endpoint
         * (6.6.1), so it's unavoidable.  It also says we must
         * retain this state until the next reset, but there's
         * no way to tell the controller driver it should ignore
         * Clear-Feature(HALT) requests.
         *
         * We aren't required to halt the OUT endpoint; instead
         * we can simply accept and discard any data received
         * until the next reset.
         */
        wedge_bulk_in_endpoint(fsg);
        fsg->ignore_bulk_out = 1;
        rc = ERR_FSG_CBW_INVALID;
        goto end;
    }

    /* Is the CBW meaningful? */
    if((cbw->Lun >= FSG_MAX_LUNS) ||
       (cbw->Flags & ~USB_BULK_IN_FLAG) || 
       (cbw->Length <= 0) ||
       (cbw->Length > MAX_COMMAND_SIZE))
    {
        FSG_ERROR " received_cbw => non-meaningful CBW: Lun %d, flags: 0x%X, cmdlen %d \n", cbw->Lun, cbw->Flags, cbw->Length FSG_END

        /* We can do anything we want here, so let's stall the
         * bulk pipes if we are allowed to. */
        fsg_set_halt(fsg, fsg->bulk_out);
        halt_bulk_in_endpoint(fsg);
        rc = ERR_FSG_CBW_INVALID;
        goto end;
    }

    /* Save the command for later */
    fsg->cmnd_size = cbw->Length;
    memcpy(fsg->cmnd, cbw->CDB, fsg->cmnd_size);
    if(cbw->Flags & USB_BULK_IN_FLAG)
        fsg->data_dir = DATA_DIR_TO_HOST;
    else
        fsg->data_dir = DATA_DIR_FROM_HOST;
    fsg->data_size = le32_to_cpu(cbw->DataTransferLength);
    if(fsg->data_size == 0)
        fsg->data_dir = DATA_DIR_NONE;
    fsg->lun = cbw->Lun;
    fsg->tag = cbw->Tag;

end:
    check_result(rc);
    return rc;
}

static int get_next_command(struct fsg_dev *fsg)
{
    struct fsg_buffhd *bh;
    int rc=0;
    uint32_t count;

    /* Wait for the next buffer to become available */
    bh = fsg->next_buffhd_to_fill;
    count = 2000;
    while(bh->state != BUF_STATE_EMPTY)
    {
        FSG_DBG " wait buffer for cbw...\n" FSG_END
        if(exception_in_progress(fsg))
        {
            rc = ERR_FSG_EXCEPTION;
            goto end;
        }
        usleep(1000);
        if(--count == 0)
        {
            rc = ERR_FSG_CBW_NO_BUF;
            goto end;
        }
    }

    /* Queue a request to read a Bulk-only CBW */
    //bh->bulk_out_intended_length = bh->outreq->length = USB_BULK_CB_WRAP_LEN;
    bh->outreq->length = USB_BULK_CB_WRAP_LEN;
    start_transfer(fsg, fsg->bulk_out, bh->outreq, &bh->outreq_busy, &bh->state);

    /* Wait for the CBW to arrive */
    count = 100*1000;
    while(bh->state != BUF_STATE_FULL)
    {
        if(!(count % 2000))
            FSG_DBG " wait cbw arrive...\n" FSG_END

        if(exception_in_progress(fsg))
        {
            rc = ERR_FSG_EXCEPTION;
            goto end;
        }
        usleep(1000);
        if(--count == 0)
            count = 100*1000;
    }

    rc = received_cbw(fsg, bh);
    bh->state = BUF_STATE_EMPTY;

end:
    check_result(rc);
    return rc;
}
