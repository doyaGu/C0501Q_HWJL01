
/*-------------------------------------------------------------------------*/

static int do_inquiry(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
    uint8_t *buf = (uint8_t*)bh->buf;
    const struct fsg_lun_config *lun_cfg = &fsg->cfg->luns[fsg->lun];

    if(!fsg->curlun) /* Unsupported LUNs are okay */
    {
        fsg->bad_lun_okay = 1;
        memset(buf, 0, 36);
        buf[0] = 0x7F;  /* Unsupported, no device-type */
        buf[4] = 31;	/* Additional length */
        return 36;
    }

    memset(buf, 0, 8);	/* Non-removable, direct-access device */
    if(lun_cfg->removable)
        buf[1] = 0x80;
    buf[2] = 2;		/* ANSI SCSI level 2 */
    buf[3] = 2;		/* SCSI-2 INQUIRY data format */
    buf[4] = 31;	/* Additional length */
    buf[5] = 0;		/* No special options */
    buf[6] = 0;
    buf[7] = 0;
    memcpy(buf + 8, lun_cfg->inquiry_string, (8+16+4));

    return 36;
}

static int do_read_format_capacities(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
    struct lun *curlun = fsg->curlun;
    uint8_t *buf = (uint8_t *)bh->buf;
    uint32_t sector_num = 0;
    uint32_t block_size = 0;

    if(fsg->cfg->ops->get_capacity(fsg->lun, &sector_num, &block_size) != 0)
    {
        FSG_ERROR " get_capacity(%d) fail \n", fsg->lun FSG_END
        return -EINVAL;
    }

    curlun->num_sectors = sector_num;

    if(block_size != 512)
        FSG_WARNING " lun %d => block size %d != 512 \n", block_size FSG_END

    buf[0] = buf[1] = buf[2] = 0;
    buf[3] = 8;  /* Only the Current/Maximum Capacity Descriptor */
    buf += 4;

    put_be32(&buf[0], sector_num);  /* Number of blocks */
    put_be32(&buf[4], 512);         /* Block length */
    buf[4] = 0x02;					/* Current capacity */

    return 12;
}

static int do_request_sense(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
    struct lun *curlun = fsg->curlun;
    uint8_t *buf = (uint8_t *)bh->buf;
    uint32_t sd, sdinfo;
    uint8_t valid;

    if(!curlun) /* Unsupported LUNs are okay */
    {
        fsg->bad_lun_okay = 1;
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
        sdinfo = 0;
        valid = 0;
    }
    else
    {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
        valid = curlun->info_valid << 7;
        curlun->sense_data = SS_NO_SENSE;
        curlun->sense_data_info = 0;
        curlun->info_valid = 0;
    }

    memset(buf, 0, 18);
    buf[0] = valid | 0x70;		/* Valid, current error */
    buf[2] = SK(sd);
    put_be32(&buf[3], sdinfo);	/* Sense information */
    buf[7] = 18-8;				/* Additional sense length */
    buf[12] = ASC(sd);
    buf[13] = ASCQ(sd);

    return 18;
}

static int do_read_capacities(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
    struct lun *curlun = fsg->curlun;
    uint8_t *buf = (uint8_t *)bh->buf;
    uint32_t sector_num = 0;
    uint32_t block_size = 0;
    uint32_t lba = get_be32(&fsg->cmnd[2]);
    uint8_t pmi = fsg->cmnd[8];

    /* Check the PMI and LBA fields */
    if((pmi > 1) || (pmi==0 && lba!=0))
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        FSG_ERROR " do_read_capacities(%d) invalid CDB! \n", fsg->lun FSG_END
        return -EINVAL;
    }

    if(!curlun->num_sectors)
    {
        if(fsg->cfg->ops->get_capacity(fsg->lun, &sector_num, &block_size) != 0)
            return -EINVAL;

        curlun->num_sectors = sector_num;
        if(block_size != 512)
            FSG_WARNING " lun %d => block size %d != 512 \n", block_size FSG_END
    }

    put_be32(&buf[0], (curlun->num_sectors-1));  /* Max logical block */
    put_be32(&buf[4], 512);    /* Block length */

    return 8;
}

