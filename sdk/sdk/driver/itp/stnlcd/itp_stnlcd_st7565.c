/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL STN LCD ST7565 driver.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "../itp_cfg.h"
#include <unistd.h>


#define ENABLE_PARALLEL_INTERFACE
/*****************************************************************
 MACRO defination
******************************************************************/
#define CMD_DISPLAY_OFF   0xAE
#define CMD_DISPLAY_ON    0xAF

#define CMD_SET_DISP_START_LINE  0x40
#define CMD_SET_PAGE  0xB0

#define CMD_SET_COLUMN_UPPER  0x10
#define CMD_SET_COLUMN_LOWER  0x00

#define CMD_SET_ADC_NORMAL  0xA0
#define CMD_SET_ADC_REVERSE 0xA1

#define CMD_SET_DISP_NORMAL 0xA6
#define CMD_SET_DISP_REVERSE 0xA7

#define CMD_SET_ALLPTS_NORMAL 0xA4
#define CMD_SET_ALLPTS_ON  0xA5
#define CMD_SET_BIAS_9 0xA2 
#define CMD_SET_BIAS_7 0xA3

#define CMD_RMW  0xE0
#define CMD_RMW_CLEAR 0xEE
#define CMD_INTERNAL_RESET  0xE2
#define CMD_SET_COM_NORMAL  0xC0
#define CMD_SET_COM_REVERSE  0xC8
#define CMD_SET_POWER_CONTROL  0x28
#define CMD_SET_RESISTOR_RATIO  0x20
#define CMD_SET_VOLUME_FIRST  0x81
#define  CMD_SET_VOLUME_SECOND  0
#define CMD_SET_STATIC_OFF  0xAC
#define  CMD_SET_STATIC_ON  0xAD
#define CMD_SET_STATIC_REG  0x0
#define CMD_SET_BOOSTER_FIRST  0xF8
#define CMD_SET_BOOSTER_234  0
#define  CMD_SET_BOOSTER_5  1
#define  CMD_SET_BOOSTER_6  3
#define CMD_NOP  0xE3
#define CMD_TEST  0xF0

#define LCM_STATUS_BUSY		0x80
#define LCM_STATUS_ADC		0x40
#define LCM_STATUS_ONOFF	0x20
#define LCM_STATUS_RST		0x10

#ifdef ENABLE_PARALLEL_INTERFACE
static const int pagemap[] = { 7, 6, 5, 4, 3, 2, 1, 0 };
#else 
static const int pagemap[] = { 3, 2, 1, 0, 7, 6, 5, 4 };
#endif

#ifdef ENABLE_PARALLEL_INTERFACE
static const int STNLCD_GPIO_DATA_PIN[] = { CFG_GPIO_STNLCD_DATA };
#endif


#ifdef ENABLE_PARALLEL_INTERFACE
void SetAllDataPinAsInput()
{
    int i;
    for (i = 0; i < 8; i++)
    {
		ithGpioSetIn(STNLCD_GPIO_DATA_PIN[i]);
    }
}

void SetAllDataPinAsOutput()
{
    int i;
    for (i = 0; i < 8; i++)
    {
		ithGpioSetOut(STNLCD_GPIO_DATA_PIN[i]);
    }
}

static void STN7565_Read(uint8_t *c)
{
    int i;
    uint8_t bit, ch, mask;
    
    SetAllDataPinAsInput();    
    
    ithGpioClear(CFG_GPIO_STNLCD_RD);	//GPIO_DATA_PIN[i]
    ithGpioSet(CFG_GPIO_STNLCD_WR);	//GPIO_DATA_PIN[i]
    
    for (i = 0; i < 8 ; i++)
    {
        bit = ithGpioGet(STNLCD_GPIO_DATA_PIN[i]);
           
        mask = ~(0x01<<i);
        ch |= (bit&mask)<<i;
    }
    ithGpioSet(CFG_GPIO_STNLCD_RD);
    *c = ch;
}

static uint8_t ReadStatus(void)
{
	uint8_t status;
	
    ithGpioClear(CFG_GPIO_STNLCD_A0);
    STN7565_Read(&status);
    return status;
}
#endif

