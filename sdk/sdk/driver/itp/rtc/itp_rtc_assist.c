/*
 * Copyright (c) 2016 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * assist RTC software functions.
 *
 * @author Joseph Chang
 * @version 1.0
 *
 * @discription:
 * use assist RTC to prevent from ext-RTC I/O frequently access issue
 *
 * if CHIP==985X then use internal RTC without timer(include micro-second)
 * if CHIP!=985X then use timer for assist RTC
 *
 */
#include <time.h>
#include "../itp_cfg.h"

/********************************************
 * compile option
 ********************************************/
#ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE

#if (CFG_CHIP_FAMILY == 9850)
//#define CFG_EXTRTC_USE_INTERNAL_RTC
#define CFG_EXTRTC_USE_TIMER
#else
#define CFG_EXTRTC_USE_TIMER
#endif

#ifdef	CFG_EXTRTC_USE_TIMER
#define RTC_TIMER		ITH_TIMER4			//CFG_RTC_TIMER
#define RTC_TIMER_INT	ITH_INTR_TIMER4		//CFG_RTC_INT_TIMER
#endif

#ifdef	CFG_EXTRTC_USE_INTERNAL_RTC
#if (CFG_CHIP_FAMILY == 9850)
    #define USE_INTERNAL_RTC_MICROSECOND_REG
    #define ENABLE_INTERNAL_RTC_DIVSRC_FROM_WCLK
#endif

#ifdef	USE_INTERNAL_RTC_MICROSECOND_REG
    //#define ENABLE_FINETUNE_MICROSECOND_SOLUTION
#else
    #define RTC_TIMER		ITH_TIMER4		//CFG_RTC_TIMER
#endif

#endif	//CFG_EXTRTC_USE_INTERNAL_RTC
#endif	//CFG_RTC_REDUCE_IO_ACCESS_ENABLE

/********************************************
 * MACRO defination
 ********************************************/
#ifdef	 CFG_EXTRTC_USE_TIMER
static void _timerInit(void);
static long _timerGetTime(long* us);
static void _timerSetTime(long sec, long us);

#define 	_aRtcInit		_timerInit
#define 	_aRtcGetTime	_timerGetTime
#define 	_aRtcSetTime	_timerSetTime
#endif

#ifdef	 CFG_EXTRTC_USE_INTERNAL_RTC
static void _internalRtcInit(void);
static long _internalRtcGetTime(long* usec);
static void _internalRtcSetTime(long sec, long usec);

#define 	_aRtcInit		_internalRtcInit
#define 	_aRtcGetTime	_internalRtcGetTime
#define 	_aRtcSetTime	_internalRtcSetTime
#endif

/********************************************
 * global variable
 ********************************************/
static long	gTimerSec = 0;

static long	gRtcFirstInit = 1;
static long	gUsAdjRatio = 0;
/********************************************
 * Private function protocol
 ********************************************/

/********************************************
 * Private function 
 ********************************************/
#ifdef	CFG_EXTRTC_USE_TIMER
static void _timerIntrHandler(void* arg)
{
    gTimerSec++;
}

static void _timerInit(void)
{
    printf("_atimerInit:1\n");
    /* Start with tick timer interrupts disabled... */
    ithTimerDisable(RTC_TIMER);
    
    /* Start with tick timer interrupts disabled... */
    ithIntrDisableIrq(RTC_TIMER_INT);
    
    // init timer4 to calc usec of gettimeofday()
    ithTimerReset(RTC_TIMER);
    
    /* Calculate timer compare value to achieve the desired tick rate... */
    ithTimerCtrlEnable(RTC_TIMER, ITH_TIMER_UPCOUNT);
    ithTimerCtrlEnable(RTC_TIMER, ITH_TIMER_PERIODIC);    
    ithTimerSetCounter(RTC_TIMER, 0);
    ithTimerSetMatch(RTC_TIMER, ithGetBusClock());  
      
    /* Start tick timer... */
    ithTimerEnable(RTC_TIMER);
        
    // init timer interrupt
    /* Clear any pending tick timer interrupts... */
    ithTimerClearIntr(RTC_TIMER);
    ithIntrClearIrq(RTC_TIMER_INT);
    ithIntrRegisterHandlerIrq(RTC_TIMER_INT, _timerIntrHandler, NULL);
    ithIntrSetTriggerModeIrq(RTC_TIMER_INT, ITH_INTR_EDGE);

    /* Enable the tick timer interrupt... */
    ithIntrEnableIrq(RTC_TIMER_INT);
    
    //get time
    
    //set
    
    printf("_atimerInit:2\n");

}

