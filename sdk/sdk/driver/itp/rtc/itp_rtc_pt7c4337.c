﻿/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL RTC software functions.
 *
 * @author Joseph Chang
 * @version 1.0
 */
#include <time.h>
#include "../itp_cfg.h"

//#define ENABLE_DBG_MSG
/********************************************
 * MACRO defination
 ********************************************/
#define IIC_RTC_ID          (0xD0>>1)
 
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

/* the defination of pt7c4337's register *****
REG[0] : Seconds (00-59)
REG[1] : Minutes (00-59)
REG[2] : Hours (00-23)
REG[3] : week (1-7, 1:sun 2:mon 3:...)
REG[4] : Dates (1-31)
REG[5] : Months (1-12)
REG[6] : Years (00-99)
REG[7~15] : ...
*********************************************/

/********************************************
 * global variable
 ********************************************/
static struct tm tInfo;
static int	gRtcFd=-1;

static long	gLastSyncSec = 0;

#ifdef	ENABLE_DBG_MSG
static int	gEnMsgCnt=0;
#endif

/********************************************
 * Private function protocol
 ********************************************/
static long _extRtcGetTime();

/********************************************
 * Private function 
 ********************************************/
static uint8_t _bin2bcd (uint8_t x)
{
    return (x%10) | ((x/10) << 4);
}

static uint8_t _bcd2bin (uint8_t x)
{
    return (x >> 4) * 10 + (x & 0x0f);
}

static uint8_t _BCD2DEC(uint8_t byte)
{
	uint8_t B1 = (byte>>4);
	uint8_t B2 = (byte&0xF);
	
	if( (B1>9) || (B2>9) )
	{
		LOG_DBG "[itp-RTC] _BCD2DEC something wrong, BCD=%x, B1=%x, B2=%x\n",byte,B1,B2 LOG_END
		B1 = B1 %9;
		B2 = B2 %9;
		return 0x00;
	}
	
	return (B1*10) + B2;
}

static uint8_t _DEC2BCD(uint8_t dec)
{
	uint8_t B1 = dec/10;
	uint8_t B2 = dec%10;
		
	if( dec>=100 )
	{
		LOG_DBG "[itp-RTC] _DEC2BCD something wrong, dec=%x, B1=%x, B2=%x\n",dec,B1,B2 LOG_END
		return 0x00;
	}
	
	return (B1<<4) | B2;
}

static int _readRtcReg(uint8_t *dBuf, uint8_t dLen)
{
	ITPI2cInfo 	evt;
	uint8_t		I2cCmd;
	int 		i2cret;
	
	I2cCmd = 0x00;
	evt.slaveAddress   = IIC_RTC_ID;
	evt.cmdBuffer      = &I2cCmd;
	evt.cmdBufferSize  = 1;
	evt.dataBuffer     = dBuf;
	evt.dataBufferSize = dLen;	
	
	i2cret = read(gRtcFd, &evt, 1);

	if(i2cret<0)
	{
		LOG_ERR "[RTC ERROR].iic read fail, %s [#%d]\n", __FILE__, __LINE__ LOG_END
		return -1;		
	}
	
	return 0;
}

static int _writeRtcReg(uint8_t *wBuf, uint8_t wLen)
{
	ITPI2cInfo 	evt;
	int 		i2cret;
	uint8_t myBuf[16];
	
	memset(myBuf,0,16);
	memcpy(&myBuf[1],wBuf,wLen);
	
	evt.slaveAddress   = IIC_RTC_ID;
	evt.cmdBuffer      = &myBuf[0];
	evt.cmdBufferSize  = wLen+1;
	evt.dataBuffer     = 0;
	evt.dataBufferSize = 0;	
		
	i2cret = write(gRtcFd, &evt, 1);
	
	if(i2cret!=wLen+1)	LOG_ERR "wrRtcReg fail, R=%x, len=%x\n",i2cret,wLen LOG_END
	
	return i2cret-1;
}

