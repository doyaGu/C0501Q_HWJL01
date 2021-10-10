/*
Copyright (c) 2008-8 by zcshev in 910 room of 211 building
All rights reserved.
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sntp.h"
#include "NtpTime.h"

#define ERROR_SOCKET          0   //初始化套接字错误  
#define ERROR_RCV_PACKET      1   //接收客户端数据包错误
#define ERROR_SEND_PACKET     2   //发送数据包错误
#define ERROR_WINSOCK_STACK   3   //载入套接字堆栈错误
#define ERROR_WINSOCK_VERSION 4   //套接字版本错误
#define ERROR_BIND_SERVER     5   //绑定服务器端口错误
#define ERROR_REPLY           6   //调用回复客户端程序错误


static int   m_hNtpServerSocket;
static struct sockaddr_in fsin;	 
static NtpBasicInfo       nbi;
static NtpFullPacket      nfp;
static const int nReceivefromSize=sizeof(nbi);
static int	alen=sizeof(fsin);
static const int nSendSize = sizeof(NtpFullPacket);
static int quit;
/************************************************************************/
/*  函数原型：reply() 
    参数类型： 
    返回值：  返回值在定义中已说明
    基本原理：收包，填包，发包                                          */
/************************************************************************/
static int reply(void)    
{   
	ulong dwError;
  	int   nSendtoSize;
  
	struct tm   st1;
	struct tm   st2;

	char szBuffer[128];
 
	int rc=0;

	printf("........................waiting packets.........................\n");
    
	rc=recvfrom(m_hNtpServerSocket, szBuffer, 128, 0,(struct sockaddr *)&fsin, &alen);

    if (quit)
        return 0;

	printf("from ip = %s --port = %d NTP Request Packets in this time\n",inet_ntoa(fsin.sin_addr), fsin.sin_port);
	//fsin.sin_addr.S_un
	memcpy(&nbi,szBuffer,sizeof(nbi));
   	if (rc== -1) 
	{
		// printf("%s",inet_ntoa(*(struct in_addr*)(fsin.sin_addr));
	 
		printf(("Could not receive from the SNTP client , get_last_error returns: %d\n"),  get_last_error());
 
		//dwError = get_last_error();   

		//set_last_error(dwError);

		return ERROR_RCV_PACKET;
   }
   else
   {
	   	printf("packet was successfully received\n"); 

		memset(&nfp, 0, nSendSize);
 
		nfp.m_Basic.m_LiVnMode = (nbi.m_LiVnMode==35)? 36 : 28; 
		nfp.m_Basic.m_Stratum = 0x01;
		nfp.m_Basic.m_OriginateTimestamp=nbi.m_TransmitTimestamp;  //发包过来的已经是网络字节序
		nfp.m_Basic.m_ReferenceTimestamp=host2network(MyGetCurrentTime()); 
		nfp.m_Basic.m_ReceiveTimestamp=nfp.m_Basic.m_ReferenceTimestamp;
        nfp.m_Basic.m_TransmitTimestamp=host2network(MyGetCurrentTime()); 		  
   }
    
	nSendtoSize = sizeof(NtpFullPacket); 

   if (sendto(m_hNtpServerSocket, (char *) &nfp, nSendtoSize, 0,(struct sockaddr *)&fsin, alen)== -1 ) 
	{
		printf(("Failed in  send NTP reply to the SNTP client, get_last_error returns %d\n"), get_last_error());
 
		dwError = get_last_error();

		set_last_error(dwError);

		return ERROR_SEND_PACKET;
	}
   //////////////////////////////////////////////////////////////////////////
   
        st1 = NtpTimePacket2SystemTime(network2host(nfp.m_Basic.m_OriginateTimestamp));
		st2 = NtpTimePacket2SystemTime(network2host(nfp.m_Basic.m_TransmitTimestamp));
 	    
		printf("Time was successfully send to NTP client\n"); 
		printf(("                            DD/MM/YYYY  HH:MM:SS.MS\n"));
		printf(("Client Originate Date was   %02d/%02d/%04d, %02d:%02d:%02d\n"), 
				  st1.tm_mday, st1.tm_mon + 1, st1.tm_year + 1900, st1.tm_hour, st1.tm_min, st1.tm_sec);
		printf(("Server Transmit Date was     %02d/%02d/%04d, %02d:%02d:%02d\n"),
				  st2.tm_mday, st2.tm_mon + 1, st2.tm_year + 1900, st2.tm_hour, st2.tm_min, st2.tm_sec);
		printf(("   T2=%f %u,%u\n   T3=%f %u,%u\n"),
			NtpTimePacket2Double(network2host(nfp.m_Basic.m_ReceiveTimestamp)),
			network2host(nfp.m_Basic.m_ReceiveTimestamp).m_dwInteger,
			network2host(nfp.m_Basic.m_ReceiveTimestamp).m_dwFractional, 
			NtpTimePacket2Double(network2host(nfp.m_Basic.m_TransmitTimestamp)),
			network2host(nfp.m_Basic.m_TransmitTimestamp).m_dwInteger,
			network2host(nfp.m_Basic.m_TransmitTimestamp).m_dwFractional);
 
        return 0;
}

void sntp_server_init(int port)
{ 
	char	 *service = "sntp";	 
	ulong   dwError; 

	printf("*********************** Network Time Protocol************************\n");
	printf("*********************** ITE Tech.(ShenZhen)inc.**********************\n");
	printf("**************************author:SladeZhag***************************\n");
	printf("***********************Date:2014-07-11 02:00:00**********************\n\n\n");
	
    memset(&nbi, 0, nReceivefromSize);
    // m_hNtpServerSocket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); 
    m_hNtpServerSocket = socket(AF_INET, SOCK_DGRAM, 0); 
    if (m_hNtpServerSocket == -1)
    {
        printf("Failed to create server socket, get_last_error returns: %d\n", get_last_error());
        goto END;
    }  

	memset(&fsin, 0, sizeof(fsin));
	fsin.sin_family = AF_INET;
	fsin.sin_addr.s_addr = htonl(INADDR_ANY);
	fsin.sin_port = htons(port);
 	
	if (bind(m_hNtpServerSocket, (struct sockaddr *)&fsin, sizeof(fsin)) == -1)
	{ 
		printf("can't bind to %s port: %d\n, get_last_error returns: %d\n", service,port, get_last_error());
		dwError = get_last_error();
 
	   set_last_error(dwError);
	   goto END;
	}

    quit = 0;

 END:
    return;
}

void sntp_server_exit(void)
{
    quit = 1;
}

 void* sntp_server_thread(void* arg)
 {
	while (!quit) 
	{   
 		reply();
	}
    return NULL;
 }
