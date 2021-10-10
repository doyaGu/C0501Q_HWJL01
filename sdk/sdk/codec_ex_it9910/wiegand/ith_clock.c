/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL clock functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static unsigned int clkCpu, clkMem, clkBus;
static uint16_t clkMem2Val, clkAhb2Val, clkApb2Val, clkApb3Val, clkIsp1Val;
static uint16_t clkGenRegs[(ITH_GEN_DFT1_REG - ITH_HOST_CLK1_REG + 2) / 2];
static uint16_t clkPllRegs[(ITH_PLL_GEN_SET_REG - ITH_PLL1_SET1_REG + 2) / 2];

static unsigned int GetPllFreq(int n)
{
    unsigned int pll = 0;
    uint32_t pllBase = n * 0x10;
    uint32_t reg1 = ithReadRegH(pllBase + ITH_PLL1_SET1_REG);
    uint32_t sdmSel    = (reg1 & (1<<15)) ? 1 : 0;
    uint32_t sdmBypass = (reg1 & (1<<14)) ? 1 : 0;
    uint32_t sdmFix    = ((reg1 & (3<<12)) >> 12) + 3;
    uint32_t preDiv    = (reg1 & 0x1f);

    if (sdmSel)
    { // SDM divider
        uint32_t reg4   = ithReadRegH(pllBase + ITH_PLL1_SET4_REG);
        uint32_t sdm    = (reg4 & 0x7ff);
        uint32_t sdm_dv = (reg4 & (3<<12)) >> 12;

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
            sdm += (uint32_t)(16.5f * 2048);
            break;
        }

        if (sdm_dv != 3)
            pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * (sdmFix * sdm / 1024.0f));
        else
            pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * (sdmFix * sdm / 2048.0f));

    }
    else
    {
        // fix divider
        uint32_t reg3 = ithReadRegH(pllBase + ITH_PLL1_SET3_REG);
        uint32_t num = reg3 & 0x3ff;
        pll = (unsigned int)((float)(CFG_OSC_CLK / preDiv) * num);
    }
    return pll;
}

static unsigned int GetPllFreqOut1(int n)
{
    uint32_t pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = srcclk / (ithReadRegH(pllBase + ITH_PLL1_SET2_REG) & 0x7f);
    return clk;
}

static unsigned int GetPllFreqOut2(int n)
{
    uint32_t pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = srcclk / ((ithReadRegH(pllBase + ITH_PLL1_SET2_REG) >> 8) & 0x7f);
    return clk;
}

void ithClockInit(void)
{
    clkCpu = clkMem = clkBus = 0;
}

void ithClockSleep(void)
{
    // backup original register's values
    clkMem2Val = ithReadRegH(ITH_MEM_CLK2_REG);
    clkAhb2Val = ithReadRegH(ITH_AHB_CLK2_REG);
    clkApb2Val = ithReadRegH(ITH_APB_CLK2_REG);
    clkApb3Val = ithReadRegH(ITH_APB_CLK3_REG);
    clkIsp1Val = ithReadRegH(ITH_ISP_CLK1_REG);

    // disable clocks
    ithClearRegBitH(ITH_MEM_CLK2_REG, ITH_EN_N2CLK_BIT);
    ithClearRegBitH(ITH_AHB_CLK2_REG, ITH_EN_N1CLK_BIT);

    ithClearRegBitH(ITH_APB_CLK2_REG, ITH_EN_W5CLK_BIT);
    ithClearRegBitH(ITH_APB_CLK2_REG, ITH_EN_W3CLK_BIT);
    ithClearRegBitH(ITH_APB_CLK2_REG, ITH_EN_W2CLK_BIT);

    ithClearRegBitH(ITH_APB_CLK3_REG, ITH_EN_W10CLK_BIT);
    ithClearRegBitH(ITH_APB_CLK3_REG, ITH_EN_W9CLK_BIT);
    ithClearRegBitH(ITH_APB_CLK3_REG, ITH_EN_W8CLK_BIT);
    ithClearRegBitH(ITH_APB_CLK3_REG, ITH_EN_W7CLK_BIT);
    ithClearRegBitH(ITH_APB_CLK3_REG, ITH_EN_W6CLK_BIT);
}

void ithClockWakeup(void)
{
    // restore original register's values
    ithWriteRegH(ITH_MEM_CLK2_REG, clkMem2Val);
    ithWriteRegH(ITH_AHB_CLK2_REG, clkAhb2Val);
    ithWriteRegH(ITH_APB_CLK2_REG, clkApb2Val);
    ithWriteRegH(ITH_APB_CLK3_REG, clkApb3Val);
    ithWriteRegH(ITH_ISP_CLK1_REG, clkIsp1Val);
}

