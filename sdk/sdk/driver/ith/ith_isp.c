/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL ISP functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

void ithIspEnableClock(void)
{
#if (CFG_CHIP_FAMILY == 9920)
    ithWriteRegMaskA(ITH_ISP_CLK1_REG,
        (0x1 << ITH_EN_I0CLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT) | (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT),
        (0x1 << ITH_EN_I0CLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT) | (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT));
    ithSetRegBitA(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
#else
    ithWriteRegMaskH(ITH_ISP_CLK2_REG,
        (0x1 << ITH_EN_N5CLK_BIT) | (0x1 << ITH_EN_M5CLK_BIT) | (0x1 << ITH_EN_ICLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT),
        (0x1 << ITH_EN_N5CLK_BIT) | (0x1 << ITH_EN_M5CLK_BIT) | (0x1 << ITH_EN_ICLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT));
    ithSetRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_ISP_MMIO_BIT);
#endif
}

void ithIspDisableClock(void)
{
#if (CFG_CHIP_FAMILY == 9920)
    ithWriteRegMaskA(ITH_ISP_CLK1_REG,
        (0x0 << ITH_EN_I0CLK_BIT) | (0x0 << ITH_EN_I1CLK_BIT) | (0x0 << ITH_EN_M4CLK_BIT) | (0x0 << ITH_EN_W13CLK_BIT),
        (0x1 << ITH_EN_I0CLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT) | (0x1 << ITH_EN_M4CLK_BIT) | (0x1 << ITH_EN_W13CLK_BIT));
    ithClearRegBitA(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
#else
    ithWriteRegMaskH(ITH_ISP_CLK2_REG,
        (0x0 << ITH_EN_M5CLK_BIT) | (0x0 << ITH_EN_ICLK_BIT) | (0x0 << ITH_EN_I1CLK_BIT),
        (0x1 << ITH_EN_M5CLK_BIT) | (0x1 << ITH_EN_ICLK_BIT) | (0x1 << ITH_EN_I1CLK_BIT));

    if ((ithReadRegH(ITH_CQ_CLK_REG) & (0x1 << ITH_EN_M3CLK_BIT)) == 0)
        ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_EN_N5CLK_BIT);   // disable N5 clock safely

    ithClearRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
    ithClearRegBitH(ITH_EN_MMIO_REG, ITH_EN_ISP_MMIO_BIT);
#endif
}

void ithIspSetClockSource(ITHClockSource src)
{
#if (CFG_CHIP_FAMILY == 9920)
    ithClearRegBitA(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
    ithWriteRegMaskA(ITH_ISP_CLK1_REG, src << ITH_ICLK_SRC_SEL_BIT, ITH_ICLK_SRC_SEL_MASK);
    ithSetRegBitA(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
#else
    ithClearRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
    ithWriteRegMaskH(ITH_ISP_CLK1_REG, src << ITH_ICLK_SRC_SEL_BIT, ITH_ICLK_SRC_SEL_MASK);
    ithSetRegBitH(ITH_ISP_CLK1_REG, ITH_EN_DIV_ICLK_BIT);
#endif
}

void ithIspResetReg(void)
{
#if (CFG_CHIP_FAMILY == 9920)
    ithSetRegBitA(ITH_ISP_CLK1_REG, ITH_ISP_REG_RST_BIT);
    ithClearRegBitA(ITH_ISP_CLK1_REG, ITH_ISP_REG_RST_BIT);
#else
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_REG_RST_BIT);
    ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_REG_RST_BIT);
#endif
}

void ithIspQResetReg(void)
{
#if (CFG_CHIP_FAMILY == 9920)
    ithSetRegBitA(ITH_ISP_CLK1_REG, ITH_ISPQ_RST_BIT);
    ithClearRegBitA(ITH_ISP_CLK1_REG, ITH_ISPQ_RST_BIT);
#else
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_ISPQ_RST_BIT);
    ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_ISPQ_RST_BIT);
#endif
}

void ithIspResetEngine(void)
{
#if (CFG_CHIP_FAMILY == 9920)
    ithSetRegBitA(ITH_ISP_CLK1_REG, ITH_ISP_RST_BIT);
    ithClearRegBitA(ITH_ISP_CLK1_REG, ITH_ISP_RST_BIT);
#else
    ithSetRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_RST_BIT);
    ithClearRegBitH(ITH_ISP_CLK2_REG, ITH_ISP_RST_BIT);
#endif
}

