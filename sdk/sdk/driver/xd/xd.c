
//#include <stdarg.h>
//#include <stdio.h>
#include "xD/xd.h"
#include "xD/xd_ecc.h"

#include "ite/ith.h"
#include "ite/itp.h"
//#include "mmp_dma.h"


////////////////////////////////////////////////////////////
//
//	Compile Option
//
////////////////////////////////////////////////////////////
#define XD_READ_ECC_CHECK

//#define ENABLE_XD_MSG
//#define ENABLE_XD_WRAP_DMA_MODE



////////////////////////////////////////////////////////////
//
//	MACRO DEFINE
//
////////////////////////////////////////////////////////////
#define SET_NDCS0_MASK			0x03000000
#define SET_XD_BUSY_MASK		0x00003000

#define SET_NDCS0_PULL_UP		0x02000000
#define SET_XD_BUSY_PULL_UP		0x00002000

////////////////////////////////////////////////////////////
//
//	Global Variable
//
////////////////////////////////////////////////////////////
static unsigned long   XDNumOfBlocks        = 0;
static unsigned char   gEnTiming_XD         = 0;
static unsigned char   gSMCheckECC          = 0;
static unsigned char   gSM_Addr4						= 0;

static unsigned char*  g_pXDdataBufAllocAdr = MMP_NULL;
static unsigned char*  g_pXDdataBuf         = MMP_NULL;

static unsigned int   g_tmpPba				= 0;
////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
#ifdef XD_DEBUG_MSG
void XD_DbgPrintf(char* str, ...)
{
	va_list ap;
    char buf[384];
    MMP_INT result;

	va_start(ap, str);
    result = vsprintf(buf, str, ap);
	va_end(ap);

    if (result >= 0)
    {
        printf(buf);
    }
}
#else
	#if defined(WIN32)
		void XD_DbgPrintf(char* str, ...) {}
	#elif defined(__OPENRTOS__)
		#define XD_DbgPrintf(x, ...)
	#endif
#endif

/*
#if defined(TEMP_FREERTOS)
static void XD_DmaCopy(void* dst, void* src, MMP_UINT32 size);
 #ifdef	ENABLE_XD_WRAP_DMA_MODE
 static void XD_DmaCopyWrapToMem(void* dst);
 static void XD_DmaCopyMemToWrap(void* src);
 #endif
#endif
*/
//static void EnableXDControl();

////////////////////////////////////////////////////////////
//
// Function Implement
//
////////////////////////////////////////////////////////////
void EnableXDControl()
{
	MMP_UINT32	dwSET_PIN=(SET_NDCS0_PULL_UP | SET_XD_BUSY_PULL_UP);
	MMP_UINT32	dwSET_PIN_MASK=(SET_NDCS0_MASK | SET_XD_BUSY_MASK);

	//ithWriteRegMaskA(GPIO_BASE + 0xD0, 0x00000001 , 0x00000007); //set NAND gpio as pull up
	//ithWriteRegMaskA(GPIO_BASE + 0xC0, 0x0030033F , 0x0030033F); //set NAND gpio as pull up
	
	//ithWriteRegMaskA( (GPIO_BASE+0x4C), dwSET_PIN, dwSET_PIN_MASK );	//pull up XD BUSY/READY and pull up NDCS0 pin	
}

MMP_UINT8
XD_WaitCmdReady(
    XD_CMD_TYPE type)
{
	return 0;
}


