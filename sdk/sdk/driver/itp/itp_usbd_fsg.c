/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL USB USB Mass Storage device
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "itp_cfg.h"
#include "ite/ite_fsg.h"
#include "ite/itp.h"

#if defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE)
#include "ite/ite_sd.h"
#endif

#ifdef  CFG_NAND_ENABLE
#include "nand/mmp_nand.h"
#endif

#if (CFG_USB_DEVICE_DISKS == 23)
#include "ramdisk.c"
#endif

#if defined(CFG_UAS_ENABLE)
#include "ite/ite_usbex.h"
#endif


#define FSG_STACK_SIZE		8*1024
#define FSG_TASK_PRIORITY	3

static struct fsg_config cfg;
static const unsigned int diskTable[] = { CFG_USB_DEVICE_DISKS };
static uint8_t lunReady[FSG_MAX_LUNS];
static uint8_t lunEject[FSG_MAX_LUNS];
static void *usbCtxt;
static int usbType;
static int fsgConnected;

#if defined (CFG_NOR_ENABLE)
static uint32_t norPartitionOffset = 0;
#endif

/*-------------------------------------------------------------------------*/
static int card_init(uint8_t lun)
{
	int rc = 0;

	switch(diskTable[lun])
	{
#if defined(CFG_SD0_ENABLE)
	case ITP_DISK_SD0:
		rc = iteSdInitializeEx(SD_0);
		break;
#endif // CFG_SD0_ENABLE

#if defined(CFG_SD1_ENABLE)
	case ITP_DISK_SD1:
		rc = iteSdInitializeEx(SD_1);
		break;
#endif // CFG_SD1_ENABLE

#if defined(CFG_NAND_ENABLE)
	case ITP_DISK_NAND:
		break;
#endif // CFG_NAND_ENABLE

#if defined(CFG_NOR_ENABLE)
	case ITP_DISK_NOR:
		break;
#endif // CFG_NOR_ENABLE

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
	case ITP_DISK_MSC00:
	case ITP_DISK_MSC01:
	case ITP_DISK_MSC02:
	case ITP_DISK_MSC03:
	case ITP_DISK_MSC04:
	case ITP_DISK_MSC05:
	case ITP_DISK_MSC06:
	case ITP_DISK_MSC07:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasInitialize(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00));
        else
        #endif
		rc = iteMscInitialize(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00));
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
	case ITP_DISK_MSC10:
	case ITP_DISK_MSC11:
	case ITP_DISK_MSC12:
	case ITP_DISK_MSC13:
	case ITP_DISK_MSC14:
	case ITP_DISK_MSC15:
	case ITP_DISK_MSC16:
	case ITP_DISK_MSC17:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasInitialize(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10));
        else
        #endif
		rc = iteMscInitialize(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10));
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)

#if (CFG_USB_DEVICE_DISKS == 23)
	case ITP_DISK_RAM:
		RAMDISK_Initialize();
		break;
#endif

	default:
		rc = -1;
		break;
	}

	if(rc == 0)
	{
		lunReady[lun] = FSG_MEDIA_READY;
		lunEject[lun] = 0;
	}

	if(rc)
		LOG_ERR " %s(%d): rc=0x%X(%d) \n", __FUNCTION__, lun, rc, rc LOG_END
	return rc;
}

