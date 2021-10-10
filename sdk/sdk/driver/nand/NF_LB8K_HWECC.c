#include "host/host.h"
#include "sys/sys.h"
#include "ite/ith.h"
#include "mem/mem.h"
#include "nandflash/NF_LB_HWECC.h"

#ifdef LARGEBLOCK_8KB
#ifdef NF_HW_ECC

#ifdef NAND_DEBUG_MSG
#include <stdarg.h>
#include <stdio.h>
#include "host/host.h"
#endif

#ifdef __FREERTOS__
//#include "sys.h"
#include "nandflash/nf_dma.h"
#endif

#include "host/ahb.h"
#include "nandflash/NF_Reg.h"
//#include "xd/xd_mlayer.h"
#include "../../share/fat/ftl/ftl/mlayer.h"

////////////////////////////////////////////////////////////
//
// Compile Option
//
////////////////////////////////////////////////////////////
#define ENABLE_HW_ECC
//#define LB_ECC_RECORD
#define NF_USE_DMA_COPY
//#define SHOW_RWE_MSG
//#define ENABLE_HW_SCRAMBLER
#define PITCH_ECC_1KB_ISSUE
//#define ENABLE_NF_TEST_CODE

#ifdef ENABLE_HW_ECC
#define ECC_PAGE_SIZE       1024
  #define BCH_ECC_SIZE        (512)	//if 4kB/page NAND use BCH 24-bit ECC = (4+((BCH 24-bit)*2))*((page size)/(ECC_PAGE_SIZE)) = (4+48)*(4)= 208
  #if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
    #define ECC_LENGTH		  42
    #define BCH_MODE		  24
    #define ECC_SPARE_SIZE    24
  #else
    #define ECC_LENGTH		  21
    #define BCH_MODE		  12
    #define ECC_SPARE_SIZE    16
  #endif
#endif

#define SPARE_SIZE     ECC_SPARE_SIZE
#define BLOCK_SIZE     PAGE_SIZE * g_NfAttrib.pagePerBlock

#ifdef NF_LARGE_BLOCK_8KB
  #define PAGE_SIZE      		8192
  #define TOTAL_SPARE_SIZE		436
  #define DATA_SIZE      		8216 //PAGE_SIZE + SPARE_SIZE
#elif NF_LARGE_BLOCK_4KB
  #define PAGE_SIZE      		4096
  #define TOTAL_SPARE_SIZE		218
  #define DATA_SIZE      		4120 //PAGE_SIZE + SPARE_SIZE
#else
  #define PAGE_SIZE      		2048
  #define TOTAL_SPARE_SIZE		64
  #define DATA_SIZE      		2064 //PAGE_SIZE + SPARE_SIZE
#endif

////////////////////////////////////////////////////////////
//
//	Global Variable
//
////////////////////////////////////////////////////////////
typedef MMP_UINT8 (*BLOCK_CHECK_METHOD) (MMP_UINT32 pba);

typedef enum{
    NAND_MAKER_TOSHIBA = 0, // 0
    NAND_MAKER_SAMSUNG,     // 1
    NAND_MAKER_HYNIX,       // 2
    NAND_MAKER_NUMONYX,     // 3
    NAND_MAKER_POWERFLASH,  // 4
    NAND_UNKNOWN_MAKER
}NAND_MAKER;

typedef enum{
    NAND_TOSHIBA_METHOD_01 = 0, // Check byte 0 in data & 1st bytes in spare of 1st & 2nd page
    NAND_TOSHIBA_METHOD_02,     // Check byte 0 in data & 1st bytes in spare of last page
    NAND_TOSHIBA_METHOD_03,     // Check byte 0 in data & 1st bytes in spare of 1st page & last page
    NAND_SAMSUNG_METHOD_01,     // Check 1st bytes in spare of 1st & 2nd page
    NAND_SAMSUNG_METHOD_02,     // Check 1st bytes in spare of last page
    NAND_HYNIX_METHOD_01,       // Check 1st bytes in spare of 1st & 2nd page
    NAND_HYNIX_METHOD_02,       // Check 1st bytes in spare of last page & (last-2) page
    NAND_NUMONYX_METHOD_01,     // Check 1st & 6th byte in spare of 1st page
    NAND_NUMONYX_METHOD_02,     // Check 1st byte in spare of last page
    NAND_POWERFLASH_METHOD_01,
    NAND_UNKNOWN_METHOD
}NAND_BLOCK_CHECK_METHOD;

typedef struct NF_ATTRIBUTE_TAG
{
    // --- nandflash object attribute
	MMP_BOOL   bInit;
	MMP_BOOL   bMultiDie;
	MMP_UINT32 dieNumber;
	MMP_UINT32 blockPerDie;

	// --- Physic nandflash attribute
	MMP_UINT16              ID1;
	MMP_UINT16              ID2;
	MMP_UINT32              numOfBlocks;
    MMP_UINT8               pagePerBlock;
    MMP_UINT32              pageSize;
    NAND_MAKER              nandMaker;
    NAND_BLOCK_CHECK_METHOD checkMethod;
    BLOCK_CHECK_METHOD      pfCheckMethod;
    MMP_CHAR                modelName[256];
    MMP_UINT32              maxRealAddress;

    // --- Controller register setting
    MMP_UINT32 clockValue;
    MMP_UINT32 register701C;
}NF_ATTRIBUTE;

static MMP_UINT8  *g_pNFdataBuf          = MMP_NULL;
static MMP_UINT8  *g_pNFspareBuf         = MMP_NULL;
static MMP_UINT8  *g_pNFeccBuf           = MMP_NULL;
static MMP_UINT8  *g_pNFdataBufAllocAdr  = MMP_NULL;
static MMP_UINT8  *g_pNFspareBufAllocAdr = MMP_NULL;
static MMP_UINT8  *g_pNFeccBufAllocAdr   = MMP_NULL;


#ifdef WIN32
static MMP_UINT8  *g_pW32NFBchBuf        = MMP_NULL;
#endif

static MMP_UINT32 ReservedBlockNum       = 0;

static NF_ATTRIBUTE g_NfAttrib;

#ifdef LB_ECC_RECORD
unsigned char* g_pBlockErrorRecord = MMP_NULL;
#endif

static MMP_UINT32 g_OriGpioValue = 0;
static MMP_UINT32 g_LastDieNumber = 0;

static MMP_UINT32 g_BchMapMask			 = 0;
static MMP_UINT32 g_TotalNumOfMapBit	 = 0;

static MMP_UINT32  g_pNFeccCurrRdMask    = 0;

#if defined(NAND_IRQ_ENABLE)
static MMP_EVENT NandIsrEvent            = MMP_NULL;
#endif

//3f f6 90 fa 2d cc 75 fb 80 23 36 08 28 69 c2 63 (it is for 2kB/page)
//3f f6 90 fa 2d cc 75 fb 80 23 36 08 28 69 c2 63 (it is ?? for 4kB/page)
const MMP_UINT8  g_ScramblerTable[16] = {0x3f, 0xf6, 0x90, 0xfa, 0x2d, 0xcc, 0x75, 0xfb, 
                                         0x80, 0x23, 0x36, 0x08, 0x28, 0x69, 0xc2, 0x63};
////////////////////////////////////////////////////////////
//
// Const variable or Marco
//
////////////////////////////////////////////////////////////
static       MMP_UINT32 REGISTER701C = 0x00002105;

#define MAX_DIE_NUMBER 1

#define NF_CLOCK_DIVIDE_VALUE 0x00777777

#define IO_SELECT_MASK 0x0007
#define IO_SELECT_NAND_FLASH 0x0000

#define CF_IOWR_PULL_UP_DOWN_MASK 0x00003000
#define NDCS0_PULL_UP_DOWN_MASK 0x03000000

#define PULL_UP_CF_IOWR 0x00002000
#define PULL_UP_NDCS0 0x02000000
#define READ_STATUS_TIME_OUT_COUNT    100000

#if defined(NAND_IRQ_ENABLE)
#define NAND_INTR_MASK    0x00000010
#endif

#ifdef ENABLE_HW_SCRAMBLER
#define IsEnHwScrambler    (1)
#else
#define IsEnHwScrambler    (0)
#endif

enum {
	LL_OK,
	LL_ERASED,
	LL_ERROR,
	LL_RETRY
};

////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////
#ifdef NAND_DEBUG_MSG
void NAND_DbgPrintf(char* str, ...)
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
		void NAND_DbgPrintf(char* str, ...) {}
	#elif defined(__FREERTOS__)
		#define NAND_DbgPrintf(x, ...)
	#endif
#endif

#if defined(NAND_IRQ_ENABLE)
static void NfIntrEnable(void);
static void NfIntrDisable(void);
#endif

static MMP_UINT8       LB8K_HWECC_WaitCmdReady(NF_CMD_TYPE type);
static HWECC_RESP_TYPE LB8K_HWECC_CheckECC(MMP_UINT8 *data, MMP_UINT8 *spare, MMP_BOOL EnableEcc);

static MMP_UINT32 GetNandId(MMP_UINT8 chipIndex, NF_ATTRIBUTE* pNfAttrib);
static MMP_BOOL	  SamsungNandflash(MMP_UINT16 ID1,MMP_UINT16 ID2, NF_ATTRIBUTE* pNfAttrib);
static MMP_BOOL	  HynixNandflash(MMP_UINT16 ID1,MMP_UINT16 ID2, NF_ATTRIBUTE* pNfAttrib);
static MMP_BOOL	  ToshibaNandflash(MMP_UINT16 ID1,MMP_UINT16 ID2, NF_ATTRIBUTE* pNfAttrib);
static MMP_BOOL	  NumonyxNandflash(MMP_UINT16 ID1,MMP_UINT16 ID2, NF_ATTRIBUTE* pNfAttrib);
static MMP_BOOL	  PowerFlashNandflash(MMP_UINT16 ID1,MMP_UINT16 ID2, NF_ATTRIBUTE* pNfAttrib);

static MMP_BOOL   IsErasedBlock(MMP_UINT8* pData, MMP_UINT8* pSpare);
static MMP_BOOL   NfChipSelect(MMP_UINT32 chipIndex);
static void       EnableNfControl();
static void       ResetChipSelect();

static MMP_UINT8 NAND_CMD_2ND_READ70_STATUS(MMP_UINT32* NfStatus);
static MMP_UINT8 NAND_CMD_1ST_READID90(MMP_UINT32* CmdID0, MMP_UINT32* CmdID1, MMP_UINT32 IdCycles);

static MMP_UINT8 GetErrorPageNum(MMP_UINT32 ErrMap);
static MMP_UINT32 GetCorrectNumInThisSegment(MMP_UINT32 PageLoop);
static MMP_UINT16 GetErrorAddr(MMP_UINT8 PageLoop,MMP_UINT8 BitLoop);
static MMP_UINT8 GetErrorBit(MMP_UINT8 PageLoop,MMP_UINT8 BitLoop);
static MMP_UINT8 GetErrorPage(MMP_UINT8 PageLoop);
static void CorrectBitIn8kBDataBuffer(MMP_UINT8 *data, MMP_UINT8 *spare,MMP_UINT8 ErrorPage,MMP_UINT16 ErrorAddr,MMP_UINT8 ErrorBit);
static void CorrectAllErrorBitsIn8kBDataBuffer(MMP_UINT8 *data, MMP_UINT8 *spare);

static MMP_UINT8 IsAllData0xFF(MMP_UINT8 *pDataPtr, MMP_UINT32 Cprlen);
static void GenBchWtPattern(MMP_UINT8 *pDataPtr);
static void GenErrBit(MMP_UINT8 *pDataPtr, MMP_UINT32 Address);
static void GenErrBitsInWhilePage(MMP_UINT8 *pDataPtr, MMP_UINT8 TotalSizeInkB, MMP_UINT32 ErrBitNumOf1KB);
static void BCH_ECC_TEST(void);

static MMP_UINT8 LB8K_HWECC_Read1kBytes( MMP_UINT32 pba, MMP_UINT32 ppo, MMP_UINT8* pBuffer, MMP_UINT8  index);

static MMP_UINT8
compare2Buf(MMP_UINT8* pBuf1, MMP_UINT8* pBuf2, MMP_UINT32 CprSize);


void LB8K_HWECC_RWE_TEST(void);

// Check byte 0 in data & 1st bytes in spare of 1st & 2nd page
static MMP_UINT8
_IsBadBlockToshibaMethod01(
    MMP_UINT32 pba);

// Check byte 0 in data & 1st bytes in spare of last page
static MMP_UINT8
_IsBadBlockToshibaMethod02(
    MMP_UINT32 pba);

// Check byte 0 in data & 1st bytes in spare of 1st page & last page
static MMP_UINT8  
_IsBadBlockToshibaMethod03(
    MMP_UINT32 pba);

// Check 1st bytes in spare of 1st & 2nd page
static MMP_UINT8  
_IsBadBlockSamsungMethod01(
    MMP_UINT32 pba);

// Check 1st bytes in spare of last page
static MMP_UINT8  
_IsBadBlockSamsungMethod02(
    MMP_UINT32 pba);

// Check 1st bytes in spare of 1st & 2nd page
static MMP_UINT8  
_IsBadBlockHynixMethod01(
    MMP_UINT32 pba);

// Check 1st bytes in spare of last page & (last-2) page
static MMP_UINT8  
_IsBadBlockHynixMethod02(
    MMP_UINT32 pba);

// Check 1st & 6th byte in spare of 1st page
static MMP_UINT8  
_IsBadBlockNumonyxMethod01(
    MMP_UINT32 pba);

// Check 1st byte in spare of last page
static MMP_UINT8  
_IsBadBlockNumonyxMethod02(
    MMP_UINT32 pba);
    
static MMP_UINT8  
_IsBadBlockPowerFlashMethod01(
    MMP_UINT32 pba);
////////////////////////////////////////////////////////////
//
// Function Implement
//
////////////////////////////////////////////////////////////
#if defined(NAND_IRQ_ENABLE)
void nand_isr(void* data)
{
	MMP_UINT32	tmp=0;
	//printf("Nf.isr.in\n");
	
	AHB_ReadRegister(NF_REG_INTR, &tmp);	//for reference
	
	if(tmp&0x01)
	{
		AHB_WriteRegisterMask(NF_REG_INTR, tmp, tmp); 	/** clear nand interrupt **/		
	}
	else
	{
		printf("Nf.ISR: something wrong!!\n");
	}
	SYS_SetEventFromIsr(NandIsrEvent);
	//printf("Nf.isr.out\n");
}
#endif

#if defined(NAND_IRQ_ENABLE)
void NfIntrEnable(void)
{
	// Initialize Timer IRQ
	printf("Enable NAND IRQ~~\n");
	ithIntrDisableIrq(ITH_INTR_NAND);
	ithIntrClearIrq(ITH_INTR_NAND);

	#if defined (__FREERTOS__)
	// register NAND Handler to IRQ
	//ithIntrRegisterHandlerIrq(ITH_INTR_TIMER1 + i, timer_isr, i);
	ithIntrRegisterHandlerIrq(ITH_INTR_NAND, nand_isr, MMP_NULL);
	#endif // defined (__FREERTOS__)

	// set IRQ to edge trigger
	//ithIntrSetTriggerModeIrq(ITH_INTR_TIMER1 + i, ITH_INTR_EDGE);
	ithIntrSetTriggerModeIrq(ITH_INTR_NAND, ITH_INTR_EDGE);

	// set IRQ to detect rising edge
	//ithIntrSetTriggerLevelIrq(ITH_INTR_TIMER1 + i, ITH_INTR_HIGH_RISING);
	ithIntrSetTriggerLevelIrq(ITH_INTR_NAND, ITH_INTR_HIGH_RISING);

	// Enable IRQ
	//ithIntrEnableIrq(ITH_INTR_TIMER1 + i); 
	ithIntrEnableIrq(ITH_INTR_NAND);
	
	AHB_WriteRegisterMask(NF_REG_INTR, 0x00000000, NAND_INTR_MASK); /** enable nand interrupt **/
	
	if(!NandIsrEvent)
	    NandIsrEvent = SYS_CreateEvent();
	
	printf("NandIsrEvent=%x\n",NandIsrEvent);
	
	printf("Enable NAND IRQ~~leave\n");
}

void NfIntrDisable(void)
{
    ithIntrDisableIrq(ITH_INTR_NAND);
}
#else
#define intrEnable(a)
#define intrDisable()
#endif

MMP_UINT8
LB8K_HWECC_WaitCmdReady(
    NF_CMD_TYPE type)
{
    const MMP_UINT32 TIME_OUT = 100000;
    
    MMP_UINT32 nfStatus;
    MMP_UINT8  result  = LL_OK;
	MMP_UINT32 timeOut = TIME_OUT;
	MMP_INT    EventRst;

	NAND_DbgPrintf("[ENTER]LB8K_HWECC_WaitCmdReady\n");

	#if defined(NAND_IRQ_ENABLE)
	//AHB_
	if( (type==NF_READ) || (type==NF_WRITE) || (type==NF_ERASE) )
	{
		//printf("WaitNfReady.in,type=%x\n",type);
		EventRst = SYS_WaitEvent(NandIsrEvent, 20);
		//printf("WaitNfReady.event.got!!rst=%x\n",EventRst);
    	if(result)
    	{
    		printf("[Nf ERR]:INTR time out\n");
    	    result = LL_ERROR;
    	    goto end;
    	}
    }
	#endif

    // ----------------------------------------
    // 1. Read status and wait nandflash finish command
    do
    {
        AHB_ReadRegister(NF_REG_STATUS, &nfStatus);

		if ( timeOut-- == 0 )
		{
			AHB_ReadRegister(NF_REG_GENERAL, &nfStatus);
			printf("\tLB8K_HWECC_WaitCmdReady wait idle time out[1]!!!,0X131A=%X.\n",nfStatus);
			AHB_ReadRegister(NF_REG_CLOCK_DIVIDE, &nfStatus);
			printf("cLOCK1=%X.\r\n",nfStatus);
			//AHB_ReadRegister(NF_REG_STORAGE_ADDR3, &nfStatus);
			//printf("SprSize=%x.\r\n",nfStatus);
			result = LL_ERROR;
			goto end;
		}
    }while( (nfStatus&0xc000) != NFIDLE );

    //NAND_CMD_CHECK_STATUS();

    // ----------------------------------------
    // 2. Wait 0x70 idle
    timeOut = TIME_OUT;

    if(NAND_CMD_2ND_READ70_STATUS(&nfStatus)!=LL_OK)
    {
        printf("NAND_CMD_2ND_READ70_STATUS fail\n");
        result = LL_ERROR;
        goto end;
    }
    //nfStatus = nfStatus & 0xFF;

    // ----------------------------------------
    // 3. Check status
    switch(type)
    {
        // Check if command is pass or fail
        case NF_ERASE:
        case NF_WRITE:
            if(   (nfStatus & 0xFF) == 0xE0
    		   || (nfStatus & 0xFF) == 0xC0 )
                result = LL_OK;
            else	
                result = LL_ERROR;
            break;
            
        case NF_READ:
            if(   (nfStatus & 0x7F) == 0x60
    		   || (nfStatus & 0x7F) == 0x40 )
                result = LL_OK;
            else	
                result = LL_ERROR;
            break;
            
    	case NF_READ_ID:
    		if (   (nfStatus & 0x7F) == 0x60
                || (nfStatus & 0x7F) == 0x40 )
    			result = LL_OK;
            else	
                result = LL_ERROR; 
    		break;
    		
        default:    //unknow CMD,return error
            result = LL_ERROR;
            break;
    }
    
end:
	NAND_DbgPrintf("[LEAVE]LB8K_HWECC_WaitCmdReady: Return %s\n", result == LL_OK ? "LL_OK" : "LL_ERROR");
	return result;
}