MMP_UINT8
XD_Initialize(MMP_UINT32* pNumOfBlocks)
{
    MMP_UINT8 result = XD_OK;
    MMP_UINT8* VramBaseAddress;

    EnableXDControl();
    XD_nWP_Disable();//to fit the power-on sequence of XD card 

    usleep(100000);		//sleep 100ms to prevent from the card power reset issue
	
	XD_SwReset();

    //set XD clock
    //ithWriteRegA(SMI_CLOCK, 0x7A);
    //ithWriteRegA(SMI_RCLOCK, 0x92);
    //ithWriteRegA(SMI_WCLOCK, 0x0A);

    //ithWriteRegA(SMI_CLOCK, 0x7F);
    //ithWriteRegA(SMI_RCLOCK, 0xBF);
    //ithWriteRegA(SMI_WCLOCK, 0x09);
    
    //for 320Mhz script XD timing
    ithWriteRegA(SMI_CLOCK, 0x7A);
    ithWriteRegA(SMI_RCLOCK, 0x82);
    ithWriteRegA(SMI_WCLOCK, 0x09);		//0x01 also work @320Mhz script

	gSMCheckECC=1;////set it as 1 for a while
	if( gSMCheckECC )
    {
        XD_WriteRegisterMask(SMI_BITCNT, bit4, bit4);
	}
    else
    {
        XD_WriteRegisterMask(SMI_BITCNT, ~bit4, bit4);
	}

	XD_nWP_Enable();
	XD_nCE_Enable();

	ithWriteRegA(SMI_CMD1VLU, XD_RESET_CMD_2ST);

	ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_WaitRnB) );

	//no INT, no card detect
	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);

	if(result)
	{
		return 	result;
	}

	XD_nCE_Disable();

	if ( GetXDId(pNumOfBlocks) == XD_OK )
	{
		XDNumOfBlocks = *pNumOfBlocks;
        //VramBaseAddress = HOST_GetVramBaseAddress();
        VramBaseAddress = 0;

        if ( g_pXDdataBufAllocAdr == MMP_NULL )
	    {
	    	//g_pXDdataBufAllocAdr = (MMP_UINT8*)MEM_Allocate(XD_DATA_SIZE+4, MEM_USER_XD, MEM_USAGE_GENERAL);
			//g_pXDdataBufAllocAdr = (MMP_UINT8*)MEM_Allocate(XD_DATA_SIZE+4, MEM_USER_XD);
			#ifdef	WIN32
			g_pXDdataBufAllocAdr = (MMP_UINT8*)itpVmemAlloc(XD_DATA_SIZE+4);
			#else
			g_pXDdataBufAllocAdr = (MMP_UINT8*)malloc(XD_DATA_SIZE+4);
			#endif
        
	    	if(g_pXDdataBufAllocAdr == MMP_NULL)
	    	{
	    		result = XD_ERROR;  //error
		    	return result;	//goto end;
		    }
		    g_pXDdataBuf = (MMP_UINT8*)(((MMP_UINT32)g_pXDdataBufAllocAdr + 3) & ~3);
    	}
	}
	else
	{
    	printf("XD_Initialize: Get XD card ID fail.\n");
 		result=XD_ERROR;
    	return result;	//goto end;
	}

	if( XDNumOfBlocks > 2048 ) // XD card more than 32 MByte
	{
			gSM_Addr4 = 1;
	}

#ifdef ENABLE_XD_DMA
	// Initial DMA
	if ( g_XdDmaContent == MMP_NULL )
    {
        mmpDmaInitialize();
        if ( mmpDmaCreateContext(&g_XdDmaContent) )
        {
            printf("XD_Initialize: Initial DMA fail.\n");
            result = XD_ERROR;
			//goto end;
        }
    }
    
	// Initial WRAP DMA
	if ( g_XdWrapDmaContent == MMP_NULL )
    {
        mmpDmaInitialize();
        if ( mmpDmaCreateContext(&g_XdWrapDmaContent) )
        {
            printf("XD_Initialize: Initial DMA fail.\n");
            result = XD_ERROR;
			//goto end;
        }
    }
#endif
	return result;
}

MMP_UINT8
XD_Terminate()
{
    XD_DbgPrintf("XD_Terminate: \n");

    // Switch controler to XD
    EnableXDControl();

	return XD_OK;
}

MMP_UINT8
XD_Erase(MMP_UINT32 pba)
{
    unsigned char result;
    unsigned int  realAddress;

    XD_DbgPrintf("XD_Ers:%x \n",pba);
    #ifdef	ENABLE_XD_MSG
	printf("XD_Ers:%x \n",pba);
	#endif

    // Switch controler to XD
    EnableXDControl();

	realAddress = pba * XD_PAGEPERBLOCK;
	ithWriteRegA(SMI_CMD1VLU, XD_ERASE_CMD_1ST);
	ithWriteRegA(SMI_CMD2VLU, XD_ERASE_CMD_2ND);

	XD_nCE_Enable();
	XD_nWP_Disable();

	ithWriteRegA(SMI_ADR3, (MMP_UINT8)((realAddress>>16)&0xFF) );
	ithWriteRegA(SMI_ADR2, (MMP_UINT8)((realAddress>>8)&0xFF) );
	ithWriteRegA(SMI_ADR1, (MMP_UINT8)(realAddress&0xFF));

	if( gSM_Addr4 )
	{
		ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr3|RTrg_Cmd2|RTrg_WaitRnB) );
	}
	else
	{
		ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr2|RTrg_Cmd2|RTrg_WaitRnB) );
	}

	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);
	if(result)
	{
		return 	result;
	}

	XD_nWP_Enable();
	XD_nCE_Disable();

	return result;
}

MMP_UINT8
XD_WriteSeparate(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pData,
	MMP_UINT8* pSpare,
    MMP_UINT32 LogicBlkAddr)
{
    MMP_UINT8   result          = XD_ERROR;
    MMP_UINT8  bLBA_L, bLBA_H;
    MMP_UINT32* pdwPtDataBuff;

    MMP_UINT32  realAddress     = 0x00;
    MMP_UINT8   trg;
    MMP_UINT32   i;
#ifdef __OPENRTOS__
    MMP_UINT8* pAlignDataBuf   = (MMP_UINT8*)(((MMP_UINT32)pData + 3) & ~3);
#endif
    XD_DbgPrintf("XD_WriteSeparate: \n");
    
#ifdef	ENABLE_XD_MSG
	printf("Wsp[%x,%x]\n",pba, ppo);
#endif	
/*
	if(LogicBlkAddr==0x337 && ppo==0x18)
	{
		g_tmpPba = pba;
		printf("pData&pSpr=[%x,%x]\n",pData, pSpare);
		for(i=0; i<512; i++)
		{
			printf("%02X ", pData[i]);
			if( (i&0x0F)==0x0F )	printf("\n");
		}
		printf("\n");
		for(i=0; i<16; i++)
		{
			printf("%02X ", pSpare[i]);
			if( (i&0x0F)==0x0F )	printf("\n");
		}
		printf("\n");
	}
*/
    return XD_WriteSingle(pba, ppo, pData, LogicBlkAddr);
}



