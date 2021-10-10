#include <stdio.h>
#include "vgmem.h"
#include "iteDefs.h"
#include "iteHardware.h"
#include "nedmalloc.h"

//=============================================================================
//                              Compile Option
//=============================================================================
//#define _VG_MEM_SHOW_MEM_STATUS

//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct _VG_MEM_BLOCK
{
	unsigned long			objID;
	unsigned long			blockAddr;
	unsigned long			blockSize;
	unsigned long			bufferAddr;
	unsigned long			bufferAddrAlign;
	unsigned long			bufferAlignValue;
	unsigned long			bufferRealSize;
	unsigned long			bufferSize;
	struct _VG_MEM_BLOCK*	prev;
	struct _VG_MEM_BLOCK*	next;
}VG_MEM_BLOCK;

typedef struct _VG_MEM_ALLOCATOR
{
	//int            vgMemPoolHandle;
	nedpool*       vgMemPoolHandle; 
	unsigned char* poolBuffer;
	unsigned long  poolBufferSize;
	VG_MEM_BLOCK*  head;
	VG_MEM_BLOCK*  tail;
	VG_MEM_BLOCK*  curr;
}VG_MEM_ALLOCATOR;

//=============================================================================
//                              Global Data Definition
//=============================================================================
VG_MEM_ALLOCATOR* VgMemAllocator = NULL;

//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Private Function Definition
//=============================================================================
void
_ShowMemoryStatus(
	VG_MEM_ALLOCATOR* memAllocator)
{
	VG_MEM_BLOCK* currBlock = memAllocator->head;

	//printf(" [  Prev  ]    [   Node   ]    [  Next  ]\n");
	while ( currBlock != NULL )
	{
		//printf(" 0x%08X    [0x%08X]    0x%08X\n", currBlock->prev, currBlock, currBlock->next);
		currBlock = currBlock->next;
	}
}

bool
_AddBlockToTail(
	VG_MEM_ALLOCATOR* memAllocator,
	VG_MEM_BLOCK*     memBlock)
{
	bool returnValue = false;;
	
	if ( memAllocator )
	{
		if (   memAllocator->head == NULL
		    && memAllocator->tail == NULL )
		{
			//printf("%s[%d] head = 0x%08X, tail = 0x%08X, block = 0x%08X\n", __func__, __LINE__, memAllocator->head, memAllocator->tail, memBlock);
			memAllocator->head = memAllocator->tail = memBlock;
			//printf("%s[%d] head = 0x%08X, tail = 0x%08X, block = 0x%08X\n", __func__, __LINE__, memAllocator->head, memAllocator->tail, memBlock);
		}
		else if ( memAllocator->head == memAllocator->tail )
		{
			//printf("%s[%d] head = 0x%08X, tail = 0x%08X, block = 0x%08X\n", __func__, __LINE__, memAllocator->head, memAllocator->tail, memBlock);
			memAllocator->head->next = memBlock;
			memAllocator->tail = memBlock;
			memAllocator->tail->prev = memAllocator->head;
			//printf("%s[%d] head = 0x%08X, tail = 0x%08X, block = 0x%08X\n", __func__, __LINE__, memAllocator->head, memAllocator->tail, memBlock);
		}
		else
		{
			//printf("%s[%d] head = 0x%08X, tail = 0x%08X, block = 0x%08X\n", __func__, __LINE__, memAllocator->head, memAllocator->tail, memBlock);
			memAllocator->tail->next = memBlock;
			memBlock->prev = memAllocator->tail;
			memAllocator->tail = memBlock;
			//printf("%s[%d] head = 0x%08X, tail = 0x%08X, block = 0x%08X\n", __func__, __LINE__, memAllocator->head, memAllocator->tail, memBlock);
		}
		returnValue = true;

#ifdef _VG_MEM_SHOW_MEM_STATUS
		_ShowMemoryStatus(memAllocator);
#endif
	}

	return returnValue;
}

bool
_DeleteBlock(
	VG_MEM_ALLOCATOR* memAllocator,
	VG_MEM_BLOCK*     memBlock)
{
	bool returnValue = false;;
	
	if ( memAllocator )
	{
		VG_MEM_BLOCK* currBlock = memAllocator->head;

		while ( currBlock != NULL )
		{
			VG_MEM_BLOCK* nextBlock = currBlock->next;
			
			if ( currBlock == memBlock )
			{
				VG_MEM_BLOCK* thePrevBlock = currBlock->prev;
				VG_MEM_BLOCK* theNextBlock = currBlock->next;

				//printf("%s[%d] thePrevBlock = 0x%08X, theNextBlock = 0x%08X\n", __func__, __LINE__, thePrevBlock, theNextBlock);
				if ( thePrevBlock != NULL )
				{
					thePrevBlock->next = theNextBlock;
					//printf("%s[%d] thePrevBlock->next = 0x%08X\n", __func__, __LINE__, thePrevBlock->next);
				}
				if ( theNextBlock != NULL )
				{
					theNextBlock->prev = thePrevBlock;
					//printf("%s[%d] theNextBlock->prev = 0x%08X\n", __func__, __LINE__, theNextBlock->prev);
				}
				if ( currBlock == memAllocator->head )
				{
					memAllocator->head = theNextBlock;
					//printf("%s[%d] memAllocator->head = 0x%08X\n", __func__, __LINE__, memAllocator->head);
				}
				if ( currBlock == memAllocator->tail )
				{
					memAllocator->tail = theNextBlock;
					//printf("%s[%d] memAllocator->tail = 0x%08X\n", __func__, __LINE__, memAllocator->tail);
				}
				nedpfree(VgMemAllocator->vgMemPoolHandle, (void*)currBlock);
				returnValue = true;
				break;
			}
			currBlock = nextBlock;
		}
	}

	return returnValue;
}

