/*
 * Copyright (c) 2015 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL DMA functions.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ith_cfg.h"
#include "ite/itp.h"

static void* dmaMutex;
ITHDma dmaCtxt[ITH_DMA_CH_NUM];
ITHDmaWidth dmaWidthMap[4] = {ITH_DMA_WIDTH_32, ITH_DMA_WIDTH_8, ITH_DMA_WIDTH_16, ITH_DMA_WIDTH_8};


#if defined(DMA_IRQ_ENABLE)

static void DmaIntrHandler(void* data)
{
    ITHDma* ctxt = (ITHDma*)data;
    uint32_t tc, err_abt, intrStatus;
    uint32_t ch;
    uint32_t need_flush=0;
    
    tc = ithReadRegA(ITH_DMA_BASE + ITH_DMA_INT_TC_REG);
    err_abt = ithReadRegA(ITH_DMA_BASE + ITH_DMA_INT_ERRABT_REG);
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_INT_TC_CLR_REG, tc);
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_INT_ERRABT_CLR_REG, err_abt);
        
    for (ch = 0; ch<ITH_DMA_CH_NUM; ch++)
    {
        intrStatus = 0;
        if ((tc >> ch) & 0x1)
            intrStatus |= ITH_DMA_INTS_TC;

        if (err_abt & ITH_DMA_EA_ERR_CH(ch))
            intrStatus |= ITH_DMA_INTS_ERR;

        if (err_abt & ITH_DMA_EA_WDT_CH(ch))
            intrStatus |= ITH_DMA_INTS_WDT;

        if (err_abt & ITH_DMA_EA_ABT_CH(ch))
            intrStatus |= ITH_DMA_INTS_ABT;

        if (ctxt[ch].isr && intrStatus)
            ctxt[ch].isr(ch, ctxt[ch].data, intrStatus);

        if (dmaCtxt[ch].dstHw == ITH_DMA_NORMAL_MODE)
            need_flush = 1;
    }

    //if(need_flush)
    //    ithFlushAhbWrap();

    if(err_abt)
        ithPrintf(" dma error 0x%08X \n\n", err_abt);

    return;
}

#endif


void ithDmaInit(void* mutex)
{
    uint32_t tmp = ithReadRegA(ITH_DMA_BASE + ITH_DMA_FEATURE_REG);

#if 1
    {
        uint8_t cq_depth[4] = { 2, 4, 8, 16 };
        uint8_t ldm_depth[4] = { 32, 64, 128, 0 };
        uint8_t fifo_depth[5] = { 8, 16, 32, 64, 128 };
        uint8_t bus_width[3] = { 32, 64, 128 };
        LOG_INFO "DMA Hardware Feature: \n" LOG_END
        LOG_INFO " command queue depth: %d \n", cq_depth[(tmp >> 28) & 0x3] LOG_END
        LOG_INFO " LDM depth: %d \n", ldm_depth[(tmp >> 24) & 0x3] LOG_END
        LOG_INFO " LDM: %s \n", ((tmp >> 20) & 0x1) ? "ON" : "OFF" LOG_END
        LOG_INFO " Number of the peripheral interfaces: %d \n", (((tmp >> 16) & 0xF) + 1) LOG_END
        LOG_INFO " Peripheral interface exists: %s \n", (((tmp >> 12) & 0x1) ? "Yes" : "No") LOG_END
        LOG_INFO " data FIFO depth: %d \n", (fifo_depth[(tmp >> 8) & 0x7]) LOG_END
        LOG_INFO " AXI data bus width of the AXI slave port: %d \n", (bus_width[(tmp >> 6) & 0x3]) LOG_END
        LOG_INFO " AXI data bus width of the AXI master port: %d \n", (bus_width[(tmp >> 4) & 0x3]) LOG_END
        LOG_INFO " Unalign transfer mode: %s \n", (((tmp >> 3) & 0x1) ? "ON" : "OFF") LOG_END
        LOG_INFO " Channel Number: %d \n", ((tmp & 0x7) + 1) LOG_END
    }
#endif

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
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_SYNC_REG, 0xFFFF);

    /* APB slave error response enable */
    ithWriteRegA(ITH_DMA_BASE + ITH_DMA_PSE_REG, 0x1);
    
    /* Clear hardware handshaking source select register */
    ithWriteRegA(ITH_DMA_BASE + 0x60, 0x0);
    ithWriteRegA(ITH_DMA_BASE + 0x64, 0x0);
    ithWriteRegA(ITH_DMA_BASE + 0x68, 0x0);
    ithWriteRegA(ITH_DMA_BASE + 0x6C, 0x0);
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
        ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(i)), ITH_DMA_INT_MASK);  // mask interrupt
        dmaCtxt[i].name = name;
        dmaCtxt[i].isr = isr;
        dmaCtxt[i].data = arg;
        dmaCtxt[i].priority = priority;
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

    if (ithGetCpuMode() == ITH_CPU_SYS)
        ithLockMutex(dmaMutex);

    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), ITH_DMA_INT_MASK);  // mask interrupt
    dmaCtxt[ch].name = NULL;
    dmaCtxt[ch].isr  = NULL;
    dmaCtxt[ch].data = NULL;

    if (ithGetCpuMode() == ITH_CPU_SYS)
        ithUnlockMutex(dmaMutex);
}