MMP_UINT8
XD_WriteSingle(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pData,
    MMP_UINT32 LogicBlkAddr)
{
    MMP_UINT8  result          = XD_ERROR;
    MMP_UINT8  bLBA_L, bLBA_H;
    MMP_UINT32* pdwPtDataBuff;
    MMP_UINT32 realAddress     = 0x00;
    MMP_UINT32 TimeOutCount=0;
    MMP_UINT32 TimeOutCnt=0x0FFF;
    MMP_UINT8   trg;
    MMP_UINT32   i,j=0;
	MMP_UINT32   Reg32;

#ifdef __OPENRTOS__
    MMP_UINT8* pAlignDataBuf   = (MMP_UINT8*)(((MMP_UINT32)pData + 3) & ~3);
    MMP_UINT8  XdDmaDataBuffer[528];
#else
	MMP_UINT8  XdDmaDataBuffer[528];
#endif

#ifdef	ENABLE_XD_MSG
    //XD_DbgPrintf("XD_WriteSingle: \n");
	printf("XD_Ws:[%x,%x,%x] \n",pba ,ppo ,LogicBlkAddr );
	//printf("w\r");
#endif
/*
	if(LogicBlkAddr==0x337 && ppo==0x18)
	{
		printf("pData=[%x,%x]\n",pData);
		for(i=0; i<512; i++)
		{
			printf("%02X ", pData[i]);
			if( (i&0x0F)==0x0F )	printf("\n");
		}
		printf("\n");
	}
*/
    // Switch controler to XD
    EnableXDControl();
    XD_nCE_Enable();

	ithWriteRegA(SMI_CMD1VLU, XD_READ_CMD_1ST);
	ithWriteRegA(SMI_RTRG, RTrg_Cmd1);
	bLBA_L=(MMP_UINT8)(LogicBlkAddr&0xFF);
	bLBA_H=(MMP_UINT8)((LogicBlkAddr)>>8);

	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);
	
	if(result)
    {
        printf("WS.wait read 0x00 trigger error..\r\n");
		return	result;
    }
	
    while(0)
    {
    	MMP_UINT32 Rg32;
    	Rg32 = ithReadRegA(SMI_ADR5);
    	if( (Rg32&0xFF)==0xAA)	break;
    }
	
    XD_nWP_Disable();

	ithReadRegA(SMI_CTRL);

	ithWriteRegA(SMI_WRAP_DATA_LENGTH	,	0x200);//read 512 bytes
	
	
	#ifdef	ENABLE_XD_WRAP_DMA_MODE
	ithWriteRegA(SMI_WRAP_CONTROL	,	0x11);//set WRITE direction, and wrap fire 
	#else
	#if defined(ENABLE_XD_DMA)
	ithWriteRegA(SMI_WRAP_CONTROL	,	0x01);//set WRITE direction, and wrap fire
	#else
	ithWriteRegA(SMI_WRAP_CONTROL	,	0x01);//set WRITE direction, and wrap fire
	#endif
	#endif

#ifdef __OPENRTOS__
    if ( pAlignDataBuf != pData )
    {
        ithInvalidateDCacheRange((void*)pData, XD_PAGE_SIZE);
        //XD_DmaCopy(g_pXDdataBuf, pData, XD_PAGE_SIZE);
        memcpy(g_pXDdataBuf, (MMP_UINT8*)pData, XD_PAGE_SIZE);
        pdwPtDataBuff=(MMP_UINT32*)g_pXDdataBuf;
    }
    else
    {
        pdwPtDataBuff=(MMP_UINT32*)pData;//data output
    }
    ithInvalidateDCacheRange((void*)pdwPtDataBuff, XD_DATA_SIZE);
#elif defined(WIN32)
    pdwPtDataBuff=(MMP_UINT32*)pData;//data output
