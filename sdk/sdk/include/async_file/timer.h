/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The timer functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_TIMER_H
#define PAL_TIMER_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _pal_uclock {
    MMP_ULONG sec;
    MMP_ULONG ms;
    MMP_ULONG us;
};

/** Clock definition */
typedef unsigned int PAL_CLOCK_T;
typedef struct _pal_uclock *PAL_UCLOCK_T;

#define PAL_CLOCKS_PER_SEC  1000
#define PAL_UCLOCKS_PER_SEC 1000000

/**
 * Get milliseconds
 *
 * @param               No.
 * @return              milliseconds
 */
PAL_CLOCK_T
PalGetClock(
    void);

/**
 * Return the duration with milliseconds.
 * It about 49 days on 4G milliseconds for maximun value.
 *
 * @param clock         last time
 * @return              milliseconds
 */
MMP_ULONG
PalGetDuration(
    PAL_CLOCK_T clock);

/**
 * Get microseconds
 *
 * @param               No.
 * @return              microseconds
 */
PAL_UCLOCK_T
PalGetUClock(
    void);

/**
 * Return the duration with microseconds.
 * It about 71 minutes on 4G microseconds for maximun value.
 * The overhead to call this function about 10 ~ 60 us.
 *
 * @param clock         last time
 * @return              microseconds
 */
MMP_ULONG
PalGetUDuration(
    PAL_UCLOCK_T clock);

#ifdef ENABLE_DEBUG_MSG_OUT
void
dbg_measureTime(
    MMP_INT  id,
    MMP_CHAR *prefix);
#else

    #define dbg_measureTime(a, b)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PAL_TIMER_H */