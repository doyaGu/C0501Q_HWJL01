#include "../ith/ith_cfg.h"

#include "inc/configs.h"
#include "inc/NFDrv.h"

#ifdef CFG_SPI_NAND

#include "inc/NF_SPI_NAND.h"

		#define NFInitialize          SpiNf_Initialize
		#define NFGetwear		      SpiNf_Getwear
		#define NFErase			      SpiNf_Erase	
		#define NFWrite			      SpiNf_Write
		#define NFLbaWrite			  SpiNf_LBA_Write
		#define NFWriteDouble         SpiNf_WriteDouble
		#define NFRead			      SpiNf_Read
		#define NFLbaRead			  SpiNf_LBA_Read
		#define NFReadPart            SpiNf_ReadPart
		#define NFIsBadBlock          SpiNf_IsBadBlock
		#define NFIsBadBlockForBoot   SpiNf_IsBadBlockForBoot
		#define NFReadOneByte         SpiNf_ReadOneByte
		#define NFGetAttribute        SpiNf_GetAttribute
		#define NFSetreservedblocks   SpiNf_SetReservedBlocks
		#define NFSetSpiCsCtrl	  	  SpiNf_SetSpiCsCtrl
#else

#include "inc/NF_LB_HWECC.h"

#ifdef NF_LARGEBLOCK_2KB

		#define NFInitialize          LB_HWECC_Initialize
		#define NFGetwear		      LB_HWECC_Getwear
		#define NFErase			      LB_HWECC_Erase	
		#define NFWrite			      LB_HWECC_Write
		#define NFLbaWrite			  LB_HWECC_LBA_Write
		#define NFWriteDouble         LB_HWECC_WriteDouble
		#define NFRead			      LB_HWECC_Read
		#define NFLbaRead			  LB_HWECC_LBA_Read
		#define NFReadPart            LB_HWECC_ReadPart
        #define NFIsBadBlock          LB_HWECC_IsBadBlock
		#define NFIsBadBlockForBoot   LB_HWECC_IsBadBlockForBoot
        #define NFReadOneByte         LB_HWECC_ReadOneByte
		#define NFGetAttribute        LB_HWECC_GetAttribute
		#define NFSetSpiCsCtrl	  LB_HWECC_SetSpiCsCtrl
#endif	//#ifdef NF_LARGEBLOCK_8KB
	#endif

#define	ithStorgeNand ITH_STOR_NAND

#ifdef CFG_SPI_NAND
#define NFSetreservedblocks   SpiNf_SetReservedBlocks
#else
#ifdef	NF_PITCH_RESERVED_BLOCK_ISSUE
#define NFSetreservedblocks   LB_HWECC_Setreservedblocks
#endif
#endif
////////////////////////////////////////////////////////////
//
//	Global Variable
//
////////////////////////////////////////////////////////////
static void    *NF_Semaphore = MMP_NULL;

#ifdef CFG_SPI_NAND
//create this mutex to prevent from that itp_nand.c & ftldrv.c access concurrently
static	pthread_mutex_t     NF_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
////////////////////////////////////////////////////////////
//
//	Const variable or Marco
//
////////////////////////////////////////////////////////////
enum {
	LL_OK,
	LL_ERASED,
	LL_ERROR,
	LL_RETRY
};
////////////////////////////////////////////////////////////
//
//	Function Implementation
//
////////////////////////////////////////////////////////////

MMP_UINT8
mmpNFInitialize(
    unsigned long* pNumOfBlocks,
    unsigned char* pPagePerBlock,
    unsigned long* PageSize)
{
	MMP_UINT8	result;

    #ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);
	
	result = NFInitialize(pNumOfBlocks, pPagePerBlock, PageSize);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}

void
mmpNFGetwear(
    MMP_UINT32 blk,
    MMP_UINT8 *dest)
{
	NFGetwear(blk, dest);
}

#ifdef	NF_PITCH_RESERVED_BLOCK_ISSUE
MMP_UINT8
mmpNFSetreservedblocks(
    unsigned int blk)
{
	return NFSetreservedblocks(blk);
}
#endif