static void Write(uint8_t c)
{
    int i;

#ifdef ENABLE_PARALLEL_INTERFACE
    //while( ReadStatus() & LCM_STATUS_BUSY );
    
    SetAllDataPinAsOutput();    
    
    ithGpioClear(CFG_GPIO_STNLCD_WR);	//GPIO_DATA_PIN[i]
    ithGpioSet(CFG_GPIO_STNLCD_RD);	//GPIO_DATA_PIN[i]  

    for (i = 7; i >= 0; i--)
    {
        if (c & (1u << i))
            ithGpioSet(STNLCD_GPIO_DATA_PIN[i]);
        else
            ithGpioClear(STNLCD_GPIO_DATA_PIN[i]);
    }
    
    ithGpioSet(CFG_GPIO_STNLCD_WR);    
#else
    for (i = 7; i >= 0; i--)
    {
        ithGpioClear(CFG_GPIO_STNLCD_SCLK);

        if (c & (1u << i))
            ithGpioSet(CFG_GPIO_STNLCD_SID);
        else
            ithGpioClear(CFG_GPIO_STNLCD_SID);
        
        ithGpioSet(CFG_GPIO_STNLCD_SCLK);
    }
#endif
}

static void WriteCommand(uint8_t c)
{
    ithGpioClear(CFG_GPIO_STNLCD_A0);
    Write(c);
}

static void WriteData(uint8_t c)
{
    ithGpioSet(CFG_GPIO_STNLCD_A0);
    Write(c);
}

static void ClearScreen(void)
{
    uint8_t p, c;

#ifdef ENABLE_PARALLEL_INTERFACE  
    for (p = 0; p < CFG_STNLCD_HEIGHT / 8; p++)
    {
        WriteCommand(CMD_SET_PAGE | p);
        WriteCommand(CMD_SET_COLUMN_LOWER | (0 & 0xF));
        WriteCommand(CMD_SET_COLUMN_UPPER | ((0 >> 4) & 0xF));
        
        for (c = 0; c < (CFG_STNLCD_WIDTH) ; c++)
        {
            WriteData(0x00);
        }
    }
#else
    for (p = 0; p < CFG_STNLCD_HEIGHT / 8; p++)
    {
        WriteCommand(CMD_SET_PAGE | p);
        for (c = 0; c < CFG_STNLCD_WIDTH + 1; c++)
        {
            WriteCommand(CMD_SET_COLUMN_LOWER | (c & 0xF));
            WriteCommand(CMD_SET_COLUMN_UPPER | ((c >> 4) & 0xF));
            WriteData(0x0);
        }
    }
#endif
}

