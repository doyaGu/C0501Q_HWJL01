/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *
 * @author Irene Lin
 */
#include "config.h"

#define usb_alloc_urb(a,b)  usb_alloc_urb(a)
#define usb_submit_urb(a,b) usb_submit_urb(a)

#define DID_ABORT       0x05
#define DID_ERROR       0x07


static int uas_submit_urbs(struct scsi_cmnd *cmnd, struct uas_dev_info *devinfo, gfp_t gfp);
static int uas_try_complete(struct scsi_cmnd *cmnd, const char *caller);


static void uas_unlink_data_urbs(struct uas_dev_info *devinfo,
				 struct uas_cmd_info *cmdinfo)
{
	unsigned long flags;

	/*
	 * The UNLINK_DATA_URBS flag makes sure uas_try_complete
	 * (called by urb completion) doesn't release cmdinfo
	 * underneath us.
	 */
	spin_lock_irqsave(&devinfo->lock, flags);
	cmdinfo->state |= UNLINK_DATA_URBS;
	spin_unlock_irqrestore(&devinfo->lock, flags);

	if (cmdinfo->data_in_urb)
		usb_unlink_urb(cmdinfo->data_in_urb);
	if (cmdinfo->data_out_urb)
		usb_unlink_urb(cmdinfo->data_out_urb);

	spin_lock_irqsave(&devinfo->lock, flags);
	cmdinfo->state &= ~UNLINK_DATA_URBS;
	spin_unlock_irqrestore(&devinfo->lock, flags);
}

void uas_abort_work(struct uas_dev_info *devinfo)
{
	struct uas_cmd_info *cmdinfo;
	struct scsi_cmnd *cmnd;
	unsigned long flags, i;

	spin_lock_irqsave(&devinfo->lock, flags);
	_uas_func_enter;

    for (i=0; i<SCSI_CMDQ_DEPTH; i++) {
		cmnd = devinfo->cmd_list[i];
		if (cmnd->tag != -1) {
		    cmdinfo = (struct uas_cmd_info *)cmnd->cmdinfo;
			if (!(cmdinfo->state & COMMAND_COMPLETED)) {
				cmdinfo->state |= COMMAND_ABORTED;
				if (devinfo->resetting) {
					/* uas_stat_cmplt() will not do that
					 * when a device reset is in
					 * progress */
					cmdinfo->state &= ~COMMAND_INFLIGHT;
				}
				uas_try_complete(cmnd, __func__);
			}
		}
    }
	_uas_func_leave;
	spin_unlock_irqrestore(&devinfo->lock, flags);
}

static void uas_sense(struct urb *urb, struct scsi_cmnd *cmnd)
{
    struct sense_iu *sense_iu = urb->transfer_buffer;

    if (urb->actual_length > 16) {
		unsigned len = be16_to_cpup(&sense_iu->len);
		if (len + 16 != urb->actual_length) {
			int newlen = min(len + 16, urb->actual_length) - 16;
			if (newlen < 0)
				newlen = 0;
			LOG_ERROR "%s: urb length %d "
				"disagrees with IU sense data length %d, "
				"using %d bytes of sense data\n", __func__,
					urb->actual_length, len, newlen LOG_END
			len = newlen;
		}
        cmnd->sense_buffer = (uint8_t*)malloc(len);
        memset((void*)cmnd->sense_buffer, 0x0, len);
		memcpy(cmnd->sense_buffer, sense_iu->sense, len);
    }
    cmnd->result = sense_iu->status;
    check_result(cmnd->result);
}

