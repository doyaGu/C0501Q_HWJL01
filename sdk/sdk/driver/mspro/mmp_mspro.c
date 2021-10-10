/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  Memory Stick extern API implementation.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"
#include "mspro/ms_type.h"
#include "mspro/ms_hw.h"
#include "mspro/ms_pro.h"
#include "mspro/ms.h"
#include "mspro/ms_common.h"
#if defined(MS_IRQ_ENABLE)
#include "ite/ith.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static MMP_INT MS_ContextInitialize(void);
static MMP_INT MS_DeviceInitialize(void);

//=============================================================================
//                              Global Data Definition
//=============================================================================
MEM_STICK_CARD_STRUCT MSCard = {0};
MMP_UINT32 INT_CED, INT_CMDNK, INT_ERR, INT_BREQ;
static MMP_BOOL inited;
static MS_GET_ATTRIB        MS_FUNC_GetAttrib = MMP_NULL;
static MS_READ_MULTISECTOR  MS_FUNC_ReadMultiSector = MMP_NULL;
static MS_WRITE_MULTISECTOR MS_FUNC_WriteMultiSector = MMP_NULL;

MS_READ_PAGEDATA SMSC_ReadPageData = MMP_NULL;
MS_WRITE_PAGEDATA SMSC_WritePageData = MMP_NULL;


#if defined(MS_IRQ_ENABLE)
MMP_EVENT ms_isr_event = MMP_NULL;
extern MMP_BOOL card_removed;
#endif

#if defined(MS_DMA_IRQ_ENABLE)
MMP_EVENT ms_dma_event = MMP_NULL;
#endif


//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Memory Sitck (pro) Initialization.
 */
//=============================================================================
MMP_INT iteMsproInitialize(void)
{
    MMP_INT result = 0;
    MMP_BOOL lockMutex = MMP_FALSE;
    LOG_ENTER "[iteMsproInitialize] Enter \n" LOG_END

    if(inited)
    {
        #if 0
        return result;
        #else
        iteMsproTerminate();
        #endif
    }

    memset((void*)&MSCard, 0x0, sizeof(MSCard));
    MSCard.dmaCh = -1;

    /**
     * Check card is inserted.
     */
    if(!iteMsproGetCardState(MS_INSERTED))
    {
        result = ERROR_MS_NO_CARD_INSERTED;
        goto end;
    }

	ithLockMutex(ithStorMutex);
	lockMutex = MMP_TRUE;
    MS_PowerOnReg();

    /**
     * Initialize hardware
     */
    result = MS_ResetReg();
    if(result)
        goto end;

    MS_SetSerialClkDivideReg();

    /**
     * Context initialize.
     */
    result = MS_ContextInitialize();
    if(result)
        goto end;
    result = MS_DeviceInitialize();
        if(result)
            goto end;

    /**
     * Get MS card attribute.
     */
    result = MS_FUNC_GetAttrib(&MSCard.attrib);
    if(result)
        goto end;
    MSCard.flags |= MS_FLAGS_ATTRIB_READY;

    #if defined(MS_WORKAROUND)
    result = MSC_WrStart();
    if(result)
        goto end;
    #endif

end:
    if(lockMutex)
        ithUnlockMutex(ithStorMutex);
    LOG_LEAVE "[iteMsproInitialize] Leave \n" LOG_END
    if(result)
    {
        //while(1); // test
        iteMsproTerminate();
        LOG_ERROR "iteMsproInitialize() return error code 0x%08X \n", result LOG_END
    }
    else
        inited = MMP_TRUE;

    return result;
}


//=============================================================================
/**
 * Memory Sitck (pro) Terminate.
 */
