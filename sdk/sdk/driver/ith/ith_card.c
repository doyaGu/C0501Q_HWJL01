/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Card functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"
#include <string.h>

static ITHCardConfig cardCfg;

void ithCardInit(const ITHCardConfig* cfg)
{
    memcpy(&cardCfg, cfg, sizeof(ITHCardConfig));
}

void ithCardPowerOn(ITHCardPin pin)
{
#ifndef CFG_ITH_FPGA
    unsigned int num = cardCfg.powerEnablePins[pin];
    if (num == (unsigned char)-1)
        return;
        
    ithGpioEnable(num);
    ithGpioSetOut(num);
    ithGpioClear(num);
#endif // !CFG_ITH_FPGA
}

void ithCardPowerOff(ITHCardPin pin)
{
#ifndef CFG_ITH_FPGA
    unsigned int num = cardCfg.powerEnablePins[pin];
    if (num == (unsigned char)-1)
        return;
        
    ithGpioEnable(num);
    ithGpioSetOut(num);
    ithGpioSet(num);
#endif // !CFG_ITH_FPGA
}

bool ithCardInserted(ITHCardPin pin)
{
#ifdef CFG_ITH_FPGA
    return true;
#else
    unsigned int num = cardCfg.cardDetectPins[pin];
    if (num == (unsigned char)-1)
        return false;
    if (num == (unsigned char)-2) // always insert
        return true;
        
    ithGpioEnable(num);
    ithGpioSetIn(num);

    if (pin == ITH_CARDPIN_SD0) {
        #if defined(CFG_SD0_CARD_DETECT_ACTIVE_HIGH)
        return ithGpioGet(num);
        #endif
    }
    if (pin == ITH_CARDPIN_SD1) {
        #if defined(CFG_SD1_CARD_DETECT_ACTIVE_HIGH)
        return ithGpioGet(num);
        #endif
    }
    return !ithGpioGet(num);
#endif // CFG_ITH_FPGA
}

bool ithCardLocked(ITHCardPin pin)
{
#ifdef CFG_ITH_FPGA
    return false;
#else
    unsigned int num;
    
    num = cardCfg.writeProtectPins[pin];
    if (num == (unsigned char)-1)
        return false;
        
    ithGpioEnable(num);
    ithGpioSetIn(num);
    return ithGpioGet(num) ? true : false;
#endif // CFG_ITH_FPGA
}


#if 0
    ITH_STOR_NAND = 0,
    ITH_STOR_XD   = 1,
    ITH_STOR_SD   = 2,
    ITH_STOR_MS_0 = 3,
    ITH_STOR_MS_1 = 4,
    ITH_STOR_CF   = 5,
    ITH_STOR_NOR  = 6,
    ITH_STOR_SD1  = 7
#endif


#if (CFG_CHIP_FAMILY == 9070)

static uint32_t storageIoPullUpDown[] =
{   /** value, mask */
#ifdef	CFG_SPI_NAND
    0, 0, // ITH_STOR_NAND(SPI NAND)
#else
    0x003003BF, 0x003003FF, // ITH_STOR_NAND
#endif
    0, 0, // ITH_STOR_XD
    0x0C3003BF, 0x0C3003FF, // ITH_STOR_SD
    0x083003BF, 0x0C3003FF, // ITH_STOR_MS_0
    0x083003BF, 0x0C3003FF, // ITH_STOR_MS_1
    0x0CBC03BF, 0x0CFC03FF, // ITH_STOR_CF
    0, 0, // ITH_STOR_NOR
    0x0C3003BF, 0x0C3003FF  // ITH_STOR_SD1
};
    
#if defined(CFG_SD1_ENABLE)
#define SD2_CLK_GPIO    7
#endif

void ithStorageUnSelect(ITHStorage storage)
{
    ithEnterCritical();

    /** disable cmd/data power */
    ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO_HOSTSEL_REG), 0x0, 0x7);

    switch(storage)
    {
    case ITH_STOR_SD:
    case ITH_STOR_MS_0:
    case ITH_STOR_MS_1:
#if defined(CFG_SD1_ENABLE)
        #if 0
        /** let sd2 clock (GPIO7) pull down to keep last state */
        ithWriteRegMaskA((ITH_GPIO_BASE+0x14), (0x1<<SD2_CLK_GPIO), (0x1<<SD2_CLK_GPIO));  /** pull up/down enable */
        ithWriteRegMaskA((ITH_GPIO_BASE+0x18), (0x0<<SD2_CLK_GPIO), (0x1<<SD2_CLK_GPIO));  /** 1:pull up, 0:pull down */
        //ithWriteRegMaskA((ITH_GPIO_BASE+0xC0), 0x0, storageIoPullUpDown[storage*2+1]); /** let bus floating */
        #else
        /** let sd2 clock (GPIO7) pull down to keep last state */
        ithGpioCtrlEnable(SD2_CLK_GPIO, ITH_GPIO_PULL_ENABLE);
        ithGpioCtrlDisable(SD2_CLK_GPIO, ITH_GPIO_PULL_UP); /* sd1 clock pull down */
        #endif
#endif
        break;

#if defined(CFG_SD1_ENABLE)
    case ITH_STOR_SD1:
        /** disable sd2 clk power */
        {
            /** let sd2 clk GPIO7 as input mode */
            ithGpioEnable(SD2_CLK_GPIO);
            ithGpioSetIn(SD2_CLK_GPIO);
            /** let sd2 clock (GPIO7) floating */
            ithWriteRegMaskA((ITH_GPIO_BASE+0x14), 0x0, (0x1<<SD2_CLK_GPIO));  /** pull up/down disable */
        }
        /** sd_clk(SD0) idle state low to keep last state */
        //ithWriteRegMaskA((ITH_GPIO_BASE+0xC0), 0x80, storageIoPullUpDown[storage*2+1]|0xC0); /** let bus floating */
        break;
#endif
    default:
        break;
    }

    ithExitCritical();
}


