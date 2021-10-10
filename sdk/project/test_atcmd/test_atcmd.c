#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "bt/bluetooth.h"

void* TestFunc(void* arg)
{
	int i;
    char getstr[256];
    char sendtr[256];
	int len = 0;
	int fd;
	const char atc0[] = "AT+NAME\r\n";  
	printf("Start uart test!\n");

    itpRegisterDevice(ITP_DEVICE_UART0, &itpDeviceUart0);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_INIT, NULL);
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_RESET, CFG_UART0_BAUDRATE);
	//usleep(5000000);
	fd = bt_dev_open(ITP_DEVICE_UART0);
	if (fd)
		printf("open device success\n");
	else
		printf("open device fail\n");
    while(1)
    {
       printf("send at command\n");
	   
       bt_dev_write(fd, atc0, (size_t)strlen(atc0));	
	   memset(getstr, 0, 256);
	   len = bt_dev_read(fd, getstr, 256);
	   if(len > 0)
	   {
		   printf("uart read: %s\n",getstr);		   
	   }
	   usleep(5000000);
	}
}