//=============================================================================
MMP_INT
iteMsproTerminate(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[mmpMsTerminate] Enter \n" LOG_END

    result = MS_ResetReg();
    if(result)
    {
        //goto end;
    }

    #if defined(MS_IRQ_ENABLE)
	ithIntrDisableIrq(ITH_INTR_MS);
	if(ms_isr_event)
	{
		SYS_DelEvent(ms_isr_event);
		ms_isr_event = MMP_NULL;
	}
	//result = mmpIntrDisableIrq(MSCard.irqGpio, MMP_FALSE); // card insert
	#endif
    #if defined(MS_DMA_IRQ_ENABLE)
    if(!ms_dma_event)
    {
		SYS_DelEvent(ms_dma_event);
        ms_dma_event = MMP_NULL;
	}
	#endif

    if(MSCard.dmaCh >= 0)
    {
        ithDmaFreeCh(MSCard.dmaCh);
        MSCard.dmaCh = -1;
    }
    
    if(MSCard.Media_type.ms_device_type == MS_CARD)
    {
        if(MSCard.tmpAddr)
        {
            SYS_Free((void*)MSCard.tmpAddr);
            MSCard.tmpAddr = MMP_NULL;
        }
    }
	
	ithLockMutex(ithStorMutex);
	MS_PowerDownReg();
	ithUnlockMutex(ithStorMutex);

//end:
    inited = MMP_FALSE;

    LOG_LEAVE "[mmpMsTerminate] Leave \n" LOG_END
    if(result)
        LOG_ERROR "mmpMsTerminate() return error code 0x%08X \n", result LOG_END

	return result;
}

//=============================================================================
/**
 * Memory Sitck (pro) read multisector function.
 */
//=============================================================================
MMP_INT
iteMsproReadMultipleSector(MMP_UINT32 sector, MMP_INT count, void* data)
{
    MMP_INT result = 0;
	MMP_UINT i = 0;
    MMP_UINT32 tmpAddr = 0;
    LOG_ENTER "[iteMsproReadMultipleSector] Enter sector = %d, count = %d, dataAddr = 0x%08X \n", sector, count, data LOG_END

    ithLockMutex(ithStorMutex);
    ithStorageSelect(MS_IO_MODE);

    if(!iteMsproGetCardState(MS_INSERTED))
    {
        result = ERROR_MS_NO_CARD_INSERTED;
        goto end;
    }

    if(MSCard.flags & MS_FLAGS_READ_PROTECT)
    {
        result = ERROR_MS_IS_READ_PROTECT;
        goto end;
    }

    #if defined(MS_WORKAROUND)
    result = MSC_WrEnd();
    if(result)
        goto end;
    #endif

#if defined(MS_WIN32_DMA)
    /* win32 with dma */
    if((MSCard.flags & MS_FLAGS_DMA_ENABLE) && (MSCard.dmaCh >= 0))
    {
        tmpAddr = itpVmemAlloc(MS_PAGE_SIZE*count);
        if(!tmpAddr)
        {
            result = ERROR_MS_ALLOC_TMP_VRAM_FAIL;
            goto end;
        }
        result = MS_FUNC_ReadMultiSector(sector, (MMP_UINT16)count, (MMP_UINT8*)tmpAddr);
        if(result)
            goto end;

        ithReadVram(data, tmpAddr, MS_PAGE_SIZE*count);
    }
#else
    /* pio mode for arm/risc/win32 */
    if(((uint32_t)data & 0x3) && (MSCard.dmaCh < 0))
    {
        uint32_t tmpAddr2;

        LOG_WARNING " iteMsproReadMultipleSector() input data address doesn't dword align! addr = 0x%08X \n", data LOG_END
        tmpAddr = (uint32_t)malloc(MS_PAGE_SIZE*count + 4);
        if(!tmpAddr)
        {
            result = ERROR_MS_ALLOC_TMP_SYSTEM_MEMORY_FAIL;
            goto end;
        }
        tmpAddr2 = ((tmpAddr + 3) & ~3);
        result = MS_FUNC_ReadMultiSector(sector, (MMP_UINT16)count, (MMP_UINT8*)tmpAddr2);
        if(result)
            goto end;

        memcpy(data, (void*)tmpAddr2, MS_PAGE_SIZE*count);
    }
#endif
    else
    {
        result = MS_FUNC_ReadMultiSector(sector, (MMP_UINT16)count, data);
        if(result)
            goto end;
    }

    #if defined(MS_WORKAROUND)
    result = MSC_WrStart();
    if(result)
        goto end;
    #endif

end:
    ithUnlockMutex(ithStorMutex);
    if(tmpAddr)
    {
        #if defined(MS_WIN32_DMA)
        itpVmemFree(tmpAddr);
        #else
        free((void*)tmpAddr);        
        #endif
    }
    LOG_LEAVE "[iteMsproReadMultipleSector] Leave \n" LOG_END
    if(result)
        LOG_ERROR "iteMsproReadMultipleSector() return error code 0x%08X \n", result LOG_END

	return result;
}

