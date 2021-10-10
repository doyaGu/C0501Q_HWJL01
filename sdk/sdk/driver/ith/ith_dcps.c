/*
 * Copyright (c) 2012 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL H/W decompress functions.
 *
 * @author Joseph Chang
 * @version 1.0
 */
#include "ite/ith.h"	
#include "ith_cfg.h"
//#include "stdlib.h"
//#include "stdio.h"
//#include "string.h"

//=============================================================================
//                              macro defination
//=============================================================================
#define DCPS_IRQ_ENABLE			
#define ENABLE_DCPS_LOG

#ifdef CFG_CHIP_PKG_IT9910
#define ITH_DCPS_BASE		(ITH_DPU_BASE+0x300)
#else
#define ITH_DCPS_BASE		(ITH_DPU_BASE+0x100)
#endif

#define DECOMPRESS_SW_ERR_DONEINDEX_OVER    0x00000001
#define DECOMPRESS_HW_FAIL                  0x00000004

#define DCPS_INTR_MASK						0x00000001
#define DCPS_INTR_ENABLE					0x00000001
#define DCPS_INTR_DISABLE					0x00000000

#define DCPS_INTR_CLEAR_MASK				0x00000002
#define DCPS_INTR_ENCLEAR					0x00000002
#define DCPS_INTR_DISCLEAR					0x00000000

#define DCPS_N5CLK_EN                       5
//=============================================================================
//                              LOG definition
//=============================================================================
#ifdef	ENABLE_DCPS_LOG
#define ITH_DCPS_LOG_INFO	    LOG_INFO
#define ITH_DCPS_LOG_WARNING	LOG_INFO
#define ITH_DCPS_LOG_DEBUG	    LOG_INFO
#else
#define ITH_DCPS_LOG_INFO	    LOG_INFO
#define ITH_DCPS_LOG_WARNING	LOG_WARN
#define ITH_DCPS_LOG_DEBUG	    LOG_DBG
#endif

#define ITH_DCPS_LOG_ERROR	    LOG_ERR
#define ITH_DCPS_LOG_END	    LOG_END	

//#define ITH_DCPS_LOG_ERROR	    printf(
//#define ITH_DCPS_LOG_END	    );	

//=============================================================================
//                              Decompress Register definition
//=============================================================================
#define ITH_DCPS_REG_DCR0		(ITH_DCPS_BASE+0x00)
#define ITH_DCPS_REG_DCR1		(ITH_DCPS_BASE+0x04)
#define ITH_DCPS_REG_SAR		(ITH_DCPS_BASE+0x08)
#define ITH_DCPS_REG_DAR		(ITH_DCPS_BASE+0x0C)
#define ITH_DCPS_REG_SRC_SIZE	(ITH_DCPS_BASE+0x10)
#define ITH_DCPS_REG_DST_SIZE	(ITH_DCPS_BASE+0x14)
#define ITH_DCPS_REG_INTCT		(ITH_DCPS_BASE+0x18)
#define ITH_DCPS_REG_DSR		(ITH_DCPS_BASE+0x1C)
#define ITH_DCPS_REG_DDCR		(ITH_DCPS_BASE+0x20)

#define ITH_AHB_REG_CLOCK2      (0x1A)
//=============================================================================
//                              TYPE definition
//=============================================================================
#ifdef	ENABLE_DCPS_TYPE
#define DCPS_BOOL	bool
#define DCPS_UINT8	uint8_t
#define DCPS_UINT16	uint16_t
#define DCPS_UINT32	uint32_t
#else
#define DCPS_BOOL	bool
#define DCPS_UINT8	unsigned char
#define DCPS_UINT16	unsigned int
#define DCPS_UINT32	unsigned long
#endif

#define DCPS_RESULT	DCPS_BOOL
/*****************************************************************/
//globol variable
/*****************************************************************/

static uint32_t g_DcpsSetIndex;
static uint32_t g_DcpsDoneIndex;
static uint32_t g_CmdQueRunIndex;
static uint32_t g_CmdQueDoneIndex;
static uint32_t g_DcpsStatus;
static uint32_t g_CmdCntIsZero;

static uint32_t g_IsrCounter;		//for temp
static uint32_t g_SwCmdQueDone;		//for temp

static uint32_t DcpsRegs[(ITH_DCPS_REG_DDCR - ITH_DCPS_REG_DCR0 + 4) / 4];

/*****************************************************************/
//function protocol type
/*****************************************************************/

#if defined(DCPS_IRQ_ENABLE)
void dcps_isr(void* data);
#endif

