/*
 * Copyright (c) 2014 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Wiegand functions.
 *
 * @author Vincent Lee
 * @version 1.0
 */

#include <errno.h>
#include "itp_cfg.h"
#include "ite/audio.h"
#include "ite/ite_risc.h"
#include "wiegand/wiegand.h"

enum {
    CMD_WIEGAND_ENABLE = 11 
};

typedef struct
{
    int index;
    int bit_count;
    int gpio_d0;
    int gpio_d1;
    char card_id[17];
} ITPWiegand;

#if(CFG_CHIP_FAMILY == 9850)
static ITPWiegand itpWiegand0 = {0, 34, 25, 26};
static ITPWiegand itpWiegand1 = {1, 34, 23, 24};

static int WiegandRead(int file, char *ptr, int len, void* info)
{
    ITPWiegand* ctxt = (ITPWiegand*)info;
    ITHUartPort port = ctxt->index ? ITH_UART3: ITH_UART2;
	unsigned long long value = 0;
	char cardid[17]= {0};
	char tmp[17] = {0};
    int  bit_count = 0;
	int  idlen = 0;
    int count = 0;
	int i = 0;
	#define TIMEOUT 1000
	
    for (;;)
    {
        if (ithUartIsRxReady(port))
        {
            int timeout = TIMEOUT;			
            cardid[count] = ithUartGetChar(port); // Read character from uart
            count++;
			
            while (1)
            {
                // Is a character waiting?
                if (ithUartIsRxReady(port))
                {
                    cardid[count] = ithUartGetChar(port); // Read character from uart
                    count++;
                    timeout = TIMEOUT;
                }
                else if (timeout-- <= 0)
                {
                	break;
                    //return count;
                }
            }
        }
        else
        {
        	break;
        	//return count;            
        }
    }
	idlen = cardid[1];
	if (idlen)
	{
		if (idlen > 17)
		{
			printf("invalid id len\n");
			return -1;
		}
	
		for (i = 0; i < idlen; i++) 	
			tmp[i] = cardid[1+idlen - i]; 				
		memcpy(&value, tmp, idlen);
		sprintf(ctxt->card_id, "%08X%03d%05d", (unsigned long)(value >> 32), ((unsigned long)value&0xFFFF0000)>>16, (unsigned long)value&0xFFFF);     
    	printf("card_id: %s, bit_count: %d\n", ctxt->card_id, bit_count);
    	*(int*)ptr = (int)ctxt->card_id;
	}
	return 0;
}

static int WiegandIoctl(int file, unsigned long request, void* ptr, void* info)
{
    ITPWiegand* ctxt = (ITPWiegand*)info;

    switch (request)
    {
    case ITP_IOCTL_INIT:		
      	init_wiegand_controller(ctxt->index);				
        break;

    case ITP_IOCTL_ENABLE:		
        wiegand_controller_enable(ctxt->index, ctxt->gpio_d0, ctxt->gpio_d1);
        break;

    case ITP_IOCTL_SET_BIT_COUNT:
        ctxt->bit_count = *(int*)ptr;		
        break;

    case ITP_IOCTL_GET_BIT_COUNT:
        *(int*)ptr = ctxt->bit_count;
        break;

    case ITP_IOCTL_SET_GPIO_PIN:
        ctxt->gpio_d0 = *(int*)ptr;
        ctxt->gpio_d1 = *(int*)(ptr+sizeof(int));        
        break;
	case ITP_IOCTL_SUSPEND:		
		wiegand_suspend(ctxt->index);
		break;
	case ITP_IOCTL_RESUME:
		wiegand_resume(ctxt->index);	
		break;
    default:
        errno = (ctxt->index ? ITP_DEVICE_WIEGAND1 : ITP_DEVICE_WIEGAND0 << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceWiegand0 =
{
    ":wiegand0",
    itpOpenDefault,
    itpCloseDefault,
    WiegandRead,
    itpWriteDefault,
    itpLseekDefault,
    WiegandIoctl,
    (void*)&itpWiegand0
};

const ITPDevice itpDeviceWiegand1 =
{
    ":wiegand1",
    itpOpenDefault,
    itpCloseDefault,
    WiegandRead,
    itpWriteDefault,
    itpLseekDefault,
    WiegandIoctl,
    (void*)&itpWiegand1
};
#else
static ITPWiegand itpWiegand0 = {0, 34, 25, 26};
static ITPWiegand itpWiegand1 = {1, 34, 23, 24};

static int WiegandRead(int file, char *ptr, int len, void* info)
{
    ITPWiegand* ctxt = (ITPWiegand*)info;
    unsigned long long value = 0;    
    int  bit_count = 0;

    *(int*)ptr = NULL;
    bit_count = ithCodecWiegandReadCard(ctxt->index, &value);
    if ((bit_count == ctxt->bit_count) && value)
    {
        switch (ctxt->bit_count)
        {
            case 26:
                value = (value & 0x1FFFFFE) >> 1;
                break;
        
            case 34:            
                value = (value & 0x1FFFFFFFE) >> 1;
                break;
        
            case 37:
                value = (value & 0xFFFFFFFFE) >> 1;
                break;
        
            default:
                break;
        }
        
        sprintf(ctxt->card_id, "%08X%03d%05d", (unsigned long)(value >> 32), ((unsigned long)value&0xFFFF0000)>>16, (unsigned long)value&0xFFFF);     
        printf("card_id: %s, bit_count: %d\n", ctxt->card_id, bit_count);
        *(int*)ptr = (int)ctxt->card_id;
	}
   
    return 0;
}

static int WiegandIoctl(int file, unsigned long request, void* ptr, void* info)
{
    ITPWiegand* ctxt = (ITPWiegand*)info;

    switch (request)
    {
    case ITP_IOCTL_INIT:
        // risc3 combine with risc2 (for audio codec) to fire        
        iteRiscOpenEngine(ITE_SW_PERIPHERAL_ENGINE, 0);
        break;

    case ITP_IOCTL_ENABLE:    	
        if (ithCodecCommand(CMD_WIEGAND_ENABLE, ctxt->index, ctxt->gpio_d0, ctxt->gpio_d1) == false)
            printf("WiegandIoctl risc codec is not running");
        break;

    case ITP_IOCTL_SET_BIT_COUNT:
        ctxt->bit_count = *(int*)ptr;
        break;

    case ITP_IOCTL_GET_BIT_COUNT:
        *(int*)ptr = ctxt->bit_count;
        break;

    case ITP_IOCTL_SET_GPIO_PIN:
        ctxt->gpio_d0 = *(int*)ptr;
        ctxt->gpio_d1 = *(int*)(ptr+sizeof(int));        
        break;

    default:
        errno = (ctxt->index ? ITP_DEVICE_WIEGAND1 : ITP_DEVICE_WIEGAND0 << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceWiegand0 =
{
    ":wiegand0",
    itpOpenDefault,
    itpCloseDefault,
    WiegandRead,
    itpWriteDefault,
    itpLseekDefault,
    WiegandIoctl,
    (void*)&itpWiegand0
};

const ITPDevice itpDeviceWiegand1 =
{
    ":wiegand1",
    itpOpenDefault,
    itpCloseDefault,
    WiegandRead,
    itpWriteDefault,
    itpLseekDefault,
    WiegandIoctl,
    (void*)&itpWiegand1
};
#endif