void
_RecycleMemoryById(
	VG_MEM_ALLOCATOR* memAllocator,
	unsigned long     recycleMaxId)
{
	if ( memAllocator && memAllocator->head )
	{
		VG_MEM_BLOCK* currBlock = memAllocator->head;

		while ( currBlock )
		{
			VG_MEM_BLOCK* nextBlock = currBlock->next;
			
			if ( currBlock->objID <= recycleMaxId )
			{
				//printf("%s[%d] Recycle block 0x%08X\n", __func__, __LINE__, currBlock); 
				_DeleteBlock(memAllocator, currBlock);
#ifdef _VG_MEM_SHOW_MEM_STATUS
				_ShowMemoryStatus(memAllocator);
#endif
			}
			currBlock = nextBlock;
		}
	}
}

void
_RecycleMemoryByAddress(
	VG_MEM_ALLOCATOR* memAllocator,
	unsigned long     recycleAddress)
{
	if ( memAllocator && memAllocator->head )
	{
		VG_MEM_BLOCK* currBlock = memAllocator->head;

		while ( currBlock )
		{
			VG_MEM_BLOCK* nextBlock = currBlock->next;
			
			if ( currBlock->bufferAddrAlign == recycleAddress )
			{
				//printf("%s[%d] Recycle block 0x%08X\n", __func__, __LINE__, currBlock); 
				_DeleteBlock(memAllocator, currBlock);
#ifdef _VG_MEM_SHOW_MEM_STATUS
				_ShowMemoryStatus(memAllocator);
#endif
				break;
			}
			currBlock = nextBlock;
		}
	}
}

static unsigned long
_GetCurrentObjID()
{
	uint16_t value;

	value = ithReadRegH(ITH_OPENVG_BASE + ITE_VG_REG_BID2_BASE);
	
	return value;
}

//=============================================================================
//                              Function Definition
//=============================================================================
void
vgMemInit(
	unsigned long startAddr,
	unsigned long size)
{
	if ( !VgMemAllocator )
	{
		VgMemAllocator = (VG_MEM_ALLOCATOR*)VG_Malloc(sizeof(VG_MEM_ALLOCATOR));
		if ( VgMemAllocator )
		{
			memset(VgMemAllocator, 0x00, sizeof(VG_MEM_ALLOCATOR));

			//VgMemAllocator->vgMemPoolHandle = itpHeapPoolInit("openVG", (void*)startAddr, size);
			VgMemAllocator->vgMemPoolHandle = nedcreatepool(size, 0);
			VgMemAllocator->poolBuffer      = (unsigned char*)startAddr;
			VgMemAllocator->poolBufferSize  = size;
		}
	}
}

#if 1
void*
vgMalloc(
	unsigned long size,
	unsigned long objID)
{
	return vgMemalign(1, size, objID);
}
#else
void*
vgMalloc(
	unsigned long size,
	unsigned long objID)
{
	void* returnAddr = NULL;
	
	/* Initialize */
	if ( !VgMemAllocator )
	{
		void* addr = (void*)VG_VMemAlloc(VG_MEM_RESERVE_SIZE);
		vgMemInit((unsigned long)addr, VG_MEM_RESERVE_SIZE);
	}

	if (   VgMemAllocator && size 
		&& (size <= VG_MEM_RESERVE_SIZE) )
	{
		unsigned long  allocSize = sizeof(VG_MEM_BLOCK) + size;
		unsigned char* allocBuffer = (unsigned char*)itpHeapMalloc(VgMemAllocator->vgMemPoolHandle, allocSize);

		// Allocate buffer, set attributes
		if ( allocBuffer == NULL )
		{
			// If fail, recycle memory from list
			printf("%s[%d] Out of memory, recycle memory id less than %d!\n", __func__, __LINE__, objID);
			_RecycleMemoryById(VgMemAllocator, objID - 1);
			allocBuffer = (unsigned char*)itpHeapMalloc(VgMemAllocator->vgMemPoolHandle, allocSize);
		}

		if ( allocBuffer )
		{
			VG_MEM_BLOCK* memBlock = (VG_MEM_BLOCK*)allocBuffer;
			
			// Set attributes
			memset(memBlock, 0x00, sizeof(VG_MEM_BLOCK));
			memBlock->objID = objID;
			memBlock->blockAddr = (unsigned long)allocBuffer;
			memBlock->blockSize = allocSize;
			memBlock->bufferAddr = (unsigned long)(allocBuffer + sizeof(VG_MEM_BLOCK));
			memBlock->bufferAddrAlign = memBlock->bufferAddr;
			memBlock->bufferAlignValue = 1;
			memBlock->bufferRealSize = size;
			memBlock->bufferSize = size;

			// Add to list
			printf("%s[%d] Allocate block 0x%08X, buffer 0x%08X\n", __func__, __LINE__, memBlock, memBlock->bufferAddrAlign);
			_AddBlockToTail(VgMemAllocator, memBlock);

			// return
			returnAddr = memBlock->bufferAddrAlign;
		}
	}

	return returnAddr;
}
#endif