void uas_log_cmd_state(struct scsi_cmnd *cmnd, const char *caller)
{
	struct uas_cmd_info *ci = (void *)cmnd->cmdinfo;

    ithPrintf(" %s cmd:%X tag %d, inflight:"
		    "%s%s%s%s%s%s%s%s%s%s%s%s\n",
		    caller, cmnd->cmnd[0], cmnd->tag,
		    (ci->state & SUBMIT_STATUS_URB)     ? " s-st"  : "",
		    (ci->state & ALLOC_DATA_IN_URB)     ? " a-in"  : "",
		    (ci->state & SUBMIT_DATA_IN_URB)    ? " s-in"  : "",
		    (ci->state & ALLOC_DATA_OUT_URB)    ? " a-out" : "",
		    (ci->state & SUBMIT_DATA_OUT_URB)   ? " s-out" : "",
		    (ci->state & ALLOC_CMD_URB)         ? " a-cmd" : "",
		    (ci->state & SUBMIT_CMD_URB)        ? " s-cmd" : "",
		    (ci->state & COMMAND_INFLIGHT)      ? " CMD"   : "",
		    (ci->state & DATA_IN_URB_INFLIGHT)  ? " IN"    : "",
		    (ci->state & DATA_OUT_URB_INFLIGHT) ? " OUT"   : "",
		    (ci->state & COMMAND_COMPLETED)     ? " done"  : "",
		    (ci->state & COMMAND_ABORTED)       ? " abort" : "",
		    (ci->state & UNLINK_DATA_URBS)      ? " unlink": "",
		    (ci->state & IS_IN_WORK_LIST)       ? " work"  : "");
}

static int uas_try_complete(struct scsi_cmnd *cmnd, const char *caller)
{
    struct uas_cmd_info *cmdinfo = (struct uas_cmd_info *)cmnd->cmdinfo;

    if (cmdinfo->state & (COMMAND_INFLIGHT |
                 DATA_IN_URB_INFLIGHT |
                 DATA_OUT_URB_INFLIGHT |
                 UNLINK_DATA_URBS)) {
        //uas_log_cmd_state(cmnd, __func__);
		return -EBUSY;
    }
    BUG_ON(cmdinfo->state & COMMAND_COMPLETED);
    cmdinfo->state |= COMMAND_COMPLETED;
    usb_free_urb(cmdinfo->data_in_urb);
    usb_free_urb(cmdinfo->data_out_urb);
    if (cmdinfo->state & COMMAND_ABORTED) {
        ithPrintf(" uas_try_complete(%s): abort completed \n", caller);
		ithPrintVram8((uint32_t)cmnd->cmnd, 16);
        cmnd->result = DID_ABORT << 16;
    }
    cmnd->scsi_done(cmnd);
    return 0;
}

static void uas_xfer_data(struct urb *urb, struct scsi_cmnd *cmnd,
			  unsigned direction)
{
    struct uas_cmd_info *cmdinfo = (struct uas_cmd_info*)cmnd->cmdinfo;
	int err;

    cmdinfo->state |= direction | SUBMIT_STATUS_URB;
    err = uas_submit_urbs(cmnd, cmnd->devinfo, GFP_ATOMIC);
    if (err) {
		check_result(err);
    }
}

static void uas_stat_cmplt(struct urb *urb)
{
    struct iu *iu = urb->transfer_buffer;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)urb->context;

    struct scsi_cmnd *cmnd;
    struct uas_cmd_info *cmdinfo;
    unsigned long flags;
    u16 tag;

    _uas_func_enter;

    if (urb->status) {
        LOG_ERROR " uas_stat_cmplt() urb status: 0x%X(%d) \n", urb->status, urb->status LOG_END
        usb_free_urb(urb);
        return;
    }

	if (devinfo->resetting) {
        LOG_ERROR " uas_stat_cmplt() resetting: (%d) \n", urb->status LOG_END
		usb_free_urb(urb);
		return;
	}

    spin_lock_irqsave(&devinfo->lock, flags);
    tag = be16_to_cpup(&iu->tag) - 1;
    if (tag == 0)
        cmnd = devinfo->cmnd;
    else
        cmnd = find_cmnd_by_tag(devinfo, tag-1);

    // TODO: for task management
    if (!cmnd) {
        LOG_DEBUG " uas_stat_cmplt() find null scsi cmnd! (tag %d) \n", tag LOG_END
        goto end;
    }
//<<
    if(iu->iu_id == IU_ID_RESPONSE)
    {
        ithPrintf(" RESP: %d \n", urb->actual_length);
        ithPrintVram8((uint32_t)iu, urb->actual_length);
        goto end;
    }
//>>

#if defined(DUMP_STATUS)
    ithPrintf(" Status: %d \n", urb->actual_length);
    ithPrintVram8((uint32_t)iu, urb->actual_length);
