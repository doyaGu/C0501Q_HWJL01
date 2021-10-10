/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for tick timer
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#include "spr_defs.h"
#define ITH_GEN_DFT1_REG                0x0072
#define ITH_HOST_CLK1_REG               0x0010
#define ITH_PLL_GEN_SET_REG             0x00D2
#define ITH_PLL1_SET1_REG               0x00A0
#define ITH_PLL1_SET2_REG               0x00A2
#define ITH_PLL1_SET3_REG               0x00A4
#define ITH_PLL1_SET4_REG               0x00A6

#if defined (CFG_CHIP_REV_A0)
#define ITH_HOST_BASE                   0xC0000000
#elif defined (CFG_CHIP_REV_A1)
    #define ITH_HOST_BASE                 0xC0200000
#else
    #define ITH_HOST_BASE                 0xC0200000    
#endif

#define CFG_OSC_CLK 30000000

static int bEnableAudioProcessor=1;
extern int tick_incr;
extern int tick_init;
extern int tick_mode;
static long long ticks = 0;
static unsigned int clkCpu, clkMem, clkBus;
static unsigned short clkMem2Val, clkAhb2Val, clkApb2Val, clkApb3Val, clkIsp1Val;
static unsigned short clkGenRegs[(ITH_GEN_DFT1_REG - ITH_HOST_CLK1_REG + 2) / 2];
static unsigned short clkPllRegs[(ITH_PLL_GEN_SET_REG - ITH_PLL1_SET1_REG + 2) / 2];

/////////////////////////////////////////////////////////////////
// Timer Functions
//
// The timer function is use to count the ticks in 2.68 seconds
// period maximun (count to 0x0fffffff in 100MHz). It dose not
// raise the tick timer exception to count the larger number
// of ticks.
//
// Usage:
//
//    int ticks;
//    start_timer();
//
//    ..... blah blah .....
//
//    ticks = get_timer();
//
/////////////////////////////////////////////////////////////////
int get_timer(void)
{
    int current_ticks = 0;
    if (!bEnableAudioProcessor)
    {
        current_ticks = mfspr(SPR_TTCR4);
        /* reset timer */
        mtspr(SPR_TTCR4, 0);
        return current_ticks;    
    }
    else
    {
        current_ticks = mfspr(SPR_TTCR);
        /* reset timer */
        mtspr(SPR_TTCR, 0);
        return current_ticks;    
    }
}

void reset_timer(void)
{
    if (!bEnableAudioProcessor)
    {
        mtspr(SPR_TTCR4, 0);    
    }
    else
    {
        mtspr(SPR_TTCR, 0);    
    }
}

void stop_timer(void) 
{
    if (!bEnableAudioProcessor)
    {    
        /* Stop Timer */
        mtspr(SPR_TTMR4, 0);    
    }
    else
    {
        /* Stop Timer */
        mtspr(SPR_TTMR, 0);
    }
}

static inline unsigned short ithReadRegH(unsigned short addr)
{
    return *(unsigned short volatile*)(ITH_HOST_BASE + addr);
}

void start_timer(void) 
{
    if (!bEnableAudioProcessor)
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR4, 0);
        /* Reset counter */
        mtspr(SPR_TTCR4, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR4, SPR_TTMR_SR | SPR_TTMR_PERIOD);    
    }
    else
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR, 0);
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_SR | SPR_TTMR_PERIOD);
    }
}

int get_wiegand_timer(int ID)
{
    int current_ticks = 0;
    if (ID == 0)
    {
        current_ticks = mfspr(SPR_TTCR);
        /* reset timer */
        return current_ticks;    
    }
    else if (ID == 1)
    {
        current_ticks = mfspr(SPR_TTCR2);
        /* reset timer */
        return current_ticks;    
    }
    else if (ID == 2)
    {
        current_ticks = mfspr(SPR_TTCR3);
        /* reset timer */
        return current_ticks;    
    }
    else
    {
        current_ticks = mfspr(SPR_TTCR4);
        /* reset timer */
        return current_ticks;    
    }
}

void reset_wiegand_timer(int ID)
{
    if (ID == 0)
    {
        mtspr(SPR_TTCR, 0);    
    }
    else if (ID == 1)
    {
        mtspr(SPR_TTCR2, 0);    
    }
    else if (ID == 2)
    {
        mtspr(SPR_TTCR3, 0);
    }
    else 
    {
        mtspr(SPR_TTCR4, 0);
    }
}

