/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The thread functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_THREAD_H
#define PAL_THREAD_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Thread definition */
//typedef void* PAL_THREAD;

/** Thread name */
#define PAL_THREAD_MAIN             0
#define PAL_THREAD_PALFILE          1
#define PAL_THREAD_GETKEY           2
#define PAL_THREAD_MPS              3
#define PAL_THREAD_TS_DEMUX         4
#define PAL_THREAD_TS_AUDIO         5
#define PAL_THREAD_TS_VIDEO         6
#define PAL_THREAD_TS_SUB           7
#define PAL_THREAD_TS_TTX           8
#define PAL_THREAD_SRC_READ         9
#define PAL_THREAD_MP3              10
#define PAL_THREAD_STORAGE          11 // for storage mgr
#ifdef ENABLE_USB_DEVICE
    #define PAL_THREAD_USB_DEVICE   12
    #define PAL_THREAD_USBEX        13
    #define PAL_THREAD_JPEG         14
#else
    #define PAL_THREAD_USBEX        12
    #define PAL_THREAD_JPEG         13
#endif

#define PAL_THREAD_FILE             20 // win32 use only

/** Thread priority */
#define PAL_THREAD_PRIORITY_HIGHEST 60
#define PAL_THREAD_PRIORITY_HIGHER  80
#define PAL_THREAD_PRIORITY_NORMAL  100
#define PAL_THREAD_PRIORITY_LOWER   120
#define PAL_THREAD_PRIORITY_LOWEST  140

void
PalSleep(
    MMP_ULONG ms);

#if 0
void
PalUSleep(
    MMP_ULONG us);

typedef void *
(*PAL_THREAD_PROC)(
    void *arg);

PAL_THREAD
PalCreateThread(
    MMP_INT         name,
    PAL_THREAD_PROC proc,
    void            *arg,
    MMP_ULONG       stackSize,
    MMP_UINT        priority);

MMP_INT
PalDestroyThread(
    PAL_THREAD thread);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PAL_THREAD_H */