static long _extRtcGetTime(void)
{
    uint8_t data[8];
    long rtcSec=0;

    if ( _readRtcReg(data, 8)==0 ) 
    {
    	#ifdef	ENABLE_DBG_MSG
    	if(gEnMsgCnt++<20000)
    	{
    		uint32_t i;
    		printf("	read ex-RTC reg:");
    		for(i=0; i<8; i++)	printf("%02x ",data[i]);
    		printf("\n");
    	}
    	#endif
    	
    	if(data[0]&0x80)	printf("[RTC warning] VL bit has detected!!(The voltage of RTC's battery is low)\n");

        tInfo.tm_sec = _BCD2DEC(data[0] & 0x7F);	//sec:0~59
        tInfo.tm_min = _BCD2DEC(data[1] & 0x7F);	//min:0~59
        tInfo.tm_hour = _BCD2DEC(data[2] & 0x3F);	//hour:0~23
        tInfo.tm_wday = (data[3] & 0x07) - 1;			//week:1~7 (1:sun, 2:mon, 3:tue,..., 7:sat)(but linux week define is 0~6)       
        tInfo.tm_mday = _BCD2DEC(data[4] & 0x3F);	//day:1~31 (without 0)
        tInfo.tm_mon = _BCD2DEC(data[5] & 0x1F) - 1;//mon:1~12 (1:jan, 2:feb, 3:mar..., 12:Dec)(without 0, but linux month define is 0~11)
        tInfo.tm_year = _BCD2DEC(data[6]);	//year:2000~2099
        if (data[7] & 0x01) 
        {
            tInfo.tm_year += 100;					//bit7 0&1 (0:year=19XX, 1:year=20XX)
        }
        
        #ifdef	ENABLE_DBG_MSG
        //tInfo.tm_year=115;tInfo.tm_mon=0;tInfo.tm_mday=27;tInfo.tm_wday=2;tInfo.tm_hour=16;tInfo.tm_min=2;tInfo.tm_sec=3;
        if(gEnMsgCnt%30)
        {
        	LOG_DBG "[rtc]yy-mm-dd=%d-%d-%d(w:%d), hr:min:sec=%02d:%02d:%02d\n",tInfo.tm_year,tInfo.tm_mon+1,tInfo.tm_mday,tInfo.tm_wday, tInfo.tm_hour,tInfo.tm_min,tInfo.tm_sec LOG_END
        	//LOG_DBG "[rSys]yy-mm-dd=%d-%d-%d(w:%d), hr:min:sec=%02d:%02d:%02d\n",t->tm_year,t->tm_mon+1,t->tm_mday,t->tm_wday, t->tm_hour,t->tm_min,t->tm_sec LOG_END
        }
        #endif
     	rtcSec = mktime((struct tm*)&tInfo);
    }
    else
    {
    	LOG_ERR "ex-RTC iic read fail, %s [#%d]\n", __FILE__, __LINE__ LOG_END
    	rtcSec = 0;
    }

    //LOG_DBG "extRtcSec=%d\n",rtcSec LOG_END

    return rtcSec;
}

#ifdef	ENABLE_DBG_MSG
static int _test_RtcI2CBus(int fd)
{
	ITPI2cInfo 	evt;
	uint8_t		I2cCmd;
	int 		i2cret;
	
	I2cCmd = 0x2;
	evt.slaveAddress   = IIC_RTC_ID;
	evt.cmdBuffer      = &I2cCmd;
	evt.cmdBufferSize  = 0;
	evt.dataBuffer     = &I2cCmd;
	evt.dataBufferSize = 0;	
		
	i2cret = write(fd, &evt, 1);
	
	LOG_DBG "_test_RtcI2CBus, R=%x\n",i2cret LOG_END
	
	return i2cret;
}
#endif

/********************************************
 * Public function 
 ********************************************/
