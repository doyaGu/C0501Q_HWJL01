/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL statistic functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <malloc.h>
#include <time.h>
#include "lwip/stats.h"
#include "itp_cfg.h"

#ifdef __OPENRTOS__
#include "openrtos/FreeRTOS.h"
#endif

extern caddr_t __heap_start__;
extern caddr_t __heap_end__;

static timer_t statsTimerId;

#ifdef CFG_DBG_STATS_FAT
    ITPFSStats itpStatsFat;
#endif

#ifdef CFG_DBG_STATS_NTFS
    ITPFSStats itpStatsNtfs;
#endif

static void StatsHandler(timer_t timerid, int arg)
{
    static signed char buf[4096];
    uint32_t servCount, servNum, cycle, req0Num, req1Num;

#if defined(CFG_DBG_STATS_HEAP) && defined(__OPENRTOS__)
    struct mallinfo mi = mallinfo();
    unsigned long total = (unsigned int)&__heap_end__ - (unsigned int)&__heap_start__;

    // heap usage
    printf("HEAP newlib: usage=%d/%d(%d%%),addr=0x%X\n",
        mi.uordblks,
        total,
        (int)(100.0f * mi.uordblks / total),
        &__heap_start__);

#ifdef CFG_DBG_MEMLEAK
    dbg_mem_stat();
#endif

#ifdef CFG_DBG_RMALLOC
    Rmalloc_stat(__FILE__);
#endif

#endif // defined(CFG_DBG_STATS_HEAP) && defined(__OPENRTOS__)

#if defined(CFG_DBG_STATS_TASK_LIST) && defined(__OPENRTOS__)
    // task list
    puts("TASKLIST:");
    vTaskList(buf);
    printf(buf);
#endif // defined(CFG_DBG_STATS_TASK_LIST) && defined(__OPENRTOS__)

#if defined(CFG_DBG_STATS_TASK_TIME) && defined(__OPENRTOS__)
    // task time usage
    printf("TASKTIME:");
    vTaskGetRunTimeStats(buf);
    printf(buf);
#endif // defined(CFG_DBG_STATS_TASK_TIME) && defined(__OPENRTOS__)

    // memory bandwidth usage
#ifdef CFG_DBG_STATS_MEM_BANDWIDTH
    ithMemStatServCounterDisable();

    servNum = ithMemStatGetServNum();
    servCount = ithMemStatGetAllServCount();
    cycle = ithMemStatGetServCount();
    req0Num = ithMemStatGetServ0Num();
    req1Num = ithMemStatGetServ1Num();
    ithMemStatSetServ0Request(ITH_MEMSTAT_ARM);
    ithMemStatSetServ1Request(ITH_MEMSTAT_RISC);

    printf("MEMSTAT: total=(util:%d%% BW:%dMB/s), req0=(req/total:%d%% util:%d%%), req1=(req/total:%d%% util:%d%%)\n",
        (int)(servNum * 100.0f / servCount),
        (int)((servCount * 8) / (cycle / (float)ithGetMemClock()) / 1000000),
        (int)(req0Num * 100.0f / servNum),
        (int)(req0Num * 100.0f / servCount),
        (int)(req1Num * 100.0f / servNum),
        (int)(req1Num * 100.0f / servCount));

    ithMemStatServCounterEnable();

#endif // CFG_DBG_STATS_MEM_BANDWIDTH

#ifdef CFG_DBG_STATS_TCPIP
    // lwip stats
    printf("TCP/IP STAT:");
    stats_display();
#endif // CFG_DBG_STATS_TCPIP


#ifdef CFG_DBG_STATS_FAT
    {
        int readSpeed = 0, writeSpeed = 0;

    #ifdef __OPENRTOS__
        portENTER_CRITICAL();
    #endif

        if (itpStatsFat.readTime > 0)
            readSpeed = itpStatsFat.readSize * 1000 / itpStatsFat.readTime;

        if (itpStatsFat.writeTime > 0)
            writeSpeed = itpStatsFat.writeSize * 1000 / itpStatsFat.writeTime;

        itpStatsFat.readSize = itpStatsFat.readTime = itpStatsFat.writeSize = itpStatsFat.writeTime = 0;

    #ifdef __OPENRTOS__
        portEXIT_CRITICAL();
    #endif

        if (readSpeed || writeSpeed)
            printf("FAT STAT: read %d B/s (%dB/%dms), write %d B/s (%dB/%dms)\n", readSpeed, itpStatsFat.readSize, itpStatsFat.readTime, writeSpeed, itpStatsFat.writeSize, itpStatsFat.writeTime);
    }
#endif // CFG_DBG_STATS_FAT

#ifdef CFG_DBG_STATS_NTFS
    {
        int readSpeed = 0, writeSpeed = 0;

    #ifdef __OPENRTOS__
        portENTER_CRITICAL();
    #endif

        if (itpStatsNtfs.readTime > 0)
            readSpeed = itpStatsNtfs.readSize * 1000 / itpStatsNtfs.readTime;

        if (itpStatsNtfs.writeTime > 0)
            writeSpeed = itpStatsNtfs.writeSize * 1000 / itpStatsNtfs.writeTime;

        itpStatsNtfs.readSize = itpStatsNtfs.readTime = itpStatsNtfs.writeSize = itpStatsNtfs.writeTime = 0;

    #ifdef __OPENRTOS__
        portEXIT_CRITICAL();
    #endif

        if (readSpeed || writeSpeed)
            printf("NTFS STAT: read %d bytes/sec, write %d bytes/sec\n", readSpeed, writeSpeed);
    }
#endif // CFG_DBG_STATS_NTFS
}

void itpStatsInit(void)
{
    struct itimerspec value;

    value.it_value.tv_sec       = CFG_DBG_STATS_PERIOD;
    value.it_value.tv_nsec      = 0;
    value.it_interval.tv_sec    = CFG_DBG_STATS_PERIOD;
    value.it_interval.tv_nsec   = 0;

#ifdef CFG_DBG_STATS_MEM_BANDWIDTH
    ithMemStatSetServCountPeriod(0xFFFF);
    ithMemStatSetServ0Request(ITH_MEMSTAT_ARM);
    ithMemStatSetServ1Request(ITH_MEMSTAT_AHB);
    ithMemStatServCounterEnable();
#endif // CFG_DBG_STATS_MEM_BANDWIDTH

    timer_create(CLOCK_REALTIME, NULL, &statsTimerId);
    timer_connect(statsTimerId, StatsHandler, 0);
    timer_settime(statsTimerId, 0, &value, NULL);

#ifdef CFG_DBG_STATS_FAT
    itpStatsFat.readSize = itpStatsFat.readTime = itpStatsFat.writeSize = itpStatsFat.writeTime = 0;
#endif

#ifdef CFG_DBG_STATS_NTFS
    itpStatsNtfs.readSize = itpStatsNtfs.readTime = itpStatsNtfs.writeSize = itpStatsNtfs.writeTime = 0;
#endif
}
