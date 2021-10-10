/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *    ms_hw.c Memory Stick Engine basic hardware setting
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"
#include "mspro/ms_reg.h"
#include "mspro/ms_type.h"
#include "mspro/ms_hw.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

extern MEM_STICK_CARD_STRUCT MSCard;

//=============================================================================
//                              Public Function Definition
//=============================================================================

static MMP_INLINE void MS_PowerReset(MMP_INT delay_ms) 
{
    ithStorageUnSelect(MS_IO_MODE);
    ithCardPowerOff(ITH_CARDPIN_MS);
    usleep(delay_ms*1000);
    ithCardPowerOn(ITH_CARDPIN_MS);
}

//=============================================================================
/**
 * MS_PowerOnReg()
 * 1. Select MS module
 */
//=============================================================================
void MS_PowerOnReg(void)
{
    ithSetRegBitH(ITH_APB_CLK3_REG, ITH_EN_W9CLK_BIT);  /* enable clock for Memory Stick */
    MS_PowerReset(15); 
    ithStorageSelect(MS_IO_MODE);
    MMP_Sleep(10); /** for power on, may try... */
}

//=============================================================================
/**
 * MS_PowerDownReg()
 */
//=============================================================================
void MS_PowerDownReg(void)
{
    MS_PowerReset(15);
    ithClearRegBitH(ITH_APB_CLK3_REG, ITH_EN_W9CLK_BIT);  /* disable clock for Memory Stick */
}

//=============================================================================
/**
 * MS_ResetReg()
 */
//=============================================================================
MMP_INT MS_ResetReg(void)
{
    MMP_INT    result = 0;
    MMP_UINT16 timeOut = 0;
    MMP_UINT32 reg = 0;
    //MMP_UINT32 mask = MS_MSK_FIFO_CLR|MS_MSK_SW_RESET; // Irene ??
    MMP_UINT32 mask = MS_MSK_SW_RESET;

	/** Clear all interrupt */
    //AHB_WriteRegister(MS_REG_INT_STATUS, MS_MSK_INT_CLR_ALL);

    /** Reset engine and clear FIFO pointer */
    AHB_WriteRegisterMask(MS_REG_CONTROL, mask, mask);

    AHB_ReadRegister(MS_REG_CONTROL, &reg);
    while(reg & mask)
    {
        MMP_Sleep(1);
        if(++timeOut > MS_TIMEOUT)
        {
            result = ERROR_MS_SW_RESET_FAIL;
            goto end;
        }
        AHB_ReadRegister(MS_REG_CONTROL, &reg);
    }

end:
    if(result)
        LOG_ERROR "MS_ResetReg() return error code 0x%08X, reg = 0x%08X \n", result, reg LOG_END
    if(timeOut)
        LOG_INFO "MS_ResetReg() timeOut = %d \n", timeOut LOG_END

    return result;
}

//=============================================================================
/** 0x0008h
 * Clear interrup register
 */
//=============================================================================
void MS_ClearInterruptReg(
    MMP_UINT32 interrupt)
{
    AHB_WriteRegister(MS_REG_INT_STATUS, interrupt);
}

//=============================================================================
/** 0x0014h
 * Wait FIFO full.
 */
//=============================================================================
MMP_INT MS_WaitFifoFullReg(void)
{
    MMP_INT    result = 0;
    MMP_UINT32 timeOut = 0;
    MMP_UINT32 status = 0;

    AHB_ReadRegister(MS_REG_STATUS, &status);
    while(!(status & MS_MSK_FIFO_FULL))
    {
        if(++timeOut > MS_BUSY_TIMEOUT)
        {
            result = ERROR_MS_WAIT_FIFO_FULL_TIMEOUT;
            goto end;
        }
        //ithDelay(MS_BUSY_DELAY);
        AHB_ReadRegister(MS_REG_STATUS, &status);
    }

end:
    if(result)
        LOG_ERROR "MS_WaitFifoFullReg() return error code 0x%08X, 0X14 = 0x%08X \n", result, status LOG_END

    return result;
}

//=============================================================================
/** 0x0014h
 * Wait FIFO Empty.
 */
