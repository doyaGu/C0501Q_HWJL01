/*
 * Copyright (c) 2014 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Software UART codec functions.
 *
 * @author Vincent Lee
 * @version 1.0
 */

#include <errno.h>
#include "itp_cfg.h"
#include "ite/audio.h"

enum{
    CMD_SWUART_DBG_CODEC_INIT = 51,
    CMD_SWUART_DBG_CODEC_WRITE = 52,
    CMD_SWUART_DBG_CODEC_SETPARITY = 53,
};

typedef struct
{
    int gpio;
    int baudrate;   
} ITPSwUartCodecDbg;

static ITPSwUartCodecDbg itpSwUart = {50, 38400};

static int SwUartCodecDbgOpen(const char* name, int flags, int mode, void* info)
{
	return 0;
}


static int SwUartCodecDbgWrite(int file, char *ptr, int len, void* info)
{    
    ithCodecUartDbgWrite(ptr, len);    
    return len;
}

static int SwUartCodecDbgPutchar(int c)
{    
    static unsigned char pBuffer[64] = { 0 };
    static int curIndex = 0;
    pBuffer[curIndex++] = (unsigned char) c;
    //if (curIndex >= 64 || ((unsigned char)c) == '\n')
    if (curIndex >= 64)
    {
        SwUartCodecDbgWrite(NULL, pBuffer, curIndex, NULL);
        curIndex = 0;
    }
    return c;
}

static int SwUartCodecDbgIoctl(int file, unsigned long request, void* ptr, void* info)
{
    ITPSwUartCodecDbg* ctxt = (ITPSwUartCodecDbg*)info;

    switch (request)
    {
    case ITP_IOCTL_INIT:                            
        if (ithCodecCommand(CMD_SWUART_DBG_CODEC_INIT, ctxt->baudrate, ctxt->gpio, NULL) == false)
            printf("risc codec is not running");
        ithPutcharFunc = SwUartCodecDbgPutchar;      
        break;
    case ITP_IOCTL_SET_GPIO_PIN:
        ctxt->gpio = *(int*)ptr;
        break;
        
    case ITP_IOCTL_SET_BAUDRATE:
        ctxt->baudrate = *(int*)ptr;
        break;

    case ITP_IOCTL_SET_PARITY:    
    	if(ithCodecCommand(CMD_SWUART_DBG_CODEC_SETPARITY, *(int*)ptr, NULL, NULL) == false)
    		printf("risc codec is not running");
    	break;    

    default:
        errno = (ITP_DEVICE_SWUARTDBG << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceSwUartCodecDbg =
{
    ":swuartcodecdbg",
    SwUartCodecDbgOpen,
    itpCloseDefault,
    itpReadDefault,
    SwUartCodecDbgWrite,
    itpLseekDefault,
    SwUartCodecDbgIoctl,
    (void*)&itpSwUart
};