void ithDmaReset(int ch)
{
    ASSERT((ch>=0) && (ch<ITH_DMA_CH_NUM));
	ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), 0x0);
    memset((void*)&dmaCtxt[ch].burst, 0x0, sizeof(ITHDma)-16);
}

void ithDmaStart(int ch)
{
    /* 8-bits, 16-bits, 32-bits, 64-bits(unalign mode) */
    static const uint32_t dmaSizeDiv[4] = { 1, 2, 4, 1 }; 
    uint32_t value, ctrl = ITH_DMA_CTRL_ENABLE;
    ITHDma *ctxt = &dmaCtxt[ch];

#if defined(CFG_CPU_WB)
    if (ctxt->srcHw == ITH_DMA_NORMAL_MODE)
        ithFlushDCacheRange((void*)ctxt->srcAddr, ctxt->txSize);
    if (ctxt->dstHw == ITH_DMA_NORMAL_MODE)
        ithFlushDCacheRange((void*)ctxt->dstAddr, ctxt->txSize);
#endif

    if ((ctxt->srcHw == ITH_DMA_NORMAL_MODE) || (ctxt->dstHw == ITH_DMA_NORMAL_MODE))
        ithFlushMemBuffer();

    // transfer size with src width unit
    value = ctxt->txSize / dmaSizeDiv[ctxt->srcWidth];
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch)), value);

    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_LLP_CH(ch)), ctxt->lldAddr);

    // csr
    ctrl |= (ITH_DMA_CTRL_SrcTcnt(ctxt->burst) |
        ITH_DMA_CTRL_SRC_WIDTH(ctxt->srcWidth) |
        ITH_DMA_CTRL_DST_WIDTH(ctxt->dstWidth) |
        ITH_DMA_CTRL_SRC_BURST_CTRL(ctxt->srcCtrl) |
        ITH_DMA_CTRL_DST_BURST_CTRL(ctxt->dstCtrl));

    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), ctrl);
}

void ithDmaDumpReg(int ch)
{
    uint32_t value;
	printf(" dma: %s \n", dmaCtxt[ch].name);
    value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch));
	printf(" 0x%08X = 0x%08X \n", (ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), value);
    value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_CFG_CH(ch));
	printf(" 0x%08X = 0x%08X \n", (ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), value);
    value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_SRC_CH(ch));
	printf(" 0x%08X = 0x%08X \n", (ITH_DMA_BASE + ITH_DMA_SRC_CH(ch)), value);
    value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_DST_CH(ch));
	printf(" 0x%08X = 0x%08X \n", (ITH_DMA_BASE + ITH_DMA_DST_CH(ch)), value);
    value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch));
	printf(" 0x%08X = 0x%08X \n", (ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch)), value);
	value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_STRIDE_CH(ch));
	printf(" 0x%08X = 0x%08X \n", (ITH_DMA_BASE + ITH_DMA_STRIDE_CH(ch)), value);
	value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_LLP_CH(ch));
	printf(" 0x%08X = 0x%08X \n", (ITH_DMA_BASE + ITH_DMA_LLP_CH(ch)), value);
}


