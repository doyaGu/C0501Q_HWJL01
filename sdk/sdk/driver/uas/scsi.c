/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *
 * @author Irene Lin
 */
#include "config.h"

#define QUEUE_CMND      1
#define CMD_TIMEOUT     12000


static inline uint8_t *put16(uint8_t *cp, uint16_t x)
{
    *cp++ = x >> 8;
    *cp++ = x;
    
    return cp;
} 

static inline uint8_t *put32(uint8_t *cp, uint32_t x)
{
    *cp++ = x >> 24;
    *cp++ = x >> 16;
    *cp++ = x >> 8;
    *cp++ = x;
    
    return cp;
} 

static inline uint32_t get32(uint8_t *a)
{
    uint32_t val;

    val = *a++;
    val <<= 8;
    val |= *a++;
    val <<= 8;
    val |= *a++;
    val <<= 8;
    val |= *a;

    return val;
}



static uint32_t scsi_get_tag(struct uas_dev_info *devinfo)
{
    int i, tmp;

    spin_lock(&devinfo->lock);

    tmp = devinfo->cmd_list_tag;

    for(i=0; i<SCSI_CMDQ_DEPTH; i++)
    {
        if(!((tmp >> i) & 0x1))
            break;
    }
    devinfo->cmd_list_tag |= (0x1 << i);

    spin_unlock(&devinfo->lock);

    if(i == SCSI_CMDQ_DEPTH)
    {
        LOG_ERROR " scsi_get_tag() fail!! \n" LOG_END
        i = -1;
    }

    LOG_INFO " get_tag: %d \n", i LOG_END
    return i;
}

static void scsi_put_tag(struct uas_dev_info *devinfo, uint32_t tag)
{
    _uas_func_enter;
    spin_lock(&devinfo->lock);

    //LOG_INFO " put_tag: %d \n", tag LOG_END

    devinfo->cmd_list_tag &= ~(0x1 << tag);

    spin_unlock(&devinfo->lock);
    _uas_func_leave;
}

static void scsi_done(struct scsi_cmnd* cmnd)
{
    LOG_DEBUG " scsi_done: cmd 0x%X \n", cmnd->cmnd[0] LOG_END

    if(cmnd->result && cmnd->sense_buffer)
    {
        LOG_ERROR " response code: 0x%X (need equal 0x70) \n", (cmnd->sense_buffer[0] & 0x7F) LOG_END
        LOG_ERROR "     sense key: 0x%X \n", (cmnd->sense_buffer[2] & 0x0F) LOG_END
        LOG_ERROR "           asc: 0x%X \n", cmnd->sense_buffer[12] LOG_END
        LOG_ERROR "          ascq: 0x%X \n", cmnd->sense_buffer[13] LOG_END
    }
    sem_post(&cmnd->sem);
}

int uscsi_init(struct uas_dev_info *devinfo)
{
    int i;
    struct scsi_cmnd *cmnd;

    for(i=0; i<SCSI_CMDQ_DEPTH; i++)
    {
        cmnd = (struct scsi_cmnd *)malloc(sizeof(struct scsi_cmnd));
        if(!cmnd)
            goto err;

        memset((void*)cmnd, 0x0, sizeof(struct scsi_cmnd));
        cmnd->tagged = QUEUE_CMND;
		cmnd->tag = -1;
        cmnd->devinfo = devinfo;
        cmnd->scsi_done = scsi_done;
        sem_init(&cmnd->sem, 0, 0);

        devinfo->cmd_list[i] = cmnd;
    }

    return 0;

err:
    uscsi_release(devinfo);
    return -1;
}

void uscsi_release(struct uas_dev_info *devinfo)
{
    int i;
    struct scsi_cmnd *cmnd;

    for(i=0; i<SCSI_CMDQ_DEPTH; i++)
    {
        cmnd = devinfo->cmd_list[i];
        if(cmnd)
        {
            sem_destroy(&cmnd->sem);
            free((void*)cmnd);
            devinfo->cmd_list[i] = NULL;
        }
    }
}