static DCPS_RESULT Decompress(ITH_DCPS_INFO *DecompressInfo);
/*****************************************************************/
//function implement
/*****************************************************************/
void ithDcpsEnIntr(void)
{
	ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_ENABLE, DCPS_INTR_MASK); /** enable DCPS interrupt **/
}

void ithDcpsDisIntr(void)
{
	ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_DISABLE, DCPS_INTR_MASK); /** disable DCPS interrupt **/
}

void ithDcpsInit(ITH_DCPS_INFO *DcpsInfo)
{
    //ITH_DCPS_LOG_INFO "DecompressInitial()!!\n" ITH_DCPS_LOG_END

    //initial Decompress register    
    g_DcpsSetIndex  	= 0;
    g_DcpsDoneIndex		= 0;
    g_CmdQueRunIndex	= 0;
    g_CmdQueDoneIndex   = 0;
    g_DcpsStatus  		= 0;
    g_CmdCntIsZero      = 0;
    
    g_IsrCounter		= 0;//for test
    g_SwCmdQueDone		= 0;//for test
    
	//#ifdef	ENABLE_PATCH_DCPS_AHB0_PRIORITY_ISSUE
	//ithPrintf("~ SET AHB0 Burst size ~\n");
	ithWriteRegMaskA( ITH_AHB0_BASE + 0x88, 0x00001F00, 0x0000FF00);	//adjust the AHB burst length
	//#endif    
	
    //reset decompress engine
    if(!DcpsInfo->IsEnableComQ)
    {
    	ithWriteRegA( ITH_DCPS_REG_DCR1, 0x01);
    }
    DcpsInfo->TotalCmdqCount = ithReadRegA( ITH_DCPS_REG_DDCR );

    //ITH_DCPS_LOG_INFO "[mmpDpuTerminate] Leave\n" ITH_DCPS_LOG_END

}

bool ithDcpsFire(ITH_DCPS_INFO *DcpsInfo)
{
	bool result=true;

	//ITH_DCPS_LOG_INFO "[ithDcpsFire] Enter\n" ITH_DCPS_LOG_END

	DcpsInfo->IsEnableComQ = 0;

#ifdef	ENABLE_DCPS_CMDQ
	if(DcpsInfo->IsEnableComQ)
	{
		CmdQ_Lock();
		CmdQ_WaitSize(8*5);
		CmdQ_PackCommand(ITH_DCPS_REG_SAR, (uint32_t)DcpsInfo->srcbuf);
		CmdQ_PackCommand(ITH_DCPS_REG_DAR, (uint32_t)DcpsInfo->dstbuf);
		CmdQ_PackCommand(ITH_DCPS_REG_DST_SIZE, DcpsInfo->dstLen);
		CmdQ_PackCommand(ITH_DCPS_REG_SRC_SIZE, DcpsInfo->srcLen);
		CmdQ_PackCommand(ITH_DCPS_REG_DCR0,0x000000a2);			//fire DPU
		CmdQ_Fire();
		CmdQ_Unlock();
		
		//ITH_DCPS_LOG_ERROR "CmdQueCnt=%d\n",CmdQueCnt ITH_DCPS_LOG_END
	}
	else
	{				
#endif
		ithWriteRegA(ITH_DCPS_REG_SAR,(uint32_t)DcpsInfo->srcbuf);			//source start address in byte
		ithWriteRegA(ITH_DCPS_REG_DAR,(uint32_t)DcpsInfo->dstbuf);			//distination start address
		
		ithWriteRegA(ITH_DCPS_REG_SRC_SIZE,DcpsInfo->srcLen);				//source size in byte
		ithWriteRegA(ITH_DCPS_REG_DST_SIZE,DcpsInfo->dstLen);				//decompress desitination size in byte
		#if (CFG_CHIP_FAMILY == 9850)
		ithWriteRegA(ITH_DCPS_REG_DCR0,0x000000A2);			//fire DPU
		#else
		ithWriteRegA(ITH_DCPS_REG_DCR0,0x00000022);			//fire DPU
		#endif
		
#ifdef	ENABLE_DCPS_CMDQ
	}
#endif
	//ITH_DCPS_LOG_INFO "[ithDcpsFire] Leave\n" ITH_DCPS_LOG_END

	return result;
}

