/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL JPEG functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"


void
ithJpegVideoEnableClock(
    void)
{
    // enable clock
    #if (CFG_CHIP_FAMILY == 9920)
	ithSetRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_W23CLK_BIT);
	ithSetRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_A3CLK_BIT);
	ithSetRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_N3CLK_BIT);
	ithSetRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_M5CLK_BIT);
	ithSetRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_XCLK_BIT);
	ithSetRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_XX2CLK_BIT);
	ithSetRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_DIV_XXCLK_BIT);
	#else
    ithSetRegBitH(ITH_VIDEO_CLK2_REG, ITH_EN_M7CLK_BIT);
    ithSetRegBitH(ITH_VIDEO_CLK2_REG, ITH_EN_XCLK_BIT);
    ithSetRegBitH(ITH_VIDEO_CLK1_REG, ITH_EN_DIV_XCLK_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_VIDEO_MMIO_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_MPEG_MMIO_BIT);
	#endif
}

void
ithJpegVideoDisableClock(
    void)
{
    // disable clock
    #if (CFG_CHIP_FAMILY == 9920)
	ithClearRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_W23CLK_BIT);
	ithClearRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_A3CLK_BIT);
	ithClearRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_N3CLK_BIT);
	ithClearRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_M5CLK_BIT);
	ithClearRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_XCLK_BIT);
	ithClearRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_XX2CLK_BIT);
	ithClearRegBitA(ITH_VIDEO_CLK1_REG,ITH_EN_DIV_XXCLK_BIT);
	#else
    ithClearRegBitH(ITH_VIDEO_CLK2_REG, ITH_EN_M7CLK_BIT);
    ithClearRegBitH(ITH_VIDEO_CLK2_REG, ITH_EN_XCLK_BIT);
    ithClearRegBitH(ITH_VIDEO_CLK1_REG, ITH_EN_DIV_XCLK_BIT);
    ithClearRegBitH(ITH_EN_MMIO_REG, ITH_EN_VIDEO_MMIO_BIT);
    ithClearRegBitH(ITH_EN_MMIO_REG, ITH_EN_MPEG_MMIO_BIT);
	#endif
}


void ithJpegEnableClock(void)
{
    // enable clock
    #if (CFG_CHIP_FAMILY == 9920)
	ithSetRegBitA(ITH_JPEG_CLK_REG, ITH_EN_W14CLK_BIT);
    ithSetRegBitA(ITH_JPEG_CLK_REG, ITH_EN_M6CLK_BIT);
    ithSetRegBitA(ITH_JPEG_CLK_REG, ITH_EN_JCLK_BIT);
	ithSetRegBitA(ITH_JPEG_CLK_REG, ITH_EN_DIV_JCLK_BIT);
	#else
    ithSetRegBitH(ITH_JPEG_CLK_REG, ITH_EN_JCLK_BIT);
    ithSetRegBitH(ITH_JPEG_CLK_REG, ITH_EN_DIV_JCLK_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_JPEG_MMIO_BIT);
	#endif
}

void ithJpegDisableClock(void)
{
    // disable clock
    #if (CFG_CHIP_FAMILY == 9920)
    ithClearRegBitA(ITH_JPEG_CLK_REG, ITH_EN_W14CLK_BIT);
    ithClearRegBitA(ITH_JPEG_CLK_REG, ITH_EN_M6CLK_BIT);
    ithClearRegBitA(ITH_JPEG_CLK_REG, ITH_EN_JCLK_BIT);
	ithClearRegBitA(ITH_JPEG_CLK_REG, ITH_EN_DIV_JCLK_BIT);
    #else
    ithClearRegBitH(ITH_JPEG_CLK_REG, ITH_EN_JCLK_BIT);
    ithClearRegBitH(ITH_EN_MMIO_REG, ITH_EN_JPEG_MMIO_BIT);
	#endif
}

void ithJpegResetReg(void)
{
	#if (CFG_CHIP_FAMILY == 9920)
	ithSetRegBitA(ITH_JPEG_CLK_REG, ITH_JPEG_RST_BIT);
	ithClearRegBitA(ITH_JPEG_CLK_REG, ITH_JPEG_RST_BIT);
	#else
    ithSetRegBitH(ITH_VIDEO_CLK2_REG, ITH_JPEG_REG_RST_BIT);
    ithClearRegBitH(ITH_VIDEO_CLK2_REG, ITH_JPEG_REG_RST_BIT);
	#endif
}

void ithJpegResetEngine(void)
{
	#if (CFG_CHIP_FAMILY == 9920)
	ithSetRegBitA(ITH_JPEG_CLK_REG, ITH_JPEG_REG_RST_BIT);
	ithClearRegBitA(ITH_JPEG_CLK_REG, ITH_JPEG_REG_RST_BIT);
	#else
    ithSetRegBitH(ITH_VIDEO_CLK2_REG, ITH_JPEG_RST_BIT);
    ithClearRegBitH(ITH_VIDEO_CLK2_REG, ITH_JPEG_RST_BIT);
	#endif
}
