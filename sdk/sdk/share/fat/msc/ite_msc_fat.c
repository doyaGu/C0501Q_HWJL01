#include "mscfat.h"
#include "ite/itp.h"
#include "ite/ite_msc.h"
#if defined(CFG_UAS_ENABLE)
#include "ite/ite_usbex.h"
#include "ite/ite_uas.h"
#endif

struct msc_param
{
    int mscIndex;   // 8*usbIndex + mscLun
    int usbIndex;   // usb0 or usb1
    int mscLun;     // lun number
    void* ctxt;     // context
    int type;       // MSC or UAS
};

#define LUN_NUM     16  /* 2(usb) * 8(lun) */

static int g_initok[LUN_NUM] = {0};
static F_DRIVER g_drivers[LUN_NUM];


/****************************************************************************
 *
 * msc_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

static int msc_readsector(F_DRIVER *driver,void *data,unsigned long sector) 
{
    int rc;
    struct msc_param* mscParam = (struct msc_param*)driver->user_data;

    if (!g_initok[mscParam->mscIndex]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    #if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(mscParam->type))
        rc = iteUasReadMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, 1, data);
    else
    #endif
    rc = iteMscReadMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, 1, data);
    if (rc)
    {
        return F_ERR_READ;
    }
    else
    {
        return F_NO_ERROR;
    }
}

/****************************************************************************
 *
 * msc_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

static int msc_writesector(F_DRIVER *driver,void *data,unsigned long sector) 
{
    int rc;
    struct msc_param* mscParam = (struct msc_param*)driver->user_data;

    if (!g_initok[mscParam->mscIndex]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    #if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(mscParam->type))
        rc = iteUasWriteMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, 1, data);
    else
    #endif
    rc = iteMscWriteMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, 1, data);
    if (rc)
    {
        return F_ERR_WRITE;
    }
    else
    {
        return F_NO_ERROR;
    }
}

/****************************************************************************
 *
 * msc_readsector
 *
 * read one sector from the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where to store data
 * sector - which sector is needed
 * cnt - number of sectors to read 
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

static int msc_readmultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    int rc;
    struct msc_param* mscParam = (struct msc_param*)driver->user_data;

    if (!g_initok[mscParam->mscIndex]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    #if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(mscParam->type))
        rc = iteUasReadMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, cnt, data);
    else
    #endif
    rc = iteMscReadMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, cnt, data);
    if (rc)
    {
        return F_ERR_READ;
    }
    else
    {
        return F_NO_ERROR;
    }
}

/****************************************************************************
 *
 * msc_writesector
 *
 * write one sector into the card
 *
 * INPUTS
 *
 * driver - driver structure
 * data - pointer where original data is
 * sector - which sector needs to be written
 * cnt - number of sectors to write
 *
 * RETURNS
 *
 * 0 - if successful
 * other if any error
 *
 ***************************************************************************/

static int msc_writemultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    int rc;
    struct msc_param* mscParam = (struct msc_param*)driver->user_data;

    if (!g_initok[mscParam->mscIndex]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    #if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(mscParam->type))
        rc = iteUasWriteMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, cnt, data);
    else
    #endif
    rc = iteMscWriteMultipleSector(mscParam->ctxt, mscParam->mscLun, sector, cnt, data);
    if (rc)
    {
        return F_ERR_WRITE;
    }
    else
    {
        return F_NO_ERROR;
    }
}

/****************************************************************************
 *
 * msc_getstatus
 *
 * get status of card, missing or/and removed,changed,writeprotect
 *
 * INPUTS
 *
 * driver - driver structure
 *
 * RETURNS
 *
 * F_ST_xxx code for high level
 *
 ***************************************************************************/

static long msc_getstatus(F_DRIVER *driver) 
{
    struct msc_param* mscParam = (struct msc_param*)driver->user_data;
    int state=0, rc=0;

    #if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(mscParam->type))
        rc = iteUasGetStatus(mscParam->ctxt, mscParam->mscLun);
    else
    #endif
    rc = iteMscGetStatus(mscParam->ctxt, mscParam->mscLun);
    if (rc)
    {
        g_initok[mscParam->mscIndex] = 0;
        state |= F_ST_MISSING;
    }
    else  // zero for success
    {
        if (g_initok[mscParam->mscIndex]==0) 
        {
            state |= F_ST_CHANGED;
            g_initok[mscParam->mscIndex] = 1;
        }
    }

    return state; 
}