HWECC_RESP_TYPE
LB8K_HWECC_CheckECC(
    MMP_UINT8 *data, 
    MMP_UINT8 *spare, 
    MMP_BOOL EnableEcc)
{
    HWECC_RESP_TYPE result;
    MMP_UINT32      BchStatus=0xFFFFFFFF;
    MMP_UINT16      erraddr, errvalue;

    AHB_ReadRegister(NF_REG_BCH_ECC_MAP, &BchStatus);

    if( EnableEcc == MMP_FALSE )
    {
        result = NF_NOERROR;
        //printf("1.ECC=%04x\r\n",BchStatus);
    }
    else if( ((BchStatus&g_BchMapMask)&g_pNFeccCurrRdMask)==(g_BchMapMask&g_pNFeccCurrRdMask) )//if( (BchStatus&0x0000000F)==0x0000000F)    	
    {
        result = NF_NOERROR;
        //printf("3.ECC=%08x[%08x,%08x]\r\n",BchStatus, g_BchMapMask, g_pNFeccCurrRdMask);
    }
    else
    {
        MMP_UINT8	*ECCpt;
        MMP_UINT32	p,Reg704C,Reg7050;
        MMP_UINT32	TempReg704C,TempReg7050;

        AHB_ReadRegister(NF_REG_BCH_ECC_MAP, &Reg704C);
        //printf("2.ECC=[%08x,%08x]",Reg704C,BchStatus);
        AHB_ReadRegister(NF_REG_ECC_ERROR_MAP, &Reg7050);
        //printf(",%08x,[%08x,%08x]\r\n",Reg7050, g_BchMapMask, g_pNFeccCurrRdMask);

        TempReg704C=((~Reg704C)&g_BchMapMask);
        TempReg7050=(Reg7050&g_BchMapMask);        

        //1.if in-correctable
        if( ((TempReg704C & TempReg7050)&g_pNFeccCurrRdMask) == (TempReg704C&g_pNFeccCurrRdMask) )
        {
            //printf("6.ECC, it can correction[%08x,%08x]x\n",Reg704C,Reg7050);
            CorrectAllErrorBitsIn8kBDataBuffer(data, spare);
            result = NF_NOERROR;            
            //	return BCH can not correct this error 
        }
        else
        {
            printf("5.ECC, it can not correction[%08x,%08x]x\n",Reg704C,Reg7050);
            result = NF_OVER4ERR;
        }
    }
    return result;
}

MMP_UINT8
LB8K_HWECC_Initialize(
    unsigned long* pNumOfBlocks,
    unsigned char* pPagePerBlock,
    unsigned long* pPageSize)
{
    MMP_UINT8  result          = LL_OK;
    MMP_UINT8* VramBaseAddress = 0x00;
	MMP_UINT32 NumOfBlocks, PageSize;
	MMP_UINT8  PagePerBlock;
	MMP_UINT32 i,Reg32;
	NF_ATTRIBUTE nfAttrib;
	
	NAND_DbgPrintf("[ENTER]LB8K_HWECC_Initialize: \n");
	
	AHB_ReadRegister(NF_REG_GENERAL, &Reg32);
	printf("0x701C = %x\n",Reg32);
	AHB_ReadRegister(0xD070003C, &Reg32);
	printf("NF_REG_STATUS = %x\n",Reg32);
	
	AHB_WriteRegister(NF_REG_CLOCK_DIVIDE, 0x40777777);
	
	// Switch controler to nand
	EnableNfControl();
	
    if(IsEnHwScrambler)
    {
        //it's HW limitation, It can not read ID in scramble mode
        AHB_WriteRegisterMask(NF_REG_GENERAL, 0x00000000,0x00080000);
    }
    // Set nandflash clock divider
    //AHB_WriteRegister(NF_REG_CLOCK_DIVIDE, NF_CLOCK_DIVIDE1_VALUE);
    AHB_WriteRegister(NF_REG_CLOCK_DIVIDE, 0x40777777);

	// Reset nandflash attribute
	memset(&g_NfAttrib, 0x00, sizeof(NF_ATTRIBUTE));

	if ( GetNandId(0, &nfAttrib) == LL_OK )
	{
		g_NfAttrib.bInit = MMP_TRUE;
		g_NfAttrib.dieNumber++;
		g_NfAttrib.ID1            = nfAttrib.ID1;
		g_NfAttrib.ID2            = nfAttrib.ID2;
		g_NfAttrib.blockPerDie    = nfAttrib.numOfBlocks;
		g_NfAttrib.numOfBlocks    = nfAttrib.numOfBlocks;
		g_NfAttrib.pagePerBlock   = nfAttrib.pagePerBlock;
		g_NfAttrib.pageSize       = nfAttrib.pageSize;
		g_NfAttrib.nandMaker      = nfAttrib.nandMaker;
		g_NfAttrib.checkMethod    = nfAttrib.checkMethod;
		g_NfAttrib.pfCheckMethod  = nfAttrib.pfCheckMethod;
		g_NfAttrib.clockValue     = nfAttrib.clockValue;
		g_NfAttrib.register701C   = nfAttrib.register701C;
		g_NfAttrib.maxRealAddress = ((nfAttrib.numOfBlocks - 1) * nfAttrib.pagePerBlock + (nfAttrib.pagePerBlock - 1)) * PAGE_SIZE;
		strcpy(g_NfAttrib.modelName, nfAttrib.modelName);

		for ( i=1; i<MAX_DIE_NUMBER; i++ )
		{
			if ( GetNandId((MMP_UINT8)i, &nfAttrib) == LL_OK )
			{
				g_NfAttrib.bMultiDie = MMP_TRUE;
				g_NfAttrib.dieNumber++;
				g_NfAttrib.numOfBlocks += NumOfBlocks;
			}
		}
		
		if ( g_NfAttrib.bMultiDie == MMP_FALSE )
		{
            NfChipSelect(0);
            ResetChipSelect();
        }
	}
    NAND_DbgPrintf("\t-------------------- new attribute Nand Flash Initial --------------------\n");
	NAND_DbgPrintf("\tInit %s\n", g_NfAttrib.bInit ? "OK" : "FAIL");
	NAND_DbgPrintf("\t%s\n", g_NfAttrib.bMultiDie ? "Multi-Die" : "Single-Die");
	NAND_DbgPrintf("\tNumber Of Blocks = %u\n", g_NfAttrib.numOfBlocks);
	NAND_DbgPrintf("\tPages Per Block  = %u\n", g_NfAttrib.pagePerBlock);
	NAND_DbgPrintf("\tPage Size        = %u Bytes\n", g_NfAttrib.pageSize);
	NAND_DbgPrintf("\tTotal Size       = %u MB\n", g_NfAttrib.pagePerBlock * g_NfAttrib.pageSize * (g_NfAttrib.numOfBlocks / 1024 )/ 1024);
	NAND_DbgPrintf("\tID1              = 0x%04X\n", g_NfAttrib.ID1);
	NAND_DbgPrintf("\tID2              = 0x%04X\n", g_NfAttrib.ID2);
	NAND_DbgPrintf("\tNand Maker       = %u\n", g_NfAttrib.nandMaker);
	NAND_DbgPrintf("\tCheck Method     = %u\n", g_NfAttrib.checkMethod);
	NAND_DbgPrintf("\tModel Name       = %s\n", g_NfAttrib.modelName);
	NAND_DbgPrintf("\tClock01(0x1320)  = 0x%08X\n", g_NfAttrib.clockValue);
	NAND_DbgPrintf("\t0x131C           = 0x%08X\n", g_NfAttrib.register701C);
	
    printf("\t-------------------- attribute Nand Flash Initial --------------------\n");
	printf("\tInit %s\n", g_NfAttrib.bInit ? "OK" : "FAIL");
	printf("\t%s\n", g_NfAttrib.bMultiDie ? "Multi-Die" : "Single-Die");
	printf("\tNumber Of Blocks = %u\n", g_NfAttrib.numOfBlocks);
	printf("\tPages Per Block  = %u\n", g_NfAttrib.pagePerBlock);
	printf("\tPage Size        = %u Bytes\n", g_NfAttrib.pageSize);
	printf("\tTotal Size       = %u MB\n", g_NfAttrib.pagePerBlock * g_NfAttrib.pageSize * (g_NfAttrib.numOfBlocks / 1024 )/ 1024);
	printf("\tID1              = 0x%04X\n", g_NfAttrib.ID1);
	printf("\tID2              = 0x%04X\n", g_NfAttrib.ID2);
	printf("\tNand Maker       = %u\n", g_NfAttrib.nandMaker);
	printf("\tCheck Method     = %u\n", g_NfAttrib.checkMethod);
	printf("\tModel Name       = %s\n", g_NfAttrib.modelName);
	printf("\tClock01(0x7024)  = 0x%08X\n", g_NfAttrib.clockValue);
	printf("\t0x701C           = 0x%08X\n", g_NfAttrib.register701C);

	{
		MMP_UINT32	Reg32;
		AHB_ReadRegister(NF_REG_GENERAL, &Reg32);
	}
	if (g_NfAttrib.bInit)
	{
		*pNumOfBlocks  = g_NfAttrib.numOfBlocks;
		*pPagePerBlock = g_NfAttrib.pagePerBlock;
		*pPageSize     = g_NfAttrib.pageSize;
		
		switch(g_NfAttrib.pageSize)
		{
			case 2048:
				//gNfInitInfo.EccLen = 21;
				g_BchMapMask=0x00000003;
				g_TotalNumOfMapBit = 2;
				break;
			case 4096:
				//gNfInitInfo.EccLen = 21;//not 21 when using 4KB/page NAND
				g_BchMapMask=0x0000000F;
				g_TotalNumOfMapBit = 4;
				break;
			case 8192:
				//gNfInitInfo.EccLen = 21;//not 21 when using 8KB/page NAND
				g_BchMapMask=0x000000FF;
				g_TotalNumOfMapBit = 8;
				break;
			case 16384:
				//gNfInitInfo.EccLen = 21;//not 21 when using 8KB/page NAND
				g_BchMapMask=0x0000FFFF;
				g_TotalNumOfMapBit = 16;
				break;
			case 32768:
				//gNfInitInfo.EccLen = 21;//not 21 when using 8KB/page NAND
				g_BchMapMask=0xFFFFFFFF;
				g_TotalNumOfMapBit = 32;
				break;
			default:
				result = LL_ERROR;
				goto end;
				break;
		}
		g_pNFeccCurrRdMask = g_BchMapMask;
		
		// Awin@20071015
		NAND_DbgPrintf("\tLB8K_HWECC_Initialize: numOfBlocks  = %d.\n\r", g_NfAttrib.numOfBlocks);
		NAND_DbgPrintf("\tLB8K_HWECC_Initialize: pagePerBlock = %d.\n\r", g_NfAttrib.pagePerBlock);
		NAND_DbgPrintf("\tLB8K_HWECC_Initialize: pageSize     = %d.\n\r", g_NfAttrib.pageSize);

#ifdef LB_ECC_RECORD
		if ( g_pBlockErrorRecord == NULL )
		{
	    	#ifdef __FREERTOS__	//for ARM
	    	g_pBlockErrorRecord = (MMP_UINT8 *)MEM_Allocate(NFNumOfBlocks, MEM_USER_NF);
	    	#elif defined(WIN32)
	        g_pBlockErrorRecord = (unsigned char*)SYS_Malloc(NFNumOfBlocks);
	        #endif
			
			if( g_pBlockErrorRecord != MMP_NULL )
			{
				memset(g_pBlockErrorRecord, 0x00, NFNumOfBlocks);
			}
		}
#endif
	}
	else
	{
		printf("Unknown nandflash ID\n");
		result = LL_ERROR;
		goto end;
    }
    
    // Get Vram base address
    VramBaseAddress = HOST_GetVramBaseAddress();

	if ( g_pNFdataBuf == MMP_NULL )
	{
        g_pNFdataBufAllocAdr = (MMP_UINT8*)MEM_Allocate(PAGE_SIZE+TOTAL_SPARE_SIZE+4, MEM_USER_NF);
		g_pNFdataBuf         = (MMP_UINT8*)(((MMP_UINT32)g_pNFdataBufAllocAdr + 3) & ~3);
	    if(g_pNFdataBuf == MMP_NULL)
	    {
	        result = LL_ERROR;  //error
	        goto end;
	    }
	}

	if ( g_pNFspareBuf == MMP_NULL )
	{
        g_pNFspareBufAllocAdr = (MMP_UINT8*)MEM_Allocate(SPARE_SIZE+16, MEM_USER_NF);
		g_pNFspareBuf         = (MMP_UINT8*)(((MMP_UINT32)g_pNFspareBufAllocAdr + 3) & ~3);
	    if(g_pNFspareBuf == MMP_NULL)
	    {
	        result = LL_ERROR;  //error
	        goto end;
	    }
	}

    if ( g_pNFeccBuf == MMP_NULL )
    {
        g_pNFeccBufAllocAdr = (MMP_UINT8*)MEM_Allocate(BCH_ECC_SIZE+4, MEM_USER_NF);
        g_pNFeccBuf         = (MMP_UINT8*)(((MMP_UINT32)g_pNFeccBufAllocAdr + 3) & ~3);
        if(g_pNFeccBuf == MMP_NULL)
        {
            result = LL_ERROR;  //error
            goto end;
        }
    }
    #ifdef __FREERTOS__	//for ARM
    
    #elif defined(WIN32)
    if ( g_pW32NFBchBuf == MMP_NULL )
    {
        g_pW32NFBchBuf = (MMP_UINT8*)SYS_Malloc(BCH_ECC_SIZE+4, MEM_USER_NF);
        if(g_pW32NFBchBuf == MMP_NULL)
        {
            result = LL_ERROR;  //error
            goto end;
        }
    }
    #endif

    // Set configuration enable bit at nand flash
    AHB_WriteRegister(NF_REG_GENERAL, g_NfAttrib.register701C);

	AHB_WriteRegisterMask(NF_REG_STORAGE_ADDR2, (MMP_UINT32)(SPARE_SIZE)<<8, 0x00001F00); // 0x1318
    
    AHB_WriteRegister(NF_REG_CLOCK_DIVIDE, g_NfAttrib.clockValue);
      
    // Set source data address at VRAM
    AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)(g_pNFdataBuf-VramBaseAddress) );

    // Set destination spare address 
    AHB_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR, (MMP_UINT32)(g_pNFspareBuf-VramBaseAddress) );

    // Set source data address at VRAM
    AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)(g_pNFdataBuf-VramBaseAddress) );

    // Set destination spare address
    AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)(g_pNFspareBuf-VramBaseAddress) );

    // Set ECC address
    AHB_WriteRegister(NF_REG_MEM_DST_CORR_ECC_ADDR, (MMP_UINT32)(g_pNFeccBuf-VramBaseAddress));

    //calculate & set the ECC initial value
    {
        MMP_UINT32  EccSprInitValue  = ((ECC_PAGE_SIZE+SPARE_SIZE)*8)+(BCH_MODE*14)-1;
        MMP_UINT32  EccDataInitValue = (ECC_PAGE_SIZE*8)+(BCH_MODE*14)-1;
        if(BCH_MODE==12)
        {
            AHB_WriteRegister(NF_REG_ECC_12BIT_SEG_INIT_VAL, EccDataInitValue&0x00003FFF);
            AHB_WriteRegister(NF_REG_ECC_12BIT_LAST_SEG_INIT_VAL, EccSprInitValue&0x00003FFF);
        }
        else if(BCH_MODE==24)
        {
            AHB_WriteRegister(NF_REG_ECC_24BIT_SEG_INIT_VAL, EccDataInitValue&0x00003FFF);
            AHB_WriteRegister(NF_REG_ECC_24BIT_LAST_SEG_INIT_VAL, EccSprInitValue&0x00003FFF);
        }
        else if(BCH_MODE==40)
        {
            AHB_WriteRegister(NF_REG_ECC_40BIT_SEG_INIT_VAL, EccDataInitValue&0x00003FFF);
            AHB_WriteRegister(NF_REG_ECC_40BIT_LAST_SEG_INIT_VAL, EccSprInitValue&0x00003FFF);    
        }        
    }

    //Enable scrambler function!!
    if(IsEnHwScrambler)
    {
        AHB_WriteRegisterMask(NF_REG_GENERAL, 0x00080000,0x00080000);
    }

    
    //for debug
    //AHB_WriteRegisterMask(NF_REG_GENERAL, 1<<28, 1<<28);
    
    // Reset nandflash chip
	//HOST_WriteRegister(NF_REG_CMD2,RESET_CMD_2ST);
    AHB_WriteRegister(NF_REG_CMD2, RESET_CMD_2ST);
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    result = LB8K_HWECC_WaitCmdReady(NF_READ_ID);

    //initial NF IRQ    
    #if defined(NAND_IRQ_ENABLE)
    NfIntrEnable();
    #endif

#if defined(__FREERTOS__) && defined(NF_USE_DMA_COPY)
	// Initial DMA
	if ( !NF_InitDma() )
	{
	    printf("LB8K_HWECC_Initialize: Initial DMA fail.\n");
	    result = LL_ERROR;
        goto end;
	}
#endif

end:
    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_Initialize: return %s\n", result == LL_OK ? "OK" : "FALSE");

    return result;
}

void
LB8K_HWECC_Getwear(
    MMP_UINT32 blk,
    MMP_UINT8 *dest)
{
	//NOTHING TO DO
    NAND_DbgPrintf("[ENTER]LB8K_HWECC_Getwear: \n");
    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_Getwear: \n");
}
    
