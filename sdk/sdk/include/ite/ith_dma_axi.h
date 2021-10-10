/*
 * Copyright (c) 2015 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor DMA hardware abstraction layer.
 *
 * @author Irene Lin
 * @version 1.0
 */
#ifndef ITH_DMA_AXI_H
#define ITH_DMA_AXI_H

#if !defined(WIN32)
#define DMA_IRQ_ENABLE
#endif

#define ITH_DMA_CH_NUM      8
#define GRANT_WINDOW	    64
#define MAX_CYCLE_PER_BLOCK	0x100000 //0x200000

typedef void (*ITHDmaIntrHandler)(int ch, void* arg, uint32_t int_status);
#define ITH_DMA_INTS_TC   0x1
#define ITH_DMA_INTS_ERR  0x2
#define ITH_DMA_INTS_WDT  0x4
#define ITH_DMA_INTS_ABT  0x8

typedef struct
{
    uint32_t    src;
    uint32_t    dst;
    uint32_t    next;
    uint32_t    ctrl;
    uint32_t    cycle;
    uint32_t    stride;
} ITHDmaLLD;

typedef struct
{
    ITHDmaLLD   lld;
    uint32_t    cfg;
    uint32_t    len;
} ITHDmaDesc;

typedef struct
{
    char*    name;
    ITHDmaIntrHandler isr;
    void*    data;

    // cfg
    uint32_t priority;

    // control
    uint8_t  burst;     /* source transfer count of one DMA handshake operation */
    uint8_t  srcWidth;
    uint8_t  dstWidth;
    uint8_t  srcCtrl;
    uint8_t  dstCtrl;

    //
    uint8_t  hw;
    uint8_t  srcHw;
    uint8_t  dstHw;

    //
    uint32_t txSize;
    uint32_t srcAddr;
    uint32_t dstAddr;
    uint32_t lldAddr;
} ITHDma;


extern ITHDma dmaCtxt[ITH_DMA_CH_NUM];
extern ITHDmaWidth dmaWidthMap[4];

static inline void ithDmaSetRequest(int ch, ITHDmaMode srcMode, ITHDmaRequest srcReq, ITHDmaMode dstMode, ITHDmaRequest dstReq)
{
    uint32_t cfg = dmaCtxt[ch].priority | ITH_DMA_CFG_GW(GRANT_WINDOW);

    cfg |= (ITH_DMA_CFG_DST_HANDSHAKE_EN(dstMode) |
        ITH_DMA_CFG_DST_HANDSHAKE(ch) |
        ITH_DMA_CFG_SRC_HANDSHAKE_EN(srcMode) |
        ITH_DMA_CFG_SRC_HANDSHAKE(ch));

#if !defined(DMA_IRQ_ENABLE)
    cfg |= ITH_DMA_INT_MASK;
#endif
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CFG_CH(ch)), cfg);

    if (srcMode == ITH_DMA_HW_HANDSHAKE_MODE) {
        ithWriteRegMaskA((ITH_DMA_BASE + ITH_DMA_HW_SRC_SEL_S_REG(ch)),
            ITH_DMA_HW_SRC_SEL_VAL(ch, srcReq),
            ITH_DMA_HW_SRC_SEL_MSK(ch));
    }

    if (dstMode == ITH_DMA_HW_HANDSHAKE_MODE) {
        ithWriteRegMaskA((ITH_DMA_BASE + ITH_DMA_HW_SRC_SEL_D_REG(ch)),
            ITH_DMA_HW_SRC_SEL_VAL(ch, dstReq),
            ITH_DMA_HW_SRC_SEL_MSK(ch));
    }

    dmaCtxt[ch].hw = srcMode | dstMode;
    dmaCtxt[ch].srcHw = srcMode;
    dmaCtxt[ch].dstHw = dstMode;
}

static inline void ithDmaSetSrcAddr(int ch, uint32_t addr)
{
#if defined(CFG_CPU_WB)	
    dmaCtxt[ch].srcAddr = addr;
#endif
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_SRC_CH(ch)), addr);
}

static inline void ithDmaSetDstAddr(int ch, uint32_t addr)
{
#if defined(CFG_CPU_WB)	
    dmaCtxt[ch].dstAddr = addr;
#endif
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_DST_CH(ch)), addr);
}

static inline void ithDmaSetTxSize(int ch, uint32_t size) // byte unit 
{
    dmaCtxt[ch].txSize = size;
}

static inline void ithDmaSetSrcParamsA(int ch, ITHDmaWidth width, ITHDmaAddrCtl ctl)
{
    dmaCtxt[ch].srcWidth = width;
    dmaCtxt[ch].srcCtrl  = ctl;
}

static inline void ithDmaSetDstParamsA(int ch, ITHDmaWidth width, ITHDmaAddrCtl ctl)
{
    dmaCtxt[ch].dstWidth = width;
    dmaCtxt[ch].dstCtrl  = ctl;
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
    uint32_t enabled = 0xff & ~(1 << ch);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CH_EN_REG), enabled);
    ithWriteRegA((ITH_DMA_BASE + ITH_DMA_CTRL_CH(ch)), 0);
}

static inline int ithDmaIsBusy(int ch)
{
    uint32_t value = ithReadRegA(ITH_DMA_BASE + ITH_DMA_CH_EN_REG);
    return (value & (1 << ch));
}

void ithDmaInit(void* mutex);
int ithDmaRequestCh(char* name, ITHDmaPriority priority, ITHDmaIntrHandler isr, void* arg);
void ithDmaFreeCh(int ch);
void ithDmaReset(int ch);
void ithDmaDumpReg(int ch);
void ithDmaStart(int ch);
int ithDmaMemcpy(uint32_t dst, uint32_t src, uint32_t len);

typedef void(*ITHDmaAsyncHandler)(void* arg, uint32_t int_status);
int ithDmaMemcpyAsnyc(uint32_t dst, uint32_t src, uint32_t len, ITHDmaAsyncHandler cb, void* arg);
int ithDmaMemset(uint32_t dst, uint32_t pattern, uint32_t len);
int ithDmaTx2D(uint32_t srcAddr, uint32_t dstAddr,
    uint16_t xCnt, uint16_t yCnt,
    int16_t srcStride, int16_t dstStride);


//  wrapper 
/*
* for priority
*/
#define ITH_DMA_CH_PRIO_LOWEST      ITH_DMA_CH_PRIO_LOW
#define ITH_DMA_CH_PRIO_HIGH_3      ITH_DMA_CH_PRIO_LOW
#define ITH_DMA_CH_PRIO_HIGH_2      ITH_DMA_CH_PRIO_HIGH
#define ITH_DMA_CH_PRIO_HIGHEST     ITH_DMA_CH_PRIO_HIGH

#define ITH_DMA_MASTER_0    0
#define ITH_DMA_MASTER_1    0

#define ithDmaSetSrcParams(ch, a, b, c)     ithDmaSetSrcParamsA(ch, a, b)
#define ithDmaSetDstParams(ch, a, b, c)     ithDmaSetDstParamsA(ch, a, b)

#endif // ITH_DMA_AXI_H
