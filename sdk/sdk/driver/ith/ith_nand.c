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
//#define ITH_NAND_BASE 	(ITH_SD_BASE+0x100)

#define NF_REG_WRITE_DATA_BASE_ADDR         (ITH_NAND_BASE+0x00)
#define NF_REG_READ_DATA_BASE_ADDR          (ITH_NAND_BASE+0x04)
#define NF_REG_WRITE_SPARE_BASE_ADDR        (ITH_NAND_BASE+0x08)
#define NF_REG_READ_SPARE_BASE_ADDR         (ITH_NAND_BASE+0x0C)
#define NF_REG_LBA_DATA_LENGTH              (ITH_NAND_BASE+0x1C)//?

#define NF_REG_MEM_DST_CORR_ECC_ADDR        (ITH_NAND_BASE+0x10)

#define NF_REG_STORAGE_ADDR1                (ITH_NAND_BASE+0x14)
#define NF_REG_STORAGE_ADDR2                (ITH_NAND_BASE+0x18)

#define NF_REG_USER_DEF_CTRL                (ITH_NAND_BASE+0x20)

#define NF_REG_CMD1                         (ITH_NAND_BASE+0x28)
#define NF_REG_CMD2                         (ITH_NAND_BASE+0x2C)
#define NF_REG_CMD3                         (ITH_NAND_BASE+0x30)
#define NF_REG_CMD4                         (ITH_NAND_BASE+0x34)

#define NF_REG_CMD_FIRE                     (ITH_NAND_BASE+0x38)

#define NF_REG_GENERAL                      (ITH_NAND_BASE+0x1C)
#define NF_REG_IDCYCLE                      (ITH_NAND_BASE+0x40)
#define NF_REG_STATUS                       (ITH_NAND_BASE+0x3C)
#define NF_REG_CLOCK_DIVIDE                 (ITH_NAND_BASE+0x24)

#define NF_REG_ECC_SECTOR1_STATUS           (ITH_NAND_BASE+0x2C)
#define NF_REG_ECC_SECTOR1_ERR_ADDR1        (ITH_NAND_BASE+0x2E)
#define NF_REG_ECC_SECTOR1_ERR_ADDR2        (ITH_NAND_BASE+0x30)
#define NF_REG_ECC_SECTOR1_ERR_ADDR3        (ITH_NAND_BASE+0x32)
#define NF_REG_ECC_SECTOR1_ERR_ADDR4        (ITH_NAND_BASE+0x34)

#define NF_REG_NFMC_ID1                     (ITH_NAND_BASE+0x44)
#define NF_REG_NFMC_ID2                     (ITH_NAND_BASE+0x48)

#define NF_REG_BCH_ECC_MAP                  (ITH_NAND_BASE+0x4C)
#define NF_REG_ECC_ERROR_MAP                (ITH_NAND_BASE+0x50)
#define NF_REG_WR_SCRAMB_INIT               (ITH_NAND_BASE+0x54)
#define NF_REG_RD_SCRAMB_INIT               (ITH_NAND_BASE+0x58)

#define NF_REG_ECC_12BIT_LAST_SEG_INIT_VAL  (ITH_NAND_BASE+0x5C)
#define NF_REG_ECC_24BIT_LAST_SEG_INIT_VAL  (ITH_NAND_BASE+0x60)
#define NF_REG_ECC_40BIT_LAST_SEG_INIT_VAL  (ITH_NAND_BASE+0x64)
#define NF_REG_ECC_12BIT_SEG_INIT_VAL       (ITH_NAND_BASE+0x68)
#define NF_REG_ECC_24BIT_SEG_INIT_VAL       (ITH_NAND_BASE+0x6C)
#define NF_REG_ECC_40BIT_SEG_INIT_VAL       (ITH_NAND_BASE+0x70)

#define NF_REG_INTR                         (ITH_NAND_BASE+0x74)
#define NF_REG_RD_SCRAMB_CHECK              (ITH_NAND_BASE+0x78)
#define NF_REG_LAST				            (ITH_NAND_BASE+0x7C)


/*****************************************************************/
//globol variable
/*****************************************************************/
static uint32_t NandRegs[(NF_REG_RD_SCRAMB_CHECK - ITH_NAND_BASE + 4) / 4];



