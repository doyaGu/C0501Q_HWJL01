/*
 * Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
 */
/** @file scsi.c
 * scsi disk driver.
 *
 * @author Irene Lin
 */

#include "msc/config.h"

#define SET_SERIAL_NUM(x)   do{ if(++x==0) x=1; } while(0)

MMP_INT scsi_test_unit_ready(struct us_data* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    Scsi_Cmnd* srb = &us->srb;

    /** clear all buffer */
    memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
    memset(srb->scsi_result, 0, SCSI_MAX_RESULT_SIZE);

    /** setup command */
    srb->cmnd[0] = TEST_UNIT_READY;
    srb->cmd_len = CB_LENGTH_TEST_UNIT_READY;

    /** setup data buffer info */
    srb->sc_data_direction = SCSI_DATA_NONE;
    srb->request_buffer = MMP_NULL;
    srb->request_bufflen = 0;

    srb->lun= lun;
    SET_SERIAL_NUM(srb->serial_number);

    usb_stor_invoke_transport(srb, us);

    result = srb->result;

    if(result)
        LOG_ERROR " scsi_test_unit_ready() result = 0x%08X \n", result LOG_END
    return result;
}

MMP_INT scsi_inquiry(struct us_data* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    Scsi_Cmnd* srb = &us->srb;

    /** clear all buffer */
    memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
    memset(srb->scsi_result, 0, SCSI_MAX_RESULT_SIZE);

    /** setup command */
    srb->cmnd[0] = INQUIRY;
    srb->cmnd[4] = DATA_LENGTH_INQUIRY;
    srb->cmd_len = CB_LENGTH_INQUIRY;

    /** setup data buffer info */
    srb->sc_data_direction = SCSI_DATA_READ;
    srb->request_buffer = srb->scsi_result;
    srb->request_bufflen = DATA_LENGTH_INQUIRY;

    srb->lun= lun;
    SET_SERIAL_NUM(srb->serial_number);

    usb_stor_invoke_transport(srb, us);

    if(srb->result == GOOD)
    {
        Scsi_Device* device = &us->device[lun];

        device->deviceType = srb->request_buffer[0] & 0x1F;
        device->removable = (srb->request_buffer[1] & 0x80) >> 7;
        memcpy(device->vendorInfo, &srb->request_buffer[8], 8);
        memcpy(device->productId, &srb->request_buffer[16], 16);
        memcpy(device->productRev, &srb->request_buffer[32], 4);
        LOG_DATA "\nLun %d: \n", lun LOG_END
        LOG_DATA " Device type: %d, removable: %x \n", device->deviceType, device->removable LOG_END
        LOG_DATA " Vendor information: %s \n", device->vendorInfo LOG_END
        LOG_DATA " Product identification: %s \n", device->productId LOG_END
        LOG_DATA " Product revision level: %s \n", device->productRev LOG_END
        if(device->deviceType != TYPE_DISK)
        {
            result = ERROR_MSC_NOT_SUPPORT_DEVICE_TYPE;
            goto end;
        }
        device->inquiried = 1;
    }
    else
        result = srb->result;

end:
    if(result)
        LOG_ERROR " scsi_inquiry() result = 0x%08X \n", result LOG_END
    return result;
}

MMP_INT scsi_read_capacity(struct us_data* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    Scsi_Cmnd* srb = &us->srb;

    /** clear all buffer */
    memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
    memset(srb->scsi_result, 0, SCSI_MAX_RESULT_SIZE);

    /** setup command */
    srb->cmnd[0] = READ_CAPACITY;
    srb->cmd_len = CB_LENGTH_READ_CAPACITY;

    /** setup data buffer info */
    srb->sc_data_direction = SCSI_DATA_READ;
    srb->request_buffer = srb->scsi_result;
    srb->request_bufflen = DATA_LENGTH_READ_CAPACITY;

    srb->lun= lun;
    SET_SERIAL_NUM(srb->serial_number);

    usb_stor_invoke_transport(srb, us);

    if(srb->result == GOOD)
    {
        Scsi_Device* device = &us->device[lun];

        device->blockTotalNum = srb->scsi_result[0] << 24 |
                                srb->scsi_result[1] << 16 |
                                srb->scsi_result[2] << 8 |
                                srb->scsi_result[3];
        device->blockTotalNum += 1;
        device->blockSize = srb->scsi_result[4] << 24 |
                            srb->scsi_result[5] << 16 |
                            srb->scsi_result[6] << 8 |
                            srb->scsi_result[7];
        LOG_DATA "\nLun %d: \n", lun LOG_END
        LOG_DATA " blockTotalNum: %d \n", device->blockTotalNum LOG_END
        LOG_DATA " blockSize: %d \n", device->blockSize LOG_END
        if(device->blockSize == 0)
        {
            device->blockSize = 512;
            LOG_DATA " [WARNING]: device's block size is zero ==> set as default 512!!! \n\n" LOG_END
        }
    }
    else
        result = srb->result;

    if(result)
        LOG_ERROR " scsi_read_capacity() result = 0x%08X \n", result LOG_END
    return result;
}

