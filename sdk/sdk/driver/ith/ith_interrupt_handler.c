/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL interrupt handler functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"
#ifdef __OPENRTOS__
#include "openrtos/portmacro.h"
#endif
#ifdef __SM32__
#include "openrtos/sm32/port_spr_defs.h"
#endif

static ITHIntrHandler irqHandlerTable[64];
static ITHIntrHandler fiqHandlerTable[64];
static void* irqArgTable[64];
static void* fiqArgTable[64];

static void intrDefaultHandler(void* arg)
{
    // DO NOTHING
}
    
void ithIntrInit(void)
{
    int i;
    
    for (i = 0; i < 64; ++i)
        irqHandlerTable[i] = fiqHandlerTable[i] = intrDefaultHandler;
}

void ithIntrRegisterHandlerIrq(ITHIntr intr, ITHIntrHandler handler, void* arg)
{
	if (intr < 64)
	{
        irqHandlerTable[intr]   = handler ? handler : intrDefaultHandler;
        irqArgTable[intr]       = arg;
    }
}

#ifdef __arm__
void ithIntrDoIrq( void ) __attribute__((naked));
#endif
void ithIntrDoIrq(void)
{
#ifdef __arm__
    portSAVE_CONTEXT();
    
    asm volatile(                                           \
"   mov     r0, #0xff0                                \n\t"	\
"   mov     r1, fp                                    \n\t"	\
"   str     r1, [r0, #0x0]                            \n\t"	\
"   mov     r0, #0xf00                                \n\t"	\
"   orr     r0, #0x0ec                                \n\t"	\
"   mov     r1, lr                                    \n\t"	\
"   str     r1, [r0, #0x0]                            \n\t"	\
    :                                                       \
    :                                                       \
    : "memory", "r0", "r1"                                  \
    );                                                       
    
#endif
    // WARNING - Do not use local (stack) variables here.  Use globals if you must!

    register uint32_t intrNum    = 0;
    register uint32_t intrSrc1   = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_STATUS_REG);    // read irq1 source
    register uint32_t intrSrc2   = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_STATUS_REG);    // read irq2 source
    
    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc1 != 0)
    {
        if ((intrSrc1 & 0xFFFF) == 0)
        {
            intrNum += 16;
            intrSrc1 >>= 16;
        }

        if ((intrSrc1 & 0xFF) == 0)
        {
            intrNum += 8;
            intrSrc1 >>= 8;
        }

        if ((intrSrc1 & 0xF) == 0)
        {
            intrNum += 4;
            intrSrc1 >>= 4;
        }

        if ((intrSrc1 & 0x3) == 0)
        {
            intrNum += 2;
            intrSrc1 >>= 2;
        }

        if ((intrSrc1 & 0x1) == 0)
        {
            intrNum += 1;
            intrSrc1 >>= 1;
        }

		ithIntrClearIrq(intrNum);

        // Get the vector and process the interrupt
        irqHandlerTable[intrNum](irqArgTable[intrNum]);
        
        intrSrc1 &= ~1;
    }
    
    intrNum = 32;

    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc2 != 0)
    {
        if ((intrSrc2 & 0xFFFF) == 0)
        {
            intrNum += 16;
            intrSrc2 >>= 16;
        }

        if ((intrSrc2 & 0xFF) == 0)
        {
            intrNum += 8;
            intrSrc2 >>= 8;
        }

        if ((intrSrc2 & 0xF) == 0)
        {
            intrNum += 4;
            intrSrc2 >>= 4;
        }

        if ((intrSrc2 & 0x3) == 0)
        {
            intrNum += 2;
            intrSrc2 >>= 2;
        }

        if ((intrSrc2 & 0x1) == 0)
        {
            intrNum += 1;
            intrSrc2 >>= 1;
        }

		ithIntrClearIrq(intrNum);

        // Get the vector and process the interrupt
        irqHandlerTable[intrNum](irqArgTable[intrNum]);
        
        intrSrc2 &= ~1;
    }

#ifdef __SM32__
	mtspr(SPR_PICSR, 0);	// clear interrupt status: all modules have level interrupts, which have to be cleared by software,
                          	// thus this is safe, since non processed interrupts will get re-asserted soon enough
#endif
#ifdef __arm__
    portRESTORE_CONTEXT();
#endif
}

void ithIntrRegisterHandlerFiq(ITHIntr intr, ITHIntrHandler handler, void* arg)
{
	if (intr < 64)
	{
        fiqHandlerTable[intr]   = handler ? handler : intrDefaultHandler;
        fiqArgTable[intr]       = arg;
    }
}

#ifdef __arm__
void ithIntrDoFiq( void ) __attribute__((naked));
#endif
void ithIntrDoFiq(void)
{
#ifdef __arm__
    portSAVE_CONTEXT();
#endif

    // WARNING - Do not use local (stack) variables here.  Use globals if you must!

    register uint32_t intrNum     = 0;
    register uint32_t intrSrc1   = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_STATUS_REG);    // read fiq1 source
    register uint32_t intrSrc2   = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_STATUS_REG);    // read fiq2 source

    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc1 != 0)
    {
        if ((intrSrc1 & 0xFFFF) == 0)
        {
            intrNum += 16;
            intrSrc1 >>= 16;
        }

        if ((intrSrc1 & 0xFF) == 0)
        {
            intrNum += 8;
            intrSrc1 >>= 8;
        }

        if ((intrSrc1 & 0xF) == 0)
        {
            intrNum += 4;
            intrSrc1 >>= 4;
        }

        if ((intrSrc1 & 0x3) == 0)
        {
            intrNum += 2;
            intrSrc1 >>= 2;
        }

        if ((intrSrc1 & 0x1) == 0)
        {
            intrNum += 1;
            intrSrc1 >>= 1;
        }

        ithIntrClearFiq(intrNum);

        // Get the vector and process the interrupt
        fiqHandlerTable[intrNum](fiqArgTable[intrNum]);
          
        intrSrc1 &= ~1;
    }
    
    intrNum = 32;

    // Test to see if there is a flag set, if not then don't do anything
    while (intrSrc2 != 0)
    {
        if ((intrSrc2 & 0xFFFF) == 0)
        {
            intrNum += 16;
            intrSrc2 >>= 16;
        }

        if ((intrSrc2 & 0xFF) == 0)
        {
            intrNum += 8;
            intrSrc2 >>= 8;
        }

        if ((intrSrc2 & 0xF) == 0)
        {
            intrNum += 4;
            intrSrc2 >>= 4;
        }

        if ((intrSrc2 & 0x3) == 0)
        {
            intrNum += 2;
            intrSrc2 >>= 2;
        }

        if ((intrSrc2 & 0x1) == 0)
        {
            intrNum += 1;
            intrSrc2 >>= 1;
        }

        ithIntrClearFiq(intrNum);

        // Get the vector and process the interrupt
        fiqHandlerTable[intrNum](fiqArgTable[intrNum]);
          
        intrSrc2 &= ~1;
    }
#ifdef __SM32__
	mtspr(SPR_PICSR, 0);	// clear interrupt status: all modules have level interrupts, which have to be cleared by software,
                          	// thus this is safe, since non processed interrupts will get re-asserted soon enough
#endif
#ifdef __arm__
    portRESTORE_CONTEXT();
#endif
}