static void card_deinit(uint8_t lun)
{
	if(lunReady[lun] != FSG_MEDIA_READY)
		return;

	switch(diskTable[lun])
	{
#if defined(CFG_SD0_ENABLE)
	case ITP_DISK_SD0:
		iteSdTerminateEx(SD_0);
		break;
#endif // CFG_SD0_ENABLE

#if defined(CFG_SD1_ENABLE)
	case ITP_DISK_SD1:
		iteSdTerminateEx(SD_1);
		break;
#endif // CFG_SD1_ENABLE

#if defined(CFG_NAND_ENABLE)
	case ITP_DISK_NAND:
		break;
#endif // CFG_NAND_ENABLE

#if defined(CFG_NOR_ENABLE)
	case ITP_DISK_NOR:
		break;
#endif // CFG_NOR_ENABLE

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
	case ITP_DISK_MSC00:
	case ITP_DISK_MSC01:
	case ITP_DISK_MSC02:
	case ITP_DISK_MSC03:
	case ITP_DISK_MSC04:
	case ITP_DISK_MSC05:
	case ITP_DISK_MSC06:
	case ITP_DISK_MSC07:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            iteUasTerminate(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00));
        else
        #endif
		iteMscTerminate(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00));
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
	case ITP_DISK_MSC10:
	case ITP_DISK_MSC11:
	case ITP_DISK_MSC12:
	case ITP_DISK_MSC13:
	case ITP_DISK_MSC14:
	case ITP_DISK_MSC15:
	case ITP_DISK_MSC16:
	case ITP_DISK_MSC17:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            iteUasTerminate(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10));
        else
        #endif
		iteMscTerminate(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10));
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)

#if (CFG_USB_DEVICE_DISKS == 23)
	case ITP_DISK_RAM:
		RAMDISK_Terminate();
		break;
#endif

	default:
		break;
	}

	lunReady[lun] = FSG_MEDIA_NOT_PRESENT;
}

/*-------------------------------------------------------------------------*/
/** callback operations */

static int pfsg_open(void)
{
	memset(lunReady, 0x0, sizeof(lunReady));
	memset(lunEject, 0x0, sizeof(lunEject));
	usbCtxt = NULL;
    usbType = 0;
}

static int pfsg_close(void)
{
	memset(lunReady, 0x0, sizeof(lunReady));
	memset(lunEject, 0x0, sizeof(lunEject));
	usbCtxt = NULL;
    usbType = 0;
}

static void pfsg_eject(uint8_t lun)
{
	lunEject[lun] = 1;
}

static int pfsg_response(uint8_t lun)
{
	int ready = FSG_MEDIA_NOT_PRESENT;
	ITPCard card;

	switch(diskTable[lun])
	{
#if defined(CFG_SD0_ENABLE)
	case ITP_DISK_SD0:
		ready = ((bool)ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_SD0)==true) ? FSG_MEDIA_READY : FSG_MEDIA_NOT_PRESENT;
		break;
#endif // CFG_SD0_ENABLE

#if defined(CFG_SD1_ENABLE)
	case ITP_DISK_SD1:
		ready = ((bool)ioctl(ITP_DEVICE_CARD, ITP_IOCTL_IS_AVAIL, (void*)ITP_CARD_SD1)==true) ? FSG_MEDIA_READY : FSG_MEDIA_NOT_PRESENT;
		break;
#endif // CFG_SD1_ENABLE

#if defined(CFG_NAND_ENABLE)
	case ITP_DISK_NAND:
		ready = FSG_MEDIA_READY;
		break;
#endif // CFG_NAND_ENABLE

#if defined(CFG_NOR_ENABLE)
	case ITP_DISK_NOR:
		ready = FSG_MEDIA_READY;
		break;
#endif // CFG_NOR_ENABLE

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
	case ITP_DISK_MSC00:
	case ITP_DISK_MSC01:
	case ITP_DISK_MSC02:
	case ITP_DISK_MSC03:
	case ITP_DISK_MSC04:
	case ITP_DISK_MSC05:
	case ITP_DISK_MSC06:
	case ITP_DISK_MSC07:
		{
			ITPUsbInfo usbInfo = {0};
            usbInfo.host = true;
			ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, (void*)&usbInfo);
			usbCtxt = usbInfo.ctxt;
            usbType = usbInfo.type;
		}
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            ready = (iteUasGetStatus(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00))==0) ? FSG_MEDIA_READY : FSG_MEDIA_NOT_PRESENT;
        else
        #endif
		ready = (iteMscGetStatus(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00))==0) ? FSG_MEDIA_READY : FSG_MEDIA_NOT_PRESENT;
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
	case ITP_DISK_MSC10:
	case ITP_DISK_MSC11:
	case ITP_DISK_MSC12:
	case ITP_DISK_MSC13:
	case ITP_DISK_MSC14:
	case ITP_DISK_MSC15:
	case ITP_DISK_MSC16:
	case ITP_DISK_MSC17:
		{
			ITPUsbInfo usbInfo = {0};
            usbInfo.host = true;
			ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, (void*)&usbInfo);
			usbCtxt = usbInfo.ctxt;
            usbType = usbInfo.type;
		}
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            ready = (iteUasGetStatus(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10))==0) ? FSG_MEDIA_READY : FSG_MEDIA_NOT_PRESENT;
        else
        #endif
		ready = (iteMscGetStatus(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10))==0) ? FSG_MEDIA_READY : FSG_MEDIA_NOT_PRESENT;
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)

