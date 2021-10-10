#include "msprofat.h"
#include "ite/itp.h"
#include "ite/ite_mspro.h"


static int g_initok = 0;
static F_DRIVER g_driver;


/****************************************************************************
 *
 * readsector
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

static int readsector(F_DRIVER *driver,void *data,unsigned long sector) 
{
    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteMsproReadMultipleSector(sector, 1, data))
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
 * writesector
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

static int writesector(F_DRIVER *driver,void *data,unsigned long sector) 
{
    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteMsproWriteMultipleSector(sector, 1, data))
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
 * readsector
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

static int readmultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteMsproReadMultipleSector(sector, cnt, data))
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
 * writesector
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

static int writemultiplesector(F_DRIVER *driver,void *data,unsigned long sector,int cnt) 
{
    if (!g_initok) 
        return F_ERR_INVALIDDRIVE; /*card is not initialized*/

    if (iteMsproWriteMultipleSector(sector, cnt, data))
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
 * getstatus
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

static long getstatus(F_DRIVER *driver) 
{
    int state=0;

    if (iteMsproGetCardState(MS_INSERTED))
    {
        if (g_initok==0) 
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
 * getphy
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

static int getphy(F_DRIVER *driver, F_PHY *phy) 
{
    int res = 0;
    uint32_t sectors=0, blockSize=0;
    
    if (g_initok)
    {
        res = iteMsproGetCapacity(&sectors, &blockSize);
        if(res)
            return res;
        phy->number_of_cylinders = 0;
        phy->sector_per_track = 63;
        phy->number_of_heads = 255;
        phy->number_of_sectors = sectors;
        phy->media_descriptor = 0xf0; /* removable */
    }
    
    return 0;
}

static void release(F_DRIVER *driver)
{
    iteMsproTerminate();
    g_initok = 0;
    (void)memset (driver,0,sizeof(F_DRIVER));
}

/****************************************************************************
 *
 * mspro_initfunc
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

F_DRIVER * mspro_initfunc(unsigned long driver_param)
{
    F_DRIVER *driver;

    g_initok = 1;

    /* init driver functions */
    driver=&g_driver;
    (void)memset (driver,0,sizeof(F_DRIVER));
    
    driver->readsector=readsector;
    driver->writesector=writesector;
    driver->readmultiplesector=readmultiplesector;
    driver->writemultiplesector=writemultiplesector;
    driver->getstatus=getstatus;
    driver->getphy=getphy;
    driver->release=release;
    getstatus(driver);

    return driver;
}