static struct scsi_cmnd* scsi_cmnd_alloc_init(struct uas_dev_info *devinfo, int lun,
                                              uint8_t cmd, uint8_t cmd_len, uint8_t dir)
{
    int index;
    struct scsi_cmnd *cmnd = NULL;
    _uas_func_enter;

    LOG_INFO " cmd: 0x%X \n", cmd LOG_END

    index = scsi_get_tag(devinfo);
    if(index < 0)
        goto end;

    cmnd = devinfo->cmd_list[index];
    memset((void*)cmnd->cmnd, 0x0, MAX_COMMAND_SIZE);
    memset((void*)cmnd->cmdinfo, 0x0, 64);

    /** setup command */
    cmnd->cmnd[0] = cmd;
    cmnd->cmd_len = cmd_len;
    cmnd->sc_data_direction = dir;

    cmnd->lun = lun;
    cmnd->tag = index;

end:
    _uas_func_leave;
    return cmnd;
}

static void scsi_cmnd_free(struct scsi_cmnd *cmnd)
{
    if(!cmnd)
        return;

    _uas_func_enter;

    scsi_put_tag(cmnd->devinfo, cmnd->tag);
	cmnd->tag = -1;

    if(cmnd->sense_buffer)
    {
        free((void*)cmnd->sense_buffer);
        cmnd->sense_buffer = NULL;
    }

    if(cmnd->scsi_buffer)
    {
        free((void*)cmnd->scsi_buffer);
        cmnd->scsi_buffer = NULL;
    }

    _uas_func_leave;
}

static int eh_timeout_abort(struct uas_dev_info *devinfo, struct scsi_cmnd *cmnd)
{
    struct uas_cmd_info *cmdinfo = (struct uas_cmd_info*)cmnd->cmdinfo;
    int rc;

    LOG_ERROR " CMD: 0x%X timeout! state=0x%X => ABORT \n", cmnd->cmnd[0], cmdinfo->state LOG_END
    uas_log_cmd_state(cmnd, __func__);

    _spin_lock_irqsave(&devinfo->lock);
    cmdinfo->state |= COMMAND_ABORTED;
    _spin_unlock_irqrestore(&devinfo->lock);

    rc = uas_eh_task_mgmt(cmnd, "ABORT TASK", TMF_ABORT_TASK);

    check_result(rc);
    return rc;
}

/* first command after device discovery */
int uscsi_test_unit_ready(struct uas_dev_info *devinfo, int lun)
{
    int rc = 0;
    struct scsi_cmnd *cmnd = NULL;
    _uas_func_enter;

    cmnd = scsi_cmnd_alloc_init(devinfo, lun,
                                TEST_UNIT_READY, 
                                CB_LENGTH_TEST_UNIT_READY,
                                DMA_NONE);
    if(!cmnd)
    {
        rc = ERROR_UAS_ALLOC_SCSI_CMD;
        goto end;
    }

    rc = uas_queuecommand(cmnd);
    if(rc)
        goto end;

    rc = itpSemWaitTimeout(&cmnd->sem, CMD_TIMEOUT);
    if(rc)
    {
        rc = ERROR_UAS_CMD_TIMEOUT;
        timeout_err();
        goto end;
    }

    rc = cmnd->result;

end:
    scsi_cmnd_free(cmnd);
    check_result(rc);
    _uas_func_leave;
    return rc;
}

int uscsi_report_luns(struct uas_dev_info *devinfo)
{
    int rc = 0;
    struct scsi_cmnd *cmnd = NULL;
    int i;

    _uas_func_enter;

    cmnd = scsi_cmnd_alloc_init(devinfo, 0,
                                REPORT_LUNS, 
                                CB_LENGTH_REPORT_LUNS,
                                DMA_FROM_DEVICE);
    if(!cmnd)
    {
        rc = ERROR_UAS_ALLOC_SCSI_CMD;
        goto end;
    }
    cmnd->transfer_len = 16;
    put32(&cmnd->cmnd[6], cmnd->transfer_len);
    cmnd->scsi_buffer = (uint8_t*)itpVmemAlloc(cmnd->transfer_len);
    memset((void*)cmnd->scsi_buffer, 0x0, cmnd->transfer_len);
    cmnd->transfer_buffer = cmnd->scsi_buffer;

    rc = uas_queuecommand(cmnd);
    if(rc)
        goto end;

    rc = itpSemWaitTimeout(&cmnd->sem, CMD_TIMEOUT);
    if(rc)
    {
        rc = ERROR_UAS_CMD_TIMEOUT;
        timeout_err();
        goto end;
    }

    rc = cmnd->result;
    if(rc)
        goto end;

    devinfo->lun_num = get32(&cmnd->scsi_buffer[0])/8;
    LOG_INFO " LUN # = %d \n", devinfo->lun_num LOG_END
    if(devinfo->lun_num > 1)
    {
        LOG_ERROR " need larger transfer length!!! \n" LOG_END
        uas_todo();
        goto end;
    }
    if(devinfo->lun_num > UAS_MAX_LUN)
    {
        LOG_WARNING " lun_num > UAS_MAX_LUN \n" LOG_END
        devinfo->lun_num = UAS_MAX_LUN;
    }

    memcpy((void*)devinfo->scsi_lun, (void*)&cmnd->scsi_buffer[8], sizeof(struct scsi_lun)*devinfo->lun_num);

end:
    scsi_cmnd_free(cmnd);
    check_result(rc);
    _uas_func_leave;
    return rc;
}

