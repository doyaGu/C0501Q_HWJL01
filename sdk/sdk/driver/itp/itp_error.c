/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL error handling functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "itp_cfg.h"
#include <execinfo.h>
#include <malloc.h>
#include <sys/ioctl.h>

#ifdef __OPENRTOS__
    #include "openrtos/FreeRTOS.h"
    #include "openrtos/task.h"
#endif // __OPENRTOS__

#define BACKTRACE_SIZE 100

#ifdef _WIN32

static char printbuf[CFG_DBG_PRINTBUF_SIZE + 4];
static char* __printbuf_addr = printbuf;
static int __printbuf_size = CFG_DBG_PRINTBUF_SIZE;
static int __printbuf_ptr;

#else

extern char* __printbuf_addr;
extern int __printbuf_size;
extern int __printbuf_ptr;

#endif // _WIN32

static void *btbuf[BACKTRACE_SIZE];
static int btcount;
static bool aborted = false;

static void InitConsole(void)
{
#ifdef CFG_DBG_BLUESCREEN
    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceLcdConsole);
    ioctl(ITP_DEVICE_STD, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_STD, ITP_IOCTL_SET_FGCOLOR, (void*)ITH_RGB565(255, 255, 255));
    ioctl(ITP_DEVICE_STD, ITP_IOCTL_SET_BGCOLOR, (void*)ITH_RGB565(0, 0, 255));
#endif // CFG_DBG_BLUESCREEN
}

static void PrintStatus(void)
{
    // clock
    ithClockStats();

    // cmdq
#ifdef CFG_CMDQ_ENABLE
    ithCmdQStats();
#endif
}

static void Reboot(void)
{
#if (CFG_CHIP_FAMILY == 9850) && defined(CFG_CHIP_REV_A0) && defined(CFG_NAND_ENABLE)
    //for workaround the 9850's SPI engine issue if SPI NAND rebooting 
    ithWriteRegA(ITH_SSP0_BASE + 0x74, 0x00000000);
#endif

#ifdef CFG_WATCHDOG_ENABLE
    ithPrintf("Reboot...\n");
    ithWatchDogEnable();
    ithWatchDogSetReload(0);
    ithWatchDogRestart();
#endif // CFG_WATCHDOG_ENABLE
}

void itpErrorUndef( void ) __naked;
void itpErrorUndef(void)
{
    int i;

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE);

    InitConsole();

    ithPrintf("Undefined Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintStatus();

    Reboot();
    while (1);
}

void itpErrorPrefetchAbort( void ) __naked;
void itpErrorPrefetchAbort(void)
{
    int i;

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE);

    InitConsole();

    ithPrintf("Prefetch Abort Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintStatus();

    Reboot();
    while (1);
}

void itpErrorDataAbort( void ) __naked;
void itpErrorDataAbort(void)
{
    int i;

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE);

    InitConsole();

    ithPrintf("Data Abort Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintStatus();

    Reboot();
    while (1);
}

void itpErrorStackOverflow( void ) __naked;
void itpErrorStackOverflow(void)
{
    int i;

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
    btcount = backtrace(btbuf, BACKTRACE_SIZE);

    InitConsole();

    ithPrintf("Stack Overflow Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintStatus();

    Reboot();
    while (1);
}

#ifdef CFG_MEMDBG_ENABLE
void itpErrorMemDbgAbort( void ) __naked;
void itpErrorMemDbgAbort(void)
{
    int i;

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
#ifndef _WIN32
    btcount = itpBacktraceIrq(btbuf, BACKTRACE_SIZE);
#endif
    InitConsole();

    ithPrintf("Memory Debug Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintStatus();

    Reboot();
    while (1);
}
#endif // CFG_MEMDBG_ENABLE

void itpErrorDivideByZero( void ) __naked;
void itpErrorDivideByZero(void)
{
    int i;

#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithWatchDogCtrlEnable(ITH_WD_RESET);
#endif

    if (aborted)
        while (1);

    aborted = true;
#ifdef __SM32__
    btcount = 0;    // FIXME: workaround align exception bug
#else
    btcount = backtrace(btbuf, BACKTRACE_SIZE);
#endif

    InitConsole();

    ithPrintf("Divide by Zero Error: %d\n", btcount);

    // backtrace
    for (i = 0; i < btcount; i++)
        ithPrintf("0x%X ", btbuf[i]);

    ithPrintf("\n");

    PrintStatus();

    Reboot();
    while (1);
}