#define DMA_MEM_TIMEOUT     10000

static void m2m_isr(int ch, void* arg, uint32_t int_status)
{
    sem_t* sem = (sem_t*)arg;

    if (int_status & ITH_DMA_INTS_ERR)
        ithPrintf(" dma ch%d error \n", ch);
    if (int_status & ITH_DMA_INTS_WDT)
        ithPrintf(" dma ch%d watchdog timeout \n", ch);
    if (int_status & ITH_DMA_INTS_ABT)
        ithPrintf(" dma ch%d abort \n", ch);

    itpSemPostFromISR(sem);
}

int ithDmaMemcpylld(uint32_t dst, uint32_t src, uint32_t len);

int ithDmaMemcpy(uint32_t dst, uint32_t src, uint32_t len)
{
    int ch;
    sem_t   sem;
    uint32_t ctrl, cfg;
#if defined(DMA_IRQ_ENABLE)
    int res;
#else
    uint32_t timeout_ms = DMA_MEM_TIMEOUT;
#endif

    if (len > ITH_DMA_CYC_MASK)
    {
        LOG_INFO "%s() size = 0x%X > 0x%X \n", __func__, len, ITH_DMA_CYC_MASK LOG_END
        return ithDmaMemcpylld(dst, src, len);
    }
    ch = ithDmaRequestCh("m2m", ITH_DMA_CH_PRIO_HIGH, m2m_isr, &sem);
    if (ch < 0)
    {
        LOG_ERR "%s(0x%X, 0x%X,%d) no available dma channel \n", __func__, src, dst, len LOG_END
        return -1;
    }
    ithDmaReset(ch);
    sem_init(&sem, 0, 0);

    ctrl = ITH_DMA_CTRL_SRC_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_DST_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_SRC_INC |
        ITH_DMA_CTRL_DST_INC |
        ITH_DMA_CTRL_ENABLE;

    cfg = ITH_DMA_CFG_UNALIGN_MODE |
        ITH_DMA_CFG_HIGH_PRIO |
        ITH_DMA_CFG_GW(GRANT_WINDOW);

#if defined(CFG_CPU_WB)
    ithFlushDCacheRange((void*)src, len);
    ithFlushDCacheRange((void*)dst, len);
#endif
    ithFlushMemBuffer();

    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SRC_CH(ch)), src);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_DST_CH(ch)), dst);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch)), len);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), cfg);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), ctrl);

#if defined(DMA_IRQ_ENABLE)
    res = itpSemWaitTimeout(&sem, DMA_MEM_TIMEOUT);
    if (res)
#else
    while (ithDmaIsBusy(ch) && --timeout_ms)
        usleep(1000);

    if (!timeout_ms)
#endif
    {
        LOG_ERR " dma timeout!\n" LOG_END
        ithDmaDumpReg(ch);
        //while (1);
    }
    ithInvalidateDCacheRange((void*)dst, len);

    ithDmaFreeCh(ch);
    sem_destroy(&sem);

    return 0;
}

struct dma_mem_async {
	unsigned int ch : 4;
	unsigned int active : 1;
    ITHDmaAsyncHandler cb;
    void* arg;
};

static void m2m2_isr(int ch, void* arg, uint32_t int_status)
{
    struct dma_mem_async *data = (struct dma_mem_async *)arg;

    if (int_status & ITH_DMA_INTS_ERR)
        ithPrintf(" dma ch%d error \n", ch);
    if (int_status & ITH_DMA_INTS_WDT)
        ithPrintf(" dma ch%d watchdog timeout \n", ch);
    if (int_status & ITH_DMA_INTS_ABT)
        ithPrintf(" dma ch%d abort \n", ch);

    if (data->cb)
        data->cb(data->arg, int_status);

    memset((void*)data, 0x0, sizeof(struct dma_mem_async));
    ithDmaFreeCh(ch);
}