void itpStnLcdInit(void)
{
    // set pin directions
#ifdef ENABLE_PARALLEL_INTERFACE  
    uint8_t i;

    ithGpioSetOut(CFG_GPIO_STNLCD_RST);
    ithGpioEnable(CFG_GPIO_STNLCD_RST);

    ithGpioSetOut(CFG_GPIO_STNLCD_A0);
    ithGpioEnable(CFG_GPIO_STNLCD_A0);

    ithGpioSetOut(CFG_GPIO_STNLCD_WR);
    ithGpioEnable(CFG_GPIO_STNLCD_WR);

    ithGpioSetOut(CFG_GPIO_STNLCD_RD);
    ithGpioEnable(CFG_GPIO_STNLCD_RD);

    for(i=0; i<8; i++)
    {
        ithGpioSetOut(STNLCD_GPIO_DATA_PIN[i]);
        ithGpioEnable(STNLCD_GPIO_DATA_PIN[i]);
    }

    // toggle RST low to reset; CS low so it'll listen to us
    ithGpioClear(CFG_GPIO_STNLCD_RST);
    //while( ReadStatus() & LCM_STATUS_BUSY );
    usleep(10);	//need to wait > 2us(SPEC defination)
    ithGpioSet(CFG_GPIO_STNLCD_RST);
    
    // LCD off
    itpStnLcdDisable();
    
    // LCD display normal/reverse
    WriteCommand(CMD_SET_DISP_NORMAL);

    // LCD bias select (depend on brightness)
    WriteCommand(CMD_SET_BIAS_7);
    
    // ADC select
    WriteCommand(CMD_SET_ADC_NORMAL);
    
    // SHL select (x,y coordinate reverse)
    WriteCommand(CMD_SET_COM_NORMAL);
    
    // Initial display line
    WriteCommand(CMD_SET_DISP_START_LINE);
    
    // select booster ratio(2x 3x 4x)
    WriteCommand(CMD_SET_BOOSTER_FIRST);
    WriteCommand(CMD_SET_BOOSTER_234);
    
    // Set V0 output Voltage
    WriteCommand(0x81 | 0x00);    

    // Set brightness
    itpStnLcdSetBrightness(0x18);
    
    // set lcd operating voltage (regulator resistor, ref voltage resistor)
    WriteCommand(CMD_SET_RESISTOR_RATIO | 0x6);  
    
    //display all points (on/normal)
    WriteCommand(CMD_SET_ALLPTS_NORMAL);  
    
    ClearScreen(); 

    // turn on voltage follower (VC=1, VR=1, VF=1)
    WriteCommand(CMD_SET_POWER_CONTROL | 0x7);
    
    // wait
    //usleep(1000);    //original waiting time is 10ms
    
    itpStnLcdEnable();
#else
    ithGpioSetOut(CFG_GPIO_STNLCD_CS);
    ithGpioEnable(CFG_GPIO_STNLCD_CS);

    ithGpioSetOut(CFG_GPIO_STNLCD_RST);
    ithGpioEnable(CFG_GPIO_STNLCD_RST);

    ithGpioSetOut(CFG_GPIO_STNLCD_A0);
    ithGpioEnable(CFG_GPIO_STNLCD_A0);

    ithGpioSetOut(CFG_GPIO_STNLCD_SCLK);
    ithGpioEnable(CFG_GPIO_STNLCD_SCLK);

    ithGpioSetOut(CFG_GPIO_STNLCD_SID);
    ithGpioEnable(CFG_GPIO_STNLCD_SID);

    // toggle RST low to reset; CS low so it'll listen to us
    ithGpioClear(CFG_GPIO_STNLCD_CS);
    ithGpioClear(CFG_GPIO_STNLCD_RST);
    usleep(500000);
    ithGpioSet(CFG_GPIO_STNLCD_RST);

    // LCD bias select
    WriteCommand(CMD_SET_BIAS_7);
    // ADC select
    WriteCommand(CMD_SET_ADC_NORMAL);
    // SHL select
    WriteCommand(CMD_SET_COM_NORMAL);
    // Initial display line
    WriteCommand(CMD_SET_DISP_START_LINE);

    // turn on voltage converter (VC=1, VR=0, VF=0)
    WriteCommand(CMD_SET_POWER_CONTROL | 0x4);
    // wait for 50% rising
    usleep(50000);

    // turn on voltage regulator (VC=1, VR=1, VF=0)
    WriteCommand(CMD_SET_POWER_CONTROL | 0x6);
    // wait >=50ms
    usleep(50000);

    // turn on voltage follower (VC=1, VR=1, VF=1)
    WriteCommand(CMD_SET_POWER_CONTROL | 0x7);
    // wait
    usleep(10000);

    // set lcd operating voltage (regulator resistor, ref voltage resistor)
    WriteCommand(CMD_SET_RESISTOR_RATIO | 0x6);

    WriteCommand(CMD_DISPLAY_ON);
    WriteCommand(CMD_SET_ALLPTS_NORMAL);
    itpStnLcdSetBrightness(0x18);
    ClearScreen();
#endif 
}

void itpStnLcdEnable(void)
{
    WriteCommand(CMD_DISPLAY_ON);
}

void itpStnLcdDisable(void)
{
    WriteCommand(CMD_DISPLAY_OFF);
}

void itpStnLcdSetBrightness(unsigned int val)
{
    WriteCommand(CMD_SET_VOLUME_FIRST);
    WriteCommand(CMD_SET_VOLUME_SECOND | (val & 0x3F));
}

void itpStnLcdWriteBuffer(uint8_t* buf)
{
    uint8_t c, p;

    for (p = 0; p < CFG_STNLCD_HEIGHT / 8; p++)
    {
        WriteCommand(CMD_SET_PAGE | pagemap[p]);
        WriteCommand(CMD_SET_COLUMN_LOWER | (0x0 & 0xF));
        WriteCommand(CMD_SET_COLUMN_UPPER | ((0x0 >> 4) & 0xF));
        WriteCommand(CMD_RMW);
#ifdef ENABLE_PARALLEL_INTERFACE  
#else
        WriteData(0x00);
#endif

        for (c = 0; c < CFG_STNLCD_WIDTH; c++)
        {
            WriteData(buf[(128 * p) + c]);
        }
    }
}