#endif 
    
    cmdinfo = (struct uas_cmd_info *)cmnd->cmdinfo;
    switch (iu->iu_id) {
    case IU_ID_STATUS:
        LOG_DEBUG " status pipe: STATUS \n" LOG_END
        if (devinfo->cmnd == cmnd) /* this command is completed */
            devinfo->cmnd = NULL;

        if (urb->actual_length < 16)
            LOG_ERROR " This is old sense spec.?? \n" LOG_END
        uas_sense(urb, cmnd);
        if (cmnd->result != 0) {
			/* cancel data transfers on error */
			spin_unlock_irqrestore(&devinfo->lock, flags);
			uas_unlink_data_urbs(devinfo, cmdinfo);
			spin_lock_irqsave(&devinfo->lock, flags);
        }
        cmdinfo->state &= ~COMMAND_INFLIGHT;
        uas_try_complete(cmnd, __func__);
        break;
    case IU_ID_READ_READY:
        LOG_DEBUG " status pipe: READ_READY \n" LOG_END
        uas_xfer_data(urb, cmnd, SUBMIT_DATA_IN_URB);
        break;
    case IU_ID_WRITE_READY:
        LOG_DEBUG " status pipe: WRITE_READY \n" LOG_END
        uas_xfer_data(urb, cmnd, SUBMIT_DATA_OUT_URB);
        break;
    default:
        LOG_ERROR " status pipe received iu_id: %d \n", iu->iu_id LOG_END
    }

end:
    usb_free_urb(urb);
    spin_unlock_irqrestore(&devinfo->lock, flags);
    _uas_func_leave;
}

static void uas_data_cmplt(struct urb *urb)
{
    struct scsi_cmnd *cmnd = (struct scsi_cmnd*)urb->context;
    struct uas_cmd_info *cmdinfo = (struct uas_cmd_info *)cmnd->cmdinfo;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)cmnd->devinfo;
    unsigned long flags;

    spin_lock_irqsave(&devinfo->lock, flags);
    _uas_func_enter;

    if (cmdinfo->data_in_urb == urb) {
        cmdinfo->state &= ~DATA_IN_URB_INFLIGHT;
#if defined(DUMP_DATA)
        ithPrintf(" Data-in: %d \n", urb->actual_length);
        ithPrintVram8((uint32_t)cmnd->transfer_buffer, (urb->actual_length>0x200) ? 0x200 : urb->actual_length);
#endif 
    } else if (cmdinfo->data_out_urb == urb)
        cmdinfo->state &= ~DATA_OUT_URB_INFLIGHT;
    else
        LOG_ERROR " %s: what happened? \n", __func__ LOG_END

    if (urb->status)
        LOG_ERROR " data_cmplt: status %d (%d/%d) \n", urb->status, urb->actual_length, cmnd->transfer_len LOG_END

    uas_try_complete(cmnd, __func__);

    _uas_func_leave;
    spin_unlock_irqrestore(&devinfo->lock, flags);
}

static struct urb *uas_alloc_data_urb(struct uas_dev_info *devinfo, gfp_t gfp,
				      unsigned int pipe, u16 stream_id,
                      struct scsi_cmnd *cmnd)
{
	struct usb_device *udev = devinfo->udev;
    struct urb *urb = usb_alloc_urb(0, gfp);

    if (!urb)
        goto out;

    usb_fill_bulk_urb(urb, udev, pipe,
                      cmnd->transfer_buffer,
                      cmnd->transfer_len,
                      uas_data_cmplt, (void*)cmnd);
	if (devinfo->use_streams)
		urb->stream_id = stream_id;
out:
    return urb;
}

static struct urb *uas_alloc_sense_urb(struct uas_dev_info *devinfo, gfp_t gfp, 
                        u16 stream_id)
{
	struct usb_device *udev = devinfo->udev;
    struct urb *urb = usb_alloc_urb(0, gfp);
    struct sense_iu *iu;

    if (!urb)
        goto out;

    iu = kzalloc(sizeof(*iu), gfp);
    if (!iu)
        goto free;

    usb_fill_bulk_urb(urb, udev, devinfo->status_pipe, iu, sizeof(*iu),
                      uas_stat_cmplt, (void*)devinfo);
    urb->stream_id = stream_id;
    urb->transfer_flags |= URB_FREE_BUFFER;
 out:
    return urb;
 free:
    usb_free_urb(urb);
    return NULL;
}