int uscsi_inquiry(struct uas_dev_info *devinfo, int lun)
{
    int rc = 0;
    struct scsi_cmnd *cmnd = NULL;
    _uas_func_enter;

    cmnd = scsi_cmnd_alloc_init(devinfo, lun,
                                INQUIRY, 
                                CB_LENGTH_INQUIRY,
                                DMA_FROM_DEVICE);
    if(!cmnd)
    {
        rc = ERROR_UAS_ALLOC_SCSI_CMD;
        goto end;
    }
    cmnd->transfer_len = DATA_LENGTH_INQUIRY;
    cmnd->cmnd[4] = DATA_LENGTH_INQUIRY;
    cmnd->scsi_buffer = (uint8_t*)itpVmemAlloc(cmnd->transfer_len);
    memset((void*)cmnd->scsi_buffer, 0x0, cmnd->transfer_len);
    cmnd->transfer_buffer = cmnd->scsi_buffer;

    rc = uas_queuecommand(cmnd);
    if(rc)
        goto end;

    rc = itpSemWaitTimeout(&cmnd->sem, CMD_TIMEOUT);
    if(rc)
    {
        rc = ERROR_UAS_CMD_TIMEOUT;
        timeout_err();
        goto end;
    }

    if(cmnd->result==0)
    {
        struct scsi_device *dev = &devinfo->scsi_device[lun];
        uint8_t tmp1[8+1] = {0};
        uint8_t tmp2[16+1] = {0};
        uint8_t tmp3[4+1] = {0};

        dev->device_type = cmnd->scsi_buffer[0] & 0x1F;
        dev->removable = (cmnd->scsi_buffer[1] & 0x80) >> 7;
        memcpy((void*)tmp1, &cmnd->scsi_buffer[8], 8);
        memcpy((void*)tmp2, &cmnd->scsi_buffer[16], 16);
        memcpy((void*)tmp3, &cmnd->scsi_buffer[32], 4);

        LOG_DATA "\nLun: %d \n", lun LOG_END
        LOG_DATA " device type: %d, removable: %d \n", dev->device_type, dev->removable LOG_END
        LOG_DATA " Vendor information: %s \n", tmp1 LOG_END
        LOG_DATA " Product identification: %s \n", tmp2 LOG_END
        LOG_DATA " Product revision level: %s \n\n", tmp3 LOG_END

        if(dev->device_type != TYPE_DISK)
        {
            rc = ERROR_UAS_DEVICE_TYPE;
            goto end;
        }
    }
    else
        rc = cmnd->result;

end:
    scsi_cmnd_free(cmnd);
    check_result(rc);
    _uas_func_leave;
    return rc;
}