//=============================================================================
/**
 * Memory Sitck (pro) read multisector function.
 */
//=============================================================================
MMP_INT
iteMsproWriteMultipleSector(MMP_UINT32 sector, MMP_INT count, void* data)
{
    MMP_INT result = 0;
	MMP_UINT i = 0;
    MMP_UINT32 tmpAddr = 0;
    LOG_ENTER "[iteMsproWriteMultipleSector] Enter sector = %d, count = %d, dataAddr = 0x%08X \n", sector, count, data LOG_END

    ithLockMutex(ithStorMutex);
    ithStorageSelect(MS_IO_MODE);

    if(!iteMsproGetCardState(MS_INSERTED))
    {
        result = ERROR_MS_NO_CARD_INSERTED;
        goto end;
    }

    if(MSCard.flags & MS_FLAGS_WRITE_PROTECT)
    {
        result = ERROR_MS_IS_WRITE_PROTECT;
        goto end;
    }

    #if defined(MS_WORKAROUND)
    result = MSC_WrEnd();
    if(result)
        goto end;
    #endif

#if defined(MS_WIN32_DMA)
    /* win32 with dma */
    if((MSCard.flags & MS_FLAGS_DMA_ENABLE) && (MSCard.dmaCh >= 0))
    {
        tmpAddr = itpVmemAlloc(MS_PAGE_SIZE*count);
        if(!tmpAddr)
        {
            result = ERROR_MS_ALLOC_TMP_VRAM_FAIL;
            goto end;
        }
        ithWriteVram(tmpAddr, data, MS_PAGE_SIZE*count);
        result = MS_FUNC_WriteMultiSector(sector, (MMP_UINT16)count, (uint8_t*)tmpAddr);
        if(result)
            goto end;
    }
#else
    /* pio mode for arm/risc/win32 */
    if(((uint32_t)data & 0x3) && (MSCard.dmaCh < 0))
    {
        uint32_t tmpAddr2;

        LOG_WARNING " iteMsproWriteMultipleSector() input data address doesn't dword align! addr = 0x%08X \n", data LOG_END
        tmpAddr = (uint32_t)malloc(MS_PAGE_SIZE*count + 4);
        if(!tmpAddr)
        {
            result = ERROR_MS_ALLOC_TMP_SYSTEM_MEMORY_FAIL;
            goto end;
        }
        tmpAddr2 = ((tmpAddr + 3) & ~3);
        memcpy((void*)tmpAddr2, data, MS_PAGE_SIZE*count);
        
        result = MS_FUNC_WriteMultiSector(sector, (MMP_UINT16)count, (uint8_t*)tmpAddr2);
        if(result)
            goto end;
    }
#endif
    else
    {
        result = MS_FUNC_WriteMultiSector(sector, (MMP_UINT16)count, data);
        if(result)
            goto end;
    }

    #if defined(MS_WORKAROUND)
    result = MSC_WrStart();
    if(result)
        goto end;
    #endif

end:
    ithUnlockMutex(ithStorMutex);
    if(tmpAddr)
    {
        #if defined(MS_WIN32_DMA)
        itpVmemFree(tmpAddr);
        #else
        free((void*)tmpAddr);        
        #endif
    }
    LOG_LEAVE "[iteMsproWriteMultipleSector] Leave \n" LOG_END
    if(result)
        LOG_ERROR "iteMsproWriteMultipleSector() return error code 0x%08X \n", result LOG_END

	return result;
}


