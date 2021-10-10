/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  SD/MMC extern API implementation.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "config.h"
#include "sd_hw.h"
#include "sd.h"

#if defined(SD_IRQ_ENABLE)
#include "ite/ith.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
enum
{
    MODE_RW,
    MODE_NON_RW
};


//=============================================================================
//                              Private Function Declaration
//=============================================================================


//=============================================================================
//                              Global Data Definition
//=============================================================================
SD_CTXT g_sdCtxt[SD_NUM] = {0};

//=============================================================================
//                              Private Function Definition
//=============================================================================
static int SD_ContextInitialize(SD_CTXT* ctxt)
{
    int result = 0;
    LOG_ENTER "[SD_ContextInitialize] Enter \n" LOG_END

    #if !defined(WIN32) || defined(SD_WIN32_DMA)
    ctxt->flags |= SD_FLAGS_DMA_ENABLE;
    LOG_INFO " Run with DMA! \n" LOG_END
    #endif

    if(ctxt->flags & SD_FLAGS_DMA_ENABLE)
    {
        #if defined(SD_IRQ_ENABLE)
        ctxt->dmaCh = ithDmaRequestCh("sd", ITH_DMA_CH_PRIO_HIGH_3, dma_isr, (void*)ctxt);
        #else
        ctxt->dmaCh = ithDmaRequestCh("sd", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
        #endif
        if(ctxt->dmaCh < 0)
        {
            result = ERROR_SD_REQUEST_DMA_FAIL;
            goto end;
        }
        ithDmaReset(ctxt->dmaCh);
        ithDmaSetBurst(ctxt->dmaCh, ITH_DMA_BURST_128);
    }

end:
    LOG_LEAVE "[SD_ContextInitialize] Leave \n" LOG_END
    if(result)
        LOG_ERROR "SD_ContextInitialize() return error code 0x%08X \n", result LOG_END

    return result;
}

static inline void SD_SwitchLun(int index, int mode)
{
    SD_CTXT* ctxt = &g_sdCtxt[index];

    if((mode == MODE_RW) && !(ctxt->flags & SD_FLAGS_LAST_LEAVE))
    {
        SD_SetClockDivReg(ctxt->clockDiv);
        SD_SetBusWidthReg(ctxt->cardBusWidth);
    }

    switch(index)
    {
    case SD_0:
		g_sdCtxt[SD_1].flags &= ~SD_FLAGS_LAST_LEAVE;
        g_sdCtxt[SD_0].flags |= SD_FLAGS_LAST_LEAVE;
        break;
    case SD_1:
        g_sdCtxt[SD_0].flags &= ~SD_FLAGS_LAST_LEAVE;
        g_sdCtxt[SD_1].flags |= SD_FLAGS_LAST_LEAVE;
        break;
    }
}

#if defined(SD_IRQ_ENABLE)

extern sem_t* sd_isr_sem;

static inline void intrEnable(SD_CTXT* ctxt)
{
    /** create event for isr */
    if(!sd_isr_sem)
    {
        sd_isr_sem = malloc(sizeof(sem_t));
	    sem_init(sd_isr_sem, 0, 0);
    }

    /** register interrupt handler to interrupt mgr */
    ithIntrRegisterHandlerIrq(ITH_INTR_SD, sd_isr, (void*)g_sdCtxt);
	#if defined(SD_NEW_HW)
    ithIntrSetTriggerModeIrq(ITH_INTR_SD, ITH_INTR_LEVEL);
	#else
    ithIntrSetTriggerModeIrq(ITH_INTR_SD, ITH_INTR_EDGE);
	#endif
    ithIntrSetTriggerLevelIrq(ITH_INTR_SD, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_SD);

    #if defined(SD_NEW_HW)
    ithWriteRegA(SD_REG_INTR, ~(SD_INTR_ALL<<SD_SHT_INTR_MSK)|SD_INTR_ALL);
    SD_WrapIntrEnable();
    #else
	ithWriteRegMaskA(SD_REG_STS1, 0, SD_MSK_INTR_MSK); /** enable sd interrupt, only INTRMSK is writable */
    #endif

}

static inline void intrDisable(void)
{
    ithIntrDisableIrq(ITH_INTR_SD);
    if(sd_isr_sem)
    {
        sem_destroy(sd_isr_sem);
        sd_isr_sem = NULL;

    }
    #if defined(SD_NEW_HW)
    ithWriteRegA(SD_REG_INTR, (SD_INTR_ALL<<SD_SHT_INTR_MSK)|SD_INTR_ALL);
    SD_WrapIntrDisable();
    #endif
}
#else
#define intrEnable(a)
#define intrDisable()
#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9850))
static void *sd_mutex = NULL;
#else
static pthread_mutex_t  sd_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static inline void SD_MutexLock(void)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9850))
    ithLockMutex(sd_mutex);