MMP_UINT8
LB8K_HWECC_Erase(
    MMP_UINT32 pba)
{
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT32 realAddress = 0x00;
	MMP_UINT32 dieNumber   = 0;

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_Erase: pba(%d)\n", pba);
#ifdef SHOW_RWE_MSG
printf("Ers[%x]\r\n",pba);
#endif
//while(1);
    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

	// ----------------------------------------
    // 2. Calculate real address from input block address
    realAddress = pba * g_NfAttrib.pagePerBlock * PAGE_SIZE;
    //realAddress = pba * g_NfAttrib.pagePerBlock * 0x10000;

    // ----------------------------------------
    // 3. Set erase block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)realAddress);

    // ----------------------------------------
    // 4. Disable write protect bit
    AHB_WriteRegisterMask(NF_REG_GENERAL,1<<17, 1<<17); // 0x131C

    // ----------------------------------------
    // 5. Fire HW and wait ready
    AHB_WriteRegisterMask(NF_REG_GENERAL, 0x00100000, 0x01F00000);
    AHB_WriteRegister(NF_REG_CMD1, ERASE_CMD_1ST);
    AHB_WriteRegister(NF_REG_CMD2, ERASE_CMD_2ND);
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);

    #if defined(NAND_IRQ_ENABLE)
    #else
    MMP_Sleep(2);
    #endif

    result = LB8K_HWECC_WaitCmdReady(NF_WRITE);	//NAND write command fire
    // ----------------------------------------
    // 6. Enable write protect bit and disable CE pin
    //HOST_WriteRegisterMask(NF_REG_GENERAL, (MMP_UINT16)0x0000, 0x0800); // 0x131A
	
    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_Erase: return %s\n", result == LL_OK ? "OK" : "FALSE");

    return result;
}

MMP_UINT8
LB8K_HWECC_Write(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer,
    MMP_UINT8* pSpareBuffer)
{
    MMP_UINT8       result         = LL_ERROR;
    MMP_UINT32      realAddress    = 0x00;
#ifdef __FREERTOS__
    MMP_UINT8*      pAlignDataBuf  = (MMP_UINT8*)(((MMP_UINT32)pDataBuffer + 3) & ~3);
    MMP_UINT8*      pAlignSpareBuf = (MMP_UINT8*)(((MMP_UINT32)pSpareBuffer + 3) & ~3);
#endif

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_Write: pba(%d), ppo(%d)\n", pba, ppo);

#ifdef SHOW_RWE_MSG
    printf("W1[%x,%x]\r\n",pba,ppo);
    //printf("W1[%x,%x,%x,%x]\r\n",pba,ppo,pDataBuffer,pSpareBuffer);
#endif
//while(1);    
    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();

    if(IsEnHwScrambler)
    {
        //AHB_WriteRegister(NF_REG_WR_SCRAMB_INIT, 0x000000FF);
        AHB_WriteRegister(NF_REG_WR_SCRAMB_INIT, ppo);//reset scrambler initial value
    }

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Calculate real address from input block address
    realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE; 
    //realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * 0x10000; 

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)realAddress);

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__
	ithInvalidateDCacheRange(pDataBuffer, PAGE_SIZE);	
    if ( pAlignDataBuf != pDataBuffer )
    {
        memcpy(g_pNFdataBuf, pDataBuffer, PAGE_SIZE);
        ithInvalidateDCacheRange(g_pNFdataBuf, PAGE_SIZE);
        
        // --- Set source data address at VRAM
		AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
    }
    else
    {
        // --- Set source data address at VRAM
		AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)pDataBuffer);
		//AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
    }
    
    ithInvalidateDCacheRange(pSpareBuffer, SPARE_SIZE);
    if ( pAlignSpareBuf != pSpareBuffer )
    {
    	ST_SPARE* pSpare = (ST_SPARE*)g_pNFspareBuf;
    	
        memcpy(g_pNFspareBuf, pSpareBuffer, SPARE_SIZE);
        
        #if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
        if(SPARE_SIZE==24)
	    {
            pSpare->dummy1 = WRITE_SYMBOL;
            pSpare->dummy2 = 0xFFFFFFFF;
	    }
	    #endif
	    
        ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);
        // --- Set destination spare address
        AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
    }
    else
    {
    	ST_SPARE* pSpare = (ST_SPARE*)pSpareBuffer;
    	
    	#if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
        if(SPARE_SIZE==24)
	    {
            pSpare->dummy1 = WRITE_SYMBOL;
            pSpare->dummy2 = 0xFFFFFFFF;
	    }
	    #endif
	    
        ithInvalidateDCacheRange(pSpareBuffer, SPARE_SIZE);

        // --- Set destination spare address
        AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)pSpareBuffer);
    }
    AHB_WriteRegister(NF_REG_MEM_DST_CORR_ECC_ADDR, (MMP_UINT32)g_pNFeccBuf);
#elif defined(WIN32)

    #if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
    if(SPARE_SIZE==24)
	{
		ST_SPARE* pSpare = (ST_SPARE*)pSpareBuffer;
	    pSpare->dummy1 = WRITE_SYMBOL;
        pSpare->dummy2 = 0xFFFFFFFF;
	}
	#endif
	
	//usb2spi_WriteMemory((MMP_UINT32)g_pNFdataBuf, (MMP_UINT32)pDataBuffer, PAGE_SIZE);
	//usb2spi_WriteMemory((MMP_UINT32)g_pNFspareBuf, (MMP_UINT32)pSpareBuffer, SPARE_SIZE);
	HOST_WriteBlockMemory((MMP_UINT32)g_pNFdataBuf,  (MMP_UINT32)pDataBuffer, PAGE_SIZE);
	HOST_WriteBlockMemory((MMP_UINT32)g_pNFspareBuf, (MMP_UINT32)pSpareBuffer, SPARE_SIZE);

	AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
	AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
	AHB_WriteRegister(NF_REG_MEM_DST_CORR_ECC_ADDR, (MMP_UINT32)g_pNFeccBuf);
#endif

    // ----------------------------------------
    // 5. Disable write protect bit
	AHB_WriteRegisterMask(NF_REG_GENERAL,0x00020000, 0x00020000); // 0x131A

	
    // ----------------------------------------
    // 6. Fire HW and wait ready
	AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
    AHB_WriteRegisterMask(NF_REG_STORAGE_ADDR2, (MMP_UINT32)(SPARE_SIZE)<<8, 0x00001F00); // 0x1318
    
	AHB_WriteRegister(NF_REG_CMD1, PROGRAM_CMD_1ST);
	AHB_WriteRegister(NF_REG_CMD2, PROGRAM_CMD_2ND);
	AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);

    #if defined(NAND_IRQ_ENABLE)
    #else
	MMP_Sleep(1);
    #endif
    
    result = LB8K_HWECC_WaitCmdReady(NF_WRITE);	//NAND write command fire

    // ----------------------------------------
    // 7. Enable write protect bit and disable CE pin
    //HOST_WriteRegisterMask(NF_REG_GENERAL,(MMP_UINT16)0x0000,0x0800); // 0x1316

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_Write: return %s\n", result == LL_OK ? "OK" : "FALSE");

    return result;
}

MMP_UINT8
LB8K_HWECC_LBA_Write(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer,
    MMP_UINT8* pSpareBuffer)
{
   MMP_UINT8       result         = LL_ERROR;
    MMP_UINT32      realAddress    = 0x00;
	MMP_UINT32      LbaAddr    = PAGE_SIZE+TOTAL_SPARE_SIZE;
#ifdef __FREERTOS__
    MMP_UINT8*      pAlignDataBuf  = (MMP_UINT8*)(((MMP_UINT32)pDataBuffer + 3) & ~3);
    MMP_UINT8*      pAlignSpareBuf = (MMP_UINT8*)(((MMP_UINT32)pSpareBuffer + 3) & ~3);
#endif

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_Write: pba(%d), ppo(%d)\n", pba, ppo);

#ifdef SHOW_RWE_MSG
    printf("lba.W1[%x,%x]\r\n",pba,ppo);
    //printf("W1[%x,%x,%x,%x]\r\n",pba,ppo,pDataBuffer,pSpareBuffer);
#endif
//while(1);    
    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();
    {
        MMP_UINT32 Reg32;

        AHB_WriteRegisterMask(NF_REG_CMD_FIRE, NF_CMD_SW_RESET, NF_CMD_SW_RESET);   //
        if(IsEnHwScrambler)
        {
            //it's scrambler bug, this register need to be reset after previous reading fire
            AHB_WriteRegister(NF_REG_WR_SCRAMB_INIT, 0x000000FF);
            //AHB_WriteRegister(NF_REG_WR_SCRAMB_INIT, ppo);
        }
        AHB_WriteRegister(NF_REG_USER_DEF_CTRL, 0x80000000|LbaAddr );
    }

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Calculate real address from input block address
    realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE; 
    //realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * 0x10000; 

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)realAddress);

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__
	ithInvalidateDCacheRange(pDataBuffer, PAGE_SIZE);	
    if ( pAlignDataBuf != pDataBuffer )
    {
        memcpy(g_pNFdataBuf, pDataBuffer, PAGE_SIZE);
        ithInvalidateDCacheRange(g_pNFdataBuf, PAGE_SIZE);
        
        // --- Set source data address at VRAM
		AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);

        //HOST_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)(g_pNFdataBuf) & 0x0000FFFF));
        //HOST_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)(g_pNFdataBuf) & 0xFFFF0000) >> 16));
    }
    else
    {
        // --- Set source data address at VRAM
        AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)pDataBuffer);
        //HOST_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)pDataBuffer & 0x0000FFFF));
        //HOST_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)pDataBuffer & 0xFFFF0000) >> 16));
    }
    
    ithInvalidateDCacheRange(pSpareBuffer, SPARE_SIZE);
    if ( pAlignSpareBuf != pSpareBuffer )
    {
    	ST_SPARE* pSpare = (ST_SPARE*)g_pNFspareBuf;
    	
        memcpy(g_pNFspareBuf, pSpareBuffer, SPARE_SIZE);
        
        #if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
        if(SPARE_SIZE==24)
	    {
		    ST_SPARE* pSpare = (ST_SPARE*)pSpareBuffer;
	        pSpare->dummy1 = WRITE_SYMBOL;
            pSpare->dummy2 = 0xFFFFFFFF;
	    }
	    #endif
	    
        ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);
        // --- Set destination spare address
		AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);

        //HOST_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)g_pNFspareBuf & 0x0000FFFF));
        //HOST_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)g_pNFspareBuf & 0xFFFF0000) >> 16));
    }
    else
    {
    	ST_SPARE* pSpare = (ST_SPARE*)pSpareBuffer;
    	
    	#if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
        if(SPARE_SIZE==24)
	    {
		    ST_SPARE* pSpare = (ST_SPARE*)pSpareBuffer;
	        pSpare->dummy1 = WRITE_SYMBOL;
            pSpare->dummy2 = 0xFFFFFFFF;
	    }
	    #endif
	    
        ithInvalidateDCacheRange(pSpareBuffer, SPARE_SIZE);

        // --- Set destination spare address
        AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)pSpareBuffer);
        //HOST_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)pSpareBuffer & 0x0000FFFF));
        //HOST_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)pSpareBuffer & 0xFFFF0000) >> 16));
    }
    AHB_WriteRegister(NF_REG_MEM_DST_CORR_ECC_ADDR, (MMP_UINT32)g_pNFeccBuf);
#elif defined(WIN32)
	HOST_WriteBlockMemory((MMP_UINT32)g_pNFdataBuf,  (MMP_UINT32)pDataBuffer, LbaAddr);
	//HOST_WriteBlockMemory((MMP_UINT32)g_pNFspareBuf, (MMP_UINT32)pSpareBuffer, SPARE_SIZE);

	AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
	AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
	AHB_WriteRegister(NF_REG_MEM_DST_CORR_ECC_ADDR, (MMP_UINT32)g_pNFeccBuf);
#endif

    // ----------------------------------------
    // 5. Disable write protect bit
	AHB_WriteRegisterMask(NF_REG_GENERAL,0x00020000, 0x00020000); // 0x131A

	AHB_WriteRegisterMask(NF_REG_STORAGE_ADDR2, (MMP_UINT32)(SPARE_SIZE)<<8, 0x00001F00); // 0x1318
	
	
    // ----------------------------------------
    // 6. Fire HW and wait ready
	AHB_WriteRegisterMask(NF_REG_GENERAL, 0x00100000, 0x01F00000);
	AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
	AHB_WriteRegister(NF_REG_CMD1, PROGRAM_CMD_1ST);
	AHB_WriteRegister(NF_REG_CMD2, PROGRAM_CMD_2ND);

	AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    
    #if defined(NAND_IRQ_ENABLE)
    #else
	MMP_Sleep(1);
    #endif
    
    result = LB8K_HWECC_WaitCmdReady(NF_WRITE);	//NAND write command fire
	AHB_WriteRegister(NF_REG_USER_DEF_CTRL, 0x00000000 );

    // ----------------------------------------
    // 7. Enable write protect bit and disable CE pin
    //HOST_WriteRegisterMask(NF_REG_GENERAL,(MMP_UINT16)0x0000,0x0800); // 0x1316

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_Write: return %s\n", result == LL_OK ? "OK" : "FALSE");

    return result;
}
	
MMP_UINT8
LB8K_HWECC_WriteDouble(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pBuffer0,
    MMP_UINT8* pBuffer1)
{
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT32 realAddress = 0x00;
#ifdef __FREERTOS__
    MMP_UINT8* pAlignBuf0  = (MMP_UINT8*)(((MMP_UINT32)pBuffer0 + 3) & ~3);
    MMP_UINT8* pAlignBuf1  = (MMP_UINT8*)(((MMP_UINT32)pBuffer1 + 3) & ~3);
#endif
#ifdef SHOW_RWE_MSG
    printf("W2[%x,%x]\r\n",pba,ppo);
    //printf("W1[%x,%x,%x,%x]\r\n",pba,ppo,pDataBuffer,pSpareBuffer);
#endif
    NAND_DbgPrintf("[ENTER]LB8K_HWECC_WriteDouble: pba(0x%X), ppo(0x%X)\n", pba, ppo);
    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();

    if(IsEnHwScrambler)
    {
        AHB_WriteRegister(NF_REG_WR_SCRAMB_INIT, 0x000000FF);
        //AHB_WriteRegister(NF_REG_WR_SCRAMB_INIT, ppo);//reset scrambler initial value
    }

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Calculate real address from input block address
    realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE; 

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)realAddress);

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__
    ithInvalidateDCacheRange(pBuffer0, PAGE_SIZE/2);    
    if ( pAlignBuf0 != pBuffer0 )
    {
        memcpy(g_pNFdataBuf, pBuffer0, PAGE_SIZE/2);
    }
    else
    {
        //NF_DmaCopy(g_pNFdataBuf, pBuffer0, PAGE_SIZE/2);
        memcpy(g_pNFdataBuf, pBuffer0, PAGE_SIZE/2);
    }
    ithInvalidateDCacheRange(pBuffer1, PAGE_SIZE/2 + SPARE_SIZE);
    if ( pAlignBuf1 != pBuffer1 )
    {        
        memcpy(g_pNFdataBuf + PAGE_SIZE/2, pBuffer1, PAGE_SIZE/2);
    	memcpy(g_pNFspareBuf, pBuffer1 + PAGE_SIZE/2, SPARE_SIZE);    	
    }
    else
    {
        memcpy(g_pNFdataBuf + PAGE_SIZE/2, pBuffer1, PAGE_SIZE/2);
    	memcpy(g_pNFspareBuf, pBuffer1 + PAGE_SIZE/2, SPARE_SIZE);
        //NF_DmaCopy(g_pNFdataBuf + PAGE_SIZE/2, pBuffer1, PAGE_SIZE/2);
    	//NF_DmaCopy(g_pNFspareBuf, pBuffer1 + PAGE_SIZE/2, SPARE_SIZE);
    }
    
    #if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
    {
		ST_SPARE* pSpare = (ST_SPARE*)(g_pNFspareBuf);
		
        pSpare->dummy1 = WRITE_SYMBOL;
        pSpare->dummy2 = 0xFFFFFFFF;
    }    
    #endif
    
    ithInvalidateDCacheRange(g_pNFdataBuf, PAGE_SIZE);
    ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);
    
    // --- Set source data address at VRAM
    AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
    AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
    AHB_WriteRegister(NF_REG_MEM_DST_CORR_ECC_ADDR, (MMP_UINT32)g_pNFeccBuf); 

#elif defined(WIN32)

    #if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
    if(SPARE_SIZE==0x18)
	{
		ST_SPARE* pSpare = (ST_SPARE*)(pBuffer1 + PAGE_SIZE/2);
	    pSpare->dummy1 = WRITE_SYMBOL;
        pSpare->dummy2 = 0xFFFFFFFF;
	}
	#endif
    HOST_WriteBlockMemory((MMP_UINT32)g_pNFdataBuf, (MMP_UINT32) pBuffer0, PAGE_SIZE/2);
    HOST_WriteBlockMemory((MMP_UINT32)g_pNFdataBuf + PAGE_SIZE/2, (MMP_UINT32)pBuffer1, PAGE_SIZE/2);
    HOST_WriteBlockMemory((MMP_UINT32)g_pNFspareBuf, (MMP_UINT32)(pBuffer1 + PAGE_SIZE/2), SPARE_SIZE);

    AHB_WriteRegister(NF_REG_WRITE_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
    AHB_WriteRegister(NF_REG_WRITE_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
    AHB_WriteRegister(NF_REG_MEM_DST_CORR_ECC_ADDR, (MMP_UINT32)g_pNFeccBuf);    
#endif

    // ----------------------------------------
    // 5. Disable write protect bit
    //HOST_WriteRegisterMask(NF_REG_GENERAL,(MMP_UINT16)0x0800, 0x0800); // 0x131A
    //HOST_WriteRegisterMask(NF_REG_STORAGE_ADDR3, (MMP_UINT16)0x0600, 0x1F00); // 0x1318
    AHB_WriteRegisterMask(NF_REG_GENERAL,0x00020000, 0x00020000); // 0x131C
    AHB_WriteRegisterMask(NF_REG_STORAGE_ADDR2, (MMP_UINT32)(SPARE_SIZE)<<8, 0x00001F00); // 0x1318

    // ----------------------------------------
    // 6. Fire HW and wait ready
    AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
    AHB_WriteRegister(NF_REG_CMD1, PROGRAM_CMD_1ST);
    AHB_WriteRegister(NF_REG_CMD2, PROGRAM_CMD_2ND);
    
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);	//NAND write command fire
    
    #if defined(NAND_IRQ_ENABLE)
    #else
    MMP_Sleep(1);
    #endif

    result = LB8K_HWECC_WaitCmdReady(NF_WRITE);

    // ----------------------------------------
    // 7. Enable write protect bit and disable CE pin
    //HOST_WriteRegisterMask(NF_REG_GENERAL,(MMP_UINT16)0x0000,0x0800); // 0x1316

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_WriteDouble: return %s\n", result == LL_OK ? "OK" : "FALSE");

    return result;
}