#if (CFG_USB_DEVICE_DISKS == 23)
	case ITP_DISK_RAM:
		ready = FSG_MEDIA_READY;
		break;
#endif

	default:
		break;
	}

	if(ready != lunReady[lun])
	{
		if(ready == FSG_MEDIA_READY)
		{
			if(card_init(lun))
			{
				card_deinit(lun);
				ready = FSG_MEDIA_NOT_PRESENT;
			}
			//else
			//	ready = FSG_MEDIA_CHANGE;
		}
		else
		{
			card_deinit(lun);
		}
	}

	if(lunEject[lun])
		ready = FSG_MEDIA_NOT_PRESENT;


	LOG_DBG " %s(%d): %d \n", __FUNCTION__, lun, ready LOG_END
	return ready;
}

static int pfsg_get_capacity(uint8_t lun, uint32_t *sector_num, uint32_t *block_size)
{
	ITPDisk disk = diskTable[lun];
	int rc = 0;

	if(lunReady[lun] != FSG_MEDIA_READY)
	{
		rc = -1;
		goto end;
	}

	(*sector_num) = 0;
	(*block_size) = 0;

	switch(disk)
	{
#ifdef CFG_SD0_ENABLE
	case ITP_DISK_SD0:
		rc = iteSdGetCapacityEx(SD_0, sector_num, block_size);
		break;
#endif // CFG_SD0_ENABLE

#ifdef CFG_SD1_ENABLE
	case ITP_DISK_SD1:
		rc = iteSdGetCapacityEx(SD_1, sector_num, block_size);
		break;
#endif // CFG_SD1_ENABLE

#if defined(CFG_NAND_ENABLE)
	case ITP_DISK_NAND:
	    rc = mmpNandGetCapacity(sector_num, block_size);
		break;
#endif // CFG_NAND_ENABLE

#if defined(CFG_NOR_ENABLE)
	case ITP_DISK_NOR:
		rc = nor_getCapacity(sector_num, block_size);
		break;
#endif // CFG_NOR_ENABLE

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
	case ITP_DISK_MSC00:
	case ITP_DISK_MSC01:
	case ITP_DISK_MSC02:
	case ITP_DISK_MSC03:
	case ITP_DISK_MSC04:
	case ITP_DISK_MSC05:
	case ITP_DISK_MSC06:
	case ITP_DISK_MSC07:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasGetCapacity(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00), sector_num, block_size);
        else
        #endif
		rc = iteMscGetCapacity(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00), sector_num, block_size);
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
	case ITP_DISK_MSC10:
	case ITP_DISK_MSC11:
	case ITP_DISK_MSC12:
	case ITP_DISK_MSC13:
	case ITP_DISK_MSC14:
	case ITP_DISK_MSC15:
	case ITP_DISK_MSC16:
	case ITP_DISK_MSC17:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasGetCapacity(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10), sector_num, block_size);
        else
        #endif
		rc = iteMscGetCapacity(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10), sector_num, block_size);
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)

#if (CFG_USB_DEVICE_DISKS == 23)
	case ITP_DISK_RAM:
		RAMDISK_GetCapacity((unsigned int*)sector_num, (unsigned int*)block_size);
		break;
#endif

	default:
		break;
	}

end:
	if(rc)
		LOG_ERR " %s(%d): rc=x%X (%d) \n", __FUNCTION__, lun, rc, rc LOG_END
	return rc;
}