MMP_UINT8
mmpNFErase(
    MMP_UINT32 pba)
{  
	MMP_UINT8	result;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	result=NFErase(pba);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	
	return result;
}

MMP_UINT8
mmpNFWrite(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer,
    MMP_UINT8* pSpareBuffer)
{
	MMP_UINT8	result;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	result=NFWrite(pba, ppo, pDataBuffer, pSpareBuffer);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}		

MMP_UINT8
mmpNFWriteDouble(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 *buffer0,
    MMP_UINT8 *buffer1)
{
	MMP_UINT8	result;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	result=NFWriteDouble(pba, ppo, buffer0, buffer1);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}

MMP_UINT8
mmpNFRead(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* buffer)
{
	MMP_UINT8	result,RetryCount=0;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	do
	{		
		result=NFRead(pba, ppo, buffer);

		//printf("mmpR:R=%x.\r\n",result);
		
		RetryCount++;		
		if(RetryCount==3)
		{
			printf("mmpR:R=ERROR\r\n");
			result=LL_ERROR;
			break;
		}
	}while(result==LL_RETRY);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}

MMP_UINT8
mmpNFReadPart(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 *buffer,
    MMP_UINT8 index)
{
	MMP_UINT8	result,RetryCount=0;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	do
	{		
		result=NFReadPart(pba, ppo, buffer, index);

		//printf("mmpRp:R=%x.\r\n",result);
		
		RetryCount++;
		if(RetryCount==3)
		{
			printf("mmpR:Rp=ERROR\r\n");
			result=LL_ERROR;
			break;
		}
	}while(result==LL_RETRY);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	

	return result;
}

MMP_UINT8
mmpNFIsBadBlock(
    MMP_UINT32 pba)
{
	MMP_UINT8	result;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	result=NFIsBadBlock(pba);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}

MMP_UINT8
mmpNFIsBadBlockForBoot(
    MMP_UINT32 pba)
{
	MMP_UINT8	result;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	result=NFIsBadBlockForBoot(pba);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}

MMP_UINT8
mmpNFReadOneByte(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 sparepos,
    MMP_UINT8 *ch)
{
	MMP_UINT8	result,RetryCount=0;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	do
	{		
		result=NFReadOneByte(pba, ppo, sparepos, ch);

		//printf("mmpR1b:R=%x,RT=%x.\r\n",result,LL_RETRY);
		
		RetryCount++;		
		if(RetryCount==3)
		{
			printf("mmpR1b:R=ERROR\r\n");
			result=LL_ERROR;
			break;
		}
	}while(result==LL_RETRY);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}

void 
mmpNFGetAttribute(
	MMP_BOOL *NfType,
	MMP_BOOL *EccStatus)
{
	NFGetAttribute(NfType, EccStatus);
}


MMP_UINT8
mmpNFLbaRead(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* buffer)
{
	MMP_UINT8	result,RetryCount=0;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
    
	ithStorageSelect(ithStorgeNand);

	do
	{		
		result=NFLbaRead(pba, ppo, buffer);

		//printf("mmpR:R=%x.\r\n",result);
		
		RetryCount++;		
		if(RetryCount==3)
		{
			printf("mmpR:R=ERROR\r\n");
			result=LL_ERROR;
			break;
		}
	}while(result==LL_RETRY);

	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}


MMP_UINT8
mmpNFLbaWrite(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer,
    MMP_UINT8* pSpareBuffer)
{
	MMP_UINT8	result;

	#ifdef	CFG_SPI_NAND
    pthread_mutex_lock(&NF_mutex);
    #else
    ithLockMutex(ithStorMutex);
    #endif
	ithStorageSelect(ithStorgeNand);

	result=NFLbaWrite(pba, ppo, pDataBuffer, pSpareBuffer);
	
	#ifdef	CFG_SPI_NAND
	pthread_mutex_unlock(&NF_mutex);
	#else
	ithUnlockMutex(ithStorMutex);
	#endif
	
	return result;
}	

DLLAPI void
mmpEnableSpiCsCtrl(
	MMP_UINT32 EnCtrl)
{
	NFSetSpiCsCtrl(EnCtrl);
}	