static int do_read(struct fsg_dev *fsg)
{
    struct lun *curlun = fsg->curlun;
    struct fsg_buffhd *bh;
    uint32_t lba = 0;
    uint32_t sector_left = 0;
    uint32_t sector_start, sector_cnt, timeout;
    int rc;

    /* Get the starting Logical Block Address and check that it's not too big */
    if(fsg->cmnd[0] == SC_READ_6)
        lba = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
    else
    {
        lba = get_be32(&fsg->cmnd[2]);

        /* We allow DPO (Disable Page Out = don't save data in the
         * cache) and FUA (Force Unit Access = don't read from the
         * cache), but we don't implement them. */
        if((fsg->cmnd[1] & ~0x18) != 0) 
        {
            curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            return -EINVAL;
        }
    }

    if(lba >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        return -EINVAL;
    }

    sector_left = fsg->data_size_from_cmnd >> 9;
    if(sector_left==0)
        return -ERR_FSG_READ_ZERO_SIZE;  // No default reply

    sector_start = lba;
    for(;;)
    {
        sector_cnt = min(sector_left, (FSG_BUFLEN >> 9));
        sector_cnt = min(sector_cnt, (curlun->num_sectors - sector_start));

        /* Wait for the next buffer to become available */
        bh = fsg->next_buffhd_to_fill;
        timeout = 5000*20; // wait 5 seconds
        while(bh->state != BUF_STATE_EMPTY)
        {
            FSG_DBG " wait buffer for read command...\n" FSG_END
            if(exception_in_progress(fsg))
                return -ERR_FSG_EXCEPTION;

            //usleep(1000);
            ithDelay(50); // 1000/20
            if(--timeout == 0)
                return -ERR_FSG_SCSI_NO_BUF;

            if(!(timeout % (1000*20)))
                ithPrintf("@\n");
        }

        /* If we were asked to read past the end of file, end with an empty buffer. */
        if(sector_cnt == 0)
        {
            curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
            curlun->sense_data_info = sector_start;
            curlun->info_valid = 1;
            bh->inreq->length = 0;
            bh->state = BUF_STATE_FULL;
            break;
        }

        /* Perform the read */
        rc = fsg->cfg->ops->read_sector(fsg->lun, sector_start, sector_cnt, bh->buf);
        if(rc)
        {
            /* If an error occurred, report it and its position */
            FSG_ERROR " read_sector(%d, %d, %d, ) rc = 0x%X (%d) \n", fsg->lun, sector_start, sector_cnt, rc, rc FSG_END
            curlun->sense_data = SS_UNRECOVERED_READ_ERROR;
            curlun->sense_data_info = sector_start;
            curlun->info_valid = 1;
            bh->inreq->length = 0;
            bh->state = BUF_STATE_FULL;
            break;
        }

        sector_start += sector_cnt;
        sector_left  -= sector_cnt;
        fsg->residue -= (sector_cnt << 9);
        bh->inreq->length = sector_cnt << 9;
        bh->state = BUF_STATE_FULL;

        /* No more left to read */
        if(sector_left == 0)
            break; 

        /* Send this buffer and go read some more */
        bh->inreq->zero = 0;
        start_transfer(fsg, fsg->bulk_in, bh->inreq, &bh->inreq_busy, &bh->state);
        fsg->next_buffhd_to_fill = bh->next;
    }

    return -1;  // No default reply
} 

