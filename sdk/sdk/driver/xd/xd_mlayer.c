#include "ite/ith.h"
#include "xd/xd_mlayer.h"
#include "xd/xd_binaryheap.h"

////////////////////////////////////////////////////////////
//
//	Compile Option
//
////////////////////////////////////////////////////////////
//#define GENTABLE_READ_BUFFER

////////////////////////////////////////////////////////////
//
// Constant variable or Marco
//
////////////////////////////////////////////////////////////
#define XD_LOGIC_BLOCK_PER_ZONE  1000
#define XD_PHYSIC_BLOCK_PER_ZONE 1024
#define XD_INVALID_BLOCK_NUMBER  0xFFFFFFFF
#define XD_INVALID_ZONE_NUMBER   0xFFFFFFFF

#define XD_SHARE_SEMAPHORE

////////////////////////////////////////////////////////////
//
//	Global Variable
//
////////////////////////////////////////////////////////////
static F_DRIVER       xd_driver;
static MMP_UINT32     g_NumOfBlocks = 0;
static MMP_UINT8      g_EvenParityTable[256];
static MMP_UINT32     g_LastUseZone = XD_INVALID_BLOCK_NUMBER;
static MMP_UINT32     g_TranslationTable[XD_PHYSIC_BLOCK_PER_ZONE];


MMP_UINT32     g_CisBlock = 0;
MMP_UINT32     g_NumOfUsableBlocks = 0;
MMP_UINT32*    g_pXdConversionTable = MMP_NULL;
XD_BINARYHEAP* g_pXdEraseBlockHeap  = MMP_NULL;

//static void    *XD_Semaphore = MMP_NULL;

////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
static MMP_BOOL  IsValidXdCard();
static MMP_UINT8 NumOfZeroBits(MMP_UINT8 ch);
static MMP_BOOL  IsNormalBlock(MMP_UINT8 ch);
static MMP_BOOL  IsValidSector(MMP_UINT8 ch);
static void      InitEvenParityTable();
static MMP_UINT8 GetEvenParity(MMP_UINT8 ch);
static MMP_BOOL  CheckEvenParity(MMP_UINT8 ch1, MMP_UINT8 ch2);
static MMP_BOOL  GetLogicalAddress(MMP_UINT8* pBuffer, MMP_UINT16* pAddr);
static MMP_UINT8 XD_ML_Initialize();
static MMP_UINT8 XD_ML_Terminate();
static void      UpdateSpare(MMP_UINT8* pData, MMP_UINT8* pSpare, MMP_UINT32 logicalAddr);
static MMP_BOOL  GenerateTranslationTable(MMP_UINT32 zone, MMP_UINT32 logicalBlock, MMP_UINT32* pPhyBlock);

////////////////////////////////////////////////////////////
//
// Function Implementation
//
////////////////////////////////////////////////////////////
void InitEvenParityTable()
{
	MMP_UINT16 i, j;

	for ( j=0; j<=0xFF; j++ )
	{
		MMP_UINT8  num = 0;
		for ( i=0; i<8; i++ )
		{
			if ( (j >> i) & 0x01 )
				num++;
		}
		g_EvenParityTable[j] = num;
	}
}

MMP_UINT8 GetEvenParity(MMP_UINT8 ch)
{
	return g_EvenParityTable[ch];
}

MMP_UINT8
XD_ML_Initialize()
{
    MMP_UINT8 result  = XD_ERROR;

	// Initial XD card
	//HOST_StorageIoSelect(MMP_CARD_XD_IP);
	ithStorageSelect(ITH_STOR_XD);
	result = XD_Initialize(&g_NumOfBlocks);
	if ( result != XD_OK )
	{
	    //printf("XD_ML_Initialize: XD_Initialize() failed\n");
		result = XD_ERROR;
		goto end;
	}

	// Initial Ecc Table
	if ( XD_EccInitial() == MMP_FALSE )
	{
		printf("XD_ML_Initialize: Initial Ecc Table Failed.\n");
		result = XD_ERROR;
		goto end;
	}

	// Initial parity table
	InitEvenParityTable();

	if ( IsValidXdCard() == MMP_FALSE)
	{
	    printf("XD_ML_Initialize: Not a valid card.\n");
		result = XD_ERROR;
		goto end;
	}

#if 0
    {
        MMP_UINT32 i;
        for ( i=(g_CisBlock + 1); i<g_NumOfBlocks; i++ )
        {
            if ( XD_Erase(i) != XD_OK )
                printf("Erase block %u fail\n", i);
        }
        printf("End of erase XD card.\n");
        while(1)
        {
            printf("");
        }
    }
#endif

	// Initial g_TranslationTable
	g_LastUseZone = XD_INVALID_BLOCK_NUMBER;
	memset(g_TranslationTable, 0xFF, XD_PHYSIC_BLOCK_PER_ZONE * sizeof(MMP_UINT32));


end:
	return result;
}

MMP_UINT8
XD_ML_Terminate()
{
	g_NumOfBlocks==0x00;
	if ( g_pXdEraseBlockHeap != MMP_NULL )
	{
		XD_DeleteHeap(g_pXdEraseBlockHeap);
		g_pXdEraseBlockHeap = MMP_NULL;
	}

	g_LastUseZone = XD_INVALID_BLOCK_NUMBER;

	return XD_OK;
}

MMP_BOOL
IsValidXdCard()
{
	MMP_UINT8 i, j, k;
	MMP_UINT8 result    = XD_ERROR;
	MMP_BOOL  findValid = MMP_FALSE;
	MMP_UINT8 buffer[XD_DATA_SIZE];
	MMP_UINT8 cisTop10[] = { 0x01, 0x03, 0xD9, 0x01, 0xFF, 0x18, 0x02, 0xDF, 0x01, 0x20 };
	const MMP_UINT8 maxNumOfSearchBlocks = 23;
	const MMP_UINT8 maxNumOfSearchPages  = 8;

	// Step A
	for ( i=0; i<maxNumOfSearchBlocks; i++ )
	{
		result = XD_ReadSingle(i, 0, buffer);
		if ( result == XD_OK )
		{
			if ( IsNormalBlock(buffer[XD_BLOCK_STATUS_FLAG]) )
			{	// 
				findValid = MMP_TRUE;
				g_CisBlock = i;
				break;
			}
		}
	}

	// Step B
	if ( findValid == MMP_TRUE )
	{
		findValid = MMP_FALSE;
		if ( IsValidSector(buffer[XD_DATA_STATUS_FLAG]) )
		{
			findValid = MMP_TRUE;
		}
		else
		{
			for( j=1; j<maxNumOfSearchPages; j++ )
			{
				if ( XD_ReadSingle(i, j, buffer) == XD_OK )
				{
					if ( IsValidSector(buffer[XD_DATA_STATUS_FLAG]) )
					{
						findValid = MMP_TRUE;
					}
				}
			}
		}
	}

	// Step C
	if ( findValid == MMP_TRUE )
	{
		// Step 1, check ECC,
		// But we skip check ECC at this moment

		// Step 2, Check top 10 bytes
		for ( k=0; k<10; k++ )
		{
			if ( buffer[k] != cisTop10[k] )
			{
				findValid = MMP_FALSE;
				break;
			}
		}
	}

	return findValid;
}