static struct dma_mem_async async_data[ITH_DMA_CH_NUM];

int ithDmaMemcpyAsync(uint32_t dst, uint32_t src, uint32_t len, ITHDmaAsyncHandler cb, void* arg)
{
    int ch, i;
    uint32_t ctrl, cfg;
	struct dma_mem_async* pdata = NULL;

    if (len > ITH_DMA_CYC_MASK)
    {
        LOG_ERR "%s() size = 0x%X > 0x%X \n", __func__, len, ITH_DMA_CYC_MASK LOG_END
        return -1;
    }

	ithEnterCritical();
	for (i = 0; i < ITH_DMA_CH_NUM; i++) {
		if (async_data[i].active == 0) {
			pdata = &async_data[i];
			pdata->active = 1;
			break;
		}
	}
	ithEnterCritical();
	if (pdata == NULL) {
		LOG_ERR "%s() can't find aysnc data struct \n", __func__ LOG_END
		return -2;
	}
	pdata->arg = arg;
	pdata->cb = cb;

	pdata->ch = ch = ithDmaRequestCh("m2m2", ITH_DMA_CH_PRIO_HIGH, m2m2_isr, pdata);
    if (ch < 0)
    {
		memset((void*)pdata, 0x0, sizeof(struct dma_mem_async));
        LOG_ERR "%s(0x%X, 0x%X,%d) no available dma channel \n", __func__, src, dst, len LOG_END
        return -1;
    }
    ithDmaReset(ch);

    ctrl = ITH_DMA_CTRL_SRC_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_DST_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_SRC_INC |
        ITH_DMA_CTRL_DST_INC |
        ITH_DMA_CTRL_ENABLE;

    cfg = ITH_DMA_CFG_UNALIGN_MODE |
        ITH_DMA_CFG_HIGH_PRIO |
        ITH_DMA_CFG_GW(GRANT_WINDOW);

#if defined(CFG_CPU_WB)
    ithFlushDCacheRange((void*)src, len);
    ithFlushDCacheRange((void*)dst, len);
#endif
    ithFlushMemBuffer();
    ithInvalidateDCacheRange((void*)dst, len);

    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SRC_CH(ch)), src);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_DST_CH(ch)), dst);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch)), len);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), cfg);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), ctrl);

    return 0;
}

int ithDmaMemcpylld(uint32_t dst, uint32_t src, uint32_t len)
{
    int ch, offset;
    sem_t   sem;
    uint32_t ctrl, cfg, cycle;
    ITHDmaDesc *first = NULL, *desc, *next;
    ITHDmaDesc *prev = NULL;
#if defined(DMA_IRQ_ENABLE)
    int res;
#else
    uint32_t timeout_ms = DMA_MEM_TIMEOUT;
#endif

    ch = ithDmaRequestCh("m2mlld", ITH_DMA_CH_PRIO_HIGH, m2m_isr, &sem);
    if (ch < 0)
    {
        LOG_ERR "%s(0x%X, 0x%X,%d) no available dma channel \n", __func__, src, dst, len LOG_END
            return -1;
    }
    ithDmaReset(ch);
    sem_init(&sem, 0, 0);

    ctrl = ITH_DMA_CTRL_SRC_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_DST_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_SRC_INC |
        ITH_DMA_CTRL_DST_INC |
        ITH_DMA_CTRL_ENABLE;

    cfg = ITH_DMA_CFG_UNALIGN_MODE |
        ITH_DMA_CFG_HIGH_PRIO |
        ITH_DMA_CFG_GW(GRANT_WINDOW);

    for (offset = 0; offset < len; offset += cycle)
    {
        cycle = ITH_MIN((len-offset), MAX_CYCLE_PER_BLOCK);
        desc = (ITHDmaDesc *)malloc(sizeof(ITHDmaDesc));
        if (!desc)
            goto error;
        memset((void*)desc, 0x0, sizeof(ITHDmaDesc));
        LOG_DBG " + %X\n", desc LOG_END

        desc->cfg = cfg;
        desc->len = len;

        if (first == NULL) /** first command */
        {
			desc->lld.src = src + offset;
			desc->lld.dst = dst + offset;
			desc->lld.cycle = cycle;
			desc->lld.ctrl = ctrl;

            first = desc;
        }
        else /** second and later command for LLD */
        {
			desc->lld.src = cpu_to_le32(src + offset);
			desc->lld.dst = cpu_to_le32(dst + offset);
			desc->lld.cycle = cpu_to_le32(cycle);
			desc->lld.ctrl = cpu_to_le32(ctrl);

            if (prev == first) {
                prev->lld.ctrl |= ITH_DMA_CTRL_MASK_TC;
                prev->lld.next = (uint32_t)desc;
            }
            else {
                prev->lld.ctrl = cpu_to_le32(le32_to_cpu(prev->lld.ctrl) | ITH_DMA_CTRL_MASK_TC);
                prev->lld.next = cpu_to_le32(desc);
            }
			#if defined(CFG_CPU_WB)
			ithFlushDCacheRange((void*)prev, sizeof(ITHDmaLLD));
			#endif
        }

        prev = desc;
    }
#if defined(CFG_CPU_WB)
    ithFlushDCacheRange((void*)prev, sizeof(ITHDmaLLD));
    ithFlushDCacheRange((void*)src, len);
    ithFlushDCacheRange((void*)dst, len);
#endif
    ithFlushMemBuffer();

    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SRC_CH(ch)), first->lld.src);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_DST_CH(ch)), first->lld.dst);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_LLP_CH(ch)), first->lld.next);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch)), first->lld.cycle);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), first->cfg);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), first->lld.ctrl);