static struct urb *uas_alloc_cmd_urb(struct uas_dev_info *devinfo, gfp_t gfp,
                    struct scsi_cmnd *cmnd, u16 stream_id)
{
	struct usb_device *udev = devinfo->udev;
    struct urb *urb = usb_alloc_urb(0, gfp);
    struct command_iu *iu;
    int len;

    if (!urb)
        goto out;

    len = cmnd->cmd_len - 16;
    if (len < 0)
        len = 0;
	len = ALIGN(len, 4);
    iu = kzalloc(sizeof(*iu) + len, gfp);
    if (!iu)
        goto free;

    iu->iu_id = IU_ID_COMMAND;
    if(cmnd->tagged)
        iu->tag = cpu_to_be16(cmnd->tag + 2);
    else
        iu->tag = cpu_to_be16(1);
    iu->prio_attr = UAS_SIMPLE_TAG;
    iu->len = len;
    memcpy((void*)&iu->lun, (void*)&devinfo->scsi_lun[cmnd->lun], sizeof(struct scsi_lun)); 
    memcpy(iu->cdb, cmnd->cmnd, cmnd->cmd_len);

    usb_fill_bulk_urb(urb, udev, devinfo->cmd_pipe, iu, sizeof(*iu) + len,
                      usb_free_urb, NULL);
    urb->transfer_flags |= URB_FREE_BUFFER;

#if defined(DUMP_CMD)
    ithPrintf(" CMD: \n");
    ithPrintVram8((uint32_t)iu, (sizeof(*iu) + len));
#endif 

 out:
    return urb;
 free:
    usb_free_urb(urb);
    return NULL;
}

static int uas_submit_task_urb(struct scsi_cmnd *cmnd, gfp_t gfp,
			       u8 function, u16 stream_id)
{
    struct uas_dev_info *devinfo = cmnd->devinfo;
	struct usb_device *udev = devinfo->udev;
    struct urb *urb = usb_alloc_urb(0, gfp);
    struct task_mgmt_iu *iu;
    int err = -ENOMEM;

    _uas_func_enter;

    if (!urb)
		goto err;

	iu = kzalloc(sizeof(*iu), gfp);
	if (!iu)
		goto err;

    iu->iu_id = IU_ID_TASK_MGMT;
    iu->tag = cpu_to_be16(stream_id);
    memcpy((void*)&iu->lun, (void*)&devinfo->scsi_lun[cmnd->lun], sizeof(struct scsi_lun));

    iu->function = function;
    switch (function) {
    case TMF_ABORT_TASK:
        if (cmnd->tagged)
            iu->task_tag = cpu_to_be16(cmnd->tag + 2);
        else
            iu->task_tag = cpu_to_be16(1);
        break;
    }

    usb_fill_bulk_urb(urb, udev, devinfo->cmd_pipe, iu, sizeof(*iu),
                      usb_free_urb, NULL);
    urb->transfer_flags |= URB_FREE_BUFFER;

	err = usb_submit_urb(urb, gfp);
	if (err)
		goto err;
	usb_anchor_urb(urb, &devinfo->cmd_urbs);

	return 0;

err:
    usb_free_urb(urb);
    check_result(err);
    return err;
}

/* stream is for USB3.0 */
static int uas_submit_sense_urb(struct uas_dev_info *devinfo, 
                gfp_t gfp, unsigned int stream)
{
    int rc = 0;
    struct urb *urb;

    urb = uas_alloc_sense_urb(devinfo, gfp, stream);
    if (!urb) {
        rc = ERROR_UAS_ALLOC_SENSE_URB;
        goto end;
    }
//ithPrintf("1: %X \n", urb);

    if (usb_submit_urb(urb, gfp)) {
        LOG_ERROR "sense urb submission failure\n" LOG_END
        usb_free_urb(urb);
        rc = ERROR_UAS_SUBMIT_SENSE_URB;
        goto end;
    }
    usb_anchor_urb(urb, &devinfo->sense_urbs);
	return 0;
end:
    check_result(rc);
    return rc;
}

static int uas_submit_urbs(struct scsi_cmnd *cmnd, 
            struct uas_dev_info *devinfo, gfp_t gfp)
{
    struct uas_cmd_info *cmdinfo = (struct uas_cmd_info *)cmnd->cmdinfo;
    int err;

