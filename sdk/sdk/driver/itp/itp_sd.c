/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL SD functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"
#include "ite/ite_sd.h"

#define ITP_SD_OUTOF_POS        0x0101

typedef struct
{
    int index;
    uint32_t sectorCnt;
    uint32_t blockSize;
    uint32_t pos;
} ITPSd;

static int SdOpen(const char* name, int flags, int mode, void* info)
{
    ITPSd* ctxt = (ITPSd*)info;
    return ctxt->index;
}

static int SdClose(int file, void* info)
{
    ITPSd* ctxt = (ITPSd*)info;
    iteSdTerminateEx(ctxt->index);
    ctxt->pos = 0;
    ctxt->blockSize = 0;
    ctxt->sectorCnt = 0;
    return 0;
}

static int SdRead(int file, char *ptr, int len, void* info) // len: sector count
{
    ITPSd* ctxt = (ITPSd*)info;
    int res;

    if((ctxt->pos+len) > ctxt->sectorCnt)
    {
        len = ctxt->sectorCnt - ctxt->pos;
        errno = (ctxt->index ? ITP_DEVICE_SD1 : ITP_DEVICE_SD0 << ITP_DEVICE_ERRNO_BIT) | ITP_SD_OUTOF_POS;
    }
    if(len==0)
        return len;

    res = iteSdReadMultipleSectorEx(ctxt->index, ctxt->pos, len, (void*)ptr);
    if(res)
    {
        len = 0;
        errno = (ctxt->index ? ITP_DEVICE_SD1 : ITP_DEVICE_SD0 << ITP_DEVICE_ERRNO_BIT) | res;
    }
    else
        ctxt->pos += len;

    return len;
}

static int SdWrite(int file, char *ptr, int len, void* info)
{
    ITPSd* ctxt = (ITPSd*)info;
    int res;

    if((ctxt->pos+len) > ctxt->sectorCnt)
    {
        len = ctxt->sectorCnt - ctxt->pos;
        errno = (ctxt->index ? ITP_DEVICE_SD1 : ITP_DEVICE_SD0 << ITP_DEVICE_ERRNO_BIT) | ITP_SD_OUTOF_POS;
    }
    if(len==0)
        return len;

    res = iteSdWriteMultipleSectorEx(ctxt->index, ctxt->pos, len, (void*)ptr);
    if(res)
    {
        len = 0;
        errno = (ctxt->index ? ITP_DEVICE_SD1 : ITP_DEVICE_SD0 << ITP_DEVICE_ERRNO_BIT) | res;
    }
    else
        ctxt->pos += len;

    return len;
}

static int SdLseek(int file, int ptr, int dir, void* info)  // ptr: sector unit
{
    ITPSd* ctxt = (ITPSd*)info;
    switch(dir)
    {
    default:
    case SEEK_SET:
        ctxt->pos = ptr;
        break;
    case SEEK_CUR:
        ctxt->pos += ptr;
        break;
    case SEEK_END:
        break;
    }
    return ctxt->pos;
}

static int SdIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int res;
    ITPSd* ctxt = (ITPSd*)info;
    switch (request)
    {
    case ITP_IOCTL_INIT:
        {
            #if defined(CFG_MMC_ENABLE)
            {
                int rc;
                SD_CARD_INFO card_info = { 0 };
                rc = iteSdcInitialize(ctxt->index, &card_info);
                if (rc)
                    return -2;
                if (card_info.type == SD_TYPE_SDIO)
                    return -3;
            }
            #endif
            int retry = 3;
            do {
                res = iteSdInitializeEx(ctxt->index);
            } while(res && retry--);
            if(res)
            {
                errno = (ctxt->index ? ITP_DEVICE_SD1 : ITP_DEVICE_SD0 << ITP_DEVICE_ERRNO_BIT) | res;
                return -1;
            }
        }
        break;

    case ITP_IOCTL_GET_BLOCK_SIZE:
        res = iteSdGetCapacityEx(ctxt->index, &ctxt->sectorCnt, &ctxt->blockSize);
        if(res)
        {
            errno = (ctxt->index ? ITP_DEVICE_SD1 : ITP_DEVICE_SD0 << ITP_DEVICE_ERRNO_BIT) | res;
            return -1;
        }
        else
            *(unsigned long*)ptr = ctxt->blockSize;
        break;

    case ITP_IOCTL_GET_GAP_SIZE:
        *(unsigned long*)ptr = 0;
        break;

    default:
        errno = (ctxt->index ? ITP_DEVICE_SD1 : ITP_DEVICE_SD0 << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}
#ifdef CFG_SD0_ENABLE
    static ITPSd itpSd0 = {SD_0,0,0,0};
    const ITPDevice itpDeviceSd0 =
    {
        ":sd0",
        SdOpen,
        SdClose,
        SdRead,
        SdWrite,
        SdLseek,
        SdIoctl,
        (void*)&itpSd0
    };
#endif // CFG_SD0_ENABLE

#ifdef CFG_SD1_ENABLE
    static ITPSd itpSd1 = {SD_1,0,0,0};
    const ITPDevice itpDeviceSd1 =
    {
        ":sd1",
        SdOpen,
        SdClose,
        SdRead,
        SdWrite,
        SdLseek,
        SdIoctl,
        (void*)&itpSd1
    };
#endif // CFG_SD1_ENABLE