MMP_UINT8
LB8K_HWECC_Read(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pBuffer)
{
    HWECC_RESP_TYPE eccResult;
    MMP_UINT8       result      = LL_ERROR;
    MMP_UINT32      realAddress = 0x00;
	MMP_BOOL        bIserased   = MMP_TRUE;
#ifdef __FREERTOS__
    MMP_UINT8*      pAlignBuf   = (MMP_UINT8*)(((MMP_UINT32)pBuffer + 3) & ~3);
#endif

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_Read: pba(0x%X), ppo(0x%X), realAddress = 0x%08x\n",
		pba, ppo, (((pba * g_NfAttrib.pagePerBlock)+ ppo)* PAGE_SIZE));

#ifdef SHOW_RWE_MSG
    printf("R1[%x,%x]\n",pba,ppo);
#endif
		
    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();

    {
        MMP_UINT32 Reg32;

        //AHB_WriteRegisterMask(NF_REG_CMD_FIRE, NF_CMD_SW_RESET, NF_CMD_SW_RESET);
        if(IsEnHwScrambler)
        {
            AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, 0x000000FF);
            //AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, ppo);//reset scrambler initial value
        }
    }

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Calculate real address from input block address
    realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE; 

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)(realAddress) );

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__
    if ( pAlignBuf != pBuffer )
    {
        // --- Set source data address at VRAM
        AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
        
        // --- Set destination spare address
        AHB_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
    }
    else
    {
        // --- Set source data address at VRAM
        AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)pBuffer);

        // --- Set destination spare address
        AHB_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR, (MMP_UINT32)(pBuffer + PAGE_SIZE));        
    }
#elif defined(WIN32)
    //
    // Do nothing in WIN32, because we won't change the address register!
    //
#endif
    {
        //MMP_UINT8  TempBuff[PAGE_SIZE];
        //MMP_UINT32  m;
/*
        for(m=0;m<PAGE_SIZE;m++)
        {
            TempBuff[m]=0xAA;
        }
        HOST_WriteBlockMemory((MMP_UINT32)g_pNFdataBuf, (MMP_UINT32)TempBuff, PAGE_SIZE);
        HOST_WriteBlockMemory((MMP_UINT32)g_pNFspareBuf, (MMP_UINT32)TempBuff, SPARE_SIZE);
*/
/*
        for(m=0;m<PAGE_SIZE;m++)
        {
            TempBuff[m]=0x55;
        }
        HOST_ReadBlockMemory((MMP_UINT32)TempBuff, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE);

        for(m=0;m<PAGE_SIZE;m++)
        {
            TempBuff[m]=0x55;
        }
        HOST_ReadBlockMemory((MMP_UINT32)TempBuff, (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE);
        */
    }

    // ----------------------------------------

    // 5. Fire HW and wait ready    
    AHB_WriteRegister(NF_REG_CMD1, READ_CMD_1ST); 
    AHB_WriteRegister(NF_REG_CMD2, READ_CMD_2ND); 
    AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);//
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    
    #if defined(NAND_IRQ_ENABLE)
    #else
    MMP_Sleep(1);
    #endif

    result = LB8K_HWECC_WaitCmdReady(NF_READ);

    if ( result == LL_OK )
    {
        // ----------------------------------------
        // 6. Get data back
#ifdef __FREERTOS__
        if ( pAlignBuf != pBuffer )
        {
            ithInvalidateDCacheRange(g_pNFdataBuf,  PAGE_SIZE);
            ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);

            memcpy(pBuffer, g_pNFdataBuf, PAGE_SIZE);
            memcpy(pBuffer + PAGE_SIZE, g_pNFspareBuf, SPARE_SIZE);
            //NF_DmaCopy(pBuffer, g_pNFdataBuf, PAGE_SIZE);
            //NF_DmaCopy(pBuffer + PAGE_SIZE, g_pNFspareBuf, SPARE_SIZE);

            //ithInvalidateDCacheRange(pBuffer, DATA_SIZE);
        }
        else
        {
            ithInvalidateDCacheRange(pBuffer, DATA_SIZE);
        }
#elif defined(WIN32)
        HOST_ReadBlockMemory((MMP_UINT32)pBuffer, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE);
        HOST_ReadBlockMemory((MMP_UINT32)(pBuffer + PAGE_SIZE), (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE);
#endif
        // ----------------------------------------
        // 7. Check if a erased block
        bIserased = IsErasedBlock((MMP_UINT8*)pBuffer, (MMP_UINT8*)(pBuffer + PAGE_SIZE));

        // ----------------------------------------
        // 8. Check ECC
        eccResult = LB8K_HWECC_CheckECC((MMP_UINT8*)pBuffer, (MMP_UINT8*)(pBuffer+PAGE_SIZE), !bIserased);
        
        switch(eccResult)
        {
            case NF_NOECCEN:
            case NF_NOERROR:
                break;
                
            case NF_ECCCORPASS:
                printf("LB8K_HWECC_Read: pba(%u), ppo(%u), eccResult == NF_ECCCORPASS\n", pba, ppo);
                break;
                
            case NF_PARITYADDRERR:
                printf("LB8K_HWECC_Read: ECC Correction failed. eccResult == PARITYADDRERR\n");
                result = LL_ERROR;
                break;

            case NF_OVER4ERR:
                printf("LB8K_HWECC_Read: ECC Correction failed. eccResult == OVER4ERR\n");
                result = LL_ERROR;
                break;

            default:
                printf("LB8K_HWECC_Read: Unknown ECC value. eccResult = %d\n", eccResult);
                result = LL_ERROR;
                break;
        }

        if (   result    == LL_OK
            && bIserased == MMP_TRUE )
        {
            result = LL_ERASED;
        }
    }

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_Read: return %s\n", result == LL_OK ? "OK" :
                                                        result == LL_ERASED ? "ERASED" : "FALSE");

    return result;
}

MMP_UINT8
LB8K_HWECC_LBA_Read(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pBuffer)
{
    HWECC_RESP_TYPE eccResult;
    MMP_UINT8       result      = LL_ERROR;
    MMP_UINT32      realAddress = 0x00;
    MMP_BOOL        bIserased   = MMP_TRUE;
	MMP_UINT32		LbaPageSize = PAGE_SIZE+TOTAL_SPARE_SIZE;
#ifdef __FREERTOS__
    MMP_UINT8*      pAlignBuf   = (MMP_UINT8*)(((MMP_UINT32)pBuffer + 3) & ~3);
#endif

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_Read: pba(0x%X), ppo(0x%X), realAddress = 0x%08x\n",
        pba, ppo, (((pba * g_NfAttrib.pagePerBlock)+ ppo)* PAGE_SIZE));

#ifdef SHOW_RWE_MSG
    printf("Ru[%x,%x]\n",pba,ppo);
#endif

    // ----------------------------------------
    // 1. Switch controller to nand
    EnableNfControl();

    {
        MMP_UINT32 Reg32;

        AHB_WriteRegisterMask(NF_REG_CMD_FIRE, NF_CMD_SW_RESET, NF_CMD_SW_RESET);   //
        if(IsEnHwScrambler)
        {
            //it's scrambler bug, this register need to be reset after previous reading fire
            AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, 0x000000FF);
            //AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, ppo);//reset scrambler initial value
        }
        AHB_WriteRegister(NF_REG_USER_DEF_CTRL, 0x80000000|LbaPageSize );
    }

    // --- Calculate block position, and set block number
    if ( g_NfAttrib.bMultiDie )
    {
        MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
        NfChipSelect(dieNumber);
        pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
    }

    // ----------------------------------------
    // 2. Calculate real address from input block address
    realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE;

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)(realAddress) );

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__
    if ( pAlignBuf != pBuffer )
    {
        // --- Set source data address at VRAM
        AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
        //HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)(g_pNFdataBuf) & 0xffff));
        //HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)(g_pNFdataBuf) & 0xffff0000) >> 16));
        // --- Set destination spare address
        AHB_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
        //HOST_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)(g_pNFspareBuf) & 0xffff));
        //HOST_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)(g_pNFspareBuf) & 0xffff0000) >> 16));
    }
    else
    {
        // --- Set source data address at VRAM
        AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)pBuffer);
        //HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)pBuffer & 0xffff));
        //HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)pBuffer & 0xffff0000) >> 16));
        // --- Set destination spare address
        AHB_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR, (MMP_UINT32)(pBuffer + PAGE_SIZE));
        //HOST_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)(pBuffer + PAGE_SIZE) & 0xffff));
        //HOST_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)(pBuffer + PAGE_SIZE) & 0xffff0000) >> 16));
    }
#elif defined(WIN32)
    //
    // Do nothing in WIN32, because we won't change the address register!
    //
#endif
    {
        //MMP_UINT8  TempBuff[PAGE_SIZE];
        //MMP_UINT32  m;
        /*
        for(m=0;m<PAGE_SIZE;m++)
        {
        TempBuff[m]=0xAA;
        }
        HOST_WriteBlockMemory((MMP_UINT32)g_pNFdataBuf, (MMP_UINT32)TempBuff, PAGE_SIZE);
        HOST_WriteBlockMemory((MMP_UINT32)g_pNFspareBuf, (MMP_UINT32)TempBuff, SPARE_SIZE);
        */
        /*
        for(m=0;m<PAGE_SIZE;m++)
        {
        TempBuff[m]=0x55;
        }
        HOST_ReadBlockMemory((MMP_UINT32)TempBuff, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE);

        for(m=0;m<PAGE_SIZE;m++)
        {
        TempBuff[m]=0x55;
        }
        HOST_ReadBlockMemory((MMP_UINT32)TempBuff, (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE);
        */
    }

    // ----------------------------------------

    // 5. Fire HW and wait ready    
    AHB_WriteRegister(NF_REG_CMD1, READ_CMD_1ST); 
    AHB_WriteRegister(NF_REG_CMD2, READ_CMD_2ND); 
    AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    
    #if defined(NAND_IRQ_ENABLE)
    #else
    MMP_Sleep(1);
    #endif

    //HOST_WriteRegister(NF_REG_CMD1,READ_CMD_1ST); 
    //HOST_WriteRegister(NF_REG_CMD2,READ_CMD_2ND);

    result = LB8K_HWECC_WaitCmdReady(NF_READ);
	AHB_WriteRegister(NF_REG_USER_DEF_CTRL, 0x00000000 );

    if ( result == LL_OK )
    {
        // ----------------------------------------
        // 6. Get data back
#ifdef __FREERTOS__
        if ( pAlignBuf != pBuffer )
        {
            ithInvalidateDCacheRange(g_pNFdataBuf,  PAGE_SIZE);
            ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);

            memcpy(pBuffer, g_pNFdataBuf, PAGE_SIZE);
            memcpy(pBuffer + PAGE_SIZE, g_pNFspareBuf, SPARE_SIZE);
            //NF_DmaCopy(pBuffer, g_pNFdataBuf, PAGE_SIZE);
            //NF_DmaCopy(pBuffer + PAGE_SIZE, g_pNFspareBuf, SPARE_SIZE);

            ithInvalidateDCacheRange(pBuffer, DATA_SIZE);
        }
        else
        {
            ithInvalidateDCacheRange(pBuffer, DATA_SIZE);
        }
#elif defined(WIN32)
        HOST_ReadBlockMemory((MMP_UINT32)pBuffer, (MMP_UINT32)g_pNFdataBuf, LbaPageSize);
		//HOST_ReadBlockMemory((MMP_UINT32)pBuffer, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE);
        //HOST_ReadBlockMemory((MMP_UINT32)(pBuffer + PAGE_SIZE), (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE);
#endif
        // ----------------------------------------
        // 7. Check if a erased block
        //bIserased = IsErasedBlock((MMP_UINT8*)pBuffer, (MMP_UINT8*)(pBuffer + PAGE_SIZE));
		bIserased = IsErasedBlock( (MMP_UINT8*)pBuffer, (MMP_UINT8*)(pBuffer + PAGE_SIZE + (ECC_LENGTH*(PAGE_SIZE/ECC_PAGE_SIZE))) );

        // ----------------------------------------
        // 8. Check ECC
        //eccResult = LB8K_HWECC_CheckECC(pBuffer, pBuffer+PAGE_SIZE, !bIserased);
		eccResult = NF_NOERROR;//not finished yet
        switch(eccResult)
        {
        case NF_NOECCEN:
        case NF_NOERROR:
            break;

        case NF_ECCCORPASS:
            printf("LB8K_HWECC_LBA_Read: pba(%u), ppo(%u), eccResult == NF_ECCCORPASS\n", pba, ppo);
            break;

        case NF_PARITYADDRERR:
            printf("LB8K_HWECC_LBA_Read: ECC Correction failed. eccResult == PARITYADDRERR\n");
            result = LL_ERROR;
            break;

        case NF_OVER4ERR:
            printf("LB8K_HWECC_LBA_Read: ECC Correction failed. eccResult == OVER4ERR\n");
            result = LL_ERROR;
            break;

        default:
            printf("LB8K_HWECC_LBA_Read: Unknown ECC value. eccResult = %d\n", eccResult);
            result = LL_ERROR;
            break;
        }

        if (   result    == LL_OK
            && bIserased == MMP_TRUE )
        {
            result = LL_ERASED;
        }
    }

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_Read: return %s\n", result == LL_OK ? "OK" :
        result == LL_ERASED ? "ERASED" : "FALSE");

    return result;
}

MMP_UINT8
LB8K_HWECC_ReadPart(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pBuffer,
    MMP_UINT8  index)
{
    HWECC_RESP_TYPE eccResult;
    MMP_UINT32      Reg32;
    MMP_UINT8       IsEnScrambler = 0;
    MMP_UINT8       result        = LL_ERROR;
    MMP_UINT32      realAddress   = 0x00;
	MMP_BOOL        bIserased     = MMP_TRUE;
    MMP_UINT8*      pPagePointer  = MMP_NULL;
    MMP_UINT8*      pSparePointer = MMP_NULL;
#ifdef __FREERTOS__
	MMP_UINT8*      pPagePt  	  = MMP_NULL;
#elif defined(WIN32)
	MMP_UINT8       tempBuf[DATA_SIZE];
#endif

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_ReadPart: pba(0x%X), ppo(0x%X), index(0x%X), realAddress = 0x%08x\n",
		pba, ppo, index, (((pba * g_NfAttrib.pagePerBlock)+ ppo)* PAGE_SIZE));

#ifdef SHOW_RWE_MSG
    printf("R2[%x,%x,%x]\n",pba,ppo,index);
#endif

    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();  

    //if( (pba==1) && (ppo==0x00) && (index==LL_RP_1STHALF) )
    {
        //AHB_WriteRegisterMask(NF_REG_CMD_FIRE, NF_CMD_SW_RESET, NF_CMD_SW_RESET);
        if(IsEnHwScrambler)
        {
            AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, 0x000000FF);//reset scrambler initial value
            //AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, ppo);//reset scrambler initial value
        }
    }

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Calculate real address from input block address
    #ifdef	PITCH_ECC_1KB_ISSUE
    realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE;
    #else
	switch(index)
	{
		case LL_RP_1STHALF:
		case LL_RP_DATA:
			realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE;
			break;
		case LL_RP_2NDHALF:
			realAddress = (((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE) + ((ECC_PAGE_SIZE+ECC_LENGTH)*((PAGE_SIZE/ECC_PAGE_SIZE)/2)); 
			break;
		case LL_RP_SPARE:
			realAddress = (((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE) + ((ECC_PAGE_SIZE+ECC_LENGTH)*((PAGE_SIZE/ECC_PAGE_SIZE)-1)); 
			break;
        default:
            printf("LB8K_HWECC_ReadPart: Unknown ECC value. eccResult = %d\n", eccResult);
            result = LL_ERROR;
            break;
	}
	#endif

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)(realAddress) );

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__	
    // --- Set source data address at VRAM        
    if( (index==LL_RP_DATA) && (((MMP_UINT32)pBuffer&0x03)==0) )
    {
        // enhance read performance. joseph 2010.02.26
        //printf("R2.2:set pBuffer=%x\n",pBuffer);
        AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)pBuffer);
		}
		else
		{	
				//printf("R2.2:set g_pNFdataBuf=%x\n",g_pNFdataBuf);
				AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
		}
    // --- Set destination spare address
    AHB_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
#elif defined(WIN32)
    //
    // Do nothing in WIN32, because we won't change the address register!
    //
#endif

    // ----------------------------------------
    // 5. Fire HW and wait ready
    //if( IsEnHwScrambler || ((index==LL_RP_DATA) || (index==LL_RP_SPARE)) )
    #ifdef	PITCH_ECC_1KB_ISSUE
    g_pNFeccCurrRdMask = g_BchMapMask;
    AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
    #else
		if( IsEnHwScrambler || (index==LL_RP_DATA) )
    {
    		//printf("R2.2:set len1=%x\n",((PAGE_SIZE/ECC_PAGE_SIZE)-1) );
    		g_pNFeccCurrRdMask = g_BchMapMask;
        AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
    }
		else if( (index==LL_RP_1STHALF) || (index==LL_RP_2NDHALF) )
		{
				//printf("R2.2:set len2=%x\n",(((PAGE_SIZE/ECC_PAGE_SIZE)/2)-1) );
				if(index==LL_RP_1STHALF)
				{
				    g_pNFeccCurrRdMask = 0x0000000F;
				}
				else
				{
				    g_pNFeccCurrRdMask = 0x000000F0;
				}
        AHB_WriteRegisterMask(NF_REG_GENERAL, (((PAGE_SIZE/ECC_PAGE_SIZE)/2)-1)<<20, 0x01F00000);
		}
    else if(index==LL_RP_SPARE)
    {
    		//printf("R2.2:set len3=%x\n",0 );
    		g_pNFeccCurrRdMask = (1<<((PAGE_SIZE/ECC_PAGE_SIZE)-1));
        AHB_WriteRegisterMask(NF_REG_GENERAL, 0x00000000, 0x01F00000);
    }
    #endif

    AHB_WriteRegister(NF_REG_CMD1, READ_CMD_1ST);
    AHB_WriteRegister(NF_REG_CMD2, READ_CMD_2ND);
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    
    #if defined(NAND_IRQ_ENABLE)
    #else
    MMP_Sleep(1);
    #endif

    result = LB8K_HWECC_WaitCmdReady(NF_READ);
    if ( result == LL_OK )
    {
        // ----------------------------------------
        // 6. Get data back
#if defined(__FREERTOS__)
		if( (index==LL_RP_DATA) && (((MMP_UINT32)pBuffer&0x03)==0) )
		{
            // enhance read performance. joseph 2010.02.26
        	ithInvalidateDCacheRange(pBuffer,  PAGE_SIZE);
		}
		else
		{
            #ifdef	PITCH_ECC_1KB_ISSUE
            ithInvalidateDCacheRange(g_pNFdataBuf,  PAGE_SIZE);   
            #else  
            ithInvalidateDCacheRange(g_pNFdataBuf,  (PAGE_SIZE/2)); 
            #endif
		}
		
		ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);
		
		if( (index==LL_RP_DATA) && (((MMP_UINT32)pBuffer&0x03)==0) )
		{
			// enhance read performance. joseph 2010.02.26
        	pPagePointer  = pBuffer;
        }
        else
        {
        	pPagePointer  = g_pNFdataBuf;
        }
        pSparePointer = g_pNFspareBuf;