MMP_UINT8
NumOfZeroBits(MMP_UINT8 ch)
{
	return (8 - g_EvenParityTable[ch]);
}

MMP_BOOL
IsNormalBlock(MMP_UINT8 ch)
{
	if ( NumOfZeroBits(ch) < 2 )
		return MMP_TRUE;
	else
		return MMP_FALSE;
}

MMP_BOOL
IsValidSector(MMP_UINT8 ch)
{
	if ( NumOfZeroBits(ch) < 4 )
		return MMP_TRUE;
	else
		return MMP_FALSE;
}

#ifdef GENTABLE_READ_BUFFER
MMP_BOOL
GetLogicalAddress(MMP_UINT8* pSpareBuffer, MMP_UINT16* pAddr)
{
	MMP_UINT8  field1_lo, field1_hi;
	MMP_UINT8  field2_lo, field2_hi;
	MMP_UINT16 field1, field2;
	MMP_BOOL   result;
	MMP_UINT16 addrField1, addrField2;

	if (   pSpareBuffer == MMP_NULL 
		|| pAddr   == MMP_NULL)
		return MMP_FALSE;
	
	field1_lo = pSpareBuffer[XD_BLOCK_ADDR_1_LO];
	field1_hi = pSpareBuffer[XD_BLOCK_ADDR_1_HI];
	field2_lo = pSpareBuffer[XD_BLOCK_ADDR_2_LO];
	field2_hi = pSpareBuffer[XD_BLOCK_ADDR_2_HI];
	field1 = (((MMP_UINT16)field1_lo) << 8) | field1_hi;
	field2 = (((MMP_UINT16)field2_lo) << 8) | field2_hi;
			
	if ( field1 == field2 )
	{	// Then check field1 parity
		if ( CheckEvenParity(field1_hi, field1_lo) )
		{
			addrField1 = (field1 & 0x7FE) >> 1;

			if ( addrField1 < 1000 )
			{
				*pAddr = addrField1;
				result = MMP_TRUE;
			}
			else
			{
				printf("GetLogicalAddress: Get Invalid Logical Block Num %d.\n", addrField1);
				*pAddr = MMP_NULL;
				result = MMP_FALSE;
			}
		}
		else
		{
			*pAddr = MMP_NULL;
			result = MMP_FALSE;
		}
	}
	else
	{	// Then check field2 parity
		if ( CheckEvenParity(field2_lo, field2_hi) )
		{
			addrField2 = (field2 & 0x7FE) >> 1;

			if ( addrField2 < 1000 )
			{	// Within address range
				if ( CheckEvenParity(field1_lo, field1_hi) )
				{
					addrField1 = (field1 & 0x7FE) >> 1;

					if ( addrField1 < 1000 )
					{
						*pAddr = MMP_NULL;
						result = MMP_FALSE;
					}
					else
					{
						*pAddr = addrField2;
						result = MMP_TRUE;
					}
				}
				else
				{
					*pAddr = addrField2;
					result = MMP_TRUE;
				}
			}
			else
			{
				if ( CheckEvenParity(field1_lo, field1_hi) )
				{
					addrField1 = (field1 & 0x7FE) >> 1;

					if ( addrField1 < 1000 )
					{	// Within address range
						*pAddr = addrField1;
						result = MMP_TRUE;
					}
					else
					{
						*pAddr = MMP_NULL;
						result = MMP_FALSE;
					}
				}
				else
				{
					*pAddr = MMP_NULL;
					result = MMP_FALSE;
				}
			}
		}
		else
		{
			if ( CheckEvenParity(field1_lo, field1_hi) )
			{
				addrField1 = (field1 & 0x7FE) >> 1;

				if ( addrField1 < 1000 )
				{	// Within address range
					*pAddr = addrField1;
					result = MMP_TRUE;
				}
				else
				{
					*pAddr = MMP_NULL;
					result = MMP_FALSE;
				}
			}
			else
			{
				*pAddr = MMP_NULL;
				result = MMP_FALSE;
			}
		}
	}

	return result;
}
#else
MMP_BOOL
GetLogicalAddress(MMP_UINT8* pSpareBuffer, MMP_UINT16* pAddr)
{
	MMP_UINT8  field1_lo, field1_hi;
	MMP_UINT8  field2_lo, field2_hi;
	MMP_UINT16 field1, field2;
	MMP_BOOL   result;
	MMP_UINT16 addrField1, addrField2;

	if (   pSpareBuffer == MMP_NULL 
		|| pAddr   == MMP_NULL)
		return MMP_FALSE;
	
	field1_lo = pSpareBuffer[XD_BLOCK_ADDR_1_LO_OFFSET];
	field1_hi = pSpareBuffer[XD_BLOCK_ADDR_1_HI_OFFSET];
	field2_lo = pSpareBuffer[XD_BLOCK_ADDR_2_LO_OFFSET];
	field2_hi = pSpareBuffer[XD_BLOCK_ADDR_2_HI_OFFSET];
	field1 = (((MMP_UINT16)field1_lo) << 8) | field1_hi;
	field2 = (((MMP_UINT16)field2_lo) << 8) | field2_hi;
			
	if ( field1 == field2 )
	{	// Then check field1 parity
		if ( CheckEvenParity(field1_hi, field1_lo) )
		{
			addrField1 = (field1 & 0x7FE) >> 1;

			if ( addrField1 < 1000 )
			{
				*pAddr = addrField1;
				result = MMP_TRUE;
			}
			else
			{
				printf("GetLogicalAddress: Get Invalid Logical Block Num %d.\n", addrField1);
				*pAddr = MMP_NULL;
				result = MMP_FALSE;
			}
		}
		else
		{
			*pAddr = MMP_NULL;
			result = MMP_FALSE;
		}
	}
	else
	{	// Then check field2 parity
		if ( CheckEvenParity(field2_lo, field2_hi) )
		{
			addrField2 = (field2 & 0x7FE) >> 1;

			if ( addrField2 < 1000 )
			{	// Within address range
				if ( CheckEvenParity(field1_lo, field1_hi) )
				{
					addrField1 = (field1 & 0x7FE) >> 1;

					if ( addrField1 < 1000 )
					{
						*pAddr = MMP_NULL;
						result = MMP_FALSE;
					}
					else
					{
						*pAddr = addrField2;
						result = MMP_TRUE;
					}
				}
				else
				{
					*pAddr = addrField2;
					result = MMP_TRUE;
				}
			}
			else
			{
				if ( CheckEvenParity(field1_lo, field1_hi) )
				{
					addrField1 = (field1 & 0x7FE) >> 1;

					if ( addrField1 < 1000 )
					{	// Within address range
						*pAddr = addrField1;
						result = MMP_TRUE;
					}
					else
					{
						*pAddr = MMP_NULL;
						result = MMP_FALSE;
					}
				}
				else
				{
					*pAddr = MMP_NULL;
					result = MMP_FALSE;
				}
			}
		}
		else
		{
			if ( CheckEvenParity(field1_lo, field1_hi) )
			{
				addrField1 = (field1 & 0x7FE) >> 1;

				if ( addrField1 < 1000 )
				{	// Within address range
					*pAddr = addrField1;
					result = MMP_TRUE;
				}
				else
				{
					*pAddr = MMP_NULL;
					result = MMP_FALSE;
				}
			}
			else
			{
				*pAddr = MMP_NULL;
				result = MMP_FALSE;
			}
		}
	}

	return result;
}
#endif

