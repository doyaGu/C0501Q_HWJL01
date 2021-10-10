/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL LCD functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../ith_cfg.h"
#include "ith_lcd.h"

#define CMD_DELAY   0xFFFF

static const uint16_t* lcdScript;
static unsigned int lcdScriptCount, lcdScriptIndex;
static uint16_t lcdBakRegH[2] = {0};
static uint16_t lcdBakFlag = 0;

void ithLcdReset(void)
{
    ithWriteRegMaskH(ITH_LCD_CLK2_REG, 0xFFFF, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
    ithWriteRegMaskH(ITH_LCD_CLK2_REG, 0x0, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
}

void ithLcdEnable(void)
{
    int i;
    
    for(i=15; i<19; i++)	ithGpioClear(i);    //set GPIO15~GPIO18 output 0
    for(i=27; i<34; i++)	ithGpioClear(i);    //set GPIO27~GPIO33 output 0
    ithGpioClear(46);                           //set LCSN as  output 0
    ithGpioClear(48);                           //set LHSYNC as output 0
    ithGpioClear(49);                           //set LHSYNC as output 0
    
    for(i=15; i<19; i++)	ithGpioSetOut(i);    //set GPIO15~GPIO18 as output mode
    for(i=27; i<34; i++)	ithGpioSetOut(i);    //set GPIO27~GPIO33 as output mode
    ithGpioSetOut(46);                           //set LCSN as as output mode
    ithGpioSetOut(48);                           //set LHSYNC as as output mode
    ithGpioSetOut(49);                           //set LHSYNC as as output mode
    
    if(lcdBakFlag)
    {
    	ithWriteRegH(ITH_LCD_CLK2_REG, lcdBakRegH[0]);   //[7]:DHCLK,[5]:MCLK, [3]:LCD clk2, [1]: LCD clk1
    	ithWriteRegH(0x0052, lcdBakRegH[1]);             // LCD clk2
    }
    
    // GPIO mode change to LCD mode (RGB888)
    for(i=15; i<19; i++)	ithGpioSetMode(i,ITH_GPIO_MODE2);    //set GPIO15~GPIO18 as LD18~LD21
    for(i=27; i<34; i++)	ithGpioSetMode(i,ITH_GPIO_MODE2);    //set GPIO27~GPIO33 as LD13~LD23
    ithGpioSetMode(46,ITH_GPIO_MODE1);                           //set LCSN as as output mode
    ithGpioSetMode(48,ITH_GPIO_MODE1);                           //set LHSYNC as as output mode
    ithGpioSetMode(49,ITH_GPIO_MODE1);                           //set LHSYNC as as output mode
}

void ithLcdDisable(void)
{
    int i;
  
    for(i=15; i<19; i++)	ithGpioSetIn(i);    //set GPIO15~GPIO18 as input mode
    for(i=27; i<34; i++)	ithGpioSetIn(i);    //set GPIO27~GPIO33 as input mode
    
    ithGpioSetIn(46);                           //set LCSN as GPIO input mode
    ithGpioSetIn(48);                           //set LHSYNC as GPIO input mode
    ithGpioSetIn(49);                           //set LHSYNC as GPIO input mode
    
    //LCD mode change to GPIO mode
    for(i=15; i<19; i++)	ithGpioSetMode(i,ITH_GPIO_MODE0);    //set GPIO15~GPIO18 as LD18~LD21
    for(i=27; i<34; i++)	ithGpioSetMode(i,ITH_GPIO_MODE0);    //set GPIO27~GPIO33 as LD13~LD23
    ithGpioSetMode(46,ITH_GPIO_MODE0);                           //set LCSN as GPIO input mode
    ithGpioSetMode(48,ITH_GPIO_MODE0);                           //set LHSYNC as GPIO input mode
    ithGpioSetMode(49,ITH_GPIO_MODE0);                           //set LHSYNC as GPIO input mode

    lcdBakRegH[0] = ithReadRegH(0x002A);
    lcdBakRegH[1] = ithReadRegH(0x0052);     
    lcdBakFlag = 1;
    
    //pci_write(0x002A, 0x0008); //[7]:DHCLK,[5]:MCLK, [3]:LCD clk2, [1]: LCD clk1
    ithWriteRegH(0x002A, 0x0008);

    //pci_write(0x0052, 0xC028); // LCD clk2 [7:0]:ratio    
    ithWriteRegH(0x0052, 0xC028);
}

void ithLcdSetBaseAddrA(uint32_t addr)
{
    ithWriteRegMaskH(ITH_LCD_BASEA_LO_REG, addr, ITH_LCD_BASEA_LO_MASK);
    ithWriteRegMaskH(ITH_LCD_BASEA_HI_REG, addr >> 16, ITH_LCD_BASEA_HI_MASK);
}

void ithLcdSetBaseAddrB(uint32_t addr)
{
    ithWriteRegMaskH(ITH_LCD_BASEB_LO_REG, addr, ITH_LCD_BASEB_LO_MASK);
    ithWriteRegMaskH(ITH_LCD_BASEB_HI_REG, addr >> 16, ITH_LCD_BASEB_HI_MASK);
}

uint32_t ithLcdGetBaseAddrB(void)
{
    return ((uint32_t)(ithReadRegH(ITH_LCD_BASEB_HI_REG) & ITH_LCD_BASEB_HI_MASK) << 16) | 
        (ithReadRegH(ITH_LCD_BASEB_LO_REG) & ITH_LCD_BASEB_LO_MASK);
}

void ithLcdSetBaseAddrC(uint32_t addr)
{
    ithWriteRegMaskH(ITH_LCD_BASEC_LO_REG, addr, ITH_LCD_BASEC_LO_MASK);
    ithWriteRegMaskH(ITH_LCD_BASEC_HI_REG, addr >> 16, ITH_LCD_BASEC_HI_MASK);
}

uint32_t ithLcdGetBaseAddrC(void)
{
    return ((uint32_t)(ithReadRegH(ITH_LCD_BASEC_HI_REG) & ITH_LCD_BASEC_HI_MASK) << 16) | 
        (ithReadRegH(ITH_LCD_BASEC_LO_REG) & ITH_LCD_BASEC_LO_MASK);
}

void ithLcdEnableHwFlip(void)
{
    ithSetRegBitH(ITH_LCD_SET1_REG, ITH_LCD_HW_FLIP_BIT);
}

void ithLcdDisableHwFlip(void)
{
    ithClearRegBitH(ITH_LCD_SET1_REG, ITH_LCD_HW_FLIP_BIT);
}

void ithLcdEnableVideoFlip(void)
{
    ithLcdDisableHwFlip();
    ithSetRegBitH(ITH_LCD_SET1_REG, ITH_LCD_VIDEO_FLIP_EN_BIT);
}

void ithLcdDisableVideoFlip(void)
{
    ithLcdDisableHwFlip();
    ithClearRegBitH(ITH_LCD_SET1_REG, ITH_LCD_VIDEO_FLIP_EN_BIT);
}

void ithLcdLoadScriptFirst(const uint16_t* script, unsigned int count)
{
    unsigned int i;

    lcdScript = script;
    lcdScriptCount = count;

    // Run script until fire    
    for (i = 0; i < count; i += 2)
    {
        unsigned int reg    = script[i];
        unsigned int val    = script[i + 1];
        
        if (reg == CMD_DELAY)
            ithDelay(val * 1000);
        else
		{
            ithWriteRegH(reg, val);
			if (reg == ITH_LCD_UPDATE_REG && (val & ITH_LCD_SYNCFIRE_MASK))
				break;
		}
    }
    lcdScriptIndex = i;
}

void ithLcdLoadScriptNext(void)
{
    unsigned int i;
    
    for (i = lcdScriptIndex; i < lcdScriptCount; i += 2)
    {
        unsigned int reg    = lcdScript[i];
        unsigned int val    = lcdScript[i + 1];
        
        if (reg != CMD_DELAY)
            ithWriteRegH(reg, val);
    }
}

void ithLcdCursorSetBaseAddr(uint32_t addr)
{
    ithWriteRegH(ITH_LCD_DHWC_BASE_LO_REG, addr);
    ithWriteRegH(ITH_LCD_DHWC_BASE_HI_REG, addr >> 16);
}

void ithLcdCursorSetColorWeight(ITHLcdCursorColor color, uint8_t value)
{
    switch (color)
    {
    case ITH_LCD_CURSOR_DEF_COLOR:
        ithWriteRegMaskH(ITH_LCD_DHWC_INVCOLORWEI_REG, value, ITH_LCD_DHWC_INVCOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_FG_COLOR:
        ithWriteRegMaskH(ITH_LCD_DHWC_FORECOLORWEI_REG, value << ITH_LCD_DHWC_FORECOLORWEI_BIT, ITH_LCD_DHWC_FORECOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_BG_COLOR:
        ithWriteRegMaskH(ITH_LCD_DHWC_BACKCOLORWEI_REG, value, ITH_LCD_DHWC_BACKCOLORWEI_MASK);
        break;
    }
}

ITHLcdFormat ithLcdGetFormat(void)
{
    return (ITHLcdFormat)((ithReadRegH(ITH_LCD_SRCFMT_REG) & ITH_LCD_SRCFMT_MASK) >> ITH_LCD_SRCFMT_BIT);
}

void ithLcdSetWidth(uint32_t width)
{
    ithWriteRegMaskH(ITH_LCD_WIDTH_REG, width << ITH_LCD_WIDTH_BIT, ITH_LCD_WIDTH_MASK);
}

void ithLcdSetHeight(uint32_t height)
{
    ithWriteRegMaskH(ITH_LCD_HEIGHT_REG, height << ITH_LCD_HEIGHT_BIT, ITH_LCD_HEIGHT_MASK);
}

void ithLcdSetPitch(uint32_t pitch)
{
    ithWriteRegMaskH(ITH_LCD_PITCH_REG, pitch << ITH_LCD_PITCH_BIT, ITH_LCD_PITCH_MASK);
}

unsigned int ithLcdGetXCounter(void)
{
    return (ithReadRegH(ITH_LCD_CTGH_CNT_REG) & ITH_LCD_CTGH_CNT_MASK) >> ITH_LCD_CTGH_CNT_BIT;
}

unsigned int ithLcdGetYCounter(void)
{
    return (ithReadRegH(ITH_LCD_READ_STATUS2_REG) & ITH_LCD_CTGV_CNT_MASK) >> ITH_LCD_CTGV_CNT_BIT;
}

 void ithLcdSyncFire(void)
{
    ithSetRegBitH(ITH_LCD_UPDATE_REG, ITH_LCD_SYNCFIRE_BIT);
}

 bool ithLcdIsSyncFired(void)
{
    return (ithReadRegH(ITH_LCD_UPDATE_REG) & ITH_LCD_SYNCFIRE_MASK) ? true : false;
}

 bool ithLcdIsEnabled(void)
{
    return ((ithReadRegH(ITH_LCD_UPDATE_REG) & (ITH_LCD_DISPEN_MASK | ITH_LCD_SYNCFIRE_MASK)) ==  (ITH_LCD_DISPEN_MASK | ITH_LCD_SYNCFIRE_MASK)) ? true : false;
}

 unsigned int ithLcdGetFlip(void)
{
    return (ithReadRegH(ITH_LCD_READ_STATUS2_REG) & ITH_LCD_FLIP_NUM_MASK) >> ITH_LCD_FLIP_NUM_BIT;
}

 unsigned int ithLcdGetMaxLcdBufCount(void)
{
#if CFG_VIDEO_ENABLE
    return 3;
#else
    return 2;
#endif
}

 void ithLcdSwFlip(unsigned int index)
{
    ithWriteRegMaskH(ITH_LCD_SWFLIPNUM_REG, index << ITH_LCD_SWFLIPNUM_BIT, ITH_LCD_SWFLIPNUM_MASK);
    ithWriteRegMaskH(ITH_LCD_UPDATE_REG, 1 << ITH_LCD_LAYER1UPDATE_BIT, ITH_LCD_LAYER1UPDATE_MASK);
}

 void ithLcdCursorEnable(void)
{
    ithSetRegBitH(ITH_LCD_DHWC_EN_REG, ITH_LCD_DHWC_EN_BIT);
}

 void ithLcdCursorDisable(void)
{
    ithClearRegBitH(ITH_LCD_DHWC_EN_REG, ITH_LCD_DHWC_EN_BIT);
}

 void ithLcdCursorCtrlEnable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
         ithSetRegBitH(ITH_LCD_DHWC_CR_REG, ITH_LCD_DHWC_ABLDEN_BIT);
         break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
         ithSetRegBitH(ITH_LCD_DHWC_CR_REG, ITH_LCD_DHWC_DEFDSTEN_BIT);
         break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
         ithSetRegBitH(ITH_LCD_DHWC_CR_REG, ITH_LCD_DHWC_DEFINVDST_BIT);
         break;  
    }
}

 void ithLcdCursorCtrlDisable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
         ithClearRegBitH(ITH_LCD_DHWC_CR_REG, ITH_LCD_DHWC_ABLDEN_BIT);
         break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
         ithClearRegBitH(ITH_LCD_DHWC_CR_REG, ITH_LCD_DHWC_DEFDSTEN_BIT);
         break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
         ithClearRegBitH(ITH_LCD_DHWC_CR_REG, ITH_LCD_DHWC_DEFINVDST_BIT);
         break;  
    }
}

 void ithLcdCursorSetWidth(unsigned int width)
{
    ithWriteRegMaskH(ITH_LCD_DHWC_WIDTH_REG, width, ITH_LCD_DHWC_WIDTH_MASK);
}

 void ithLcdCursorSetHeight(unsigned int height)
{
    ithWriteRegMaskH(ITH_LCD_DHWC_HEIGHT_REG, height, ITH_LCD_DHWC_HEIGHT_MASK);
}

 void ithLcdCursorSetPitch(unsigned int pitch)
{
    ithWriteRegMaskH(ITH_LCD_DHWC_PITCH_REG, pitch, ITH_LCD_DHWC_PITCH_MASK);
}

 void ithLcdCursorSetX(unsigned int x)
{
    ithWriteRegMaskH(ITH_LCD_DHWC_POSX_REG, x, ITH_LCD_DHWC_POSX_MASK);
}

 void ithLcdCursorSetY(unsigned int y)
{
    ithWriteRegMaskH(ITH_LCD_DHWC_POSY_REG, y, ITH_LCD_DHWC_POSY_MASK);
}

 void ithLcdCursorSetColor(ITHLcdCursorColor color, uint16_t value)
{
    switch(color)
    {
        case ITH_LCD_CURSOR_DEF_COLOR:   
            ithWriteRegH(ITH_LCD_DHWC_DEFCOLOR_REG, value);
            break;
        case ITH_LCD_CURSOR_FG_COLOR:  
            ithWriteRegH(ITH_LCD_DHWC_FORECOLOR_REG, value);
            break;
        case ITH_LCD_CURSOR_BG_COLOR:
            ithWriteRegH(ITH_LCD_DHWC_BACKCOLOR_REG, value);
            break;      
    }
}
 void ithLcdCursorUpdate(void)
{
    ithSetRegBitH(ITH_LCD_DHWC_UPDATE_REG, ITH_LCD_DHWC_UPDATE_BIT);
}

 bool ithLcdCursorIsUpdateDone(void)
{
    return ithReadRegH(ITH_LCD_DHWC_UPDATE_REG) & (0x1 << ITH_LCD_DHWC_UPDATE_BIT);
}

 void ithLcdIntrCtrlEnable(ITHLcdIntrCtrl ctrl)
{
    switch (ctrl)
    {
    case ITH_LCD_INTR_ENABLE:
         ithSetRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_EN_BIT);
         break;
    case ITH_LCD_INTR_FIELDMODE1:
         ithSetRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE1_BIT);
         break;
    case ITH_LCD_INTR_OUTPUT2:
         ithSetRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_OUTPUT2_BIT);
         break;
    case ITH_LCD_INTR_FIELDMODE2:
         ithSetRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE2_BIT);
         break;
    case ITH_LCD_INTR_OUTPUT1:
         ithSetRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_OUTPUT1_BIT);
         break;
    }
}

 void ithLcdIntrCtrlDisable(ITHLcdIntrCtrl ctrl)
{
    switch (ctrl)
    {
    case ITH_LCD_INTR_ENABLE:
         ithClearRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_EN_BIT);
         break;
    case ITH_LCD_INTR_FIELDMODE1:
         ithClearRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE1_BIT);
         break;
    case ITH_LCD_INTR_OUTPUT2:
         ithClearRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_OUTPUT2_BIT);
         break;
    case ITH_LCD_INTR_FIELDMODE2:
         ithClearRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE2_BIT);
         break;
    case ITH_LCD_INTR_OUTPUT1:
         ithClearRegBitH(ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_OUTPUT1_BIT);
         break;
    }
}

 void ithLcdIntrEnable(void)
{
    ithLcdIntrCtrlEnable(ITH_LCD_INTR_ENABLE);
}

 void ithLcdIntrDisable(void)
{
    ithLcdIntrCtrlDisable(ITH_LCD_INTR_ENABLE);
}

 void ithLcdIntrClear(void)
{
     ithWriteRegMaskH(ITH_LCD_INT_CLR_REG, 0x1 << ITH_LCD_INT_CLR_BIT, ITH_LCD_INT_CLR_MASK);
}

void ithLcdIntrSetScanLine1(unsigned int line)
{
    ithWriteRegMaskH(ITH_LCD_INT_LINE1_REG, line << ITH_LCD_INT_LINE1_BIT, ITH_LCD_INT_LINE1_MASK);
}

 void ithLcdIntrSetScanLine2(unsigned int line)
{
   ithWriteRegMaskH(ITH_LCD_INT_LINE2_REG, line << ITH_LCD_INT_LINE2_BIT, ITH_LCD_INT_LINE2_MASK);
}