#elif defined(WIN32)
		if( IsEnHwScrambler || (index==LL_RP_DATA) )
		{
			HOST_ReadBlockMemory((MMP_UINT32)tempBuf, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE);
			HOST_ReadBlockMemory((MMP_UINT32)tempBuf+PAGE_SIZE, (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE);
		}
		else if( index==LL_RP_1STHALF )
		{
			HOST_ReadBlockMemory((MMP_UINT32)tempBuf, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE/2);
		}
		else if(index==LL_RP_2NDHALF)
		{
			HOST_ReadBlockMemory((MMP_UINT32)tempBuf, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE/2);
			HOST_ReadBlockMemory((MMP_UINT32)tempBuf+PAGE_SIZE, (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE);
		}
		else
		{
			HOST_ReadBlockMemory((MMP_UINT32)tempBuf, (MMP_UINT32)g_pNFdataBuf, PAGE_SIZE/4);
			HOST_ReadBlockMemory((MMP_UINT32)tempBuf+PAGE_SIZE, (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE); 
		}

        pPagePointer  = tempBuf;
        pSparePointer = tempBuf+PAGE_SIZE;
#endif

        // ----------------------------------------
        // 7. Check if a erased block
        #ifdef	PITCH_ECC_1KB_ISSUE
        bIserased = IsErasedBlock((MMP_UINT8*)pPagePointer, (MMP_UINT8*)pSparePointer);
        #else
        switch(index)
        {
            case LL_RP_1STHALF:
                //bIserased = IsErasedBlock((MMP_UINT8*)pPagePointer, (MMP_UINT8*)pPagePointer);
                bIserased = IsAllData0xFF((MMP_UINT8*)pPagePointer, 16);
                break;
            case LL_RP_2NDHALF:
            case LL_RP_DATA:
            case LL_RP_SPARE:
            default:
                bIserased = IsErasedBlock((MMP_UINT8*)pPagePointer, (MMP_UINT8*)pSparePointer);
                break;
        }
        #endif

        // ----------------------------------------
        // 8. Check ECC
        eccResult = LB8K_HWECC_CheckECC((MMP_UINT8*)pPagePointer, (MMP_UINT8*)pSparePointer, !bIserased);
        switch(eccResult)
        {
            case NF_NOECCEN:
            case NF_NOERROR:
                break;
                
            case NF_ECCCORPASS:
                printf("LB8K_HWECC_ReadPart: pba(%u), ppo(%u), eccResult == NF_ECCCORPASS\n", pba, ppo);
                break;
                
            case NF_PARITYADDRERR:
                printf("LB8K_HWECC_ReadPart: ECC Correction failed. eccResult == PARITYADDRERR\n");
                result = LL_ERROR;
                break;

            case NF_OVER4ERR:
                printf("LB8K_HWECC_ReadPart: ECC Correction failed. eccResult == OVER4ERR\n");
                result = LL_ERROR;
                break;

            default:
                printf("LB8K_HWECC_ReadPart: Unknown ECC value. eccResult = %d\n", eccResult);
                result = LL_ERROR;
                break;
        }

        // ----------------------------------------
        // 9. Copy data back to FTL
        if ( result == LL_OK )
        {
            switch(index)
            {
            case LL_RP_1STHALF: 
#if defined(__FREERTOS__)
								if(pBuffer!=pPagePointer)
								{
                	memcpy(pBuffer, pPagePointer, PAGE_SIZE/2);
                	//NF_DmaCopy(pBuffer, pPagePointer, PAGE_SIZE/2);
                	ithInvalidateDCacheRange(pBuffer, PAGE_SIZE/2); 
                }
                //printf("R2.2:mcp1[%x,%x,%x]\n",pBuffer,pPagePointer,PAGE_SIZE/2 );
#elif defined(WIN32)
                memcpy(pBuffer, pPagePointer, PAGE_SIZE/2);
#endif
                break;
                
            case LL_RP_2NDHALF:
#if defined(__FREERTOS__)
				if(pBuffer!=pPagePointer)
				{
					#ifdef	PITCH_ECC_1KB_ISSUE
                	memcpy(pBuffer, pPagePointer+(PAGE_SIZE/2), PAGE_SIZE/2);
                	#else
                	memcpy(pBuffer, pPagePointer, PAGE_SIZE/2);
                	#endif
                	//NF_DmaCopy(pBuffer, pPagePointer+(PAGE_SIZE/2), PAGE_SIZE/2);
                	ithInvalidateDCacheRange(pBuffer, PAGE_SIZE/2);
                }
                //printf("R2.2:mcp2[%x,%x,%x]\n",pBuffer,pPagePointer+(PAGE_SIZE/2),PAGE_SIZE/2 );
#elif defined(WIN32)
                if(IsEnHwScrambler)
                {
                    memcpy(pBuffer, pPagePointer+(PAGE_SIZE/2), PAGE_SIZE/2);
                }
                else
                {
                    memcpy(pBuffer, pPagePointer, PAGE_SIZE/2);
                }
#endif
                break;

            case LL_RP_DATA:
#if defined(__FREERTOS__)
				if((MMP_UINT32)pBuffer&0x03)
				{
                    // enhance read performance. joseph 2010.02.26
                	memcpy(pBuffer, pPagePointer, PAGE_SIZE);
                	//NF_DmaCopy(pBuffer, pPagePointer, PAGE_SIZE);
            	}                	
                ithInvalidateDCacheRange(pBuffer, PAGE_SIZE);
                //printf("R2.2:mcp3[%x,%x,%x]%x\n",pBuffer,pPagePointer,PAGE_SIZE );
#elif defined(WIN32)
                memcpy(pBuffer, pPagePointer, PAGE_SIZE);
#endif
                break;
                
            case LL_RP_SPARE:
#if defined(__FREERTOS__)
                memcpy(pBuffer, pSparePointer, SPARE_SIZE);
                //NF_DmaCopy(pBuffer, pSparePointer, SPARE_SIZE);
                ithInvalidateDCacheRange(pBuffer, SPARE_SIZE);
#elif defined(WIN32)
                memcpy(pBuffer, pSparePointer, SPARE_SIZE);
                //memcpy(pBuffer, pPagePointer+1024, SPARE_SIZE);
#endif
                break;
                
            default: 
                result = LL_ERROR;
                break;
            }
        }

        if (   result    == LL_OK 
            && bIserased == MMP_TRUE )
        {
            result = LL_ERASED;
        }
    }
    g_pNFeccCurrRdMask = g_BchMapMask;

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_ReadPart: return %s\n", result == LL_OK ? "OK" :
                                                            result == LL_ERASED ? "ERASED" : "FALSE");
    return result;
}

MMP_UINT8
LB8K_HWECC_Read1kBytes(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pBuffer,
    MMP_UINT8  index)
{
    HWECC_RESP_TYPE eccResult;
    MMP_UINT32      Reg32;
    MMP_UINT8       IsEnScrambler = 0;
    MMP_UINT8       result        = LL_ERROR;
    MMP_UINT32      realAddress   = 0x00;
	MMP_BOOL        bIserased     = MMP_TRUE;
    MMP_UINT8*      pPagePointer  = MMP_NULL;
    MMP_UINT8*      pSparePointer = MMP_NULL;
#ifdef __FREERTOS__
	MMP_UINT8*      pPagePt  	  = MMP_NULL;
#elif defined(WIN32)
	MMP_UINT8       tempBuf[DATA_SIZE];
#endif

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_Read1kB: pba(0x%X), ppo(0x%X), index(0x%X), realAddress = 0x%08x\n",
		pba, ppo, index, (((pba * g_NfAttrib.pagePerBlock)+ ppo)* PAGE_SIZE));

#ifdef SHOW_RWE_MSG
    printf("R4[%x,%x,%x]\n",pba,ppo,index);
#endif

    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();  

    //if( (pba==1) && (ppo==0x00) && (index==1) )
    {
        //AHB_WriteRegisterMask(NF_REG_CMD_FIRE, NF_CMD_SW_RESET, NF_CMD_SW_RESET);
        if(IsEnHwScrambler)
        {
			if(index==1)
			{
				AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, 0x000000FF);//reset scrambler initial value
			}
        }
    }

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Calculate real address from input block address
	realAddress = (((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE) + ((ECC_PAGE_SIZE+ECC_LENGTH)*(index-1));

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)(realAddress) );

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__	
    // --- Set source data address at VRAM        
    if( (index==LL_RP_DATA) && (((MMP_UINT32)pBuffer&0x03)==0) )
    {
        // enhance read performance. joseph 2010.02.26
        AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)pBuffer);
    	//HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)(pBuffer) & 0xffff));
    	//HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)(pBuffer) & 0xffff0000) >> 16));
	}
	else
	{	
			AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);
    	//HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)(g_pNFdataBuf) & 0xffff));
    	//HOST_WriteRegister(NF_REG_READ_DATA_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)(g_pNFdataBuf) & 0xffff0000) >> 16));
	}
    // --- Set destination spare address
    AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);
    //HOST_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR1, (MMP_UINT16)((MMP_UINT32)(g_pNFspareBuf) & 0xffff));
    //HOST_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR2, (MMP_UINT16)(((MMP_UINT32)(g_pNFspareBuf) & 0xffff0000) >> 16));
#elif defined(WIN32)
    //
    // Do nothing in WIN32, because we won't change the address register!
    //
#endif

    // ----------------------------------------
    // 5. Fire HW and wait ready
    //if( IsEnHwScrambler || ((index==LL_RP_DATA) || (index==LL_RP_SPARE)) )
    AHB_WriteRegisterMask(NF_REG_GENERAL, 0x00000000, 0x01F00000);

    AHB_WriteRegister(NF_REG_CMD1, READ_CMD_1ST); 
    AHB_WriteRegister(NF_REG_CMD2, READ_CMD_2ND); 
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    
    #if defined(NAND_IRQ_ENABLE)
    #else
    MMP_Sleep(1);
    #endif

    result = LB8K_HWECC_WaitCmdReady(NF_READ);
    if ( result == LL_OK )
    {
        // ----------------------------------------
        // 6. Get data back
#if defined(__FREERTOS__)
		if( (index==LL_RP_DATA) && (((MMP_UINT32)pBuffer&0x03)==0) )
		{
            // enhance read performance. joseph 2010.02.26
        	ithInvalidateDCacheRange(pBuffer,  PAGE_SIZE);
		}
		else
		{
        	ithInvalidateDCacheRange(g_pNFdataBuf,  PAGE_SIZE);        				
		}
		
		ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);
		
		if( (index==LL_RP_DATA) && (((MMP_UINT32)pBuffer&0x03)==0) )
		{
			// enhance read performance. joseph 2010.02.26
        	pPagePointer  = pBuffer;
        }
        else
        {
        	pPagePointer  = g_pNFdataBuf;
        }
        pSparePointer = g_pNFspareBuf;
#elif defined(WIN32)
		HOST_ReadBlockMemory((MMP_UINT32)tempBuf, (MMP_UINT32)g_pNFdataBuf, ECC_PAGE_SIZE);

        pPagePointer  = tempBuf;
        //pSparePointer = tempBuf+PAGE_SIZE;
#endif

        // ----------------------------------------
        // 7. Check if a erased block
        bIserased = IsAllData0xFF(pPagePointer, 16);

        // ----------------------------------------
        // 8. Check ECC
        eccResult = LB8K_HWECC_CheckECC(pPagePointer, pSparePointer, !bIserased);
        switch(eccResult)
        {
            case NF_NOECCEN:
            case NF_NOERROR:
                break;
                
            case NF_ECCCORPASS:
                printf("LB8K_HWECC_ReadPart: pba(%u), ppo(%u), eccResult == NF_ECCCORPASS\n", pba, ppo);
                break;
                
            case NF_PARITYADDRERR:
                printf("LB8K_HWECC_ReadPart: ECC Correction failed. eccResult == PARITYADDRERR\n");
                result = LL_ERROR;
                break;

            case NF_OVER4ERR:
                printf("LB8K_HWECC_ReadPart: ECC Correction failed. eccResult == OVER4ERR\n");
                result = LL_ERROR;
                break;

            default:
                printf("LB8K_HWECC_ReadPart: Unknown ECC value. eccResult = %d\n", eccResult);
                result = LL_ERROR;
                break;
        }

        // ----------------------------------------
        // 9. Copy data back to FTL
        if ( result == LL_OK )
        {
			memcpy(pBuffer, pPagePointer, ECC_PAGE_SIZE);
        }

        if (   result    == LL_OK 
            && bIserased == MMP_TRUE )
        {
            result = LL_ERASED;
        }
    }

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_ReadPart: return %s\n", result == LL_OK ? "OK" :
                                                            result == LL_ERASED ? "ERASED" : "FALSE");
    return result;


}

MMP_UINT8
LB8K_HWECC_IsBadBlock(
    MMP_UINT32 pba)
{
    MMP_UINT8  result    = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 firstPage = 0;
    MMP_UINT32 lastPage  = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8  dataBuffer[DATA_SIZE+SPARE_SIZE];

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_IsBadBlock: pba(%d), realAddress = 0x%08x\n",
		pba, ((pba * g_NfAttrib.pagePerBlock)* PAGE_SIZE));
    #ifdef SHOW_RWE_MSG
	printf("IBB[%x,%x]\n",pba,dataBuffer);
    #endif
    // ----------------------------------------
    // 0. PRECONDITION
    if ( pba >= g_NfAttrib.numOfBlocks )
    {
        printf("\tLB8K_HWECC_IsBadBlock: ERROR! pba(%u) >= g_NfAttrib.numOfBlocks(%u)\n", pba, g_NfAttrib.numOfBlocks);
        return LL_ERROR;
    }
    
    // 1. Switch controller to nand
	EnableNfControl();

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

#ifdef NF_NAND_BOOT
    // Reserve boot section
    // Reserve 2M in large block
    ReservedBlockNum = 8 * 1024 * 1024 / (g_NfAttrib.pagePerBlock * g_NfAttrib.pageSize);
    if ( (pba >= 0) && (pba < ReservedBlockNum) )	
    	return LL_ERROR;					
#elif defined(NF_USE_OLD_VERSION)
    // Reserve boot section
    // Reserve 8M in small block
    ReservedBlockNum = 8 * 1024 * 1024 / (g_NfAttrib.pagePerBlock * g_NfAttrib.pageSize);
    if ( (pba >= 0) && (pba < ReservedBlockNum) )
        return LL_ERROR;
#endif


    // ----------------------------------------
    // 2. Read first page
    {
    	MMP_UINT8	bIibResult;
        ST_SPARE* pSpare = MMP_NULL;
        
        bIibResult = LB8K_HWECC_Read(pba, 0, dataBuffer);
        pSpare = (ST_SPARE*)(dataBuffer + PAGE_SIZE);
        
        //printf("Spr->dummy1=%08x,[%08x]\r\n",pSpare->dummy1,WRITE_SYMBOL);

        #if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
        if ( (SPARE_SIZE==0x18) && (pSpare->dummy1 == WRITE_SYMBOL) )
        {
            blockStatus = LL_OK;
        }
        else
        #endif
        {
        	MMP_UINT8	IfDataEqual00FF=LL_OK;
        	MMP_UINT32	*PtCmprData0,i;
        	
        	if( g_NfAttrib.pagePerBlock==0x80 )
        	{
        		PtCmprData0=(MMP_UINT32	*)dataBuffer;
        		//try to find out the MLC flash that has error data at page0.(ex:writed by other NAND controller)
                if(IsEnHwScrambler)
                {
                    if(IsAllData0xFF(dataBuffer, 16)==MMP_FALSE)
                    {
                        printf("this NAND flash does NOT match our NAND DATA structure[1]\r\n");
                        IfDataEqual00FF = LL_ERROR;
                    }
                    if(IsErasedBlock((MMP_UINT8*)dataBuffer, (MMP_UINT8*)pSpare)==MMP_FALSE)
                    {
                        printf("this NAND flash does NOT match our NAND DATA structure[2]\r\n");
                        IfDataEqual00FF = LL_ERROR;
                    }
                }
                else
                {
                    for(i=0;i<DATA_SIZE/4;i++)
                    {
                        if( (PtCmprData0[i]) && (PtCmprData0[i]!=0xFFFFFFFF) )
                        {
                            printf("this NAND flash does NOT match our NAND DATA structure[3][%x,%x]\n",i,PtCmprData0[i]);
                            {
                            	MMP_UINT32	k;
                            	for(k=0;k<1024;k++)
                            	{
                            		printf("%02x ",dataBuffer[k]);
                            		if( (k&0x1FF)==0x1FF )	printf("\n[%x]",k+1);
                            		if( (k&0x0F)==0x0F )	printf("\n");
                            	}                            	
                            }
                            
                            IfDataEqual00FF = LL_ERROR;
                            break;
                        }
                    }
                }
        	}
        	
        	if( IfDataEqual00FF == LL_OK )
        	{
            	if ( g_NfAttrib.pfCheckMethod == MMP_NULL )
            	{
            	    printf("LB8K_HWECC_IsBadBlock: ERROR! g_NfAttrib.pfCheckMethod == MMP_NULL\n");
            	    blockStatus = LL_ERROR;
            	}
            	else
            	{
            	    blockStatus = g_NfAttrib.pfCheckMethod(pba);
            	}
            }
        }
    }

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_IsBadBlock: pba(%d), %s\n", pba, blockStatus == LL_OK ? "Normal Block" : "Bad Block");

    return blockStatus;
}

