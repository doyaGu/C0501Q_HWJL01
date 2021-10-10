#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

/*#define MACRO_RETURN_ERR(x)	{\
								printf("err %s\n", x);\
								return -1;\
							}*/

void* TestFunc(void* arg)
{
    int ir_handle = 0;
    int flags = O_RDWR;
    //int rev = -1;
    //uint32_t pCode = 0;
    int cnt=0;

    //ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);

    // Register device Remote control
    itpRegisterDevice(ITP_DEVICE_IR, &itpDeviceIr);

    // Do initialization
    ioctl(ITP_DEVICE_IR, ITP_IOCTL_INIT, (void *) 0);

    // Open device
    ir_handle = open(":ir", 0, flags);

    for (;;)
    {
        ITPKeypadEvent ev;
        //uint16_t	TxBuf[]={0x7c, 0x15, 0x15, 0x0a, 0x0a, 0x0b, 0x0b, 0x0a, 
					   //     0x0b, 0x0a, 0x0a, 0x15, 0x15, 0x14, 0x15, 0x15, 
					   //     0x15, 0x14, 0x0b, 0x0a, 0x0a, 0x15, 0x0a, 0x0b,
					   //     0x0a, 0x0b, 0x14, 0x15, 0x15, 0x0a, 0x15, 0x14,
					   //     0x15, 0x174,0x68};

        //if( (cnt++&0x0F)==0 )	printf("IR test...count=%d:\n",cnt);

        //if( (cnt&0xFF)==0x00 )
        //{
        //    if(cnt++&0x01)	ev.code = 0x55;
        //    else			ev.code = 0xAA;

        //    printf(" IR-Tx ev->code=%x\n",ev.code);
        //    write(ITP_DEVICE_IR, &ev, sizeof (ITPKeypadEvent) );
        //    ev.code = 0;				
        //}

        if (read(ITP_DEVICE_IR, &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
            printf("key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,flags=0x%X\r\n", 
            ev.time.tv_sec,
            ev.time.tv_usec / 1000,
            ev.code,
            (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
            (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
            (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
            ev.flags);

        usleep(33000);
    }

    //while(1)
    //{
    //    uint32_t code;
    //    if (read(ITP_DEVICE_IR, &code, sizeof(uint32_t)) == sizeof(uint32_t))
    //        ithPrintf("[Test IR] key: code=0x%X\n", code);

    //    usleep(33000);
    //}

    return NULL;
}

//TODO: how to deal with interrupt occur? where to put code or how to be invoked?
//Answer, same as keypad