#if defined(DMA_IRQ_ENABLE)
    res = itpSemWaitTimeout(&sem, DMA_MEM_TIMEOUT);
    if (res)
#else
    while (ithDmaIsBusy(ch) && --timeout_ms)
        usleep(1000);

    if (!timeout_ms)
#endif
    {
        LOG_ERR " dma timeout!\n" LOG_END
        ithDmaDumpReg(ch);
        //while (1);
    }

    ithInvalidateDCacheRange((void*)dst, len);

    desc = first;
    do {
		if (desc != first)
			next = (ITHDmaDesc *)le32_to_cpu(desc->lld.next);
		else
            next = (ITHDmaDesc *)desc->lld.next;
        free(desc);
        LOG_DBG " - %X\n", desc LOG_END
        desc = next;
    } while (desc);

error:
    ithDmaFreeCh(ch);
    sem_destroy(&sem);

    return 0;
}

int ithDmaMemset(uint32_t dst, uint32_t pattern, uint32_t len)
{
    int ch;
    sem_t   sem;
    uint32_t ctrl, cfg;
#if defined(DMA_IRQ_ENABLE)
    int res;
#else
    uint32_t timeout_ms = DMA_MEM_TIMEOUT;
#endif

    if (len > ITH_DMA_CYC_MASK)
    {
        LOG_INFO "%s() size = 0x%X > 0x%X \n", __func__, len, ITH_DMA_CYC_MASK LOG_END
        return -1;
    }
    if ((len | dst) & 0x3)
    {
        LOG_ERR "%s: dst 0x%X, len %d => need 4-bytes alignment \n", __func__, dst, len LOG_END
        return -1;
    }

    ch = ithDmaRequestCh("m2m", ITH_DMA_CH_PRIO_HIGH, m2m_isr, &sem);
    if (ch < 0)
    {
        LOG_ERR "%s(0x%X, 0x%X,%d) no available dma channel \n", __func__, dst, pattern, len LOG_END
            return -1;
    }
    ithDmaReset(ch);
    sem_init(&sem, 0, 0);

    ctrl = ITH_DMA_CTRL_SRC_WIDTH(ITH_DMA_WIDTH_32) |
        ITH_DMA_CTRL_DST_WIDTH(ITH_DMA_WIDTH_32) |
        ITH_DMA_CTRL_SRC_FIXED |
        ITH_DMA_CTRL_DST_INC |
        ITH_DMA_CTRL_ENABLE;

    cfg = ITH_DMA_CFG_WO_MODE |
        ITH_DMA_CFG_HIGH_PRIO |
        ITH_DMA_CFG_GW(GRANT_WINDOW);

#if defined(CFG_CPU_WB)
    ithFlushDCacheRange((void*)dst, len);
#endif
    ithFlushMemBuffer();

    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_WO_VALUE_REG), pattern);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_DST_CH(ch)), dst);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch)), (len/sizeof(uint32_t)));
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), cfg);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), ctrl);

