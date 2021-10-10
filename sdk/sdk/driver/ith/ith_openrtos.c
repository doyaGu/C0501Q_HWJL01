/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL FreeRTOS functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/param.h>
#include "ith_cfg.h"
#include "openrtos/FreeRTOS.h"
#include "openrtos/semphr.h"
#include "openrtos/task.h"

#ifdef __SM32__
#include "openrtos/sm32/port_spr_defs.h"
#elif defined(__NDS32__)
#include "openrtos/nds32/cache.h"
#endif

#define CACHE_LINESZ 32
#define TIMER_LOAD_VAL  0xFFFFFFFF

extern uint32_t __bootimage_func_start;
extern uint32_t _text_end;
extern uint32_t __cmdq_base;

static ITHCmdQ cmdQ;
static void* dmaMutex;
void* ithStorMutex = NULL;

static void GpioIntrHandler(void* arg)
{
    ithGpioDoIntr();
}

void ithInit(void)
{
    const ITHCardConfig cardCfg =
    {
        {
            // card detect pin of sd0
        #if defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)
            CFG_GPIO_SD0_CARD_DETECT,
        #elif defined(CFG_SD0_STATIC) || defined(CFG_SDIO0_STATIC)
            (uint8_t)-2,  // always insert
        #else
            (uint8_t)-1,
        #endif

            // card detect pin of sd1
        #if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
            CFG_GPIO_SD1_CARD_DETECT,
        #elif defined(CFG_SD1_STATIC) || defined(CFG_SDIO1_STATIC)
            (uint8_t)-2,  // always insert
        #else
            (uint8_t)-1,
        #endif
        },

        {
            // power enable pin of sd0
        #if defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)
            CFG_GPIO_SD0_POWER_ENABLE,
        #else
            (uint8_t)-1,
        #endif

            // power enable pin of sd1
        #if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
            CFG_GPIO_SD1_POWER_ENABLE,
        #else
            (uint8_t)-1,
        #endif
        },

        {
            // write protect pin of sd0
        #if defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)
            CFG_GPIO_SD0_WRITE_PROTECT,
        #else
            (uint8_t)-1,
        #endif

            // write protect pin of sd1
        #if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
            CFG_GPIO_SD1_WRITE_PROTECT,
        #else
            (uint8_t)-1,
        #endif
        },

        {
    #if defined(CFG_GPIO_SD0_IO)
            CFG_GPIO_SD0_IO,
    #else
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    #endif
        },

        {
    #if defined(CFG_GPIO_SD1_IO)
            CFG_GPIO_SD1_IO,
    #else
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    #endif
        },
    };

    // init memory debug
#ifdef CFG_MEMDBG_ENABLE
    ithMemDbgSetMode(ITH_MEMDBG0, ITH_MEMDGB_WRITEONLY);
    ithMemDbgSetRange(ITH_MEMDBG0, 0x0, 0x1000 - 64 - 4);   // watch writing null pointer error
    ithMemDbgEnable(ITH_MEMDBG0);

    ithMemDbgSetMode(ITH_MEMDBG1, ITH_MEMDGB_WRITEONLY);
    ithMemDbgSetRange(ITH_MEMDBG1, (uint32_t)&__bootimage_func_start, (uint32_t)&_text_end);   // watch writing .text section error
    ithMemDbgEnable(ITH_MEMDBG1);

    ithMemDbgEnableIntr();
#endif // CFG_MEMDBG_ENABLE

#if !defined(CFG_JPEG_HW_ENABLE) && !defined(CFG_VIDEO_ENABLE)
    // disable isp clock for power saving
    ithIspDisableClock();
#endif

    #if !defined(CFG_USB0_ENABLE)
    ithUsbPhyDisableClock(ITH_USB0);
    #endif
    #if !defined(CFG_USB1_ENABLE)
    ithUsbPhyDisableClock(ITH_USB1);
    #endif

    // init mpu
    ithGetRevisionId();
    ithGetPackageId(); // for power saving
    ithIntrReset();
    ithIntrInit();

    // recover AHB0 control register value
    ithAhb0SetCtrlReg();

    // init timer for delay counting
    ithTimerReset(portTIMER);
    ithTimerCtrlEnable(portTIMER, ITH_TIMER_UPCOUNT);
    ithTimerCtrlEnable(portTIMER, ITH_TIMER_PERIODIC);
    ithTimerSetCounter(portTIMER, 0);
    ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);
    ithTimerEnable(portTIMER);

    // init gpio
    ithGpioInit();
    ithGpioSetDebounceClock(800);
    ithIntrDisableIrq(ITH_INTR_GPIO);
    ithIntrClearIrq(ITH_INTR_GPIO);
    ithIntrRegisterHandlerIrq(ITH_INTR_GPIO, GpioIntrHandler, NULL);
    ithGpioEnableClock();

    // init dma controller
    dmaMutex = xSemaphoreCreateMutex();
    ithDmaInit(dmaMutex);

    // init card
