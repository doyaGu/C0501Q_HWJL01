#ifndef _CACHE2_H
#define _CACHE2_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t sec_t;

typedef bool (* FN_MEDIUM_READSECTORS)(sec_t sector, sec_t numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(sec_t sector, sec_t numSectors, const void* buffer) ;

struct DISC_INTERFACE_STRUCT {
	FN_MEDIUM_READSECTORS	readSectors ;
	FN_MEDIUM_WRITESECTORS	writeSectors ;
} ;

typedef struct DISC_INTERFACE_STRUCT DISC_INTERFACE ;

typedef struct {
	sec_t           sector;
	unsigned int    count;
	uint64_t             last_access;
	bool            dirty;
	uint8_t*             cache;
} NOR_CACHE_ENTRY;

typedef struct {
	const DISC_INTERFACE* disc;
	sec_t		          endOfPartition;
	unsigned int          numberOfPages;
	unsigned int          sectorsPerPage;
	sec_t                 sectorSize;
	NOR_CACHE_ENTRY*     cacheEntries;
} NOR_CACHE;

/*
Read data from a sector in the NOR_CACHE
If the sector is not in the NOR_CACHE, it will be swapped in
offset is the position to start reading from
size is the amount of data to read
Precondition: offset + size <= BYTES_PER_READ
*/
//bool _NOR_cache_readPartialSector (NOR_CACHE* NOR_CACHE, void* buffer, sec_t sector, unsigned int offset, size_t size);

//bool _NOR_cache_readLittleEndianValue (NOR_CACHE* NOR_CACHE, uint32_t *value, sec_t sector, unsigned int offset, int num_bytes);

/*
Write data to a sector in the NOR_CACHE
If the sector is not in the NOR_CACHE, it will be swapped in.
When the sector is swapped out, the data will be written to the disc
offset is the position to start writing to
size is the amount of data to write
Precondition: offset + size <= BYTES_PER_READ
*/
//bool _NOR_cache_writePartialSector (NOR_CACHE* NOR_CACHE, const void* buffer, sec_t sector, unsigned int offset, size_t size);

//bool _NOR_cache_writeLittleEndianValue (NOR_CACHE* NOR_CACHE, const uint32_t value, sec_t sector, unsigned int offset, int num_bytes);

/*
Write data to a sector in the NOR_CACHE, zeroing the sector first
If the sector is not in the NOR_CACHE, it will be swapped in.
When the sector is swapped out, the data will be written to the disc
offset is the position to start writing to
size is the amount of data to write
Precondition: offset + size <= BYTES_PER_READ
*/
//bool _NOR_cache_eraseWritePartialSector (NOR_CACHE* NOR_CACHE, const void* buffer, sec_t sector, unsigned int offset, size_t size);

/*
Read several sectors from the NOR_CACHE
*/
bool _NOR_cache_readSectors (NOR_CACHE* NOR_CACHE, sec_t sector, sec_t numSectors, void* buffer);

/*
Read a full sector from the NOR_CACHE
*/
//static inline bool _NOR_cache_readSector (NOR_CACHE* NOR_CACHE, void* buffer, sec_t sector) {
//	return _NOR_cache_readPartialSector (NOR_CACHE, buffer, sector, 0, BYTES_PER_READ);
//}

/*
Write a full sector to the NOR_CACHE
*/
//static inline bool _NOR_cache_writeSector (NOR_CACHE* NOR_CACHE, const void* buffer, sec_t sector) {
//	return _NOR_cache_writePartialSector (NOR_CACHE, buffer, sector, 0, BYTES_PER_READ);
//}

bool _NOR_cache_writeSectors (NOR_CACHE* NOR_CACHE, sec_t sector, sec_t numSectors, const void* buffer);

/*
Write any dirty sectors back to disc and clear out the contents of the NOR_CACHE
*/
bool _NOR_cache_flush (NOR_CACHE* NOR_CACHE);

/*
Clear out the contents of the NOR_CACHE without writing any dirty sectors first
*/
void _NOR_cache_invalidate (NOR_CACHE* NOR_CACHE);

NOR_CACHE* _NOR_cache_constructor (unsigned int numberOfPages, unsigned int sectorsPerPage, const DISC_INTERFACE* discInterface, sec_t endOfPartition, sec_t sectorSize);

void _NOR_cache_destructor (NOR_CACHE* NOR_CACHE);

#endif // _CACHE_H

