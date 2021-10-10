/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL RTC functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <time.h>
#include "itp_cfg.h"

#ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE
static long	gLastSyncSec = 0;
static long	gRtcSyncPeriod = CFG_RTC_SYNC_PERIOD;
#endif

static int RtcRead(int file, char *ptr, int len, void* info)
{
    if (len > sizeof(long))
    {
        long* value = (long*)ptr;
        
        #ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE
        long us, aRtc = assistRtcGetTime(&us);
        if( (aRtc - gLastSyncSec) > gRtcSyncPeriod )
        {
            aRtc = itpRtcGetTime(&us);
            if(aRtc != assistRtcGetTime(&us))
            {
                assistRtcSetTime(aRtc,0);
            }   
            gLastSyncSec = aRtc;
            LOG_DBG "	[EXT-RTC]Do Sync RTC, syncS==%x, us=%06d\n",gLastSyncSec,us LOG_END
        }
        *value = aRtc;
        #else
        *value = itpRtcGetTime(NULL);
        #endif

        return sizeof(long);
    }
    return 0;
}

static int RtcWrite(int file, char *ptr, int len, void* info)
{
    if (len > sizeof(long))
    {
        long* value = (long*)ptr;
        itpRtcSetTime(*value, 0);
        
        #ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE
        {
            assistRtcSetTime(*value,0); 
        }
        #endif
        	
        return sizeof(long);
    }
    return 0;
}

static int RtcIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        itpRtcInit();
        #ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE
        {
            long us;
            assistRtcInit(); 
            assistRtcSetTime(itpRtcGetTime(&us), 0);
        }
        #endif  
        break;

    case ITP_IOCTL_GET_TIME:
        {
            struct timeval* tv = (struct timeval*)ptr;
            
            #ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE
            long aRtc = assistRtcGetTime(&tv->tv_usec);
            if( (aRtc - gLastSyncSec) > gRtcSyncPeriod )
            {
                long s1 = 0, s2 = aRtc;
                int abs = 0;
                long rUs = 0;

                s1 = itpRtcGetTime(&rUs);                
                if( s1 != s2 )
                {                	
                	if(s1 > s2)	abs = s1 - s2;
                	else		abs = s2 - s1;
                
                	if(abs>=2)
                    {
                    	assistRtcSetTime(s1,0);
                    	tv->tv_usec = 0;
                	}
                }   
                gLastSyncSec = aRtc;
                LOG_DBG "	Do Sync Sec with ext-RTC, syncS=%x, us=%06d\n",gLastSyncSec,tv->tv_usec LOG_END
            }
            tv->tv_sec = aRtc;
           	#else
            tv->tv_sec = itpRtcGetTime(&tv->tv_usec);
            #endif
        }
        break;

    case ITP_IOCTL_SET_TIME:
        {
            struct timeval* tv = (struct timeval*)ptr;
            itpRtcSetTime(tv->tv_sec, tv->tv_usec);
            
        	#ifdef	CFG_RTC_REDUCE_IO_ACCESS_ENABLE
      	    assistRtcSetTime(tv->tv_sec,0);
        	#endif
        }
        break;

    default:
        errno = (ITP_DEVICE_RTC << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceRtc =
{
    ":rtc",
    itpOpenDefault,
    itpCloseDefault,
    RtcRead,
    RtcWrite,
    itpLseekDefault,
    RtcIoctl,
    NULL
};
