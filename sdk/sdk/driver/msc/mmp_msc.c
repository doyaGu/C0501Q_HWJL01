/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  CF extern API implementation.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "msc/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define US_RETRY_COUNT      3

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
static MMP_INT _checkCondition(MMP_INT res, Scsi_Cmnd* srb)
{
    if(res == CHECK_CONDITION)
    {
        if( ((srb->sense_buffer[2] & 0xf) == NOT_READY) &&
            (srb->sense_buffer[12] == LOGICAL_UNIT_NOT_READY) && 
            (srb->sense_buffer[13] == 0x01) )
            return NEEDS_RETRY;
    }
    return 0;
}

//=============================================================================
/**
 * USB Mass Storage lun Initialization.
 * @param us    the related device's context
 * @param lun   the lun will to be initailized
 */
//=============================================================================
MMP_INT mmpMscInitialize(void* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    struct us_data* ss = (struct us_data*)us;
    MMP_INT retries;

    if(!ss || !ss->pusb_dev)     
    {
        result = ERROR_MSC_NO_DEVICE;
        return result;
    }
    if(lun > ss->max_lun)
    {
        result = ERROR_MSC_INVALID_LUN;
        return result;
    }
    SYS_WaitSemaphore(ss->semaphore);

#if 0
    retries = US_RETRY_COUNT;
    do {
        result = scsi_test_unit_ready(ss, lun);
        if(!result)
            break;
        retries--;
    } while(retries);
#endif
    if(!ss->device[lun].inquiried)
    {
        result = scsi_inquiry(ss, lun);
        if(result)
            goto end;
    }

    retries = US_RETRY_COUNT;
    do {
        result = scsi_read_capacity(ss, lun);
        if(!result)
            break;
        retries--;
    } while(retries);

#if 0
    /**
     * Disk-type devices use MODE SENSE(6) if the protocol
     * (SubClass) is Transparent SCSI, otherwise they use
     * MODE SENSE(10). 
     */
    if(ss->device[lun].removable)
    {
        if(ss->use_mode_sense6)
            scsi_mode_sense6(ss, lun);
    }
#endif

end:
    SYS_ReleaseSemaphore(ss->semaphore);
    if(result)
        LOG_ERROR " mmpMscInitialize(%d) err 0x%08X \n", lun, result LOG_END
    return result;
}


//=============================================================================
/**
 * USB Mass Storage lun Terminate.
 * @param us    the related device's context
 * @param lun   the lun will to be terminated
 */
//=============================================================================
MMP_INT
mmpMscTerminate(void* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    struct us_data* ss = (struct us_data*)us;

    if(!ss || !ss->pusb_dev)     
    {
        result = ERROR_MSC_NO_DEVICE;
        goto end;
    }
    if(lun > ss->max_lun)
    {
        result = ERROR_MSC_INVALID_LUN;
        goto end;
    }

end:
    if(result)
        LOG_ERROR " mmpMscTerminate(%d) err 0x%08X \n", lun, result LOG_END
    return result;
}

//=============================================================================
/**
 * USB Mass Storage read multisector function.
 *
 * @param us    the related device's context
 * @param lun   the lun will to be processed
 * @param sector the sector start to read
 * @param count  the count will read
 * @param data   the buffer to receive the read data
 */
//=============================================================================
MMP_INT
mmpMscReadMultipleSector(void* us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void* data)
{
    MMP_INT result = 0;
    struct us_data* ss = (struct us_data*)us;
    MMP_INT retryCnt = 5;
    LOG_ENTER "[mmpMscReadMultipleSector] lun=%d, sector=%d, count=%d, dataAddr=0x%08X \n", lun, sector, count, data LOG_END

    if(!ss || !ss->pusb_dev)     
    {
        result = ERROR_MSC_NO_DEVICE;
        return result;
    }
    if(lun > ss->max_lun)
    {
        result = ERROR_MSC_INVALID_LUN;
        return result;
    }

    SYS_WaitSemaphore(ss->semaphore);
    if(!usb_dev_exist(ss->pusb_dev))
    {
        result = ERROR_MSC_DEVICE_NOT_EXIST;
        goto end;
    }
	if(sector > ss->device[lun].blockTotalNum)
    {
		result = ERROR_MSC_INVALID_SECTOR;
        goto end;
    }

retry:
    ss->in_rw_access = 1;
    result = scsi_read10(ss, lun, sector, count, data);
    ss->in_rw_access = 0;

    if(result && (_checkCondition(result, &ss->srb) == NEEDS_RETRY) && (retryCnt >= 0) )
    {
        retryCnt--;
        MMP_Sleep(3500);
        goto retry;
    }

end:
    SYS_ReleaseSemaphore(ss->semaphore);
    if(result)
        LOG_ERROR " mmpMscReadMultipleSector(%d) err 0x%08X \n", lun, result LOG_END
    return result;
}

//=============================================================================
/**
 * USB Mass Storage write multisector function.
 *
 * @param us    the related device's context
 * @param lun   the lun will to be processed
 * @param sector the sector start to write
 * @param count  the count will write
 * @param data   the buffer to transfer the data to be writed
 */
