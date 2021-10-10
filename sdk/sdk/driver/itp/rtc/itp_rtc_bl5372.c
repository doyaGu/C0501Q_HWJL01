/*
 * Copyright (c) 2017 CVTE RX. Inc. All Rights Reserved.
 */
/** @file
 * PAL RTC software functions.
 *
 * @author ZhangXiangLong
 * @version 1.0
 */
#include <time.h>
#include <unistd.h>
#include "../itp_cfg.h"

/********************************************
* MACRO defination
********************************************/
#define IIC_RTC_ID  (0x64 >> 1)   //RTC从地址

#define RTC_SECOND_COM      0x00        //RTC秒读写指令
#define RTC_MINUTE_COM      0x10        //RTC分读写指令        
#define RTC_HOUR_COM        0x20        //RTC时读写指令
#define RTC_WEEK_COM        0x30        //RTC周读写指令
#define RTC_DAY_COM         0x40        //RTC天读写指令
#define RTC_MONTH_COM       0x50        //RTC月读写指令
#define RTC_YEAR_COM        0x60        //RTC年读写指令
#define RTC_ADJUST_OSC      0x70        //RTC晶振调整指令

#define RTC_MINUTE_A        0x80        //定时器A秒读写指令        
#define RTC_HOUR_A          0x90        //定时器A时读写指令  
#define RTC_DAY_A           0xA0        //定时器A天读写指令  
#define RTC_MINUTE_B        0xB0        //定时器B秒读写指令        
#define RTC_HOUR_B          0xC0        //定时器B时读写指令  
#define RTC_DAY_B           0xD0        //定时器B天读写指令

#define RTC_INT_MODE_COM    0xE0         //中断模式选择指令
#define RTC_INT_FG_COM      0xF0         //中断标志读写指令

#define ADJUST_OSC          0x00         //32.768KHz
#define INTRB_ENABLE        0x24         //INTRB为电平模式，秒周期中断
#define CLEAR_INTR          0x28         //清除所有中断，设置为24时制
#define CVTE_2036_SEC       2114179200   //2036年12月30号的时间戳
#define CVTE_2017_SEC       1483200000   //2017年1月1号的时间戳

/* the defination of tm structure ***********
struct tm
{
	int tm_sec;		//0-59
	int tm_min;		//0-59
	int tm_hour;	//0-23
	int tm_mday;	//1-31
	int tm_mon;		//0-11
	int tm_year;	//00-99(from 1900)
	int tm_wday;	//0-6(0:sun 1:mon 2:...)
}
*********************************************/

/**
* add cvte_tqw 2017-8-5{
* 在执行u盘获取Log的时候，在RTC中printf打印会导致重启问题，因此在这注释掉printf打印改用ithPrintf中断打印
*/
//精简代码并防止在该函数中误用了printf（）而导致重启2017-8-10
#define printf(...)    ithPrintf(__VA_ARGS__);
//add cvte_tqw 2017-8-5}

/*********************************************
[00H]			//sec   :0~59
[01H]			//min   :0~59
[02H]			//hour  :0~23
[03H]			//week  :0~6 (0:sun, 1:mon, 2:tue,..., 6:sat)
[04H]			//day   :1~31 (without 0)
[05H]			//mon   :1~12 (1:jan, 2:feb, 3:mar..., 12:Dec)(without 0)
[06H]			//year  :0~99(0:2000, 1:2001,...., 99:2099)

but for linux system defination
t->tm_mon			//month :0~11
t->tm_year			//year  :00~ from 1900~(ex: t->tm_year=115 means 2015)
*********************************************/

/********************************************
 * global variable
 ********************************************/
static struct tm    tInfo;
static int	        gRtcFd=-1;

/********************************************
 * Private function 
 ********************************************/
static uint8_t _bin2bcd (uint8_t x)
{
    return (x % 10) | ((x / 10) << 4);
}

static uint8_t _bcd2bin (uint8_t x)
{
    return (x >> 4) * 10 + (x & 0x0F);
}

static uint8_t _BCD2DEC(uint8_t byte)
{
    uint8_t B1 = (byte >> 4);
    uint8_t B2 = (byte & 0x0F);

    if((B1 > 9) || (B2 > 9))
    {
        printf("[itp-RTC] _BCD2DEC something wrong, BCD=%x, B1=%x, B2=%x\r\n", byte, B1, B2); //ithPrintf
        B1 = B1 % 9;
        B2 = B2 % 9;
        return 0x00;
    }

    return (B1 * 10) + B2;
}