#if defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE)
    ithCardInit(&cardCfg);
    // storage need always power on for pin share issue
    ithCardPowerOn(ITH_CARDPIN_SD0);
    ithCardPowerOn(ITH_CARDPIN_SD1);
#endif // defined(CFG_SD0_ENABLE) || defined(CFG_SD1_ENABLE)

    // create mutex for storage pin share
    ithStorMutex = xSemaphoreCreateMutex();

    // init command queue
#ifdef CFG_CMDQ_ENABLE
#if (CFG_CHIP_FAMILY == 9920)
    cmdQ.size = CFG_CMDQ_SIZE;
    cmdQ.addr = (uint32_t)&__cmdq_base;
    cmdQ.mutex = xSemaphoreCreateMutex();

    ithCmdQInit(&cmdQ, ITH_CMDQ0_OFFSET);
    ithCmdQReset(ITH_CMDQ0_OFFSET);
    #if BYTE_ORDER == BIG_ENDIAN
    ithCmdQCtrlEnable(ITH_CMDQ_BIGENDIAN, ITH_CMDQ0_OFFSET);
    #endif
#else
    cmdQ.size   = CFG_CMDQ_SIZE;
    cmdQ.addr   = (uint32_t)&__cmdq_base;
    cmdQ.mutex  = xSemaphoreCreateMutex();

    ithCmdQInit(&cmdQ);
    ithCmdQReset();
    #if BYTE_ORDER == BIG_ENDIAN
    ithCmdQCtrlEnable(ITH_CMDQ_BIGENDIAN);
    #endif
#endif //(CFG_CHIP_FAMILY == 9920)
#endif // CFG_CMDQ_ENABLE

    // init stc
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)    
#ifdef CFG_VIDEO_ENABLE
    ithFpcReset();
#endif
#endif
}

void ithAssertFail(const char* exp, const char* file, int line, const char* func)
{
    ithPrintf("\r\n*** ASSERTION FAILED in %s:%d/%s():\r\n%s\r\n", file, line, func, exp);

    for (;;)
        taskYIELD();
}

void ithDelay(unsigned long us)
{
    TickType_t t = (TickType_t)((uint64_t) us / (1000 * portTICK_PERIOD_MS));
    
    /*
    if (t > 0)
    {
        vTaskDelay(t);
        us = us % (1000 * portTICK_PERIOD_MS);
    }
    */

    if (us > 0)
    {
        long tmo = (uint64_t)ithGetBusClock() * us / 1000000;
        unsigned long now, last = ithTimerGetCounter(portTIMER);

        while (tmo > 0)
        {
            now = ithTimerGetCounter(portTIMER);
            if (now > last)
                tmo -= now - last;
            else
                tmo -= configCPU_CLOCK_HZ / configTICK_RATE_HZ - last + now; /* count up timer overflow */

            last = now;
        }
    }
}

void ithLockMutex(void* mutex)
{
    xSemaphoreTake((SemaphoreHandle_t)mutex, portMAX_DELAY);
}

void ithUnlockMutex(void* mutex)
{
    xSemaphoreGive((SemaphoreHandle_t)mutex);
}

void ithFlushDCache(void)
{
#ifdef CFG_CPU_WB
#ifdef __arm__
    ithEnterCritical();

    __asm__ __volatile__ (
        "mov r3,#0\n"
        "mcr p15,0,r3,c7,c10,0\n"   /* flush d-cache all */
        "mcr p15,0,r3,c7,c10,4\n"   /* flush d-cache write buffer */
        :
        :
        : "r3"  /* clobber list */
        );

    ithExitCritical();

#elif defined(__NDS32__)
    ithEnterCritical();
    n12_dcache_flush();
    ithExitCritical();
#else
    // DO NOTHING
#endif //   __arm__
#endif // CFG_CPU_WB
}

__attribute__((used)) void ithInvalidateDCache(void)
{
#ifdef __arm__
    ithEnterCritical();

    __asm__ __volatile__ (
        "mov r3,#0\n"
        "mcr p15,0,r3,c7,c6,0\n"    /* invalidate d-cache all */
        :
        :
        : "r3"  /* clobber list */
        );

    ithExitCritical();

#elif defined(__SM32__)
    int ncs, bs;
    int cache_size, cache_line_size;
    int i = 0;

    // Number of cache sets
    ncs = ((mfspr(SPR_DCCFGR) >> 3) & 0xf);

    // Number of cache block size
    bs  = ((mfspr(SPR_DCCFGR) >> 7) & 0x1) + 4;

    // Calc Cache size
    cache_line_size = 1 << bs;
    cache_size      = 1 << (ncs+bs);

    ithEnterCritical();

    // Disable DC
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_DCE);

    // Flush DC
    do {
        mtspr(SPR_DCBIR, i);
        i += cache_line_size;
    } while(i < cache_size);

    // Enable DC
    mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_DCE);
    asm volatile("l.nop 0x0\nl.nop 0x0\nl.nop 0x0\nl.nop 0x0\n");

    ithExitCritical();

