/*
 * Copyright (c) 2016 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL LCD functions.
 *
 * @author Irene Wang
 * @version 1.0
 */
#include "../ith_cfg.h"
#include "ith_lcd.h"

#define CMD_DELAY   0xFFFFFFFF

static const uint32_t* lcdScript;
static unsigned int lcdScriptCount, lcdScriptIndex;

void ithLcdReset(void)
{
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0xFFFFFFFF, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
    ithWriteRegMaskA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, 0x0, (0x1 << ITH_LCD_REG_RST_BIT) | (0x1 << ITH_LCD_RST_BIT));
}

void ithLcdEnable(void)
{
    // enable clock
    ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_M3CLK_BIT);
    ithSetRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_W12CLK_BIT);
    
    //from GPIO65~91, set input mode
    {
        int i;
        
        for(i = 65; i < 92; i++)
        {
            ithGpioSetMode(i, ITH_GPIO_MODE1);
        }
    }

    //set reg 0x0020 as 0x81000000 for test mode
    // wait for 0x81000000 become 0x01000000
    ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x81000000);
    while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG) & 0x80000000)
    {
        ithDelay(1000);
    }
}

void ithLcdDisable(void)
{
    //from GPIO65~91, set input mode
    {
        int i;

        for(i = 65; i < 92; i++)
        {
            ithGpioSetMode(i, ITH_GPIO_MODE0);
        }
    }

    //set reg 0x0020 as 0 for test mode
    ithWriteRegA(ITH_LCD_BASE + 0x0020, 0);
        
    //set reg 0x0020 as 0x81000000 for test mode
    // wait for 0x81000000 become 0x01000000
    ithWriteRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG, 0x81000000);
    while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_TEST_COLOR_SET_REG) & 0x80000000)
    {
        ithDelay(1000);
    }
    
    // disable clock
    ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_M3CLK_BIT);
    ithClearRegBitA(ITH_HOST_BASE + ITH_LCD_CLK1_REG, ITH_EN_W12CLK_BIT);
}

void ithLcdSetBaseAddrA(uint32_t addr)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_BASEA_REG, addr, ITH_LCD_BASEA_MASK);
}

void ithLcdSetBaseAddrB(uint32_t addr)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_BASEB_REG, addr, ITH_LCD_BASEB_MASK);
}

uint32_t ithLcdGetBaseAddrB(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_BASEB_REG) & ITH_LCD_BASEB_MASK);
}

void ithLcdSetBaseAddrC(uint32_t addr)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_BASEC_REG, addr, ITH_LCD_BASEC_MASK);
}

uint32_t ithLcdGetBaseAddrC(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_BASEC_REG) & ITH_LCD_BASEC_MASK);
}

void ithLcdEnableHwFlip(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_HW_FLIP_BIT);
}

void ithLcdDisableHwFlip(void)
{
    ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_HW_FLIP_BIT);
}

void ithLcdEnableVideoFlip(void)
{
    ithLcdDisableHwFlip();
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_VIDEO_FLIP_EN_BIT);
}

void ithLcdDisableVideoFlip(void)
{
    ithLcdDisableHwFlip();
    ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_SET1_REG, ITH_LCD_VIDEO_FLIP_EN_BIT);
}

void ithLcdLoadScriptFirst(const uint32_t* script, unsigned int count)
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
            ithWriteRegA(reg, val);
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
            ithWriteRegA(reg, val);
    }
}

void ithLcdCursorSetBaseAddr(uint32_t addr)
{
    ithWriteRegA(ITH_LCD_BASE + ITH_LCD_HWC_BASE_REG, addr);
}

