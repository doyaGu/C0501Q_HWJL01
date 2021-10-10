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

void ithLcdReset(void)
{
    ithWriteRegMaskH(ITH_LCD_CLK2_REG, 0xFFFF, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
    ithWriteRegMaskH(ITH_LCD_CLK2_REG, 0x0, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
}

void ithLcdEnable(void)
{
    // enable clock
    ithSetRegBitH(ITH_LCD_CLK2_REG, ITH_EN_M4CLK_BIT);
    ithSetRegBitH(ITH_EN_MMIO_REG, ITH_EN_LCD_MMIO_BIT);
    
    //from GPIO39~65 67~68, set input mode
    {
        int i;
        
        for(i=39; i<64; i++)
        {
            ithGpioSetMode(i, ITH_GPIO_MODE1);
        }        
        ithGpioSetMode(67, ITH_GPIO_MODE1);
        ithGpioSetMode(68, ITH_GPIO_MODE1);
    }

    //set reg 0x1220 as 0x8002 for test mode
    // wait for 0x8002 become 0x0002
    ithWriteRegH(0x1220, 0x8002);
    while( ithReadRegH(0x1220)&0x8000 )
    {
        ithDelay(1000);
    }
}

void ithLcdDisable(void)
{
    //from GPIO39~65 67~68, set input mode
    {
        int i;

        for(i=39; i<64; i++)
        {
            ithGpioSetMode(i, ITH_GPIO_MODE0);
        }
        
        ithGpioSetMode(67, ITH_GPIO_MODE0);
        ithGpioSetMode(68, ITH_GPIO_MODE0);
    }

    //set reg 0x1222 as 0 for test mode
     ithWriteRegH(0x1222, 0);
    
    //set reg 0x1224 as 0 for test mode
    ithWriteRegH(0x1224, 0);
    
    //set reg 0x1220 as 0x8006 for test mode
    // wait for 0x8006 become 0x0006
    ithWriteRegH(0x1220, 0x8006);
    while( ithReadRegH(0x1220)&0x8000 )
    {
        ithDelay(1000);
    }
    
    // disable clock
    ithClearRegBitH(ITH_LCD_CLK2_REG, ITH_EN_M4CLK_BIT);
    ithClearRegBitH(ITH_EN_MMIO_REG, ITH_EN_LCD_MMIO_BIT);
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
    ithWriteRegH(ITH_LCD_HWC_BASE_LO_REG, addr);
    ithWriteRegH(ITH_LCD_HWC_BASE_HI_REG, addr >> 16);
}

void ithLcdCursorSetColorWeight(ITHLcdCursorColor color, uint8_t value)
{
    switch (color)
    {
    case ITH_LCD_CURSOR_DEF_COLOR:
        ithWriteRegMaskH(ITH_LCD_HWC_INVCOLORWEI_REG, value, ITH_LCD_HWC_INVCOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_FG_COLOR:
        ithWriteRegMaskH(ITH_LCD_HWC_FORECOLORWEI_REG, value << ITH_LCD_HWC_FORECOLORWEI_BIT, ITH_LCD_HWC_FORECOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_BG_COLOR:
        ithWriteRegMaskH(ITH_LCD_HWC_BACKCOLORWEI_REG, value, ITH_LCD_HWC_BACKCOLORWEI_MASK);
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
    return (ithReadRegH(ITH_LCD_READ_STATUS1_REG) & ITH_LCD_CTGV_CNT_MASK) >> ITH_LCD_CTGV_CNT_BIT;
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
    return (ithReadRegH(ITH_LCD_READ_STATUS1_REG) & ITH_LCD_FLIP_NUM_MASK) >> ITH_LCD_FLIP_NUM_BIT;
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
    ithSetRegBitH(ITH_LCD_HWC_EN_REG, ITH_LCD_HWC_EN_BIT);
}

void ithLcdCursorDisable(void)
{
    ithClearRegBitH(ITH_LCD_HWC_EN_REG, ITH_LCD_HWC_EN_BIT);
}

void ithLcdCursorCtrlEnable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
         ithSetRegBitH(ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_ABLDEN_BIT);
         break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
         ithSetRegBitH(ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFDSTEN_BIT);
         break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
         ithSetRegBitH(ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFINVDST_BIT);
         break;  
    }
}

void ithLcdCursorCtrlDisable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
         ithClearRegBitH(ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_ABLDEN_BIT);
         break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
         ithClearRegBitH(ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFDSTEN_BIT);
         break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
         ithClearRegBitH(ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFINVDST_BIT);
         break;  
    }
}

void ithLcdCursorSetWidth(unsigned int width)
{
    ithWriteRegMaskH(ITH_LCD_HWC_WIDTH_REG, width, ITH_LCD_HWC_WIDTH_MASK);
}

void ithLcdCursorSetHeight(unsigned int height)
{
    ithWriteRegMaskH(ITH_LCD_HWC_HEIGHT_REG, height, ITH_LCD_HWC_HEIGHT_MASK);
}

void ithLcdCursorSetPitch(unsigned int pitch)
{
    ithWriteRegMaskH(ITH_LCD_HWC_PITCH_REG, pitch, ITH_LCD_HWC_PITCH_MASK);
}

void ithLcdCursorSetX(unsigned int x)
{
    ithWriteRegMaskH(ITH_LCD_HWC_POSX_REG, x, ITH_LCD_HWC_POSX_MASK);
}

void ithLcdCursorSetY(unsigned int y)
{
    ithWriteRegMaskH(ITH_LCD_HWC_POSY_REG, y, ITH_LCD_HWC_POSY_MASK);
}

void ithLcdCursorSetColor(ITHLcdCursorColor color, uint16_t value)
{
    switch(color)
    {
        case ITH_LCD_CURSOR_DEF_COLOR:   
            ithWriteRegH(ITH_LCD_HWC_DEFCOLOR_REG, value);
            break;
        case ITH_LCD_CURSOR_FG_COLOR:  
            ithWriteRegH(ITH_LCD_HWC_FORECOLOR_REG, value);
            break;
        case ITH_LCD_CURSOR_BG_COLOR:
            ithWriteRegH(ITH_LCD_HWC_BACKCOLOR_REG, value);
            break;      
    }
}

void ithLcdCursorUpdate(void)
{
    ithSetRegBitH(ITH_LCD_HWC_UPDATE_REG, ITH_LCD_HWC_UPDATE_BIT);
}

 bool ithLcdCursorIsUpdateDone(void)
{
    return ithReadRegH(ITH_LCD_HWC_UPDATE_REG) & (0x1 << ITH_LCD_HWC_UPDATE_BIT);
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
    case ITH_LCD_INTR_FIELDMODE2:
    case ITH_LCD_INTR_OUTPUT1:
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
    case ITH_LCD_INTR_FIELDMODE2:
    case ITH_LCD_INTR_OUTPUT1:
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

}

