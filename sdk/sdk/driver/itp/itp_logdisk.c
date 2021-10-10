/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Log to disk functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "itp_cfg.h"

static FILE* itpLogDiskFile;

static int LogDiskPutchar(int c)
{
    if (itpLogDiskFile)
    {
        return fputc(c, itpLogDiskFile);
    }
    return EOF;
}

void LogDiskInit(const char* path)
{
    itpLogDiskFile = fopen(path, "w");
    if (itpLogDiskFile)
	    ithPutcharFunc = LogDiskPutchar;
}

static int LogDiskWrite(int file, char *ptr, int len, void* info)
{
    if (itpLogDiskFile)
    {
        int ret = fwrite(ptr, 1, len, itpLogDiskFile);
        fflush(itpLogDiskFile);
        return ret;
    }
    return EOF;
}

static int LogDiskIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
	case ITP_IOCTL_INIT:
        LogDiskInit((const char*)ptr);
        break;
        
    default:
        errno = (ITP_DEVICE_LOGDISK << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceLogDisk =
{
    ":logdisk",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    LogDiskWrite,
    itpLseekDefault,
    LogDiskIoctl,
    NULL
};