#endif

	#ifndef	ENABLE_XD_WRAP_DMA_MODE
	for(i=0;i<128;i++)
	{
        ithWriteRegA(SMI_WRAP_DATA_PORT , pdwPtDataBuff[i]);
	}
	#endif
	Reg32 = ithReadRegA(SMI_WRAP_CONTROL);
	Reg32 = ithReadRegA(SMI_WRAP_STATUS);

	ithWriteRegA(SMI_LBAH, (MMP_UINT32)bLBA_H);
	ithWriteRegA(SMI_LBAL, (MMP_UINT32)bLBA_L);

	XD_WriteRegisterMask(SMI_CTRL, SMI_CTRL_Ecc_En, SMI_CTRL_Ecc_En);

	realAddress = ((pba * XD_PAGEPERBLOCK) + ppo);

	ithWriteRegA(SMI_RDNTLEN, 0x10);
	ithWriteRegA(SMI_ADR4, ((realAddress>>16)&0xFF) );
	ithWriteRegA(SMI_ADR3, ((realAddress>>8)&0xFF) );
	ithWriteRegA(SMI_ADR2, (realAddress&0xFF) );
	ithWriteRegA(SMI_ADR1, 0x00);

	if( gSM_Addr4 )
	{
		trg = (WTrg_Cmd1|WTrg_Adr4|WTrg_Cmd2|WTrg_Dat|WTrg_Rdnt|WTrg_WaitRnB);
	}
	else
	{
		trg = (WTrg_Cmd1|WTrg_Adr3|WTrg_Cmd2|WTrg_Dat|WTrg_Rdnt|WTrg_WaitRnB);
	}

	ithWriteRegA(SMI_CMD1VLU,XD_PROGRAM_CMD_1ST);
	ithWriteRegA(SMI_CMD2VLU,XD_PROGRAM_CMD_2ND);

	#ifdef	ENABLE_XD_WRAP_DMA_MODE	
	//usb2spi_WriteMemory((MMP_UINT32)XdDmaDataBuffer, (MMP_UINT32)pdwPtDataBuff, 512);
	HOST_WriteBlockMemory((MMP_UINT32)XdDmaDataBuffer, (MMP_UINT32)pdwPtDataBuff, 512);
	//XD_DmaCopyMemToWrap(pdwPtDataBuff);	
	XD_DmaCopyMemToWrap(XdDmaDataBuffer);	
	#else
	ithWriteRegA(SMI_WTRG,trg);
	Reg32 = ithReadRegA(SMI_WTRG);

	//wait wrap empty
	result = XD_WaitXDWrapFullEmpty(WAIT_WRAP_EMPTY);

	if(result)
	{
	    printf("WS.wait Write wrap empty error..\r\n");
		return result;
	}
	#endif

	//wait SMI_WTRG DONE
	result = XD_WaitReadWriteTrigger(WAIT_WRITE_TRIGGER);
	if(result)
    {
        printf("WS.wait Write trigger error..\r\n");
		return	result;
	}

    result = XD_WaitCmdReady(XD_WRITE);

    if(result)
    {
        printf("WS. read status error, %x..\r\n",result);
    }

    XD_nWP_Enable();
    XD_nCE_Disable();   
    
    if( LogicBlkAddr==0x1E8 && (ppo==0x1F) )
    {
    	printf("lba=1E8, ppo=1F\n");  
    }
    
    return result;

}

MMP_UINT8
XD_ReadSeparate(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer,
	MMP_UINT8* pSparebuffer)
{
    MMP_UINT8  result          = XD_ERROR;
	MMP_UINT8  data[XD_PAGE_SIZE];

    XD_nCE_Disable();
	return result;
}

MMP_UINT8
XD_ReadSingle(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer)
{
    MMP_UINT8  result          = XD_ERROR;
    MMP_UINT32 i               = 0;

#ifdef	ENABLE_XD_MSG
	printf("XdRdSin[%04x,%02x]\n",pba,ppo);
#endif

    EnableXDControl();

    XD_nCE_Enable();
    
    result=XD_ReadPage( pba, ppo, pDataBuffer);

    //copy 16 bytes of spare data to *pDataBuffer;
    if(result!=XD_ERROR)
	{
        MMP_UINT32  RegValue32,j=512;
        for( i=SMI_RDNT0; i<=SMI_RDNTF; i+=4 )
		{
            XD_ReadRegister(i,&RegValue32);
            /*
            if(i==0xE158)
			{
                if( (RegValue32&0xFF)==0x00 )
                {
                    printf("Error!!!!, LBAH==0x00,%x\r\n",RegValue32);
				}
			}
            if(i==0xE15C)
			{
                if( (RegValue32&0xFF)==0x00 )
				{
                    printf("Error!!!!, LBAL==0x00,%x\r\n",RegValue32);
				}
			}
            */
            pDataBuffer[j++]=(MMP_UINT8)(RegValue32&0x000000FF);
		}
    }
    XD_nCE_Disable();
	return result;
}