//=============================================================================
/**
 * Get Memory Sitck Attribute.
 */
//=============================================================================
MMP_INT
iteMsproGetAttrib(MS_PRO_CARD_ATTRIB* attrib)
{
    MMP_INT result = 0;
    LOG_ENTER "[iteMsproGetAttrib] Enter \n" LOG_END

    if(!iteMsproGetCardState(MS_INSERTED))
    {
        result = ERROR_MS_NO_CARD_INSERTED;
        goto end;
    }

    if(!(MSCard.flags & MS_FLAGS_ATTRIB_READY))
    {
        ithLockMutex(ithStorMutex);
        ithStorageSelect(MS_IO_MODE);
        result = MS_FUNC_GetAttrib(&MSCard.attrib);
        if(result)
            goto end;
        MSCard.flags |= MS_FLAGS_ATTRIB_READY;
        ithUnlockMutex(ithStorMutex);
    }
    memcpy((void*)attrib, (void*)&MSCard.attrib, sizeof(MS_PRO_CARD_ATTRIB));

end:
    LOG_LEAVE "[iteMsproGetAttrib] Leave \n" LOG_END
    if(result)
        LOG_ERROR "iteMsproGetAttrib() return error code 0x%08X \n", result LOG_END

	return result;
}


//=============================================================================
/**
 * Memory Sitck card status API.
 */
//=============================================================================
//#define REG_BIT_MS_CARD_DETECT         (1u << 20)
    
MMP_BOOL
iteMsproGetCardState(MS_CARD_STATE state)
{
    MMP_UINT32 data = 0;
    MMP_BOOL result = MMP_FALSE;
    LOG_ENTER "[mmpMsGetCardState] Enter \n" LOG_END

    switch(state)
    {
        case MS_INIT_OK:           
            result = inited;
           break;
        case MS_INSERTED:
            result = ithCardInserted(ITH_CARDPIN_MS);
	        break;
	    default:
	        break;
	}    
	    
    LOG_LEAVE "[mmpMsGetCardState] Leave result = %d \n", result LOG_END
	return  result;
}  


//=============================================================================
/**
 * Memory Sitck card status API.
 */
//=============================================================================
MMP_INT
iteMsproGetCapacity(MMP_UINT32* sectorNum,
                    MMP_UINT32* blockLength)
{
    MMP_INT   result = 0;
    LOG_ENTER "[iteMsproGetCapacity] Enter \n" LOG_END

    if(!iteMsproGetCardState(MS_INSERTED))
    {
        result = ERROR_MS_NO_CARD_INSERTED;
        goto end;
    }

    if(!(MSCard.flags & MS_FLAGS_ATTRIB_READY))
    {
        ithLockMutex(ithStorMutex);
        ithStorageSelect(MS_IO_MODE);
        result = MS_FUNC_GetAttrib(&MSCard.attrib);
        if(result)
            goto end;
        MSCard.flags |= MS_FLAGS_ATTRIB_READY;
        ithUnlockMutex(ithStorMutex);
    }
    
    *sectorNum   = MSCard.attrib.numSectors; /** this value is sector numbers, not last block ID. */
    *blockLength = 512;

end:
    LOG_LEAVE "[iteMsproGetCapacity] Leave \n" LOG_END
    if(result)
        LOG_ERROR "iteMsproGetCapacity() return error code 0x%08X \n", result LOG_END

    return result;
}  

//=============================================================================
/**
 * Memory Sitck card Is in write-protected status?
 */