void ithLcdCursorSetColorWeight(ITHLcdCursorColor color, uint8_t value)
{
    switch (color)
    {
    case ITH_LCD_CURSOR_DEF_COLOR:
        ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_INVCOLORWEI_REG, value << ITH_LCD_HWC_INVCOLORWEI_BIT, ITH_LCD_HWC_INVCOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_FG_COLOR:
        ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_FORECOLORWEI_REG, value << ITH_LCD_HWC_FORECOLORWEI_BIT, ITH_LCD_HWC_FORECOLORWEI_MASK);
        break;

    case ITH_LCD_CURSOR_BG_COLOR:
        ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_BACKCOLORWEI_REG, value << ITH_LCD_HWC_BACKCOLORWEI_BIT, ITH_LCD_HWC_BACKCOLORWEI_MASK);
        break;
    }
}

ITHLcdFormat ithLcdGetFormat(void)
{
    return (ITHLcdFormat)((ithReadRegA(ITH_LCD_BASE + ITH_LCD_SRCFMT_REG) & ITH_LCD_SRCFMT_MASK) >> ITH_LCD_SRCFMT_BIT);
}

void ithLcdSetWidth(uint32_t width)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_WIDTH_REG, width << ITH_LCD_WIDTH_BIT, ITH_LCD_WIDTH_MASK);
}


void ithLcdSetHeight(uint32_t height)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HEIGHT_REG, height << ITH_LCD_HEIGHT_BIT, ITH_LCD_HEIGHT_MASK);
}

void ithLcdSetPitch(uint32_t pitch)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_PITCH_REG, pitch << ITH_LCD_PITCH_BIT, ITH_LCD_PITCH_MASK);
}

unsigned int ithLcdGetXCounter(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_CTGH_CNT_REG) & ITH_LCD_CTGH_CNT_MASK) >> ITH_LCD_CTGH_CNT_BIT;
}

unsigned int ithLcdGetYCounter(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_READ_STATUS1_REG) & ITH_LCD_CTGV_CNT_MASK) >> ITH_LCD_CTGV_CNT_BIT;
}

void ithLcdSyncFire(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, ITH_LCD_SYNCFIRE_BIT);
}

 bool ithLcdIsSyncFired(void)
{
     return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG) & ITH_LCD_SYNCFIRE_MASK) ? true : false;
}

bool ithLcdIsEnabled(void)
{
    return ((ithReadRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG) & (ITH_LCD_DISPEN_MASK | ITH_LCD_SYNCFIRE_MASK)) == (ITH_LCD_DISPEN_MASK | ITH_LCD_SYNCFIRE_MASK)) ? true : false;
}

unsigned int ithLcdGetFlip(void)
{
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_READ_STATUS1_REG) & ITH_LCD_FLIP_NUM_MASK) >> ITH_LCD_FLIP_NUM_BIT;
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
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_SWFLIPNUM_REG, index << ITH_LCD_SWFLIPNUM_BIT, ITH_LCD_SWFLIPNUM_MASK);
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 1 << ITH_LCD_LAYER1UPDATE_BIT, ITH_LCD_LAYER1UPDATE_MASK);
}

void ithLcdCursorEnable(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_EN_REG, ITH_LCD_HWC_EN_BIT);
}

void ithLcdCursorDisable(void)
{
    ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_EN_REG, ITH_LCD_HWC_EN_BIT);
}

void ithLcdCursorCtrlEnable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_ABLDEN_BIT);
        break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFDSTEN_BIT);
        break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFINVDST_BIT);
        break;  
    }
}

void ithLcdCursorCtrlDisable(ITHLcdCursorCtrl ctrl)
{
    switch(ctrl)
    {
    case ITH_LCD_CURSOR_ALPHABLEND_ENABLE:
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_ABLDEN_BIT);
        break;   
    case ITH_LCD_CURSOR_DEFDEST_ENABLE:  
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFDSTEN_BIT);
        break;
    case ITH_LCD_CURSOR_INVDEST_ENABLE:
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_CR_REG, ITH_LCD_HWC_DEFINVDST_BIT);
        break;  
    }
}

void ithLcdCursorSetWidth(unsigned int width)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_WIDTH_REG, width << ITH_LCD_HWC_WIDTH_BIT, ITH_LCD_HWC_WIDTH_MASK);
}

