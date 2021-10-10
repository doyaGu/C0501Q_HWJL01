/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Chip ID functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static ITHPackageId packageId = -1;

ITHPackageId ithGetPackageId(void)
{
    if (packageId != -1)
        return packageId;

#if CFG_CHIP_FAMILY == 9070
    // disable htrap[2:0] pull-up 
    ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 0 << ITH_GPIO_HTRAP_PULLUP_BIT, 7 << ITH_GPIO_HTRAP_PULLUP_BIT); 
    
    // enable htrap[1:0] pull-down & disable htrap[2] pull-down 
    ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 3 << ITH_GPIO_HTRAP_PULLDOWN_BIT, 7 << ITH_GPIO_HTRAP_PULLDOWN_BIT); 

    // read htrap[2:0]
    packageId = (ithReadRegH(ITH_HTRAP_REG) & ITH_HTRAP_MASK) >> ITH_HTRAP_BIT; 
    
    if (packageId & 1) // disable pull-down htrap[0] if htrap [0] == 1 
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 0 << ITH_GPIO_HTRAP_PULLDOWN_BIT, 1 << ITH_GPIO_HTRAP_PULLDOWN_BIT); 
        
    if (packageId & 2) // disable pull-down htrap[1] if htrap [1] == 1 
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 0 << (ITH_GPIO_HTRAP_PULLDOWN_BIT + 1), 1 << (ITH_GPIO_HTRAP_PULLDOWN_BIT + 1)); 

#elif CFG_CHIP_FAMILY == 9850
    // read htrap[2:0]
    packageId = (ithReadRegH(ITH_HTRAP_REG) & ITH_HTRAP_MASK) >> ITH_HTRAP_BIT; 
    
    if (!(packageId & 1)) { // pull-down htrap[0] if htrap [0] == 0
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 0 << ITH_GPIO_HTRAP_PULLUP_BIT, 1 << ITH_GPIO_HTRAP_PULLUP_BIT); 
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 1 << ITH_GPIO_HTRAP_PULLDOWN_BIT, 1 << ITH_GPIO_HTRAP_PULLDOWN_BIT); 
    }
        
    if (!(packageId & 2)) { // pull-down htrap[1] if htrap [1] == 0
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 0 << (ITH_GPIO_HTRAP_PULLUP_BIT + 1), 1 << (ITH_GPIO_HTRAP_PULLUP_BIT + 1)); 
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 1 << (ITH_GPIO_HTRAP_PULLDOWN_BIT + 1), 1 << (ITH_GPIO_HTRAP_PULLDOWN_BIT + 1)); 
    }

    if (!(packageId & 4)) { // pull-down htrap[0] if htrap [2] == 0 
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 0 << (ITH_GPIO_HTRAP_PULLUP_BIT + 2), 1 << (ITH_GPIO_HTRAP_PULLUP_BIT + 2)); 
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 1 << (ITH_GPIO_HTRAP_PULLDOWN_BIT + 2), 1 << (ITH_GPIO_HTRAP_PULLDOWN_BIT + 2)); 
    }

#elif CFG_CHIP_FAMILY == 9910
    // TODO

#elif CFG_CHIP_FAMILY == 9920
    // TODO

#else
#error "No project defined"
#endif

    return packageId;
}