void start_wiegand_timer(int ID) 
{
    if (ID == 0)
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR, 0);
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_SR | SPR_TTMR_PERIOD);
    }
    else if (ID == 1)
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR2, 0);
        /* Reset counter */
        mtspr(SPR_TTCR2, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR2, SPR_TTMR_SR | SPR_TTMR_PERIOD);
    }
    else if (ID == 2)
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR3, 0);
        /* Reset counter */
        mtspr(SPR_TTCR3, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR3, SPR_TTMR_SR | SPR_TTMR_PERIOD);

    }
    else
    {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
        /* stop timer */
        mtspr(SPR_TTMR4, 0);
        /* Reset counter */
        mtspr(SPR_TTCR4, 0);
        /* single run mode and disable interrupt */
        mtspr(SPR_TTMR4, SPR_TTMR_SR | SPR_TTMR_PERIOD);
    }
}

void UART_TIMER_START (void) {
    /* Disable tick timer exception recognition */
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);

    /* stop timer */
    mtspr(SPR_TTMR3, 0);

    /* Reset counter */
    mtspr(SPR_TTCR3, 0);

    /* single run mode and disable interrupt */
    mtspr(SPR_TTMR3, SPR_TTMR_SR | SPR_TTMR_PERIOD);
}

void UART_TIMER_END (void) {
    /* stop timer */
    mtspr(SPR_TTMR3, 0);

    /* Reset counter */
    mtspr(SPR_TTCR3, 0);

}

void UART_DELAY (int k) {
    int ticks;
    do {
        ticks = mfspr(SPR_TTCR3); // get timer
    } while(ticks < k);

}

static unsigned int GetPllFreq(int n)
{
    unsigned int pll = 0;
    unsigned int pllBase = n * 0x10;
    unsigned int reg1 = ithReadRegH(pllBase + ITH_PLL1_SET1_REG);
    unsigned int sdmSel    = (reg1 & (1<<15)) ? 1 : 0;
    unsigned int sdmBypass = (reg1 & (1<<14)) ? 1 : 0;
    unsigned int sdmFix    = ((reg1 & (3<<12)) >> 12) + 3;
    unsigned int preDiv    = (reg1 & 0x1f);

    if (sdmSel)
    { // SDM divider
        unsigned int reg4   = ithReadRegH(pllBase + ITH_PLL1_SET4_REG);
        unsigned int sdm    = (reg4 & 0x7ff);
        unsigned int sdm_dv = (reg4 & (3<<12)) >> 12;

        if (sdm & (1<<10))
            sdm = sdm | 0xfffff800;

        switch(sdm_dv)
        {
        case 0:
            sdm += 16 * 1024;
            break;

        case 1:
            sdm += 17 * 1024;
            break;

        case 2:
            sdm += 18 * 1024;
            break;

        case 3:
            sdm += (unsigned int)(16.5f * 1024);
            break;
        }
        pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * (sdmFix * sdm / 1024.0));
    }
    else
    {
        // fix divider
        unsigned int reg3 = ithReadRegH(pllBase + ITH_PLL1_SET3_REG);
        unsigned int num = reg3 & 0x3ff;
        pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * num);
    }
    return pll;
}

static unsigned int GetPllFreqOut1(int n)
{
    unsigned int pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = srcclk / (ithReadRegH(pllBase + ITH_PLL1_SET2_REG) & 0x7f);
    return clk;
}

static unsigned int GetPllFreqOut2(int n)
{
    unsigned int pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = srcclk / ((ithReadRegH(pllBase + ITH_PLL1_SET2_REG) >> 8) & 0x7f);
    return clk;
}

#define MMIO_NCLK_SRC   (0x0018)
#define NCLK_SRC_BIT      (11)
#define NCLK_SRC              (0x7 << NCLK_SRC_BIT)
#define MMIO_NCLK_DIV   (0x0018)
#define NCLK_DIV_BIT       (0)
#define NCLK_DIV              (0x3FF << NCLK_DIV_BIT)

/*
 * Get CPU clock in Hz
 */
