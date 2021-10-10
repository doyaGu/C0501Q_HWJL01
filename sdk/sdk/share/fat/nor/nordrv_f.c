#include "nordrv_f.h"
#include "ite/ith.h"
#include <stdlib.h>
#include <stdio.h>
#include "cache2.h"
#include "nor/mmp_nor.h"
#include "ssp/mmp_spi.h"
#include <pthread.h>

typedef struct
{
    unsigned long reserved;     // reserved size in byte
    unsigned long cacheSize;    // norCache size
} NORDrvParam;

typedef struct _NOR_CONTEXT
{
    unsigned long   reservrdFatSectors;	// reserved size in fat sector
	unsigned long   pageSize;			// page size in byte
	unsigned long	pagesPerSector;
	unsigned long	sectorsPerBlock;
	unsigned long	blockSize;
	unsigned long	fatSectorSize;
	unsigned long   blockSizeInBytes;
	unsigned long   fatSectorsPerBlock;
	pthread_mutex_t flushMutex;
}NOR_CONTEXT;

static F_DRIVER		nor_drv;
static NOR_CACHE*	norCache;
NOR_CONTEXT	NorContext;
bool		CacheValidFlush = false; // Cache flush sync flag

static bool readSectors(sec_t sector, sec_t numSectors, void* buffer);
static bool writeSectors(sec_t sector, sec_t numSectors, const void* buffer);

void iteFatNorFlush(void)
{	
    if (norCache)
    {
        _NOR_cache_flush(norCache);
    }
}

int nor_getCapacity(unsigned int* sec_num, unsigned int* block_size)
{
	*sec_num = NorGetAttitude(SPI_0, SPI_CSN_0, NOR_ATTITUDE_TOTAL_SIZE)*NorGetAttitude(SPI_0, SPI_CSN_0, NOR_ATTITUDE_PAGE_SIZE)/512;
	*block_size = 512;
	//printf("sec_num = %d, block_size=%d \n", *sec_num, *block_size);
	return 0;
}

/****************************************************************************
 *
 * nor_readmultiplesector
 *
 * read multiple sectors from nordrive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer where to store data
 * sector - where to read data from
 * cnt - number of sectors to read
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int nor_readmultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt)
{
	if ( norCache )
	{
		return _NOR_cache_readSectors(norCache, sector, cnt, data) ? 0 : 1;
	}
	else
	{
		return readSectors(sector, cnt, data) ? 0 : 1;
	}
}

/****************************************************************************
 * Read one sector
 ***************************************************************************/

static int nor_readsector(F_DRIVER *driver,void *data, unsigned long sector)
{
	return nor_readmultiplesector(driver, data, sector, 1);
}

/****************************************************************************
 *
 * nor_writemultiplesector
 *
 * write multiple sectors into nordrive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer
 * sector - where to write data
 * cnt - number of sectors to write
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

int nor_writemultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt)
{
    if ( norCache )
	{
		return _NOR_cache_writeSectors(norCache, sector, cnt, data) ? 0 : 1;
	}
	else
	{
		return writeSectors(sector, cnt, data) ? 0 : 1;
	}
}

/****************************************************************************
 * Write one sector
 ***************************************************************************/

static int nor_writesector(F_DRIVER *driver,void *data, unsigned long sector)
{
	return nor_writemultiplesector(driver, data, sector, 1);
}

/****************************************************************************
 *
 * nor_getphy
 *
 * determinate nordrive physicals
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

static int nor_getphy(F_DRIVER *driver,F_PHY *phy)
{
	if ( phy )
	{
		phy->bytes_per_sector  = NorContext.fatSectorSize;
		phy->number_of_sectors = NorContext.blockSize * NorContext.fatSectorsPerBlock - NorContext.reservrdFatSectors;
	}
	
	return NOR_NO_ERROR;
}

/****************************************************************************
 *
 * nor_release
 *
 * Releases a drive
 *
 * INPUTS
 *
 * driver_panor - driver panoreter
 *
 ***************************************************************************/

static void nor_release (F_DRIVER *driver)
{
	if (norCache)
    {
        _NOR_cache_destructor(norCache);
    }

	/* Disable norCache flush */
	pthread_mutex_lock(&NorContext.flushMutex);
	CacheValidFlush = false;
	pthread_mutex_unlock(&NorContext.flushMutex);

	if ( NorContext.flushMutex )
	{
    	pthread_mutex_destroy(&NorContext.flushMutex);
    }
}