MMP_UINT8
XD_ReadPage(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer)
{
    MMP_UINT8  result          = XD_ERROR;
    MMP_UINT32 realAddress     = MMP_NULL;
    MMP_BOOL   iserased        = MMP_FALSE;
    MMP_UINT32* pdwDataBuf,RegValue32;
    MMP_UINT8* pAlignDataBuf   = (MMP_UINT8*)(((MMP_UINT32)pDataBuffer + 3) & ~3);
    MMP_UINT32 i               = 0;
    MMP_UINT8  XdDmaDataBuffer[528];

#ifdef	ENABLE_XD_MSG
    //XD_DbgPrintf("XDRS:%x, page(%u)\n", pba, ppo);
	printf("XDRS:(%x,%x)\n", pba, ppo);
	//printf("p\n");
#endif

	if(pba==0x3E8 && !ppo)
	{
		for(i=0; i<512; i++)
		{
			printf("%02X ",pDataBuffer[i]);
			if( (i&0x0F)==0x0F )	printf("\n");
		}
		printf("\n");
	}
	if(pba==0x18 && !ppo)
	{
		for(i=0; i<512; i++)
		{
			printf("%02X ",pDataBuffer[i]);
			if( (i&0x0F)==0x0F )	printf("\n");
		}
		printf("\n");
	}
    // Switch controler to XD
    EnableXDControl();
    XD_nCE_Enable();

	ithWriteRegA(SMI_WRAP_DATA_LENGTH, 0x200);	//read 512 bytes 

#ifdef	ENABLE_XD_WRAP_DMA_MODE
    ithWriteRegA(SMI_WRAP_CONTROL, 0x51);	//set READ direction, and wrap fire
#else
    #if defined(TEMP_FREERTOS)
	ithWriteRegA(SMI_WRAP_CONTROL, 0x41);	//set READ direction, and wrap fire
    #else
	ithWriteRegA(SMI_WRAP_CONTROL, 0x41);	//set READ direction, and wrap fire
    #endif
#endif

    realAddress = ((pba * XD_PAGEPERBLOCK) + ppo) ;

	XD_WriteRegisterMask(SMI_CTRL, SMI_CTRL_Ecc_En,SMI_CTRL_Ecc_En);

	ithWriteRegA(SMI_CMD1VLU, XD_READ_CMD_1ST);
	ithWriteRegA(SMI_ADR4, ((realAddress>>16)&0xFF));
	ithWriteRegA(SMI_ADR3, ((realAddress>>8)&0xFF));
	ithWriteRegA(SMI_ADR2, (realAddress&0xFF));
	ithWriteRegA(SMI_ADR1, 0x00);

	ithWriteRegA(SMI_RDNTLEN, 0x10);

	#ifdef	ENABLE_XD_WRAP_DMA_MODE
	pdwDataBuf=(MMP_UINT32*)pDataBuffer;
	#else	
	#ifdef TEMP_FREERTOS
	if ( pAlignDataBuf != pDataBuffer )
	{
	    pdwDataBuf=(MMP_UINT32*)g_pXDdataBuf;
	}
	else
	{
	    pdwDataBuf=(MMP_UINT32*)pDataBuffer;
	}
	#else
	pdwDataBuf=(MMP_UINT32*)pDataBuffer;
	#endif	
	#endif	

    #if defined(TEMP_FREERTOS)
	ithWriteRegMaskA(SMI_WRAP_CONTROL, 0x51, 0x51);
    #else
	ithWriteRegMaskA(SMI_WRAP_CONTROL, 0x41, 0x41);//set READ direction, and wrap fire
    #endif

    result = XD_WaitCmdReady(XD_READ);

	if(result)
	{
		printf("WaitCmdReadyError,R=%x\r\n",result);
		return result;
	}

	if( gSM_Addr4 )
    {
		ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr4|RTrg_Rdnt |RTrg_WaitRnB|RTrg_Dat));
    }
    else
    {
		ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr3|RTrg_Rdnt |RTrg_WaitRnB|RTrg_Dat));
    }

	#ifdef	ENABLE_XD_WRAP_DMA_MODE
	XD_DmaCopyWrapToMem(pdwDataBuf);
	//XD_DmaCopyWrapToMem(XdDmaDataBuffer);
	//HOST_ReadBlockMemory((MMP_UINT32)pdwDataBuf, XdDmaDataBuffer, 512);
	#else
	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);
	if(result)
	{
		printf("WaitTriggerError,R=%x\r\n",result);
		return 	result;
	}
    #endif

#ifndef	ENABLE_XD_WRAP_DMA_MODE
	result = XD_WaitXDWrapFullEmpty(WAIT_WRAP_FULL);
	if(result)
	{
		printf("WaitWrapError,R=%x\r\n",result);
		return result;
	}
    //read data from wrap data port 4-bytes by 4-bytes
	for(i=0;i<128;i++)
	{		
		pdwDataBuf[i] = ithReadRegA(SMI_WRAP_DATA_PORT);
	}
#endif//ENABLE_XD_WRAP_DMA_MODE   

#if defined(__OPENRTOS__)
		// Clean cache
		ithInvalidateDCacheRange((void*)pdwDataBuf, XD_PAGE_SIZE);
		#ifndef	ENABLE_XD_WRAP_DMA_MODE
		if( pAlignDataBuf != pDataBuffer )
		{
			//XD_DmaCopy(pDataBuffer, (void*)pdwDataBuf, XD_PAGE_SIZE);
			memcpy(pDataBuffer, (MMP_UINT8*)pdwDataBuf, XD_PAGE_SIZE);
			ithInvalidateDCacheRange((void*)pDataBuffer, XD_PAGE_SIZE);
		}
		#endif
