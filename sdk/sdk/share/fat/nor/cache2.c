#include <string.h>
#include <limits.h>
#include <malloc.h>
#include <stdint.h>

#include "cache2.h"
#include "ite/ith.h"
#include <pthread.h>

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

/* Cache flush sync flag */
extern bool			CacheValidFlush;
extern NOR_CONTEXT	NorContext;

/*-----------------------------------------------------------------
Functions to deal with little endian values stored in uint8_t arrays
-----------------------------------------------------------------*/
static inline uint16_t uint8_tarray_to_u16 (const uint8_t* item, int offset) {
	return ( item[offset] | (item[offset + 1] << 8));
}

static inline uint32_t uint8_tarray_to_uint32_t (const uint8_t* item, int offset) {
	return ( item[offset] | (item[offset + 1] << 8) | (item[offset + 2] << 16) | (item[offset + 3] << 24));
}

static inline void u16_to_uint8_tarray (uint8_t* item, int offset, uint16_t value) {
	item[offset]     = (uint8_t) value;
	item[offset + 1] = (uint8_t)(value >> 8);
}

static inline void uint32_t_to_uint8_tarray (uint8_t* item, int offset, uint32_t value) {
	item[offset]     = (uint8_t) value;
	item[offset + 1] = (uint8_t)(value >> 8);
	item[offset + 2] = (uint8_t)(value >> 16);
	item[offset + 3] = (uint8_t)(value >> 24);
}

static inline void* nor_alloc (size_t size) {
    return malloc(size);
}

static inline void* nor_align (size_t size) {
    #ifndef _WIN32
    return memalign(32, size);
    #else
    return malloc(size);
    #endif
}

static inline void nor_free (void* mem) {
    free(mem);
}

#define CACHE_FREE UINT_MAX

NOR_CACHE* _NOR_cache_constructor (unsigned int numberOfPages, unsigned int sectorsPerPage, const DISC_INTERFACE* discInterface, sec_t endOfPartition, sec_t sectorSize) {
	NOR_CACHE* cache;
	unsigned int i;
	NOR_CACHE_ENTRY* cacheEntries;

	if(numberOfPages==0 || sectorsPerPage==0) return NULL;

	if (numberOfPages < 4) {
		numberOfPages = 4;
	}

	if (sectorsPerPage < 32) {
		sectorsPerPage = 32;
	}

	cache = (NOR_CACHE*) nor_alloc (sizeof(NOR_CACHE));
	if (cache == NULL) {
		return NULL;
	}

	cache->disc = discInterface;
	cache->endOfPartition = endOfPartition;
	cache->numberOfPages = numberOfPages;
	cache->sectorsPerPage = sectorsPerPage;
	cache->sectorSize = sectorSize;


	cacheEntries = (NOR_CACHE_ENTRY*) nor_alloc ( sizeof(NOR_CACHE_ENTRY) * numberOfPages);
	if (cacheEntries == NULL) {
		nor_free (cache);
		return NULL;
	}

	for (i = 0; i < numberOfPages; i++) {
		cacheEntries[i].sector = CACHE_FREE;
		cacheEntries[i].count = 0;
		cacheEntries[i].last_access = 0;
		cacheEntries[i].dirty = false;
		cacheEntries[i].cache = (uint8_t*) nor_align ( sectorsPerPage * cache->sectorSize );
	}

	cache->cacheEntries = cacheEntries;

	return cache;
}

void _NOR_cache_destructor (NOR_CACHE* cache) {
	unsigned int i;

	if(cache==NULL) return;

	// Clear out cache before destroying it
	_NOR_cache_flush(cache);

	// Free memory in reverse allocation order
	for (i = 0; i < cache->numberOfPages; i++) {
		nor_free (cache->cacheEntries[i].cache);
	}
	nor_free (cache->cacheEntries);
	nor_free (cache);
}

static uint32_t accessCounter = 0;

static uint32_t accessTime(){
	accessCounter++;
	return accessCounter;
}

