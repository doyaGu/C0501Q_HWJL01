/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL print buffer functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"

#ifdef __OPENRTOS__

#include "openrtos/FreeRTOS.h"

extern char* __printbuf_addr;
extern int __printbuf_size;
extern int __printbuf_ptr;

#else

char __printbuf_addr[CFG_DBG_PRINTBUF_SIZE];
int __printbuf_size = CFG_DBG_PRINTBUF_SIZE;
int __printbuf_ptr = 0;

#endif // __OPENRTOS__

int itpPrintBufPutchar(int c)
{
#ifdef __OPENRTOS__
    portSAVEDISABLE_INTERRUPTS();
#endif

    if (__printbuf_ptr >= __printbuf_size)
        __printbuf_ptr = 0;

    __printbuf_addr[__printbuf_ptr++] = (char) c;

    ithFlushDCacheRange(&__printbuf_addr[__printbuf_ptr], 1);
    ithFlushDCacheRange(&__printbuf_ptr, 4);
    
#ifdef __OPENRTOS__
    portRESTORE_INTERRUPTS();
#endif

    return c;
}

int itpPrintBufWrite(int file, char *ptr, int len, void* info)
{
    int i, prev_ptr = __printbuf_ptr;
    
#ifdef __OPENRTOS__
    portSAVEDISABLE_INTERRUPTS();
#endif

    for (i = 0; i < len && ptr[i] != 0; i++)
    {
        if (__printbuf_ptr >= __printbuf_size)
            __printbuf_ptr = 0;

        __printbuf_addr[__printbuf_ptr++] = ptr[i];
    }
    
    if (__printbuf_ptr < prev_ptr)
    {
        ithFlushDCacheRange(&__printbuf_addr[prev_ptr], __printbuf_size - prev_ptr);
        ithFlushDCacheRange(__printbuf_addr, __printbuf_ptr);
    }
    else
    {
        ithFlushDCacheRange(&__printbuf_addr[prev_ptr], __printbuf_ptr - prev_ptr);
    }
    ithFlushDCacheRange(&__printbuf_ptr, 4);

#ifdef __OPENRTOS__
    portRESTORE_INTERRUPTS();
#endif

    return i;
}

static int PrintBufIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        ithPutcharFunc = itpPrintBufPutchar;
        break;
        
    default:
        errno = (ITP_DEVICE_PRINTBUF << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDevicePrintBuf =
{
    ":printbuf",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpPrintBufWrite,
    itpLseekDefault,
    PrintBufIoctl,
    NULL
};
