/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Net console functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "itp_cfg.h"

extern char* __printbuf_addr;
extern int __printbuf_size;
extern int __printbuf_ptr;
extern int itpPrintBufPutchar(int c);
extern int itpPrintBufWrite(int file, char *ptr, int len, void* info);

static int ncPtr = 0;
static int ncSocket = -1;
struct sockaddr_in ncSockAddr;

#ifdef CFG_DBG_CLI
static int ncRecvSock = -1;
#endif

static void NetConsoleWriteHandler(void)
{
#ifdef CFG_NET_ETHERNET
    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
#elif defined(CFG_NET_WIFI)
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL))
#endif // CFG_NET_ETHERNET
    {
        fd_set fds;
        struct timeval tv;
        int bufptr = __printbuf_ptr;
            
        if (ncPtr > bufptr)
        {
            FD_ZERO(&fds);
            FD_SET(ncSocket, &fds); 
            tv.tv_sec = tv.tv_usec = 0;
            
            select(ncSocket + 1, NULL, &fds, NULL, &tv);

            if (FD_ISSET(ncSocket, &fds))
            {
                int count = sendto(ncSocket, __printbuf_addr + ncPtr, __printbuf_size - ncPtr, 0, (const struct sockaddr*)&ncSockAddr, sizeof (ncSockAddr));
                if (count > 0)
                {
                    ncPtr += count;
                    if (ncPtr >= __printbuf_size)
                        ncPtr = 0;
                }
            }
            else
                return;
        }

        if (ncPtr < bufptr)
        {
            FD_ZERO(&fds);
            FD_SET(ncSocket, &fds); 
            tv.tv_sec = tv.tv_usec = 0;
            
            select(ncSocket + 1, NULL, &fds, NULL, &tv);
        
            if (FD_ISSET(ncSocket, &fds))
            {
                int count = sendto(ncSocket, __printbuf_addr + ncPtr, bufptr - ncPtr, 0, (const struct sockaddr*)&ncSockAddr, sizeof (ncSockAddr));
                if (count > 0)
                {
                    ncPtr += count;
                    if (ncPtr >= __printbuf_size)
                        ncPtr = 0;
                }
            }
        }
    }
}

void NetConsoleInit(void)
{
    ncSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ncSocket == -1)
        return;

	//int val = 1;
	//ioctlsocket(ncSocket, FIONBIO, &val);

    memset((char*)&ncSockAddr, 0, sizeof(ncSockAddr));
    ncSockAddr.sin_family = AF_INET;
    ncSockAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    ncSockAddr.sin_port = htons(CFG_DBG_NETCONSOLE_PORT);

	ithPutcharFunc = itpPrintBufPutchar;
	itpRegisterIdleHandler(NetConsoleWriteHandler);

#ifdef CFG_DBG_CLI
    {
        struct sockaddr_in si;
    
        ncRecvSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (ncRecvSock == -1)
            return;
    
        memset((char*)&si, 0, sizeof(si));
        si.sin_family = AF_INET;
        si.sin_addr.s_addr = htonl(INADDR_ANY);
        si.sin_port = htons(CFG_DBG_NETCLI_PORT);
        if (bind(ncRecvSock, (struct sockaddr*)&si, sizeof(si)) == -1)
            return;
    }
#endif // CFG_DBG_CLI
}

static int NetConsoleRead(int file, char *ptr, int len, void* info)
{
#ifdef CFG_DBG_CLI
    for (;;)
    {
    #ifdef CFG_NET_ETHERNET
        if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    #elif defined(CFG_NET_WIFI)
        if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_AVAIL, NULL))
    #endif // CFG_NET_ETHERNET
        {
            break;
        }
        usleep(1000);
    }

    for (;;)
    {
        int ret = recvfrom(ncRecvSock, ptr, len, 0, NULL, 0);
        if (ret > 0)
            return ret;
    }
#endif // CFG_DBG_CLI

    return -1;
}

static int NetConsoleIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
	case ITP_IOCTL_INIT:
        NetConsoleInit();
        break;

    default:
        errno = (ITP_DEVICE_NETCONSOLE << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceNetConsole =
{
    ":netconsole",
    itpOpenDefault,
    itpCloseDefault,
    NetConsoleRead,
    itpPrintBufWrite,
    itpLseekDefault,
    NetConsoleIoctl,
    NULL
};