static NOR_CACHE_ENTRY* _NOR_cache_getPage(NOR_CACHE *cache,sec_t sector)
{
	unsigned int i;
	NOR_CACHE_ENTRY* cacheEntries = cache->cacheEntries;
	unsigned int numberOfPages = cache->numberOfPages;
	unsigned int sectorsPerPage = cache->sectorsPerPage;

	bool foundFree = false;
	unsigned int oldUsed = 0;
	unsigned int oldAccess = UINT_MAX;
    sec_t next_page;

	for(i=0;i<numberOfPages;i++) {
		if(sector>=cacheEntries[i].sector && sector<(cacheEntries[i].sector + cacheEntries[i].count)) {
			cacheEntries[i].last_access = accessTime();
			return &(cacheEntries[i]);
		}

		if(foundFree==false && (cacheEntries[i].sector==CACHE_FREE || cacheEntries[i].last_access<oldAccess)) {
		    if(cacheEntries[i].sector==CACHE_FREE) foundFree = true;
			oldUsed = i;
			oldAccess = (unsigned int)cacheEntries[i].last_access;
		}
	}

	if(foundFree==false && cacheEntries[oldUsed].dirty==true) {
		//if(!cache->disc->writeSectors(cacheEntries[oldUsed].sector,cacheEntries[oldUsed].count,cacheEntries[oldUsed].cache)) return NULL;       
		//cacheEntries[oldUsed].dirty = false;
        _NOR_cache_flush(cache);
	}
	sector = (sector/sectorsPerPage)*sectorsPerPage; // align base sector to page size
	next_page = sector + sectorsPerPage;
	if(next_page > cache->endOfPartition)	next_page = (cache->endOfPartition + 1);

	if(!cache->disc->readSectors(sector,next_page-sector,cacheEntries[oldUsed].cache)) return NULL;

	cacheEntries[oldUsed].sector = sector;
	cacheEntries[oldUsed].count = next_page-sector;
	cacheEntries[oldUsed].last_access = accessTime();

	return &(cacheEntries[oldUsed]);
}

static NOR_CACHE_ENTRY* _NOR_cache_findPage(NOR_CACHE *cache, sec_t sector, sec_t count) {

	unsigned int i;
	NOR_CACHE_ENTRY* cacheEntries = cache->cacheEntries;
	unsigned int numberOfPages = cache->numberOfPages;
	NOR_CACHE_ENTRY *entry = NULL;
	sec_t	lowest = UINT_MAX;

	for(i=0;i<numberOfPages;i++) {
		if (cacheEntries[i].sector != CACHE_FREE) {
			bool intersect = false;
			if (sector > cacheEntries[i].sector) {
				intersect = sector - cacheEntries[i].sector < cacheEntries[i].count;
			} 
			//else {
			//	intersect = cacheEntries[i].sector - sector < count;
			//}

			if ( intersect && (cacheEntries[i].sector < lowest)) {
				lowest = cacheEntries[i].sector;
				entry = &cacheEntries[i];
				break;
			}
		}
	}

	return entry;
}

bool _NOR_cache_readSectors(NOR_CACHE *cache,sec_t sector,sec_t numSectors,void *buffer)
{
	sec_t sec;
	sec_t secs_to_read;
	NOR_CACHE_ENTRY *entry;
	uint8_t *dest = buffer;

	while(numSectors>0) {
		entry = _NOR_cache_getPage(cache,sector);
		if(entry==NULL) return false;

		sec = sector - entry->sector;
		secs_to_read = entry->count - sec;
		if(secs_to_read>numSectors) secs_to_read = numSectors;

		memcpy(dest,entry->cache + (sec*cache->sectorSize),(secs_to_read*cache->sectorSize));

		dest += (secs_to_read*cache->sectorSize);
		sector += secs_to_read;
		numSectors -= secs_to_read;
	}

	return true;
}

/*
Reads some data from a cache page, determined by the sector number
*/

bool _NOR_cache_readPartialSector (NOR_CACHE* cache, void* buffer, sec_t sector, unsigned int offset, size_t size)
{
	sec_t sec;
	NOR_CACHE_ENTRY *entry;

	if (offset + size > cache->sectorSize) return false;

	entry = _NOR_cache_getPage(cache,sector);
	if(entry==NULL) return false;

	sec = sector - entry->sector;
	memcpy(buffer,entry->cache + ((sec*cache->sectorSize) + offset),size);

	return true;
}

bool _NOR_cache_readLittleEndianValue (NOR_CACHE* cache, uint32_t *value, sec_t sector, unsigned int offset, int num_bytes) {
  uint8_t buf[4];
  if (!_NOR_cache_readPartialSector(cache, buf, sector, offset, num_bytes)) return false;

  switch(num_bytes) {
  case 1: *value = buf[0]; break;
  case 2: *value = uint8_tarray_to_u16(buf,0); break;
  case 4: *value = uint8_tarray_to_uint32_t(buf,0); break;
  default: return false;
  }
  return true;
}

/*
Writes some data to a cache page, making sure it is loaded into memory first.
*/

bool _NOR_cache_writePartialSector (NOR_CACHE* cache, const void* buffer, sec_t sector, unsigned int offset, size_t size)
{
	sec_t sec;
	NOR_CACHE_ENTRY *entry;

	if (offset + size > cache->sectorSize) return false;

	entry = _NOR_cache_getPage(cache,sector);
	if(entry==NULL) return false;

	sec = sector - entry->sector;
	memcpy(entry->cache + ((sec*cache->sectorSize) + offset),buffer,size);

	entry->dirty = true;
	return true;
}