void ithClockSuspend(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(clkGenRegs); i++)
        clkGenRegs[i] = ithReadRegH(ITH_HOST_CLK1_REG + i * 2);

    for (i = 0; i < ITH_COUNT_OF(clkPllRegs); i++)
        clkPllRegs[i] = ithReadRegH(ITH_PLL1_SET1_REG + i * 2);
}

void ithClockResume(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(clkGenRegs); i++)
        ithWriteRegH(ITH_HOST_CLK1_REG + i * 2, clkGenRegs[i]);

    for (i = 0; i < ITH_COUNT_OF(clkPllRegs); i++)
        ithWriteRegH(ITH_PLL1_SET1_REG + i * 2, clkPllRegs[i]);
}

/*
 * Get CPU clock in Hz
 */
unsigned int ithGetCpuClock(void)
{
#ifdef CFG_ITH_FPGA
    return CFG_ITH_FPGA_CLK_CPU;

#else
    if (clkCpu == 0)
    {
    #ifdef __SM32__
        unsigned int src = (ithReadRegH(ITH_AHB_CLK1_REG) & ITH_NCLK_SRC_SEL_MASK) >> ITH_NCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_AHB_CLK1_REG) & ITH_NCLK_RATIO_MASK) >> ITH_NCLK_RATIO_BIT;
    #else
        unsigned int src = (ithReadRegH(ITH_ARM_CLK1_REG) & ITH_FCLK_SRC_SEL_MASK) >> ITH_FCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_ARM_CLK1_REG) & ITH_FCLK_RATIO_MASK) >> ITH_FCLK_RATIO_BIT;
    #endif
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

#endif // CFG_ITH_FPGA
}

/*
 * Get memory clock in Hz
 */
unsigned int ithGetMemClock(void)
{
#ifdef CFG_ITH_FPGA
    return CFG_ITH_FPGA_CLK_MEM;

#else
    if (clkMem == 0)
    {
        unsigned int src = (ithReadRegH(ITH_MEM_CLK1_REG) & ITH_MCLK_SRC_SEL_MASK) >> ITH_MCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_MEM_CLK1_REG) & ITH_MCLK_RATIO_MASK) >> ITH_MCLK_RATIO_BIT;
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
        clkMem = clk / (div + 1);
    }
    return clkMem;

#endif // CFG_ITH_FPGA
}

/*
 * Get bus clock in Hz
 */
unsigned int ithGetBusClock(void)
{
#ifdef CFG_ITH_FPGA
    return CFG_ITH_FPGA_CLK_BUS;

#else
    if (clkBus == 0)
    {
        unsigned int src = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_SRC_SEL_MASK) >> ITH_WCLK_SRC_SEL_BIT;
        unsigned int div = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) >> ITH_WCLK_RATIO_BIT;
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
        clkBus = clk / (div + 1);
    }
    return clkBus;

#endif // CFG_ITH_FPGA
}

void ithClockSetSource(ITHClock clk, ITHClockSource src)
{
    ithWriteRegMaskH(clk,
        (0x1 << ITH_MCLK_UPD_BIT) | (src << ITH_MCLK_SRC_SEL_BIT),
        ITH_MCLK_UPD_MASK | ITH_MCLK_SRC_SEL_MASK);

    //ithClearRegBitH(clk, ITH_MCLK_UPD_BIT);
    ithClockInit();
}

void ithClockSetRatio(ITHClock clk, unsigned int ratio)
{
    ithWriteRegMaskH(clk,
        (0x1 << ITH_MCLK_UPD_BIT) | (ratio << ITH_MCLK_RATIO_BIT),
        ITH_MCLK_UPD_MASK | ITH_MCLK_RATIO_MASK);

    //ithClearRegBitH(clk, ITH_MCLK_UPD_BIT);
    ithClockInit();
}

void ithClockEnablePll(ITHPll pll)
{
    ithWriteRegMaskH(ITH_PLL1_SET3_REG + pll * 0x10,
        (0x0 << ITH_PLL1_PD_BIT) | (0x1 << ITH_PLL1_UPDATE_BIT),
        ITH_PLL1_PD_MASK | ITH_PLL1_UPDATE_MASK);

    //ithClearRegBitH(pll, ITH_PLL1_UPDATE_BIT);
}