static bool pfsg_readonly(uint8_t lun)
{
	ITPDisk disk = diskTable[lun];
	bool ro = false;

	switch(disk)
	{
#ifdef CFG_SD0_ENABLE
	case ITP_DISK_SD0:
		ro = iteSdIsLockEx(SD_0);
		break;
#endif // CFG_SD0_ENABLE

#ifdef CFG_SD1_ENABLE
	case ITP_DISK_SD1:
		ro = iteSdIsLockEx(SD_1);
		break;
#endif // CFG_SD1_ENABLE

#if defined(CFG_NAND_ENABLE)
	case ITP_DISK_NAND:
	    ro = false;
		break;
#endif // CFG_NAND_ENABLE

#if defined(CFG_NOR_ENABLE)
	case ITP_DISK_NOR:
		ro = false;
		break;
#endif // CFG_NOR_ENABLE

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
	case ITP_DISK_MSC00:
	case ITP_DISK_MSC01:
	case ITP_DISK_MSC02:
	case ITP_DISK_MSC03:
	case ITP_DISK_MSC04:
	case ITP_DISK_MSC05:
	case ITP_DISK_MSC06:
	case ITP_DISK_MSC07:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            ro = iteUasIsLock(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00));
        else
        #endif
		ro = iteMscIsLock(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00));
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
	case ITP_DISK_MSC10:
	case ITP_DISK_MSC11:
	case ITP_DISK_MSC12:
	case ITP_DISK_MSC13:
	case ITP_DISK_MSC14:
	case ITP_DISK_MSC15:
	case ITP_DISK_MSC16:
	case ITP_DISK_MSC17:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            ro = iteUasIsLock(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10));
        else
        #endif
		ro = iteMscIsLock(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10));
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)

#if (CFG_USB_DEVICE_DISKS == 23)
	case ITP_DISK_RAM:
		ro = false;
		break;
#endif

	default:
		break;
	}

	return ro;
}

static int pfsg_read_sector(uint8_t lun, uint32_t blockId, uint32_t sizeInSector, uint8_t *buf)
{
	int rc=0;

	if(lunReady[lun] != FSG_MEDIA_READY)
		return -1;

	switch(diskTable[lun])
	{
#if defined(CFG_SD0_ENABLE)
	case ITP_DISK_SD0:
		rc = iteSdReadMultipleSectorEx(SD_0, blockId, sizeInSector, buf);
		break;
#endif // CFG_SD0_ENABLE

#if defined(CFG_SD1_ENABLE)
	case ITP_DISK_SD1:
		rc = iteSdReadMultipleSectorEx(SD_1, blockId, sizeInSector, buf);
		break;
#endif // CFG_SD1_ENABLE

#if defined(CFG_NAND_ENABLE)
	case ITP_DISK_NAND:
	    rc = mmpNandReadSector(blockId, sizeInSector, buf);	    
		break;
#endif // CFG_NAND_ENABLE

#if defined(CFG_NOR_ENABLE)
	case ITP_DISK_NOR:
		blockId += norPartitionOffset;
		rc = nor_readmultiplesector(NULL, buf, (unsigned long)blockId, (int)sizeInSector);		
		break;
#endif // CFG_NOR_ENABLE

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
	case ITP_DISK_MSC00:
	case ITP_DISK_MSC01:
	case ITP_DISK_MSC02:
	case ITP_DISK_MSC03:
	case ITP_DISK_MSC04:
	case ITP_DISK_MSC05:
	case ITP_DISK_MSC06:
	case ITP_DISK_MSC07:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasReadMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00), blockId, sizeInSector, buf);
        else
        #endif
		rc = iteMscReadMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00), blockId, sizeInSector, buf);
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
	case ITP_DISK_MSC10:
	case ITP_DISK_MSC11:
	case ITP_DISK_MSC12:
	case ITP_DISK_MSC13:
	case ITP_DISK_MSC14:
	case ITP_DISK_MSC15:
	case ITP_DISK_MSC16:
	case ITP_DISK_MSC17:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasReadMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10), blockId, sizeInSector, buf);
        else
        #endif
		rc = iteMscReadMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10), blockId, sizeInSector, buf);
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)

