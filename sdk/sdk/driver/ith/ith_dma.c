/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL DMA functions.
 *
 * @author Irene Lin
 * @version 1.0
 */
#if defined(CFG_AXI_DMA)

#include "ith_dma_axi.c"

#else

#include <string.h>
#include "ith_cfg.h"


static void* dmaMutex;
ITHDma dmaCtxt[ITH_DMA_CH_NUM];
ITHDmaWidth dmaWidthMap[4] = {ITH_DMA_WIDTH_32, ITH_DMA_WIDTH_8, ITH_DMA_WIDTH_16, ITH_DMA_WIDTH_8};


#if defined(DMA_IRQ_ENABLE)

static void DmaIntrHandler(void* data)
{
    ITHDma* ctxt = (ITHDma*)data;
    uint32_t tc, err_abt, intrStatus;
    uint32_t channel;
    uint32_t need_flush=0;
    
    tc = ithReadRegA(ITH_DMA_BASE + ITH_DMA_INT_TC_REG);
    err_abt = ithReadRegA(ITH_DMA_BASE + ITH_DMA_INT_ERRABT_REG);
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_INT_TC_CLR_REG, tc);
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_INT_ERRABT_CLR_REG, err_abt);
        
    for(channel=0; channel<ITH_DMA_CH_NUM; channel++)
    {
        if(((tc>>channel) & 0x1) || (((err_abt>>channel) & 0x1)))
        {
            intrStatus = 0;
            if((tc>>channel) & 0x1)
                intrStatus |= ITH_DMA_INTS_TC;
            if((err_abt>>channel) & 0x1)
                intrStatus |= ITH_DMA_INTS_ERR;
            if((err_abt>>(16+channel)) & 0x1)
                intrStatus |= ITH_DMA_INTS_ABT;

            if(ctxt[channel].isr)
            {
                ctxt[channel].isr(channel, ctxt[channel].data , intrStatus);
            }

            if(dmaCtxt[channel].dstHw == ITH_DMA_NORMAL_MODE)
                need_flush = 1;
        }
    }

    if(need_flush)
        ithFlushAhbWrap();

    if(err_abt)
        ithPrintf(" dma error 0x%08X \n\n", err_abt);

    return;
}

#endif


void ithDmaInit(void* mutex)
{
    uint32_t tmp = ithReadRegA(ITH_DMA_BASE + ITH_DMA_FEATURE_REG);
    tmp = (uint8_t)((tmp & ITH_DMA_MAX_CHNO_N_MASK) >> ITH_DMA_MAX_CHNO_N_BIT); // channel number

    ASSERT(mutex);
    dmaMutex = mutex;

    #if defined(DMA_IRQ_ENABLE)
    /** clear interrupt status */
    tmp = ithReadRegA(ITH_DMA_BASE + ITH_DMA_INT_TC_REG);
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_INT_TC_CLR_REG, tmp);
    tmp = ithReadRegA(ITH_DMA_BASE + ITH_DMA_INT_ERRABT_REG);
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_INT_ERRABT_CLR_REG, tmp);

    /** register interrupt handler to interrupt mgr */
    ithIntrRegisterHandlerIrq(ITH_INTR_DMA, DmaIntrHandler, dmaCtxt);
    ithIntrSetTriggerModeIrq(ITH_INTR_DMA, ITH_INTR_LEVEL);
    ithIntrSetTriggerLevelIrq(ITH_INTR_DMA, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_DMA);
    #endif

    /** open channel synchronization logic */
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_SYNC_REG, 0xFF);

    /** enable dma controller */
    ithSetRegBitA(ITH_DMA_BASE + ITH_DMA_CSR_REG, ITH_DMA_EN_BIT);
}

int ithDmaRequestCh(char* name, ITHDmaPriority priority, ITHDmaIntrHandler isr, void* arg)
{
    int i, found = 0;
    ASSERT(name);

    ithLockMutex(dmaMutex);

    for(i=0; i<ITH_DMA_CH_NUM; i++)
    {
        if(!dmaCtxt[i].name)
        {
            found = 1;
            break;
        }
    }

    if(found)
    {
        ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_CFG_REG+i*ITH_DMA_CH_OFFSET), ITH_DMA_INT_MASK);  // mask interrupt
        dmaCtxt[i].name = name;
        dmaCtxt[i].isr = isr;
        dmaCtxt[i].data = arg;
        dmaCtxt[i].priority = priority;
        dmaCtxt[i].fifoThreshold = ITH_DMA_FF_TH_8;
    }
    else
    {
        LOG_ERR " request dma channel fail! name:%s \r\n", name LOG_END
        i = -1;
    }

    ithUnlockMutex(dmaMutex);

    return i;
}