long _timerGetTime(long* usec)
{
    long sec1, sec2;
    static long lastMs=0, cMs;
    
    if (usec)
    {
    	*usec = ithTimerGetTime(RTC_TIMER);
    	
    	cMs = *usec/100000;
    }
    
    sec1 = gTimerSec;
    
    //if(cMs!=lastMs)
    {
        LOG_DBG "_tRtcGT:us=%06d, %d\n",*usec,sec1 LOG_END
        lastMs = cMs;
    }

    return sec1;
}

static void _timerSetTime(long sec, long usec)
{
    ithIntrDisableIrq(RTC_TIMER_INT);
    gTimerSec = sec;
    ithIntrEnableIrq(RTC_TIMER_INT);
    
    if(usec==0)	
    {
    	ithTimerSetCounter(RTC_TIMER, 0);
    }
}
#endif	//CFG_EXTRTC_USE_TIMER


#ifdef	CFG_EXTRTC_USE_INTERNAL_RTC
static void _iRtcIntrHandler(void* arg)
{
#ifndef	USE_INTERNAL_RTC_MICROSECOND_REG
    ithTimerSetCounter(RTC_TIMER, 0);  // reset counter
#endif
    ithRtcClearIntr(ITH_RTC_SEC);
}

#ifdef	USE_INTERNAL_RTC_MICROSECOND_REG
static void _internalRtcEnUsClkDiv(uint32_t clkDiv)
{
#ifdef	ENABLE_FINETUNE_MICROSECOND_SOLUTION  
    ithWriteRegA(ITH_TIMER_BASE + 0xB0, clkDiv/1000000);
#else
    ithWriteRegA(ITH_TIMER_BASE + 0xB0, (clkDiv/1000000)-1);    
#endif
    ithWriteRegMaskA(ITH_TIMER_BASE + 0xB0, 1<<31, 1<<31);
}

static long _internalRtcGetUs(void)
{
    long aUs = 0;
    long us = (long)ithReadRegA(ITH_TIMER_BASE + 0xB4) & 0x00FFFFFF;
    
    #ifdef	ENABLE_FINETUNE_MICROSECOND_SOLUTION
    if( gUsAdjRatio && (us<1100000) )
    {
        //for project solution
        LOG_DBG "A solution\n" LOG_END
        aUs = (us*1000)/gUsAdjRatio;
    }
    else
    {
        //for bootloader solution
        LOG_DBG "B solution\n" LOG_END
        aUs = us%1000000;
    }
    #else
    aUs = us;
    #endif
    
    while(aUs>999999)	aUs -= 1000000;

    LOG_DBG "iRtcUs:%06d,%06d\n",us,aUs LOG_END
    return aUs;
}

//auto fine-tune mocro-second to match RTC (source clock is 80 Mhz)    
static long _internalRtcGetUsAdjustRatio(void)
{
    long sec1, sec2, us1, us2;
    long cnt=0, adjRatio, adjCnt=0;
    
    while(1)
    {
    	//get us = 0
    	usleep(10000);
    	LOG_DBG "adj:1\n" LOG_END
    	sec1 = ithRtcGetTime();
    	LOG_DBG "adj:2, %d\n",sec1 LOG_END
    	
    	cnt=0;
    	do
    	{
    	    sec2 = ithRtcGetTime();
    	    if(cnt++>2000)	break;
    	    usleep(1000);    	    
    	}while(sec1 == sec2);
    	
    	if(cnt>2000)
    	{
    	    LOG_INFO "cnt>2000, counterA not work, c=%d\n",cnt LOG_END
    		break;
    	}
    	LOG_DBG "adj:3, %d\n",sec2 LOG_END
    	
    	cnt = 0;
    	sec1 = sec2;//ithRtcGetTime();
    	do
    	{
    	    us1 = (long)ithReadRegA(ITH_TIMER_BASE + 0xB4) & 0x00FFFFFF;
    	    sec2 = ithRtcGetTime();
    	    us2 = (long)ithReadRegA(ITH_TIMER_BASE + 0xB4) & 0x00FFFFFF;
    	    //usleep(1000);
    	    if(cnt++>1000000)	break;
    	}while(sec1 == sec2);
    	
    	LOG_DBG "adj:4, %06d,%06d,(%d)\n",us1,us2,cnt LOG_END
    	if(cnt>1000000)
    	{
    	    LOG_INFO "cnt>1000000, counter not work, c=%d\n",cnt LOG_END
    		break;
    	}
    	
    	//get 1 second us counter
    	adjRatio = (us1)/1000;    	
    	if( (us1%1000)>500 )	adjRatio++;//aound up if >500us 
    	
    	LOG_DBG "adjFcr1:%d(%d)\n",adjRatio,us1 LOG_END
    	if( (adjRatio>950) && (adjRatio<1050) )	break;
    	
    	if(adjCnt++>10)
    	{
    	    LOG_INFO "adjust times has over %d\n",adjCnt LOG_END
    		break;  	   	
        }
    }
    
    if( (adjRatio<950) || (adjRatio>1050) )
    {
        LOG_DBG "adjust us ratio from %d to 1000\n",adjRatio LOG_END
        adjRatio = 1000;   
    }
    
    LOG_INFO "internal Rtc micro-second adjust ratio:%d(%d)\n",adjRatio,us1 LOG_END
    return adjRatio;
}
#endif