static int do_mode_sense(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
    struct lun *curlun = fsg->curlun;
    uint8_t mscmnd = fsg->cmnd[0];
    uint8_t *buf = (uint8_t *)bh->buf;
    uint8_t *buf0 = buf;
    int pc, page_code;
    int len = 0;

    if((fsg->cmnd[1] & ~0x08) != 0) /* Mask away DBD */
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        FSG_ERROR " do_mode_sense(%d) invalid CDB! \n", fsg->lun FSG_END
        return -EINVAL;
    }

    pc = fsg->cmnd[2] >> 6;
    page_code = fsg->cmnd[2] & 0x3F;
    if(pc != 0)
    {
        curlun->sense_data = SS_SAVING_PARAMETERS_NOT_SUPPORTED;
        FSG_ERROR " do_mode_sense(%d) pc=%d ! \n", fsg->lun, pc FSG_END
        return -EINVAL;
    }

    curlun->ro = (fsg->cfg->ops->ro(fsg->lun) == true) ? 1 : 0;

    /* Write the mode parameter header.  Fixed values are: default
     * medium type, no cache control (DPOFUA), and no block descriptors.
     * The only variable value is the WriteProtect bit.  We will fill in
     * the mode data length later. */
    memset(buf, 0, 8);
    if(mscmnd == SC_MODE_SENSE_6)
    {
        buf[2] = (curlun->ro ? 0x80 : 0x00); /* WP, DPOFUA */
        buf += 4;
    }
    else /* MODE_SENSE_10 */
    {
        buf[3] = (curlun->ro ? 0x80 : 0x00); /* WP, DPOFUA */
        buf += 8;
    }

    /* No block descriptors */

    len = buf - buf0;
    if(mscmnd == SC_MODE_SENSE_6)
        buf0[0] = len -1;
    else
        put_be16(buf0, len-2);

    return len;
}


static int do_write(struct fsg_dev *fsg)
{
    struct lun *curlun = fsg->curlun;
    struct fsg_buffhd *bh;
    uint32_t lba = 0;
    uint32_t sector_left_to_req, sector_left_to_write;
    uint32_t sector_start_to_req, sector_start_to_write, sector_cnt, timeout;
    int rc;
    int get_some_more;

    if(curlun->ro)
    {
        curlun->sense_data = SS_WRITE_PROTECTED;
        return -EINVAL;
    }

    /* Get the starting Logical Block Address and check that it's not too big */
    if(fsg->cmnd[0] == SC_WRITE_6)
        lba = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
    else
    {
        lba = get_be32(&fsg->cmnd[2]);

        /* We allow DPO (Disable Page Out = don't save data in the
         * cache) and FUA (Force Unit Access = don't read from the
         * cache), but we don't implement them. */
        if((fsg->cmnd[1] & ~0x18) != 0) 
        {
            curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            return -EINVAL;
        }
    }

    if(lba >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        return -EINVAL;
    }

    get_some_more = 1;
    sector_left_to_req = sector_left_to_write = fsg->data_size_from_cmnd >> 9;
    sector_start_to_req = sector_start_to_write = lba;

    while(sector_left_to_write > 0)
    {
        /* Queue a request for more data from the host */
        bh = fsg->next_buffhd_to_fill;
        if((bh->state == BUF_STATE_EMPTY) && get_some_more)
        {
            sector_cnt = min(sector_left_to_req, (FSG_BUFLEN >> 9));
            sector_cnt = min(sector_cnt, (curlun->num_sectors - sector_start_to_req));
            if(sector_cnt == 0)
            {
                get_some_more = 0;
                curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
                curlun->sense_data_info = sector_start_to_req;
                curlun->info_valid = 1;
                continue;
            }

            sector_start_to_req += sector_cnt;
            sector_left_to_req -= sector_cnt;
            fsg->usb_amount_left -= (sector_cnt << 9);
            if(sector_left_to_req == 0)
                get_some_more = 0;

            bh->outreq->length = (sector_cnt << 9);
            start_transfer(fsg, fsg->bulk_out, bh->outreq, &bh->outreq_busy, &bh->state);
            fsg->next_buffhd_to_fill = bh->next;
            continue;
        }

        /* Write the received data to the backing storage */
        bh = fsg->next_buffhd_to_drain;
        if((bh->state == BUF_STATE_EMPTY) && !get_some_more)
        {
            ithPrintf(" stop early \n");
            break; // stop early
        }
        if(bh->state == BUF_STATE_FULL)
        {
            fsg->next_buffhd_to_drain = bh->next;
            bh->state = BUF_STATE_EMPTY;

            /* Did something go wrong with the transfer? */
            if(bh->outreq->status != 0)
            {
                curlun->sense_data = SS_COMMUNICATION_FAILURE;
                curlun->sense_data_info = sector_start_to_write;
                curlun->info_valid = 1;
                break;
            }
            /* Perform the write */
            sector_cnt = bh->outreq->actual >> 9;
            rc = fsg->cfg->ops->write_sector(fsg->lun, sector_start_to_write, sector_cnt, bh->buf);
            FSG_DBG " write_sector(%d, %d, %d, ) rc = 0x%X (%d) \n", fsg->lun, sector_start_to_write, sector_cnt, rc, rc FSG_END
            if(rc)
            {
                FSG_ERROR " write_sector(%d, %d, %d, ) rc = 0x%X (%d) \n", fsg->lun, sector_start_to_write, sector_cnt, rc, rc FSG_END
                curlun->sense_data = SS_WRITE_ERROR;
                curlun->sense_data_info = sector_start_to_write;
                curlun->info_valid = 1;
                break;
            }
            sector_start_to_write += sector_cnt;
            sector_left_to_write -= sector_cnt;
            fsg->residue -= (sector_cnt << 9);

            /* Did the host decide to stop early? */
            if(bh->outreq->actual < bh->outreq->length)
            {
                FSG_ERROR " do_write() => receive %d/%d \n", bh->outreq->actual, bh->outreq->length FSG_END
                fsg->short_packet_received = 1;
                break;
            }
            continue;
        }
        //ithPrintf("#\n");
        if(exception_in_progress(fsg))
            return -ERR_FSG_EXCEPTION;

        //usleep(1000);
    }

    return -1;  // No default reply
} 