void ithDcpsGetStatus(ITH_DCPS_INFO *DcpsInfo)
{
	
	//return the decompress status 
	if(g_CmdCntIsZero)
	{
		g_DcpsStatus |= 0x10000000;
	}
	else
	{
		g_DcpsStatus &= ~0x10000000;
	}
	
	if( g_CmdQueDoneIndex==(g_CmdQueRunIndex-1) )
	{
		uint32_t QueCnt=0; 
		QueCnt = ithReadRegA( ITH_DCPS_REG_DDCR );
		if( QueCnt==g_CmdQueRunIndex )
		{
			ITH_DCPS_LOG_ERROR "error, ISR not done!!\n" ITH_DCPS_LOG_END
			g_CmdQueDoneIndex++;
			g_DcpsDoneIndex++;
		}
	}
	DcpsInfo->RegDcpsStatus = ithReadRegA(ITH_DCPS_REG_DSR);
	DcpsInfo->RegDcpsCmdqCnt = ithReadRegA( ITH_DCPS_REG_DDCR );
}

void ithDcpsGetCmdqCount(ITH_DCPS_INFO *DcpsInfo)
{	
	DcpsInfo->TotalCmdqCount = ithReadRegA( ITH_DCPS_REG_DDCR );
}

void ithDcpsWait(ITH_DCPS_INFO *DcpsInfo)
{
	uint32_t Reg32;
	do
	{
		Reg32 = ithReadRegA(ITH_DCPS_REG_DSR);
	} while((Reg32&0x00000001) != 0x00000001);
	DcpsInfo->RegDcpsStatus = Reg32;
}

void ithDcpsGetDoneLen(uint32_t *DcpsDoneLen)
{
	*DcpsDoneLen = ithReadRegA( ITH_DCPS_REG_DST_SIZE );
}

void ithDcpsClearIntr(void)
{
    ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_ENCLEAR, DCPS_INTR_CLEAR_MASK); /** enable clear DCPS interrupt **/
    ithWriteRegMaskA(ITH_DCPS_REG_INTCT, DCPS_INTR_DISCLEAR, DCPS_INTR_CLEAR_MASK);  /** disable clear DCPS interrupt **/
}

void ithDcpsExit(void)
{
    //ITH_DCPS_LOG_INFO "[ithDcpsClose] Enter\n" ITH_DCPS_LOG_END
    
    g_DcpsSetIndex   	= 0;
    g_DcpsDoneIndex   	= 0;
    g_CmdQueRunIndex    = 0;
    g_CmdQueDoneIndex   = 0;
    g_DcpsStatus  		= 0;
    
    //clear command queue counter
    ithWriteRegA( ITH_DCPS_REG_DCR1, 0x01);    //clear CmdQue counter

    //ITH_DCPS_LOG_INFO "[ithDcpsClose] Leave\n" ITH_DCPS_LOG_END

    //return true;
}
 

void ithDcpsSuspend(void)
{
	unsigned int    Reg32,TomeOutCnt=0;
    int i;
    
    //wait DCPS idle
	do
	{
		Reg32 = ithReadRegA(ITH_DCPS_REG_DSR);
		if(TomeOutCnt++>=10000)
		{
			ithPrintf("[dcps][warning]: TimeOutCnt,reg=%x\n",Reg32);
			TomeOutCnt=0;
		}
	} while((Reg32&0x00000002) == 0x00000002);
	
	//backup DCPS registers
    for (i = ITH_DCPS_REG_DCR0; i < ITH_DCPS_REG_DDCR; i+=4)
    {
        switch (i)
        {
        case ITH_DCPS_REG_INTCT:
            DcpsRegs[(i-ITH_DCPS_REG_DCR0)/4] = ithReadRegA(i);
            ithPrintf("suspend, backup dpu reg,[%x,%08x][%x,%x,%x]\n",ITH_DCPS_REG_INTCT,DcpsRegs[(i-ITH_DCPS_REG_DCR0)/4], i, (i-ITH_DCPS_REG_DCR0)/4, ITH_DCPS_REG_DCR0);
            break;
            
        default:
            // don't need to backup 
            break;
        }
    }
}

void ithDcpsResume(void)
{
    int i;
    
    //restore DCPS registers
    for (i = ITH_DCPS_REG_DCR0; i < ITH_DCPS_REG_DDCR; i+=4)
    {
        switch (i)
        {
        case ITH_DCPS_REG_INTCT:
        	ithWriteRegA( i, DcpsRegs[(i-ITH_DCPS_REG_DCR0)/4]);
            break;
            
        default:
            // don't need to backup 
            break;
        }
    }
}

void ithDcpsEnableClock(void)
{
	ithSetRegBitH(ITH_AHB_REG_CLOCK2, DCPS_N5CLK_EN);
}

void ithDcpsDisableClock(void)
{
	ithClearRegBitH(ITH_AHB_REG_CLOCK2, DCPS_N5CLK_EN);
}