void ithNandSuspend(void)
{
	unsigned int    StatusReg32,TomeOutCnt=0;
    int i;
    
    //printf("NandS1\n");
	
	//backup NAND registers
    for (i = ITH_NAND_BASE; i < (NF_REG_RD_SCRAMB_CHECK+4); i+=4)
    {
        switch (i)
        {
        case NF_REG_CMD1:
        case NF_REG_CMD2:
        case NF_REG_CMD3:
        case NF_REG_CMD4:
        case NF_REG_CMD_FIRE:
        case NF_REG_STATUS:    
        case NF_REG_BCH_ECC_MAP:
        case NF_REG_ECC_ERROR_MAP:            	
            // don't need to backup 
            //printf("BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, NandRegs[(i-ITH_NAND_BASE)/4]);
            break;
            
        default:            
            NandRegs[(i-ITH_NAND_BASE)/4] = ithReadRegA(i);
            //printf("BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, NandRegs[(i-ITH_NAND_BASE)/4]);
            break;
        }
    }
    //printf("NandS2\n");
}

void ithNandResume(void)
{
    int i;
    //printf("NandR1\n");
    
    //restore NAND registers
    for (i = ITH_NAND_BASE; i < (NF_REG_RD_SCRAMB_CHECK+4); i+=4)
    {
        switch (i)
        {
        case NF_REG_CMD1:
        case NF_REG_CMD2:
        case NF_REG_CMD3:
        case NF_REG_CMD4:
        case NF_REG_CMD_FIRE:
        case NF_REG_STATUS:    
        case NF_REG_BCH_ECC_MAP:
        case NF_REG_ECC_ERROR_MAP:    
        	// don't need to backup 
        	//printf("restore->BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, ithReadRegA(i));
            break;
            
        default:            
            ithWriteRegA( i, NandRegs[(i-ITH_NAND_BASE)/4]);
            //printf("restore->BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, ithReadRegA(i));
            break;
        }
    }
    //printf("NandR2\n");
}

void ithNandDisableClock(void)
{
	unsigned int    StatusReg32,TomeOutCnt=0;
    int i;
    
    //printf("NandS1\n");
	
	//backup NAND registers
    for (i = ITH_NAND_BASE; i < (NF_REG_RD_SCRAMB_CHECK+4); i+=4)
    {
        switch (i)
        {
        case NF_REG_CMD1:
        case NF_REG_CMD2:
        case NF_REG_CMD3:
        case NF_REG_CMD4:
        case NF_REG_CMD_FIRE:
        case NF_REG_STATUS:    
        case NF_REG_BCH_ECC_MAP:
        case NF_REG_ECC_ERROR_MAP:            	
            // don't need to backup 
            //printf("BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, NandRegs[(i-ITH_NAND_BASE)/4]);
            break;
            
        default:            
            //NandRegs[(i-ITH_NAND_BASE)/4] = ithReadRegA(i);
            //printf("BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, NandRegs[(i-ITH_NAND_BASE)/4]);
            break;
        }
    }
    //printf("NandS2\n");
}

void ithNandEnableClock(void)
{
    int i;
    //printf("NandR1\n");
    
    //restore NAND registers
    for (i = ITH_NAND_BASE; i < (NF_REG_RD_SCRAMB_CHECK+4); i+=4)
    {
        switch (i)
        {
        case NF_REG_CMD1:
        case NF_REG_CMD2:
        case NF_REG_CMD3:
        case NF_REG_CMD4:
        case NF_REG_CMD_FIRE:
        case NF_REG_STATUS:    
        case NF_REG_BCH_ECC_MAP:
        case NF_REG_ECC_ERROR_MAP:    
        	// don't need to backup 
        	//printf("restore->BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, ithReadRegA(i));
            break;
            
        default:            
            //ithWriteRegA( i, NandRegs[(i-ITH_NAND_BASE)/4]);
            //printf("restore->BkNfReg[%x][%x]=[%08x]\n",(i-ITH_NAND_BASE)/4, i, ithReadRegA(i));
            break;
        }
    }
    //printf("NandR2\n");
}