static int do_prevent_allow(struct fsg_dev *fsg)
{
    struct lun *curlun = fsg->curlun;
    int prevent;

    if(!fsg->cfg->luns[fsg->lun].removable)
    {
        curlun->sense_data = SS_INVALID_COMMAND;
        return -EINVAL;
    }

    prevent = fsg->cmnd[4] & 0x1;
    if((fsg->cmnd[4] & ~0x01) != 0) /* Mask away Prevent */
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -EINVAL;
    }
    curlun->prevent_medium_removal = prevent;
    FSG_INFO "(%d) prevent:%d \n", fsg->lun, prevent FSG_END

    return 0;
}

static int do_start_stop(struct fsg_dev *fsg)
{
    struct lun *curlun = fsg->curlun;
    int loej, start;

    if(!curlun)
        return -EINVAL;

    if(!fsg->cfg->luns[fsg->lun].removable)
    {
        curlun->sense_data = SS_INVALID_COMMAND;
        return -EINVAL;
    }

    if(((fsg->cmnd[1] & ~0x01) != 0) || /* Mask away Immed */
       ((fsg->cmnd[4] & ~0x03) != 0))   /* Mask LoEj, Start */
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -EINVAL;
    }

    loej = fsg->cmnd[4] & 0x2;
    start = fsg->cmnd[4] & 0x1;
    FSG_INFO "(%d) loej:x%x, start:%d \n", fsg->lun, loej, start FSG_END

    if(!loej)
        return 0;

    fsg->cfg->ops->eject(fsg->lun);

    return 0;
}

static int do_mode_select(struct fsg_dev *fsg, struct fsg_buffhd *bh)
{
    struct lun *curlun = fsg->curlun;

    /* We don't support MODE SELECT */
    curlun->sense_data = SS_INVALID_COMMAND;
    return -EINVAL;
}

static int do_verify(struct fsg_dev *fsg)
{
    struct lun *curlun = fsg->curlun;
    uint32_t lba,  verification_length;

    /* Get the starting Logical Block Address and check that it's
     * not too big */
    lba = get_be32(&fsg->cmnd[2]);
    if(lba >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        return -EINVAL;
    }
    if((fsg->cmnd[1] & ~0x10) != 0)
    {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -EINVAL;
    }

    verification_length = get_be16(&fsg->cmnd[7]);
    if(verification_length == 0)
        return -EIO;		/* No default reply */

    if((lba+verification_length) >= curlun->num_sectors)
    {
        curlun->sense_data = SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE;
        return -EINVAL;
    }

    return 0;
}