/****************************************************************************
 *
 * nor_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_panor - driver panoreter
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

static bool readSectors(sec_t sector, sec_t numSectors, void* buffer)
{
    uint32_t norResult = 0;

#if defined(CFG_NOR_SHARE_GPIO) || defined(CFG_NOR_SHARE_SPI_NAND)
    mmpSpiSetControlMode(SPI_CONTROL_NOR);
#endif

	norResult = NorRead(SPI_0, SPI_CSN_0, (NorContext.reservrdFatSectors + sector) * NorContext.fatSectorSize, buffer, numSectors * NorContext.fatSectorSize);

#if defined(CFG_NOR_SHARE_GPIO) || defined(CFG_NOR_SHARE_SPI_NAND)
    mmpSpiResetControl();
#endif

	if ( norResult == 0 )
	{
		// Success
		return true;
	}
	else
	{
		return false;
	}
}

static bool writeSectors(sec_t sector, sec_t numSectors, const void* buffer)
{
    uint32_t norResult = 0;

#if defined(CFG_NOR_SHARE_GPIO) || defined(CFG_NOR_SHARE_SPI_NAND)
    mmpSpiSetControlMode(SPI_CONTROL_NOR);
#endif

	norResult = NorWrite(SPI_0, SPI_CSN_0, (NorContext.reservrdFatSectors + sector) * NorContext.fatSectorSize, (void*)buffer, numSectors * NorContext.fatSectorSize);

#if defined(CFG_NOR_SHARE_GPIO) || defined(CFG_NOR_SHARE_SPI_NAND)
    mmpSpiResetControl();
#endif

	if ( norResult == 0 )
	{
		// Success
		return true;
	}
	else
	{
		return false;
	}
}

static const DISC_INTERFACE discInterface =
{
    readSectors,
    writeSectors
};

F_DRIVER *nor_initfunc(unsigned long driver_panor)
{
    NORDrvParam* drvParam = (NORDrvParam*)driver_panor;
    unsigned long reservrdBlocks;

    if ( pthread_mutex_init(&NorContext.flushMutex, NULL) != 0 )
   	{
   		// Create mutext fail!
   		NorContext.flushMutex = (pthread_mutex_t)NULL;
   		return NULL;
   	}

	NorContext.pageSize             = NorGetAttitude(SPI_0, SPI_CSN_0, NOR_ATTITUDE_PAGE_SIZE);
	NorContext.pagesPerSector       = NorGetAttitude(SPI_0, SPI_CSN_0, NOR_ATTITUDE_PAGE_PER_SECTOR);
	NorContext.sectorsPerBlock      = NorGetAttitude(SPI_0, SPI_CSN_0, NOR_ATTITUDE_SECTOR_PER_BLOCK);
	NorContext.blockSize            = NorGetAttitude(SPI_0, SPI_CSN_0, NOR_ATTITUDE_BLOCK_SIZE);
	NorContext.fatSectorSize        = NorContext.pageSize > 512 ? NorContext.pageSize : 512;
	NorContext.blockSizeInBytes     = NorContext.pageSize * NorContext.pagesPerSector * NorContext.sectorsPerBlock;
	NorContext.fatSectorsPerBlock   = NorContext.blockSizeInBytes / NorContext.fatSectorSize;
	    
    reservrdBlocks = drvParam->reserved / NorContext.blockSizeInBytes;

    if (drvParam->reserved % NorContext.blockSizeInBytes > 0)
        printf("nor reserved size is not align on block.\n");

    NorContext.reservrdFatSectors = reservrdBlocks * NorContext.fatSectorsPerBlock;
    
    // init norCache system
    if (drvParam->cacheSize > 0)
    {
        unsigned int cacheBlockCount = drvParam->cacheSize / NorContext.blockSizeInBytes;
        unsigned int endOfFatSector = NorContext.blockSize * NorContext.fatSectorsPerBlock - NorContext.reservrdFatSectors - 1;

        // Create the norCache
        norCache = _NOR_cache_constructor(cacheBlockCount, NorContext.fatSectorsPerBlock, &discInterface, endOfFatSector, NorContext.fatSectorSize);
    }
    
	(void)memset (&nor_drv,0,sizeof(F_DRIVER));

	nor_drv.readsector=nor_readsector;
	nor_drv.writesector=nor_writesector;
	nor_drv.readmultiplesector=nor_readmultiplesector;
	nor_drv.writemultiplesector=nor_writemultiplesector;
	nor_drv.getphy=nor_getphy;
	nor_drv.release=nor_release;
	nor_drv.user_ptr=drvParam;

	/* Enable norCache flush */
	pthread_mutex_lock(&NorContext.flushMutex);
	CacheValidFlush = true;
	pthread_mutex_unlock(&NorContext.flushMutex);
	
	return &nor_drv;
}

/******************************************************************************
 *
 *  End of nordrv.c
 *
 *****************************************************************************/