#endif

	//check ecc error
	XD_ReadRegister(SMI_ERR,&RegValue32);

	if(RegValue32&(Ecc1_2bit|Ecc2_2bit))
	{
		result = XD_ERROR;
	}

	if(RegValue32&(Lba1_PrytErr|Lba2_PrytErr))
	{
	    printf("XD_LBA parity check Error:%x\n\r", RegValue32);
	    result  = XD_ERROR;
	}

	if(RegValue32&(Lba1_OvrRng|Lba2_OvrRng))
    {
	    MMP_UINT32  Reg_1,Reg_2;
	    XD_ReadRegister(SMI_RDNT6,&Reg_1);
	    XD_ReadRegister(SMI_RDNT7,&Reg_2);
        Reg_1&=0xFF;
        Reg_2&=0xFF;

	    if( (Reg_1!=0xFF) || (Reg_2!=0xFF) )
    	{
    		if( (Reg_1!=0x00) || (Reg_2!=0x00) )
    		{
            	printf("SMI_ERR=%x,SMI_RDNT6=%x,SMI_RDNT7=%x\n\r",RegValue32, Reg_1,Reg_2);
	        	printf("XD_Read LBA over Range:%x\n\r", RegValue32);

	        	XD_ReadRegister(SMI_ADR1,&Reg_1);
	        	XD_ReadRegister(SMI_ADR2,&Reg_2);

	        	printf("Adr1=%x,Adr2=%x,",Reg_1, Reg_2);
	        	XD_ReadRegister(SMI_ADR3,&Reg_1);
	        	XD_ReadRegister(SMI_ADR4,&Reg_2);
	        	printf("Adr3=%x,Adr4=%x.\n\r",Reg_1, Reg_2);
	        }
    	}
    }

    //if ecc is pass, check spare data ?= 0xff
    if( result == XD_OK )
    {
    	iserased        = MMP_TRUE;
        for ( i=SMI_RDNT0; i<=SMI_RDNTF; i+=4 )
		{
            XD_ReadRegister(i,&RegValue32);
            if ( (RegValue32&0xFF) != 0xFF )
			{
				iserased = MMP_FALSE;
				break;
			}
		}
    }

	if ( iserased )	result = XD_ERASED;
	else		    result = XD_OK;

	XD_DbgPrintf("[LEAVE]XD_ReadPage: \n");
	
	if(result == XD_OK)
	{
		if( g_tmpPba && g_tmpPba==pba && ppo==0x18 )
		{
			printf("pData=[%x]\n",pDataBuffer);
			for(i=0; i<512; i++)
			{
				printf("%02X ", pDataBuffer[i]);
				if( (i&0x0F)==0x0F )	printf("\n");
			}
			printf("\n");
		}
	}

	return result;
}

MMP_UINT8
XD_ReadSpare(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pSpareBuffer)
{
    MMP_UINT8  result          = XD_ERROR;
    MMP_UINT32 realAddress     = MMP_NULL;
    MMP_BOOL   iserased        = MMP_TRUE;
    MMP_UINT32  RegData;
    MMP_UINT8*   pTempSprPt=pSpareBuffer;
//#ifdef __OPENRTOS__
    MMP_UINT32 i               = 0;
/*
#endif
#ifdef WIN32
	MMP_UINT8* VramBaseAddress = MMP_NULL;
    MMP_UINT16 i               = 0;
#endif
*/
#ifdef	ENABLE_XD_MSG
    printf("Rspr:%x,%x,%x.\n\r", pba, ppo, pSpareBuffer);
	//printf("r\r");
#endif

    EnableXDControl();
	XD_nCE_Enable();

	realAddress = ((pba * XD_PAGEPERBLOCK) + ppo) ;

	XD_WriteRegisterMask(SMI_CTRL, ~SMI_CTRL_Ecc_En,SMI_CTRL_Ecc_En);//disable ECC ?? but, why

	ithWriteRegA(SMI_CMD1VLU, XD_READ_CMD_SPARE);
	ithWriteRegA(SMI_ADR4, ((realAddress>>16)&0xFF));
	ithWriteRegA(SMI_ADR3, ((realAddress>>8)&0xFF));
	ithWriteRegA(SMI_ADR2, (realAddress&0xFF));
	ithWriteRegA(SMI_ADR1, 0x00);
	ithWriteRegA(SMI_RDNTLEN, 0x10);

	if( gSM_Addr4 )
    {
		ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr4|RTrg_Rdnt |RTrg_WaitRnB));
    }
    else
    {
		ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr3|RTrg_Rdnt |RTrg_WaitRnB));
    }

	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);
	if(result)
	{
		return 	result;
	}

	//result = XD_WaitCmdReady(XD_READ_ID);

    if(!result)
	{
        //read data from register
        for ( i=SMI_RDNT0; i<=SMI_RDNTF; i+=4 )
	    {
            XD_ReadRegister(i, &RegData);
            //printf("%08x ",RegData);
            *(pTempSprPt)=(MMP_UINT8)(RegData&0xFF);
            pTempSprPt++;
        }
        pTempSprPt=pSpareBuffer;

		for ( i=0; i<XD_SPARE_SIZE; i++ )
		{
			// we should go through on all data+spare
			if ( pTempSprPt[i] != 0xFF )
			{
				iserased = MMP_FALSE; // simple erase chk
				break;
			}
		}

        XD_nCE_Disable();

		if ( iserased )
		{
			XD_nCE_Disable();
		    return XD_ERASED;
		}
		else
		{
			XD_nCE_Disable();
		    return XD_OK;
		}
	}
    else
    {
        XD_nCE_Disable();
        return XD_ERROR;
	}

    XD_nCE_Disable();
	return XD_ERROR;
}