static uint8_t _DEC2BCD(uint8_t dec)
{
    uint8_t B1 = dec / 10;
    uint8_t B2 = dec % 10;
	
    if(dec >= 100)
    {
        printf("[itp-RTC] _DEC2BCD something wrong, dec=%x, B1=%x, B2=%x\r\n", dec, B1, B2);
        return 0x00;
    }
	
    return (B1<<4) | B2;
}

/**
* This function is added by cvte_hgf, To check if the RTC has been stopped.
*/
static int _readRtcCtrlReg(uint8_t *dBuf, uint8_t dLen)
{
    ITPI2cInfo 	evt;
    uint8_t		I2cCmd;
    int 		i2cret;

    I2cCmd = RTC_INT_FG_COM;

    evt.slaveAddress = IIC_RTC_ID;
    evt.cmdBuffer = &I2cCmd;
    evt.cmdBufferSize = 1;
    evt.dataBuffer = dBuf;
    evt.dataBufferSize = dLen;

    i2cret = read(gRtcFd, &evt, 1);

    if (i2cret < 0)
    {
        printf("[RTC ERROR].iic read fail, %s [#%d]\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}
static int _readRtcReg(uint8_t *dBuf, uint8_t dLen)
{
    ITPI2cInfo 	evt;
    uint8_t		I2cCmd;
    int 		i2cret;

    I2cCmd = RTC_SECOND_COM;

    evt.slaveAddress   = IIC_RTC_ID;
    evt.cmdBuffer      = &I2cCmd;
    evt.cmdBufferSize  = 1;
    evt.dataBuffer     = dBuf;
    evt.dataBufferSize = dLen;

    i2cret = read(gRtcFd, &evt, 1);

    if (i2cret < 0)
    {
        printf("[RTC ERROR].iic read fail, %s [#%d]\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

static int _writeRtcReg(uint8_t *wBuf, uint8_t wLen)
{
    ITPI2cInfo 	evt;
    int 		i2cret;

    evt.slaveAddress   = IIC_RTC_ID;
    evt.cmdBuffer      = wBuf;
    evt.cmdBufferSize  = wLen;
    evt.dataBuffer     = 0;
    evt.dataBufferSize = 0;	

    i2cret = write(gRtcFd, &evt, 1);

    if (i2cret != wLen)
    {
        printf("wrRtcReg fail, R=%x, len=%x\n", i2cret, wLen);
    }

    return i2cret;
}

static long _extRtcGetTime(void)
{
    uint8_t data[7];
    int i = 0;
    long rtcSec=0;

    // 2017/6/28, add by cvte_hgf, 判断时钟是否有效。 @{
    uint8_t ctrlData;
    if (0 == _readRtcCtrlReg(&ctrlData, 1))
    {
        if (0x10 == (ctrlData & 0x10))
        {
            // 2017/6/29, add by cvte_hgf, 设置RTC为24小时制，清除停振标志。 @{
            data[0] = RTC_INT_FG_COM;
            data[1] = 0x20;
            _writeRtcReg(data, 2);
            // 2017/6/29, add by cvte_hgf. @}
            printf("Invalid time \n");
        }
        if (0x20 == (ctrlData & 0x20))
        {
            //printf(" 24 hour mode\n");
        }
    }
    // 2017/6/28， add by cvte_hgf @}

    if (_readRtcReg(data, 7) == 0)
    {
        tInfo.tm_sec    = _BCD2DEC(data[0]);	    //sec   :0~59
        tInfo.tm_min    = _BCD2DEC(data[1]);	    //min   :0~59
        tInfo.tm_hour   = _BCD2DEC(data[2]);	    //hour  :0~23
        tInfo.tm_mday   = _BCD2DEC(data[4]);	    //day   :1~31 (without 0)
        tInfo.tm_wday   = data[3];				    //week  :0~6 (0:sun, 1:mon, 2:tue,..., 6:sat)
        tInfo.tm_mon    = _BCD2DEC(data[5]) - 1;    //mon   :1~12 (1:jan, 2:feb, 3:mar..., 12:Dec)(without 0, but linux month define is 0~11)
        tInfo.tm_year   = _BCD2DEC(data[6]) + 100;  //year  :0~99(0:2000, 1:2001,...., 99:2099)(but linux year define 0:1900)
        // 2017/6/28, add by cvte_hgf, debug information. @{
        //for (i = 0; i < 7; i++)
        //{
        //    printf("data is %x\n", data[i]);
        //}
        // 2017/6/28, add by cvte_hgf. @}
     	rtcSec = mktime((struct tm*)&tInfo);
    }
    else
    {
        printf("RTC I2C read fail\r\n");
    	rtcSec = 0;
    }

    return rtcSec;
}

void itpRtcInit(void)
{
    uint8_t data[2];
    long  rtcSec, rtcusec;
    printf("RTC BL5372 INITIAL~~~\r\n");

#ifdef	CFG_RTC_I2C1
    gRtcFd = open(":i2c1", O_RDWR, 0);
#else
    gRtcFd = open(":i2c0", O_RDWR, 0);
#endif

    if (gRtcFd == -1)
    {
        printf("RTC error: open fd fail(fd=%d)! [#%d]\n", gRtcFd, __LINE__);
    }
#ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE  
    assistRtcInit();
#endif

    //晶振调整
    data[0] = RTC_ADJUST_OSC;
    data[1] = ADJUST_OSC;
    _writeRtcReg(data, 2);

    // 2017/6/28, add by cvte_hgf, 设置RTC为24小时制，清除停振标志。 @{
    data[0] = RTC_INT_FG_COM;
    data[1] = 0x20;
    _writeRtcReg(data, 2);
    // 2017/6/28, add by cvte_hgf. @}
    // 2017/5/12, modified out by cvte_lzq, To set time to RTC module. @{
    rtcSec = itpRtcGetTime(&rtcusec);
    if (rtcSec <= CVTE_2017_SEC || rtcSec >= CVTE_2036_SEC)
    {
#ifdef CFG_RTC_DEFAULT_TIMESTAMP
        itpRtcSetTime(CFG_RTC_DEFAULT_TIMESTAMP, 0);
#else
        itpRtcSetTime(1483228800, 0); //2017-01-01 00:08:00
#endif
    }
    else
    {
        // 2017/6/20, commented out by cvte_hgf, Not need to set again. @{
        //itpRtcSetTime(rtcSec, 0);
        // @}
        printf("Rtc unix timestamp: %d\n\n", rtcSec);
    }
    // @}
    // 2017/4/17, commented out by cvte_hgf, To save real time in RTC module. @{
#if 0
#ifdef CFG_RTC_DEFAULT_TIMESTAMP
    itpRtcSetTime(CFG_RTC_DEFAULT_TIMESTAMP, 0);
#else
    itpRtcSetTime(1483228800, 0); //2017-01-01 00:00:00
#endif
#endif
    // @}
}

long itpRtcGetTime(long* usec)
{
    long rtcSec = 0;

    //printf("Enter external RTC read.\n");
    ithEnterCritical();

    rtcSec = _extRtcGetTime();
    *usec = 0;  //微秒忽略处理

    ithExitCritical();

    return rtcSec;
}

void itpRtcSetTime(long sec, long usec)
{
    uint8_t data[8];
    int     rst;
	struct  tm *t = localtime(&sec);

    ithEnterCritical();

    memset(data, 0, sizeof(data));	

    data[0] = RTC_SECOND_COM;
    data[1] = _DEC2BCD(t->tm_sec);
    data[2] = _DEC2BCD(t->tm_min);
    data[3] = _DEC2BCD(t->tm_hour);
    data[4] = t->tm_wday % 7;
    data[5] = _DEC2BCD(t->tm_mday);
    data[6] = _DEC2BCD(t->tm_mon + 1);      //mon   :1~12 (1:jan, 2:feb, 3:mar..., 12:Dec)(without 0, but linux month define is 0~11)
    data[7] = _DEC2BCD(t->tm_year - 100);   //year  :0~99(0:2000, 1:2001,...., 99:2099)(but linux year define 0:1900)

	rst = _writeRtcReg(data, 8);

    if (rst != 8)
    {
        printf("[RTC] write reg error(r=%d):[#%d]\n", rst, __LINE__);
    }

    ithExitCritical();
}