void ithClockDisablePll(ITHPll pll)
{
    ithWriteRegMaskH(ITH_PLL1_SET3_REG + pll * 0x10,
        (0x1 << ITH_PLL1_PD_BIT) | (0x1 << ITH_PLL1_UPDATE_BIT),
        ITH_PLL1_PD_MASK | ITH_PLL1_UPDATE_MASK);

    //ithClearRegBitH(pll, ITH_PLL1_UPDATE_BIT);
}

void ithSetSpreadSpectrum(ITHPll pll, ITHSpreadSpectrumMode mode, uint32_t width, uint32_t freq)
{
    #define DIV_ROUND(a,b) (((a) + ((b)/2)) / (b))

    int sp_stepy, sp_stepx, sp_level, sp_n, n1, n2, SDM_Sel, sdm, sdm_dv;
    uint32_t PLL_BASE, reg1, reg4, reg5, reg6;

    PLL_BASE = pll * 0x10;
    reg1 = ithReadRegH(PLL_BASE + ITH_PLL1_SET1_REG);
    SDM_Sel = (reg1 & (1<<15)) ? 1 : 0;
    if (!SDM_Sel)
    {
        LOG_ERR "it dose not support spread spectrum on non-SDM PLL; reg 0x%X=0x%X\n",
            PLL_BASE + ITH_PLL1_SET1_REG, reg1
        LOG_END
        return;
    }

    reg4   = ithReadRegH(PLL_BASE + ITH_PLL1_SET4_REG);
    sdm    = (reg4 & 0x7FF);
    sdm_dv = (reg4 & (3<<12)) >> 12;
    switch(sdm_dv)
    {
        case 0: sdm += (int)(16 * 1024); break;
        case 1: sdm += (int)(17 * 1024); break;
        case 2: sdm += (int)(18 * 1024); break;
        case 3: sdm += (int)(16.5f * 2048); break;
    }

    switch(mode)
    {
        case 0: sp_n = 2; break; // up-spread
        case 1: sp_n = 2; break; // down-spread
        case 2: sp_n = 4; break; // center-spread
        default:
            LOG_ERR "unknown mode of spread spectrum\n" LOG_END
            return;
    }

    n1 = DIV_ROUND(width * sdm, 1000);
    n2 = DIV_ROUND(CFG_OSC_CLK, freq * sp_n);

    // search step x and level by step y.
    for(sp_stepy=1; sp_stepy<=63; sp_stepy++)
    {
        sp_stepx = 0;
        sp_level = DIV_ROUND(n1, sp_stepy);
        if (sp_level < 2 || sp_level > 511) continue;

        sp_stepx = DIV_ROUND(n2, sp_level) - 1;
        if (sp_stepx < 0 || sp_stepx > 63) continue;

        break;
    }

    // Is it out of range on step y?
    if (sp_stepy >= 63)
    {
        LOG_ERR "setting spread spectrum fails\n" LOG_END
        return;
    }

    // setting step x, step y and level
    reg5 = sp_level;
    reg6 = (1 << 15) + (mode << 12) + (sp_stepx << 6) + sp_stepy;
    ithWriteRegMaskH(PLL_BASE + ITH_PLL1_SET6_REG, (0 << 15), (1 << 15));
    ithWriteRegH(PLL_BASE + ITH_PLL1_SET5_REG, reg5);
    ithWriteRegH(PLL_BASE + ITH_PLL1_SET6_REG, reg6);

    LOG_INFO "PLL %d spread spectrum: modulation width=%4.2f%%, modulation frequency=%d\n", pll, (float)sp_stepy*sp_level*100.0f/sdm, CFG_OSC_CLK/(sp_n*sp_level*(sp_stepx+1)) LOG_END
}

void ithClockStats(void)
{
#ifdef CFG_ITH_FPGA
//    PRINTF("CLK: cpu=%lu,mem=%lu,bus=%lu\r\n", ithGetCpuClock(), ithGetMemClock(), ithGetBusClock());
#else
//    PRINTF("CLK: cpu=%lu,mem=%lu,bus=%lu,pll1=%lu,pll2=%lu,pll3=%lu\r\n",
//        ithGetCpuClock(), ithGetMemClock(), ithGetBusClock(), GetPllFreq(0), GetPllFreq(1), GetPllFreq(2));
#endif // CFG_ITH_FPGA

    //ithPrintRegH(ITH_HTRAP_REG, ITH_OPVG_CLK3_REG - ITH_HTRAP_REG + sizeof (uint16_t));
    //ithPrintRegH(ITH_PLL1_SET1_REG, ITH_PLL3_SET3_REG - ITH_PLL1_SET1_REG + sizeof (uint16_t));
}