MMP_UINT8
GetXDId( unsigned long* pNumOfBlocks)
{
	MMP_UINT8  result = XD_ERROR;
	MMP_UINT32 regData;

	//1.check XD 9A ID
	EnableXDControl();
	XD_nCE_Enable();

	ithWriteRegA(SMI_CMD1VLU, XD_READID9A_CMD_1ST);
	ithWriteRegA(SMI_ADR1, 0x00);
	ithWriteRegA(SMI_RDNTLEN, 0x03);
	ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr1|RTrg_Rdnt ) );


	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);
	if(result)
	{
			return 	result;
	}

	XD_nCE_Disable();

	XD_ReadRegister(SMI_RDNT2, &regData);
	
	if( regData != 0xB5 )
	{
		printf("\n It's not xD card");
		result= XD_ERROR; 
		return 	result;
	}


	//2.Read 0x90 2nd ID code
	XD_nCE_Enable();

	ithWriteRegA(SMI_CMD1VLU, XD_READID90_CMD_1ST);
	ithWriteRegA(SMI_ADR1, 0x00);
	ithWriteRegA(SMI_RDNTLEN, 0x02);
	ithWriteRegA(SMI_RTRG, (RTrg_Cmd1|RTrg_Adr1|RTrg_Rdnt ) );

	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);
	if(result)
	{
			return 	result;
	}

	XD_nCE_Disable();

	//3.check XD size(set pNumOfBlocks)
		XD_ReadRegister(SMI_RDNT1, &regData);
		regData&=0xFF;
        result=XD_OK;
        
        //printf("XD ID=%x\n",regData);

		if (regData == 0x73)     // 16 MByte
		{
		   *pNumOfBlocks  = 1024;
        }
		else if(regData == 0x75) // 32 MByte
		{
		   *pNumOfBlocks  = 2048;
  		}
		else if(regData == 0x76) // 64 MByte
		{
		   *pNumOfBlocks  = 4096;
		}
		else if(regData == 0x79) // 128 MByte
		{
		   *pNumOfBlocks  = 8192;
		}
		else if(regData == 0x71) // 256 MByte
		{
		   *pNumOfBlocks  = 16384;
		}
		else if(regData == 0xDC) // 512 MByte
		{
		   *pNumOfBlocks  = 32768;
		}
		else if(regData == 0xF1) // 512 MByte
		{
		   *pNumOfBlocks  = 32768;
		}
		else if(regData == 0xD3) // 1G Byte
		{
		   *pNumOfBlocks  = 65536;
		}
		else if(regData == 0xD5) // 2G Byte
		{
		   //*pNumOfBlocks  = 131072;
		   *pNumOfBlocks  = 8192;
		}
		else
		{
			*pNumOfBlocks = 0;
			result = XD_ERROR;
		}
	//}

	return result;
}

MMP_UINT8
XD_WaitXDWrapFullEmpty(MMP_UINT8 Type)
{
    MMP_UINT32 regData32,dwWaitFlag,TimeOutCnt=0xFFFFF;

    if(Type)
    {
    	dwWaitFlag=WRAP_FULL_BIT;
    }
    else
    {
    	dwWaitFlag=WRAP_EMPTY_BIT;
    }

	regData32 = ithReadRegA(SMI_WRAP_STATUS);

    while( !(regData32&dwWaitFlag) )
    {
        if(--TimeOutCnt==0x00)
        {
            printf("XD Wait wrap full time out!!\r\n");
            return  XD_ERROR;
        }
		regData32 = ithReadRegA(SMI_WRAP_STATUS);
    }
    return  XD_OK;
}


MMP_UINT8
XD_WaitReadWriteTrigger(MMP_UINT8 Type)
{
	MMP_UINT32 regData32,TimeOutCnt=0x7FFFFF;
	MMP_UINT32	XD_TRIGGER_REG;

	if(Type)
	{
		XD_TRIGGER_REG=(MMP_UINT32)SMI_WTRG;
	}
	else
	{
		XD_TRIGGER_REG=(MMP_UINT32)SMI_RTRG;
	}

	regData32 = ithReadRegA(XD_TRIGGER_REG);
	regData32&=0xFF;

	while (regData32 != 0x00)
	{
		if(--TimeOutCnt==0x00)
		{
		    printf("1.XD Wait Read/Write Trigger time out[1][%x,%x]!!!\n",XD_TRIGGER_REG,regData32);
			regData32 = ithReadRegA(0xDED00104);
		    printf("1.XD Wait Read/Write Trigger time out[2][%x,%x]!!!\n",XD_TRIGGER_REG,regData32);
		    //while(1);
		    return XD_ERROR;
		}
		regData32 = ithReadRegA(XD_TRIGGER_REG);
		//printf("1.XD Wait Read/Write Trigger time out[2][%x,%x]!!!\n",XD_TRIGGER_REG,regData32);
        regData32&=0xFF;
    }
    return  XD_OK;
}