//I don't know if PT7C4337 needs to use the "CFG_RTC_DEFAULT_TIMESTAMP"
//I think(guess) external RTC maybe not need this "time stamp"
//I think(guess) external RTC can get correct time at any-time without S/W initialization. 
void itpRtcInit(void)
{
    #ifdef	ENABLE_DBG_MSG
    long	usec = 0, currTime;
    #endif

	LOG_INFO "RTC PT7C4337 INITIAL~~~\n" LOG_END
    
    #ifdef	CFG_RTC_I2C1
    gRtcFd = open(":i2c1", O_RDWR);
    #else
    gRtcFd = open(":i2c0", O_RDWR);
    #endif
    
    if(gRtcFd==-1)	LOG_ERR "RTC error: open fd fail(fd=%d)! [#%d]\n", gRtcFd, __LINE__ LOG_END  
    
    #ifdef	ENABLE_DBG_MSG
    //if(_test_RtcI2CBus(gRtcFd))	printf("IIC bus test FAIL\n");
    //else							printf("IIC bus test PASS\n");
    
    #ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE  
    assistRtcInit(); 
    #endif
    
    itpRtcSetTime(1420070400 + 60*60*24*121 + 60*60*15 + 60*29 + 33 + 18144000 + 86400*3, 0);
    
    usleep(100000);
    
    currTime = _extRtcGetTime();
    LOG_DBG " current time is %d (h:%d, m:%d, s:%d)\n",currTime, (currTime/3600)%24, (currTime/60)%60, currTime%60 LOG_END
    #endif	//ENABLE_DBG_MSG
}

/*
 * At present, I skip the information of micro-second level 
 * Becouse this RTC can't handle this job(micro second)
 * maybe need to combine the information of IT9079's timer...
 * However, this solution needs to consider the IIC timing issue.
 * (IIC needs several decades ms to get RTC info, 
 * and it's hard to handle(sync) these two timers in us-level)
 */
long itpRtcGetTime(long* usec)
{
    uint8_t data[8];
    long rtcSec=0;
    
    #ifdef	ENABLE_DBG_MSG
    long 	tSmp = 1420070400 + 60*60*24*26 + 60*60*16 + 123; //time stamp of 2015 : 1420070400
    struct tm *t = gmtime(&tSmp);
    #endif
    
    ithEnterCritical();

   	rtcSec = _extRtcGetTime();
    *usec = 0;   	

    ithExitCritical();

    return rtcSec;
}

void itpRtcSetTime(long sec, long usec)
{
    uint8_t data[16];
    int     rst;
	struct  tm *t = localtime(&sec);    	
    #ifdef	ENABLE_DBG_MSG
    long    testSec;
	#endif
	
    ithEnterCritical();

    memset(data, 0, sizeof(data));	
	
	data[0] = _DEC2BCD(t->tm_sec);
    data[1] = _DEC2BCD(t->tm_min);
    data[2] = _DEC2BCD(t->tm_hour);
    data[3] = t->tm_wday%7 + 1;
    data[4] = _DEC2BCD(t->tm_mday);
    
    if (t->tm_year > 99) 
    {
        data[5] = _DEC2BCD(t->tm_mon + 1);
        data[6] = _DEC2BCD(t->tm_year % 100);
        data[7] = 0x01;
    }
    else 
    {
    	data[5] = _DEC2BCD(t->tm_mon + 1);
    	data[6] = _DEC2BCD(t->tm_year);
    	data[7] = 0x00;
    }
    
    #ifdef	ENABLE_DBG_MSG
    printf("\n");
    printf("[wSYS]yy-mm-dd=%d-%d-%d(w:%d), hr:min:sec=%d:%d:%d\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_wday,  t->tm_hour,t->tm_min,t->tm_sec);    
    {
    	int i;
    	printf("	RtcReg[0~7]=[");
    	for(i=0; i<8; i++)	printf("%02x ",data[i]);
    	printf("]\n");
    }

	testSec = mktime(t);	
	if(testSec==sec)	printf("time stamp are matched(%d)\n",sec);
	else				printf("time stamp are NOT matched, ori=%d, test=%d",sec,testSec);
	#endif
	
	rst = _writeRtcReg(data, 8);
	
    if(rst!=8)	LOG_ERR "[RTC] write reg error(r=%d):[#%d]\n", rst, __LINE__ LOG_END
    
    ithExitCritical();
}