#else
    pthread_mutex_lock(&sd_mutex);
#endif
}

static inline void SD_MutexUnlock(void)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9850))
    ithUnlockMutex(sd_mutex);
#else
    pthread_mutex_unlock(&sd_mutex);
#endif
}

//=============================================================================
/**
 * SD Initialization.
 */
//=============================================================================
int iteSdInitializeEx(int index)
{
    int  result = 0;
    SD_CTXT* ctxt = &g_sdCtxt[index];
    bool releaseSemaphore = false;
    LOG_ENTER "[%s(%d)] Enter \n", __FUNCTION__, index LOG_END

    if(ctxt->flags & SD_FLAGS_INIT_READY)
        return result;

    memset((void*)ctxt, 0x0, sizeof(SD_CTXT));
    ctxt->dmaCh = -1;

    /** Check card is inserted. */
    if(!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end;
    }

    result = SD_ContextInitialize(ctxt);
    if(result)
        goto end;
    ctxt->index = index;

#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9850))
    sd_mutex = ithStorMutex;
#endif

    SD_MutexLock();
    releaseSemaphore = true;
    SD_SwitchLun(index, MODE_NON_RW);

    SD_PowerOnReg(index);
    intrEnable(ctxt);

    result = SDMMC_StartUp(ctxt);
    if(result)
        goto end;

    result = SDMMC_GetCapacity(ctxt);
    if(result)
        goto end;

    result = SDMMC_TransferState(ctxt);
    if(result)
        goto end;

    result = SDMMC_Switch(ctxt);
    if(result)
        goto end;

    if(SD_IsCardLocked(index))
        ctxt->flags |= SD_FLAGS_WRITE_PROTECT;

    ctxt->flags |= SD_FLAGS_INIT_READY;

end:
    if(releaseSemaphore)
        SD_MutexUnlock();

    LOG_LEAVE "[%s] Leave \n", __FUNCTION__ LOG_END
    if(result)
    {
        ctxt->flags |= SD_FLAGS_INIT_FAIL;
        iteSdTerminateEx(index);
        LOG_ERROR "%s(%d) return error code 0x%08X \n", __FUNCTION__, index, result LOG_END
    }

    return result;
}


//=============================================================================
/**
 * SD Terminate.
 */
//=============================================================================
int
iteSdTerminateEx(int index)
{
    int result = 0;
    SD_CTXT* ctxt = &g_sdCtxt[index];
    LOG_ENTER "[%s(%d)] Enter \n", __FUNCTION__, index LOG_END

    if(ctxt->dmaCh >= 0)
    {
        ithDmaFreeCh(ctxt->dmaCh);
        ctxt->dmaCh = -1;
    }
	
    SD_MutexLock();
	
	SD_SwitchLun(index, MODE_NON_RW);
    SD_PowerDownReg(ctxt);

    if(!(g_sdCtxt[!index].flags & SD_FLAGS_INIT_READY)) // for two sd
        intrDisable();
	else
    {
        #if defined(SD_NEW_HW)
        ithWriteRegA(SD_REG_INTR, ~(SD_INTR_ALL<<SD_SHT_INTR_MSK)|SD_INTR_ALL);
        SD_WrapIntrEnable();
        #else
    	ithWriteRegMaskA(SD_REG_STS1, 0, SD_MSK_INTR_MSK); /** enable sd interrupt, only INTRMSK is writable */
        #endif
    }

    SD_MutexUnlock();

    ctxt->flags &= ~SD_FLAGS_INIT_READY;

    LOG_LEAVE "[%s] Leave \n", __FUNCTION__ LOG_END
    if(result)
        LOG_ERROR "%s(%d) return error code 0x%08X \n", __FUNCTION__, index, result LOG_END

    return result;
}

//=============================================================================
/**
 * SD read multisector function.
 */