MMP_BOOL
CheckEvenParity(MMP_UINT8 hi, MMP_UINT8 lo)
{
	MMP_UINT8 evenParitylo = GetEvenParity(lo);
	MMP_UINT8 evenParityhi = GetEvenParity((MMP_UINT8)(hi & 0xFE));

	if (   ((evenParitylo + evenParityhi) % 2 == 0 && (hi & 0x01) == 0x00)
		|| ((evenParitylo + evenParityhi) % 2 == 1 && (hi & 0x01) == 0x01) )
		return MMP_TRUE;
	else
		return MMP_FALSE;
}

#if 1
F_DRIVER* f_xdinit(unsigned long driver_param)
{
	memset (&xd_driver, 0x00, sizeof(xd_driver));

	xd_driver.readsector          = xddrv_readsector;
	xd_driver.writesector         = xddrv_writesector;
	xd_driver.readmultiplesector  = xddrv_readmultiplesector;
	xd_driver.writemultiplesector = xddrv_writemultiplesector;
	xd_driver.getphy              = xddrv_getphy;
	xd_driver.release             = xddrv_release;

	if (XD_ML_Initialize() != XD_OK) 
		return 0;;

	return &xd_driver;
}
#else
MMP_UINT8 f_xdinit(unsigned long driver_param)
{
	if (XD_ML_Initialize() != XD_OK) 
		return XD_ERROR;

    return XD_OK;
}
#endif

int xddrv_readsector(F_DRIVER* driver,void* data, unsigned long sector)
{
	MMP_UINT8  result         = XD_ERROR;
	MMP_UINT32 logicBlock     = sector / XD_PAGEPERBLOCK;
	MMP_UINT32 pureLogicBlock = logicBlock % XD_LOGIC_BLOCK_PER_ZONE;
	MMP_UINT32 zoneNumber     = logicBlock / XD_LOGIC_BLOCK_PER_ZONE;
	MMP_UINT32 phyBlock       = 0;
	MMP_UINT32 phyPage        = sector % XD_PAGEPERBLOCK;

	//HOST_StorageIoSelect(MMP_CARD_XD_IP);
	ithStorageSelect(ITH_STOR_XD);

	if (   driver == MMP_NULL
		|| data   == MMP_NULL)
	{
		printf("xddrv_readsector: Invalid Input Data. Abort!\n");
		result = XD_ERROR;
		goto end;
	}

	if ( zoneNumber != g_LastUseZone )
	{
		if ( GenerateTranslationTable(zoneNumber, pureLogicBlock, &phyBlock) == MMP_FALSE )
		{
			printf("xddrv_readsector: GenerateTranslationTable Failed. Abort!\n");
			result = XD_ERROR;
			goto end;
		}
		g_LastUseZone = zoneNumber;
	}
	else
	{
		phyBlock = g_TranslationTable[pureLogicBlock];
	}

	if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
	{
	    if(phyBlock>=0x400)
	    {
	        printf("3.phyBlock >= 0x400!!\r\n");
	    }
	    phyBlock=phyBlock+(zoneNumber*0x400);
		result = XD_ReadPage(phyBlock, phyPage, data);
		if ( result == XD_ERROR )
		{
			printf("xddrv_readsector: XD_Read() Read Front Data Back Failed. Abort!\n");
		}
		else
		{
			//see the result is OK include of result=ERASED
			result = XD_OK;
		}
	}
	else
	{
	    MMP_UINT16  j;
        MMP_UINT8*  pbuf=(MMP_UINT8*)data;
		//printf("No Mapping\r\n");
	    for(j=0;j<512;j++)
	    {
	        pbuf[j]=0xFF;
	    }	    
		result = XD_OK;
	}

end:
	return result;
}

int xddrv_readsector528(F_DRIVER* driver,void* data, unsigned long sector)
{
	MMP_UINT8  result         = XD_ERROR;
	MMP_UINT32 logicBlock     = sector / XD_PAGEPERBLOCK;
	MMP_UINT32 pureLogicBlock = logicBlock % XD_LOGIC_BLOCK_PER_ZONE;
	MMP_UINT32 zoneNumber     = logicBlock / XD_LOGIC_BLOCK_PER_ZONE;
	MMP_UINT32 phyBlock       = 0;
	MMP_UINT32 phyPage        = sector % XD_PAGEPERBLOCK;

	//HOST_StorageIoSelect(MMP_CARD_XD_IP);
	ithStorageSelect(ITH_STOR_XD);

	if (   driver == MMP_NULL
		|| data   == MMP_NULL)
	{
		printf("xddrv_readsector: Invalid Input Data. Abort!\n");
		result = XD_ERROR;
		goto end;
	}

	if ( zoneNumber != g_LastUseZone )
	{
		if ( GenerateTranslationTable(zoneNumber, pureLogicBlock, &phyBlock) == MMP_FALSE )
		{
			printf("xddrv_readsector: GenerateTranslationTable Failed. Abort!\n");
			result = XD_ERROR;
			goto end;
		}
		g_LastUseZone = zoneNumber;
	}
	else
	{
		phyBlock = g_TranslationTable[pureLogicBlock];
	}

	if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
	{
	    if(phyBlock>=0x400)
	    {
	        printf("4.phyBlock >= 0x400!!\r\n");
	    }

	    phyBlock=phyBlock+(zoneNumber*0x400);
		//result = XD_ReadSingle(phyBlock, phyPage, data);
		result = XD_ReadPage(phyBlock, phyPage, data);
		if ( result == XD_ERROR )
		{
			printf("xddrv_readsector: XD_Read() Read Front Data Back Failed. Abort!\n");
		}
		else
		{
			//see the result is OK include of result=ERASED
			result = XD_OK;
		}
	}
	else
	{
	    MMP_UINT16  j;
        MMP_UINT8*  pbuf=(MMP_UINT8*)data;
		//printf("No Mapping\r\n");

	    for(j=0;j<528;j++)
	    {
	        pbuf[j]=0xFF;
	    }
		result = XD_OK;
	}

end:
	return result;
}