void ithLcdCursorSetHeight(unsigned int height)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_HEIGHT_REG, height << ITH_LCD_HWC_HEIGHT_BIT, ITH_LCD_HWC_HEIGHT_MASK);
}

void ithLcdCursorSetPitch(unsigned int pitch)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_PITCH_REG, pitch << ITH_LCD_HWC_PITCH_BIT, ITH_LCD_HWC_PITCH_MASK);
}

void ithLcdCursorSetX(unsigned int x)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_POSX_REG, x << ITH_LCD_HWC_POSX_BIT, ITH_LCD_HWC_POSX_MASK);
}

void ithLcdCursorSetY(unsigned int y)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_POSY_REG, y << ITH_LCD_HWC_POSY_BIT, ITH_LCD_HWC_POSY_MASK);
}

void ithLcdCursorSetColor(ITHLcdCursorColor color, uint16_t value)
{
    switch(color)
    {
        case ITH_LCD_CURSOR_DEF_COLOR:   
            ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_DEFCOLOR_REG, value << ITH_LCD_HWC_DEFCOLOR_BIT, ITH_LCD_HWC_DEFCOLOR_MASK);
            break;
        case ITH_LCD_CURSOR_FG_COLOR:  
            ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_FORECOLOR_REG, value << ITH_LCD_HWC_FORECOLOR_BIT, ITH_LCD_HWC_FORECOLOR_MASK);
            break;
        case ITH_LCD_CURSOR_BG_COLOR:
            ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_HWC_BACKCOLOR_REG, value << ITH_LCD_HWC_BACKCOLOR_BIT, ITH_LCD_HWC_BACKCOLOR_MASK);
            break;      
    }
}

void ithLcdCursorUpdate(void)
{
    ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_HWC_UPDATE_REG, ITH_LCD_HWC_UPDATE_BIT);
}

 bool ithLcdCursorIsUpdateDone(void)
{
     return ithReadRegA(ITH_LCD_BASE + ITH_LCD_HWC_UPDATE_REG) & (0x1 << ITH_LCD_HWC_UPDATE_BIT);
}

void ithLcdIntrCtrlEnable(ITHLcdIntrCtrl ctrl)
{
    switch (ctrl)
    {
    case ITH_LCD_INTR_ENABLE:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_EN_BIT);
        break;
    case ITH_LCD_INTR_FIELDMODE1:
        ithSetRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE1_BIT);
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
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_EN_BIT);
        break;
    case ITH_LCD_INTR_FIELDMODE1:
        ithClearRegBitA(ITH_LCD_BASE + ITH_LCD_INT_CTRL_REG, ITH_LCD_INT_FIELDMODE1_BIT);
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
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_INT_CLR_REG, 0x1 << ITH_LCD_INT_CLR_BIT, ITH_LCD_INT_CLR_MASK);
}

void ithLcdIntrSetScanLine1(unsigned int line)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_INT_LINE1_REG, line << ITH_LCD_INT_LINE1_BIT, ITH_LCD_INT_LINE1_MASK);
}

void ithLcdIntrSetScanLine2(unsigned int line)
{

}

void ithLcdSetRotMode(ITHLcdScanType type, ITHLcdRotMode mode)
{
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_SET1_REG, type << ITH_LCD_SCAN_TYPE_BIT, 0x1 << ITH_LCD_SCAN_TYPE_BIT);
    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_SET1_REG, mode << ITH_LCD_ROT_MODE_BIT, ITH_LCD_ROT_MODE_MASK);

    ithWriteRegMaskA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG, 1 << ITH_LCD_LAYER1UPDATE_BIT, ITH_LCD_LAYER1UPDATE_MASK);

    while (ithReadRegA(ITH_LCD_BASE + ITH_LCD_UPDATE_REG) & (0x1 << ITH_LCD_LAYER1UPDATE_BIT))
    {
        ithDelay(1000);
    }
}