//=============================================================================
MMP_INT MS_WaitFifoEmptyReg(void)
{
    MMP_INT    result = 0;
    MMP_UINT32 timeOut = 0;
    MMP_UINT32 status = 0;

    AHB_ReadRegister(MS_REG_STATUS, &status);
    while(!(status & MS_MSK_FIFO_EMPTY))
    {
        if(++timeOut > MS_BUSY_TIMEOUT)
        {
            result = ERROR_MS_WAIT_FIFO_EMPTY_TIMEOUT;
            goto end;
        }
        //ithDelay(MS_BUSY_DELAY);
        AHB_ReadRegister(MS_REG_STATUS, &status);
    }

end:
    if(result)
        LOG_ERROR "MS_WaitFifoEmptyReg() return error code 0x%08X, 0X14 = 0x%08X \n", result, status LOG_END

    return result;
}

//=============================================================================
/** 0x0018h
 * Set serial clock divide register.
 */
//=============================================================================
void MS_SetSerialClkDivideReg(void)
{
	if(MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE)
	{
		AHB_WriteRegister(MS_REG_SERIAL_CLK_DIVIDE, MS_PARALLEL_CLK_DIV);
	}
	else
	{
		AHB_WriteRegister(MS_REG_SERIAL_CLK_DIVIDE, MS_SERIAL_CLK_DIV);
	}
}

//=============================================================================
/** 0x001Ch
 * Get FIFO depth
 */
//=============================================================================
MMP_UINT32 MS_GetFifoSizeReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &value);
    return (MMP_UINT32)((value & MS_MSK_FIFO_SIZE) >> MS_SHT_FIFO_SIZE);
}

//=============================================================================
/** 0x001Ch
 * Get FIFO width
 */
//=============================================================================
MMP_UINT32 MS_GetFifoWidthReg(void)
{
    MMP_UINT32 value = 0;
    AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &value);
    return (MMP_UINT32)((value & MS_MSK_FIFO_WIDTH) >> MS_SHT_FIFO_WIDTH);
}

//=============================================================================
/** 0x001Ch
 * Wait command complete.
 */
//=============================================================================
#if defined(MS_IRQ_ENABLE)

#include "ite/ith.h"

extern void* ms_isr_event;
static volatile MMP_UINT32 int_status;
MMP_BOOL card_removed = MMP_FALSE;

void ms_isr2(void* data)
{
    card_removed = MMP_TRUE;
	SYS_SetEventFromIsr(ms_isr_event);
	//ithPrintf(" ms card removed....\n");
}

void ms_isr(void* data)
{
    AHB_ReadRegister(MS_REG_INT_STATUS, (MMP_UINT32*)&int_status);
	AHB_WriteRegister(MS_REG_INT_STATUS, (MMP_UINT32)((int_status << 16)&0x00FF0000));
	SYS_SetEventFromIsr(ms_isr_event);
    #if defined(__OPENRTOS__)
    ithYieldFromISR(true);
    #endif
}

MMP_INT MS_WaitCmdCompleteReg(void)
{
    MMP_INT result;
	MMP_UINT32 status;
    result = SYS_WaitEvent(ms_isr_event, 100);
	if(result) /** timeout */
	{
	    result = ERROR_MS_WAIT_CMD_COMPLETE;
		goto end;
	}
	if(card_removed)
	{
	    result = ERROR_MS_NO_CARD_INSERTED;
		goto end;
	}
	if(!(int_status & MS_MSK_INT_CMD_COMPLETE))
	{
		result = ERROR_MS_IRQ_WHAT_HAPPEN;
		goto end;
	}
	AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &status);
    if((status & 0x1C00) != 0x0)
    {
        result = ERROR_MS_CMD_DEVICE_FAIL;
        goto end;
    }
	if(int_status & MS_MSK_INT_2STATE_ERROR)
	{
        result = ERROR_MS_2STATE_ACCESS_MODE;
        LOG_ERROR "MS_WaitCmdCompleteReg() => ERROR_MS_2STATE_ACCESS_MODE \n" LOG_END
	}
	if(int_status & MS_MSK_INT_CRC_ERROR)
	{
        result = ERROR_MS_CRC_ERROR;
        LOG_ERROR "MS_WaitCmdCompleteReg() => ERROR_MS_CRC_ERROR \n" LOG_END
	}

end:
    int_status = 0;
    if(result)
    {
        MMP_UINT32 reg = 0;
        AHB_ReadRegister(MS_REG_STATUS, &reg);
        LOG_ERROR "MS_WaitCmdCompleteReg() return error code 0x%08X, 0x14 = 0x%08X, intStatus = 0x%08X \n", result, reg, int_status LOG_END
    }

    return result;
}

#else // #if defined(MS_IRQ_ENABLE)

