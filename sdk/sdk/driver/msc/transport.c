/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file transport.c
 * Driver for USB Mass Storage compliant devices.
 *
 * @author Irene Lin
 */

#include "msc/config.h"


/**
 * This is a version of usb_clear_halt() that doesn't read the status from
 * the device -- this is because some devices crash their internal firmware
 * when the status is requested after a halt
 */
MMP_INT usb_stor_clear_halt(struct usb_device *dev, MMP_INT pipe)
{
    MMP_INT result = 0;
    MMP_UINT8 status[2] = {0};
    MMP_UINT16 endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

    result = usb_control_msg(dev, 
                             usb_sndctrlpipe(dev, 0),
                             USB_REQ_CLEAR_FEATURE, 
                             USB_RECIP_ENDPOINT, 
                             0, 
                             endp, 
                             MMP_NULL, 
                             0, 
                             2000);
    /* don't clear if failed */
    if (result < 0)
        return result;

   // usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

    /* toggle is reset on clear */
    //usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);

    return 0;
}

MMP_INT usb_stor_transfer(Scsi_Cmnd *srb, struct us_data *us)
{
    MMP_INT result = 0;
    MMP_INT actual_len = 0;
    MMP_INT32 pipe;

    /** calculate the appropriate pipe information */
    if(us->srb.sc_data_direction == SCSI_DATA_READ)
        pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
    else
        pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

    /** transfer the data */
    LOG_DEBUG "usb_stor_transfer(): xfer %d bytes\n", srb->request_bufflen LOG_END
    result = usb_bulk_msg(us->pusb_dev, pipe, srb->request_buffer, srb->request_bufflen, &actual_len, US_BULK_TIMEOUT);
    LOG_DEBUG "usb_stor_transfer() returned %d xferred %d/%d\n", result, actual_len, srb->request_bufflen LOG_END

    /** if we stall, we need to clear it before we go on */
    if(result == -EPIPE) 
    {
        LOG_WARNING "clearing endpoint halt for pipe 0x%x\n", pipe LOG_END
        if(usb_stor_clear_halt(us->pusb_dev, pipe))
            return US_BULK_TRANSFER_FAILED;

        return US_BULK_TRANSFER_STALLED;
    }

    /* did we abort this command? */
    if(result == -ENOENT) 
    {
        LOG_WARNING "usb_stor_transfer(): transfer aborted\n" LOG_END
        return US_BULK_TRANSFER_ABORTED;
    }

    /** did we send all the data? */
    if(actual_len == srb->request_bufflen) 
    {
        LOG_DEBUG "usb_stor_transfer(): transfer complete\n" LOG_END
        return US_BULK_TRANSFER_GOOD;
    }

    /** NAK - that means we've retried a few times already */
    if(result == -ETIMEDOUT) 
    {
        LOG_WARNING "usb_stor_transfer(): device NAKed\n" LOG_END

        if(!usb_dev_exist(us->pusb_dev))
            return ERROR_MSC_DEVICE_NOT_EXIST;

        return US_BULK_TRANSFER_FAILED;
    }

    /** the catch-all error case */
    if(result) 
    {
        LOG_ERROR "usb_stor_transfer(): unknown error\n" LOG_END

        if(!usb_dev_exist(us->pusb_dev))
            return ERROR_MSC_DEVICE_NOT_EXIST;

        return US_BULK_TRANSFER_FAILED;
    }

    /** no error code, so we must have transferred some data, just not all of it */
    return US_BULK_TRANSFER_SHORT;
}

/***********************************************************************
 * Transport routines
 ***********************************************************************/

/* Invoke the transport and basic error-handling/recovery methods
 *
 * This is used by the protocol layers to actually send the message to
 * the device and receive the response.
 */