void XD_WriteRegisterMask(MMP_UINT32 RegisterAddress, MMP_UINT32 Data, MMP_UINT32 Mask)
{
    MMP_UINT32  dRegValue,dWriteData,RMask,dwByte0,dwByte1,dwByte2,dwByte3;

    XD_ReadRegister(RegisterAddress,&dRegValue);

    RMask=(~Mask)&0xFF;
    Mask&=0xFF;
    dWriteData=(dRegValue&RMask)|(Data&Mask);
    ithWriteRegA(RegisterAddress,dWriteData);
}

void XD_ReadRegister(MMP_UINT32 RegisterAddress, MMP_UINT32 *dRegValue)
{
    MMP_UINT32  dRegValue32,dByte1;

	dRegValue32 = ithReadRegA(RegisterAddress);

    *dRegValue=dRegValue32&0xFF;
}

#ifdef TEMP_FREERTOS
void XD_DmaCopy(void* dst, void* src, MMP_UINT32 size)
{
    const MMP_UINT32 attribList[] =
    {
        MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_MEM,
        MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)src,
        MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)dst,
        MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)size,
        MMP_DMA_ATTRIB_NONE
    };

    mmpDmaSetAttrib(g_XdDmaContent, attribList);
    mmpDmaFire(g_XdDmaContent);

    if (mmpDmaWaitIdle(g_XdDmaContent))
    {
        printf("XdDmaCopy: Copy Failed.\n");
    }
}

#ifdef	ENABLE_XD_WRAP_DMA_MODE
void XD_DmaCopyWrapToMem(void* dst)
{	
	MMP_UINT8	result;
    const MMP_UINT32 attribList[] =
    {
    	MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_XD_TO_MEM,
    	MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)0xDED001A0,
    	MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)dst,
    	MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)512,    	
    	MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    	MMP_DMA_ATTRIB_SRC_TX_WIDTH, 4,
    	MMP_DMA_ATTRIB_SRC_BURST_SIZE, 128,
    	MMP_DMA_ATTRIB_NONE
    };

    mmpDmaSetAttrib(g_XdWrapDmaContent, attribList);
    
    mmpDmaFire(g_XdWrapDmaContent);
    
	result = XD_WaitReadWriteTrigger(WAIT_READ_TRIGGER);	
    
    if (mmpDmaWaitIdle(g_XdWrapDmaContent))
    {
        printf("XdDmaCopyWrap: Copy Failed.\n");
    }
}


void XD_DmaCopyMemToWrap(void* src)
{
    const MMP_UINT32 attribList[] =
    {
    	MMP_DMA_ATTRIB_DMA_TYPE, (MMP_UINT32)MMP_DMA_TYPE_MEM_TO_XD,
    	MMP_DMA_ATTRIB_SRC_ADDR, (MMP_UINT32)src,
    	MMP_DMA_ATTRIB_DST_ADDR, (MMP_UINT32)0xDED001A0,
    	MMP_DMA_ATTRIB_TOTAL_SIZE_IN_BYTE, (MMP_UINT32)512,
    	MMP_DMA_ATTRIB_HW_HANDSHAKING, (MMP_UINT32)MMP_TRUE,
    	MMP_DMA_ATTRIB_SRC_TX_WIDTH, 4,
    	MMP_DMA_ATTRIB_SRC_BURST_SIZE, 128,
    	MMP_DMA_ATTRIB_NONE
    };
    
    mmpDmaSetAttrib(g_XdWrapDmaContent, attribList);

    mmpDmaFire(g_XdWrapDmaContent);
    
    #if defined(__OPENRTOS__)
	ithWriteRegA(SMI_WRAP_CONTROL	,	0x11);//set write direction, and wrap fire
	#else
	ithWriteRegA(SMI_WRAP_CONTROL	,	0x31);//set write direction, and wrap fire
	#endif
	
	if( gSM_Addr4 )
	{
		ithWriteRegA(SMI_WTRG,(WTrg_Cmd1|WTrg_Adr4|WTrg_Cmd2|WTrg_Dat|WTrg_Rdnt|WTrg_WaitRnB));
	}
	else
	{
		ithWriteRegA(SMI_WTRG,(WTrg_Cmd1|WTrg_Adr3|WTrg_Cmd2|WTrg_Dat|WTrg_Rdnt|WTrg_WaitRnB));
	}

    if (mmpDmaWaitIdle(g_XdWrapDmaContent))
    {
        printf("XdDmaCopyWrap: Copy Failed.\n");
    }

}
#endif

#endif