#if 1
void*
vgMemalign(
	unsigned long byteAlign,
	unsigned long size,
	unsigned long objID)
{
	void* returnAddr = NULL;
	
	/* Initialize */
	if ( !VgMemAllocator )
	{
		void* addr = (void*)VG_VMemAlloc(VG_MEM_RESERVE_SIZE);
		vgMemInit((unsigned long)addr, VG_MEM_RESERVE_SIZE);
	}

	if (   VgMemAllocator && size 
		&& ((size + byteAlign) <= VG_MEM_RESERVE_SIZE) )
	{
		unsigned long  allocSize = sizeof(VG_MEM_BLOCK) + size + byteAlign;
		//unsigned char* allocBuffer = (unsigned char*)itpHeapMalloc(VgMemAllocator->vgMemPoolHandle, allocSize);
		unsigned char* allocBuffer = (unsigned char*)nedpmalloc(VgMemAllocator->vgMemPoolHandle, allocSize);

		// Allocate buffer, set attributes
		if ( allocBuffer == NULL )
		{
			// If fail, recycle memory from list
			printf("%s[%d] Out of memory, recycle memory id less than %d!\n", __func__, __LINE__, objID);
			_RecycleMemoryById(VgMemAllocator, objID - 1);
			allocBuffer = (unsigned char*)nedpmalloc(VgMemAllocator->vgMemPoolHandle, allocSize);
		}

		if ( allocBuffer )
		{
			VG_MEM_BLOCK* memBlock = (VG_MEM_BLOCK*)allocBuffer;
			
			// Set attributes
			memset(memBlock, 0x00, sizeof(VG_MEM_BLOCK));
			memBlock->objID = objID;
			memBlock->blockAddr = (unsigned long)allocBuffer;
			memBlock->blockSize = allocSize;
			memBlock->bufferAddr = (unsigned long)(allocBuffer + sizeof(VG_MEM_BLOCK));
			memBlock->bufferAddrAlign = ((ITEuint32)memBlock->bufferAddr + (byteAlign-1)) & ~(byteAlign-1);;
			memBlock->bufferAlignValue = byteAlign;
			memBlock->bufferRealSize = size + byteAlign;
			memBlock->bufferSize = size;

			// Add to list
			//printf("%s[%d] Allocate block 0x%08X, buffer 0x%08X\n", __func__, __LINE__, memBlock, memBlock->bufferAddrAlign);
			_AddBlockToTail(VgMemAllocator, memBlock);

			// return
			returnAddr = memBlock->bufferAddrAlign;
		}
	}

	return returnAddr;
}
#else
void*
vgMemalign(
	unsigned long byteAlign,
	unsigned long size,
	unsigned long objID)
{
	unsigned long allocSize = size + byteAlign;

	allocSize = ((allocSize + 3) >> 2) << 2;

	/* Initialize */
	if ( !VgMemAllocator )
	{
		void* addr = (void*)VG_VMemAlloc(VG_MEM_RESERVE_SIZE);
		vgMemInit((unsigned long)addr, VG_MEM_RESERVE_SIZE);
	}

	if (   VgMemAllocator && size 
		&& (size <= VG_MEM_RESERVE_SIZE) )
	{
		/* Search free block */
		do
		{
			VG_MEM_BLOCK* mb = NULL;
			
			mb = _SearchUsefulMemBlock(allocSize);
			if ( mb )
			{
				/* Found */
				VG_MEM_BLOCK* newMb = _SplitMemBlock(mb, allocSize);
				if ( newMb )
				{
					newMb->objID = objID;
					newMb->addrAlign = ((ITEuint32)newMb->addr + (byteAlign-1)) & ~(byteAlign-1);
					newMb->alignValue = byteAlign;
					return (void*)newMb->addrAlign;
				}
			}
			else
			{
				/* No block found, recycle */
				_RecycleMemBlockID(objID);
			}
		}while(1);
	}
	else
	{
		return NULL;
	}
}
#endif

void
vgFree(
   void* addr)
{
	if ( addr )
	{
		_RecycleMemoryByAddress(VgMemAllocator, (unsigned long)addr);
	}
}