//=============================================================================
MMP_BOOL
iteMsproIsLock(void)
{
    MMP_BOOL   isLock = MMP_FALSE;
    LOG_ENTER "[iteMsproIsLock] Enter \n" LOG_END

    isLock = (MSCard.flags & MS_FLAGS_WRITE_PROTECT) ? MMP_TRUE : MMP_FALSE;

    LOG_LEAVE "[iteMsproIsLock] Leave \n" LOG_END

    return isLock;
}  




//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_INT MS_ContextInitialize(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[MS_ContextInitialize] Enter \n" LOG_END

    /** INT status (serial interface) */
    INT_CED   = SMS_INT_CED_S;
    INT_CMDNK = SMS_INT_CMDNK_S;
    INT_ERR   = SMS_INT_ERR_S;
    INT_BREQ  = SMS_INT_BREQ_S;

    #if 0
    MSCard.chipId = (MMP_UINT16)HOST_CHIP_ID;
    MSCard.chipVersion = (MMP_UINT16)HOST_CHIP_REV;
    #endif

    MSCard.dmaCh = -1;
    #if defined(__OPENRTOS__) || defined(MS_WIN32_DMA)
    MSCard.flags |= MS_FLAGS_DMA_ENABLE;
    LOG_INFO " run with DMA! \n" LOG_END
    #endif

    MSCard.FIFO_Size = MS_GetFifoSizeReg();
#if defined(FPGA)
	MSCard.FIFO_Size = 32;
#endif
    MSCard.FIFO_Width = Qualet_4_Byte;
    LOG_INFO " FIFO Size  = %d \n", MSCard.FIFO_Size LOG_END
    LOG_INFO " FIFO Width = %d \n", MSCard.FIFO_Width LOG_END
    #if defined(MS_WORKAROUND)
    MSCard.sysReg = 0x80; /** default is serial interface */
    #endif

    #if defined(MS_IRQ_ENABLE)
	card_removed = MMP_FALSE;
    #if 0
	{ /** for gpio card detect */
	    extern MMP_UINT32 gpio_card_detect;
		MSCard.irqGpio = GPIO_GetNum(gpio_card_detect) + IRQ_GPIO_SHT;
		result = mmpIntrRequestIrq(MSCard.irqGpio, ms_isr2, (LEVEL_TRIGGER|ACTIVE_HIGH)|(GPIO_SINGLE_EDGE|GPIO_RISING_EDGE|GPIO_BOUNCE_EN), MMP_NULL);
		if(result)
			goto end;
		result = mmpIntrEnableIrq(MSCard.irqGpio, MMP_TRUE);
		if(result)
			goto end;
	}
    #endif
    ithIntrRegisterHandlerIrq(ITH_INTR_MS, ms_isr, (void*)&MSCard);
    ithIntrSetTriggerModeIrq(ITH_INTR_MS, ITH_INTR_LEVEL);
    ithIntrSetTriggerLevelIrq(ITH_INTR_MS, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_MS);
	if(!ms_isr_event)
	{
	    ms_isr_event = SYS_CreateEvent();
		if(!ms_isr_event)
		{
		    result = ERROR_MS_CREATE_ISR_EVENT_FAIL;
			goto end;
		}
	}
    #endif

    result = SMSC_GetMediaType(&MSCard.Media_type);
    if(result)
        goto end;

    if(MSCard.flags & MS_FLAGS_DMA_ENABLE)
    {
        #if defined(MS_DMA_IRQ_ENABLE)
        MSCard.dmaCh = ithDmaRequestCh("ms", ITH_DMA_CH_PRIO_HIGH_3, ms_dma_isr, (void*)&MSCard);
        #else
        MSCard.dmaCh = ithDmaRequestCh("ms", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
        #endif
        if(MSCard.dmaCh < 0)
        {
            result = ERROR_MS_REQUEST_DMA_FAIL;
            goto end;
        }
        ithDmaReset(MSCard.dmaCh);
        ithDmaSetTxSize(MSCard.dmaCh, MS_PAGE_SIZE); /* it's fixed size 512 (page size) */

        #if defined(MS_DMA_IRQ_ENABLE)
	    if(!ms_dma_event)
	    {
	        ms_dma_event = SYS_CreateEvent();
		    if(!ms_dma_event)
		    {
		        result = ERROR_MS_CREATE_ISR_EVENT_FAIL;
			    goto end;
		    }
	    }
        #endif
    }

    if((MSCard.flags & MS_FLAGS_DMA_ENABLE) && (MSCard.dmaCh >= 0))
    {
        SMSC_ReadPageData = SMSC_ReadPageDataDma;
        SMSC_WritePageData = SMSC_WritePageDataDma;
    }
    else
    {
        SMSC_ReadPageData = SMSC_ReadPageDataCpu;
        SMSC_WritePageData = SMSC_WritePageDataCpu;
    }

    /**
     * Assign related function pointer.
     */
    if(MSCard.Media_type.ms_device_type == MS_CARD_PRO)
    {
        MS_FUNC_GetAttrib = MS_PRO_GetAttrib;
        MS_FUNC_ReadMultiSector = MS_PRO_ReadLongData;
        MS_FUNC_WriteMultiSector = MS_PRO_WriteLongData;
    }
    else
    {
        MS_FUNC_GetAttrib = MS_ReadAttrib;
        MS_FUNC_ReadMultiSector = MS_ReadMultiSector;
        MS_FUNC_WriteMultiSector = MS_WriteMultiSectorEx;
    }

end:
    LOG_LEAVE "[MS_ContextInitialize] Leave \n" LOG_END
    if(result)
        LOG_ERROR "MS_ContextInitialize() return error code 0x%08X \n", result LOG_END

	return result;
}


