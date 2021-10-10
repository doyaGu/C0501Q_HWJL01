/*
 * Copyright (c) 2015 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL CtrlBoard codec functions.
 *
 * @author Barry Wu
 * @version 1.0
 */

#include <errno.h>
#include "itp_cfg.h"
#include "ite/audio.h"
#include "ite/ite_risc.h"

enum {
    CMD_CTRLBOARD_CODEC_INIT            = 31,
    CMD_CTRLBOARD_CODEC_WRITE           = 32,
    CMD_CTRLBOARD_CODEC_READ            = 33,
    CMD_CTRLBOARD_CODEC_HEARTBEAT_WRITE = 34,
    CMD_CTRLBOARD_CODEC_HEARTBEAT_READ  = 35
};

static int CtrlBoardCodecWrite(int file, char *ptr, int len, void *info)
{
    //printf("CtrlBoardCodecWrite\n");
    ithCodecCtrlBoardWrite((uint8_t *)ptr, len);
    if (ithCodecCommand(CMD_CTRLBOARD_CODEC_WRITE, len, (int)NULL, (int)NULL) == false)
    {
        printf("risc codec is not running");
    }
    return len;
}

static int CtrlBoardCodecIoctl(int file, unsigned long request, void *ptr, void *info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        // risc3 combine with risc2 (for audio codec) to fire
        iteRiscOpenEngine(ITE_SW_PERIPHERAL_ENGINE, 0);
#ifndef _WIN32
        PalSleep(2000);
#endif
        break;

    case ITP_IOCTL_SET_GPIO_PIN:
        if (ithCodecCommand(CMD_CTRLBOARD_CODEC_INIT, *(int *)ptr, *(int *)((uint8_t *)ptr + sizeof(int)), *(int *)((uint8_t *)ptr + 2 * sizeof(int))) == false)
            printf("risc codec is not running!!!\n");
        break;

    case ITP_IOCTL_GET_HEARTBEAT:
        break;

    default:
        errno = (ITP_DEVICE_CTRLBOARD << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpCtrlBoard =
{
    ":ctrlboard_codec",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    CtrlBoardCodecWrite,
    itpLseekDefault,
    CtrlBoardCodecIoctl,
    (void *)CMD_CTRLBOARD_CODEC_INIT
};