void ithStorageSelect(ITHStorage storage)
{
    ithEnterCritical();

    if(storage == ITH_STOR_SD1)
        ithWriteRegMaskA(ITH_GPIO_BASE+ITH_GPIO_HOSTSEL_REG, ITH_STOR_SD<<ITH_GPIO_STORSEL_BIT , ITH_GPIO_STORSEL_MASK);
    else
        ithWriteRegMaskA(ITH_GPIO_BASE+ITH_GPIO_HOSTSEL_REG, storage<<ITH_GPIO_STORSEL_BIT , ITH_GPIO_STORSEL_MASK);

    switch (storage)
    {
    case ITH_STOR_SD:  // Storage Mode 2
#if defined(CFG_SD1_ENABLE)
        /** disable SD2 clock */
        ithGpioSetMode(SD2_CLK_GPIO, ITH_GPIO_MODE0);  
        //ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO1_MODE_REG), 0x0, 0xC000); // GPIO7 mode 0 (as gpio)
#endif
        ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, ITH_GPIO_SD_SEL_BIT); /* SD clock from SDCL pin, sd clock from GPIO7 mode 1 */
        break;
        
#if defined(CFG_SD1_ENABLE)
    case ITH_STOR_SD1: // Storage Mode 2
        #if 0
        ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO1_MODE_REG), 0x4000, 0xC000); // GPIO7 mode 1 (as sd clock)
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, ITH_GPIO_SD_SEL_BIT);   /* SD clock from GPIO7 mode 1 */
        ithWriteRegMaskA((ITH_GPIO_BASE+0x14), (0x1<<SD2_CLK_GPIO), (0x1<<SD2_CLK_GPIO));  /** pull up/down enable */
        ithWriteRegMaskA((ITH_GPIO_BASE+0x18), (0x0<<SD2_CLK_GPIO), (0x1<<SD2_CLK_GPIO));  /** 1:pull up, 0:pull down */
        #else
        ithGpioSetMode(SD2_CLK_GPIO, ITH_GPIO_MODE1);
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, ITH_GPIO_SD_SEL_BIT);   /* SD clock from GPIO7 mode 1 */
        ithGpioCtrlEnable(SD2_CLK_GPIO, ITH_GPIO_PULL_ENABLE);
        ithGpioCtrlDisable(SD2_CLK_GPIO, ITH_GPIO_PULL_UP); /* sd1 clock pull down */
        #endif
        break;
#endif

    case ITH_STOR_MS_0: // Storage Mode 3
        ithGpioSetMode(13, ITH_GPIO_MODE2);
        //ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO1_MODE_REG), (0x2 << 26), (0x3 << 26));   // Data1: GPIO13
        break;
        
    case ITH_STOR_MS_1: // Storage Mode 4
        ithGpioSetMode(13, ITH_GPIO_MODE2);
        //ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO1_MODE_REG), (0x2 << 26), (0x3 << 26)); // Data1: GPIO13
        ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO2_MODE_REG), 0x0003F000, 0x0003F000);   // Data0:GPIO22, Data2:GPIO23, Data3:GPIO24
        break;
        
    case ITH_STOR_CF: // Storage Mode 5
        ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO2_MODE_REG), 0x002AAA80, 0x003FFFC0);   /** CF_D8 ~ CF_D15, gpio19~26, FINAL!! */
        ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO1_PULLTYPE_REG), 0x0, 0x07F80000);      /** CF_D8 ~ CF_D15, gpio19~26, pull low!! */
        ithWriteRegMaskA((ITH_GPIO_BASE+ITH_GPIO1_PULLEN_REG), 0x07F80000, 0x07F80000); /** CF_D8 ~ CF_D15, gpio19~26, pull high/low enable!! */
        break;

    case ITH_STOR_NAND:
        //ithWriteRegMaskA(ITH_GPIO_BASE + 0xC0, 0x0030033F , 0x0030033F); //set NAND gpio as pull up
        break;

    case ITH_STOR_NOR:
        break;

    default:
        break;
    }

    // pull high/down
    ithWriteRegMaskA(ITH_GPIO_BASE + 0xC0,  storageIoPullUpDown[storage*2], storageIoPullUpDown[storage*2+1]);

    ithExitCritical();
}
#else  /**  9910/9850/9920  */