MMP_UINT8
LB8K_HWECC_IsBadBlockForBoot(
    MMP_UINT32 pba)
{
    MMP_UINT8  result    = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 firstPage = 0;
    MMP_UINT32 lastPage  = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8  dataBuffer[DATA_SIZE];

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_IsBadBlock: pba(%d), realAddress = 0x%08x\n",
		pba, ((pba * g_NfAttrib.pagePerBlock)* PAGE_SIZE));
    // ----------------------------------------
    // 0. PRECONDITION
    if ( pba >= g_NfAttrib.numOfBlocks )
    {
        printf("\tLB8K_HWECC_IsBadBlock: ERROR! pba(%u) >= g_NfAttrib.numOfBlocks(%u)\n", pba, g_NfAttrib.numOfBlocks);
        return LL_ERROR;
    }
    
    // 1. Switch controller to nand
	EnableNfControl();

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Read first page
    {
    	MMP_UINT8	bIibResult;
        ST_SPARE* pSpare = MMP_NULL;
        
        bIibResult = LB8K_HWECC_Read(pba, 0, dataBuffer);
        pSpare = (ST_SPARE*)(dataBuffer + PAGE_SIZE);
        
        //printf("Spr->dummy1=%08x,[%08x]\r\n",pSpare->dummy1,WRITE_SYMBOL);

		#if defined(LARGEBLOCK_4KB) || defined(LARGEBLOCK_8KB)
        if ( (SPARE_SIZE==0x18) && (pSpare->dummy1 == WRITE_SYMBOL) )
        {
            blockStatus = LL_OK;
        }
        else
        #endif
        {
        	MMP_UINT8	IfDataEqual00FF=LL_OK;
        	MMP_UINT32	*PtCmprData0,i;
        	
        	if( g_NfAttrib.pagePerBlock==0x80 )
        	{
        		PtCmprData0=(MMP_UINT32	*)dataBuffer;
        		//try to find out the MLC flash that has error data at page0.(ex:writed by other NAND controller)
        	
        		for(i=0;i<DATA_SIZE/4;i++)
        		{
        			if( (PtCmprData0[i]) && (PtCmprData0[i]!=0xFFFFFFFF) )
        			{
        				printf("this NAND flash does NOT match our NAND DATA structure[0]\r\n");
        				IfDataEqual00FF = LL_ERROR;
        				break;
        			}
        		}
        	}
        	
        	if( IfDataEqual00FF == LL_OK )
        	{
            	if ( g_NfAttrib.pfCheckMethod == MMP_NULL )
            	{
            	    printf("LB8K_HWECC_IsBadBlock: ERROR! g_NfAttrib.pfCheckMethod == MMP_NULL\n");
            	    blockStatus = LL_ERROR;
            	}
            	else
            	{
            	    blockStatus = g_NfAttrib.pfCheckMethod(pba);
            	}
            }
        }
    }

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_IsBadBlock: pba(%d), %s\n", pba, blockStatus == LL_OK ? "Normal Block" : "Bad Block");

    return blockStatus;
}


MMP_UINT8
LB8K_HWECC_ReadOneByte(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  offsetPos,
    MMP_UINT8* pByte)
{
    HWECC_RESP_TYPE eccResult;
    MMP_UINT8       result        = LL_ERROR;
    MMP_UINT32      realAddress   = 0x00;
	MMP_BOOL        bIserased     = MMP_TRUE;
    MMP_UINT8*      pPagePointer  = MMP_NULL;
    MMP_UINT8*      pSparePointer = MMP_NULL;
    MMP_UINT8       dataBuffer[DATA_SIZE];

    NAND_DbgPrintf("[ENTER]LB8K_HWECC_ReadOneByte: pba(0x%X), ppo(0x%X), sparepos(0x%X), realAddress = 0x%08x\n",
		pba, ppo, offsetPos, (((pba * g_NfAttrib.pagePerBlock)+ ppo)* PAGE_SIZE));

#ifdef SHOW_RWE_MSG
    printf("R3[%x,%x,%x]\n",pba,ppo,offsetPos);
#endif
    // ----------------------------------------
    // 1. Switch controller to nand
	EnableNfControl();

    {
        //AHB_WriteRegisterMask(NF_REG_CMD_FIRE, NF_CMD_SW_RESET, NF_CMD_SW_RESET);
        if(IsEnHwScrambler)
        {
            AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, 0x000000FF);
            //AHB_WriteRegister(NF_REG_RD_SCRAMB_INIT, ppo);//reset scrambler initial value
        }
    }

	// --- Calculate block position, and set block number
	if ( g_NfAttrib.bMultiDie )
	{
		MMP_UINT32 dieNumber = pba / g_NfAttrib.blockPerDie;
		NfChipSelect(dieNumber);
		pba = pba - (g_NfAttrib.blockPerDie * dieNumber);
	}

    // ----------------------------------------
    // 2. Calculate real address from input block address
	if(IsEnHwScrambler)
	{
		realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE; 
	}
	else
	{
        #ifdef	PITCH_ECC_1KB_ISSUE
        realAddress = ((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE; 
        #else
		realAddress = (((pba * g_NfAttrib.pagePerBlock) + ppo) * PAGE_SIZE) + ((ECC_PAGE_SIZE+ECC_LENGTH)*((PAGE_SIZE/ECC_PAGE_SIZE)-1)); 
        #endif
	}

    // ----------------------------------------
    // 3. Set program block address at nandflash region
    AHB_WriteRegister(NF_REG_STORAGE_ADDR1, (MMP_UINT32)realAddress);

    // ----------------------------------------
    // 4. Set read/write address
#ifdef __FREERTOS__
    // --- Set source data address at VRAM
    AHB_WriteRegister(NF_REG_READ_DATA_BASE_ADDR, (MMP_UINT32)g_pNFdataBuf);

    // --- Set destination spare address
    AHB_WriteRegister(NF_REG_READ_SPARE_BASE_ADDR, (MMP_UINT32)g_pNFspareBuf);

#elif defined(WIN32)
    //
    // Do nothing in WIN32, because we won't change the address register!
    //
#endif

    // ----------------------------------------
    // 5. Fire HW and wait ready
	if(IsEnHwScrambler)
	{
	    g_pNFeccCurrRdMask = g_BchMapMask;
	    AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
	}
	else
	{
        #ifdef	PITCH_ECC_1KB_ISSUE
	    g_pNFeccCurrRdMask = g_BchMapMask;
	    AHB_WriteRegisterMask(NF_REG_GENERAL, ((PAGE_SIZE/ECC_PAGE_SIZE)-1)<<20, 0x01F00000);
        #else
	    g_pNFeccCurrRdMask = 0x00000001<<((PAGE_SIZE/ECC_PAGE_SIZE)-1);
        AHB_WriteRegisterMask(NF_REG_GENERAL, 0x00000000, 0x01F00000);
        #endif
	}
	
    AHB_WriteRegister(NF_REG_CMD1, READ_CMD_1ST); 
    AHB_WriteRegister(NF_REG_CMD2, READ_CMD_2ND); 
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    
    #if defined(NAND_IRQ_ENABLE)
    #else
    MMP_Sleep(1);
    #endif

    result = LB8K_HWECC_WaitCmdReady(NF_READ);
    if ( result == LL_OK )
    {
        // ----------------------------------------
        // 6. Get data back
#ifdef __FREERTOS__
        ithInvalidateDCacheRange(g_pNFdataBuf,  PAGE_SIZE);
        ithInvalidateDCacheRange(g_pNFspareBuf, SPARE_SIZE);

        pPagePointer  = g_pNFdataBuf;
        pSparePointer = g_pNFspareBuf;
#elif defined(WIN32)
        HOST_ReadBlockMemory((MMP_UINT32)(dataBuffer + PAGE_SIZE), (MMP_UINT32)g_pNFspareBuf, SPARE_SIZE);

        pPagePointer  = dataBuffer;
        pSparePointer = dataBuffer + PAGE_SIZE;
#endif

        // ----------------------------------------
        // 7. Check if a erased block
        bIserased = IsErasedBlock((MMP_UINT8*)pPagePointer, (MMP_UINT8*)pSparePointer);

        // ----------------------------------------
        // 8. Check ECC
        eccResult = LB8K_HWECC_CheckECC(pPagePointer, pSparePointer, !bIserased);
        switch(eccResult)
        {
            case NF_NOECCEN:
            case NF_NOERROR:
                break;
                
            case NF_ECCCORPASS:
                printf("LB8K_HWECC_Read 1B: pba(%u), ppo(%u), eccResult == NF_ECCCORPASS\n", pba, ppo);
                break;
                
            case NF_PARITYADDRERR:
                printf("LB8K_HWECC_Read 1B: ECC Correction failed. eccResult == PARITYADDRERR\n");
                result = LL_ERROR;
                break;

            case NF_OVER4ERR:
                printf("LB8K_HWECC_Read 1B: ECC Correction failed. eccResult == OVER4ERR\n");
                result = LL_ERROR;
                break;

            default:
                printf("LB8K_HWECC_Read 1B: Unknown ECC value. eccResult = %d\n", eccResult);
                result = LL_ERROR;
                break;
        }

        if ( result == LL_OK )
        {
            *pByte= *(pSparePointer + offsetPos);
        }
    }
    g_pNFeccCurrRdMask = g_BchMapMask;

    NAND_DbgPrintf("[LEAVE]LB8K_HWECC_ReadOneByte: *ch = 0x%X, result(%d)\n", *pByte, result);

    return result;
}

/****************************************************************************
 *
 * LB8K_HWECC_GetAttribute
 *
 * Get nandflash type
 *
 * RETURNS
 *
 * NfType->    TRUE:  LARGE
 *             FALSE: SMALL
 * EccStatus-> TRUE:  HWECC
 *             FALSE: SWECC
 *
 ***************************************************************************/

void
LB8K_HWECC_GetAttribute(
	MMP_BOOL* NfType,
	MMP_BOOL* EccStatus)
{
#ifdef LARGEBLOCK_2KB
	*NfType = MMP_TRUE;
#else
	*NfType = MMP_FALSE;
#endif

#ifdef NF_HW_ECC
	*EccStatus = MMP_TRUE;
#else
	*EccStatus = MMP_FALSE;
#endif
}

unsigned char*
LB8K_HWECC_GetBlockErrorRecord()
{
#ifdef LB_ECC_RECORD
	return g_pBlockErrorRecord;
#else
	return MMP_NULL;
#endif
}

static MMP_UINT32
GetNandId(
    MMP_UINT8     chipIndex,
    NF_ATTRIBUTE* pNfAttrib)
{
	MMP_UINT32 result = LL_OK;
	MMP_UINT16 regData1, regData2;
    MMP_UINT32  IdCycles = READ_ID_4_CYCLE;
    MMP_UINT32  ID0=0, ID1=0;
    MMP_UINT32  NfTmpStatus=0;

	if ( !NfChipSelect(chipIndex) )		return LL_ERROR;

	if ( NAND_CMD_2ND_READ70_STATUS(&NfTmpStatus) == LL_ERROR )    return LL_ERROR;

    AHB_WriteRegister(NF_REG_CMD2, RESET_CMD_2ST);
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
    
    result = LB8K_HWECC_WaitCmdReady(NF_READ_ID);

	result = NAND_CMD_1ST_READID90(&ID0, &ID1, IdCycles);
	
	printf("ret=%x\n",result);
	
	if ( !result )
	{
		MMP_UINT8 Byte1, Byte2, Byte3, Byte4;

        regData1 = ID0 & 0x0000FFFF;
        regData2 = (ID0&0xFFFF0000)>>16;

		Byte1 = regData1 & 0x00FF;
		Byte2 = (regData1 & 0xFF00) >> 8;
		Byte3 = regData2 & 0x00FF;
		Byte4 = (regData2 & 0xFF00) >> 8;

		// Awin@20071015
		NAND_DbgPrintf("GetNandId: ID1 = 0x%04X.\n\r", regData1);
		NAND_DbgPrintf("GetNandId: ID2 = 0x%04X.\n\r", regData2);

		printf("GetNandId: ID1 = 0x%04X.\n\r", regData1);
		printf("GetNandId: ID2 = 0x%04X.\n\r", regData2);

		switch(Byte1)
		{
		case 0xAD:  // Hynix
			if ( HynixNandflash(regData1, regData2, pNfAttrib) == MMP_FALSE )
			{
                printf("GetNandId: Get Hynix Nandflash ID Failed. ID = 0x%02X 0x%02X 0x%02X 0x%02X\n",
                    Byte1, Byte2, Byte3, Byte4);
				result = LL_ERROR;
			}
			break;
			
		case 0x98:  // Toshiba
			if ( ToshibaNandflash(regData1, regData2, pNfAttrib) == MMP_FALSE )
			{
                printf("GetNandId: Get Toshiba Nandflash ID Failed. ID = 0x%02X 0x%02X 0x%02X 0x%02X\n",
                    Byte1, Byte2, Byte3, Byte4);
				result = LL_ERROR;
			}
			break;
			
		case 0xEC:  // Samsung
			if ( SamsungNandflash(regData1, regData2, pNfAttrib) == MMP_FALSE )
			{
                printf("GetNandId: Get Samsung Nandflash ID Failed. ID = 0x%02X 0x%02X 0x%02X 0x%02X\n",
                    Byte1, Byte2, Byte3, Byte4);
				result = LL_ERROR;
			}
			break;
			
		case 0x20:  // Numonyx
			if ( NumonyxNandflash(regData1, regData2, pNfAttrib) == MMP_FALSE )
			{
                printf("GetNandId: Get Numonyx Nandflash ID Failed. ID = 0x%02X 0x%02X 0x%02X 0x%02X\n",
                    Byte1, Byte2, Byte3, Byte4);
				result = LL_ERROR;
			}
			break;

		case 0x92:  // PowerFlash
			if ( PowerFlashNandflash(regData1, regData2, pNfAttrib) == MMP_FALSE )
			{
                printf("GetNandId: Get Numonyx Nandflash ID Failed. ID = 0x%02X 0x%02X 0x%02X 0x%02X\n",
                    Byte1, Byte2, Byte3, Byte4);
				result = LL_ERROR;
			}
			break;
			
		default:
			result = LL_ERROR;
			break;
        }
	}
	
	return result;
}

static MMP_BOOL
HynixNandflash(
    MMP_UINT16    ID1,
    MMP_UINT16    ID2,
    NF_ATTRIBUTE* pNfAttrib)
{
    MMP_UINT32 BlockSize = 0;
    MMP_BOOL   bResult   = MMP_TRUE;
    
	MMP_UINT32 NandId = 0;
    MMP_UINT32 nandSize  = 0;
    MMP_UINT32 blockSize = 0;
    
	NandId=((MMP_UINT32)ID2 << 16) | (MMP_UINT32)ID1;
    switch( NandId)
    {
    case 0x1D80F1AD:
        nandSize                 = 128 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_HYNIX;
        pNfAttrib->checkMethod   = NAND_HYNIX_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockHynixMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0884;
        strcpy(pNfAttrib->modelName, "HY27UF081G2A");
        break;

    case 0x9510DAAD:  // This model has 5 bytes ID, that is 0xAD DA 10 95 44
        nandSize                 = (MMP_UINT32)256 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_HYNIX;
        pNfAttrib->checkMethod   = NAND_HYNIX_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockHynixMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "HY27UF082G2B");
        break;
        
    case 0xA514DCAD:
        nandSize                 = 512 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_HYNIX;
        pNfAttrib->checkMethod   = NAND_HYNIX_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockHynixMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "HY27UT084G2A");
        break;

    case 0xA514D3AD:  // This model has 5 bytes ID, that is 0xAD D3 14 A5 34
        nandSize                 = (MMP_UINT32)1024 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_HYNIX;
        pNfAttrib->checkMethod   = NAND_HYNIX_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockHynixMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "HY27UT088G2A");
        break;

    case 0xA555D5AD:  // This model has 5 bytes ID, that is 0xAD D5 55 A5 38
        nandSize                 = (MMP_UINT32)2048 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_HYNIX;
        pNfAttrib->checkMethod   = NAND_HYNIX_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockHynixMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "HY27UV08BG5A");
        break;
        
    default:
        bResult = MMP_FALSE;
        break;
    }

	return bResult;
}

static MMP_BOOL
ToshibaNandflash(
    MMP_UINT16    ID1,
    MMP_UINT16    ID2,
    NF_ATTRIBUTE* pNfAttrib)
{
	MMP_UINT32 BlockSize = 0;
	MMP_BOOL   bResult   = MMP_TRUE;
   
	MMP_UINT32 NandId = 0;
    MMP_UINT32 nandSize  = 0;
    MMP_UINT32 blockSize = 0;
    
	NandId=((MMP_UINT32)ID2 << 16) | (MMP_UINT32)ID1;
    switch( NandId)
    {
    case 0x1590D198:  // This model has 4 bytes ID, that is 0x98 D1 90 15
        nandSize                 = 128 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_TOSHIBA;
        pNfAttrib->checkMethod   = NAND_TOSHIBA_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockToshibaMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0884;
        strcpy(pNfAttrib->modelName, "TC58NVG0S3ETA00");
        break;
        
    case 0x1590DA98:  // This model has 5 bytes ID, that is 0x98 DA 90 15
        nandSize                 = 256 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_TOSHIBA;
        pNfAttrib->checkMethod   = NAND_TOSHIBA_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockToshibaMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "TC58NVG1S3ETA00");
        break;

    default:
        bResult = MMP_FALSE;
        break;
    }

	return bResult;
}

static MMP_BOOL
SamsungNandflash(
    MMP_UINT16    ID1,
    MMP_UINT16    ID2,
    NF_ATTRIBUTE* pNfAttrib)
{
    MMP_UINT32 BlockSize = 0;
    MMP_BOOL   bResult = MMP_TRUE;
    
	MMP_UINT32 NandId = 0;
    MMP_UINT32 nandSize  = 0;
    MMP_UINT32 blockSize = 0;
    
	NandId=((MMP_UINT32)ID2 << 16) | (MMP_UINT32)ID1;
    switch( NandId)
    {
	case 0x7284D5EC:  // This model has 5 bytes ID, that is 0xEC D3 14 25 64
        nandSize                 = (MMP_UINT32)2048 * 1024 * 1024; // Bytes
        blockSize                = 1024 * 1024;         // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod02;
        pNfAttrib->pageSize      = 8192;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        //pNfAttrib->clockValue    = 0x00343333;
        pNfAttrib->clockValue    = 0x40777777;
        pNfAttrib->register701C  = 0x00003105;
        strcpy(pNfAttrib->modelName, "K9GAG08U0E");
        break;
         
	case 0x2514D3EC:  // This model has 5 bytes ID, that is 0xEC D3 14 25 64
        nandSize                 = (MMP_UINT32)1024 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;         // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x00001005;
        strcpy(pNfAttrib->modelName, "K9GAG08U0M");
        break;
        
    case 0xA514D3EC:
        nandSize                 = (MMP_UINT32)1024 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;         // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00777777;
        pNfAttrib->register701C  = 0x00001005;
        strcpy(pNfAttrib->modelName, "K9G8G08U0A,K9G8G08U0B");
        break;
        
    case 0xA555D5EC:  // This model has 5 bytes ID, that is 0xEC D5 55 A5 68
        nandSize                 = (MMP_UINT32)2048 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;         // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "K9LAG08U0A,K9HBG08U1A");
        break;
    
    case 0x2555D5EC:  // This model has 5 bytes ID, that is 0xEC D5 55 25 68
        nandSize                 = (MMP_UINT32)2048 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;         // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "K9LAG08U0M");
        break;

    case 0x9500F1EC:
        nandSize                 = 128 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0884;
        strcpy(pNfAttrib->modelName, "K9F1G08U0A");
        break;
        
    case 0x9510DCEC:  // This model has 5 bytes ID, that is 0xEC DC 10 95 54
        nandSize                 = 512 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "K9F4G08U0A");
        break;
        
    case 0x2514DCEC:  // This model has 5 bytes ID, that is 0xEC DC 14 25 54
        nandSize                 = 512 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x40777777;
        pNfAttrib->register701C  = 0x00001005;
        strcpy(pNfAttrib->modelName, "K9G4G08U0A");
        break;

    case 0xA514DCEC:  // This model has 5 bytes ID, that is 0xEC DC 14 A5 54
        nandSize                 = 512 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x40777777;
        pNfAttrib->register701C  = 0x00001005;
        strcpy(pNfAttrib->modelName, "K9G4G08U0B");
        break;


    case 0x9551D3EC:	// This model has 5 bytes ID, that is 0xEC D3 51 95 58
        nandSize                 = 1024 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_SAMSUNG;
        pNfAttrib->checkMethod   = NAND_SAMSUNG_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockSamsungMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "K9K8G08U0A");
        break;
        
    default:
        bResult = MMP_FALSE;
        break;
    }

	return bResult;
}