bool _NOR_cache_writeLittleEndianValue (NOR_CACHE* cache, const uint32_t value, sec_t sector, unsigned int offset, int size) {
  uint8_t buf[4] = {0, 0, 0, 0};

  switch(size) {
  case 1: buf[0] = value; break;
  case 2: u16_to_uint8_tarray(buf, 0, value); break;
  case 4: uint32_t_to_uint8_tarray(buf, 0, value); break;
  default: return false;
  }

  return _NOR_cache_writePartialSector(cache, buf, sector, offset, size);
}

/*
Writes some data to a cache page, zeroing out the page first
*/

bool _NOR_cache_eraseWritePartialSector (NOR_CACHE* cache, const void* buffer, sec_t sector, unsigned int offset, size_t size)
{
	sec_t sec;
	NOR_CACHE_ENTRY *entry;

	if (offset + size > cache->sectorSize) return false;

	entry = _NOR_cache_getPage(cache,sector);
	if(entry==NULL) return false;

	sec = sector - entry->sector;
	memset(entry->cache + (sec*cache->sectorSize),0,cache->sectorSize);
	memcpy(entry->cache + ((sec*cache->sectorSize) + offset),buffer,size);

	entry->dirty = true;
	return true;
}

bool _NOR_cache_writeSectors (NOR_CACHE* cache, sec_t sector, sec_t numSectors, const void* buffer)
{
	sec_t sec;
	sec_t secs_to_write;
	NOR_CACHE_ENTRY* entry;
	const uint8_t *src = buffer;

	while(numSectors>0)
	{
		entry = _NOR_cache_findPage(cache,sector,numSectors);
		if(entry==NULL)
		    entry = _NOR_cache_getPage(cache,sector);
    		    
		if(entry!=NULL) {

			if ( entry->sector > sector) {

				secs_to_write = entry->sector - sector;

				cache->disc->writeSectors(sector,secs_to_write,src);
				src += (secs_to_write*cache->sectorSize);
				sector += secs_to_write;
				numSectors -= secs_to_write;
			}

			sec = sector - entry->sector;
			secs_to_write = entry->count - sec;

			if(secs_to_write>numSectors) secs_to_write = numSectors;

			memcpy(entry->cache + (sec*cache->sectorSize),src,(secs_to_write*cache->sectorSize));

			src += (secs_to_write*cache->sectorSize);
			sector += secs_to_write;
			numSectors -= secs_to_write;

			entry->dirty = true;

		} else {
    		return false;
		}
	}
	return true;
}

/*
Flushes all dirty pages to disc, clearing the dirty flag.
*/
bool _NOR_cache_flush (NOR_CACHE* cache) 
{
	unsigned int i, j, flushSector;
    int flushIndex;
	bool         ret;

	pthread_mutex_lock(&NorContext.flushMutex);

	if ( CacheValidFlush == false )
	{
		ret = false;
		goto end;
	}
	
	if(cache==NULL)
	{
		ret = false;
		goto end;
	}

	for (i = 0; i < cache->numberOfPages; i++)
	{
        flushIndex = -1;
        flushSector = CACHE_FREE;
        for (j = 0; j < cache->numberOfPages; j++)
        {
    		if (cache->cacheEntries[j].dirty)
    		{
                if (cache->cacheEntries[j].sector < flushSector)
                {
                    flushIndex = j;
                    flushSector = cache->cacheEntries[j].sector;
                }
    		}
        }
        if (flushIndex >= 0)
        {
			if ( !cache->disc->writeSectors(
					cache->cacheEntries[flushIndex].sector, 
					cache->cacheEntries[flushIndex].count, 
					cache->cacheEntries[flushIndex].cache) )
			{
				ret = false;
				goto end;
			}
            cache->cacheEntries[flushIndex].dirty = false;
        }
        else //no more flush entry.
        {
            break;
        }
	}
end:
	pthread_mutex_unlock(&NorContext.flushMutex);

	return ret;
}

void _NOR_cache_invalidate (NOR_CACHE* cache) {
	unsigned int i;
	if(cache==NULL)
        return;

	_NOR_cache_flush(cache);
	for (i = 0; i < cache->numberOfPages; i++) {
		cache->cacheEntries[i].sector = CACHE_FREE;
		cache->cacheEntries[i].last_access = 0;
		cache->cacheEntries[i].count = 0;
		cache->cacheEntries[i].dirty = false;
	}
}
