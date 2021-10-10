/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

int iteUasInitialize(void* uas, uint8_t lun)
{
    int rc = 0;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)uas;

    _uas_func_enter;

    if(!devinfo || !devinfo->udev)
    {
        rc = ERROR_UAS_NODEV;
        goto end;
    }
    if(lun >= devinfo->lun_num)
    {
        rc = ERROR_UAS_INVALID_LUN;
        goto end;
    }

    rc = uscsi_inquiry(devinfo, lun);
    if(rc)
        goto end;
    
    rc = uscsi_read_capacity(devinfo, lun);
    if(rc)
        goto end;

end:
    check_result(rc);
    _uas_func_leave;
    return rc;
}

int iteUasTerminate(void* uas, uint8_t lun)
{
    int rc = 0;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)uas;

    _uas_func_enter;

    if(!devinfo || !devinfo->udev)
    {
        rc = ERROR_UAS_NODEV;
        goto end;
    }
    if(lun >= devinfo->lun_num)
    {
        rc = ERROR_UAS_INVALID_LUN;
        goto end;
    }

end:
    check_result(rc);
    _uas_func_leave;
    return rc;
}

int iteUasReadMultipleSector(void* uas, uint8_t lun, uint32_t sector, int count, void* data)
{
    int rc = 0;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)uas;

    _uas_func_enter;

    if(!devinfo || !devinfo->udev)
    {
        rc = ERROR_UAS_NODEV;
        goto end;
    }
    if(lun >= devinfo->lun_num)
    {
        rc = ERROR_UAS_INVALID_LUN;
        goto end;
    }
	if(sector > devinfo->scsi_device[lun].block_num)
	{
		rc = ERROR_UAS_INVALID_SECTOR;
		goto end;
	}

    rc = uscsi_read10(devinfo, lun, sector, count, data);
    if(rc)
        goto end;

end:
    if(rc)
        LOG_ERROR "%s(lun %d, sector %d, cnt %d, buf 0x%X) \n", __FUNCTION__, lun, sector, count, data LOG_END
    
    _uas_func_leave;
    return rc;
}

int iteUasWriteMultipleSector(void* uas, uint8_t lun, uint32_t sector, int count, void* data)
{
    int rc = 0;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)uas;

    _uas_func_enter;

    if(!devinfo || !devinfo->udev)
    {
        rc = ERROR_UAS_NODEV;
        goto end;
    }
    if(lun >= devinfo->lun_num)
    {
        rc = ERROR_UAS_INVALID_LUN;
        goto end;
    }
	if(sector > devinfo->scsi_device[lun].block_num)
	{
		rc = ERROR_UAS_INVALID_SECTOR;
		goto end;
	}

    rc = uscsi_write10(devinfo, lun, sector, count, data);
    if(rc)
        goto end;

end:
    if(rc)
        LOG_ERROR "%s(lun %d, sector %d, cnt %d, buf 0x%X) \n", __FUNCTION__, lun, sector, count, data LOG_END

    _uas_func_leave;
    return rc;
}

int iteUasGetStatus(void* uas, uint8_t lun)
{
    int rc = 0;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)uas;

    if(!devinfo || !devinfo->udev)
    {
        rc = ERROR_UAS_NODEV;
        goto end;
    }
    if(lun >= devinfo->lun_num)
    {
        rc = ERROR_UAS_INVALID_LUN;
        goto end;
    }

    rc = uscsi_test_unit_ready(devinfo, lun);

    check_result(rc);
end:
    return rc;
} 

int iteUasGetCapacity(void* uas, uint8_t lun, uint32_t* sectorNum, uint32_t* blockLength)
{
    int rc = 0;
    struct uas_dev_info *devinfo = (struct uas_dev_info *)uas;

    _uas_func_enter;

    if(!devinfo || !devinfo->udev)
    {
        rc = ERROR_UAS_NODEV;
        goto end;
    }
    if(lun >= devinfo->lun_num)
    {
        rc = ERROR_UAS_INVALID_LUN;
        goto end;
    }

    (*sectorNum) = devinfo->scsi_device[lun].block_num;
    (*blockLength) = devinfo->scsi_device[lun].block_size;

    check_result(rc);
end:
    _uas_func_leave;
    return rc;
}  

bool iteUasIsLock(void* uas, uint8_t lun)
{
    _uas_func_enter;

    return false;
}  

int iteUasGetLunNum(void* uas)
{
    struct uas_dev_info *devinfo = (struct uas_dev_info *)uas;

    if(!devinfo || !devinfo->udev)
        return -1;

    return devinfo->lun_num;
}