//=============================================================================
int
iteSdReadMultipleSectorEx(int index, uint32_t sector, int count, void* data)
{
    int result = 0;
    SD_CTXT* ctxt = &g_sdCtxt[index];
    uint32_t i = 0;
    uint32_t tmpAddr = 0;
    LOG_ENTER "[%s(%d)] Enter sector = %d, count = %d, dataAddr = 0x%08X \n", __FUNCTION__, index, sector, count, data LOG_END

    if(!(ctxt->flags & SD_FLAGS_INIT_READY))
    {
        result = ERROR_SD_NO_INIT;
        goto end1;
    }
    if(!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end1;
    }

    SD_MutexLock();
    SD_SwitchLun(index, MODE_RW);
    SD_SelectIo(index);

#if defined(SD_WIN32_DMA)
    /* win32 with dma */
    if((ctxt->flags & SD_FLAGS_DMA_ENABLE) && (ctxt->dmaCh >= 0))
    {
        tmpAddr = itpVmemAlloc(SD_SECTOR_SIZE*count);
        if(!tmpAddr)
        {
            result = ERROR_SD_ALLOC_TMP_VRAM_FAIL;
            goto end;
        }
        result = SDMMC_ReadMultiSector(ctxt, sector, (uint32_t)count, (void*)tmpAddr);
        if(result)
            goto end;

        ithReadVram(data, tmpAddr, SD_SECTOR_SIZE*count);
    }
#else
    /* pio mode for arm/risc/win32 */
    if(((uint32_t)data & 0x3) && (ctxt->dmaCh < 0))
    {
        uint32_t tmpAddr2;

        LOG_WARNING " iteSdReadMultipleSector() input data address doesn't dword align! addr = 0x%08X \n", data LOG_END
        tmpAddr = (uint32_t)malloc(SD_SECTOR_SIZE*count + 4);
        if(!tmpAddr)
        {
            result = ERROR_SD_ALLOC_TMP_SYSTEM_MEMORY_FAIL;
            goto end;
        }
        tmpAddr2 = ((tmpAddr + 3) & ~3);
        result = SDMMC_ReadMultiSector(ctxt, sector, (uint32_t)count, (void*)tmpAddr2);
        if(result)
            goto end;

        memcpy(data, (void*)tmpAddr2, SD_SECTOR_SIZE*count);
    }
#endif
    else
    {
        result = SDMMC_ReadMultiSector(ctxt, sector, (uint32_t)count, data);
        if(result)
            goto end;
    }

#if 0
	if((sector==0) && (count==1))
	{
		uint32_t i = 0;
		uint8_t* status = data;
		for(i=0; i<512; i+=16)
		{
			LOG_DATA " Byte %04X: %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X \n",
				i, status[i+0], status[i+1], status[i+2], status[i+3], status[i+4], status[i+5], status[i+6], status[i+7]
			, status[i+8], status[i+9], status[i+10], status[i+11], status[i+12], status[i+13], status[i+14], status[i+15] LOG_END
		}
	}
#endif

end:
    SD_MutexUnlock();
    if(tmpAddr)
    {
        #if defined(SD_WIN32_DMA)
        itpVmemFree(tmpAddr);
        #else
        free((void*)tmpAddr);        
        #endif
    }
end1:
    LOG_LEAVE "[%s] Leave \n", __FUNCTION__ LOG_END
    if(result)
        LOG_ERROR "%s(%d, %d, %d) return error code 0x%08X \n", __FUNCTION__, index, sector, count, result LOG_END

    return result;
}

//=============================================================================
/**
 * SD read multisector function.
 */
