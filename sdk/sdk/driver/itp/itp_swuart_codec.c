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

enum {
    CMD_SWUARTTX_CODEC_INIT  = 41,
    CMD_SWUARTRX_CODEC_INIT  = 42,
    CMD_SWUART_READ_CODEC    = 43,
    CMD_SWUART_WRITE_CODEC   = 44,
    CMD_SWUART_SETPARITY     = 45,
};

typedef struct
{
    int gpio;
    int baudrate;    
} ITPSwUartCodec;

static ITPSwUartCodec itpSwUart = {50, 115200};

static int SwUartCodecOpen(const char* name, int flags, int mode, void* info)
{
	return 0;
}
                              
static int SwUartCodecWrite(int file, char *ptr, int len, void* info)
{	
    ithCodecUartWrite(ptr, len);
    return len;
}

static int SwUartCodecRead(int file, char *ptr, int len, void* info)
{	
    return ithCodecUartRead(ptr, len);  ;
}
static int SwUartCodecIoctl(int file, unsigned long request, void* ptr, void* info)
{
    ITPSwUartCodec* ctxt = (ITPSwUartCodec*)info;

    switch (request)
    {
    case ITP_IOCTL_INIT:        
        if (ithCodecCommand(CMD_SWUARTTX_CODEC_INIT, ctxt->baudrate, ctxt->gpio, NULL) == false)
        	printf("risc codec is not running");
        break;
    case ITP_IOCTL_SWUARTRX_INIT:    	
    	if (ithCodecCommand(CMD_SWUARTRX_CODEC_INIT, ctxt->baudrate, ctxt->gpio, NULL) == false)
        	printf("risc codec is not running");
  		break;
    case ITP_IOCTL_SET_GPIO_PIN:    	
        ctxt->gpio = *(int*)ptr;        
        break;
        
    case ITP_IOCTL_SET_BAUDRATE:    	
        ctxt->baudrate = *(int*)ptr;        
        break;
	case ITP_IOCTL_SET_PARITY:
		if(ithCodecCommand(CMD_SWUART_SETPARITY, *(int*)ptr, NULL, NULL) == false)
    		printf("risc codec is not running");
		break;
		
    default:
        errno = (ITP_DEVICE_SWUART << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceSwUartCodec =
{
    ":swuartcodec",
    SwUartCodecOpen,
    itpCloseDefault,
    SwUartCodecRead,
    SwUartCodecWrite,
    itpLseekDefault,
    SwUartCodecIoctl,
    (void*)&itpSwUart
};
