/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL STC functions.
 *
 * @author Evan Chang
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

#define MACRO_RETURN_ERR                                            \
    {                                                               \
        errno = (ITP_DEVICE_STC << ITP_DEVICE_ERRNO_BIT) | EPERM;   \
        return -1;                                                  \
    }

static STCInfo stc_info[STC_MAX_CNT] = {0};

static int StcOpen(const char* name, int flags, int mode, void* info)
{
    int index;
    for (index = 0; (index < STC_MAX_CNT) && (stc_info[index].state != STATE_FREE); index++);

    if (index == STC_MAX_CNT)   // it means none of all stc device are free...
        MACRO_RETURN_ERR;       // return operation is not permitted

    stc_info[index].state = STATE_STOP;
    stc_info[index].stcBaseCountHi = 0;
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
    stc_info[index].stcBaseCountHi  = 0;

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
    {
        *(uint64_t*)ptr = ithStcGetBaseClock64(&stc_info[index]) - stc_info[index].offset - stc_info[index].pause_duration;
    }    
    else
    {
        *(uint64_t*)ptr = stc_info[index].duration;
    }    

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
    curr = ithStcGetBaseClock64(&stc_info[index]);
    stc_info[index].offset = (int64_t)curr - offset;
    return sizeof(uint64_t);
}

static void StcInit(void)
{    
    // PCR clock settings move to initial script
    ithSetRegBitH(ITH_PCR_CLOCK_REG2, 0x1); // Enable PCLK
    ithWriteRegMaskH(ITH_PCR_SETTING_REG, 0xc000, 0xc000); //Enable and Update PCR Counter   
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
                            uint64_t clk = ithStcGetBaseClock64(&stc_info[index]); //Benson
                            stc_info[index].duration = clk - stc_info[index].offset - stc_info[index].pause_duration;
                            stc_info[index].last_pause = clk;
                            stc_info[index].state = STATE_PAUSE;
                            break;
                        }
                    case STATE_PAUSE: // Set resume
                        stc_info[index].pause_duration += ithStcGetBaseClock64(&stc_info[index]) - stc_info[index].last_pause;
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