//=============================================================================
MMP_INT
mmpMscWriteMultipleSector(void* us, MMP_UINT8 lun, MMP_UINT32 sector, MMP_INT count, void* data)
{
    MMP_INT result = 0;
    struct us_data* ss = (struct us_data*)us;
    MMP_INT retryCnt = 5;
    LOG_ENTER "[mmpMscWriteMultipleSector] lun=%d, sector=%d, count=%d, dataAddr=0x%08X \n", lun, sector, count, data LOG_END

    if(!ss || !ss->pusb_dev)     
    {
        result = ERROR_MSC_NO_DEVICE;
        return result;
    }
    if(lun > ss->max_lun)
    {
        result = ERROR_MSC_INVALID_LUN;
        return result;
    }

    SYS_WaitSemaphore(ss->semaphore);
    if(!usb_dev_exist(ss->pusb_dev))
    {
        result = ERROR_MSC_DEVICE_NOT_EXIST;
        goto end;
    }
    if(ss->device[lun].writeProtect)
    {
        result = ERROR_MSC_WRITE_PROTECTED;
        goto end;
    }
	if(sector > ss->device[lun].blockTotalNum)
    {
		result = ERROR_MSC_INVALID_SECTOR;
        goto end;
    }
    
retry:
    ss->in_rw_access = 1;
    result = scsi_write10(ss, lun, sector, count, data);
    ss->in_rw_access = 0;

    if(result && (_checkCondition(result, &ss->srb) == NEEDS_RETRY) && (retryCnt >= 0) )
    {
        retryCnt--;
        MMP_Sleep(3500);
        goto retry;
    }

end:
    SYS_ReleaseSemaphore(ss->semaphore);
    if(result)
        LOG_ERROR " mmpMscWriteMultipleSector(%d) err 0x%08X \n", lun, result LOG_END
    return result;
}


//=============================================================================
/**
 * USB Mass Storage lun status API.
 */
//=============================================================================
MMP_INT
mmpMscGetStatus(void* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    struct us_data* ss = (struct us_data*)us;
    MMP_INT retryCnt = 0;
    Scsi_Device* device = MMP_NULL;

    if(!ss || !ss->pusb_dev)     
    {
        result = ERROR_MSC_NO_DEVICE;
        return result;
    }
    if(lun > ss->max_lun)
    {
        result = ERROR_MSC_INVALID_LUN;
        return result;
    }
    device = &ss->device[lun];
    if(device->inquiried && !device->removable)
        return result;

    SYS_WaitSemaphore(ss->semaphore);
    if(!usb_dev_exist(ss->pusb_dev))
    {
        result = ERROR_MSC_DEVICE_NOT_EXIST;
        goto end;
    }

retry:
    result = scsi_test_unit_ready(ss, lun);

    if(result && (_checkCondition(result, &ss->srb) == NEEDS_RETRY) && (retryCnt >= 0) )
    {
        retryCnt--;
        MMP_Sleep(3500);
        goto retry;
    }

end:
    SYS_ReleaseSemaphore(ss->semaphore);
    if(result)
        LOG_ERROR " mmpMscGetStatus(%d) err 0x%08X \n", lun, result LOG_END
    return result;
} 

/** Just for file system use........ for performance issue. */
MMP_INT
mmpMscGetStatus2(void* us, MMP_UINT8 lun)
{
    MMP_INT result = 0;
    struct us_data* ss = (struct us_data*)us;

    if(!ss || !ss->pusb_dev)     
    {
        result = ERROR_MSC_NO_DEVICE;
        return result;
    }
    if(lun > ss->max_lun)
    {
        result = ERROR_MSC_INVALID_LUN;
        return result;
    }

    SYS_WaitSemaphore(ss->semaphore);
    if(!usb_dev_exist(ss->pusb_dev))
    {
        result = ERROR_MSC_DEVICE_NOT_EXIST;
        goto end;
    }

end:
    SYS_ReleaseSemaphore(ss->semaphore);
    if(result)
        LOG_ERROR " mmpMscGetStatus(%d) err 0x%08X \n", lun, result LOG_END
    return result;
} 


//=============================================================================
/**
 * USB Mass Storage get lun capacity API.
 */
//=============================================================================
MMP_INT
mmpMscGetCapacity(void* us, MMP_UINT8 lun, MMP_UINT32* sectorNum, MMP_UINT32* blockLength)
{
    MMP_INT   result = 0;
    struct us_data* ss = (struct us_data*)us;

    if(!ss || !ss->pusb_dev)     
    {
        result = ERROR_MSC_NO_DEVICE;
        return result;
    }
    if(lun > ss->max_lun)
    {
        result = ERROR_MSC_INVALID_LUN;
        return result;
    }

    if(sectorNum)
        (*sectorNum) = ss->device[lun].blockTotalNum;
    if(blockLength)
        (*blockLength) = ss->device[lun].blockSize;

end:
    if(result)
        LOG_ERROR " mmpMscGetCapacity(%d) err 0x%08X \n", lun, result LOG_END
    return result;
}  

//=============================================================================
/**
 * USB Mass Storage lun Is in write-protected status?
 */
//=============================================================================
MMP_BOOL
mmpMscIsLock(void* us, MMP_UINT8 lun)
{
    struct us_data* ss = (struct us_data*)us;

    if(!ss || !ss->pusb_dev)     
        return MMP_FALSE;
    if(lun > ss->max_lun)
        return MMP_FALSE;

    return (ss->device[lun].writeProtect) ? MMP_TRUE : MMP_FALSE;
}  

/** Irene: 2011_0106
 * Is in read/write procedure? Just for application workaround. 
 */
MMP_BOOL
mmpMscInDataAccess(void* us)
{
    struct us_data* ss = (struct us_data*)us;

    if(!ss || !ss->pusb_dev)     
        return MMP_FALSE;

    if(ss->in_rw_access)
        return MMP_TRUE;

    return MMP_FALSE;
}


