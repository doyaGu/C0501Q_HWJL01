/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  NAND flash extern API implementation.
 *
 * @author Joseph
 */

//=============================================================================
//                              Include Files
//=============================================================================
//#include "config.h"
#include "ite/ith.h"
#include "inc/NFDrv.h"
#include "../include/ite/ite_nand.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define 	BOOT_ROM_RESERVED_SIZE		CFG_NAND_RESERVED_SIZE	//16MB(*2)=32MB(for real nand space, the most small nand size of 2kB/page is 128MB)


#define NF_API
//=============================================================================
//                              Structure Definition
//=============================================================================
/*
typedef struct NF_INFO_TAG
{
	uint32_t   Init;
	uint32_t   CurBlk;
	uint32_t   BootRomSize;

	uint32_t   NumOfBlk;
    uint32_t   PageInBlk;
    uint32_t   PageSize;
    uint32_t   SprSize;    
}NF_INFO;
*/

enum {
	LL_OK,
	LL_ERASED,
	LL_ERROR,
	LL_RETRY
};
//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================


//=============================================================================
//                              Private Function Definition
//=============================================================================
////////////////////////////////////////////////////////////
//
//	Function Implementation
//
////////////////////////////////////////////////////////////
NF_API void
iteNfGetAttribute(
	uint8_t *NfType,
	uint8_t *EccStatus)
{
	MMP_BOOL type;
	MMP_BOOL status;

	mmpNFGetAttribute(&type, &status);

	*NfType		= (uint8_t)type;
	*EccStatus  = (uint8_t)status;
}

NF_API void
iteNfGetwear(
    uint32_t blk,
    uint8_t *dest)
{
	mmpNFGetwear(blk, dest);
}

NF_API int
iteNfInitialize(ITE_NF_INFO *Info)
//iteNfInitialize(int *Info)
{
	int	result = -1;
	unsigned long blks=0;
	unsigned char Pgs=0;
	unsigned long PgSize=0;
	unsigned long revBlks=0;
	
	mmpEnableSpiCsCtrl(Info->enSpiCsCtrl);
	
	if(!mmpNFInitialize(&blks, &Pgs, &PgSize))
	{
		Info->NumOfBlk = (uint32_t)blks;
		Info->PageInBlk = (uint32_t)Pgs;
		Info->PageSize = (uint32_t)PgSize;
		
		if(Info->BootRomSize)
		{
			uint32_t _blkSize = (uint32_t)(Pgs*PgSize);
			if(_blkSize)
			{
				revBlks = Info->BootRomSize/_blkSize;
				if(Info->BootRomSize%_blkSize)	revBlks++;
			}
		}
		
		if( !mmpNFSetreservedblocks(revBlks) )
		{
		
			if(PgSize == 2048)
#ifdef CFG_SPI_NAND
				Info->SprSize   = 24;
#else
				Info->SprSize   = 16;
#endif
			else
				Info->SprSize   = 24;

			result = true;	
		}
	}

    return result;
}

NF_API int
iteNfRead(
    unsigned long pba,
    unsigned long ppo,
    unsigned char* buffer)
{
	int	result = -1;
	
	if(mmpNFRead(pba, ppo, buffer)!=LL_ERROR)
	{
		result = true;			
	}

	return result;
}

NF_API int
iteNfReadOneByte(
    unsigned long pba,
    unsigned long ppo,
    unsigned char sparepos,
    unsigned char *ch)
{
	int	result = -1;
	
	if(mmpNFReadOneByte(pba, ppo, sparepos, ch)!=LL_ERROR)
	{
		result = true;		
	}
	
	return result;
}

NF_API int
iteNfReadPart(
    unsigned long pba,
    unsigned long ppo,
    unsigned char *buffer,
    unsigned char index)
{
	int	result = -1;

	if(mmpNFReadPart(pba, ppo, buffer, index)!=LL_ERROR)
	{
		result = true;		
	}

	return result;
}

NF_API int
iteNfWrite(
    unsigned long pba,
    unsigned long ppo,
    unsigned char* pDataBuffer,
    unsigned char* pSpareBuffer)
{
	int	result = -1;

	if( mmpNFWrite(pba, ppo, pDataBuffer, pSpareBuffer)==LL_OK )
	{
		result = true;		
	}

	return result;
}

NF_API int
iteNfWriteDouble(
    unsigned long pba,
    unsigned long ppo,
    unsigned char *buffer0,
    unsigned char *buffer1)
{
	int	result = -1;
	
	if(mmpNFWriteDouble(pba, ppo, buffer0, buffer1)==LL_OK )
	{
		result = true;		
	}		

	return result;
}

NF_API int
iteNfErase(
    unsigned long pba)
{  
	int	result = -1;

	if(mmpNFErase(pba)==LL_OK )
	{
		result = true;		
	}

	return result;
}

NF_API int
iteNfIsBadBlock(
    unsigned long pba)
{
	int	result = -1;

	if(mmpNFIsBadBlock(pba)==LL_OK)
	{
		result = true;		
	}

	return result;
}

NF_API int
iteNfIsBadBlockForBoot(
    unsigned long pba)
{
	int	result = -1;

	if(mmpNFIsBadBlockForBoot(pba)==LL_OK)
	{
		result = true;		
	}
	
	return result;
}

NF_API int
iteNfLbaRead(
    unsigned long pba,
    unsigned long ppo,
    unsigned char* buffer)
{
	int	result = -1;
	
	if(mmpNFLbaRead(pba, ppo, buffer)==MMP_TRUE)
	{
		result = true;		
	}
	
	return result;
}

NF_API int
iteNfLbaWrite(
    unsigned long pba,
    unsigned long ppo,
    unsigned char* pDataBuffer,
    unsigned char* pSpareBuffer)
{
	int	result = -1;

	if( mmpNFLbaWrite(pba, ppo, pDataBuffer, pSpareBuffer)==LL_OK )
	{
		result = true;		
	}

	return result;
}	

NF_API int iteNfTerminate(void)
{
	return true;		
}