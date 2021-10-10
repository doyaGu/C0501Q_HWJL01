/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor DMA hardware abstraction layer.
 *
 * @author Irene Lin
 * @version 1.0
 */
#if defined(CFG_AXI_DMA)

#include "ith_dma_axi.h"

#else  // #if defined(CFG_AXI_DMA)

#ifndef ITH_DMA_H
#define ITH_DMA_H


#define DMA_IRQ_ENABLE

#define ITH_DMA_CH_NUM      8


typedef void (*ITHDmaIntrHandler)(int ch, void* arg, uint32_t int_status);
#define ITH_DMA_INTS_TC   0x1
#define ITH_DMA_INTS_ERR  0x2
#define ITH_DMA_INTS_ABT  0x4

typedef struct
{
    char*    name;
    ITHDmaIntrHandler isr;
    void*    data;

    // csr
    uint32_t priority;
    uint8_t  fifoThreshold;
    uint8_t  burst;
    uint8_t  srcWidth;
    uint8_t  dstWidth;
    uint8_t  hw;
    uint8_t  srcCtrl;
    uint8_t  dstCtrl;
    uint8_t  srcSel;
    uint8_t  dstSel;
    uint8_t  srcHw;
    uint8_t  dstHw;
    uint8_t  reserved[1];
    
    uint32_t lldAddr;

    /** src width unit */
    uint32_t txSize;  

	uint32_t srcAddr;
	uint32_t dstAddr;
} ITHDma;


extern ITHDma dmaCtxt[ITH_DMA_CH_NUM];
extern ITHDmaWidth dmaWidthMap[4];

static inline void ithDmaSetRequest(int ch, ITHDmaMode srcMode, ITHDmaRequest srcReq, ITHDmaMode dstMode, ITHDmaRequest dstReq)
{
    uint32_t value;
    value = (dstMode << ITH_DST_HE_BIT) |
            (dstReq << ITH_DST_RS_BIT) |
            (srcMode << ITH_SRC_HE_BIT) |
            (srcReq << ITH_SRC_RS_BIT);
#if !defined(DMA_IRQ_ENABLE)
    value |= ITH_DMA_INT_MASK;
#endif
    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_CFG_REG+ch*ITH_DMA_CH_OFFSET), value);

    dmaCtxt[ch].hw = srcMode | dstMode;
	dmaCtxt[ch].srcHw = srcMode;
	dmaCtxt[ch].dstHw = dstMode;
}

static inline void ithDmaSetSrcAddr(int ch, uint32_t addr)
{
#if defined(CFG_CPU_WB)	
    dmaCtxt[ch].srcAddr = addr;
#endif
    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_SRCADR_REG+ch*ITH_DMA_CH_OFFSET), addr);
}

static inline void ithDmaSetDstAddr(int ch, uint32_t addr)
{
#if defined(CFG_CPU_WB)	
    dmaCtxt[ch].dstAddr = addr;
#endif
    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_DSTADR_REG+ch*ITH_DMA_CH_OFFSET), addr);
}

static inline void ithDmaSetTxSize(int ch, uint32_t size) // byte unit 
{
    dmaCtxt[ch].txSize = size;
}

static inline void ithDmaSetSrcParams(int ch, ITHDmaWidth width, ITHDmaAddrCtl ctl, ITHDmaDataMaster master)
{
    dmaCtxt[ch].srcWidth = width;
    dmaCtxt[ch].srcCtrl  = ctl;
    dmaCtxt[ch].srcSel   = master;
}

static inline void ithDmaSetDstParams(int ch, ITHDmaWidth width, ITHDmaAddrCtl ctl, ITHDmaDataMaster master)
{
    dmaCtxt[ch].dstWidth = width;
    dmaCtxt[ch].dstCtrl  = ctl;
    dmaCtxt[ch].dstSel   = master;
}

static inline void ithDmaSetBurst(int ch, ITHDmaBurst burst)
{
    dmaCtxt[ch].burst = burst;
}

static inline void ithDmaSetLLPAddr(int ch, uint32_t llpaddr)  
{    
    dmaCtxt[ch].lldAddr = llpaddr;
} 

static inline void ithDmaAbort(int ch)
{
    ithWriteRegA((ITH_DMA_BASE+ITH_DMA_C0_CSR_REG+ch*ITH_DMA_CH_OFFSET), ITH_DMA_ABT_MASK);
}

static inline int ithDmaIsBusy(int ch)
{
    uint32_t value = ithReadRegA(ITH_DMA_BASE+ITH_DMA_C0_CSR_REG+ch*ITH_DMA_CH_OFFSET);
    return (value & ITH_DMA_CH_EN_MASK);
}

void ithDmaInit(void* mutex);
int ithDmaRequestCh(char* name, ITHDmaPriority priority, ITHDmaIntrHandler isr, void* arg);
void ithDmaFreeCh(int ch);
void ithDmaReset(int ch);
void ithDmaDumpReg(int ch);
void ithDmaStart(int ch);

#endif // ITH_DMA_H


#endif // #if defined(CFG_AXI_DMA)