unsigned int PalGetSysClock(void)
{
    if (clkCpu == 0)
    {
        unsigned int src =  (MMIO_Read(MMIO_NCLK_SRC) & NCLK_SRC) >> NCLK_SRC_BIT;
        unsigned int div =  ((MMIO_Read(MMIO_NCLK_DIV) & NCLK_DIV) >> NCLK_DIV_BIT) ;
        unsigned int clk;

        switch (src)
        {
        case 0x1: // From PLL1 output2
            clk = GetPllFreqOut2(0);
            break;

        case 0x2: // From PLL2 output1
            clk = GetPllFreqOut1(1);
            break;

        case 0x3: // From PLL2 output2
            clk = GetPllFreqOut2(1);
            break;

        case 0x4: // From PLL3 output1
            clk = GetPllFreqOut1(2);
            break;

        case 0x5: // From PLL3 output2
            clk = GetPllFreqOut2(2);
            break;

        case 0x6: // From CKSYS (12MHz/30MHz)
            clk = CFG_OSC_CLK;
            break;

        case 0x7: // From Ring OSC (200KHz)
            clk = 200000;
            break;

        case 0x00: // From PLL1 output1 (default)
        default:
            clk = GetPllFreqOut1(0);
            break;
        }
        clkCpu = clk / (div + 1);
    }
    return clkCpu;

}

void MySleep(unsigned long ms)
{
    int i = 0;
    //for(i=0; i<ms*133000/5; i++);
    for(i=0; i<ms*100000; i++) asm volatile("");
}

static __inline void set_audio_processor_timer(int nEnable) 
{
    if (nEnable)
    {
        bEnableAudioProcessor =1;
    }
    else
    {
        bEnableAudioProcessor =0;
    }        
}

#if 0
/////////////////////////////////////////////////////////////////
// Ticks Functions
//
// It's use the tick timer exception to count the 64bits ticks.
//
// Functions: void init_tick(int long_period);
//
//            long_perido       0: Do not use the tick timer exception,
//                                 the period between two get_ticks()
//                                 must less than the 2 seconds.
//                              1: Use the tick timer exception to count
//                                 the long period between two get_ticks()
//                                 functions.
//
// Usage:
//
//    long long old_ticks, new_tikcs;
//
//    init_ticks(1);
//    old_tikcs = get_ticks();
//
//    ..... blah blah .....
//
//    new_ticks = get_ticks();  // the totaly ticks is new_ticks-old_ticks
//
//    ..... blah blah .....
//
//    new_ticks = get_ticks();
//    stop_ticks();
//
/////////////////////////////////////////////////////////////////
static __inline void init_ticks (int long_period) {
    ticks     = 0;
    tick_init = 1;
    tick_mode = long_period;

    if (long_period) {
        /* Enable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);
    
        /* stop timer */
        mtspr(SPR_TTMR, 0);
    
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
    
        /* single run mode and enable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_SR | SPR_TTMR_IE | SPR_TTMR_PERIOD);
    } else {
        /* Disable tick timer exception recognition */
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);
    
        /* stop timer */
        mtspr(SPR_TTMR, 0);
    
        /* Reset counter */
        mtspr(SPR_TTCR, 0);
    
        /* Continuous run mode and disable interrupt */
        mtspr(SPR_TTMR, SPR_TTMR_CR | SPR_TTMR_PERIOD);
    }
}

static __inline long long get_ticks (void) {
    int current_ticks;
    static int last_ticks = 0;

    if (!tick_init) init_ticks(1);
    current_ticks = mfspr(SPR_TTCR);

    if (tick_mode == 1) {
        // Use tick timer exception to count the ticks. It should restart
        // the timer manually.
        ticks += ((long long)current_ticks + ((long long)tick_incr << 28));

        /* reset timer */
        mtspr(SPR_TTCR, 0);
    } else {
        // Use the countinuous timer to count the ticks.
        if (current_ticks < last_ticks) { // Timer is wrapped.
            tick_incr++;
        }
        last_ticks = current_ticks;
        ticks = ((long long)tick_incr << 32) + current_ticks;
    }

    return ticks;
}

static __inline void stop_ticks(void) {
    /* Disable tick timer exception recognition */
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);

    /* Stop Timer */
    mtspr(SPR_TTMR, 0);
}
#endif