//=============================================================================
int
iteSdWriteMultipleSectorEx(int index, uint32_t sector, int count, void* data)
{
    int result = 0;
    SD_CTXT* ctxt = &g_sdCtxt[index];
    uint32_t i = 0;
    uint32_t tmpAddr = 0;
    LOG_ENTER "[%s(%d)] Enter sector = %d, count = %d, dataAddr = 0x%08X \n", __FUNCTION__, index, sector, count, data LOG_END
	//printf(" W(%d,%d) \n", sector, count);

    if(!(ctxt->flags & SD_FLAGS_INIT_READY))
    {
        result = ERROR_SD_NO_INIT;
        goto end1;
    }
    if(!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end1;
    }
    if(ctxt->flags & SD_FLAGS_WRITE_PROTECT)
    {
        result = ERROR_SD_IS_WRITE_PROTECT;
        goto end1;
    }

    SD_MutexLock();
    SD_SwitchLun(index, MODE_RW);
    SD_SelectIo(index);

#if defined(SD_WIN32_DMA)
    /* win32 with dma */
    if((ctxt->flags & SD_FLAGS_DMA_ENABLE) && (ctxt->dmaCh >= 0))
    {
        tmpAddr = itpVmemAlloc(SD_SECTOR_SIZE*count);
        if(!tmpAddr)
        {
            result = ERROR_SD_ALLOC_TMP_VRAM_FAIL;
            goto end;
        }
        ithWriteVram(tmpAddr, data, SD_SECTOR_SIZE*count);
        result = SDMMC_WriteMultiSector(ctxt, sector, (uint32_t)count, (void*)tmpAddr);
        if(result)
            goto end;
    }
#else
    /* pio mode for arm/risc/win32 */
    if(((uint32_t)data & 0x3) && (ctxt->dmaCh < 0))
    {
        uint32_t tmpAddr2;

        LOG_WARNING " iteSdWriteMultipleSector() input data address doesn't dword align! addr = 0x%08X \n", data LOG_END
        tmpAddr = (uint32_t)malloc(SD_SECTOR_SIZE*count + 4);
        if(!tmpAddr)
        {
            result = ERROR_SD_ALLOC_TMP_SYSTEM_MEMORY_FAIL;
            goto end;
        }
        tmpAddr2 = ((tmpAddr + 3) & ~3);
        memcpy((void*)tmpAddr2, data, SD_SECTOR_SIZE*count);
        
        result = SDMMC_WriteMultiSector(ctxt, sector, (uint32_t)count, (void*)tmpAddr2);
        if(result)
            goto end;
    }
#endif
    else
    {
        result = SDMMC_WriteMultiSector(ctxt, sector, (uint32_t)count, data);
        if(result)
            goto end;
    }

end:
    SD_MutexUnlock();
    if(tmpAddr)
    {
        #if defined(SD_WIN32_DMA)
        itpVmemFree(tmpAddr);
        #else
        free((void*)tmpAddr);        
        #endif
    }
end1:
    LOG_LEAVE "[%s] Leave \n", __FUNCTION__ LOG_END
    if(result)
        LOG_ERROR "%s(%d, %d, %d) return error code 0x%08X \n", __FUNCTION__, index, sector, count, result LOG_END

    return result;
}


//=============================================================================
/**
 * SD card status API.
 */
//=============================================================================
int
iteSdGetCardStateEx(int index, int state)
{
    int result = 0;
    LOG_ENTER "[%s] Enter \n", __FUNCTION__ LOG_END

    switch(state)
    {
    case SD_INIT_OK:           
        result = (g_sdCtxt[index].flags & SD_FLAGS_INIT_READY);
        break;
    case SD_INSERTED:
        result = SD_IsCardInserted(index);
        break;
    default:
        break;
    }    
        
    LOG_LEAVE "[%s] Leave result = %d \n", __FUNCTION__, result LOG_END

    return  result;
}  


//=============================================================================
/**
 * SD card capacity API.
 */
//=============================================================================
int
iteSdGetCapacityEx(int index,
                 uint32_t* sectorNum,
                 uint32_t* blockLength)
{
    int   result = 0;
    SD_CTXT* ctxt = &g_sdCtxt[index];
    LOG_ENTER "[%s(%d)] Enter \n", __FUNCTION__, index LOG_END

    if(!(ctxt->flags & SD_FLAGS_INIT_READY))
    {
        result = ERROR_SD_NO_INIT;
        goto end;
    }
    if(!SD_IsCardInserted(index))
    {
        result = ERROR_SD_NO_CARD_INSERTED;
        goto end;
    }
    
    *sectorNum   = ctxt->totalSectors; /** this value is sector numbers, not last block ID. */
    *blockLength = SD_SECTOR_SIZE;

end:
    LOG_LEAVE "[%s(%d)] Leave \n", __FUNCTION__, index LOG_END
    if(result)
        LOG_ERROR "%s(%d) return error code 0x%08X \n", __FUNCTION__, index, result LOG_END

    return result;
}  

//=============================================================================
/**
 * SD card Is in write-protected status?
 */
//=============================================================================
bool
iteSdIsLockEx(int index)
{
    bool   isLock = 0;
    SD_CTXT* ctxt = &g_sdCtxt[index];
    LOG_ENTER "[%s(%d)] Enter \n", __FUNCTION__, index LOG_END

    isLock = (ctxt->flags & SD_FLAGS_WRITE_PROTECT);

    LOG_LEAVE "[%s(%d)] Leave \n", __FUNCTION__, index LOG_END

    return isLock;
}  

/** Get CID information */
void
iteSdGetCidEx(int index, uint8_t *buf)
{
    SD_CTXT* ctxt = &g_sdCtxt[index];
    
    if(!(ctxt->flags & SD_FLAGS_INIT_READY))
        return;

    memcpy((void*)buf, ctxt->cid, sizeof(ctxt->cid));
}