static MMP_BOOL
NumonyxNandflash(
    MMP_UINT16    ID1,
    MMP_UINT16    ID2,
    NF_ATTRIBUTE* pNfAttrib)
{
    MMP_UINT32 BlockSize = 0;
    MMP_BOOL   bResult = MMP_TRUE;
    
	MMP_UINT32 NandId = 0;
    MMP_UINT32 nandSize  = 0;
    MMP_UINT32 blockSize = 0;
    
	NandId=((MMP_UINT32)ID2 << 16) | (MMP_UINT32)ID1;
    switch( NandId)
    {
    case 0x9510DA20:
        nandSize                 = 256 * 1024 * 1024; // Bytes
        blockSize                = 128 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_NUMONYX;
        pNfAttrib->checkMethod   = NAND_NUMONYX_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockNumonyxMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "NAND02GW3B2D");
        break;

    case 0xA514D320:
        nandSize                 = 1024 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_NUMONYX;
        pNfAttrib->checkMethod   = NAND_NUMONYX_METHOD_02;
        pNfAttrib->pfCheckMethod = _IsBadBlockNumonyxMethod02;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "NAND08GW3C2B");
        break;

    default:
        bResult = MMP_FALSE;
        break;
    }

	return bResult;
}

static MMP_BOOL
PowerFlashNandflash(
    MMP_UINT16    ID1,
    MMP_UINT16    ID2,
    NF_ATTRIBUTE* pNfAttrib)
{
    MMP_UINT32 BlockSize = 0;
    MMP_BOOL   bResult = MMP_TRUE;
    
	MMP_UINT32 NandId = 0;
    MMP_UINT32 nandSize  = 0;
    MMP_UINT32 blockSize = 0;
    
	NandId=((MMP_UINT32)ID2 << 16) | (MMP_UINT32)ID1;
    switch( NandId)
    {
    case 0x2504DC92: // This model has 5 bytes ID, that is 0x92 DC 04 25 00
        nandSize                 = 512 * 1024 * 1024; // Bytes
        blockSize                = 256 * 1024;        // Bytes
        pNfAttrib->ID1           = ID1;
        pNfAttrib->ID2           = ID2;
        pNfAttrib->nandMaker     = NAND_MAKER_POWERFLASH;
        pNfAttrib->checkMethod   = NAND_POWERFLASH_METHOD_01;
        pNfAttrib->pfCheckMethod = _IsBadBlockPowerFlashMethod01;
        pNfAttrib->pageSize      = 2048;
        pNfAttrib->pagePerBlock  = blockSize / pNfAttrib->pageSize;
        pNfAttrib->numOfBlocks   = nandSize / blockSize;
        pNfAttrib->clockValue    = 0x00333333;
        pNfAttrib->register701C  = 0x0885;
        strcpy(pNfAttrib->modelName, "A1U4GA30GT");
        break;
        
    default:
        bResult = MMP_FALSE;
        break;
    }

	return bResult;
}

MMP_BOOL
NfChipSelect(
	MMP_UINT32 chipIndex)
{
#ifdef __FREERTOS__
	MMP_UINT16 regValue   = 0;
	MMP_UINT32 chipSelect = 0;
	MMP_BOOL   result     = MMP_TRUE;

#if 0
	switch(chipIndex)
	{
	case 0:  regValue = 0x00; chipSelect = 0x00000008; break;
	case 1:  regValue = 0x08; chipSelect = 0x00010008; break;
	case 2:  regValue = 0x0C; chipSelect = 0x00030008; break;
	case 3:  regValue = 0x04; chipSelect = 0x00030008; break;
	default: result = MMP_FALSE; break;
	}
#else
    switch(chipIndex)
	{
	case 0:  regValue = 0x00; chipSelect = 0x00000008; break;
	case 1:  regValue = 0x04; chipSelect = 0x00010008; break;
	default: result = MMP_FALSE; break;
	}
#endif

	//HOST_WriteRegisterMask(0x16B0, regValue, 0x0C);
	//AHB_WriteRegisterMask(0x68000048, chipSelect, 0x00030008);
	
#if 0
    if ( chipIndex == 1 )
    {
        while(1){}
    }
#endif

	return result;
#elif defined(WIN32)
	MMP_UINT16 regValue   = 0;
	MMP_UINT16 chipSelect = 0;
	MMP_BOOL   result     = MMP_TRUE;

	if ( chipIndex == 1 )
		printf("NfChipSelect: Use 2nd die\n");
	else if ( chipIndex > 1 )
		printf("NfChipSelect: Use unknown die, FATAL ERROR!\n");

	switch(chipIndex)
	{
	case 0:  regValue = 0x00; chipSelect = 0x0000; break;
	case 1:  regValue = 0x04; chipSelect = 0x0001; break;
	default: result = MMP_FALSE; break;
	}

	// Set controller chip select
	//HOST_WriteRegisterMask(0x16B0, regValue, 0x0C);

	// Set GPIO8
	//HOST_WriteRegisterMask(0x784A, chipSelect, 0x0003);

	return result;
#endif
}

void
ResetChipSelect()
{
    //AHB_WriteRegisterMask(0x68000048, 0x00000000, 0x00030008);
    return;
    AHB_WriteRegister(0x68000048, g_OriGpioValue);
}

MMP_BOOL
IsErasedBlock(
    MMP_UINT8* pData, 
    MMP_UINT8* pSpare)
{
    MMP_UINT32 i;
    MMP_UINT32  reg32;
    MMP_BOOL   bErased = MMP_TRUE;
#ifdef ENABLE_HW_2K_PAGE
    MMP_UINT8  SprSbr[SPARE_SIZE] = {0x43, 0x07, 0x02, 0x98, 0xc0, 0xd7, 0x8f, 0x22, 
                             0xe6, 0x84, 0xf9, 0x34, 0xf4, 0x94, 0x56, 0xc0};
#else
    MMP_UINT8  SprSbr[SPARE_SIZE] = {0x32, 0xaf, 0x49, 0x07, 0xd0, 0xc2, 0xe7, 0x40, 
                             0xbb, 0xe4, 0x4c, 0x26, 0x7f, 0x61, 0x63, 0xc3,
							 0xda, 0x79, 0xc2, 0xae, 0x40, 0x5d, 0xdd, 0x1f};	//BCH 24-bit
#endif
#if 0 //#ifdef __FREERTOS__
    MMP_UINT8* pAlignSpareBuf  = (MMP_UINT8*)(((MMP_UINT32)pSpare + 3) & ~3);

    if ( pAlignSpareBuf == pSpare )
    {
        for ( i=0; i<SPARE_SIZE; i+=4 )
        {
            if ( *((MMP_UINT32*)(pSpare + i)) != 0xFFFFFFFF )
            {
                bErased = MMP_FALSE;
                break;
            }
        }
    }
    else
#endif

#if 0
    {
        printf("---------- SPARE DATA ----------\n");
        for ( i=0; i<SPARE_SIZE; i+=8 )
        {
            printf("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
                pSpare[i], pSpare[i+1], pSpare[i+2], pSpare[i+3],
                pSpare[i+4], pSpare[i+5], pSpare[i+6], pSpare[i+7]);
        }
    }
#endif
    AHB_ReadRegister(NF_REG_WR_SCRAMB_INIT, &reg32);
    AHB_ReadRegister(NF_REG_RD_SCRAMB_INIT, &reg32);
    AHB_ReadRegister(NF_REG_GENERAL, &reg32);

    if(reg32&0x0080000)
    {
        for ( i=0; i<SPARE_SIZE; i++ )
        {
            if ( pSpare[i] != SprSbr[i] )
            {
                bErased = MMP_FALSE;
                break;
            }
        }
    }
    else
    {
        for ( i=0; i<SPARE_SIZE; i++ )
        {
            if ( pSpare[i] != 0xFF )
            {
                bErased = MMP_FALSE;
                break;
            }
        }
    }

    return bErased;
}

void EnableNfControl()
{    
	//MMP_UINT32 BlockNum;//for FPGA verify, set this variable as 0
    MMP_UINT32 BlockNum=0;//for FPGA verify
	MMP_UINT32 SecLength;
	MMP_INT	Result;
	
	//Result = mmpxDGetCapacity(&BlockNum, &SecLength);

	if(BlockNum)
	{
		//if XD card exist, then pull up XDCEN pin
		//HOST_WriteRegisterMask(0x784E, 0x0080,0x00C0);//pull up XDCE pin
	}
	//HOST_WriteRegisterMask(0x784C, 0x0008,0x000C);//pull up CF_RST pin
   
    //AHB_WriteRegisterMask((GPIO_BASE+0x4C), PULL_UP_CF_IOWR, CF_IOWR_PULL_UP_DOWN_MASK);
    //AHB_WriteRegisterMask((GPIO_BASE+0x4C), PULL_UP_NDCS0, NDCS0_PULL_UP_DOWN_MASK);
    return;
}

// Check byte 0 in data & 1st bytes in spare of 1st & 2nd page
// Read either column 0 or 2048 of the 1st page or the 2nd page of each block.
static MMP_UINT8
_IsBadBlockToshibaMethod01(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8 result      = LL_ERROR;
    MMP_UINT8 blockStatus = LL_ERROR;
    MMP_UINT8 dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockToshibaMethod01: \n");
    
    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, 0, dataBuffer);
    if (   result == LL_ERASED
        && dataBuffer[0]      == 0xFF 
        && dataBuffer[CI2048] == 0xFF )
    {
        // ----------------------------------------
        // 2. Read 2nd page
        result = LB8K_HWECC_Read(pba, 1, dataBuffer);
        if (   result == LL_ERASED
            && dataBuffer[0]      == 0xFF 
            && dataBuffer[CI2048] == 0xFF )
        {
            blockStatus = LL_OK;
        }
    }

    return blockStatus;
}

// Check byte 0 in data & 1st bytes in spare of last page
// Read either column 0 or 2048 of the last page of each block.
static MMP_UINT8
_IsBadBlockToshibaMethod02(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 lastPage    = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8  dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockToshibaMethod02: \n");
    
    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, lastPage, dataBuffer);
    if (   result == LL_ERASED
        && dataBuffer[0]      == 0xFF 
        && dataBuffer[CI2048] == 0xFF )
    {
        // ----------------------------------------
        // 2. Read 2nd page
        result = LB8K_HWECC_Read(pba, 1, dataBuffer);
        if (   result == LL_ERASED
            && dataBuffer[0]      == 0xFF 
            && dataBuffer[CI2048] == 0xFF )
        {
            blockStatus = LL_OK;
        }
    }

    return blockStatus;
}

// Check byte 0 in data & 1st bytes in spare of 1st page & last page
// Read FFh Check Column 0 or 2048 of the First page
// Read FFh Check Column 0 or 2048 of the last page
static MMP_UINT8
_IsBadBlockToshibaMethod03(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 lastPage    = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8  dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockToshibaMethod03: \n");
    
    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, 0, dataBuffer);
    if (   result == LL_ERASED
        && dataBuffer[0]      == 0xFF 
        && dataBuffer[CI2048] == 0xFF )
    {
        // ----------------------------------------
        // 2. Read last page
        result = LB8K_HWECC_Read(pba, lastPage, dataBuffer);
        if (   result == LL_ERASED
            && dataBuffer[0]      == 0xFF 
            && dataBuffer[CI2048] == 0xFF )
        {
            blockStatus = LL_OK;
        }
    }

    return blockStatus;
}

// Check 1st bytes in spare of 1st & 2nd page
// The invalid block(s) status is defined by the 1st byte in the spare area.
// Samsung makes sure that either the 1st or 2nd page of every invalid block has non-FFh data at the column address of 2048.
static MMP_UINT8 
_IsBadBlockSamsungMethod01(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 lastPage    = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8  dataBuffer[DATA_SIZE];
    MMP_UINT32 i;

    //printf(" --> _IsBadBlockSamsungMethod01: \n");
    
    // ----------------------------------------
    // 1. Read last page
    // --- Check 1st bytes in spare of 1st & 2nd page
    result = LB8K_HWECC_Read(pba, 0, dataBuffer);

    if (   result == LL_ERASED
        && dataBuffer[CI2048] == 0xFF )
    {
        // ----------------------------------------
        // 2. Read 2nd page
        //printf("R2\r\n");
        result = LB8K_HWECC_Read(pba, 1, dataBuffer);
    	
        if (   result == LL_ERASED
            && dataBuffer[CI2048] == 0xFF )
        {
            blockStatus = LL_OK;
        }
    }
    
    return blockStatus;
}

// Check 1st bytes in spare of last page
// The initial invalid block(s) status is defined by the 1st byte in the spare area.
// Samsung makes sure that the last page of every initial invalid block has non-FFh data at the column address of 2,048.
static MMP_UINT8 
_IsBadBlockSamsungMethod02(
    MMP_UINT32 pba)
{
    //const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    const MMP_UINT32 CI2048 = 2027;  // CI2048 means "column index 2027 in BCH 12-bit mode"
    MMP_UINT8  CheckValue=0xFF;
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 lastPage    = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8  dataBuffer[DATA_SIZE+SPARE_SIZE];

    //printf(" --> _IsBadBlockSamsungMethod02: \n");
    
    // ----------------------------------------
    // 1. Read last page
    // --- Check 1st bytes in spare of last page
    result = LB8K_HWECC_Read(pba, lastPage, dataBuffer);

    if(IsEnHwScrambler) 
    {
        CheckValue=0x1F;
    }

    if (   result == LL_ERASED
        && dataBuffer[CI2048] == CheckValue )
    {
        blockStatus = LL_OK;
    }

    return blockStatus;
}

// Check 1st bytes in spare of 1st & 2nd page
// Any block where the 1st Byte in the spare area of the 1st or 2nd page(if the 1st page is Bad) does not contain FFh is a Bad Block.
static MMP_UINT8
_IsBadBlockHynixMethod01(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8 result      = LL_ERROR;
    MMP_UINT8 blockStatus = LL_ERROR;
    MMP_UINT8 dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockHynixMethod01: \n");
    
    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, 0, dataBuffer);
    if (   result == LL_ERASED
        && dataBuffer[CI2048] == 0xFF )
    {
        // ----------------------------------------
        // 2. Read 2nd page
        result = LB8K_HWECC_Read(pba, 1, dataBuffer);
        if (   result == LL_ERASED
            && dataBuffer[CI2048] == 0xFF )
        {
            blockStatus = LL_OK;
        }
    }

    return blockStatus;
}

// Check 1st bytes in spare of last page & (last-2) page
// Any block where the 1st Byte in the spare area of the Last or (Last-2)th page (if the last page is Bad) does not contain FFh is a Bad Block.
static MMP_UINT8
_IsBadBlockHynixMethod02(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 lastPage    = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT32 last2Page   = lastPage - 2;
    MMP_UINT8  dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockHynixMethod02: \n");
    
    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, lastPage, dataBuffer);
    if (   (result == LL_ERASED)
        && (dataBuffer[CI2048] == 0xFF) )
    {
        // ----------------------------------------
        // 2. Read 2nd page
        result = LB8K_HWECC_Read(pba, last2Page, dataBuffer);
        if (   (result == LL_ERASED)
            && (dataBuffer[CI2048] == 0xFF) )
        {
            blockStatus = LL_OK;
        }
    }

    return blockStatus;
}

// Check 1st & 6th byte in spare of 1st page
// Any block, where the 1st and 6th bytes or the 1st word in the spare area of the 1st page, does not contain FFh, is a bad block.
static MMP_UINT8 
_IsBadBlockNumonyxMethod01(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8 result      = LL_ERROR;
    MMP_UINT8 blockStatus = LL_ERROR;
    MMP_UINT8 dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockNumonyxMethod01: \n");
    
    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, 0, dataBuffer);
    if (   result == LL_ERASED
        && dataBuffer[CI2048]     == 0xFF
        && dataBuffer[CI2048 + 5] == 0xFF )
    {
        blockStatus = LL_OK;
    }

    return blockStatus;
}

// Check 1st byte in spare of last page
// Any block, where the 1st and 6th bytes or the 1st word in the spare area of the 1st page, does not contain FFh, is a bad block.
static MMP_UINT8 
_IsBadBlockNumonyxMethod02(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8 result      = LL_ERROR;
    MMP_UINT8 blockStatus = LL_ERROR;
    MMP_UINT32 lastPage    = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8 dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockNumonyxMethod02: \n");
    
    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, lastPage, dataBuffer);
    if (   result == LL_ERASED
        && dataBuffer[CI2048] == 0xFF)
    {
        blockStatus = LL_OK;
    }

    return blockStatus;
}

// Bad blocks can be read by accessing the first byte and the 2048th byte of the first page and the last page.
static MMP_UINT8 
_IsBadBlockPowerFlashMethod01(
    MMP_UINT32 pba)
{
    const MMP_UINT32 CI2048 = 2000;  // CI2048 means "column index 2048"
    
    MMP_UINT8  result      = LL_ERROR;
    MMP_UINT8  blockStatus = LL_ERROR;
    MMP_UINT32 lastPage    = g_NfAttrib.pagePerBlock - 1;
    MMP_UINT8  dataBuffer[DATA_SIZE];

    //printf(" --> _IsBadBlockPowerFlashMethod01: \n");

    // ----------------------------------------
    // 1. Read 1st page
    result = LB8K_HWECC_Read(pba, 0, dataBuffer);
    if (   result == LL_ERASED
        && dataBuffer[0]         == 0xFF 
        && dataBuffer[CI2048] == 0xFF )
    {
        // ----------------------------------------
        // 2. Read last page
        result = LB8K_HWECC_Read(pba, lastPage, dataBuffer);
        if (   result == LL_ERASED
            && dataBuffer[0]         == 0xFF
            && dataBuffer[CI2048] == 0xFF )
        {
            blockStatus = LL_OK;
        }
    }
    
    return blockStatus;
}

