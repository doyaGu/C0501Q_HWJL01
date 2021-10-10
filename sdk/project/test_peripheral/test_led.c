#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

void* TestFunc(void* arg)
{
    int fd = open(":led:0", O_RDONLY);
    for (;;)
    {
    	printf("LED ON\n");
        ioctl(fd, ITP_IOCTL_ON, NULL);
        sleep(1);
        
        printf("LED OFF\n");
        ioctl(fd, ITP_IOCTL_OFF, NULL);
        sleep(1);

		printf("LED FLICKER\n");
        ioctl(fd, ITP_IOCTL_FLICKER, (void*)33);
        sleep(1);
    }
    return NULL;
}