#elif defined(__NDS32__)
    ithEnterCritical();
    n12_dcache_invalidate();
    ithExitCritical();
#elif defined(__riscv)
    // TODO: RISCV
#warning "no define for RISCV @ithInvalidateDCache"
#endif //   __arm__
}

__attribute__((used)) void ithFlushDCacheRange(void* addr, uint32_t size)
{
#ifdef CFG_CPU_WB
#ifdef __arm__
    unsigned int linesz = CACHE_LINESZ;
    unsigned long start = (uint32_t) addr;

    // aligned to cache line
    size += (start % linesz) ? linesz : 0;
    start &= ~(linesz - 1);

    ithEnterCritical();

    // do it!
    __asm__ __volatile__ (
        "mov r0,%0\n"
        "mov r1,%1\n"
        "add r1,r0,r1\n"
        "1:\n"
        "mcr p15,0,r0,c7,c10,1\n"
        "add r0,r0,%2\n"
        "cmp r0,r1\n"
        "blo 1b\n"
        "mov r0,#0\n"
        "mcr p15,0,r0,c7,c10,4\n"
        :
        : "r"(start), "r"(size), "r"(linesz)    /* input */
        : "r0", "r1", "cc"  /* clobber list */
        );

    ithExitCritical();

#elif defined(__NDS32__)
    ithEnterCritical();
    n12_dcache_flush_range((unsigned long)addr, ((unsigned long)addr) + size);
    ithExitCritical();

#else
    // DO NOTHING
#endif //   __arm__
#endif // CFG_CPU_WB
}

__attribute__((used)) void ithInvalidateDCacheRange(void* addr, uint32_t size)
{
#ifdef __arm__
    unsigned int linesz = CACHE_LINESZ;
    unsigned long start = (uint32_t) addr;

    // aligned to cache line
    size += (start % linesz) ? linesz : 0;
    start &= ~(linesz - 1);

    ithEnterCritical();

    // do it!
    __asm__ __volatile__ (
        "mov r0,%0\n"
        "mov r1,%1\n"
        "add r1,r0,r1\n"
        "1:\n"
        "mcr p15,0,r0,c7,c6,1\n"
        "add r0,r0,%2\n"
        "cmp r0,r1\n"
        "blo 1b\n"
        :
        : "r"(start), "r"(size), "r"(linesz)    /* input */
        : "r0", "r1", "cc"  /* clobber list */
        );

    ithExitCritical();

#elif defined(__SM32__)

    unsigned int line_size;
    unsigned int cache_size;
    unsigned int start;
    unsigned int end;
    int ncs, bs;

    // Number of cache sets
    ncs = ((mfspr(SPR_DCCFGR) >> 3) & 0xf);

    // Number of cache block size
    bs  = ((mfspr(SPR_DCCFGR) >> 7) & 0x1) + 4;

    cache_size = (1 << (ncs+bs));
    line_size  = (1 << bs);

    start = ((unsigned int)addr) & ~(line_size-1);
    end   = ((unsigned int)addr) + size - 1;
    end   = ((unsigned int)end) & ~(line_size-1);

    if (end > start + cache_size - line_size)
        end = start + cache_size - line_size;

    ithEnterCritical();

    do {
        mtspr(SPR_DCBIR, start);
        start += line_size;
    } while (start <= end);

    ithExitCritical();

#elif defined(__NDS32__)
    ithEnterCritical();
    n12_dcache_invalidate_range((unsigned long)addr, ((unsigned long)addr) + size);
    ithExitCritical();
#elif defined(__riscv)
    // TODO: RISCV
#warning "no defined for RISCV @ithInvalidateDCacheRange"
#endif //   __arm__
}

void ithInvalidateICache(void)
{
#ifdef __arm__	
    ithEnterCritical();

    __asm__ __volatile__ (
        "mcr p15,0,%0,c7,c14,0\n"
        "mcr p15,0,%0,c7,c5,0\n"    /* invalidate i-cache all */
        :
        :
        "r"(0)  /* clobber list */
        );

    ithExitCritical();

#elif defined(__SM32__)
    int ncs, bs;
    int cache_size, cache_line_size;
    int i = 0;

    // Number of cache sets
    ncs = (mfspr(SPR_ICCFGR) >> 3) & 0xf;

    // Number of cache block size
    bs  = ((mfspr(SPR_ICCFGR) >> 7) & 0x1) + 4;

    // Calc Cache size
    cache_line_size = 1 << bs;
    cache_size      = 1 << (ncs+bs);

    ithEnterCritical();

    // Disable IC, DC
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_ICE);
	mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_DCE);

    // Flush IC, DC
    do {
        mtspr(SPR_ICBIR, i);
		mtspr(SPR_DCBIR, i);
        i += cache_line_size;
    } while (i < cache_size);

    // Enable IC, DC
    mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_ICE);
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_DCE);
    asm volatile("l.nop 0x0\nl.nop 0x0\nl.nop 0x0\nl.nop 0x0\n");

    ithExitCritical();