/****************************************************************************
 *
 * msc_getphy
 *
 * determinate flash card physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

static int msc_getphy(F_DRIVER *driver, F_PHY *phy) 
{
    struct msc_param* mscParam = (struct msc_param*)driver->user_data;
    int res = 0;
    uint32_t sectors=0, blockSize=0;
    
    if (g_initok[mscParam->mscIndex])
    {
        #if defined(CFG_UAS_ENABLE)
        if(USB_DEVICE_UAS(mscParam->type))
            res = iteUasGetCapacity(mscParam->ctxt, mscParam->mscLun, &sectors, &blockSize);
        else
        #endif
        res = iteMscGetCapacity(mscParam->ctxt, mscParam->mscLun, &sectors, &blockSize);
	ithPrintf("msc_getphy: %d, %d \n", sectors, blockSize);
        if(res)
            return res;
        phy->number_of_cylinders = 0;
        phy->sector_per_track = 63;
        phy->number_of_heads = 255;
        phy->number_of_sectors = sectors;
        phy->media_descriptor = 0xf0; /* removable */
        phy->bytes_per_sector = 512;
    }
    
    return 0;
}

static void msc_release(F_DRIVER *driver)
{
    struct msc_param* mscParam = (struct msc_param*)driver->user_data;

    #if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(mscParam->type))
        iteUasTerminate(mscParam->ctxt, mscParam->mscLun);
    else
    #endif
    iteMscTerminate(mscParam->ctxt, mscParam->mscLun);
    g_initok[mscParam->mscIndex] = 0;
    free((void*)mscParam);
    (void)memset (driver,0,sizeof(F_DRIVER));
}

/****************************************************************************
 *
 * msc_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - special code for driver initialization
 *
 * RETURNS
 *
 * driver pointer or zero if error
 *
 ***************************************************************************/

F_DRIVER * msc_initfunc(unsigned long driver_param)
{
    int rc = 0;
    F_DRIVER *driver;
    ITPUsbInfo usbInfo = {0};
    struct msc_param* mscParam = (struct msc_param*)malloc(sizeof(struct msc_param));
#if 1
    if((int)driver_param < 16)
    {
        mscParam->mscIndex = (int)driver_param;
        mscParam->usbIndex = (mscParam->mscIndex < 8) ? 0 : 1;
        mscParam->mscLun   = mscParam->mscIndex % 8;

        usbInfo.host = true;
        usbInfo.usbIndex = mscParam->usbIndex;
        ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, (void*)&usbInfo);
        mscParam->ctxt = usbInfo.ctxt;
        mscParam->type = usbInfo.type;
    }
    else
    {   /* for test code use, it will give the entire structure directly. */
        memcpy((void*)mscParam, (void*)driver_param, sizeof(struct msc_param));
    }
#else
    mscParam->mscIndex = (int)driver_param;
    mscParam->usbIndex = (mscParam->mscIndex < 8) ? 0 : 1;
    mscParam->mscLun   = mscParam->mscIndex % 8;

    usbInfo.host = true;
    usbInfo.usbIndex = mscParam->usbIndex;
    ioctl(ITP_DEVICE_USB, ITP_IOCTL_GET_INFO, (void*)&usbInfo);
    mscParam->ctxt = usbInfo.ctxt;
    mscParam->type = usbInfo.type;
#endif

    #if defined(CFG_UAS_ENABLE)
    if(USB_DEVICE_UAS(mscParam->type))
        rc = iteUasInitialize(mscParam->ctxt, mscParam->mscLun);
    else
    #endif
    rc = iteMscInitialize(mscParam->ctxt, mscParam->mscLun);
    if(rc)
    {
        free((void*)mscParam);
        return 0;
    }

    g_initok[mscParam->mscIndex] = 1;

    /* init driver functions */
    driver=&g_drivers[mscParam->mscIndex];
    (void)memset (driver,0,sizeof(F_DRIVER));
    
    driver->readsector=msc_readsector;
    driver->writesector=msc_writesector;
    driver->readmultiplesector=msc_readmultiplesector;
    driver->writemultiplesector=msc_writemultiplesector;
    driver->getstatus=msc_getstatus;
    driver->getphy=msc_getphy;
    driver->release=msc_release;
    driver->user_data=(unsigned long)mscParam;

    return driver;
}