void usb_stor_invoke_transport(Scsi_Cmnd *srb, struct us_data *us)
{
    MMP_BOOL need_auto_sense = MMP_FALSE;
    MMP_INT result = 0;

    /** send the command to the transport layer */
    result = us->transport(srb, us);

    /** Irene: check if device is removed */
    if(result != US_BULK_TRANSFER_GOOD)
    {
        if(!usb_dev_exist(us->pusb_dev))
        {
            srb->result = DID_ERROR << 16;
            goto end;
        }
    }

    // +wlHsu
    if( result == USB_STOR_HW_HANG_ERROR )
    {
        sdk_msg_ex(SDK_MSG_TYPE_ERROR, "-- USB controller hang !! \n" );
        srb->result = DID_HW_HANG << 16;
        goto end;    
    }
    // -wlHsu

    /**
     * if the command gets aborted by the higher layers, we need to
     * short-circuit all other processing
     */
    if(result == USB_STOR_TRANSPORT_ABORTED) 
    {
        LOG_WARNING "-- transport indicates command was aborted\n" LOG_END
        srb->result = DID_ABORT << 16;
        goto end;
    }

    /** if there is a transport error, reset and don't auto-sense */
    if(result == USB_STOR_TRANSPORT_ERROR) 
    {
        LOG_WARNING "-- transport indicates error, resetting\n" LOG_END
        us->transport_reset(us);
        srb->result = DID_ERROR << 16;
        goto end;
    }

    /**
     * Determine if we need to auto-sense
     */

    /**
     * If we have a failure, we're going to do a REQUEST_SENSE 
     * automatically.  Note that we differentiate between a command
     * "failure" and an "error" in the transport mechanism.
     */
    if(result == USB_STOR_TRANSPORT_FAILED) 
    {
        LOG_WARNING "-- transport indicates command failure\n" LOG_END
        need_auto_sense = MMP_TRUE;
    }

    /**
     * Also, if we have a short transfer on a command that can't have
     * a short transfer, we're going to do this.
     */
    if((srb->result == US_BULK_TRANSFER_SHORT) &&
        !((srb->cmnd[0] == REQUEST_SENSE) ||
          (srb->cmnd[0] == INQUIRY) ||
          (srb->cmnd[0] == MODE_SENSE))) 
    {
        LOG_WARNING "-- unexpectedly short transfer\n" LOG_END
        need_auto_sense = MMP_TRUE;
    }

    /** Now, if we need to do the auto-sense, let's do it */
    if(need_auto_sense) 
    {
        MMP_INT temp_result;
        void* old_request_buffer;
        MMP_UINT32 old_request_bufflen;
        MMP_UINT8 old_sc_data_direction;
        MMP_UINT8 old_cmnd[MAX_COMMAND_SIZE];

        LOG_DEBUG "Issuing auto-REQUEST_SENSE\n" LOG_END

        /** save the old command */
        memcpy(old_cmnd, srb->cmnd, MAX_COMMAND_SIZE);
        memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
        memset(srb->sense_buffer, 0, SCSI_SENSE_BUFFERSIZE);

        /** set the command and the LUN */
        srb->cmnd[0] = REQUEST_SENSE;
        srb->cmnd[1] = old_cmnd[1] & 0xE0;
        srb->cmnd[4] = DATA_LENGTH_REQUEST_SENSE;

        /** set the transfer direction */
        old_sc_data_direction = srb->sc_data_direction;
        srb->sc_data_direction = SCSI_DATA_READ;

        /** use the new buffer we have */
        old_request_buffer = srb->request_buffer;
        srb->request_buffer = srb->sense_buffer;

        /** set the buffer length for transfer */
        old_request_bufflen = srb->request_bufflen;
        srb->request_bufflen = DATA_LENGTH_REQUEST_SENSE;

        /** issue the auto-sense command */
        temp_result = us->transport(&us->srb, us);

        /** let's clean up right away */
        srb->request_buffer = old_request_buffer;
        srb->request_bufflen = old_request_bufflen;
        srb->sc_data_direction = old_sc_data_direction;
        memcpy(srb->cmnd, old_cmnd, MAX_COMMAND_SIZE);

        if(temp_result == USB_STOR_TRANSPORT_ABORTED) 
        {
            LOG_WARNING "-- auto-sense aborted\n" LOG_END
            srb->result = DID_ABORT << 16;
            goto end;
        }
        if(temp_result != USB_STOR_TRANSPORT_GOOD) 
        {
            LOG_ERROR "-- auto-sense failure\n" LOG_END
            /**
             * we skip the reset if this happens to be a
             * multi-target device, since failure of an
             * auto-sense is perfectly valid
             */
            us->transport_reset(us);
            srb->result = DID_ERROR << 16;
            goto end;
        }

        LOG_WARNING "-- Result from auto-sense is %d\n", temp_result LOG_END
        LOG_WARNING "-- code: 0x%x, key: 0x%02X, ASC: 0x%02X, ASCQ: 0x%02X\n",
              srb->sense_buffer[0],
              srb->sense_buffer[2] & 0xf,
              srb->sense_buffer[12], 
              srb->sense_buffer[13] LOG_END

        /** set the result so the higher layers expect this data */
        srb->result = CHECK_CONDITION;

        /* If things are really okay, then let's show that */
        if((srb->sense_buffer[2] & 0xF) == 0x0)
            srb->result = GOOD;
    } 
    else /* if (need_auto_sense) */
    {
        srb->result = GOOD;
    }

    /**
     * Regardless of auto-sense, if we _know_ we have an error
     * condition, show that in the result code
     */
    if(result == USB_STOR_TRANSPORT_FAILED)
        srb->result = CHECK_CONDITION;

end:
    if(result || srb->result)
        LOG_ERROR " usb_stor_invoke_transport() result = 0x%08X, srb->result = 0x%08X \n", result, srb->result LOG_END
    return;
}