int xddrv_writesector(F_DRIVER* driver,void *data, unsigned long sector)
{
	MMP_UINT8  result         = XD_ERROR;
	MMP_UINT32 logicBlock     = sector / XD_PAGEPERBLOCK;
	MMP_UINT32 zoneNumber     = logicBlock / XD_LOGIC_BLOCK_PER_ZONE;
	MMP_UINT32 pureLogicBlock = logicBlock % XD_LOGIC_BLOCK_PER_ZONE;
	MMP_UINT32 phyBlock       = 0;
	MMP_UINT32 phyPage        = sector % XD_PAGEPERBLOCK;
	MMP_INT    newBlock       = 0;
	MMP_BOOL   bReadBlock     = MMP_FALSE;
	//MMP_UINT8  readBackBuffer[XD_PAGEPERBLOCK][XD_DATA_SIZE];
	MMP_UINT8* pReadBackBufferAllocAddr = MMP_NULL;
    MMP_UINT8* pReadBackBuffer = MMP_NULL;
	MMP_UINT8  spareBuffer[XD_SPARE_SIZE];
	MMP_UINT32 i;

	//HOST_StorageIoSelect(MMP_CARD_XD_IP);
	ithStorageSelect(ITH_STOR_XD);

	if (   driver == MMP_NULL
		|| data   == MMP_NULL)
	{
		printf("xddrv_writesector: Invalid Input Data. Abort!\n");
		result = XD_ERROR;
		goto end;
	}

    pReadBackBufferAllocAddr = (MMP_UINT8*)malloc(XD_PAGEPERBLOCK * XD_DATA_SIZE + 4);
    if ( pReadBackBufferAllocAddr == MMP_NULL )
    {
        printf("xddrv_writesector: Allocate Memory Failed. Abort!\n");
		result = XD_ERROR;
		goto end;
    }
    pReadBackBuffer = (MMP_UINT8*)(((MMP_UINT32)pReadBackBufferAllocAddr + 3) & ~3);

	//////////////////////////////////////////////////////////////////////////
	// Step 1, Initial array
	memset(pReadBackBuffer, 0xFF, XD_PAGEPERBLOCK*XD_DATA_SIZE);
	memset(spareBuffer,    0xFF, XD_SPARE_SIZE);

	//////////////////////////////////////////////////////////////////////////
	// Step 2, Create translation table of used zone
	if ( zoneNumber != g_LastUseZone )
	{
		if ( GenerateTranslationTable(zoneNumber, pureLogicBlock, &phyBlock) )
		{
			g_LastUseZone = zoneNumber;
			bReadBlock    = MMP_TRUE;
		}
		else
		{
			printf("xddrv_writesector: GenerateTranslationTable Failed. Abort!\n");
			result = XD_ERROR;
			goto end;
		}
	}
	else
	{
		phyBlock = g_TranslationTable[pureLogicBlock];
	}

	if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
	{
	    if(phyBlock>=0x400)
	    {
	        printf("1.phyBlock>=0x400!!\r\n");
	    }
	    phyBlock=phyBlock+(zoneNumber*0x400);
	}
	else
	{
	    printf("1.PhyBlk=0xFFFF.\r\n");
	}

	//////////////////////////////////////////////////////////////////////////
	// Step 3, Get an empty block
    if ( XD_HeapPopMinElement(g_pXdEraseBlockHeap, &newBlock) == MMP_FALSE )
	{
		printf("xddrv_writesector: Can't Get Empty Block For Write. Abort!\n");
		result = XD_ERROR;
		goto end;
	}

	newBlock=newBlock+(zoneNumber*0x400);

	if ( bReadBlock == MMP_TRUE )
	{
		XD_ReadSeparate(newBlock, 0, MMP_NULL, MMP_NULL);
	}

	if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
	{
		//////////////////////////////////////////////////////////////////////////
		// Step 4, Read old data back
		// Read front block
		for ( i=0; i<phyPage; i++ )
		{
			result = XD_ReadSingle(phyBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE));
			if ( result == XD_ERROR )
			{
				printf("xddrv_writesector: XD_Read() Read Front Data Back Failed. Abort!\n");
				goto end;
			}
		}
		// Read rear block
		for ( i=(phyPage+1); i<XD_PAGEPERBLOCK; i++ )
		{
			result = XD_ReadSingle(phyBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE));
			if ( result == XD_ERROR )
			{
				printf("xddrv_writesector: XD_Read() Read Rear Data Back Failed. Abort!\n");
				goto end;
			}
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// Step 4, Prepare null data
		UpdateSpare(pReadBackBuffer, pReadBackBuffer + XD_PAGE_SIZE, pureLogicBlock);
	}

	//////////////////////////////////////////////////////////////////////////
	// Step 5, Prepare new data
	UpdateSpare(data, spareBuffer, pureLogicBlock);

	//////////////////////////////////////////////////////////////////////////
	// Step 6, Write data to new block
	// Step 6.1, Write front block
	if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
	{
		for ( i=0; i<phyPage; i++ )
		{
			result = XD_WriteSingle(newBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE),pureLogicBlock);
			if ( result == XD_ERROR )
			{
				printf("xddrv_writesector: XD_Write() Write Front Data Failed. Abort!\n");
				goto end;
			}
		}
	}
	else
	{
		for ( i=0; i<phyPage; i++ )
		{
			result = XD_WriteSingle(newBlock, i, pReadBackBuffer,pureLogicBlock);
			if ( result == XD_ERROR )
			{
				printf("xddrv_writesector: XD_Write() Write Front Data Failed. Abort!\n");
				goto end;
			}
		}
	}
	// Step 6.2, Write new data
	result = XD_WriteSeparate(newBlock, phyPage, data, spareBuffer,pureLogicBlock);
	if ( result == XD_ERROR )
	{
		printf("xddrv_writesector: XD_Write() Write New Data Failed. Abort!\n");
		goto end;
	}
	// Step 6.3, Write rear block
	if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
	{
		for ( i=(phyPage+1); i<XD_PAGEPERBLOCK; i++ )
		{
			result = XD_WriteSingle(newBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE),pureLogicBlock);
			if ( result == XD_ERROR )
			{
				printf("xddrv_writesector: XD_Write() Write Rear Data Failed. Abort!\n");
				goto end;
			}
		}
	}
	else
	{
		for ( i=(phyPage+1); i<XD_PAGEPERBLOCK; i++ )
		{
			result = XD_WriteSingle(newBlock, i, pReadBackBuffer,pureLogicBlock);
			if ( result == XD_ERROR )
			{
				printf("xddrv_writesector: XD_Write() Write Rear Data Failed. Abort!\n");
				goto end;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Step 7, Update Translation Table
	if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
	{
		if ( XD_Erase(phyBlock) == XD_OK )
		{
            XD_HeapInsertElement(g_pXdEraseBlockHeap, phyBlock&0x3FF);
		}
	}

    g_TranslationTable[pureLogicBlock] = (newBlock&0x3FF);


end:
    if ( pReadBackBufferAllocAddr )
    {
        free(pReadBackBufferAllocAddr);
        pReadBackBufferAllocAddr = MMP_NULL;
        pReadBackBuffer = MMP_NULL;
    }
    
	return result;
}

void 
UpdateSpare(
	MMP_UINT8* pData,
	MMP_UINT8* pSpare,
	MMP_UINT32 logicalAddr)
{
	MMP_UINT8  field1_lo, field1_hi;
	MMP_UINT8  field2_lo, field2_hi;
	MMP_UINT8  ecc1, ecc2, ecc3;

	if (   pData  == MMP_NULL 
		|| pSpare == MMP_NULL)
		return;
	
	field1_lo = (MMP_UINT8)(0x10 | ((logicalAddr & 0x3FF) >> 7));
	field1_hi = (((MMP_UINT8)logicalAddr) & 0x7F) << 1;

	if ( (GetEvenParity(field1_lo) + GetEvenParity(field1_hi)) % 2 )
	{
		field1_hi |= 0x01;
	}
	else
	{
		field1_hi |= 0x00;
	}

	field2_lo = field1_lo;
	field2_hi = field1_hi;

	 pSpare[XD_BLOCK_ADDR_1_LO_OFFSET] = field1_lo;
	 pSpare[XD_BLOCK_ADDR_1_HI_OFFSET] = field1_hi;
	 pSpare[XD_BLOCK_ADDR_2_LO_OFFSET] = field2_lo;
	 pSpare[XD_BLOCK_ADDR_2_HI_OFFSET] = field2_hi;
}

int xddrv_readmultiplesector(F_DRIVER* driver,void *data, unsigned long sector, int cnt)
{
	MMP_UINT8* pData  = (MMP_UINT8*)data;
	MMP_UINT8  result = XD_ERROR;
	MMP_UINT32 startSector = sector;
	MMP_UINT32 endSector   = sector+cnt-1;
	MMP_UINT32 i;
	MMP_UINT32 secCount = 0;

	//HOST_StorageIoSelect(MMP_CARD_XD_IP);
	ithStorageSelect(ITH_STOR_XD);

	for ( i=startSector; i<endSector; i++ )
	{
		result = xddrv_readsector528(driver, pData, i);
		if ( result == XD_ERROR )
		{
			printf("xd_readmultiplesector: xddrv_readsector() Read sector(%d) Error. Abort!\n", i);
			break;
		}
		pData += 512;
		if ( secCount++ > 32)
		{
		    //PalSleep(0);
			usleep(50);
		    secCount = 0;
		}
	}

	result = xddrv_readsector(driver, pData, endSector);
	if ( result == XD_ERROR )
	{
		printf("xd_readmultiplesector: xddrv_readsector() Read sector(%d) Error. Abort!\n", endSector+1);
	}

	return result;
}

int xddrv_writemultiplesector(F_DRIVER* driver,void *data, unsigned long sector, int cnt)
{
	MMP_UINT8  result         = XD_ERROR;
	MMP_UINT32 logicBlock     = 0;
	MMP_UINT32 zoneNumber     = 0;
	MMP_UINT32 pureLogicBlock = 0;
	MMP_UINT32 phyBlock       = 0;
	MMP_UINT32 phyPage        = 0;
	MMP_INT    newBlock       = 0;
	MMP_UINT32 writePageCount = 0;
	MMP_UINT8* pDataForUpdate = (MMP_UINT8*)data; // For update spare
	MMP_UINT8* pDataForWrite  = (MMP_UINT8*)data; // For write
	MMP_BOOL   bReadBlock     = MMP_FALSE;
	//MMP_UINT8  readBackBuffer[XD_PAGEPERBLOCK][XD_DATA_SIZE];
	MMP_UINT8* pReadBackBufferAllocAddr = MMP_NULL;
    MMP_UINT8* pReadBackBuffer = MMP_NULL;
	MMP_UINT8  nullBuffer[XD_DATA_SIZE];
	MMP_UINT32 i;

	//HOST_StorageIoSelect(MMP_CARD_XD_IP);
	ithStorageSelect(ITH_STOR_XD);

	if(sector==0x7ff3)
	{
	    printf("check Secter=0x7FF3 issue..\r\n");
	}

	//printf("LW1:%06x,lba=%x,Zn=%x,plba=%x,HE=%x.\r\n",sector,logicBlock,zoneNumber,pureLogicBlock,g_pXdEraseBlockHeap->Size);

	if (   driver == MMP_NULL
		|| data   == MMP_NULL)
	{
		printf("xd_writemultiplesector: Invalid Input Data. Abort!\n");
		result = XD_ERROR;
		goto end;
	}

    pReadBackBufferAllocAddr = (MMP_UINT8*)malloc(XD_PAGEPERBLOCK * XD_DATA_SIZE + 4);
    if ( pReadBackBufferAllocAddr == MMP_NULL )
    {
        printf("xd_writemultiplesector: Allocate Memory Failed. Abort!\n");
		result = XD_ERROR;
		goto end;
    }
    pReadBackBuffer = (MMP_UINT8*)(((MMP_UINT32)pReadBackBufferAllocAddr + 3) & ~3);

	while(cnt)
	{
		logicBlock     = sector / XD_PAGEPERBLOCK;
		zoneNumber     = logicBlock / XD_LOGIC_BLOCK_PER_ZONE;
		pureLogicBlock = logicBlock % XD_LOGIC_BLOCK_PER_ZONE;
		phyBlock       = 0;
		phyPage        = sector % XD_PAGEPERBLOCK;
		newBlock       = 0;
		writePageCount = XD_PAGEPERBLOCK - phyPage;
		bReadBlock     = MMP_FALSE;

		if ( writePageCount > (MMP_UINT32)cnt )
		{
			writePageCount = cnt;
		}

		//////////////////////////////////////////////////////////////////////////
		// Step 1, Initial array
		memset(pReadBackBuffer        , 0xFF, XD_PAGEPERBLOCK * XD_DATA_SIZE);
		memset(nullBuffer             , 0x55, XD_PAGE_SIZE);
		memset(nullBuffer+XD_PAGE_SIZE, 0xFF, XD_SPARE_SIZE);

		//////////////////////////////////////////////////////////////////////////
		// Step 2, Create translation table of used zone
		if ( zoneNumber != g_LastUseZone )
		{
			if ( GenerateTranslationTable(zoneNumber, pureLogicBlock, &phyBlock) )
			{
				g_LastUseZone = zoneNumber;
				bReadBlock    = MMP_TRUE;
			}
			else
			{
				printf("xd_writemultiplesector: GenerateTranslationTable Failed. Abort!\n");
				result = XD_ERROR;
				goto end;
			}
		}
		else
		{
			phyBlock = g_TranslationTable[pureLogicBlock];
		}


	    if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
		{
		    if(phyBlock>=0x400)
		    {
		        printf("2.phyBlock >= 0x400!!\r\n");
		    }
		    phyBlock=phyBlock+(zoneNumber*0x400);
		}
		else
		{
	        printf("2.PhyBlk=0xFFFF.\r\n");
        }

		//////////////////////////////////////////////////////////////////////////
		// Step 3, Get an empty block
        if ( XD_HeapPopMinElement(g_pXdEraseBlockHeap, &newBlock) == MMP_FALSE )
		{
			printf("xd_writemultiplesector: Can't Get Empty Block For Write. Abort!\n");
			result = XD_ERROR;
			goto end;
		}

		newBlock=newBlock+(zoneNumber*0x400);

		if ( bReadBlock == MMP_TRUE )
		{
			XD_ReadSeparate(newBlock, 0, MMP_NULL, MMP_NULL);
		}

		if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
		{
			//////////////////////////////////////////////////////////////////////////
			// Step 4, Read old data back
			// Read front block
			for ( i=0; i<phyPage; i++ )
			{
				result = XD_ReadSingle(phyBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE));
				if ( result == XD_ERROR )
				{
					printf("xd_writemultiplesector: XD_Read() Read Front Data Back Failed. Abort!\n");
					goto end;
				}
			}
			// Read rear block
			for ( i=(phyPage+writePageCount); i<XD_PAGEPERBLOCK; i++ )
			{
				result = XD_ReadSingle(phyBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE));
				if ( result == XD_ERROR )
				{
					printf("xd_writemultiplesector: XD_Read() Read Front Data Back Failed. Abort!\n");
					goto end;
				}
			}
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// Step 4, Prepare null data
			UpdateSpare(nullBuffer, nullBuffer + XD_PAGE_SIZE, pureLogicBlock);
		}

		//////////////////////////////////////////////////////////////////////////
		// Step 5, Prepare new data
		for ( i=phyPage; i<(phyPage+writePageCount); i++ )
		{
			UpdateSpare(pDataForUpdate, pReadBackBuffer + (i * XD_DATA_SIZE) + XD_PAGE_SIZE, pureLogicBlock);
			pDataForUpdate += 512;
		}

		//////////////////////////////////////////////////////////////////////////
		// Step 6, Write data to new block
		// Step 6.1, Write front block
		if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
		{
			for ( i=0; i<phyPage; i++ )
			{
				result = XD_WriteSingle(newBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE),pureLogicBlock);
				if ( result == XD_ERROR )
				{
					printf("xd_writemultiplesector: XD_Write() Write Front Data Failed. Abort!\n");
					goto end;
				}
			}
		}
		else
		{
			for ( i=0; i<phyPage; i++ )
			{
				result = XD_WriteSingle(newBlock, i, nullBuffer,pureLogicBlock);
				if ( result == XD_ERROR )
				{
					printf("xd_writemultiplesector: XD_Write() Write Front Data Failed. Abort!\n");
					goto end;
				}
			}
		}
		
		// Step 6.2, Write new data
		for ( i=phyPage; i<(phyPage+writePageCount); i++ )
		{
			result = XD_WriteSeparate(newBlock, i, pDataForWrite, pReadBackBuffer + (i * XD_DATA_SIZE) + XD_PAGE_SIZE,pureLogicBlock);
			if ( result == XD_ERROR )
			{
				printf("xd_writemultiplesector: XD_Write() Write Front Data Failed. Abort!\n");
				goto end;
			}
			pDataForWrite += 512;
		}
		
		// Step 6.3, Write rear block
		if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
		{
			for ( i=(phyPage+writePageCount); i<XD_PAGEPERBLOCK; i++ )
			{
				result = XD_WriteSingle(newBlock, i, pReadBackBuffer + (i * XD_DATA_SIZE),pureLogicBlock);
				if ( result == XD_ERROR )
				{
					printf("xd_writemultiplesector: XD_Write() Write Front Data Failed. Abort!\n");
					goto end;
				}
			}
		}
		else
		{
			for ( i=(phyPage+writePageCount); i<XD_PAGEPERBLOCK; i++ )
			{
				result = XD_WriteSingle(newBlock, i, nullBuffer,pureLogicBlock);
				if ( result == XD_ERROR )
				{
					printf("xd_writemultiplesector: XD_Write() Write Front Data Failed. Abort!\n");
					goto end;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Step 7, Update Translation Table
		if ( phyBlock != XD_INVALID_BLOCK_NUMBER )
		{
			if ( XD_Erase(phyBlock) == XD_OK )
			{
                XD_HeapInsertElement(g_pXdEraseBlockHeap, phyBlock&0x3FF);
			}
		}

        g_TranslationTable[pureLogicBlock] = (newBlock&0x3FF);

		cnt    -= writePageCount;
		sector += writePageCount;
	}

end:
    if ( pReadBackBufferAllocAddr )
    {
        free(pReadBackBufferAllocAddr);
        pReadBackBufferAllocAddr = MMP_NULL;
        pReadBackBuffer = MMP_NULL;
    }
    
	return result;
}

int xddrv_getphy(F_DRIVER* driver, F_PHY *phy)
{
	phy->number_of_sectors = g_NumOfBlocks / XD_PHYSIC_BLOCK_PER_ZONE * XD_LOGIC_BLOCK_PER_ZONE* XD_PAGEPERBLOCK;
	return 0; // Indicate succeeded!
}

int xddrv_getstatus(F_DRIVER* driver, XD_CARD_STATE state)
{
    MMP_UINT32 data = 0;
    MMP_BOOL result = MMP_FALSE;

    switch(state)
    {
        case XD_INIT_OK:           
            result = MMP_TRUE;
            break;
        case XD_INSERTED:
	    	//result = HOST_IsCardInserted(STORAGE_XD_IP);
	    	result=ithCardInserted(ITH_CARDPIN_XD);	
	        break;
	    default:
	        break;
	}    
	    
	return  result;
}

void xddrv_release(F_DRIVER* driver)
{
	XD_Terminate();
	XD_EccTerminate();
	XD_ML_Terminate();
}

#ifdef GENTABLE_READ_BUFFER
MMP_BOOL
GenerateTranslationTable(
	MMP_UINT32  zone, 
	MMP_UINT32  logicalBlock, 
	MMP_UINT32* pPhyBlock)
{
	MMP_UINT32 startBlock       = 0;
	MMP_UINT32 endBlock         = 0;
	MMP_UINT16 logicalAddr      = 0;
	MMP_BOOL   bFindTargetBlock = MMP_FALSE;
	MMP_UINT8  result           = XD_ERROR;

	MMP_UINT8  dataBuffer[XD_DATA_SIZE];
	MMP_UINT32 i;

	*pPhyBlock = XD_INVALID_BLOCK_NUMBER;

	if ( zone == 0 )
	{
		startBlock = g_CisBlock + 1;
		endBlock   = XD_PHYSIC_BLOCK_PER_ZONE - 1;
	}
	else
	{
		startBlock = zone * XD_PHYSIC_BLOCK_PER_ZONE;
		endBlock   = zone * XD_PHYSIC_BLOCK_PER_ZONE + XD_PHYSIC_BLOCK_PER_ZONE - 1;
	}

	// Create empty block heap
	if ( g_pXdEraseBlockHeap != MMP_NULL )
	{
		XD_DeleteHeap(g_pXdEraseBlockHeap);
		g_pXdEraseBlockHeap = MMP_NULL;
	}
	g_pXdEraseBlockHeap = XD_CreateHeap(XD_PHYSIC_BLOCK_PER_ZONE);
	if ( g_pXdEraseBlockHeap == MMP_NULL )
	{
		printf("GenerateTranslationTable: Create Empty Block Heap Failed. Abort!\n");
		return MMP_FALSE;
	}

	// Clean Translation Table
	memset(g_TranslationTable, 0xFF, XD_PHYSIC_BLOCK_PER_ZONE * sizeof(MMP_UINT32));

	for ( i=startBlock; i<=endBlock; i++ )
	{
		//result = XD_ReadSpare(i, 0, spareBuffer);
		result = XD_ReadSingle(i, 0, dataBuffer);
		if ( result == XD_OK )
		{
			if (   IsNormalBlock(dataBuffer[XD_BLOCK_STATUS_FLAG]) 
				&& GetLogicalAddress(dataBuffer, &logicalAddr))
			{
				if ( g_TranslationTable[logicalAddr] == XD_INVALID_BLOCK_NUMBER)
				{
					g_TranslationTable[logicalAddr] = (i&0x3FF);
					if (   logicalAddr      == logicalBlock 
						&& bFindTargetBlock == MMP_FALSE)
					{
						bFindTargetBlock = MMP_TRUE;
						*pPhyBlock = (i&0x3FF);
					}
				}
				else
				{	// Table already has value
					MMP_UINT16 logicalAddrLast = 0;
					//if (   XD_ReadSpare(i, XD_PAGEPERBLOCK-1, spareBuffer) == XD_OK
					if (   XD_ReadSingle(i, XD_PAGEPERBLOCK-1, dataBuffer) == XD_OK
						&& GetLogicalAddress(dataBuffer, &logicalAddrLast) )
					{
						if ( logicalAddrLast == logicalAddr )
						{
							//if ( XD_ReadSpare(g_TranslationTable[logicalAddr], 0, spareBuffer) == XD_OK )
							if ( XD_ReadSingle(g_TranslationTable[logicalAddr], 0, dataBuffer) == XD_OK )
							{
								MMP_UINT16 logicalAddr1 = 0;
								MMP_UINT16 logicalAddr2 = 0;
								//if (   GetLogicalAddress(spareBuffer, &logicalAddr1)
								//    && XD_ReadSpare(g_TranslationTable[logicalAddr], XD_PAGEPERBLOCK-1, spareBuffer) == XD_OK )
								if (   GetLogicalAddress(dataBuffer, &logicalAddr1)
									&& XD_ReadSingle(g_TranslationTable[logicalAddr], XD_PAGEPERBLOCK-1, dataBuffer) == XD_OK )
								{
									if ( GetLogicalAddress(dataBuffer, &logicalAddr2) )
									{
										if ( logicalAddr1 == logicalAddr2 )
										{
											if ( XD_Erase(i) == XD_OK )
											{
												if ( XD_HeapInsertElement(g_pXdEraseBlockHeap, i&0x3FF) != MMP_TRUE )
												{
													printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
												}
											}
										}
										else
										{
											if ( XD_Erase(g_TranslationTable[logicalAddr]) == XD_OK )
											{
												if ( XD_HeapInsertElement(g_pXdEraseBlockHeap, g_TranslationTable[logicalAddr]&0x3FF) != MMP_TRUE )
												{
													printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
												}
											}
											g_TranslationTable[logicalAddr] = (i&0x3FF);
										}
									}
								}
							}
						}
						else
						{	// Erase or skip current physical block
							if ( XD_Erase(i) == XD_OK )
							{
								if ( XD_HeapInsertElement(g_pXdEraseBlockHeap, i&0x3FF) != MMP_TRUE )
								{
									printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
								}
							}
						}
					}
				}
			}
			else
			{
				//printf("Bad Blcok!\n");
			}
		}
		else if ( result == XD_ERASED )
		{
			if ( !XD_HeapInsertElement(g_pXdEraseBlockHeap, i&0x3FF) )
			{
				printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
			}
		}
	}

	return MMP_TRUE;
}
#else
MMP_BOOL
GenerateTranslationTable(
	MMP_UINT32  zone, 
	MMP_UINT32  logicalBlock, 
	MMP_UINT32* pPhyBlock)
{
	MMP_UINT32 startBlock       = 0;
	MMP_UINT32 endBlock         = 0;
	MMP_UINT16 logicalAddr      = 0;
	MMP_BOOL   bFindTargetBlock = MMP_FALSE;
	MMP_UINT8  result           = XD_ERROR;

	MMP_UINT8  spareBuffer[XD_SPARE_SIZE];
	MMP_UINT32 i;
	//PAL_CLOCK_T startTick, endTick;
	
	//startTick = PalGetClock();

	*pPhyBlock = XD_INVALID_BLOCK_NUMBER;

	if ( zone == 0 )
	{
		startBlock = g_CisBlock + 1;
		endBlock   = XD_PHYSIC_BLOCK_PER_ZONE - 1;
	}
	else
	{
		startBlock = zone * XD_PHYSIC_BLOCK_PER_ZONE;
		endBlock   = zone * XD_PHYSIC_BLOCK_PER_ZONE + XD_PHYSIC_BLOCK_PER_ZONE - 1;
	}

	// Create empty block heap
	if ( g_pXdEraseBlockHeap != MMP_NULL )
	{
		XD_DeleteHeap(g_pXdEraseBlockHeap);
		g_pXdEraseBlockHeap = MMP_NULL;
	}
	g_pXdEraseBlockHeap = XD_CreateHeap(XD_PHYSIC_BLOCK_PER_ZONE);
	if ( g_pXdEraseBlockHeap == MMP_NULL )
	{
		printf("GenerateTranslationTable: Create Empty Block Heap Failed. Abort!\n");
		return MMP_FALSE;
	}

	// Clean Translation Table
	memset(g_TranslationTable, 0xFF, XD_PHYSIC_BLOCK_PER_ZONE * sizeof(MMP_UINT32));

	for ( i=startBlock; i<=endBlock; i++ )
	{
		/*
		if( (i&0xFF)==0xFF )
        {
            endTick = PalGetClock();
            if ( ((double)(endTick - startTick))/80000000 >= 0.01 )
            {
                PalSleep(0);
                startTick = PalGetClock();
            }
        }
        */
        
		result = XD_ReadSpare(i, 0, spareBuffer);
		if ( result == XD_OK )
		{
			if (   IsNormalBlock(spareBuffer[XD_BLOCK_STATUS_FLAG_OFFSET]) 
				&& GetLogicalAddress(spareBuffer, &logicalAddr))
			{
				if ( g_TranslationTable[logicalAddr] == XD_INVALID_BLOCK_NUMBER)
				{
					g_TranslationTable[logicalAddr] = (i&0x3FF);
					if (   logicalAddr      == logicalBlock 
						&& bFindTargetBlock == MMP_FALSE)
					{
						bFindTargetBlock = MMP_TRUE;
						*pPhyBlock = (i&0x3FF);
					}
				}
				else
				{	// Table already has value
					MMP_UINT16 logicalAddrLast = 0;
					if (   XD_ReadSpare(i, XD_PAGEPERBLOCK-1, spareBuffer) == XD_OK
						&& GetLogicalAddress(spareBuffer, &logicalAddrLast) )
					{
						if ( logicalAddrLast == logicalAddr )
						{
							if ( XD_ReadSpare(g_TranslationTable[logicalAddr], 0, spareBuffer) == XD_OK )
							{
								MMP_UINT16 logicalAddr1 = 0;
								MMP_UINT16 logicalAddr2 = 0;
								if (   GetLogicalAddress(spareBuffer, &logicalAddr1)
									&& XD_ReadSpare(g_TranslationTable[logicalAddr], XD_PAGEPERBLOCK-1, spareBuffer) == XD_OK )
								{
									if ( GetLogicalAddress(spareBuffer, &logicalAddr2) )
									{
										if ( logicalAddr1 == logicalAddr2 )
										{
											if ( XD_Erase(i) == XD_OK )
											{
												if ( XD_HeapInsertElement(g_pXdEraseBlockHeap, i&0x3FF) != MMP_TRUE )
												{
													printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
												}
											}
										}
										else
										{
											if ( XD_Erase(g_TranslationTable[logicalAddr]) == XD_OK )
											{
												if ( XD_HeapInsertElement(g_pXdEraseBlockHeap, g_TranslationTable[logicalAddr]&0x3FF) != MMP_TRUE )
												{
													printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
												}
											}
											g_TranslationTable[logicalAddr] = (i&0x3FF);
										}
									}
								}
							}
						}
						else
						{	// Erase or skip current physical block
							if ( XD_Erase(i) == XD_OK )
							{
								if ( XD_HeapInsertElement(g_pXdEraseBlockHeap, i&0x3FF) != MMP_TRUE )
								{
									printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
								}
							}
						}
					}
				}
			}
			else
			{
				//printf("Bad Blcok!\n");
			}
		}
		else if ( result == XD_ERASED )
		{
		    	if ( !XD_HeapInsertElement(g_pXdEraseBlockHeap, i&0x3FF) )
			{
				printf("GenerateTranslationTable: Store erased block %d failed.\n", i);
			}
		}
		else
		{
			//HOST_StorageIoSelect(MMP_CARD_XD_IP);
			
			//result=HOST_IsCardInserted(MMP_CARD_XD_IP);
			result=ithCardInserted(ITH_CARDPIN_XD);	

			if(result==0)
			{
				printf("XD was removed!!\r\n");
				g_LastUseZone = XD_INVALID_BLOCK_NUMBER;
				return	result;
			}
			
		}
	}

	return MMP_TRUE;
}
#endif

MMP_INT 
mmpxDInitialize(void)
{
    MMP_UINT8 mmpRes  = MMP_TRUE;

	// Initial XD card

    ithLockMutex(ithStorMutex);
	ithStorageSelect(ITH_STOR_XD);

	if ( XD_Initialize(&g_NumOfBlocks) != XD_OK )
	{
	    printf("XD_ML_Initialize: XD_Initialize() failed\n");
		mmpRes = MMP_FALSE;
		goto end;
	}
	
	printf("ml.NOB=%x\n",g_NumOfBlocks);

	// Initial Ecc Table
	if ( XD_EccInitial() == MMP_FALSE )
	{
		printf("XD_ML_Initialize: Initial Ecc Table Failed.\n");
		mmpRes = MMP_FALSE;
		goto end;
	}

	// Initial parity table
	InitEvenParityTable();

	if ( IsValidXdCard() != MMP_TRUE)
	{
	    printf("XD_ML_Initialize: Not a valid card.\n");
		mmpRes = MMP_FALSE;
		goto end;
	}

#if 0
    {
        MMP_UINT32 i;
        for ( i=(g_CisBlock + 1); i<g_NumOfBlocks; i++ )
        {
            if ( XD_Erase(i) != XD_OK )
                printf("Erase block %u fail\n", i);
        }
        printf("End of erase XD card.\n");
        while(1)
        {
            printf("");
        }
    }
#endif

	// Initial g_TranslationTable
	g_LastUseZone = XD_INVALID_BLOCK_NUMBER;
	memset(g_TranslationTable, 0xFF, XD_PHYSIC_BLOCK_PER_ZONE * sizeof(MMP_UINT32));

end:
	ithUnlockMutex(ithStorMutex);
	return mmpRes;
}

MMP_INT 
mmpxDTerminate(void)
{
	ithLockMutex(ithStorMutex);
    //XD_Terminate();
    //XD_EccTerminate();
    XD_ML_Terminate();
    ithUnlockMutex(ithStorMutex);
    return MMP_TRUE;
}

MMP_INT 
mmpxDRead(
    MMP_UINT32 sector, 
    MMP_INT count, 
    void* data)
{
  MMP_UINT8  result = MMP_FALSE;
  
  	//SYS_WaitSemaphore(XD_Semaphore);
  	ithLockMutex(ithStorMutex);
  	if(xddrv_readmultiplesector( (F_DRIVER*)MMP_TRUE, data, sector, count)==XD_OK)
  	{
  		result = MMP_TRUE;
  	}
    // SYS_ReleaseSemaphore(XD_Semaphore);
    ithUnlockMutex(ithStorMutex);

	return result; 
}    

MMP_INT 
mmpxDWrite(
    MMP_UINT32 sector, 
    MMP_INT count, 
    void* data)
{
  MMP_UINT8  result = MMP_FALSE;
  
  	//SYS_WaitSemaphore(XD_Semaphore);
  	ithLockMutex(ithStorMutex);
  	if(xddrv_writemultiplesector( (F_DRIVER*)MMP_TRUE, data, sector, count)==XD_OK)
  	{
  		result = MMP_TRUE;
  	}

  	//SYS_ReleaseSemaphore(XD_Semaphore);
  	ithUnlockMutex(ithStorMutex);

  	return result;
}    

MMP_INT 
mmpxDGetCapacity(
    MMP_UINT32* sectorNum, 
    MMP_UINT32* blockLength)
{
	if(g_NumOfBlocks)
	{
	  	*sectorNum = g_NumOfBlocks / XD_PHYSIC_BLOCK_PER_ZONE * XD_LOGIC_BLOCK_PER_ZONE* XD_PAGEPERBLOCK;
	  	*blockLength = 512;
	  	return MMP_TRUE;
	}
	else
	{
	  	*sectorNum = 0;
	  	*blockLength = 512;
	  	return MMP_FALSE;
	}
}    
    
#define REG_BIT_XD_CARD_DETECT         (1u << 17)

MMP_BOOL 
mmpxDGetCardState(
    XD_CARD_STATE state)
{
    MMP_UINT32 data = 0;
    MMP_BOOL result = MMP_FALSE;

    switch(state)
    {
        case XD_INIT_OK:           
            result = MMP_TRUE;
           break;
        case XD_INSERTED:
	    //result = HOST_IsCardInserted(STORAGE_XD_IP);
	        break;
	    default:
	        break;
	}    
	    
	return  result;
}