static MMP_INT MS_DeviceInitialize(void)
{
    MMP_INT result = 0;
    LOG_ENTER "[MS_DeviceInitialize] Enter \n" LOG_END

    if(MSCard.Media_type.ms_device_type == MS_CARD_PRO) // MS pro card
    {
        result = MS_Pro_ConfirmCpuStartup();
        if(result)
            goto end;
        result = SMSC_InterfaceSwitch(MS_INTERFACE_PARALLEL);
        if(result)
            goto end;
        LOG_INFO " MS pro card device switch to parallel interface!! \n" LOG_END
    }
    else // MS card
    {
        result = MS_SearchBootBlock();
        if(result)
            goto end;

        result = MS_GetCapacity();
        if(result)
            goto end;

        #if defined(MS_SERIAL_ONLY_DISABLE)
        if(!(MSCard.flags & MS_FLAGS_MS_SUPPORT_PARALLEL))
        {
            result = MS_CARD_DISABLE;
            goto end;
        }
        #endif
                
        if(MSCard.flags & MS_FLAGS_BOOT_AREA_PROTECTION_FLAG)
        {
            result = MS_BootAreaProtection();
            if(result)
                goto end;
        }

        if(MSCard.flags & MS_FLAGS_MS_SUPPORT_PARALLEL)
        {
            result = SMSC_InterfaceSwitch(MS_INTERFACE_PARALLEL);
            if(result)
                goto end;
            LOG_INFO " MS card device switch to parallel interface!! \n" LOG_END
        }

        result = MS_CreateLookupTable();
        if(result)
            goto end;

        #if !defined(MS_STANDARD_WRITE)
        /** Allocate temp buffer for ms write use */
        MSCard.tmpAddr = (MMP_UINT8*)SYS_Malloc(MSCard.pageNum * MS_PAGE_SIZE);
        if(!MSCard.tmpAddr)
        {
            result = ERROR_MS_ALLOC_WRITE_TMP_BUF_FAIL;
            goto end;
        }
        #endif
    }

end:
    LOG_LEAVE "[MS_DeviceInitialize] Leave \n" LOG_END
    if(result)
        LOG_ERROR "MS_DeviceInitialize() return error code 0x%08X \n", result LOG_END

	return result;
}