/*
 * Bulk only transport
 */

/* Determine what the maximum LUN supported is */
MMP_INT usb_stor_Bulk_max_lun(struct us_data *us)
{
    MMP_UINT8 max_lun=0;
    MMP_INT result;
    MMP_UINT32 pipe = usb_rcvctrlpipe(us->pusb_dev, 0);

    result = usb_control_msg(us->pusb_dev, 
                             pipe,
                             US_BULK_GET_MAX_LUN, 
                             USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE,
                             0, 
                             us->ifnum, 
                             &max_lun, 
                             sizeof(max_lun), 
                             1000);
    LOG_INFO " GetMaxLUN command result is %d, max_lun is %d\n", result, max_lun LOG_END

    /* if we have a successful request, return the result */
    if(result == 1)
        return max_lun;

    /* if we get a STALL, clear the stall */
    if(result == -EPIPE) 
    {
        LOG_WARNING "clearing endpoint halt for pipe 0x%x\n", pipe LOG_END

        /* Use usb_stor_clear_halt() because the state machine
         *  is not yet alive */
        usb_stor_clear_halt(us->pusb_dev, pipe);
    }

    /* return the default -- no LUNs */
    return 0;
}

MMP_INT usb_stor_Bulk_reset(struct us_data *us)
{
    MMP_INT result = 0;

    LOG_DEBUG "Bulk reset requested \n" LOG_END

    /** if the device was removed, then we're already reset */
    if(!us->pusb_dev)
    {
        result = ERROR_MSC_NO_DEVICE;
        goto end;
    }

    result = usb_control_msg(us->pusb_dev, 
                             usb_sndctrlpipe(us->pusb_dev,0), 
                             US_BULK_RESET_REQUEST, 
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                             0, 
                             us->ifnum, 
                             MMP_NULL, 
                             0, 
                             1000);
    if(result < 0) 
    {
        LOG_ERROR "Bulk soft reset failed %d\n", result LOG_END
        result = ERROR_MSC_SW_RESET_FAIL;
        goto end;
    }

    /* long wait for reset?? */
    MMP_Sleep(200);

    usb_stor_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
    usb_stor_clear_halt(us->pusb_dev, usb_sndbulkpipe(us->pusb_dev, us->ep_out));

    LOG_DEBUG " Bulk soft reset completed!! \n" LOG_END

end:
    if(result)
        LOG_ERROR " usb_stor_Bulk_reset() err 0x%08X \n", result LOG_END
    return result;
}