static MMP_UINT8
NAND_CMD_2ND_READ70_STATUS(
    MMP_UINT32* NfStatus)
{
    //MMP_UINT16 nfStatus;
    MMP_UINT8  result  = LL_OK;
    MMP_UINT32 timeOut = READ_STATUS_TIME_OUT_COUNT;
    MMP_INT    EventRst;

    *NfStatus = 0;

    AHB_WriteRegister(NF_REG_CMD2, READSTATUS_CMD_2ND);
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);

    #if defined(NAND_IRQ_ENABLE)    
    //need wait event
    if(NandIsrEvent)
    {	
		EventRst = SYS_WaitEvent(NandIsrEvent, 20);
		//printf("WaitNfReady.event.got!!rst=%x\n",EventRst);
    	if(result)
    	{
    		printf("[Nf ERR]:INTR time out\n");
    	    result = LL_ERROR;
    	    goto end;
    	}
    }
    #else
    MMP_Sleep(1);
    #endif
    
    do
    {
        AHB_ReadRegister(NF_REG_STATUS, NfStatus);

        if ( timeOut-- == 0 )
        {
            printf("\tLB8K_HWECC_WaitCmdReady wait idle time out[2]!!!\n");
            result = LL_ERROR;
            goto end;
        }
    }while( (*NfStatus&0xc000) != NFIDLE );

    *NfStatus = *NfStatus & 0xFF;

end:
    return  result;
}

static MMP_UINT8
NAND_CMD_1ST_READID90(
    MMP_UINT32* CmdID0, MMP_UINT32* CmdID1, MMP_UINT32 IdCycles)
{
    MMP_UINT8  result  = LL_OK;
    //MMP_UINT32  Reg32Tmp;

#ifdef WIN32
    HOST_WriteRegister(NF_REG_CMD1, READID90_CMD_1ST);
    HOST_WriteRegister(NF_REG_IDCYCLE, IdCycles);
    HOST_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
#else
    AHB_WriteRegister(NF_REG_CMD1, READID90_CMD_1ST);
    AHB_WriteRegister(NF_REG_IDCYCLE, IdCycles);
    AHB_WriteRegister(NF_REG_CMD_FIRE, NF_CMD_FIRE);
#endif

    if ( LB8K_HWECC_WaitCmdReady(NF_READ_ID)==LL_OK )
    {
        AHB_ReadRegister(NF_REG_NFMC_ID1, CmdID0);
        AHB_ReadRegister(NF_REG_NFMC_ID2, CmdID1);
    }
    else
    {
        result = LL_ERROR;
    }

    NAND_DbgPrintf("GetNandId: ID32_1 = 0x%08X.\n\r", *CmdID0);
    NAND_DbgPrintf("GetNandId: ID32_2 = 0x%08X.\n\r", *CmdID1);

    return result;
}

MMP_UINT8 GetErrorPageNum(MMP_UINT32 ErrMap)
{
    MMP_UINT8	i,ErrorPageNum=0;

    for(i=0;i<g_TotalNumOfMapBit;i++)
    {
        if( ((ErrMap>>i)&0x0001)==0x0000 )	ErrorPageNum++;
    }

    return	ErrorPageNum;
}


MMP_UINT32 GetCorrectNumInThisSegment(MMP_UINT32 PageLoop)
{
    MMP_UINT32	ErrNum;
    MMP_UINT16	k;

    MMP_UINT32  TotalOffset=0;
    MMP_UINT16	*p16BchPt;    
    //MMP_UINT8	*BCHpt=g_pNFeccBuf;
    #ifdef WIN32
    MMP_UINT16	*BCHpt=(MMP_UINT16 *)g_pW32NFBchBuf;
    #else
    MMP_UINT16	*BCHpt=(MMP_UINT16 *)g_pNFeccBuf;
    MMP_UINT8		*BCH8pt;
		#endif


    //printf("GCNTS:PageLoop=%x.\r\n",PageLoop);
    //printf("GCNTS:g_pNFeccBuf=%x.\r\n",BCHpt);
    for(k=0;k<(PageLoop+1);k++)
    {
    		#ifdef WIN32    		
        ErrNum=(((MMP_UINT32)BCHpt[1])<<16)+((MMP_UINT32)BCHpt[0]);
				BCHpt = &BCHpt[((ErrNum+1)&0xFFFFFFFE)+2];        
        #else
        BCH8pt = (MMP_UINT8	*)BCHpt;
        ErrNum=( (((MMP_UINT32)BCH8pt[3])<<24)+(((MMP_UINT32)BCH8pt[2])<<16)+(((MMP_UINT32)BCH8pt[1])<<8)+(MMP_UINT32)BCH8pt[0] );
        BCHpt = &BCHpt[((ErrNum+1)&0xFFFFFFFE)+2];
        #endif
    }

    return	ErrNum;	
}

MMP_UINT16 GetErrorAddr(MMP_UINT8 PageLoop, MMP_UINT8 BitLoop)
{
    MMP_UINT32  ErrNum;
    MMP_UINT16	ErrAddr,k;
    //MMP_UINT8	*BCHpt=g_pNFeccBuf;
    #ifdef WIN32
    MMP_UINT16	*BCHpt=(MMP_UINT16 *)g_pW32NFBchBuf;
    #else
    MMP_UINT16	*BCHpt=(MMP_UINT16 *)g_pNFeccBuf;
    MMP_UINT8		*BCH8pt;
		#endif


    //printf("GEA:PageLoop=%x,BitLoop=%x.\r\n",PageLoop,BitLoop);

    //BCHpt=(MMP_UINT8 *)((MMP_UINT32)g_pNFeccBuf+(MMP_UINT32)LocAddr);

    for(k=0;k<(PageLoop+1);k++)
    {		
    		#ifdef WIN32    		
        ErrNum=(((MMP_UINT32)BCHpt[1])<<16)+((MMP_UINT32)BCHpt[0]);
        #else
        BCH8pt = (MMP_UINT8	*)BCHpt;
        ErrNum=( (((MMP_UINT32)BCH8pt[3])<<24)+(((MMP_UINT32)BCH8pt[2])<<16)+(((MMP_UINT32)BCH8pt[1])<<8)+(MMP_UINT32)BCH8pt[0] );
        #endif
        
				if(k==PageLoop)
				{
						BCHpt = &BCHpt[2];
				}
				else
				{
						if(ErrNum&0x01)
						{
								BCHpt = &BCHpt[ErrNum+3];
						}
						else
						{
								BCHpt = &BCHpt[ErrNum+2];
						}			
				}
    }
    #ifdef WIN32
    ErrAddr = BCHpt[BitLoop];
    #else
    BCH8pt = (MMP_UINT8	*)BCHpt;
    ErrAddr = (((MMP_UINT32)BCH8pt[(BitLoop*2)+1])<<8) + (MMP_UINT32)BCH8pt[(BitLoop*2)];
    #endif
    ErrAddr = ErrAddr&0x03FF;

    return	ErrAddr;	
}

MMP_UINT8 GetErrorBit(MMP_UINT8 PageLoop,MMP_UINT8 BitLoop)
{
    MMP_UINT32  ErrNum;
    MMP_UINT16	Errbit;
    MMP_UINT16	k;
    //MMP_UINT8	*BCHpt=g_pNFeccBuf;
    #ifdef WIN32
    MMP_UINT16	*BCHpt=(MMP_UINT16 *)g_pW32NFBchBuf;
    #else
    MMP_UINT16	*BCHpt=(MMP_UINT16 *)g_pNFeccBuf;
    MMP_UINT8   *BCH8pt;
    #endif

    for(k=0;k<(PageLoop+1);k++)
    {		
        #ifdef WIN32    		
        ErrNum=(((MMP_UINT32)BCHpt[1])<<16)+((MMP_UINT32)BCHpt[0]);
        #else
        BCH8pt = (MMP_UINT8	*)BCHpt;
        ErrNum=( (((MMP_UINT32)BCH8pt[3])<<24)+(((MMP_UINT32)BCH8pt[2])<<16)+(((MMP_UINT32)BCH8pt[1])<<8)+(MMP_UINT32)BCH8pt[0] );
        #endif

        if(k==PageLoop)
        {
            BCHpt = &BCHpt[2];
        }
        else
        {
            if(ErrNum&0x01)
            {
                BCHpt = &BCHpt[ErrNum+3];
            }
            else
            {
                BCHpt = &BCHpt[ErrNum+2];
            }			
        }
    }

    #ifdef WIN32
    Errbit = BCHpt[BitLoop];
    #else
    BCH8pt = (MMP_UINT8	*)BCHpt;
    Errbit = (((MMP_UINT32)BCH8pt[(BitLoop*2)+1])<<8) + (MMP_UINT32)BCH8pt[BitLoop*2];
    #endif
		
    Errbit = (Errbit>>12);

    return	Errbit;		
}

MMP_UINT8 GetErrorPage(MMP_UINT8 PageLoop)
{
    MMP_UINT32	Reg704C;
    MMP_UINT8	i,ErrorPageNum=0;

    AHB_ReadRegister(NF_REG_BCH_ECC_MAP, &Reg704C);
    //printf("Reg704C=%x\r\n",Reg704C);

    for(i=0;i<g_TotalNumOfMapBit;i++)    //for 2kB/page nand
    {
        if( ((Reg704C>>i)&0x00000001)==0x0000 )
        {
            ErrorPageNum++;
            //printf("PL=%x,EPN=%x.\r\n",PageLoop,ErrorPageNum);
            if(ErrorPageNum==(PageLoop+1))
            {
                //printf("i=%x,PL=%x,EPN=%x.\r\n",i,PageLoop,ErrorPageNum);
                if(g_BchMapMask==g_pNFeccCurrRdMask)
                {
                    return	i;
                }
                else
                {
                    MMP_UINT32 CurrRdLen;
                    CurrRdLen=GetErrorPageNum((~g_pNFeccCurrRdMask)&g_BchMapMask);
                    //printf("CurrRdLen=%x,[%x,%x,%x]\n", CurrRdLen, g_pNFeccCurrRdMask, g_BchMapMask, (~g_pNFeccCurrRdMask)&g_BchMapMask);
                    return	i%CurrRdLen;
                }
            }
        }
    }
    //printf("2.i=%x,PL=%x,EPN=%x.\r\n",i,PageLoop,ErrorPageNum);
    return	0xFF;
}

void CorrectBitIn8kBDataBuffer(MMP_UINT8 *data, MMP_UINT8 *spare, MMP_UINT8 ErrorPage,MMP_UINT16 ErrorAddr,MMP_UINT8 ErrorBit)
{
    MMP_UINT8	*DataBuffer=data;
    MMP_UINT8	ErrByte,i;
    MMP_UINT16	ErrorByteAddress,ErrBitMask;
    MMP_UINT32	Last1kB_Index;
    MMP_UINT32	IsErrorAtSpareArea=0;
    
    //check if Read the last 1kB of all page
    Last1kB_Index=(MMP_UINT32)0x01<<(g_TotalNumOfMapBit-1);
    if(g_pNFeccCurrRdMask==0x80)
    {
    	//printf("0.Last1kB[%x,%x,%x]\n",Last1kB_Index,g_pNFeccCurrRdMask,(Last1kB_Index&g_pNFeccCurrRdMask));
    }
    if( (Last1kB_Index&g_pNFeccCurrRdMask)==Last1kB_Index )
    {
    	Last1kB_Index=GetErrorPageNum((~g_pNFeccCurrRdMask)&g_BchMapMask);//get the total read counts(unit:1kB)
    	//printf("1.Last1kB_Index=%x\n",Last1kB_Index);
    }
    else
    {
    	Last1kB_Index=g_TotalNumOfMapBit;
    	//printf("2.Last1kB_Index=%x\n",Last1kB_Index);
    }  	
    
    //if(ErrorPage>=(g_TotalNumOfMapBit-1))
    if(ErrorPage>=(Last1kB_Index-1))
    {
        if(ErrorAddr>=((ECC_PAGE_SIZE/2)+(SPARE_SIZE/2)))
        {
            printf("Error Address >= 0x418, means Error bit is in ECC code!!\r\n");
            return;
        }
        
        if( ErrorAddr>=(ECC_PAGE_SIZE/2) )
        {
            printf("Error Address >= 0x400, means Error bit is in spare area!!\r\n");
			IsErrorAtSpareArea=1;
        }
    }
	else
	{
        if(ErrorAddr>=(ECC_PAGE_SIZE/2))
        {
            printf("Error Address >= 0x400, means Error bit is in ECC code!!\r\n");
            return;
        }
	}
	
	if(IsErrorAtSpareArea)
	{
		DataBuffer=spare; 
    	ErrorByteAddress=(ErrorAddr-0x200)*2;
    	printf("3.DataBuffer=%x,ErrAdr=[%x,%x]\n",DataBuffer,ErrorByteAddress,ErrorAddr);
    }
    else
    {    	
    	ErrorByteAddress=ErrorPage*1024+ErrorAddr*2;
    	printf("4.DataBuffer=%x,ErrAdr=[%x,%x]\n",DataBuffer,ErrorByteAddress,ErrorAddr);
    }

    if(ErrorBit>0x07)
    {
        ErrorByteAddress++;
        ErrorBit=ErrorBit-8;
    }

    ErrByte=DataBuffer[ErrorByteAddress];

    ErrBitMask=0x01<<ErrorBit;

    if(ErrByte&ErrBitMask)
    {
        ErrByte=ErrByte-ErrBitMask;
    }
    else
    {
        ErrByte=ErrByte+ErrBitMask;
    }

    DataBuffer[ErrorByteAddress]=ErrByte;
}

void CorrectAllErrorBitsIn8kBDataBuffer(MMP_UINT8 *data, MMP_UINT8 *spare)
{
    MMP_UINT8	TotalErrorPageNum;
    MMP_UINT32	PageLoop,Reg704C;
    MMP_UINT8	i,ErrorPageNum=0;
    MMP_UINT32  j;

    AHB_ReadRegister(NF_REG_BCH_ECC_MAP, &Reg704C);		
    //it can correct
    //1.get how many errors
    TotalErrorPageNum=GetErrorPageNum(Reg704C&g_BchMapMask);	//it have to do for 2kB/page NAND
    printf("TotalErrorPageNum=%x.\r\n",TotalErrorPageNum);
    #ifdef WIN32
    HOST_ReadBlockMemory((MMP_UINT32)g_pW32NFBchBuf, (MMP_UINT32)g_pNFeccBuf, (8+(BCH_MODE*2))*TotalErrorPageNum);
	#else
    ithInvalidateDCacheRange(g_pNFeccBuf, (8+(BCH_MODE*2))*TotalErrorPageNum);
    #endif
/*
    printf("g_pNFeccBuf=[");
    for(i=0;i<32;i++)
    {
        printf("%02x,",g_pNFeccBuf[i] );
        if( (i&0x0F)==0x0F)	printf("\r\n             ");
    }
    printf("]\n");
*/
/*
    printf("g_dataBuf=\n");
    for(j=0;j<1024;j++)
    {
        printf("%02x,",data[j] );
        if( (j&0x0F)==0x0F)	printf("\r\n");
    }
    printf("\n");
*/
    for(PageLoop=0;PageLoop<TotalErrorPageNum;PageLoop++)
    {
        MMP_UINT32	ErrorNum,BitLoop,ErrorBit,ErrorPage;
        MMP_UINT16	ErrorAddr;

        ErrorNum=GetCorrectNumInThisSegment(PageLoop);
        printf("ErrorNum=%x,loop=%x.\r\n",ErrorNum,PageLoop);

        for(BitLoop=0;BitLoop<ErrorNum;BitLoop++)
        {
            ErrorAddr=GetErrorAddr(PageLoop,BitLoop);
            ErrorBit=GetErrorBit(PageLoop,BitLoop);
            ErrorPage=GetErrorPage(PageLoop);

            printf("PgLp=%x,BtLp=%x,[",PageLoop,BitLoop);
            printf("ErrPg=%x,ErrAddr=%x,ErrBit=%x.]\r\n", ErrorPage, ErrorAddr, ErrorBit);

            CorrectBitIn8kBDataBuffer(data, spare, ErrorPage, ErrorAddr, ErrorBit);
        }
    }
}

static MMP_UINT8
compare2Buf(MMP_UINT8* pBuf1, MMP_UINT8* pBuf2, MMP_UINT32 CprSize)
{
    MMP_UINT32  i;
    MMP_UINT8   CompareResult=LL_OK;

    for(i=0;i<CprSize;i++)
    {
        if(pBuf1[i]!=pBuf2[i])
        {
            printf("Compare error, i=%x,pf1=%x,pf2=%x,Ori=[%x],Dis[%x]\n",i,pBuf1,pBuf2,pBuf1[i],pBuf2[i]);
            CompareResult = LL_ERROR;
            //break;
        }
    }
    return CompareResult;
}

MMP_UINT8 IsAllData0xFF(MMP_UINT8 *pDataPtr, MMP_UINT32 Cprlen)
{
    MMP_UINT32 i;
    MMP_UINT32  reg32;
    MMP_BOOL   bErased = MMP_TRUE;
/*
    MMP_UINT8  Scrambler[16] = {0x43, 0x07, 0x02, 0x98, 0xc0, 0xd7, 0x8f, 0x22, 
                             0xe6, 0x84, 0xf9, 0x34, 0xf4, 0x94, 0x56, 0xc0};
*/
    if(IsEnHwScrambler)
    {
        for ( i=0; i<Cprlen; i++ )
        {
            if ( pDataPtr[i] != g_ScramblerTable[i] )
            {
                bErased = MMP_FALSE;
                break;
            }
        }
    }
    else
    {
		for ( i=0; i<Cprlen; i++ )
		{
			if ( pDataPtr[i] != 0xFF )
			{
				bErased = MMP_FALSE;
				break;
			}
		}
		if( bErased==MMP_TRUE )
		{
			MMP_UINT32	Reg32;
			AHB_ReadRegister(NF_REG_BCH_ECC_MAP, &Reg32);
			if( (Reg32 & g_BchMapMask) == g_BchMapMask )
			{
				bErased = MMP_FALSE;
			}
		}
    }

    return bErased;
}
#endif  // End #ifdef NF_HW_ECC
#endif  // End #ifdef LARGEBLOCK_8KB

