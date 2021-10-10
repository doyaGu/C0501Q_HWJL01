#include "ite/ith.h"
#include "xd/xd_mlayer.h"

#include "fat_xddrv.h"


#define BLOCK_SIZE      512
static int g_initok = 0;
static F_DRIVER g_drivers;
/****************************************************************************
 *
 * xd_readsector
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

static int xd_readsector(F_DRIVER *driver,void *data,unsigned long sector) 
{
    struct xd_param* xdParam = (struct xd_param*)driver->user_data;

    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (xddrv_readsector(driver, data, sector))
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
 * xd_writesector
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

static int xd_writesector(F_DRIVER *driver,void *data, unsigned long sector) 
{
    struct xd_param* xdParam = (struct xd_param*)driver->user_data;

    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (xddrv_writesector(driver, data, sector))
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
 * xd_readsector
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

static int xd_readmultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    struct xd_param* xdParam = (struct xd_param*)driver->user_data;

    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (xddrv_readmultiplesector(driver, data, sector, cnt))
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
 * xd_writesector
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

static int xd_writemultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    struct xd_param* xdParam = (struct xd_param*)driver->user_data;

    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (xddrv_writemultiplesector(driver, data, sector, cnt))
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
 * mmcsd_getstatus
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

static long xd_getstatus(F_DRIVER *driver) 
{
    struct xd_param* xdParam = (struct xd_param*)driver->user_data;
    int state=0;

    if (ithCardInserted(ITH_CARDPIN_XD))
    {
        if (g_initok==0 && xddrv_getstatus(driver, XD_INIT_OK)) 
        //if (g_initok==0 && xd_init()==true) 
        {
            state |= F_ST_CHANGED;
            g_initok = 1;
        }
    }
    else
    {
        g_initok = 0;
        state |= F_ST_MISSING;
    }

    return state; 
}

/****************************************************************************
 *
 * xd_getphy
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

static int xd_getphy(F_DRIVER *driver, F_PHY *phy) 
{
    struct xd_param* xdParam = (struct xd_param*)driver->user_data;
    int res = 0;
    uint32_t sectors=0, blockSize=0;
    
    if (g_initok)
    {
    	//xddrv_getphy()
        //res = xddrv_getcapacity( &sectors, &blockSize);
        res = mmpxDGetCapacity( &sectors, &blockSize);
        if(!res)
            return 1;
        phy->number_of_cylinders = 0;
        phy->sector_per_track = 63;
        phy->number_of_heads = 255;
        phy->number_of_sectors = sectors;// - xdParam->reserved/BLOCK_SIZE;

        //if(xdParam->removable)
        //    phy->media_descriptor = 0xf0; /* removable */
        //else
        //    phy->media_descriptor = 0xf8; /* fixdrive */

    }
    
    return 0;
}

static void xd_release(F_DRIVER *driver)
{
    struct xd_param* xdParam = (struct xd_param*)driver->user_data;

    xddrv_release(driver);
    (void)memset (driver,0,sizeof(F_DRIVER));
}

/****************************************************************************
 *
 * xd_initfunc
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

F_DRIVER * xd_initfunc(unsigned long driver_param)
{
    struct xd_param* xdParam = (struct xd_param*)driver_param;
    F_DRIVER *driver;

    g_initok = 0;

    /* init driver functions */
    driver=&g_drivers;

    (void)memset (driver,0,sizeof(F_DRIVER));
    
    driver->readsector=xd_readsector;
    driver->writesector=xd_writesector;
    driver->readmultiplesector=xd_readmultiplesector;
    driver->writemultiplesector=xd_writemultiplesector;
    driver->getstatus=xd_getstatus;
    driver->getphy=xd_getphy;
    driver->release=xd_release;
    driver->user_data=driver_param;
    xd_getstatus(driver);

    return driver;
}