MMP_INT usb_stor_Bulk_transport(Scsi_Cmnd *srb, struct us_data *us)
{
    struct bulk_cb_wrap bcb = {0};
    struct bulk_cs_wrap bcs = {0};
    MMP_INT result = 0;
    MMP_UINT32 pipe = 0;
    MMP_INT actual_len = 0;

    //============= CBW ===============
    /** set up the command wrapper */
    bcb.Signature = cpu_to_le32(US_BULK_CB_SIGN);
    bcb.DataTransferLength = cpu_to_le32(srb->request_bufflen);
    bcb.Flags = (srb->sc_data_direction == SCSI_DATA_READ) ? 1 << 7 : 0;
    bcb.Tag = cpu_to_le32(srb->serial_number);
    //bcb.Lun = srb->cmnd[1] >> 5;
    bcb.Lun = srb->lun;
    bcb.Length = srb->cmd_len;

    /** construct the pipe handle */
    pipe = usb_sndbulkpipe(us->pusb_dev, us->ep_out);

    /** copy the command payload */
    memcpy(bcb.CDB, srb->cmnd, bcb.Length);

    /** send it to out endpoint */
#if defined(DUMP_CMD)
    {
        MMP_UINT8 i=0;
        MMP_UINT8* cmd = (MMP_UINT8*)&bcb;
        LOG_DATA "\nCBW:" LOG_END
        for(i=0; i<31; i++)
        {
            if(!(i%4)) 
                LOG_DATA " " LOG_END
            LOG_DATA "%02X ", cmd[i] LOG_END
        }
        LOG_DATA "\n" LOG_END
    }
#endif
#if defined(US_CBW_BUSY_WAIT_EN)
    result = usb_bulk_busy_msg(us->pusb_dev, pipe, &bcb, US_BULK_CB_WRAP_LEN, &actual_len, US_BULK_TIMEOUT);
#else
    result = usb_bulk_msg(us->pusb_dev, pipe, &bcb, US_BULK_CB_WRAP_LEN, &actual_len, US_BULK_TIMEOUT);
#endif
    if(result)
        LOG_ERROR "Bulk command transfer result=%d\n", result LOG_END

    /** if the command was aborted, indicate that */
    if(result == -ENOENT)
        return USB_STOR_TRANSPORT_ABORTED;

    /** if we stall, we need to clear it before we go on */
    if(result == -EPIPE) 
    {
        LOG_WARNING "clearing endpoint halt for pipe 0x%x\n", pipe LOG_END
        /** Irene: Should we re-write it for special device that crash their internal firmware
                   when the status is requested after a halt ??*/
        result = usb_stor_clear_halt(us->pusb_dev, pipe);

        /** if the command was aborted, indicate that */
        if(result == -ENOENT)
            return USB_STOR_TRANSPORT_ABORTED;
        result = -EPIPE;
    } 
    else if(result == -ERR_HW_HANG)
    {
        // +wlHsu
        return USB_STOR_HW_HANG_ERROR;
        // -wlHsu
    }
    else if(result) 
    {
        /** unknown error -- we've got a problem */
        return USB_STOR_TRANSPORT_ERROR;
    }

    //============= DATA ===============
    /** if the command transfered well, then we go to the data stage */
    if(result == 0) 
    {
        /** send/receive data payload, if there is any */
        if(bcb.DataTransferLength) 
        {
            result = srb->result = usb_stor_transfer(srb, us);
            LOG_DEBUG "Bulk data transfer result 0x%x\n", result LOG_END

            /* if it was aborted, we need to indicate that */
            if(result == US_BULK_TRANSFER_ABORTED)
                return USB_STOR_TRANSPORT_ABORTED;
            else if(result == ERROR_MSC_DEVICE_NOT_EXIST)
                return result;
            else if(result == US_BULK_TRANSFER_FAILED)
                return USB_STOR_TRANSPORT_ERROR;
            else if(result == US_BULK_TRANSFER_STALLED)
                ;
        }
    }


    //============= CSW ===============
    /** construct the pipe handle */
    pipe = usb_rcvbulkpipe(us->pusb_dev, us->ep_in);

    /** get CSW for device status */
    LOG_DEBUG "Attempting to get CSW...\n" LOG_END
    result = usb_bulk_msg(us->pusb_dev, pipe, &bcs, US_BULK_CS_WRAP_LEN, &actual_len, US_BULK_TIMEOUT);

    /** if the command was aborted, indicate that */
    if(result == -ENOENT)
        return USB_STOR_TRANSPORT_ABORTED;

    /** did the attempt to read the CSW fail? */
    if(result == -EPIPE) 
    {
        LOG_WARNING "clearing endpoint halt for pipe 0x%x\n", pipe LOG_END
        result = usb_stor_clear_halt(us->pusb_dev, pipe);

        /** if the command was aborted, indicate that */
        if(result == -ENOENT)
            return USB_STOR_TRANSPORT_ABORTED;

        /** get the status again */
        LOG_WARNING "Attempting to get CSW (2nd try)...\n" LOG_END
        result = usb_bulk_msg(us->pusb_dev, pipe, &bcs, US_BULK_CS_WRAP_LEN, &actual_len, US_BULK_TIMEOUT);

        /** if the command was aborted, indicate that */
        if(result == -ENOENT)
            return USB_STOR_TRANSPORT_ABORTED;

        /** if it fails again, we need a reset and return an error*/
        if(result == -EPIPE) 
        {
            LOG_WARNING "clearing halt for pipe 0x%x\n", pipe LOG_END
            result = usb_stor_clear_halt(us->pusb_dev, pipe);

            /** if the command was aborted, indicate that */
            if(result == -ENOENT)
                return USB_STOR_TRANSPORT_ABORTED;
            return USB_STOR_TRANSPORT_ERROR;
        }
        else if(result == -ERR_HW_HANG)
        {
            // +wlHsu
            return USB_STOR_HW_HANG_ERROR;
            // -wlHsu
    }
    }
    else if( result == -ERR_HW_HANG )
    {
        // +wlHsu
        return USB_STOR_HW_HANG_ERROR;
        // -wlHsu
    }


    /** if we still have a failure at this point, we're in trouble */
    LOG_DEBUG "Bulk status result = %d\n", result LOG_END
    if(result)
        return USB_STOR_TRANSPORT_ERROR;

    /** check bulk status */
#if defined(DUMP_CMD)
    {
        MMP_UINT8 i=0;
        MMP_UINT8* cmd = (MMP_UINT8*)&bcs;
        LOG_DATA "CSW:" LOG_END
        for(i=0; i<13; i++)
        {
            if(!(i%4)) 
                LOG_DATA " " LOG_END
            LOG_DATA "%02X ", cmd[i] LOG_END
        }
        LOG_DATA "\n" LOG_END
    }
#endif
    if(bcs.Signature != cpu_to_le32(US_BULK_CS_SIGN) || 
        bcs.Tag != bcb.Tag || 
        bcs.Status > US_BULK_STAT_PHASE || actual_len != 13) 
    {
        LOG_ERROR "Bulk logical error\n" LOG_END
        return USB_STOR_TRANSPORT_ERROR;
    }

    /** based on the status code, we report good or bad */
    switch(bcs.Status) 
    {
    case US_BULK_STAT_OK:
        /** command good -- note that data could be short */
        return USB_STOR_TRANSPORT_GOOD;

    case US_BULK_STAT_FAIL:
        /** command failed */
        LOG_WARNING "US_BULK_STAT_FAIL\n" LOG_END
        return USB_STOR_TRANSPORT_FAILED;
    case US_BULK_STAT_PHASE:
        /** phase error -- note that a transport reset will be
         *  invoked by the invoke_transport() function
         */
        LOG_ERROR "US_BULK_STAT_PHASE\n" LOG_END
        return USB_STOR_TRANSPORT_ERROR;
    }

    /** we should never get here, but if we do, we're in trouble */
    return USB_STOR_TRANSPORT_ERROR;
}



