/*
 * Copyright (c) 2015 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL STC functions.
 *
 * @author I-Chun Lai
 * @version 1.0
 */
#include <errno.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "itp_cfg.h"

//#define STC_MAX_CNT         4
#define MACRO_RETURN_ERR                                            \
    {                                                               \
        errno = (ITP_DEVICE_STC << ITP_DEVICE_ERRNO_BIT) | EPERM;   \
        return -1;                                                  \
    }

#if 0
typedef enum
{
    STATE_FREE,
    STATE_STOP,
    STATE_PAUSE,
    STATE_RUN
} STC_STATE;

struct STCInfo
{
    STC_STATE   state;
    int64_t     offset;
    uint64_t    last_pause;
    uint64_t    pause_duration;
    uint64_t    duration;
} stc_info[STC_MAX_CNT] = {0};
#endif

extern STCInfo stc_info[];

static uint64_t _GetTickCount64()
{
    LARGE_INTEGER        t;
    double               microseconds;
    static LARGE_INTEGER offset;
    static double        frequencyToMicroseconds;
    static int           initialized = 0;
    static BOOL          usePerformanceCounter = 0;
    uint64_t             ticks;

    if (!initialized)
    {
        LARGE_INTEGER performanceFrequency;

        initialized = 1;
        QueryPerformanceFrequency(&performanceFrequency);
        QueryPerformanceCounter(&offset);
        frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
    }

    QueryPerformanceCounter(&t);
    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = (LONGLONG)microseconds;
    ticks = t.QuadPart * 9 / 100;
    return ticks;
}

static int StcOpen(const char* name, int flags, int mode, void* info)
{
    int index;
    for (index = 0; (index < STC_MAX_CNT) && (stc_info[index].state != STATE_FREE); index++);

    if (index == STC_MAX_CNT)   // it means none of stc devices are free...
        MACRO_RETURN_ERR;       // return operation is not permitted

    stc_info[index].state = STATE_STOP;
    return index;
}

static int StcClose(int file, void* info)
{
    int index = file;
    if ((index < 0) || (STC_MAX_CNT <= index))
        MACRO_RETURN_ERR;

    stc_info[index].state           = STATE_FREE;
    stc_info[index].offset          = 0;
    stc_info[index].last_pause      = 0;
    stc_info[index].pause_duration  = 0;
    stc_info[index].duration        = 0;

    return 0;
}

static int StcRead(int file, char *ptr, int len, void* info)
{
    int index = file;
    if ((index < 0) || (STC_MAX_CNT <= index) || (stc_info[index].state == STATE_FREE))
        MACRO_RETURN_ERR;

    if (len < sizeof (uint64_t))
        MACRO_RETURN_ERR;

    // duration = current - start_offset - total_pause
    if (stc_info[index].state == STATE_RUN)
        *(uint64_t*)ptr = _GetTickCount64() - stc_info[index].offset - stc_info[index].pause_duration;
    else
        *(uint64_t*)ptr = stc_info[index].duration;

    return sizeof(uint64_t);
}

static int StcWrite(int file, char *ptr, int len, void* info)
{
    uint64_t curr, offset;
    int index = file;

    if ((index < 0) || (STC_MAX_CNT <= index))
        MACRO_RETURN_ERR;

    //if (len < sizeof(uint64_t))
    //  MACRO_RETURN_ERR;

    if (stc_info[index].state == STATE_FREE)
        MACRO_RETURN_ERR;

    stc_info[index].state = STATE_RUN;
    //stc_info[index].offset            = 0;
    stc_info[index].last_pause      = 0;
    stc_info[index].pause_duration  = 0;
    stc_info[index].duration        = 0;

    offset = *(uint64_t *)ptr;
    curr = _GetTickCount64();
    stc_info[index].offset = (int64_t)curr - offset;
    //printf("\tcur(%llu) - offset(%llu) = %lld\n", curr, offset, stc_info[index].offset);
    return sizeof(uint64_t);
}

static void StcInit(void)
{
}

static int StcIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int index = file;
    if ((index < 0) || (STC_MAX_CNT <= index))
        MACRO_RETURN_ERR;

    switch (request)
    {
        case ITP_IOCTL_INIT:
            StcInit();
            break;

        case ITP_IOCTL_PAUSE:
            {
                int state = stc_info[index].state;
                switch(state)
                {
                    case STATE_RUN: // Set pause
                        {
                            uint64_t clk = _GetTickCount64();
                            stc_info[index].duration = clk - stc_info[index].offset - stc_info[index].pause_duration;
                            stc_info[index].last_pause = clk;
                            stc_info[index].state = STATE_PAUSE;
                            break;
                        }
                    case STATE_PAUSE: // Set resume
                        stc_info[index].pause_duration += _GetTickCount64() - stc_info[index].last_pause;
                        stc_info[index].state = STATE_RUN;
                        break;

                    default:
                        MACRO_RETURN_ERR;
                }

                // [Evan, 2011/11/07] support software pause only
                //ithStcCtrlEnable(ITH_STC_PAUSE);
                break;
            }

        default:
            errno = (ITP_DEVICE_STC << ITP_DEVICE_ERRNO_BIT) | 1;
            return -1;
    }
    return 0;
}

const ITPDevice itpDeviceStc =
{
    ":stc",
    StcOpen,
    StcClose,
    StcRead,
    StcWrite,
    itpLseekDefault,
    StcIoctl,
    NULL
};