static void _internalRtcInit(void)
{
    printf("internal RTC init:\n");
    #ifdef ENABLE_INTERNAL_RTC_DIVSRC_FROM_WCLK
    ithRtcSetDivSrc(ITH_RTC_DIV_SRC_INNER_12MHZ);
    #else
    ithRtcSetDivSrc(ITH_RTC_DIV_SRC_EXT_32KHZ);
    #endif
    
    ithRtcInit(CFG_RTC_EXTCLK);
    
    if (ithRtcEnable() && gRtcFirstInit)
    {
        LOG_DBG "First time boot\n" LOG_END
        ithRtcSetTime(CFG_RTC_DEFAULT_TIMESTAMP);
        gRtcFirstInit = 0;        
        
        #ifdef ENABLE_INTERNAL_RTC_DIVSRC_FROM_WCLK
        ithRtcSetDivSrc(ITH_RTC_DIV_SRC_INNER_12MHZ);
        #endif
    }
    
    #ifndef	USE_INTERNAL_RTC_MICROSECOND_REG
    // init timer4 to calc usec of gettimeofday()    
    ithTimerReset(RTC_TIMER);
    ithTimerCtrlEnable(RTC_TIMER, ITH_TIMER_UPCOUNT);
    ithTimerSetCounter(RTC_TIMER, 0);
    ithTimerEnable(RTC_TIMER);

    // init rtc sec interrupt
    ithRtcCtrlEnable(ITH_RTC_INTR_SEC);
    ithIntrRegisterHandlerIrq(ITH_INTR_RTCSEC, _iRtcIntrHandler, NULL);
    ithIntrSetTriggerModeIrq(ITH_INTR_RTCSEC, ITH_INTR_EDGE);
    ithIntrEnableIrq(ITH_INTR_RTCSEC);
    #endif    
    
        
    #ifdef	USE_INTERNAL_RTC_MICROSECOND_REG
    //To enable IT985X function of internal RTC micro-second (wclk=80Mhz)
    {
        LOG_DBG "iRTC GOT BUS clock: %d\n",ithGetBusClock() LOG_END
        _internalRtcEnUsClkDiv(ithGetBusClock());
        #ifdef	ENABLE_FINETUNE_MICROSECOND_SOLUTION
        gUsAdjRatio = _internalRtcGetUsAdjustRatio();
        #endif
    }
    #endif
}	

long _internalRtcGetTime(long* usec)
{
    long sec1, sec2;
    static long lastMs=0, cMs;
    
    do 
    {
        sec1 = ithRtcGetTime();
        if (usec)
        {
            #ifdef	USE_INTERNAL_RTC_MICROSECOND_REG
                *usec = _internalRtcGetUs();
            #else
                *usec = ithTimerGetTime(RTC_TIMER);
            #endif
                
            cMs = *usec/100000;
        }
        sec2 = ithRtcGetTime();        
    } while (sec1 != sec2);

    if(cMs!=lastMs)
    {
        LOG_DBG "_xRtcGT:us=%06d, %d\n",*usec,sec1 LOG_END
        lastMs = cMs;
    }

    return sec1;
}

static void _internalRtcSetTime(long sec, long us)
{
    LOG_DBG "inRtc:%d, %d\n",sec,us LOG_END
    
    ithRtcSetTime(sec + (us / 1000000));
    
    #ifndef	USE_INTERNAL_RTC_MICROSECOND_REG
    if(us==0)	
    {
    	ithTimerSetCounter(RTC_TIMER, 0);
    	//LOG_DBG "set iRTC us=0\n" LOG_END
    }
    #endif
}
#endif	//CFG_EXTRTC_USE_INTERNAL_RTC


/*************************
public funtion
**************************/
void assistRtcInit(void)
{
    _aRtcInit();
}

long assistRtcGetTime(long* us)
{
    return _aRtcGetTime(us);
}

void assistRtcSetTime(long sec, long us)
{
    LOG_DBG "aRtc:%d, %d\n",sec,us LOG_END
    _aRtcSetTime(sec,us);    
}