int uscsi_read_capacity(struct uas_dev_info *devinfo, int lun)
{
    int rc = 0;
    struct scsi_cmnd *cmnd = NULL;
    _uas_func_enter;

    cmnd = scsi_cmnd_alloc_init(devinfo, lun,
                                READ_CAPACITY, 
                                CB_LENGTH_READ_CAPACITY,
                                DMA_FROM_DEVICE);
    if(!cmnd)
    {
        rc = ERROR_UAS_ALLOC_SCSI_CMD;
        goto end;
    }
    cmnd->transfer_len = DATA_LENGTH_READ_CAPACITY;
    cmnd->scsi_buffer = (uint8_t*)itpVmemAlloc(cmnd->transfer_len);
    memset((void*)cmnd->scsi_buffer, 0x0, cmnd->transfer_len);
    cmnd->transfer_buffer = cmnd->scsi_buffer;

    rc = uas_queuecommand(cmnd);
    if(rc)
        goto end;

    rc = itpSemWaitTimeout(&cmnd->sem, CMD_TIMEOUT);
    if(rc)
    {
        rc = ERROR_UAS_CMD_TIMEOUT;
        timeout_err();
        goto end;
    }

    if(cmnd->result==0)
    {
        struct scsi_device *dev = &devinfo->scsi_device[lun];

        dev->block_num = get32(&cmnd->scsi_buffer[0]);
        dev->block_size = get32(&cmnd->scsi_buffer[4]);

        LOG_DATA "\nLun: %d \n", lun LOG_END
        LOG_DATA " Block Number: %d \n", dev->block_num LOG_END
        LOG_DATA " Block Size: %d \n\n", dev->block_size LOG_END
    }
    else
        rc = cmnd->result;

end:
    scsi_cmnd_free(cmnd);
    check_result(rc);
    _uas_func_leave;
    return rc;
}

static uint32_t gtime;

int uscsi_read10(struct uas_dev_info *devinfo, int lun, uint32_t sector, int count, void* data)
{
    int rc = 0;
    struct scsi_cmnd *cmnd = NULL;
    _uas_func_enter;
//ithPrintf("<%d> \n", sector);
    if((sector+count) >= devinfo->scsi_device[lun].block_num)
    {
        LOG_ERROR " (%d + %d = %d) >= %d \n", sector, count, devinfo->scsi_device[lun].block_num LOG_END
        rc = ERROR_UAS_OUT_OF_RANGE;
        goto end1;
    }

    cmnd = scsi_cmnd_alloc_init(devinfo, lun,
                                READ_10, 
                                CB_LENGTH_READ_10,
                                DMA_FROM_DEVICE);
    if(!cmnd)
    {
        rc = ERROR_UAS_ALLOC_SCSI_CMD;
        goto end;
    }

    put32(&cmnd->cmnd[2], sector);
    put16(&cmnd->cmnd[7], (uint16_t)count);
    cmnd->transfer_len = (uint32_t)count * devinfo->scsi_device[lun].block_size;
    cmnd->transfer_buffer = data;

    rc = uas_queuecommand(cmnd);
    if(rc)
        goto end;

    rc = itpSemWaitTimeout(&cmnd->sem, CMD_TIMEOUT);
    if(rc)
    {
        rc = ERROR_UAS_CMD_TIMEOUT;
        eh_timeout_abort(devinfo, cmnd);
        goto end;
    }

    rc = cmnd->result;

end:
    scsi_cmnd_free(cmnd);
end1:
    check_result(rc);
    _uas_func_leave;
    return rc;
}

int uscsi_write10(struct uas_dev_info *devinfo, int lun, uint32_t sector, int count, void* data)
{
    int rc = 0;
    struct scsi_cmnd *cmnd = NULL;
    _uas_func_enter;
//ithPrintf("[%d] \n", sector);

    if((sector+count) >= devinfo->scsi_device[lun].block_num)
    {
        LOG_ERROR " (%d + %d = %d) >= %d \n", sector, count, devinfo->scsi_device[lun].block_num LOG_END
        rc = ERROR_UAS_OUT_OF_RANGE;
        goto end1;
    }

    cmnd = scsi_cmnd_alloc_init(devinfo, lun,
                                WRITE_10, 
                                CB_LENGTH_WRITE_10,
                                DMA_TO_DEVICE);
    if(!cmnd)
    {
        rc = ERROR_UAS_ALLOC_SCSI_CMD;
        goto end;
    }

    put32(&cmnd->cmnd[2], sector);
    put16(&cmnd->cmnd[7], (uint16_t)count);
    cmnd->transfer_len = (uint32_t)count * devinfo->scsi_device[lun].block_size;
    cmnd->transfer_buffer = data;

    rc = uas_queuecommand(cmnd);
    if(rc)
        goto end;

    rc = itpSemWaitTimeout(&cmnd->sem, CMD_TIMEOUT);
    if(rc)
    {
        rc = ERROR_UAS_CMD_TIMEOUT;
        eh_timeout_abort(devinfo, cmnd);
        goto end;
    }

    rc = cmnd->result;

end:
    scsi_cmnd_free(cmnd);
end1:
    check_result(rc);
    _uas_func_leave;
    return rc;
}
