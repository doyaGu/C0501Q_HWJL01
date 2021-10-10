/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL gpio interrupt handler functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../ith_cfg.h"

static ITHGpioIntrHandler gpioHandlerTable[102];
static void* gpioArgTable[102];

static void GpioDefaultHandler(unsigned int pin, void* arg)
{
    // DO NOTHING
}
    
void ithGpioInit(void)
{
    int i;
    
    for (i = 0; i < 102; ++i)
        gpioHandlerTable[i] = GpioDefaultHandler;
}

void ithGpioRegisterIntrHandler(unsigned int pin, ITHGpioIntrHandler handler, void* arg)
{
	if (pin < 102)
	{
        gpioHandlerTable[pin]   = handler ? handler : GpioDefaultHandler;
        gpioArgTable[pin]       = arg;
    }
}

void ithGpioDoIntr(void)
{
    register uint32_t gpioNum   = 0;
    register uint32_t intrSrc1  = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_INTRMASKSTATE_REG);   // read gpio1's intr source
    register uint32_t intrSrc2  = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO2_INTRMASKSTATE_REG);   // read gpio2's intr source
    register uint32_t intrSrc3  = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO3_INTRMASKSTATE_REG);   // read gpio3's intr source
    register uint32_t intrSrc4  = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO4_INTRMASKSTATE_REG);   // read gpio4's intr source
    
#if (CFG_CHIP_FAMILY == 9910  && CFG_WIEGAND0_ENABLE)
    if (CFG_WIEGAND0_GPIO0 < 32)
        intrSrc1 &= (0 << CFG_WIEGAND0_GPIO0);
    else
        intrSrc2 &= (0 << CFG_WIEGAND0_GPIO0);    

    if (CFG_WIEGAND0_GPIO1 < 32)
        intrSrc1 &= (0 << CFG_WIEGAND0_GPIO1);
    else
        intrSrc2 &= (0 << CFG_WIEGAND0_GPIO1);    
#endif
#if (CFG_CHIP_FAMILY == 9910 && CFG_DBG_SWUART_CODEC)
    if (CFG_SWUARTDBGPRINTF_GPIO < 32)
        intrSrc1 &= (0 << CFG_SWUARTDBGPRINTF_GPIO);
    else
        intrSrc2 &= (0 << CFG_SWUARTDBGPRINTF_GPIO);    
#endif
    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO1_INTRCLR_REG, intrSrc1); // clear irq1 source
    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO2_INTRCLR_REG, intrSrc2); // clear irq2 source
    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO3_INTRCLR_REG, intrSrc3); // clear irq3 source
    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO4_INTRCLR_REG, intrSrc4); // clear irq4 source

    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc1 != 0)
    {
        if ((intrSrc1 & 0xFFFF) == 0)
        {
            gpioNum += 16;
            intrSrc1 >>= 16;
        }

        if ((intrSrc1 & 0xFF) == 0)
        {
            gpioNum += 8;
            intrSrc1 >>= 8;
        }

        if ((intrSrc1 & 0xF) == 0)
        {
            gpioNum += 4;
            intrSrc1 >>= 4;
        }

        if ((intrSrc1 & 0x3) == 0)
        {
            gpioNum += 2;
            intrSrc1 >>= 2;
        }

        if ((intrSrc1 & 0x1) == 0)
        {
            gpioNum += 1;
            intrSrc1 >>= 1;
        }

        // Call the handler
        gpioHandlerTable[gpioNum](gpioNum, gpioArgTable[gpioNum]);

        intrSrc1 &= ~1;
    }
    
    gpioNum = 32;

    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc2 != 0)
    {
        if ((intrSrc2 & 0xFFFF) == 0)
        {
            gpioNum += 16;
            intrSrc2 >>= 16;
        }

        if ((intrSrc2 & 0xFF) == 0)
        {
            gpioNum += 8;
            intrSrc2 >>= 8;
        }

        if ((intrSrc2 & 0xF) == 0)
        {
            gpioNum += 4;
            intrSrc2 >>= 4;
        }

        if ((intrSrc2 & 0x3) == 0)
        {
            gpioNum += 2;
            intrSrc2 >>= 2;
        }

        if ((intrSrc2 & 0x1) == 0)
        {
            gpioNum += 1;
            intrSrc2 >>= 1;
        }

        // Call the handler
        gpioHandlerTable[gpioNum](gpioNum, gpioArgTable[gpioNum]);
          
        intrSrc2 &= ~1;
    }

    gpioNum = 64;

    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc3 != 0)
    {
        if ((intrSrc3 & 0xFFFF) == 0)
        {
            gpioNum += 16;
            intrSrc3 >>= 16;
        }

        if ((intrSrc3 & 0xFF) == 0)
        {
            gpioNum += 8;
            intrSrc3 >>= 8;
        }

        if ((intrSrc3 & 0xF) == 0)
        {
            gpioNum += 4;
            intrSrc3 >>= 4;
        }

        if ((intrSrc3 & 0x3) == 0)
        {
            gpioNum += 2;
            intrSrc3 >>= 2;
        }

        if ((intrSrc3 & 0x1) == 0)
        {
            gpioNum += 1;
            intrSrc3 >>= 1;
        }

        // Call the handler
        gpioHandlerTable[gpioNum](gpioNum, gpioArgTable[gpioNum]);
          
        intrSrc3 &= ~1;
    }

    gpioNum = 96;

    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc4 != 0)
    {
        if ((intrSrc4 & 0xFFFF) == 0)
        {
            gpioNum += 16;
            intrSrc4 >>= 16;
        }

        if ((intrSrc4 & 0xFF) == 0)
        {
            gpioNum += 8;
            intrSrc4 >>= 8;
        }

        if ((intrSrc4 & 0xF) == 0)
        {
            gpioNum += 4;
            intrSrc4 >>= 4;
        }

        if ((intrSrc4 & 0x3) == 0)
        {
            gpioNum += 2;
            intrSrc4 >>= 2;
        }

        if ((intrSrc4 & 0x1) == 0)
        {
            gpioNum += 1;
            intrSrc4 >>= 1;
        }

        // Call the handler
        gpioHandlerTable[gpioNum](gpioNum, gpioArgTable[gpioNum]);
          
        intrSrc4 &= ~1;
    }
}