void ithDmaFreeCh(int ch)
{
    ASSERT((ch>=0) && (ch<ITH_DMA_CH_NUM));

    if(!dmaCtxt[ch].name)
    {
        LOG_ERR " trying to free channel %d which is already freed. \r\n", ch LOG_END
        return;
    }

    ithLockMutex(dmaMutex);
    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_CFG_REG+ch*ITH_DMA_CH_OFFSET), ITH_DMA_INT_MASK);  // mask interrupt
    dmaCtxt[ch].name = NULL;
    dmaCtxt[ch].isr  = NULL;
    dmaCtxt[ch].data = NULL;
    ithUnlockMutex(dmaMutex);
}

void ithDmaReset(int ch)
{
    ASSERT((ch>=0) && (ch<ITH_DMA_CH_NUM));
    memset((void*)&dmaCtxt[ch].burst, 0x0, sizeof(ITHDma)-17);
}

void ithDmaStart(int ch)
{
    static const uint32_t dmaSizeDiv[3] = {1, 2, 4};
    uint32_t value;

#if defined(CFG_CPU_WB)
    if(dmaCtxt[ch].srcHw == ITH_DMA_NORMAL_MODE)
        ithFlushDCacheRange((void*)dmaCtxt[ch].srcAddr, dmaCtxt[ch].txSize);
    if(dmaCtxt[ch].dstHw == ITH_DMA_NORMAL_MODE)
        ithFlushDCacheRange((void*)dmaCtxt[ch].dstAddr, dmaCtxt[ch].txSize);
#endif

    if((dmaCtxt[ch].srcHw == ITH_DMA_NORMAL_MODE) || (dmaCtxt[ch].dstHw == ITH_DMA_NORMAL_MODE))
        ithFlushMemBuffer();

    // transfer size with src width unit
    value = dmaCtxt[ch].txSize / dmaSizeDiv[dmaCtxt[ch].srcWidth];
    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_TX_SIZE_REG+ch*ITH_DMA_CH_OFFSET), value);

    // csr
    value =((dmaCtxt[ch].fifoThreshold << ITH_DMA_FF_TH_BIT) |
            (dmaCtxt[ch].priority << ITH_DMA_CH_PRIO_BIT) |
            (dmaCtxt[ch].burst << ITH_DMA_BURST_SIZE_BIT) |
            (dmaCtxt[ch].srcWidth << ITH_DMA_SRC_WIDTH_BIT) |
            (dmaCtxt[ch].dstWidth << ITH_DMA_DST_WIDTH_BIT) |
            (dmaCtxt[ch].hw << ITH_DMA_MODE_BIT) |
            (dmaCtxt[ch].srcCtrl << ITH_DMA_SRCAD_CTRL_BIT) |
            (dmaCtxt[ch].dstCtrl << ITH_DMA_DSTAD_CTRL_BIT) |
            (dmaCtxt[ch].srcSel << ITH_DMA_SRC_SEL_BIT) |
            (dmaCtxt[ch].dstSel << ITH_DMA_DST_SEL_BIT) |
            ITH_DMA_CH_EN_MASK);
    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_CSR_REG+ch*ITH_DMA_CH_OFFSET), value);

    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_LLP_REG+(ch*ITH_DMA_CH_OFFSET)), dmaCtxt[ch].lldAddr);
}

void ithDmaDumpReg(int ch)
{
    uint32_t value;
    value = ithReadRegA(ITH_DMA_BASE+ITH_DMA_C0_CSR_REG+ch*ITH_DMA_CH_OFFSET);
    LOG_ERR " 0x%08X = 0x%08X \n", (ITH_DMA_BASE+ITH_DMA_C0_CSR_REG+ch*ITH_DMA_CH_OFFSET), value LOG_END
    value = ithReadRegA(ITH_DMA_BASE+ITH_DMA_C0_CFG_REG+ch*ITH_DMA_CH_OFFSET);
    LOG_ERR " 0x%08X = 0x%08X \n", (ITH_DMA_BASE+ITH_DMA_C0_CFG_REG+ch*ITH_DMA_CH_OFFSET), value LOG_END
    value = ithReadRegA(ITH_DMA_BASE+ITH_DMA_C0_SRCADR_REG+ch*ITH_DMA_CH_OFFSET);
    LOG_ERR " 0x%08X = 0x%08X \n", (ITH_DMA_BASE+ITH_DMA_C0_SRCADR_REG+ch*ITH_DMA_CH_OFFSET), value LOG_END
    value = ithReadRegA(ITH_DMA_BASE+ITH_DMA_C0_DSTADR_REG+ch*ITH_DMA_CH_OFFSET);
    LOG_ERR " 0x%08X = 0x%08X \n", (ITH_DMA_BASE+ITH_DMA_C0_DSTADR_REG+ch*ITH_DMA_CH_OFFSET), value LOG_END
    value = ithReadRegA(ITH_DMA_BASE+ITH_DMA_C0_TX_SIZE_REG+ch*ITH_DMA_CH_OFFSET);
    LOG_ERR " 0x%08X = 0x%08X \n", (ITH_DMA_BASE+ITH_DMA_C0_TX_SIZE_REG+ch*ITH_DMA_CH_OFFSET), value LOG_END
}

#endif // #if defined(CFG_AXI_DMA)