MMP_INT MS_WaitCmdCompleteReg(void)
{
    MMP_INT    result = 0;
    MMP_UINT32 timeOut = 0;
    MMP_UINT32 status = 0;

    AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &status);
    while(!(status & MS_MSK_CMD_COMPLETE))
    {
        if(++timeOut > MS_BUSY_TIMEOUT)
        {
            result = ERROR_MS_WAIT_CMD_COMPLETE;
            goto end;
        }
        //ithDelay(MS_BUSY_DELAY);
        AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &status);
    }

    if((status & 0x1C00) != 0x0)
    {
        result = ERROR_MS_CMD_DEVICE_FAIL;
        goto end;
    }

    AHB_ReadRegister(MS_REG_STATUS, &status);
    if(status & MS_MSK_2STATE_ACCESS_MODE)
    {
        result = ERROR_MS_2STATE_ACCESS_MODE;
        LOG_ERROR "MS_WaitCmdCompleteReg() => ERROR_MS_2STATE_ACCESS_MODE \n" LOG_END
    }
    if(status & MS_MSK_CRC_ERROR)
    {
        result = ERROR_MS_CRC_ERROR;
        LOG_ERROR "MS_WaitCmdCompleteReg() => ERROR_MS_CRC_ERROR \n" LOG_END
    }

    MS_ClearInterruptReg(MS_MSK_INT_CLR_CMD_COMPLETE);

end:
    if(result)
    {
        MMP_UINT32 reg = 0;
        AHB_ReadRegister(MS_REG_STATUS, &reg);
        LOG_ERROR "MS_WaitCmdCompleteReg() return error code 0x%08X, 0x14 = 0x%08X, 0x1C = 0x%08X \n", result, reg, status LOG_END
        LOG_INFO "MS_WaitCmdCompleteReg() timeOut = %d \n", timeOut LOG_END
        #if 0
        MMP_Sleep(1000);
        LOG_ERROR " after 1 second ==> \n" LOG_END
        AHB_ReadRegister(MS_REG_STATUS, &reg);
        AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &status);
        LOG_ERROR "0x14 = 0x%08X, 0x1C = 0x%08X \n", reg, status LOG_END
        #endif			
    }

    return result;
}

#endif // #if defined(MS_IRQ_ENABLE)


//=============================================================================
/** 0x001Ch
 * Wait and get INT status for multi-bits.
 */
//=============================================================================
MMP_INT MS_GetIntStatusReg(MMP_UINT32* intStatus, MMP_UINT32 sTimeout)
{
    MMP_INT    result = 0;
    MMP_UINT32 status = 0;
    MMP_UINT32 mask = MS_MSK_INT_CED_P|MS_MSK_INT_ERR_P|MS_MSK_INT_BREQ_P|MS_MSK_INT_CMDNK_P;
    struct timeval startT, endT;

    gettimeofday(&startT, NULL);

    AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &status);
    while(!(status & mask))
    {
        gettimeofday(&endT, NULL);
        if(itpTimevalDiff(&startT, &endT) > (long)sTimeout)
        {
            result = ERROR_MS_WAIT_INT_TIMEOUT_P;
            goto end;
        }
        AHB_ReadRegister(MS_REG_BUS_MONITOR_STATUS, &status);
		#if defined(MS_IRQ_ENABLE)
		if(card_removed)
        {
			result = ERROR_MS_NO_CARD_INSERTED;
            goto end;
        }
        #endif
    }

    (*intStatus) = (status & mask);

end:
    if(result)
    {
        MMP_UINT32 reg = 0;
        AHB_ReadRegister(MS_REG_STATUS, &reg);
        LOG_ERROR "MS_GetIntStatusReg() return error code 0x%08X, 0x14 = 0x%08X, 0x1C = 0x%08X \n", result, reg, status LOG_END
        LOG_INFO "MS_GetIntStatusReg() sTimeout = %d \n", sTimeout LOG_END
    }

    return result;
}


#if defined(MS_DMA_IRQ_ENABLE)

#include "ite/ith.h"
extern void* ms_dma_event;

void ms_dma_isr(int ch, void* arg, MMP_UINT32 status)
{
    if(status & (ITH_DMA_INTS_ERR|ITH_DMA_INTS_ABT))
        ithPrintf(" ms: dma irq error 0x%X \n", status);

	SYS_SetEventFromIsr(ms_dma_event);
}

#endif // #if defined(MS_DMA_IRQ_ENABLE)
