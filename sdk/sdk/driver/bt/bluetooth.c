#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"

typedef struct _BT_DEV
{
	ITPDeviceType type;
	const char* name;
}BT_DEV;

BT_DEV dev[] = 
{
	{ITP_DEVICE_UART0, ":uart0"},
	{ITP_DEVICE_UART1, ":uart1"},
	{ITP_DEVICE_UART2, ":uart2"},
	{ITP_DEVICE_UART3, ":uart3"},
	{ITP_DEVICE_MAX,   ""},
};

int bt_dev_open(ITPDeviceType devtype)
{
	int fd = -1;
	int i;
	
	for(i=0; i < sizeof(dev)/sizeof(dev[0]); i++)
	{
		if (dev[i].type == devtype)
			break;		
	}
	
	fd = open(dev[i].name, O_RDONLY);
	return fd;
}

void bt_dev_close(int fd)
{
	close(fd);
}

int bt_dev_write(int fd, char* ptr, size_t len)
{
	return write(fd, ptr, len);	
}

int bt_dev_read(int fd, char* ptr, size_t len)
{
	return read(fd, ptr, len);
}
