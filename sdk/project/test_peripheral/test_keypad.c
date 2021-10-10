#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

void* TestFunc(void* arg)
{
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);

    #ifdef CFG_I2C0_ENABLE
    printf("wait 5s for removing the SPI cable~~\n");
	usleep(5*1000*1000);
	#endif
	
    for (;;)
    {
        ITPKeypadEvent ev;
    
        ioctl(ITP_DEVICE_KEYPAD, ITP_IOCTL_PROBE, NULL);
        if (read(ITP_DEVICE_KEYPAD, &ev, sizeof (ITPKeypadEvent)) == sizeof (ITPKeypadEvent))
        {
            printf("key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,flags=0x%X\r\n", 
                ev.time.tv_sec,
                ev.time.tv_usec / 1000,
                ev.code,
                (ev.flags & ITP_KEYPAD_DOWN) ? 1 : 0,
                (ev.flags & ITP_KEYPAD_UP) ? 1 : 0,
                (ev.flags & ITP_KEYPAD_REPEAT) ? 1 : 0,
                ev.flags);
        }

        usleep(33000);
    }
    return NULL;
}