MMP_INT scsi_mode_sense6(struct us_data* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    Scsi_Cmnd* srb = &us->srb;

    /** clear all buffer */
    memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
    memset(srb->scsi_result, 0, SCSI_MAX_RESULT_SIZE);

    /** setup command */
    srb->cmnd[0] = MODE_SENSE;
    srb->cmnd[2] = 0x3F; /* Get all pages */
    srb->cmd_len = CB_LENGTH_MODE_SENSE6;

    /** setup data buffer info */
    srb->sc_data_direction = SCSI_DATA_READ;
    srb->request_buffer = srb->scsi_result;
    srb->request_bufflen = DATA_LENGTH_MODE_SENSE;

    srb->lun= lun;
    SET_SERIAL_NUM(srb->serial_number);

    usb_stor_invoke_transport(srb, us);

    if(srb->result == GOOD)
    {
        Scsi_Device* device = &us->device[lun];
        device->writeProtect = (srb->request_buffer[2] & 0x80) >> 7;
        LOG_DATA "\nLun %d: write protected %d\n", lun, device->writeProtect LOG_END
    }
    else
        result = srb->result;

    if(result)
        LOG_ERROR " scsi_mode_sense6() result = 0x%08X \n", result LOG_END
    return result;
}

MMP_INT scsi_read10(struct us_data* us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void* data)
{
    MMP_INT result = 0;
    Scsi_Cmnd* srb = &us->srb;

    /** clear all buffer */
    memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
    memset(srb->scsi_result, 0, SCSI_MAX_RESULT_SIZE);

    /** setup command */
    srb->cmnd[0] = READ_10;

    srb->cmnd[2] = (MMP_UINT8)((sector & 0xFF000000) >> 24);
    srb->cmnd[3] = (MMP_UINT8)((sector & 0x00FF0000) >> 16);
    srb->cmnd[4] = (MMP_UINT8)((sector & 0x0000FF00) >> 8);
    srb->cmnd[5] = (MMP_UINT8)(sector & 0xFF);

    srb->cmnd[7] = (MMP_UINT8)((count & 0xFF00) >> 8);
    srb->cmnd[8] = (MMP_UINT8)(count & 0x00FF);

    srb->cmd_len = CB_LENGTH_READ_10;

    /** setup data buffer info */
    srb->sc_data_direction = SCSI_DATA_READ;
    srb->request_buffer = data;
    srb->request_bufflen = (MMP_UINT32)count * us->device[lun].blockSize;

    srb->lun= lun;
    SET_SERIAL_NUM(srb->serial_number);

    usb_stor_invoke_transport(srb, us);

    result = srb->result;

    if(result)
        LOG_ERROR " scsi_read10() result = 0x%08X \n", result LOG_END
    return result;
}

MMP_INT scsi_write10(struct us_data* us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void* data)
{
    MMP_INT result = 0;
    Scsi_Cmnd* srb = &us->srb;

    /** clear all buffer */
    memset(srb->cmnd, 0, MAX_COMMAND_SIZE);
    memset(srb->scsi_result, 0, SCSI_MAX_RESULT_SIZE);

    /** setup command */
    srb->cmnd[0] = WRITE_10;

    srb->cmnd[2] = (MMP_UINT8)((sector & 0xFF000000) >> 24);
    srb->cmnd[3] = (MMP_UINT8)((sector & 0x00FF0000) >> 16);
    srb->cmnd[4] = (MMP_UINT8)((sector & 0x0000FF00) >> 8);
    srb->cmnd[5] = (MMP_UINT8)(sector & 0xFF);

    srb->cmnd[7] = (MMP_UINT8)((count & 0xFF00) >> 8);
    srb->cmnd[8] = (MMP_UINT8)(count & 0x00FF);

    srb->cmd_len = CB_LENGTH_READ_10;

    /** setup data buffer info */
    srb->sc_data_direction = SCSI_DATA_WRITE;
    srb->request_buffer = data;
    srb->request_bufflen = (MMP_UINT32)count * us->device[lun].blockSize;

    srb->lun= lun;
    SET_SERIAL_NUM(srb->serial_number);

    usb_stor_invoke_transport(srb, us);

    result = srb->result;

    if(result)
        LOG_ERROR " scsi_read10() result = 0x%08X \n", result LOG_END
    return result;
}