#if defined(DMA_IRQ_ENABLE)
    res = itpSemWaitTimeout(&sem, DMA_MEM_TIMEOUT);
    if (res)
#else
    while (ithDmaIsBusy(ch) && --timeout_ms)
        usleep(1000);

    if (!timeout_ms)
#endif
    {
        LOG_ERR " dma timeout!\n" LOG_END
            ithDmaDumpReg(ch);
        //while (1);
    }
    ithInvalidateDCacheRange((void*)dst, len);

    ithDmaFreeCh(ch);
    sem_destroy(&sem);

    return 0;

}

int ithDmaTx2D(uint32_t srcAddr, uint32_t dstAddr, 
    uint16_t xCnt, uint16_t yCnt, 
    int16_t srcStride, int16_t dstStride)
{
    int ch;
    sem_t   sem;
    uint32_t ctrl, cfg;
#if defined(DMA_IRQ_ENABLE)
    int res;
#else
    uint32_t timeout_ms = DMA_MEM_TIMEOUT;
#endif

    if ((srcStride | dstStride) & 0x7)
    {
        LOG_ERR "%s: srcStride %d, dstStride %d => need 64-bits alignment \n", __func__, srcStride, dstStride LOG_END
        return -1;
    }

    ch = ithDmaRequestCh("m2m", ITH_DMA_CH_PRIO_HIGH, m2m_isr, &sem);
    if (ch < 0)
    {
        LOG_ERR "%s() no available dma channel \n", __func__ LOG_END
        return -1;
    }
    ithDmaReset(ch);
    sem_init(&sem, 0, 0);

    ctrl = ITH_DMA_CTRL_SRC_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_DST_WIDTH(ITH_DMA_WIDTH_64) |
        ITH_DMA_CTRL_SRC_INC |
        ITH_DMA_CTRL_DST_INC |
        ITH_DMA_CTRL_EXP |
        ITH_DMA_CTRL_2D;

    cfg = ITH_DMA_CFG_UNALIGN_MODE |
        ITH_DMA_CFG_HIGH_PRIO |
        ITH_DMA_CFG_GW(GRANT_WINDOW);

#if defined(CFG_CPU_WB)
    ithFlushDCacheRange((void*)srcAddr, (srcStride*yCnt));
    ithFlushDCacheRange((void*)dstAddr, (dstStride*yCnt));
#endif
    ithFlushMemBuffer();

	ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), ctrl);  /* need enable 2d function first */
	ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SRC_CH(ch)), srcAddr);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_DST_CH(ch)), dstAddr);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SIZE_CH(ch)), (uint32_t)((yCnt << 16) | xCnt));
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_STRIDE_CH(ch)), (uint32_t)((dstStride << 16) | srcStride));
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), cfg);
	//ithDmaDumpReg(ch);
	ctrl |= ITH_DMA_CTRL_ENABLE;
	ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), ctrl);

#if defined(DMA_IRQ_ENABLE)
    res = itpSemWaitTimeout(&sem, DMA_MEM_TIMEOUT);
    if (res)
#else
    while (ithDmaIsBusy(ch) && --timeout_ms)
        usleep(1000);

    if (!timeout_ms)
#endif
    {
        LOG_ERR " dma timeout!\n" LOG_END
        ithDmaDumpReg(ch);
        //while (1);
    }
    ithInvalidateDCacheRange((void*)dstAddr, (dstStride*yCnt));

    ithDmaFreeCh(ch);
    sem_destroy(&sem);

    return 0;
}