/**
 * This issues a CB[I] Reset to the device in question
 */
MMP_INT usb_stor_CB_reset(struct us_data *us)
{
    MMP_UINT8 cmd[12];
    MMP_INT result = 0;

    LOG_DEBUG " CB_reset() called\n" LOG_END

    /** if the device was removed, then we're already reset */
    if(!us->pusb_dev)
        goto end;

    memset(cmd, 0xFF, sizeof(cmd));
    cmd[0] = SEND_DIAGNOSTIC;
    cmd[1] = 4;
    result = usb_control_msg(us->pusb_dev, 
                             usb_sndctrlpipe(us->pusb_dev,0),
                             US_CBI_ADSC, 
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                             0, 
                             us->ifnum, 
                             cmd, 
                             sizeof(cmd), 
                             1000);
    if(result < 0) 
    {
        LOG_ERROR " CB[I] soft reset failed %d\n", result LOG_END
        result = ERROR_MSC_CBI_SOFT_RESET_FAIL;
        goto end;
    }

    /** long wait for reset */
    //MMP_Sleep(2000);

    LOG_DEBUG " CB_reset: clearing endpoint halt\n" LOG_END
    usb_stor_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
    usb_stor_clear_halt(us->pusb_dev, usb_sndbulkpipe(us->pusb_dev, us->ep_out));
    LOG_DEBUG " CB_reset done\n" LOG_END

end:
    return result;
}

