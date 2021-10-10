/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 *
 * @description Used as System Time Clock in front panel control.
 * @file ${Program}/ith_stc.c
 * @author Irene Wang
 * @version 1.0.0
 */
#include "ite/ith.h"

#define VIDEO_OUTPUT_FREQUENCY		27

static uint32_t stcBaseCountLo;
STCInfo stc_info[STC_MAX_CNT] = {0};

int getFrequency(void)
{
	int value;
	int pclk_src, pclk_ratio;
	int pll_div, pll_num;

	// Read PCR Clock source and ratio
	value = ithReadRegH(0x0040);
	pclk_src = (value >> 12) & 0x7;
	pclk_ratio = (value & 0xF) + 1;

	// Read PLL setting
	switch (pclk_src)
	{
		case 0:
		case 1:
			// PLL 1
			pll_num = ithReadRegH(0x00A4) & 0x3FF;
			value = ithReadRegH(0x00A2);
			if (pclk_src == 0) // n1
				pll_div = value & 0x7F;
			else // n2
				pll_div = (value >> 8) & 0x7F;
			break;
		case 2:
		case 3:
			// PLL 2
			pll_num = ithReadRegH(0x00B4) & 0x3FF;
			value = ithReadRegH(0x00B2);
			if (pclk_src == 0) // n1
				pll_div = value & 0x7F;
			else // n2
				pll_div = (value >> 8) & 0x7F;
			break;
		case 4:
		case 5:
			// PLL 3
			pll_num = ithReadRegH(0x00C4) & 0x3FF;
			value = ithReadRegH(0x00C2);
			if (pclk_src == 0) // n1
				pll_div = value & 0x7F;
			else // n2
				pll_div = (value >> 8) & 0x7F;
			break;
		default:
			break;
	};

	//printf("%s %d %d %d\n", __FUNCTION__, pll_num, pll_div, pclk_ratio);
	return (int) (((float)pll_num / pll_div) / pclk_ratio);
}
	
void ithFpcReset(void)
{
	// Enable FPC PCLK
	ithSetRegBitH(0x42, 0x1); // Enable PCLK
	ithWriteRegH(0x40, 0x8009); // Set PCLK_Ratio and enable
	ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_FPC_MMIO_BIT);

	ithStcSetVideoOutFreq(getFrequency(), 0);

	// Reset->Fire
	// Notice : before fire, disable reset bit first
	ithStcCtrlDisable(ITH_STC_FIRE);
	ithStcCtrlEnable(ITH_STC_RESET);
	ithStcCtrlDisable(ITH_STC_RESET);
	ithStcCtrlEnable(ITH_STC_FIRE);

	//stcBaseCountHi = stcBaseCountLo = 0; //Benson
}

uint64_t ithStcGetBaseClock64(STCInfo *pstc_info)
{
     //printf("[%s] hi=%u lo = %u\n", __FUNCTION__, pstc_info->stcBaseCountHi, stcBaseCountLo);
     return (((uint64_t)pstc_info->stcBaseCountHi << 32) | stcBaseCountLo);
}

void ithStcUpdateBaseClock(void)
{
    int index;
	uint32_t baseCount = ithStcGetBaseClock();

    if (baseCount < stcBaseCountLo)
    {
        for (index = 0; index < STC_MAX_CNT; index++)
        {
            if (stc_info[index].state != STATE_FREE)
            {
                stc_info[index].stcBaseCountHi++;
            }
        }
    }
    stcBaseCountLo = baseCount;
	//ithPrintf("[%s] hi=%u, lo=%u,REG1D24=%ld ,index=%d\n", __FUNCTION__, stc_info[index].stcBaseCountHi, stcBaseCountLo,ithReadRegH(ITH_STC_BASECNT_LO_REG),index);
}