    if (cmdinfo->state & SUBMIT_STATUS_URB) {
        err = uas_submit_sense_urb(devinfo, gfp,
                        cmdinfo->stream);
        if (err) {
            goto end;
        }
        cmdinfo->state &= ~SUBMIT_STATUS_URB;
    }

    if (cmdinfo->state & ALLOC_DATA_IN_URB) {
        cmdinfo->data_in_urb = uas_alloc_data_urb(devinfo, gfp,
                  devinfo->data_in_pipe, cmdinfo->stream,
                  cmnd);
        if (!cmdinfo->data_in_urb)
        {
            err = ERROR_UAS_ALLOC_DATA_URB;
            goto end;
        }
        cmdinfo->state &= ~ALLOC_DATA_IN_URB;
    }

    if (cmdinfo->state & SUBMIT_DATA_IN_URB) {
//ithPrintf("3: %X \n", cmdinfo->data_in_urb);
        if (usb_submit_urb(cmdinfo->data_in_urb, gfp)) {
            err = ERROR_UAS_SUBMIT_DATA_IN_URB;
            goto end;
        }
        cmdinfo->state &= ~SUBMIT_DATA_IN_URB;
        cmdinfo->state |= DATA_IN_URB_INFLIGHT;
		usb_anchor_urb(cmdinfo->data_in_urb, &devinfo->data_urbs);
    }

    if (cmdinfo->state & ALLOC_DATA_OUT_URB) {
        cmdinfo->data_out_urb = uas_alloc_data_urb(devinfo, gfp,
                      devinfo->data_out_pipe, cmdinfo->stream,
                      cmnd);
        if (!cmdinfo->data_out_urb)
        {
            err = ERROR_UAS_ALLOC_DATA_URB;
            goto end;
        }
        cmdinfo->state &= ~ALLOC_DATA_OUT_URB;
    }

    if (cmdinfo->state & SUBMIT_DATA_OUT_URB) {
#if defined(DUMP_DATA)
        ithPrintf(" Data-out: %d \n", cmnd->transfer_len);
        ithPrintVram8((uint32_t)cmnd->transfer_buffer, (cmnd->transfer_len>0x200) ? 0x200 : cmnd->transfer_len);
#endif 
//ithPrintf("3: %X \n", cmdinfo->data_out_urb);
        if (usb_submit_urb(cmdinfo->data_out_urb, gfp)) {
            err = ERROR_UAS_SUBMIT_DATA_OUT_URB;
            goto end;
        }
        cmdinfo->state &= ~SUBMIT_DATA_OUT_URB;
        cmdinfo->state |= DATA_OUT_URB_INFLIGHT;
		usb_anchor_urb(cmdinfo->data_out_urb, &devinfo->data_urbs);
    }

    if (cmdinfo->state & ALLOC_CMD_URB) {
        cmdinfo->cmd_urb = uas_alloc_cmd_urb(devinfo, gfp, cmnd,
                             cmdinfo->stream);
        if (!cmdinfo->cmd_urb)
        {
            err = ERROR_UAS_ALLOC_CMD_URB;
            goto end;
        }
        cmdinfo->state &= ~ALLOC_CMD_URB;
    }

    if (cmdinfo->state & SUBMIT_CMD_URB) {
		usb_get_urb(cmdinfo->cmd_urb);
//ithPrintf("2: %X \n", cmdinfo->cmd_urb);
        if (usb_submit_urb(cmdinfo->cmd_urb, gfp)) {
			LOG_ERROR "cmd urb submission failure\n" LOG_END
			usb_put_urb(cmdinfo->cmd_urb);
			usb_free_urb(cmdinfo->cmd_urb); /* usb willn't callback complete to free urb */
            err = ERROR_UAS_SUBMIT_CMD_URB;
            goto end;
        }
        usb_anchor_urb(cmdinfo->cmd_urb, &devinfo->cmd_urbs);
		usb_put_urb(cmdinfo->cmd_urb);
        cmdinfo->cmd_urb = NULL;  /* it will be freed with complete function */
        cmdinfo->state &= ~SUBMIT_CMD_URB;
        cmdinfo->state |= COMMAND_INFLIGHT;
    }

end:
    check_result(err);
    return err;
}

