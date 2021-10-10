/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL USB functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

void ithUsbSuspend(ITHUsbModule usb)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    switch (usb)
    {
    case ITH_USB0:
        ithClearRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_OSC_OUT_EN_BIT);
        ithClearRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_PLL_ALIV_BIT);
        break;

    case ITH_USB1:
        ithClearRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_OSC_OUT_EN_BIT);
        ithClearRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_PLL_ALIV_BIT);
        break;
    }
#else
    ithWriteRegMaskA(usb + 0x3C, 0x0, 0x14);
#endif
    ithSetRegBitA(usb + ITH_USB_HC_MISC_REG, ITH_USB_HOSTPHY_SUSPEND_BIT);
}

void ithUsbResume(ITHUsbModule usb)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    switch (usb)
    {
    case ITH_USB0:
        ithSetRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_OSC_OUT_EN_BIT);
        ithSetRegBitH(ITH_USB0_PHY_CTRL_REG, ITH_USB0_PHY_PLL_ALIV_BIT);
        break;

    case ITH_USB1:
        ithSetRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_OSC_OUT_EN_BIT);
        ithSetRegBitH(ITH_USB1_PHY_CTRL_REG, ITH_USB1_PHY_PLL_ALIV_BIT);
        break;
    }
#else
    ithWriteRegMaskA(usb + 0x3C, 0x14, 0x14);
#endif
    ithClearRegBitA(usb + ITH_USB_HC_MISC_REG, ITH_USB_HOSTPHY_SUSPEND_BIT);
}

void ithUsbEnableClock(void)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    ithSetRegBitH(ITH_USB_CLK_REG, ITH_EN_N6CLK_BIT);
    ithSetRegBitH(ITH_USB_CLK_REG, ITH_EN_M11CLK_BIT);
#else
    ithWriteRegMaskA(ITH_HOST_BASE + 0x64, 0xA, 0xA);
#endif
}

void ithUsbDisableClock(void)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    ithClearRegBitH(ITH_USB_CLK_REG, ITH_EN_N6CLK_BIT);
    ithClearRegBitH(ITH_USB_CLK_REG, ITH_EN_M11CLK_BIT);
#else
	ithWriteRegMaskA(ITH_HOST_BASE + 0x64, 0x0, 0xA);
#endif
}

void ithUsbReset(void)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    ithWriteRegH(0x46, 0x100A);
    usleep(5*1000);
    ithWriteRegH(0x46, 0x000A);
    usleep(5*1000);
#else
	ithWriteRegA(ITH_HOST_BASE + 0x64, 0x8000000A);
    usleep(5*1000);
	ithWriteRegA(ITH_HOST_BASE + 0x64, 0x0000000A);
    usleep(5*1000);
#endif
}

void ithUsbInterfaceSel(ITHUsbInterface intf)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
	if (intf == ITH_USB_AMBA)
		ithWriteRegMaskH(0x900, (0x1 << 8), (0x1 << 8));  // for usb AMBA path
	else 
	{
		ithWriteRegMaskH(0x900, (0x0 << 8), (0x1 << 8));  // for usb wrap path
		
		/** for performance issue */
		#if (CFG_CHIP_FAMILY == 9850)
		ithWriteRegH(0x914, 0x3508);
		ithWriteRegH(0x916, 0xF);
		#elif (CFG_CHIP_FAMILY == 9070)
		ithWriteRegH(0x914, 0x4627);
		ithWriteRegH(0x916, 0x8);
		#endif
	}
#else
    if (intf == ITH_USB_AMBA)
		ithWriteRegMaskA(ITH_USB0_BASE + 0x34, 0x1, 0x1);
	else
		ithWriteRegMaskA(ITH_USB0_BASE + 0x34, 0x0, 0x1);
#endif
}

void ithUsbPhyPowerOn(ITHUsbModule usb)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
	switch (usb)
	{
	case ITH_USB0:
        #if (CFG_CHIP_FAMILY == 9070)
        ithWriteRegMaskH(0x904, 0x40, 0x40);  /** D[6]=1 : usb0 PLL 30MHz */
        #endif
        ithWriteRegMaskH(0x904, 0x10, 0x10);  /** D[4]=1 : usb0 phy power on */
        break;
	
	case ITH_USB1:
        #if (CFG_CHIP_FAMILY == 9070)
        ithWriteRegMaskH(0x90C, 0x40, 0x40);  /** D[6]=1 : usb1 PLL 30MHz */
        #endif
        ithWriteRegMaskH(0x90C, 0x10, 0x10);  /** D[4]=1 : usb1 phy power on */
        break;
	}
#else
    ithWriteRegMaskA(usb + 0x3C, 0x1, 0x1);
#endif
}