void ithStorageUnSelect(ITHStorage storage)
{
    int i=0;
    unsigned char num;

    //ithEnterCritical();

    switch(storage)
    {
    case ITH_STOR_SD:
        for(i=0; i<SD_PIN_NUM; i++)
        {
            num = cardCfg.sd0Pins[i];
            if(num != (unsigned char)-1)
            {
                ithGpioSetMode(num, ITH_GPIO_MODE0); /* set input mode */
                ithGpioCtrlDisable(num, ITH_GPIO_PULL_ENABLE); /* sd io pull disable */
            }
        }
        break;
    case ITH_STOR_SD1:
        for(i=0; i<SD_PIN_NUM; i++)
        {
            num = cardCfg.sd1Pins[i];
            if(num != (unsigned char)-1)
            {
                ithGpioSetMode(num, ITH_GPIO_MODE0); /* set input mode */
                ithGpioCtrlDisable(num, ITH_GPIO_PULL_ENABLE); /* sd io pull disable */
            }
        }
        break;
    default:
        break;
    }

    //ithExitCritical();
}

void ithStorageSelect(ITHStorage storage)
{
    int i=0;
    unsigned char num;

    //ithEnterCritical();

    switch(storage)
    {
    case ITH_STOR_SD:
        {
            unsigned char mode;

            /* disable sd1 clock */
            num = cardCfg.sd1Pins[0];
            if(num != (unsigned char)-1)
                ithGpioSetMode(num, ITH_GPIO_MODE0);

            /* switch to sd0 io */
            for(i=0; i<SD_PIN_NUM; i++)
            {
                num = cardCfg.sd0Pins[i];
                if(num != (unsigned char)-1)
                {
                    #if (CFG_CHIP_FAMILY == 9910)
                    mode = ITH_GPIO_MODE2;
                    #elif (CFG_CHIP_FAMILY == 9850)
                    mode = ITH_GPIO_MODE1;
                    #elif (CFG_CHIP_FAMILY == 9920)
                    mode = (num < 30) ? ITH_GPIO_MODE1 : ITH_GPIO_MODE2;
                    #endif
                    ithGpioSetMode(num, mode);
                    ithGpioCtrlEnable(num, ITH_GPIO_PULL_ENABLE); /* sd io pull up */
                    ithGpioCtrlEnable(num, ITH_GPIO_PULL_UP);
                }
            }
        }
        break;
    case ITH_STOR_SD1:
        {
            /* disable sd0 clock */
            num = cardCfg.sd0Pins[0];
            if(num != (unsigned char)-1)
                ithGpioSetMode(num, ITH_GPIO_MODE0);

            /* switch to sd1 io */
            for(i=0; i<SD_PIN_NUM; i++)
            {
                num = cardCfg.sd1Pins[i];
                if(num != (unsigned char)-1)
                {
                #if (CFG_CHIP_FAMILY == 9910)
                    if(num == 5)
                        ithGpioSetMode(num, ITH_GPIO_MODE3);
                    else
                        ithGpioSetMode(num, ITH_GPIO_MODE2);
     			#elif (CFG_CHIP_FAMILY == 9850)
					if((num == 8) || (num == 13))
						ithGpioSetMode(num, ITH_GPIO_MODE3);
					else
                        ithGpioSetMode(num, ITH_GPIO_MODE1);
                #elif (CFG_CHIP_FAMILY == 9920)
                    ithGpioSetMode(num, (num < 30) ? ITH_GPIO_MODE1 : ITH_GPIO_MODE2);
				#endif
                    ithGpioCtrlEnable(num, ITH_GPIO_PULL_ENABLE); /* sd io pull up */
                    ithGpioCtrlEnable(num, ITH_GPIO_PULL_UP);
                }
            }
        }
        break;
	case ITH_STOR_NOR:
#ifdef CFG_SPI0_40MHZ_ENABLE
      	ithWriteRegMaskA(ITH_SSP0_BASE + 0x74, CFG_SPI0_NORCLK_PARA << 16, (0xFF<<16));	
#endif
      	break;
   case ITH_STOR_NAND:
#ifdef CFG_SPI0_40MHZ_ENABLE		
      	ithWriteRegMaskA(ITH_SSP0_BASE + 0x74, CFG_SPI0_NANDCLK_PARA << 16, (0xFF<<16));
#endif
     	break;	
    default:
        break;
    }

    //ithExitCritical();
}

#if (CFG_CHIP_FAMILY == 9850)
bool ithSdSwitchPin1(int idx)
{
    if (idx == 0) {
        if (cardCfg.sd0Pins[3] == 22)
            return true;
    }
    else {
        if (cardCfg.sd1Pins[3] == 22)
            return true;
    }
    
    return false;
}
#endif

#endif
