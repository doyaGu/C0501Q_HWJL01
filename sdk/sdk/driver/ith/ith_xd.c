/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Xd functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"


//=============================================================================
//                              XD Register definition
//=============================================================================
#define ITH_XD_BASE 	(ITH_SD_BASE+0x100)

#define SMI_XD_CTRL					(ITH_XD_BASE)
#define SMI_XD_RTRG					(ITH_XD_BASE+0x04)
#define SMI_XD_WTRG					(ITH_XD_BASE+0x08)
#define SMI_XD_ERR					(ITH_XD_BASE+0x0C)
#define SMI_XD_ADR5					(ITH_XD_BASE+0x10)
#define SMI_XD_ADR4					(ITH_XD_BASE+0x14)
#define SMI_XD_ADR3					(ITH_XD_BASE+0x18)
#define SMI_XD_ADR2					(ITH_XD_BASE+0x1C)
#define SMI_XD_ADR1					(ITH_XD_BASE+0x20)
#define SMI_XD_CMD1VLU				(ITH_XD_BASE+0x24)
#define SMI_XD_CMD2VLU				(ITH_XD_BASE+0x28)
#define SMI_XD_LBAH					(ITH_XD_BASE+0x2C)
#define SMI_XD_LBAL				    (ITH_XD_BASE+0x30)
#define SMI_XD_RDNTLEN				(ITH_XD_BASE+0x34)
#define SMI_XD_DATALEN				(ITH_XD_BASE+0x38)
#define SMI_XD_CLOCK				(ITH_XD_BASE+0x3C)
#define SMI_XD_RDNT0				(ITH_XD_BASE+0x40)
#define SMI_XD_RDNT1				(ITH_XD_BASE+0x44)
#define SMI_XD_RDNT2				(ITH_XD_BASE+0x48)
#define SMI_XD_RDNT3				(ITH_XD_BASE+0x4C)
#define SMI_XD_RDNT4				(ITH_XD_BASE+0x50)
#define SMI_XD_RDNT5				(ITH_XD_BASE+0x54)
#define SMI_XD_RDNT6				(ITH_XD_BASE+0x58)
#define SMI_XD_RDNT7				(ITH_XD_BASE+0x5C)
#define SMI_XD_RDNT8				(ITH_XD_BASE+0x60)
#define SMI_XD_RDNT9				(ITH_XD_BASE+0x64)
#define SMI_XD_RDNTA				(ITH_XD_BASE+0x68)
#define SMI_XD_RDNTB				(ITH_XD_BASE+0x6C)
#define SMI_XD_RDNTC				(ITH_XD_BASE+0x70)
#define SMI_XD_RDNTD				(ITH_XD_BASE+0x74)
#define SMI_XD_RDNTE				(ITH_XD_BASE+0x78)
#define SMI_XD_RDNTF				(ITH_XD_BASE+0x7C)
#define SMI_XD_TRG_STATUS			(ITH_XD_BASE+0x80)
#define SMI_XD_STATUS				(ITH_XD_BASE+0x84)
#define	SMI_XD_RCLOCK				(ITH_XD_BASE+0x90)
#define	SMI_XD_WCLOCK				(ITH_XD_BASE+0x94)
#define SMI_XD_BITCNT				(ITH_XD_BASE+0x98)
#define SMI_XD_WRAP_DATA_LENGTH	    (ITH_XD_BASE+0x9C)
#define SMI_XD_WRAP_DATA_PORT		(ITH_XD_BASE+0xA0)
#define SMI_XD_WRAP_STATUS			(ITH_XD_BASE+0xA4)
#define SMI_XD_WRAP_CONTROL		    (ITH_XD_BASE+0xA8)


/*****************************************************************/
//globol variable
/*****************************************************************/
static uint32_t XdRegs[(SMI_XD_WRAP_CONTROL - SMI_XD_CTRL + 4) / 4];



void ithXdSuspend(void)
{
	unsigned int    StatusReg32,TomeOutCnt=0;
    int i;
    
    //printf("XdS1\n");
	
	//backup XD registers
    for (i = SMI_XD_CTRL; i < SMI_XD_WRAP_CONTROL; i+=4)
    {
        switch (i)
        {
        case ITH_SD_BASE:        	
            // don't need to backup 
            break;
            
        default:            
            XdRegs[(i-SMI_XD_CTRL)/4] = ithReadRegA(i);
            //printf("BkXdRed[%x][%x]=[%08x]\n",(i-SMI_XD_CTRL)/4, i, XdRegs[(i-SMI_XD_CTRL)/4]);
            break;
        }
    }
    //printf("XdS2\n");
}

void ithXdResume(void)
{
    int i;
    //printf("XdR1\n");
    
    //restore XD registers
    for (i = SMI_XD_CTRL; i < SMI_XD_WRAP_CONTROL; i+=4)
    {
        switch (i)
        {
        case ITH_SD_BASE:
        	// don't need to backup 
            break;
            
        default:            
            ithWriteRegA( i, XdRegs[(i-SMI_XD_CTRL)/4]);
            break;
        }
    }
    //printf("XdR2\n");
}

void ithXdEnableClock(void)
{
    int i;
    //printf("XdR1\n");
    
    //restore XD registers
    for (i = SMI_XD_CTRL; i < SMI_XD_WRAP_CONTROL; i+=4)
    {
        switch (i)
        {
        case ITH_SD_BASE:
        	// don't need to backup 
            break;
            
        default:            
            //ithWriteRegA( i, XdRegs[(i-SMI_XD_CTRL)/4]);
            break;
        }
    }
    //printf("XdR2\n");
}

void ithXdDisableClock(void)
{
	unsigned int    StatusReg32,TomeOutCnt=0;
    int i;
    
    //printf("XdS1\n");
	
	//backup XD registers
    for (i = SMI_XD_CTRL; i < SMI_XD_WRAP_CONTROL; i+=4)
    {
        switch (i)
        {
        case ITH_SD_BASE:        	
            // don't need to backup 
            break;
            
        default:            
            //XdRegs[(i-SMI_XD_CTRL)/4] = ithReadRegA(i);
            //printf("BkXdRed[%x][%x]=[%08x]\n",(i-SMI_XD_CTRL)/4, i, XdRegs[(i-SMI_XD_CTRL)/4]);
            break;
        }
    }
    //printf("XdS2\n");
}