#if (CFG_USB_DEVICE_DISKS == 23)
	case ITP_DISK_RAM:
		RAMDISK_ReadSector((unsigned int)blockId, (unsigned int)sizeInSector, (unsigned short*)buf);
		break;
#endif

	default:
		rc = -2;
		break;
	}

	LOG_DBG " %s(%d,%d,%d) rc=0x%X(%d) \n", __FUNCTION__, lun, blockId, sizeInSector, rc, rc LOG_END
	return rc;
}

static int pfsg_write_sector(uint8_t lun, uint32_t blockId, uint32_t sizeInSector, uint8_t *buf)
{
	int rc=0;

	if(lunReady[lun] != FSG_MEDIA_READY)
		return -1;

	switch(diskTable[lun])
	{
#if defined(CFG_SD0_ENABLE)
	case ITP_DISK_SD0:
		rc = iteSdWriteMultipleSectorEx(SD_0, blockId, sizeInSector, buf);
		break;
#endif // CFG_SD0_ENABLE

#if defined(CFG_SD1_ENABLE)
	case ITP_DISK_SD1:
		rc = iteSdWriteMultipleSectorEx(SD_1, blockId, sizeInSector, buf);
		break;
#endif // CFG_SD1_ENABLE

#if defined(CFG_NAND_ENABLE)
	case ITP_DISK_NAND:
	    rc = mmpNandWriteSector(blockId, sizeInSector, buf);
		break;
#endif // CFG_NAND_ENABLE

#if defined(CFG_NOR_ENABLE)
	case ITP_DISK_NOR:
	    blockId += norPartitionOffset;		
		rc = nor_writemultiplesector(NULL, buf, (unsigned long)blockId, (int)sizeInSector);
		break;
#endif // CFG_NOR_ENABLE

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)
	case ITP_DISK_MSC00:
	case ITP_DISK_MSC01:
	case ITP_DISK_MSC02:
	case ITP_DISK_MSC03:
	case ITP_DISK_MSC04:
	case ITP_DISK_MSC05:
	case ITP_DISK_MSC06:
	case ITP_DISK_MSC07:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasWriteMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00), blockId, sizeInSector, buf);
        else
        #endif
		rc = iteMscWriteMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC00), blockId, sizeInSector, buf);
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB1)

#if defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)
	case ITP_DISK_MSC10:
	case ITP_DISK_MSC11:
	case ITP_DISK_MSC12:
	case ITP_DISK_MSC13:
	case ITP_DISK_MSC14:
	case ITP_DISK_MSC15:
	case ITP_DISK_MSC16:
	case ITP_DISK_MSC17:
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(usbType))
            rc = iteUasWriteMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10), blockId, sizeInSector, buf);
        else
        #endif
		rc = iteMscWriteMultipleSector(usbCtxt, (diskTable[lun]-ITP_DISK_MSC10), blockId, sizeInSector, buf);
		break;
#endif // defined(CFG_MSC_ENABLE) && defined(CFG_USB_DEVICE_USB0)

#if (CFG_USB_DEVICE_DISKS == 23)
	case ITP_DISK_RAM:
		RAMDISK_WriteSector((unsigned int)blockId, (unsigned int)sizeInSector, (unsigned short*)buf);
		break;
#endif

	default:
		rc = -2;
		break;
	}

	LOG_DBG " %s(%d,%d,%d) rc=0x%X(%d) \n", __FUNCTION__, lun, blockId, sizeInSector, rc, rc LOG_END
	return rc;
}

static struct fsg_operations fsg_ops = {
	pfsg_open,
	pfsg_close,
	pfsg_response,
	pfsg_get_capacity,
	pfsg_readonly,
	pfsg_eject,
	pfsg_read_sector,
	pfsg_write_sector,
};

/*-------------------------------------------------------------------------*/


#define PRODUCT_REVISION	'1','.','0','0'

