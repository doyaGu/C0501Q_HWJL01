/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL WatchDog functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <time.h>
#include "itp_cfg.h"

static bool wdEnabled;

#ifdef CFG_WATCHDOG_IDLETASK
static uint32_t wdLastTick;
#endif

static void WatchDogRefreshHandler(timer_t timerid, int arg)
{
    if (wdEnabled)
        ithWatchDogRestart();

    //ithPrintf("ithWatchDogRestart(),reload=%d,cnt=%d\n", ithWatchDogGetReload(), ithWatchDogGetCounter());
    
    #ifdef	CFG_EXT_WATCHDOG_ENABLE
	if(ithGpioGet(CFG_GPIO_EXT_WATCHDOG))	
		ithGpioClear(CFG_GPIO_EXT_WATCHDOG);
	else	
		ithGpioSet(CFG_GPIO_EXT_WATCHDOG);
    #endif 
}

#ifdef CFG_WATCHDOG_IDLETASK

static void WatchDogIdleRefreshHandler(void)
{
    if (itpGetTickDuration(wdLastTick) >= CFG_WATCHDOG_REFRESH_INTERVAL * 1000)
    {
        WatchDogRefreshHandler(0, 0);
        wdLastTick = itpGetTickCount();
    }
}
#endif // CFG_WATCHDOG_IDLETASK

#ifdef CFG_WATCHDOG_INTR

static void WatchDogIntrHandler(void* arg)
{
    ithWatchDogCtrlDisable(ITH_WD_INTR);

    ithPrintf("Watch Dog Reboot!!!\n");

    #if (CFG_CHIP_FAMILY == 9850) && defined(CFG_CHIP_REV_A0) && defined(CFG_NAND_ENABLE)
    //for workaround the 9850's SPI engine issue if SPI NAND rebooting 
    ithWriteRegA(ITH_SSP0_BASE + 0x74, 0x00000000);
    #endif
    
#ifdef CFG_DBG_UART0
    while (!ithUartIsTxEmpty(ITH_UART0));
#elif defined(CFG_DBG_UART1)
    while (!ithUartIsTxEmpty(ITH_UART1));
#endif
    ithDelay(10);

    ithWatchDogCtrlEnable(ITH_WD_RESET);
    ithWatchDogSetReload(0);
    ithWatchDogRestart();
    ithWatchDogEnable();
}
#endif // CFG_WATCHDOG_INTR

static void WatchDogInit(void)
{
    ithWatchDogSetTimeout(CFG_WATCHDOG_TIMEOUT * 1000);
    //printf("ithWatchDogSetTimeout(),reload=%d,cnt=%d\n", ithWatchDogGetReload(), ithWatchDogGetCounter());
    wdEnabled = false;
    
#ifdef	CFG_EXT_WATCHDOG_ENABLE
	ithGpioSetMode(CFG_GPIO_EXT_WATCHDOG,ITH_GPIO_MODE0);
   	ithGpioSetOut(CFG_GPIO_EXT_WATCHDOG);
   	ithGpioSet(CFG_GPIO_EXT_WATCHDOG);    	
   	ithGpioEnable(CFG_GPIO_EXT_WATCHDOG);
#endif

#if defined(CFG_WATCHDOG_IDLETASK) && CFG_WATCHDOG_REFRESH_INTERVAL > 0

    itpRegisterIdleHandler(WatchDogIdleRefreshHandler);

#elif CFG_WATCHDOG_REFRESH_INTERVAL > 0
    {
        timer_t timer;
        struct itimerspec value;
        timer_create(CLOCK_REALTIME, NULL, &timer);
        timer_connect(timer, (VOIDFUNCPTR)WatchDogRefreshHandler, 0);
        value.it_value.tv_sec = value.it_interval.tv_sec = CFG_WATCHDOG_REFRESH_INTERVAL;
        value.it_value.tv_nsec = value.it_interval.tv_nsec = 0;
        timer_settime(timer, 0, &value, NULL);
    }
#endif // defined(CFG_WATCHDOG_IDLETASK) && CFG_WATCHDOG_REFRESH_INTERVAL > 0

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlEnable(ITH_WD_INTR);
    ithIntrRegisterHandlerIrq(ITH_INTR_WD, WatchDogIntrHandler, NULL);
    ithIntrEnableIrq(ITH_INTR_WD);

#else
    ithWatchDogCtrlEnable(ITH_WD_RESET);

#endif // CFG_WATCHDOG_INTR
}

static int WatchDogIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        WatchDogInit();
        break;

	case ITP_IOCTL_RESET:
		WatchDogRefreshHandler(0, 0);
		break;

    case ITP_IOCTL_ENABLE:
        ithWatchDogRestart();
        ithWatchDogEnable();
        wdEnabled = true;
        //printf("ithWatchDogEnable(),reload=%d,cnt=%d\n", ithWatchDogGetReload(), ithWatchDogGetCounter());
        break;

    case ITP_IOCTL_DISABLE:
        ithWatchDogDisable();
        wdEnabled = false;
        break;

    default:
        errno = (ITP_DEVICE_WATCHDOG << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceWatchDog =
{
    ":watchdog",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    WatchDogIoctl,
    NULL
};