#elif defined(__NDS32__)
    ithEnterCritical();
    n12_icache_flush();
    ithExitCritical();
#elif defined(__riscv)
    // TODO: RISCV
#warning "no define for RISC @ithInvalidateICache"
#endif //   __arm__
}

void ithCpuDoze(void)
{
#ifdef __arm__
    __asm__ __volatile__ (
        "mov r3,#0\n"
        "mcr p15,0,r3,c7,c10,0\n"   /* Clean D-Cache All */
        "mcr p15,0,r3,c7,c10,4\n"   /* Sync (drain write buffer) */
        "mcr p15,0,r3,c7,c0,4\n"    /* execute the IDLE instruction but it only executes when it is not user mode */
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        :
        :
        : "r3"  /* clobber list */
        );
#elif defined(__SM32__)
    asm volatile("l.mtspr\t\t%0,%1,0": : "r" (SPR_PMR), "r" (SPR_PMR_DME));
    asm volatile("l.nop\t\t0x3388"); // One delay slot to enter doze mode
    asm volatile("l.nop\t\t0x3388"); // Another delay slot to avoid the
                                     // delay interrupt

#elif defined(__NDS32__)
    // TODO: IMPLEMENT

#elif defined(__riscv)
    // TODO: RISCV

#endif //   __arm__
}

__attribute__((used)) void ithFlushMemBuffer(void)
{
    ithEnterCritical();

#ifdef __arm__
    __asm__ __volatile__ (
        "mov r3,#0\n"
        "mcr p15,0,r3,c7,c10,4\n"   /* flush d-cache write buffer */
        :
        :
        : "r3"  /* clobber list */
        );

		#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    ithWriteRegMaskH(0x1F00, (0x1 << 14), 0x4000);

    //  Wait ARM engine flush finish!   0x1f00 D[15] 1: empty
    while (!(ithReadRegH(0x1F00) & 0x8000));
		#endif

#elif defined(__SM32__)

		#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    ithWriteRegMaskH(0x16F6, (0x1 << 15), (0x1 << 15));
    ithWriteRegMaskH(0x16F6, 0, (0x1 << 15));
		#else
    // 9920: TODO
    ithWriteRegMaskA(0xB0200000 + 0x78, (0x1 << 13), (0x1 << 13));
    ithWriteRegMaskA(0xB0200000 + 0x78, 0, (0x1 << 13));
		#endif

#elif defined(__NDS32__)
    // TODO: IMPLEMENT

#elif defined(__riscv)
    // TODO: RISCV

#endif //   __arm__

    ithExitCritical();
}

void ithFlushAhbWrap(void)
{
    uint8_t reg;

    ithEnterCritical();

    ithWriteRegMaskH(0x3DA, (0x1 << 11), (0x1 << 11));
    ithWriteRegMaskH(0x3F8, 0x2, 0x2);

    // wait AHB Wrap flush finish!
    while ((ithReadRegH(0x3F8) & 0x2));

    ithWriteRegMaskH(0x3DA, 0x0, (0x1 << 11));

    ithExitCritical();
}

void ithYieldFromISR(bool yield)
{
    portYIELD_FROM_ISR(yield ? pdTRUE : pdFALSE);
}

void ithTaskYield(void)
{
    taskYIELD();
}

void ithEnterCritical(void)
{
    if (ithGetCpuMode() == ITH_CPU_SYS)
        portENTER_CRITICAL();
}

void ithExitCritical(void)
{
    if (ithGetCpuMode() == ITH_CPU_SYS)
        portEXIT_CRITICAL();
}

ITHCpuMode ithGetCpuMode(void)
{
#ifdef __arm__
    uint32_t mode = 0;

    __asm__ __volatile__ (
        "mrs %0, CPSR\n"
        : "=r"(mode)
        );
    return (mode & 0x1F);

#elif defined(__SM32__)

    return (mfspr(SPR_SR) & SPR_SR_SM) ? ITH_CPU_SVC : ITH_CPU_SYS;

#elif defined(__NDS32__)
    // TODO: IMPLEMENT
    return ITH_CPU_SYS;

#elif defined(__riscv)
    // TODO: RISCV
    return ITH_CPU_SYS;

#endif //   __arm__
}