static const char inquiryTable[][8+16+4] = {
	{CFG_USBD_LUN_VENDOR_ID, CFG_USBD_LUN0_PRODUCT_ID, PRODUCT_REVISION},
	{CFG_USBD_LUN_VENDOR_ID, CFG_USBD_LUN1_PRODUCT_ID, PRODUCT_REVISION},
	{CFG_USBD_LUN_VENDOR_ID, CFG_USBD_LUN2_PRODUCT_ID, PRODUCT_REVISION},
	{CFG_USBD_LUN_VENDOR_ID, CFG_USBD_LUN3_PRODUCT_ID, PRODUCT_REVISION}
};


static int UsbdFsgGetDeviceOffset(void)
{
	ITPPartition disk;
	int ret = 0;
	int pid, i;
	char* str;

	str = CFG_USB_DEVICE_DRIVE;

	disk.disk = ITP_DISK_NOR;
	ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_GET_PARTITION, &disk);

	if (ret == 0)
	{
		if (strcmp(str, "A") == 0)
			norPartitionOffset = 0x3F;
		else if (strcmp(str, "B") == 0)
			norPartitionOffset = 0x3F + disk.size[0]/512;
		else if (strcmp(str, "C") == 0)
			norPartitionOffset = 0x3F + (disk.size[0]+ disk.size[1])/512;
		else if (strcmp(str, "D") == 0)
			norPartitionOffset = 0x3F +(disk.size[0]+ disk.size[1]+ disk.size[2])/512;
		else
		{
			printf("invalid disk driver\n");
			norPartitionOffset = 0x3F;
			return -1;
		}
		//printf("norPartitionOffset=0x%x\n", norPartitionOffset);
		return 0;
	}
	
}

static void UsbdFsgInit(void)
{
	int i;
	memset((void*)&cfg, 0x0, sizeof(struct fsg_config));

	cfg.nluns = ITH_COUNT_OF(diskTable);
	if(cfg.nluns >= 4)
		printf(" [ITP][FSG] luns >= 4 (%d) \n", cfg.nluns);

	for(i=0; i<cfg.nluns; i++)
	{
		cfg.luns[i].removable = 1;
		#if defined(CFG_SD0_STATIC)
		if(diskTable[i] == ITP_DISK_SD0)
			cfg.luns[i].removable = 0;
		#endif
		#if defined(CFG_SD1_STATIC)
		if(diskTable[i] == ITP_DISK_SD1)
			cfg.luns[i].removable = 0;
		#endif
		cfg.luns[i].inquiry_string = inquiryTable[i];
	}
	cfg.ops = &fsg_ops;

	cfg.manufacturer = CFG_USBD_STR_MANUFACTURER;
	cfg.product = CFG_USBD_STR_PRODUCT;
	cfg.serial = CFG_USBD_STR_SERIAL_NUMBER;

	cfg.vendor_id = CFG_USB_VENDOR_ID;
	cfg.product_id = CFG_USB_PRODUCT_ID;
	cfg.release_num = 0x0100;

	{
		/* create file storage thread */
		pthread_t task;
		pthread_attr_t attr;
		struct sched_param param;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, FSG_STACK_SIZE);
		param.sched_priority = FSG_TASK_PRIORITY;
		pthread_attr_setschedparam(&attr, &param);
		if(pthread_create(&task, &attr, fsg_main_thread, NULL))
			LOG_ERR " create fsg thread fail! \n" LOG_END
	}
}

static int UsbdFsgIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        UsbdFsgInit();
        break;
        
    case ITP_IOCTL_ENABLE:
        iteFsgInitialize(&cfg);
		if (CFG_USB_DEVICE_DISKS == ITP_DISK_NOR)
			UsbdFsgGetDeviceOffset();
        fsgConnected = 1;
        break;

    case ITP_IOCTL_DISABLE:
        iteFsgTerminate();
        fsgConnected = 0;
        break;

    case ITP_IOCTL_IS_CONNECTED:
        return fsgConnected;

    default:
        errno = (ITP_DEVICE_USBDFSG << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceUsbdFsg =
{
    ":usbd fsg",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    UsbdFsgIoctl,
    NULL
};

