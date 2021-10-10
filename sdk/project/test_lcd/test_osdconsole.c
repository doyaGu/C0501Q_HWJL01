#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"

void* TestFunc(void* arg)
{
	int i;

    itpInit();
    
#ifndef CFG_DBG_OSDCONSOLE
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);

    itpRegisterDevice(ITP_DEVICE_STD, &itpDeviceOsdConsole);
    itpRegisterDevice(ITP_DEVICE_OSDCONSOLE, &itpDeviceOsdConsole);
    ioctl(ITP_DEVICE_OSDCONSOLE, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_OSDCONSOLE, ITP_IOCTL_CLEAR, NULL);
#endif // !CFG_DBG_OSDCONSOLE
    
    for (i = 0;; i++)
    {
        printf("i = %d\n", i);
        sleep(1);
    }
    return NULL;
}