int uas_queuecommand(struct scsi_cmnd *cmnd)
{
    struct uas_dev_info *devinfo = cmnd->devinfo;
    struct uas_cmd_info *cmdinfo = (struct uas_cmd_info *)cmnd->cmdinfo;
    unsigned long flags;
    int err;

	if (devinfo->resetting)
		return ERROR_UAS_NODEV;

    spin_lock_irqsave(&devinfo->lock, flags);

    /* not use queue */
    if (devinfo->cmnd) {
        err = ERROR_UAS_DEVICE_BUSY;
        goto end;
    }

    if (cmnd->tagged) {
        cmdinfo->stream = cmnd->tag + 2;
    } else {
        devinfo->cmnd = cmnd;     
        cmdinfo->stream = 1;
    }

    cmdinfo->state = SUBMIT_STATUS_URB |
             ALLOC_CMD_URB | SUBMIT_CMD_URB;

    switch (cmnd->sc_data_direction) {
    case DMA_FROM_DEVICE:
        cmdinfo->state |= ALLOC_DATA_IN_URB | SUBMIT_DATA_IN_URB;
        break;
    case DMA_BIDIRECTIONAL:
        cmdinfo->state |= ALLOC_DATA_IN_URB | SUBMIT_DATA_IN_URB;
    case DMA_TO_DEVICE:
        cmdinfo->state |= ALLOC_DATA_OUT_URB | SUBMIT_DATA_OUT_URB;
    case DMA_NONE:
        break;
    }

    /* not USB3.0 */
    if (!devinfo->use_streams) {
        cmdinfo->state &= ~(SUBMIT_DATA_IN_URB | SUBMIT_DATA_OUT_URB);
        cmdinfo->stream = 0;
    }

    err = uas_submit_urbs(cmnd, devinfo, GFP_ATOMIC);
    if (err) {
		/* If we did nothing, give up now */
		if (cmdinfo->state & SUBMIT_STATUS_URB)
			goto end;

		LOG_ERROR " should be abort later! \n" LOG_END
		err = 0;
    }

end:
    spin_unlock_irqrestore(&devinfo->lock, flags);
    check_result(err);
    return err;
}

int uas_eh_task_mgmt(struct scsi_cmnd *cmnd,
			    const char *fname, u8 function)
{
    struct uas_dev_info *devinfo = cmnd->devinfo;
    struct uas_cmd_info *cmdinfo = (struct uas_cmd_info *)cmnd->cmdinfo;
	u16 tag = SCSI_CMDQ_DEPTH-1;
	unsigned long flags;
    int rc;

    spin_lock_irqsave(&devinfo->lock, flags);
    memset(&devinfo->response, 0, sizeof(devinfo->response));
    rc = uas_submit_sense_urb(devinfo, GFP_ATOMIC, tag);
    if(rc)
    {
        LOG_ERROR "%s: %s: submit sense urb failed\n",
			     __func__, fname LOG_END
        rc = -1;//ERROR_UAS_TMF_SUBMIT_SENSE_URB;
        spin_unlock_irqrestore(&devinfo->lock, flags);
        return rc;
    }
    rc = uas_submit_task_urb(cmnd, GFP_ATOMIC, function, tag);
    if(rc)
    {
        LOG_ERROR "%s: %s: submit task mgmt urb failed\n",
			     __func__, fname LOG_END
        rc = -2;//ERROR_UAS_TMF_SUBMIT_TASK_MGNT_URB;
        spin_unlock_irqrestore(&devinfo->lock, flags);
        return rc;
    }
    spin_unlock_irqrestore(&devinfo->lock, flags);

	if (usb_wait_anchor_empty_timeout(&devinfo->sense_urbs, 3000) == 0) {
		LOG_ERROR "%s: %s timed out\n", __func__, fname LOG_END
		return -3;
	}
	if (be16_to_cpu(devinfo->response.tag) != tag) {
		LOG_ERROR "%s: %s failed (wrong tag %d/%d)\n", __func__,
			     fname, be16_to_cpu(devinfo->response.tag), tag LOG_END
		return -4;
	}
	if (devinfo->response.response_code != RC_TMF_COMPLETE) {
		LOG_ERROR "%s: %s failed (rc 0x%x)\n", __func__,
			     fname, devinfo->response.response_code LOG_END
		return -5;
	}

    return rc;
}

