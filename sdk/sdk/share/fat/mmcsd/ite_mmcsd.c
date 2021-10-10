#include "mmcsd.h"
#include "ite/ite_sd.h"


#define BLOCK_SIZE      512

/** same with itp_fat.c */
struct sd_param
{
    int sd;                 // sd0 or sd1
    int removable;          // removable or not
    unsigned long reserved; // reserved size
};


static int g_initok[2] = {0,0};
static F_DRIVER g_drivers[2];


/****************************************************************************
 *
 * mmcsd_readsector
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

static int mmcsd_readsector(F_DRIVER *driver,void *data,unsigned long sector) 
{
    struct sd_param* sdParam = (struct sd_param*)driver->user_data;

    if (!g_initok[sdParam->sd]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteSdReadMultipleSectorEx(sdParam->sd, (sector+sdParam->reserved/BLOCK_SIZE),1,data))
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
 * mmcsd_writesector
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

static int mmcsd_writesector(F_DRIVER *driver,void *data,unsigned long sector) 
{
    struct sd_param* sdParam = (struct sd_param*)driver->user_data;

    if (!g_initok[sdParam->sd]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteSdWriteMultipleSectorEx(sdParam->sd, (sector+sdParam->reserved/BLOCK_SIZE),1,data))
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
 * mmcsd_readsector
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

static int mmcsd_readmultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    struct sd_param* sdParam = (struct sd_param*)driver->user_data;

    if (!g_initok[sdParam->sd]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteSdReadMultipleSectorEx(sdParam->sd, (sector+sdParam->reserved/BLOCK_SIZE),cnt,data))
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
 * mmcsd_writesector
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

static int mmcsd_writemultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    struct sd_param* sdParam = (struct sd_param*)driver->user_data;

    if (!g_initok[sdParam->sd]) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteSdWriteMultipleSectorEx(sdParam->sd, (sector+sdParam->reserved/BLOCK_SIZE), cnt,data))
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

static long mmcsd_getstatus(F_DRIVER *driver) 
{
    struct sd_param* sdParam = (struct sd_param*)driver->user_data;
    int state=0;

    if (iteSdGetCardStateEx(sdParam->sd, SD_INSERTED))
    {
        if (g_initok[sdParam->sd]==0 && iteSdGetCardStateEx(sdParam->sd, SD_INIT_OK)) 
        {
            state |= F_ST_CHANGED;
            if (iteSdIsLockEx(sdParam->sd)) 
                state |= F_ST_WRPROTECT;
            g_initok[sdParam->sd] = 1;
        }
    }
    else
    {
        g_initok[sdParam->sd] = 0;
        state |= F_ST_MISSING;
    }

    return state; 
}

/****************************************************************************
 *
 * mmcsd_getphy
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

static int mmcsd_getphy(F_DRIVER *driver, F_PHY *phy) 
{
    struct sd_param* sdParam = (struct sd_param*)driver->user_data;
    int res = 0;
    uint32_t sectors=0, blockSize=0;
    
    if (g_initok[sdParam->sd])
    {
        res = iteSdGetCapacityEx(sdParam->sd, &sectors, &blockSize);
        if(res)
            return res;
        phy->number_of_cylinders = 0;
        phy->sector_per_track = 63;
        phy->number_of_heads = 255;
        phy->number_of_sectors = sectors - sdParam->reserved/BLOCK_SIZE;
		phy->bytes_per_sector = blockSize;
        if(sdParam->removable)
            phy->media_descriptor = 0xf0; /* removable */
        else
            phy->media_descriptor = 0xf8; /* fixdrive */
    }
    
    return 0;
}

static void mmcsd_release(F_DRIVER *driver)
{
    struct sd_param* sdParam = (struct sd_param*)driver->user_data;

    if (!iteSdGetCardStateEx(sdParam->sd, SD_INSERTED))
        iteSdTerminateEx(sdParam->sd);
    (void)memset (driver,0,sizeof(F_DRIVER));
}

/****************************************************************************
 *
 * mmcsd_initfunc
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

F_DRIVER * mmcsd_initfunc(unsigned long driver_param)
{
    struct sd_param* sdParam = (struct sd_param*)driver_param;
    F_DRIVER *driver;

    g_initok[sdParam->sd] = 0;
   

    /* init driver functions */
    driver=&g_drivers[sdParam->sd];

    (void)memset (driver,0,sizeof(F_DRIVER));
    
    driver->readsector=mmcsd_readsector;
    driver->writesector=mmcsd_writesector;
    driver->readmultiplesector=mmcsd_readmultiplesector;
    driver->writemultiplesector=mmcsd_writemultiplesector;
    driver->getstatus=mmcsd_getstatus;
    driver->getphy=mmcsd_getphy;
    driver->release=mmcsd_release;
    driver->user_data=driver_param;
    mmcsd_getstatus(driver);

    return driver;
}