/**
 * Control/Bulk/Interrupt transport
 */

/* The interrupt handler for CBI devices */
void usb_stor_CBI_irq(struct urb *urb)
{
    /** TODO */
}

MMP_INT usb_stor_CBI_transport(Scsi_Cmnd *srb, struct us_data *us)
{
    /** TODO */
    return 0;
}

/**
 * Control/Bulk transport
 */
MMP_INT usb_stor_CB_transport(Scsi_Cmnd *srb, struct us_data *us)
{
    MMP_INT result = 0;

    //============ COMMAND STAGE ==============
    /** send the command via the control pipe */
    result = usb_control_msg(us->pusb_dev, 
                             usb_sndctrlpipe(us->pusb_dev, 0),
                             US_CBI_ADSC, 
                             USB_TYPE_CLASS | USB_RECIP_INTERFACE, 
                             0, 
                             us->ifnum, 
                             srb->cmnd, 
                             0xC/*srb->cmd_len*/,
                             US_ADSC_TIMEOUT);

    /** check the return code for the command */
    LOG_DEBUG "Call to usb_stor_control_msg() returned %d\n", result LOG_END
    if(result < 0) 
    {
        /** if the command was aborted, indicate that */
        if(result == -ENOENT)
            return USB_STOR_TRANSPORT_ABORTED;

        /** a stall is a fatal condition from the device */
        if(result == -EPIPE) 
        {
            LOG_WARNING "-- Stall on control pipe. Clearing\n" LOG_END
            result = usb_stor_clear_halt(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev, 0));

            /** if the command was aborted, indicate that */
            if(result == -ENOENT)
                return USB_STOR_TRANSPORT_ABORTED;
            return USB_STOR_TRANSPORT_FAILED;
        }

        /** Uh oh... serious problem here */
        return USB_STOR_TRANSPORT_ERROR;
    }

    //============ DATA STAGE ==============
    /** transfer the data payload for this command, if one exists */
    if(srb->request_bufflen) 
    {
        result = srb->result = usb_stor_transfer(srb, us);
        if(result)
            LOG_ERROR "CB data stage result is 0x%x\n", result LOG_END

        /** report any errors */
        if(result == US_BULK_TRANSFER_ABORTED) 
            return USB_STOR_TRANSPORT_ABORTED;

        if(result == US_BULK_TRANSFER_FAILED)
            return USB_STOR_TRANSPORT_FAILED;
    }

    //============ STATUS STAGE ==============
    /** 
     * NOTE: CB does not have a status stage.
     */
    return USB_STOR_TRANSPORT_GOOD;
}

