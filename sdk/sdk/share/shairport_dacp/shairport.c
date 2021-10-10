/*
 * ShairPort Initializer - Network Initializer/Mgr for Hairtunes RAOP
 * Copyright (c) M. Andrew Webster 2011
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "md5utils.h"
#include "socketlib.h"
#include "shairport.h"
#include "hairtunes.h"
#include "ite/itp.h"
#include "iniparser/iniparser.h"

#undef AF_INET6


#define SHAIRPORT_INIFILENAME_MAXLEN              56
#define SHAIRPORT_NAME_MAXLEN                     56
#define SHAIRPORT_INIKEY_NAME                     "shairport:friendlyName"

#define SHAIRPORT_INIKEY_PASSWORD           "shairport:password"


#define MDNS_RECORD  "tp=UDP", "sm=false", "ek=1", "et=0,1", "cn=0,1", "ch=2", \
    "ss=16", "sr=44100", "vn=3", "txtvers=1", \
    "pw=false"

#define MDNS_RECORDS  "tp=UDP sm=false ek=1 et=0,1 cn=0,1 ch=2 ss=16 sr=44100 vn=3 txtvers=1 pw=false"

#define SHAIRPORT_ACTIVE_REMOTE                 "Active-Remote"
#define SHAIRPORT_DACP_ID                 "DACP-ID"

#define SHAIRPORT_STR_HOST                "iTE-teki-iPad.local."

// keep DACP ActiveRemote
static char pActiveRemote[10];
//keep DACP ID
static char pDacpId[26];
// keep controller dacp port
static int gnDacpPort;
// keep controller host name
static char gtHost[SERVLEN];
static struct sockaddr gDACPAddr;

static char pDacpIp[26];
static double gVolume;
#ifndef TRUE
#define TRUE (-1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

// TEMP

int kCurrentLogLevel = LOG_INFO;
int bufferStartFill = -1;

#ifdef _WIN32
#define DEVNULL "nul"
#else
#define DEVNULL "/dev/null"
#endif
#define RSA_LOG_LEVEL LOG_DEBUG_VV
#define SOCKET_LOG_LEVEL LOG_DEBUG_VV
#define HEADER_LOG_LEVEL LOG_DEBUG
#define AVAHI_LOG_LEVEL LOG_DEBUG

#define ACCEPT_CLIENT_STACK_SIZE                  (255 * 1024)
static sem_t                                semAcceptClinet;
static int gnWaitClient = 0;
static char giniName[SHAIRPORT_INIFILENAME_MAXLEN];

static char gtName[100 + HWID_SIZE + 3];

static  struct timeval gStartT, gEndT;
static  struct timeval gStartSwitchT, gEndSwitchT;
static  struct timeval gBAirplay, gEAirplay;

static int gInit = 0;
SHAIRPORT_PARAM gShairport;

// http digest auth enable
static int gnDigest =0;

// digest method, 1:option 2:announce
static int gnMethod = 1;

// check gnDigest 
static int gnPasswordEnable = 0;

static char* gptPassword;
typedef enum tagAUDIOLINK_AIRPLAY_COMMAND_E
{
    AUDIOLINK_AIRPLAY_UNKNOWN = 0,
    AUDIOLINK_AIRPLAY_PLAY_PAUSE, //  1
    AUDIOLINK_AIRPLAY_NEXT_ITEM,  //  2
    AUDIOLINK_AIRPLAY_PREV_ITEM,  //  3
    AUDIOLINK_AIRPLAY_VOLUME_UP,  //  4
    AUDIOLINK_AIRPLAY_VOLUME_DOWN,  // 5
    AUDIOLINK_AIRPLAY_BUSY_ON,  // 6
    AUDIOLINK_AIRPLAY_PREVENT_PLAYBACK_ON,  // 7
    AUDIOLINK_AIRPLAY_PREVENT_PLAYBACK_OFF,  //8
    AUDIOLINK_AIRPLAY_BUSY_OFF,  // 9    
} AUDIOLINK_AIRPLAY_COMMAND_E;

static void handleClient(int pSock, char *pPassword, char *pHWADDR);
static void writeDataToClient(int pSock, struct shairbuffer *pResponse);
static int readDataFromClient(int pSock, struct shairbuffer *pClientBuffer);

static int parseMessage(struct connection *pConn,unsigned char *pIpBin, unsigned int pIpBinLen, char *pHWADDR);
static void propogateCSeq(struct connection *pConn);

static void cleanupBuffers(struct connection *pConnection);
static void cleanup(struct connection *pConnection);

static int startAvahi(const char *pHwAddr, const char *pServerName, int pPort);
static char * DataToHexCStringEx( const void *inData, size_t inSize, char *outStr, const char * const inHexDigits );

static int getAvailChars(struct shairbuffer *pBuf);
static void addToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf);
static void addNToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf, int pNofNewBuf);

static char *getTrimmedMalloc(char *pChar, int pSize, int pEndStr, int pAddNL);
static char *getTrimmed(char *pChar, int pSize, int pEndStr, int pAddNL, char *pTrimDest);

static void slog(int pLevel, char *pFormat, ...);
static int isLogEnabledFor(int pLevel);

static void initConnection(struct connection *pConn, struct keyring *pKeys, struct comms *pComms, int pSocket, char *pPassword);
static void closePipe(int *pPipe);
static void initBuffer(struct shairbuffer *pBuf, int pNumChars);

static void setKeys(struct keyring *pKeys, char *pIV, char* pAESKey, char *pFmtp);
static RSA *loadKey(void);

//===========================================================================================================================
//	_AirTunesDACPClient_ResolveControllerCallBack
//===========================================================================================================================

static void	_AirTunesDACPClient_ResolveControllerCallBack(
		DNSServiceRef			inServiceRef,
		DNSServiceFlags			inFlags,
		uint32_t				inIFI,
		DNSServiceErrorType		inErrorCode,
		const char *			inFullName,
		const char *			inTargetName,
		uint16_t				inPort,
		uint16_t				inTXTLen,
		const unsigned char *	inTXTPtr,
		void *					inContext )
{		
	(void) inServiceRef;	// Unused
	(void) inFlags;			// Unused
	(void) inIFI;			// Unused
	(void) inFullName;		// Unused
	(void) inTXTLen;		// Unused
	(void) inTXTPtr;		// Unused

	printf("[shairport] _AirTunesDACPClient_ResolveControllerCallBack \n");
    
	if( inErrorCode == -65791 ) goto exit; // Ignore mStatus_ConfigChanged
	//require_noerr( inErrorCode, exit );
	gnDacpPort = inPort;
	//strlcpy( me->controllerDNSName, inTargetName, sizeof( me->controllerDNSName ) );
	//me->controllerTCPPort = ntohs( inPort );
	//me->gotController = true;
    sprintf(gtHost, "%s", inTargetName); 
	//shairportSendDACPCommand(0,inTargetName);
exit:
	return;
}

static void	_AirTunesDACPClient_QueryRecordCallBack(
    DNSServiceRef                       sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    DNSServiceErrorType                 errorCode,
    const char                          *fullname,
    uint16_t                            rrtype,
    uint16_t                            rrclass,
    uint16_t                            rdlen,
    const void                          *rdata,
    uint32_t                            ttl,
    void                                *context)
{		
	(void) sdRef;	// Unused
	(void) flags;			// Unused
	(void) interfaceIndex;			// Unused
//	(void) inFullName;		// Unused
	(void) rrclass;		// Unused
	(void) context;		// Unused
	
	if( errorCode == -65791 ) goto exit; // Ignore mStatus_ConfigChanged
	//require_noerr( inErrorCode, exit );
	//gnDacpPort = inPort;
	//strlcpy( me->controllerDNSName, inTargetName, sizeof( me->controllerDNSName ) );
	//me->controllerTCPPort = ntohs( inPort );
	//me->gotController = true;
    //sprintf(gtHost, "%s", inTargetName); 
    printf("DACP IP Address is %s \n",rdata);
	//shairportSendDACPCommand(0,inTargetName);
exit:
	return;
}

_AirTunesDACPClient_GetAddrInfoCallBack(
    DNSServiceRef                    sdRef,
    DNSServiceFlags                  flags,
    uint32_t                         interfaceIndex,
    DNSServiceErrorType              errorCode,
    const char                       *hostname,
    const struct sockaddr            *address,
    uint32_t                         ttl,
    void                             *context
                                        )
{
    printf("DACP IP Address is %s \n",hostname);
    memcpy(&gDACPAddr,address,sizeof(gDACPAddr));
    return 0;
}

//===========================================================================================================================
//	DataToHexCString
//===========================================================================================================================

static char * DataToHexCStringEx( const void *inData, size_t inSize, char *outStr, const char * const inHexDigits )
{
	const uint8_t *		src;
	const uint8_t *		end;
	char *				dst;
	uint8_t				b;
	
	src = (const uint8_t *) inData;
	end = src + inSize;
	dst = outStr;
	while( src < end )
	{
		b = *src++;
		*dst++ = inHexDigits[ b >> 4 ];
		*dst++ = inHexDigits[ b & 0xF ];
	}
	*dst = '\0';
	return( outStr );	
}

int shairportCheckPassword(char* pPassword,char* pURI,int nMethod)
{

    MD5_CTX md5Context;
    uint8_t md5[ 16 ];
    char    a1Str[ 32 + 1 ];
    char    a2Str[ 32 + 1 ];
    char *inUsernamePtr = "iTunes";
    size_t inUsernameLen = 6; 
    char *inPasswordPtr = "pass";
    size_t inPasswordLen = 4;

    char *inMethodPtr = "OPTIONS";
    size_t inMethodLen = 7;
    char *inURLPtr;    
    char *inURLPtrp ="*" ; //= "rtsp://192.168.111.2/3496302550";
    size_t inURLLen = 1; //= 31 ;
    char *inRealmPtr = "PC";
    size_t inRealmLen = 2;
    char *inNoncePtr = "573674E90C155258179CAA445F66B2AB";
    size_t inNonceLen= 32;
    char *inHexDigits;
    char *inHexDigitsl = "0123456789abcdef";
    char *inHexDigitsU = "0123456789ABCDEF";    
    char outDigestStr[ 32 + 1 ] = {0};
    char urlstr[40] = {0}; 
    char * pch= NULL;
    char * pch2= NULL;
    // Calculate Hex( MD5( A1 ) ).
    char *pExt = NULL;
    int nUpper = 0;
    int nTemp = 0;
    pExt = pPassword;
    if (pExt) {
        while (*pExt)
        {   
            if (isupper(*pExt)){
                printf("is upper %c \n",*pExt);
                nUpper = 1;
                break;
            }
            pExt++;

            if (nTemp>=33)
                break;

            nTemp++;
        }
    }
    if (nUpper){
        inHexDigits = inHexDigitsU;
    } else {
        inHexDigits = inHexDigitsl;
    }

    pch = strstr(pURI,"rtsp://");
    if (pch){
        pch2 = strchr(pURI,'"');
        if (pch2>pch){
            memcpy(urlstr,pch,pch2-pch);
            inURLLen = pch2-pch;
            inURLPtr =urlstr;
            printf("uri ,%d [%s] \n",inURLLen,urlstr);

        } else {
            return 0;
        }
    } else {
        inURLPtr = inURLPtrp;
        inURLLen = 1;
    }

    if (nMethod==1){
        // OPTION
    } else if (nMethod ==2) {
        //ANNOUNCE
        inMethodPtr = "ANNOUNCE";
        inMethodLen = 8;
  
}
    
    inRealmPtr = gShairport.pName;	
    inRealmLen = strlen(gShairport.pName);
    inPasswordPtr = gptPassword;
    inPasswordLen = strlen(gptPassword);
    MD5_Init( &md5Context );
    MD5_Update( &md5Context, inUsernamePtr, inUsernameLen );
    MD5_Update( &md5Context, ":", 1 );
    MD5_Update( &md5Context, inRealmPtr, inRealmLen );
    MD5_Update( &md5Context, ":", 1 );
    MD5_Update( &md5Context, inPasswordPtr, inPasswordLen );
    MD5_Final( md5, &md5Context );
    DataToHexCStringEx( md5, sizeof( md5 ), a1Str, inHexDigits );

    // Calculate Hex( MD5( A2 ) ).

    MD5_Init( &md5Context );
    MD5_Update( &md5Context, inMethodPtr, inMethodLen );
    MD5_Update( &md5Context, ":", 1 );
    MD5_Update( &md5Context, inURLPtr, inURLLen );
    MD5_Final( md5, &md5Context );
    DataToHexCStringEx( md5, sizeof( md5 ), a2Str, inHexDigits );

    // Calculate the final hash: MD5( Hex( MD5( A1 ) ):nonce:Hex( MD5( A2 ) ) ) and see if it matches.

    MD5_Init( &md5Context );
    MD5_Update( &md5Context, a1Str, strlen( a1Str ) );
    MD5_Update( &md5Context, ":", 1 );
    MD5_Update( &md5Context, inNoncePtr, inNonceLen );
    MD5_Update( &md5Context, ":", 1 );
    MD5_Update( &md5Context, a2Str, strlen( a2Str ) );
    MD5_Final( md5, &md5Context );
    DataToHexCStringEx( md5, sizeof( md5 ), outDigestStr, inHexDigits );
    printf("[Shairport] check password %s %s \n",outDigestStr,pPassword);

    malloc_stats();
    
    if(!strncmp(outDigestStr, pPassword, strlen(outDigestStr))){
        printf("[Shairport] password pass  \n");
        return 1;
    } else {
        printf("[Shairport] password fail  \n");    
        return 0;
    }
    
}

int shairportDACP_Connect(int nCmd)
{
    DNSServiceRef		resolverRef;
    char				name[256];
    int                 resolverSock;
    //uint64_t			timeout;
    int err;
    int nfds;

    fd_set readfds;
    struct timeval tv;
    int result;
    int stopNow =0;

    resolverRef = NULL;
    // Start a Bonjour resolve of the iTunes_Ctrl controller.
    if (strlen(pDacpId)){
        printf("[shairport] dacp_connect %s %d \n",pDacpId,strlen(pDacpId));
    } else {
        printf("[Shairport] dacp_connect id 0 return \n");
        return 0;
    }
    snprintf( name, sizeof( name ), "%s%s", kDACPBonjourServiceNamePrefix, pDacpId );
    err = DNSServiceResolve( &resolverRef, 0, kDNSServiceInterfaceIndexAny, name, kDACPBonjourServiceType,
    kDACPBonjourServiceDomain, _AirTunesDACPClient_ResolveControllerCallBack, NULL );	

    // Wait for the Bonjour response.
    printf("[shairport] connecting DNSServiceResolve %d \n",err);
    resolverSock = DNSServiceRefSockFD( resolverRef );
    nfds = resolverSock +1;
    while (!stopNow)
    {
        FD_ZERO(&readfds);
        FD_SET(resolverSock, &readfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;   
        result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
        if (result > 0) {
            err = kDNSServiceErr_NoError;

            if (FD_ISSET(resolverSock, &readfds))
                err = DNSServiceProcessResult(resolverRef);

            if (err)
                stopNow = 1;

            usleep(4000);
            shairportSendDACPCommand(nCmd);
            
            if( resolverRef )
                DNSServiceRefDeallocate( resolverRef );

            return 0;
        } else  {
            printf("select( ) returned %d errno %d %s\n",result, errno, strerror(errno));
            if (errno != EINTR) stopNow = 1;
            stopNow = 1;
        }
        //usleep(40000);
    }

    if( resolverRef )
        DNSServiceRefDeallocate( resolverRef );

    return 0;

}


int shairportDACPGetIP()
{
    DNSServiceRef		resolverRef;
    char				name[256];
    int                 resolverSock;
    //uint64_t			timeout;
    int err;
    int nfds;

    fd_set readfds;
    struct timeval tv;
    int result;
    int stopNow =0;
    resolverRef = NULL;
    // Start a Bonjour resolve of the iTunes_Ctrl controller.

    snprintf( name, sizeof( name ), "%s%s", kDACPBonjourServiceNamePrefix, pDacpId );
    /*err = DNSServiceQueryRecord(&resolverRef,0,0,gtHost,kDNSServiceType_A,kDNSServiceClass_IN,
    _AirTunesDACPClient_QueryRecordCallBack,NULL);*/
    err = DNSServiceGetAddrInfo(&resolverRef,0,0,kDNSServiceProtocol_IPv4,gtHost,_AirTunesDACPClient_GetAddrInfoCallBack,NULL);
    // Wait for the Bonjour response.

    resolverSock = DNSServiceRefSockFD( resolverRef );
    nfds = resolverSock +1;
    while (!stopNow)
    {
        FD_ZERO(&readfds);
        FD_SET(resolverSock, &readfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;   
        result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
        if (result > 0) {
            err = kDNSServiceErr_NoError;
            if (FD_ISSET(resolverSock, &readfds))
                err = DNSServiceProcessResult(resolverRef);
            if (err) 
               stopNow = 1;
            //usleep(40000);
        } else  {
            printf("select( ) returned %d errno %d %s\n",result, errno, strerror(errno));
            if (errno != EINTR)
                stopNow = 1;
            
            stopNow = 1;
        }
        //usleep(40000);
    }

    if( resolverRef )
        DNSServiceRefDeallocate( resolverRef );
    
}

int shairportSendDACPCommand(int nCommand)
{
    char tService[SERVLEN];
    int tFamily = AF_INET;
    int tSocketDescriptor;
    char tAddr[INET6_ADDRSTRLEN];
    socklen_t tSize = INET6_ADDRSTRLEN;
    char tHost[SERVLEN];
    struct addrinfo *tAddrInfo;
    struct sockaddr_in *ipv4;
    struct sockaddr_in dest;    
    char buf[256];
    int nTemp;
    char pCommand[128];
    char ipstr[64];
    fd_set set;
    fd_set eset;
    int m_sd;
    int  port ;
    struct sockaddr_in addr;

    port = gnDacpPort;    
    sprintf(tService, "%d", ntohs(port)); // copies port to string
    sprintf(tHost, "%s", gtHost); 

#if 1
    if (1){
        m_sd = socket(AF_INET,SOCK_STREAM,0);
        nTemp = 0;
        ioctl(m_sd, FIONBIO, &nTemp);
        
        //ipv4 = (struct sockaddr_in *)&addr;
        //printf("ip = %s \n",inet_ntop(AF_INET, &ipv4->sin_addr, ipstr, sizeof(ipstr) ) );
        addr.sin_family = AF_INET;
        addr.sin_port = (port);
        addr.sin_addr.s_addr = inet_addr(pDacpIp);
        printf("[DACP] ip = %s \n",pDacpIp);
        nTemp = connect(m_sd,(struct sockaddr *)&addr,sizeof(addr));
    } else {
        shairportDACPGetIP();
        m_sd = socket(gDACPAddr.sa_family,SOCK_STREAM,0);
        nTemp = 0;
        ioctl(m_sd, FIONBIO, &nTemp);

        memcpy(&addr,&gDACPAddr,sizeof(addr));
        ipv4 = (struct sockaddr_in *)&addr;
        printf("ip = %s \n",inet_ntop(AF_INET, &ipv4->sin_addr, ipstr, sizeof(ipstr) ) );
        addr.sin_family = AF_INET;
        addr.sin_port = (port);
        nTemp = connect(m_sd,(struct sockaddr *)&addr,sizeof(addr));
    }

#else
    if(getAddrDACP(tHost, tService, tFamily, SOCK_STREAM, &tAddrInfo)) {
        printf("shairportSendDACPCommand error \n");
        return ERROR; // getAddr prints out error message
    }
//    ipv4 = (struct sockaddr_in *)tAddrInfo->ai_addr;
//    printf("ip = %s \n",inet_ntop(AF_INET, &ipv4->sin_addr, ipstr, sizeof(ipstr) ) );
    
    m_sd = socket(tAddrInfo->ai_family,SOCK_STREAM,0);
    nTemp = 0;
    ioctl(m_sd, FIONBIO, &nTemp);
    nTemp = connect(m_sd,tAddrInfo->ai_addr,tAddrInfo->ai_addrlen);
    freeaddrinfo(tAddrInfo);
#endif
    printf("connecting  %d \n",nTemp);
    if (!nTemp){
        memset(pCommand,0,128);
        switch (nCommand)
        {
            case AUDIOLINK_AIRPLAY_PLAY_PAUSE:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_PlayPause,pActiveRemote);
                break;
            case AUDIOLINK_AIRPLAY_NEXT_ITEM:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_NextItem,pActiveRemote);
                break;
            case AUDIOLINK_AIRPLAY_PREV_ITEM:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_PrevItem,pActiveRemote);
                break;
            case AUDIOLINK_AIRPLAY_VOLUME_UP:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_VolumeUp,pActiveRemote);
                break;
            case AUDIOLINK_AIRPLAY_VOLUME_DOWN:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_VolumeDown,pActiveRemote);
                break;
            case AUDIOLINK_AIRPLAY_BUSY_ON:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s%s=1 HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_SetProperty,kDACPProperty_DeviceBusy,pActiveRemote);
                break;
            case AUDIOLINK_AIRPLAY_PREVENT_PLAYBACK_ON:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s%s=1 HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_SetProperty,kDACPProperty_DevicePreventPlayback,pActiveRemote);
                break;
            case AUDIOLINK_AIRPLAY_PREVENT_PLAYBACK_OFF:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s%s=0 HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_SetProperty,kDACPProperty_DevicePreventPlayback,pActiveRemote);
                break;                
            case AUDIOLINK_AIRPLAY_BUSY_OFF:
                snprintf(pCommand,128,"GET /ctrl-int/1/%s%s=0 HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_SetProperty,kDACPProperty_DeviceBusy,pActiveRemote);
                break;
                
            default:
                printf("[Shariport]DACP unknow command %d  \n",nCommand);
                return 0;
                
                break;
        }
 //       snprintf(pCommand,128,"GET /ctrl-int/1/%s HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_PrevItem,pActiveRemote);
//        snprintf(pPause,128,"GET /ctrl-int/1/%s HTTP/1.1\r\nActive-Remote: %s\r\nHost: lwip.local.\r\n\r\n",kDACPCommandStr_Pause,pActiveRemote);
        printf("send \n%s",pCommand);	
	   
        FD_ZERO(&set);
        FD_ZERO(&eset);
        FD_SET(m_sd, &set);
        FD_SET(m_sd, &eset);

        nTemp = select(m_sd+1, NULL, &set, &eset, NULL);
        printf("selecting  %d \n",nTemp);
        
         // printf("----Beg Send Response Header----\n %d \n %s\n", nClient, pPause);
        if (nTemp > 0 && FD_ISSET(m_sd, &set))
        {
            nTemp= send(m_sd, pCommand, sizeof(pCommand),0);
            printf("sending  %d \n",nTemp);        

            FD_ZERO(&set);
            FD_ZERO(&eset);
            FD_SET(m_sd, &set);
            FD_SET(m_sd, &eset);

            nTemp = select(m_sd+1, &set, NULL, &eset, NULL);
            if (nTemp > 0 && FD_ISSET(m_sd, &set))
            {
                memset(buf, 0, sizeof(buf));
                nTemp = recv(m_sd, buf, sizeof(buf),0);
                printf("[DACP] recv %s \n",buf);
            }
        }
    }
    closesocket(m_sd);

    return 0;
}



static void MyDNSServiceResolveReply
    (
    )
{
    printf("MyDNSServiceResolveReply \n");
}


static int shairportStopOtherProtocol();

static int shairportStopOtherProtocol()
{
#ifdef CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY
    if (shairportGetALStatus() == gShairport.nUpnpPlaying || shairportGetALStatus() == 2  /*AL_PLAYSTATE_UPNP_PAUSE*/){
        printf("[Shairport] shairportStopOtherProtocol %d \n",shairportGetALStatus());
        return -1;
    } else {
        return 0;
    }
#else
    if (shairportGetALStatus() == gShairport.nUpnpPlaying || shairportGetALStatus() == 2  /*AL_PLAYSTATE_UPNP_PAUSE*/){
        slog(LOG_DEBUG_VV, "[Shairport]shairportStopOtherProtocol upnp playing , stop #line %d \n",__LINE__);
        hairtunesSetAudioOutputWakeUp(0);
        gShairport.stop_upnp();
        gShairport.set_playstate(gShairport.nAirPlayPlaying);
        // sleep 50 ms 
        usleep(50*1000);
        
        hairtunesSetAudioOutputWakeUp(1);
    }
	#ifdef CFG_AUDIOLINK_LOCAL_PLAYER_ENABLE
    if (shairportGetALStatus()==gShairport.nLocalPlaying){
        slog(LOG_DEBUG_VV, "[Shairport]shairportStopOtherProtocol local playing , stop #line %d \n",__LINE__);
        hairtunesSetAudioOutputWakeUp(0);          
        gShairport.stop_local();
        gShairport.set_playstate(gShairport.nAirPlayPlaying);
        // sleep 50 ms 
        usleep(50*1000);
        hairtunesSetAudioOutputWakeUp(1);
    }
	#endif
    return 0;
    
#endif
}

int shairportgetOtherProtocolPlaying()
{
    if (shairportGetALStatus() == gShairport.nUpnpPlaying)
        return 1;

#ifdef CFG_AUDIOLINK_LOCAL_PLAYER_ENABLE
    if (shairportGetALStatus() == gShairport.nLocalPlaying)
        return 1;
#endif

    return 0;

}

static void handle_sigchld(int signo) {
    int status;
    waitpid(-1, &status, WNOHANG);
}

int setShairportStatus(int nStatus)
{
    printf("[Shairport] setShairportStatus %d \n",nStatus);
    gShairport.nCurrState = nStatus;
    do{
        usleep(3000);
    }while(shairportGetALStatus()==gShairport.nStop);
}

int shairportGetALStatus()
{
    int nALStatus = 0;
#if defined(__OPENRTOS__)
    gShairport.get_playstate(&nALStatus);
#endif
    return nALStatus; 
}

int shairportSetException(int nException)
{
    int nResult = 0;
#if defined(__OPENRTOS__)
    printf("shairportSetException %d  \n",nException);
    gShairport.set_exception(nException);
#endif
    return nResult; 
}

int shairportGetException()
{
    int nException = 0;
#if defined(__OPENRTOS__)
    nException = gShairport.get_exception();
#endif
    return nException; 
}

int shairportChangeSpeed(int nSpeed)
{
    // change speed 
#ifdef CFG_AUDIOLINK_CHANGE_CPU_SPEED_ENABLE
    if (nSpeed == 400) {
        gettimeofday(&gBAirplay, NULL);        
        ithWriteRegH(0x004c, 0xE800);
    } else {
        ithWriteRegH(0x004c, 0xC000);
        gettimeofday(&gEAirplay, NULL);
        printf("Start time %d \n",itpTimevalDiff(&gBAirplay, &gEAirplay));        
    }
    ithClockInit();
    printf("cpu %d %d \n",ithGetCpuClock(),nSpeed);
#else
    printf("[Shairport]not support change cpu speed \n");
#endif

}
    
int setAudioLinkStatus(SHAIRPORT_PARAM *pShairport)
{
    gShairport.get_playstate = pShairport->get_playstate;
    gShairport.set_playstate = pShairport->set_playstate;
    gShairport.stop_upnp = pShairport->stop_upnp;
    gShairport.nAirPlayPlaying = pShairport->nAirPlayPlaying;
    gShairport.nUpnpPlaying = pShairport->nUpnpPlaying;    
    gShairport.nStop = pShairport->nStop;
    gShairport.nCurrState = SHAIRPORT_NONE;
    gShairport.get_exception = pShairport->get_exception;
    gShairport.set_exception = pShairport->set_exception;
    gShairport.nWifiMode = pShairport->nWifiMode;
#ifdef CFG_AUDIOLINK_LOCAL_PLAYER_ENABLE
    gShairport.nLocalPlaying = pShairport->nLocalPlaying;
    gShairport.stop_local = pShairport->stop_local;
#endif    
}

int setShairportIni(char* ptIni)
{
    strncpy(giniName,ptIni,SHAIRPORT_INIFILENAME_MAXLEN);
    return 0;
}

int getShairportPort()
{
    return gShairport.nPort;
}

void setShairportThreadID(pthread_t tshairport)
{
    gShairport.taskShairport = tshairport;
}

int setShairportDACP(int cmd)
{
    printf("[Shairport] key ,DACP command %d \n",cmd);
    shairportDACP_Connect(cmd);
}

pthread_t getShairportThreadID()
{
    return gShairport.taskShairport;
}

pthread_t getmDNSResponderThreadID()
{
    return gShairport.taskmDNSResponder;
}

pthread_t getAcceptClientThreadID()
{
    return gShairport.taskAcceptClient;
}

unsigned int getShairportsaddr()
{

    return gShairport.sinaddr;
    
}
int getShairportWifiMode()
{

    return gShairport.nWifiMode;
    
}

void closeShairport(int nClose)
{

    close(gShairport.nServerSock);
    close(gShairport.nClientSock);
    if (gShairport.ptConn!=NULL && nClose==2){
        cleanup(gShairport.ptConn);    
        //fflush(stdout);
    }    
    if(gShairport.pAddrInfo != NULL) {
        freeaddrinfo(gShairport.pAddrInfo);
    }
    gShairport.pAddrInfo = NULL;
    printf("[Shairport]closeShairport \n");
}

void closeAcceptClient()
{
    sem_destroy(&semAcceptClinet);
}

static void*
shairportAcceptClientThreadFunc(void* arg)
{
    int nResult;
    
    while (1)
    { 
        sem_wait(&semAcceptClinet);
        usleep(15*1000);        
        printf("[Shairport]shairportAcceptClientThreadFunc %d 0x%x \n",gShairport.nServerSock,gShairport.pAddrInfo);
        nResult = acceptClient(gShairport.nServerSock, gShairport.pAddrInfo);
        if (gShairport.nCurrState == SHAIRPORT_PLAY){
            shairportSetException(2);
            hairtunesSetAudioOutputWakeUp(1);           
            do{
                usleep(5*1000);
            }while(shairportGetException()==0);
            gettimeofday(&gStartT, NULL);
            //if (gShairport.nWifiMode == 1){
            //    printf("shairport, softAP mode \n");
            //} else {
                i2s_deinit_DAC();
            //}

            gettimeofday(&gEndT, NULL);
            if (itpTimevalDiff(&gStartT, &gEndT)<1300){
                slog(LOG_DEBUG, "[Shairport]shairportAcceptClientThreadFunc %d %d \n",itpTimevalDiff(&gStartT, &gEndT),shairportGetALStatus());
                if (gShairport.nWifiMode == 1){
                    //softAP mode
                    usleep(1200*1000);
                } else {                
                    usleep(1200*1000);
                }
            } else {
                slog(LOG_DEBUG, "[Shairport]shairportAcceptClientThreadFunc change time %d  \n",itpTimevalDiff(&gStartT, &gEndT));
            }

        }
        gShairport.nClientSock = nResult;        
        gnWaitClient = 1;
        printf("[Shairport]shairportAcceptClientThreadFunc acceptClient  %d  %d \n",gShairport.nClientSock,shairportGetALStatus());
        usleep(5*1000);
        if (gnPasswordEnable){
            if (gnDigest==0)
                gnDigest = 1;
        }
    }

    return;
}

int checkAcceptClient(){
#if defined(__OPENRTOS__)    
      gnWaitClient = 0;        
      sem_post(&semAcceptClinet);
        
      printf("[Shairport] checkAcceptClient \n");
#endif
}
int shairportStartAcceptClient()
{
    int nResult;
    pthread_t task;
    pthread_attr_t attr;

    // init semaphore
    nResult = sem_init(&semAcceptClinet, 0, 0);
    if (nResult == -1) {
        printf("[Shairport] ERROR, semAcceptClinet sem_init() fail!\r\n");
    }
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, ACCEPT_CLIENT_STACK_SIZE);
    pthread_create(&task, &attr, shairportAcceptClientThreadFunc, NULL);
    
    gShairport.taskAcceptClient = task;
    gnWaitClient = 0;
    return 0;
}

int shairport_update_password()
{
  dictionary* ini = NULL;
  char ini_filename[SHAIRPORT_INIFILENAME_MAXLEN];

  char str_password[SHAIRPORT_NAME_MAXLEN];  

#ifdef CFG_USE_SD_INI
    snprintf(ini_filename, SHAIRPORT_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
    snprintf(ini_filename, SHAIRPORT_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
    ini = iniparser_load(ini_filename);
    printf("[shairport] shairport_update_password #line %d \n",__LINE__);
    if (ini) {
        snprintf(str_password, SHAIRPORT_NAME_MAXLEN, iniparser_getstring(ini, SHAIRPORT_INIKEY_PASSWORD, ""));
        if (strlen(str_password) > 0) {
            gptPassword= strdup(str_password);
            printf("[Shairport] password=%s\r\n", gptPassword);
            gnDigest = 1;
            gnPasswordEnable = 1;
        } else {
            gnDigest = 0;
            gnPasswordEnable = 0;
        }
        
        iniparser_freedict(ini);
    } else {
        printf("[Shairport]%s() L#%ld: Cannot load ini file. Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
        ugResetFactory();
        i2s_CODEC_standby();
        exit(0);
        while (1);
    }
    
    return 0;
    
}

int shairport_main(int argc, char **argv)
{
  // unfortunately we can't just IGN on non-SysV systems
  struct sigaction sa;
  char tHWID[HWID_SIZE] = {0,51,52,53,54,55};
  char tHWID_Hex[HWID_SIZE * 2 + 1];
#if defined(__OPENRTOS__)  
  char tServerName[56] = "AudioLink";
#else
  char tServerName[56] = "AudioLink_Win32";
#endif
  char tPassword[56] = "";
  char* pServerName;

  dictionary* ini = NULL;
  char ini_filename[SHAIRPORT_INIFILENAME_MAXLEN];
  char str_name[SHAIRPORT_NAME_MAXLEN];
  char str_password[SHAIRPORT_NAME_MAXLEN];  
  struct addrinfo *tAddrInfo;
  int  tSimLevel = 0;
  int  tUseKnownHWID = FALSE;
  int  tDaemonize = FALSE;
  int  tPort = PORT;

  char *arg;
  int tIdx = 0;
  int sock = -1;
  struct ifreq ifr;

  sa.sa_handler = handle_sigchld;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGCHLD, &sa, NULL) < 0) {
      perror("sigaction");
      return 1;
  }
  gShairport.nPort = 0;
  memset(tHWID_Hex, 0, sizeof(tHWID_Hex));

  while ( (arg = *++argv) ) {
    if(!strcmp(arg, "-a"))
    {
       strncpy(tServerName, *++argv, 55);
       argc--;
    }
    else if(!strncmp(arg, "--apname=", 9))
    {
      strncpy(tServerName, arg+9, 55);
    }
    else if(!strcmp(arg, "-p"))
    {
      strncpy(tPassword, *++argv, 55);
      argc--;
    }
    else if(!strncmp(arg, "--password=",11 ))
    {
      strncpy(tPassword, arg+11, 55);
    }
    else if(!strcmp(arg, "-o"))
    {
      tPort = atoi(*++argv);
      argc--;
    }
    else if(!strncmp(arg, "--server_port=", 14))
    {
      tPort = atoi(arg+14);
    }
    else if(!strcmp(arg, "-b")) 
    {
      bufferStartFill = atoi(*++argv);
      argc--;
    }
    else if(!strncmp(arg, "--buffer=", 9))
    {
      bufferStartFill = atoi(arg + 9);
    }
    else if(!strcmp(arg, "-k"))
    {
      tUseKnownHWID = TRUE;
    }
    else if(!strcmp(arg, "-q") || !strncmp(arg, "--quiet", 7))
    {
      kCurrentLogLevel = 0;
    }
    else if(!strcmp(arg, "-d"))
    {
      tDaemonize = TRUE;
      kCurrentLogLevel = 0;
    }
    else if(!strcmp(arg, "-v"))
    {
      kCurrentLogLevel = LOG_DEBUG;
    }
    else if(!strcmp(arg, "-v2"))
    {
      kCurrentLogLevel = LOG_DEBUG_V;
    }
    else if(!strcmp(arg, "-vv") || !strcmp(arg, "-v3"))
    {
      kCurrentLogLevel = LOG_DEBUG_VV;
    }    
    else if(!strcmp(arg, "-h") || !strcmp(arg, "--help"))
    {
      slog(LOG_INFO, "ShairPort version 0.05 C port - Airport Express emulator\n");
      slog(LOG_INFO, "Usage:\nshairport [OPTION...]\n\nOptions:\n");
      slog(LOG_INFO, "  -a, --apname=AirPort    Sets Airport name\n");
      slog(LOG_INFO, "  -p, --password=secret   Sets Password (not working)\n");
      slog(LOG_INFO, "  -o, --server_port=5002  Sets Port for Avahi/dns-sd/howl\n");
      slog(LOG_INFO, "  -b, --buffer=282        Sets Number of frames to buffer before beginning playback\n");
      slog(LOG_INFO, "  -d                      Daemon mode\n");
      slog(LOG_INFO, "  -q, --quiet             Supresses all output.\n");
      slog(LOG_INFO, "  -v,-v2,-v3,-vv          Various debugging levels\n");
      slog(LOG_INFO, "\n");
      return 0;
    }    
  }
  kCurrentLogLevel = LOG_DEBUG_VV;

  if ( bufferStartFill < 30 || bufferStartFill > BUFFER_FRAMES ) {
     fprintf(stderr, "buffer value must be > 30 and < %d\n", BUFFER_FRAMES);
     return(0);
  }

  if(tDaemonize)
  {
    int tPid = fork();
    if(tPid < 0)
    {
      exit(1); // Error on fork
    }
    else if(tPid > 0)
    {
      exit(0);
    }
    else
    {
      int tIdx = 0;
      setsid();
      for(tIdx = getdtablesize(); tIdx >= 0; --tIdx)
      {
        close(tIdx);
      }
      tIdx = open(DEVNULL, O_RDWR);
      dup(tIdx);
      dup(tIdx);
    }
  }
  srandom ( time(NULL) );
  // Copy over empty 00's
  //tPrintHWID[tIdx] = tAddr[0];
#if defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI)
    if( iteEthGetLink2()){
        strcpy (ifr.ifr_name, "eth0");
        printf("[Shairport] eth and wlan, using eth \n");
    } else {
        strcpy (ifr.ifr_name, "wlan0");
        printf("[Shairport] eth and wlan, using wlan \n");        
    }
#else

    #ifdef CFG_NET_WIFI
        strcpy (ifr.ifr_name, "wlan0");
    #else
        strcpy (ifr.ifr_name, "eth0");
    #endif
#endif
  strcpy (ifr.ifr_hwaddr.sa_data, "");

  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock >= 0 && ioctl (sock, SIOCGIFHWADDR, &ifr) == 0)
  {
      unsigned char *ptr = (unsigned char *) ifr.ifr_hwaddr.sa_data;
      for(tIdx=0;tIdx<HWID_SIZE;tIdx++)
      {
        int tVal = ptr[tIdx] & 0377;
        tHWID[tIdx] = tVal;
        sprintf(tHWID_Hex+(tIdx*2), "%02X",tHWID[tIdx]);
      }
      close (sock);
  }
  else
  {
      for(tIdx=0;tIdx<HWID_SIZE;tIdx++)
      {
        if(tIdx > 0)
        {
          if(!tUseKnownHWID)
          {
            int tVal = ((random() % 80) + 33);
            tHWID[tIdx] = tVal;

          }
          //tPrintHWID[tIdx] = tAddr[tIdx];
        }
        sprintf(tHWID_Hex+(tIdx*2), "%02X",tHWID[tIdx]);
      }
      //tPrintHWID[HWID_SIZE] = '\0';
  }

#if defined(__OPENRTOS__)
    // read the setting from ini
#ifdef CFG_USE_SD_INI
    snprintf(ini_filename, SHAIRPORT_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
    snprintf(ini_filename, SHAIRPORT_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
    ini = iniparser_load(ini_filename);
    if (pServerName) {
        pServerName = NULL;
    }
    pServerName = tServerName;
    if (ini) {
        snprintf(str_name, SHAIRPORT_NAME_MAXLEN, iniparser_getstring(ini, SHAIRPORT_INIKEY_NAME, ""));
        if (strlen(str_name) > 0) {
            pServerName = strdup(str_name);
            printf("[Shairport] pServerName=%s\r\n", pServerName);
        }
        snprintf(str_password, SHAIRPORT_NAME_MAXLEN, iniparser_getstring(ini, SHAIRPORT_INIKEY_PASSWORD, ""));
        if (strlen(str_password) > 0) {
            gptPassword= strdup(str_password);
            printf("[Shairport] password=%s\r\n", gptPassword);
            gnDigest = 1;
            gnPasswordEnable = 1;
        }
        
        iniparser_freedict(ini);
    }
    else
    {
        printf("[Shairport]%s() L#%ld: Cannot load ini file. Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
        ugResetFactory();
        i2s_CODEC_standby();
        exit(0);
        while (1);
    }
#else
        pServerName = tServerName;
#endif

  slog(LOG_INFO, "LogLevel: %d\n", kCurrentLogLevel);
  slog(LOG_INFO, "AirName: %s\n", pServerName);
  slog(LOG_INFO, "HWID: %.*s\n", HWID_SIZE, tHWID+1);
  slog(LOG_INFO, "HWID_Hex(%d): %s\n", strlen(tHWID_Hex), tHWID_Hex);

  if(tSimLevel >= 1)
  {
    #ifdef SIM_INCL
    sim(tSimLevel, tTestValue, tHWID);
    #endif
    return(1);
  }
  else
  {
    int tServerSock;
    int tClientSock = 0;
#if defined(__OPENRTOS__)
    //slog(LOG_DEBUG_V, "gShairportnPort %d\n",gShairport.nPort );

    gShairport.pName = pServerName;
    gShairport.nPort = tPort;
    gShairport.pHWIDHex = tHWID_Hex;
    shairportSetException(0);
    usleep(1000);
#else
    startAvahi(tHWID_Hex, pServerName, tPort);
#endif
    slog(LOG_DEBUG_V, "Starting connection server: specified server port: %d\n", tPort);
    tServerSock = setupListenServer(&tAddrInfo, tPort);
    gShairport.pAddrInfo = tAddrInfo;
    if(tServerSock < 0)
    {
      freeaddrinfo(tAddrInfo);
      slog(LOG_INFO, "Error setting up server socket on port %d, try specifying a different port\n", tPort);
      exit(1);
    }

#if defined(__OPENRTOS__)
      gShairport.nServerSock = tServerSock;
      sem_post(&semAcceptClinet);
      printf("[Shairport] main post semAcceptClinet \n");     
#endif

    while(1)
    {
      slog(LOG_DEBUG_V, "Waiting for clients to connect %d\n",gnWaitClient);
      gShairport.nServerSock = tServerSock;
#if defined(__OPENRTOS__)
      do {
        usleep(5*1000);
      }while(gnWaitClient==0);
      tClientSock = gShairport.nClientSock;      
#else      
      tClientSock = acceptClient(tServerSock, tAddrInfo);
      gShairport.nClientSock = tClientSock;
#endif

      if(tClientSock > 0)
      {
        int tPid = fork();
        if(tPid == 0)
        {
          //freeaddrinfo(tAddrInfo);
          //tAddrInfo = NULL;
          slog(LOG_DEBUG, "...Accepted Client Connection..\n");
          //close(tServerSock);
          checkAcceptClient();
          
          // speed up
          shairportChangeSpeed(400);
    
          handleClient(tClientSock, tPassword, tHWID);
          close(tClientSock);
          shairportSetException(0);
          //return 0;
        }
        else
        {
          slog(LOG_DEBUG_VV, "Child now busy handling new client\n");
          close(tClientSock);
        }
      }
      else
      {
        // failed to init server socket....try waiting a moment...
        sleep(2);
      }
    }
  }

  slog(LOG_DEBUG_VV, "Finished, and waiting to clean everything up\n");
  sleep(1);
  if(tAddrInfo != NULL)
  {
    freeaddrinfo(tAddrInfo);
  }
  return 0;
}

static int findEnd(char *tReadBuf)
{
  // find \n\n, \r\n\r\n, or \r\r is found
  int tIdx = 0;
  int tLen = strlen(tReadBuf);
  for(tIdx = 0; tIdx < tLen; tIdx++)
  {
    if(tReadBuf[tIdx] == '\r')
    {
      if(tIdx + 1 < tLen)
      {
        if(tReadBuf[tIdx+1] == '\r')
        {
          return (tIdx+1);
        }
        else if(tIdx+3 < tLen)
        {
          if(tReadBuf[tIdx+1] == '\n' &&
             tReadBuf[tIdx+2] == '\r' &&
             tReadBuf[tIdx+3] == '\n')
          {
            return (tIdx+3);
          }
        }
      }
    }
    else if(tReadBuf[tIdx] == '\n')
    {
      if(tIdx + 1 < tLen && tReadBuf[tIdx+1] == '\n')
      {
        return (tIdx + 1);
      }
    }
  }
  // Found nothing
  return -1;
}

static void handleClient(int pSock, char *pPassword, char *pHWADDR)
{
  socklen_t len,len2;
  struct sockaddr_storage addr,addr2;
  #ifdef AF_INET6
  unsigned char ipbin[INET6_ADDRSTRLEN];
  #else
  unsigned char ipbin[INET_ADDRSTRLEN];
  #endif
  unsigned int ipbinlen;
  int port;
  char ipstr[64];
  int tMoreDataNeeded = 1;
  struct keyring     tKeys;
  struct comms       tComms = {{-1,-1}, {-1,-1}};
  struct connection  tConn;

  slog(LOG_DEBUG_VV, "In Handle Client %d \n",pSock);
  fflush(stdout);

  len = sizeof addr;
  getsockname(pSock, (struct sockaddr*)&addr, &len);

  // get client address
  len2 = sizeof addr2;
   getpeername(pSock, (struct sockaddr*)&addr2, &len2);
  // deal with both IPv4 and IPv6:
  if (addr.ss_family == AF_INET) {
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
      struct sockaddr_in *s0 = (struct sockaddr_in *)&addr2;
      
      port = ntohs(s->sin_port);
      inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
      memcpy(ipbin, &s->sin_addr, 4);
      ipbinlen = 4;
      memcpy(&gShairport.sinaddr,&s0->sin_addr,4);

      usleep(30*1000);
      
      slog(LOG_DEBUG_V, "Constructing ipv4 address %d 0x%x 0x%x\n",pSock,gShairport.sinaddr,s0->sin_addr);
  } else { // AF_INET6
#if 0  
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      union {
          struct sockaddr_in6 s;
          unsigned char bin[sizeof(struct sockaddr_in6)];
      } addr;

      slog(LOG_DEBUG_V, "Constructing ipv6 address\n");
      port = ntohs(s->sin6_port);
      #ifdef AF_INET6
      inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
      #endif
      memcpy(&addr.s, &s->sin6_addr, sizeof(struct sockaddr_in6));

      if(memcmp(&addr.bin[0], "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\xff\xff", 12) == 0)
      {
        // its ipv4...
        memcpy(ipbin, &addr.bin[12], 4);
        ipbinlen = 4;
      }
      else
      {
        memcpy(ipbin, &s->sin6_addr, 16);
        ipbinlen = 16;
      }
#endif      
    slog(LOG_DEBUG_V, "Constructing ipv4 address fail %d \n",pSock);
    return;
  }

  slog(LOG_DEBUG_V, "Peer IP address: %s\n", ipstr);
  slog(LOG_DEBUG_V, "Peer port      : %d\n", port);

  initConnection(&tConn, &tKeys, &tComms, pSock, pPassword);
  gShairport.ptConn = &tConn;
  while(1)
  {
    int tError = FALSE;

    tMoreDataNeeded = 1;

    initBuffer(&tConn.recv, 80); // Just a random, small size to seed the buffer with.
    initBuffer(&tConn.resp, 80);
    
    while(1 == tMoreDataNeeded)
    {
      tError = readDataFromClient(pSock, &(tConn.recv));
      // check audio link status      
      if (shairportgetOtherProtocolPlaying() && (gShairport.nCurrState == SHAIRPORT_PLAY || gShairport.nCurrState == HAIRTUNE_STOP)){
        slog(LOG_DEBUG_VV, "[Shairport] Airplay is playing,stop\n");
        gShairport.nCurrState = SHAIRPORT_STOP;
        cleanup(&tConn);
        // pSock was already closed
        return;        
      } else if (gnWaitClient && (gShairport.nCurrState == SHAIRPORT_PLAY || gShairport.nCurrState == HAIRTUNE_STOP)){
        slog(LOG_DEBUG_VV, "[Shairport] another Airplay device is playing,stop\n");
        gShairport.nCurrState = SHAIRPORT_STOP;
        cleanup(&tConn);
        // pSock was already closed
        return;        
      } else if (gShairport.nCurrState == HAIRTUNE_STOP){
        slog(LOG_DEBUG_VV, "[Shairport] Airplay is playing,hairtune stop\n");
        gShairport.nCurrState = SHAIRPORT_STOP;
        cleanup(&tConn);
        return;        
      }
      else{
        //gShairport.set_playstate(gShairport.nAirPlayPlaying);
        //slog(LOG_DEBUG_VV, "[Shairport] Upnp is playing,set shairport init\n");        
      }
      if(!tError && strlen(tConn.recv.data) > 0)
      {
        slog(LOG_DEBUG_VV, "Finished Reading some data from client\n");

       
        // parse client request
        tMoreDataNeeded = parseMessage(&tConn, ipbin, ipbinlen, pHWADDR);
        if(1 == tMoreDataNeeded)
        {
          slog(LOG_DEBUG_VV, "\n\nNeed to read more data\n");
        }
        else if(-1 == tMoreDataNeeded) // Forked process down below ended.
        {
          slog(LOG_DEBUG_V, "Forked Process ended...cleaning up\n");
          cleanup(&tConn);
          // pSock was already closed
          return;
        }
        // if more data needed,
      }
      else
      {
        slog(LOG_DEBUG, "Error reading from socket, closing client\n");
        // Error reading data....quit.
        cleanup(&tConn);
        return;
      }
    }
    slog(LOG_DEBUG_VV, "Writing: %d chars to socket\n", tConn.resp.current);
    
    //tConn->resp.data[tConn->resp.current-1] = '\0';
    writeDataToClient(pSock, &(tConn.resp));
   // Finished reading one message...
    cleanupBuffers(&tConn);
  }
  cleanup(&tConn);
  fflush(stdout);
  printf("[Shairport] return\n");
}

static void writeDataToClient(int pSock, struct shairbuffer *pResponse)
{
  slog(LOG_DEBUG_VV, "\n----Beg Send Response Header----\n%.*s\n", pResponse->current, pResponse->data);
  send(pSock, pResponse->data, pResponse->current,0);
  slog(LOG_DEBUG_VV, "----Send Response Header----\n");
}

static int readDataFromClient(int pSock, struct shairbuffer *pClientBuffer)
{
  char tReadBuf[MAX_SIZE];
  char* pActive;
  char pBuf[64];
  char* pURI;

  int tRetval = 1;
  int tEnd = -1;
  tReadBuf[0] = '\0';
  while(tRetval > 0 && tEnd < 0)
  {
     // Read from socket until \n\n, \r\n\r\n, or \r\r is found
      slog(LOG_DEBUG_V, "Waiting To Read...\n");
      fflush(stdout);
      tRetval = read(pSock, tReadBuf, MAX_SIZE);
      //gConStart = ithTimerGetTime(ITH_TIMER4);
      slog(LOG_DEBUG_V, "Reading ....\n");

      // if new buffer contains the end of request string, only copy partial buffer?
      tEnd = findEnd(tReadBuf);
      if(tEnd >= 0)
      {
        if(pClientBuffer->marker == 0)
        {
          pClientBuffer->marker = tEnd+1; // Marks start of content
        }
        slog(SOCKET_LOG_LEVEL, "Found end of http request at: %d\n", tEnd);
        fflush(stdout);        
      }
      else
      {
        tEnd = MAX_SIZE;
        slog(SOCKET_LOG_LEVEL, "Read %d of data so far\n%s\n", tRetval, tReadBuf);
        fflush(stdout);
      }
      if(tRetval > 0)
      {
        // Copy read data into tReceive;
        slog(SOCKET_LOG_LEVEL, "Read %d data, using %d of it\n", tRetval, tEnd);
        addNToShairBuffer(pClientBuffer, tReadBuf, tRetval);
        slog(LOG_DEBUG_VV, "Finished copying data\n");
      }
      else
      {
        slog(LOG_DEBUG, "Error reading data from socket, got: %d bytes", tRetval);
        return tRetval;
      }
  }
  if(tEnd + 1 != tRetval)
  {
    slog(SOCKET_LOG_LEVEL, "Read more data after end of http request. %d instead of %d\n", tRetval, tEnd+1);
  }
  slog(SOCKET_LOG_LEVEL, "Finished Reading Data:\n%s\nEndOfData\n", pClientBuffer->data);

  // check dacp ip and active remote
  pActive = strstr(pClientBuffer->data, SHAIRPORT_ACTIVE_REMOTE);
  if (pActive){
      sscanf(pActive+14,"%s",pActiveRemote);
      slog(SOCKET_LOG_LEVEL, "Find Active remote :%s\n", pActiveRemote);
  }
  pActive = strstr(pClientBuffer->data, SHAIRPORT_DACP_ID);
  if (pActive){
      sscanf(pActive+8,"%s",pDacpId);
      slog(SOCKET_LOG_LEVEL, "Find DACP ID :%s\n", pDacpId);
  }

    if (gnDigest){
        pActive = strstr(pClientBuffer->data, RAOP_AUTH);
        pActive = strstr(pClientBuffer->data, RAOP_RESPONSE); 
        pURI = strstr(pClientBuffer->data,"uri");

        if (pActive){
            printf("[Shairport] password response %s \n",pActive);
            printf("[Shairport]uri %s \n",pURI);
            //
            if (shairportCheckPassword(pActive+10,pURI+5,gnMethod)==1)
                gnDigest =0;
        }
    }

  fflush(stdout);
  return 0;
}

static char *getFromBuffer(char *pBufferPtr, const char *pField, int pLenAfterField, int *pReturnSize, char *pDelims)
{
  char* tFound;
  int tSize = 0;
  slog(LOG_DEBUG_V, "GettingFromBuffer: %s\n", pField);
  tFound = strstr(pBufferPtr, pField);
  if(tFound != NULL)
  {
    int tIdx = 0;
    char tDelim = pDelims[tIdx];
    char *tShortest = NULL;
    char *tEnd = NULL;
    tFound += (strlen(pField) + pLenAfterField);
    while(tDelim != '\0')
    {
      tDelim = pDelims[tIdx++]; // Ensures that \0 is also searched.
      tEnd = strchr(tFound, tDelim);
      if(tEnd != NULL && (NULL == tShortest || tEnd < tShortest))
      {
        tShortest = tEnd;
      }
    }
    
    tSize = (int) (tShortest - tFound);
    slog(LOG_DEBUG_VV, "Found %.*s  length: %d\n", tSize, tFound, tSize);
    if(pReturnSize != NULL)
    {
      *pReturnSize = tSize;
    }
  }
  else
  {
    slog(LOG_DEBUG_V, "Not Found\n");
  }
  return tFound;
}

static char *getFromHeader(char *pHeaderPtr, const char *pField, int *pReturnSize)
{
  return getFromBuffer(pHeaderPtr, pField, 2, pReturnSize, "\r\n");
}

static char *getFromContent(char *pContentPtr, const char* pField, int *pReturnSize)
{
  return getFromBuffer(pContentPtr, pField, 1, pReturnSize, "\r\n");
}

static char *getFromSetup(char *pContentPtr, const char* pField, int *pReturnSize)
{
  return getFromBuffer(pContentPtr, pField, 1, pReturnSize, ";\r\n");
}

// Handles compiling the Apple-Challenge, HWID, and Server IP Address
// Into the response the airplay client is expecting.
static int buildAppleResponse(struct connection *pConn, unsigned char *pIpBin,
                              unsigned int pIpBinLen, char *pHWID)
{
  // Find Apple-Challenge
  char *tResponse = NULL;

  int tFoundSize = 0;
  char* tFound = getFromHeader(pConn->recv.data, "Apple-Challenge", &tFoundSize);
  int nResult=0;
  if(tFound != NULL)
  {
    char* tTrim;
    int tChallengeDecodeSize = 16;
    char *tChallenge;
    int tCurSize = 0;
    unsigned char tChalResp[38];
    int tPad;
    char *tTmp;
    RSA *rsa;  // Free RSA
    int tSize;
    unsigned char* tTo;
    int tLen;

    tTrim = (char*) alloca(tFoundSize + 2);
    getTrimmed(tFound, tFoundSize, TRUE, TRUE, tTrim);
    slog(LOG_DEBUG_VV, "HeaderChallenge:  [%s] len: %d  sizeFound: %d\n", tTrim, strlen(tTrim), tFoundSize);
    tChallenge = decode_base64((unsigned char *)tTrim, tFoundSize, &tChallengeDecodeSize);
    slog(LOG_DEBUG_VV, "Challenge Decode size: %d  expected 16\n", tChallengeDecodeSize);

    memcpy(tChalResp, tChallenge, tChallengeDecodeSize);
    tCurSize += tChallengeDecodeSize;
    
    memcpy(tChalResp+tCurSize, pIpBin, pIpBinLen);
    tCurSize += pIpBinLen;

    memcpy(tChalResp+tCurSize, pHWID, HWID_SIZE);
    tCurSize += HWID_SIZE;

    tPad = 32 - tCurSize;
    if (tPad > 0)
    {
      memset(tChalResp+tCurSize, 0, tPad);
      tCurSize += tPad;
    }

    tTmp = encode_base64((unsigned char *)tChalResp, tCurSize);
    slog(LOG_DEBUG_VV, "Full sig: %s\n", tTmp);
    free(tTmp);

    gettimeofday(&gStartT, NULL);
    
#if defined(__OPENRTOS__)      
    #ifdef CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY
        nResult = shairportStopOtherProtocol();
        if (nResult==-1){
            printf("[Shairport] UPnP is playing , airplay can not play #line %d \n",__LINE__);
            return FALSE;
        }
    #else
        nResult = shairportStopOtherProtocol();
    #endif
#endif


    // RSA Encrypt
    rsa = loadKey();  // Free RSA
    tSize = RSA_size(rsa);
    tTo = (unsigned char*) alloca(tSize);
    RSA_private_encrypt(tCurSize, (unsigned char *)tChalResp, tTo, rsa, RSA_PKCS1_PADDING);
    
    // Wrap RSA Encrypted binary in Base64 encoding
    tResponse = encode_base64(tTo, tSize);
    tLen = strlen(tResponse);
    while(tLen > 1 && tResponse[tLen-1] == '=')
    {
      tResponse[tLen-1] = '\0';
    }
    free(tChallenge);
    RSA_free(rsa);
    gettimeofday(&gEndT, NULL);      
    slog(LOG_INFO, "buildAppleResponse encrypt %d \n",itpTimevalDiff(&gStartT, &gEndT));
    
  }

  if(tResponse != NULL)
  {
    // Append to current response
    addToShairBuffer(&(pConn->resp), "Apple-Response: ");
    addToShairBuffer(&(pConn->resp), tResponse);
    addToShairBuffer(&(pConn->resp), "\r\n");
    free(tResponse);
    return TRUE;
  }
  return FALSE;
}

//parseMessage(tConn->recv.data, tConn->recv.mark, &tConn->resp, ipstr, pHWADDR, tConn->keys);
static int parseMessage(struct connection *pConn, unsigned char *pIpBin, unsigned int pIpBinLen, char *pHWID)
{
  int tReturn = 0; // 0 = good, 1 = Needs More Data, -1 = close client socket.
  char *tContent;
  int nResult = 0;
  char tCStr[1024];

  if(pConn->resp.data == NULL)
  {
    initBuffer(&(pConn->resp), MAX_SIZE);
  }

  tContent = getFromHeader(pConn->recv.data, "Content-Length", NULL);
  if(tContent != NULL)
  {
    int tContentSize = atoi(tContent);
    if(pConn->recv.marker == 0 || strlen(pConn->recv.data+pConn->recv.marker) != tContentSize)
    {
      if(isLogEnabledFor(HEADER_LOG_LEVEL))
      {
        slog(HEADER_LOG_LEVEL, "Content-Length: %s value -> %d\n", tContent, tContentSize);
        if(pConn->recv.marker != 0)
        {
          slog(HEADER_LOG_LEVEL, "ContentPtr has %d, but needs %d\n", 
                  strlen(pConn->recv.data+pConn->recv.marker), tContentSize);
        }
      }
      // check if value in tContent > 2nd read from client.
      return 1; // means more content-length needed
    }
  }
  else
  {
    slog(LOG_DEBUG_VV, "No content, header only\n");
  }

  // "Creates" a new Response Header for our response message
  if (gnDigest==0){
     addToShairBuffer(&(pConn->resp), "RTSP/1.0 200 OK\r\n");
  } else {
     addToShairBuffer(&(pConn->resp), "RTSP/1.0 401 Unauthorized\r\n");

      memset(tCStr,0,sizeof(tCStr));
//      sprintf(tCStr, sizeof(tCStr), "%s%s%s\n", RAOP_AUTHENTICATE,gShairport.pName, RAOP_NONCE);
      snprintf(tCStr, sizeof(tCStr), "%s%s%s\n", RAOP_AUTHENTICATE,gShairport.pName, RAOP_NONCE);
      //addToShairBuffer(&(pConn->resp), "Server: AirTunes/105.1\n");

      addToShairBuffer(&(pConn->resp), tCStr);
     
  }

  if(isLogEnabledFor(LOG_INFO))
  {
    int tLen = strchr(pConn->recv.data, ' ') - pConn->recv.data;
    if(tLen < 0 || tLen > 20)
    {
      tLen = 20;
    }
    slog(LOG_INFO, "********** RECV %.*s **********\n", tLen, pConn->recv.data);
  }

  if(pConn->password != NULL)
  {
    
  }

  if(buildAppleResponse(pConn, pIpBin, pIpBinLen, pHWID)) // need to free sig
  {
    gettimeofday(&gEndSwitchT, NULL);      

    slog(LOG_INFO, "Added AppleResponse to Apple-Challenge request %d \n",itpTimevalDiff(&gStartT, &gEndT));
  }

  // Find option, then based on option, do different actions.
  if(strncmp(pConn->recv.data, "OPTIONS", 7) == 0)
  {
     gnMethod = 1;
     propogateCSeq(pConn);
     addToShairBuffer(&(pConn->resp),
      "Public: ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, GET_PARAMETER, SET_PARAMETER\r\n");
    
  }
  else if(!strncmp(pConn->recv.data, "ANNOUNCE", 8))
  {
    char *tContent = pConn->recv.data + pConn->recv.marker;
    int tSize = 0;
    int tIPSize = 0;
    char *tHeaderVal = getFromContent(tContent, "a=aesiv", &tSize); // Not allocated memory, just pointing
    char *ptIPVal = getFromContent(tContent, "IN IP4", &tIPSize); // Not allocated memory, just pointing
    gnMethod =2;
    memset(pDacpIp,0,sizeof(pDacpIp));
    memcpy(pDacpIp,ptIPVal,tIPSize);
    printf("[Shairport] dacp ip %s %d \n",pDacpIp,strlen(pDacpIp));

    if(tSize > 0)
    {
      int tKeySize = 0;
      char* tEncodedAesIV = alloca(tSize + 2);
      char *tDecodedIV;
      char* tEncodedAesKey;
      char *tDecodedAesKey;
      int tFmtpSize = 0;
      char *tFmtp;
      RSA *rsa;
      char *tDecryptedKey;
      gettimeofday(&gStartSwitchT, NULL);
      
#if defined(__OPENRTOS__)      
    #ifdef CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY
        nResult = shairportStopOtherProtocol();
        if (nResult==-1){
            printf("[Shairport] UPnP is playing , airplay can not play #line %d \n",__LINE__);
            return -1;
        }
    #else
        nResult = shairportStopOtherProtocol();
    #endif
#endif

      getTrimmed(tHeaderVal, tSize, TRUE, TRUE, tEncodedAesIV);
      slog(LOG_DEBUG_VV, "AESIV: [%.*s] Size: %d  Strlen: %d\n", tSize, tEncodedAesIV, tSize, strlen(tEncodedAesIV));
      tDecodedIV =  decode_base64((unsigned char*) tEncodedAesIV, tSize, &tSize);

      // grab the key, copy it out of the receive buffer
      tHeaderVal = getFromContent(tContent, "a=rsaaeskey", &tKeySize);
      tEncodedAesKey = alloca(tKeySize + 2); // +1 for nl, +1 for \0
      getTrimmed(tHeaderVal, tKeySize, TRUE, TRUE, tEncodedAesKey);
      slog(LOG_DEBUG_VV, "AES KEY: [%s] Size: %d  Strlen: %d\n", tEncodedAesKey, tKeySize, strlen(tEncodedAesKey));
      // remove base64 coding from key
      tDecodedAesKey = decode_base64((unsigned char*) tEncodedAesKey,
                              tKeySize, &tKeySize);  // Need to free DecodedAesKey

      // Grab the formats
      tFmtp = getFromContent(tContent, "a=fmtp", &tFmtpSize);  // Don't need to free
      tFmtp = getTrimmedMalloc(tFmtp, tFmtpSize, TRUE, FALSE); // will need to free
      slog(LOG_DEBUG_VV, "Format: %s\n", tFmtp);

      gettimeofday(&gStartT, NULL);

      rsa = loadKey();
      // Decrypt the binary aes key
      tDecryptedKey = malloc(RSA_size(rsa) * sizeof(char)); // Need to Free Decrypted key
      //char tDecryptedKey[RSA_size(rsa)];
      if(RSA_private_decrypt(tKeySize, (unsigned char *)tDecodedAesKey, 
      (unsigned char*) tDecryptedKey, rsa, RSA_PKCS1_OAEP_PADDING) >= 0)
      {
        slog(LOG_DEBUG, "Decrypted AES key from RSA Successfully\n");
      }
      else
      {
        slog(LOG_INFO, "Error Decrypting AES key from RSA\n");
      }
      free(tDecodedAesKey);
      RSA_free(rsa);
      gettimeofday(&gEndT, NULL);      
      slog(LOG_INFO, "Decrypted AES key from RSA %d\n",itpTimevalDiff(&gStartT, &gEndT));
      


      setKeys(pConn->keys, tDecodedIV, tDecryptedKey, tFmtp);

      propogateCSeq(pConn);
    }
  }
  else if(!strncmp(pConn->recv.data, "SETUP", 5))
  {
    char tPort[8] = "6000";  // get this from dup()'d stdout of child pid
    int tPid;
    char *tFound;

    // Setup pipes
    struct comms *tComms = pConn->hairtunes;
    if (! (pipe(tComms->in) == 0 && pipe(tComms->out) == 0))
    {
       //slog(LOG_INFO, "Error setting up hairtunes communications...some things probably wont work very well.\n");
    }
    
    // Setup fork
    tPid = fork();
    if(tPid == 0)
    {
      int tDataport=0;
      char tCPortStr[8] = "59010";
      char tTPortStr[8] = "59012";
      int tSize = 0;
      int tControlport;
      int tTimingport;
      char *tRtp = NULL;
      char *tPipe = NULL;
      char *tAoDriver = NULL;
      char *tAoDeviceName = NULL;
      char *tAoDeviceId = NULL;
      struct keyring *tKeys;

      char *tFound  =getFromSetup(pConn->recv.data, "control_port", &tSize);
      getTrimmed(tFound, tSize, 1, 0, tCPortStr);
      tFound = getFromSetup(pConn->recv.data, "timing_port", &tSize);
      getTrimmed(tFound, tSize, 1, 0, tTPortStr);

      slog(LOG_DEBUG_VV, "converting %s and %s from str->int\n", tCPortStr, tTPortStr);
      tControlport = atoi(tCPortStr);
      tTimingport = atoi(tTPortStr);

      slog(LOG_DEBUG_V, "Got %d for CPort and %d for TPort\n", tControlport, tTimingport);

      // *************************************************
      // ** Setting up Pipes, AKA no more debug/output  **
      // *************************************************
#if 0
      dup2(tComms->in[0],0);   // Input to child
      closePipe(&(tComms->in[0]));
      closePipe(&(tComms->in[1]));

      dup2(tComms->out[1], 1); // Output from child
      closePipe(&(tComms->out[1]));
      closePipe(&(tComms->out[0]));
#endif // 0
      tKeys = pConn->keys;
#if 0
      pConn->keys = NULL;
      pConn->hairtunes = NULL;

       Free up any recv buffers, etc..
      if(pConn->clientSocket != -1)
      {
        close(pConn->clientSocket);
        pConn->clientSocket = -1;
      }
      cleanupBuffers(pConn);
#endif // 0

#if defined(__OPENRTOS__)
    #ifdef CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY
        nResult = shairportStopOtherProtocol();
        if (nResult==-1){
            printf("[Shairport] UPnP is playing , airplay can not play #line %d \n",__LINE__);
            return -1;
        }
    #else
        nResult = shairportStopOtherProtocol();
    #endif
#endif

       // speed down
       shairportChangeSpeed(270);
    
      slog(LOG_DEBUG_VV, "[Shairport] hairtunes_init #line %d %d\n",__LINE__,gShairport.nCurrState);
      hairtunes_init(tKeys->aeskey, tKeys->aesiv, tKeys->fmt, tControlport, tTimingport,
                      tDataport, tRtp, tPipe, tAoDriver, tAoDeviceName, tAoDeviceId,
                      bufferStartFill,gInit);
      gInit =1;
      // Quit when finished.

#if defined(__OPENRTOS__)
      slog(LOG_DEBUG_VV, "[Shairport] set_playstate airplaying #line %d \n",__LINE__);
      gShairport.set_playstate(gShairport.nAirPlayPlaying);
      gShairport.nCurrState = SHAIRPORT_PLAY;
#endif

#if 0
      slog(LOG_DEBUG, "Returned from hairtunes init....returning -1, should close out this whole side of the fork\n");
      return -1;
#endif // 0
    }
    if(tPid ==0)
    {
      char tFromHairtunes[80];
      int tRead;
      int tSize = 0;
      char *tTransport;

      // Ensure Connection has access to the pipe.
      closePipe(&(tComms->in[0]));
      closePipe(&(tComms->out[1]));

      tRead = read(tComms->out[0], tFromHairtunes, 80);
      if(tRead <= 0)
      {
        slog(LOG_INFO, "Error reading port from hairtunes function, assuming default port: %d\n", tPort);
      }
      else
      {
        int tSize = 0;
        char *tPortStr = getFromHeader(tFromHairtunes, "port", &tSize);
        if(tPortStr != NULL)
        {
          getTrimmed(tPortStr, tSize, TRUE, FALSE, tPort);
        }
        else
        {
          slog(LOG_INFO, "Read %d bytes, Error translating %s into a port\n", tRead, tFromHairtunes);
        }
      }
      //  READ Ports from here?close(pConn->hairtunes_pipes[0]);
      propogateCSeq(pConn);
      tTransport = getFromHeader(pConn->recv.data, "Transport", &tSize);
      addToShairBuffer(&(pConn->resp), "Transport: ");
      addNToShairBuffer(&(pConn->resp), tTransport, tSize);
      // Append server port:
      addToShairBuffer(&(pConn->resp), ";server_port=");
      addToShairBuffer(&(pConn->resp), tPort);
      addToShairBuffer(&(pConn->resp), "\r\nSession: DEADBEEF\r\n");
    }
    else
    {
      slog(LOG_INFO, "Error forking process....dere' be errors round here.\n");
      return -1;
    }
  }
  else if(!strncmp(pConn->recv.data, "TEARDOWN", 8))
  {
    // Be smart?  Do more finish up stuff...
    addToShairBuffer(&(pConn->resp), "Connection: close\r\n");
    propogateCSeq(pConn);
    //close(pConn->hairtunes->in[1]);
    slog(LOG_INFO, "Tearing down connection, closing pipes\n");
    //close(pConn->hairtunes->out[0]);
    tReturn = -1;  // Close client socket, but sends an ACK/OK packet first
#if defined(__OPENRTOS__)      
    gShairport.nCurrState = SHAIRPORT_STOP;
    hairtunes_flush(1);
    gShairport.set_playstate(gShairport.nStop);
    slog(LOG_INFO, "[Shairport] set_playstate stop #line %d \n",__LINE__);
#endif    
  }
  else if(!strncmp(pConn->recv.data, "FLUSH", 5))
  {
    //write(pConn->hairtunes->in[1], "flush\n", 6);
    hairtunes_flush(1);
    propogateCSeq(pConn);
  }
  else if(!strncmp(pConn->recv.data, "SET_PARAMETER", 13))
  {
    int tSize = 0;
    char *tVol;
    propogateCSeq(pConn);
    tVol = getFromHeader(pConn->recv.data, "volume", &tSize);
    slog(LOG_DEBUG_VV, "About to write [vol: %.*s] data to hairtunes\n", tSize, tVol);

    //write(pConn->hairtunes->in[1], "vol: ", 5);
    //write(pConn->hairtunes->in[1], tVol, tSize);
    //write(pConn->hairtunes->in[1], "\n", 1);
    gVolume = atof(tVol);
    hairtunes_vol(atof(tVol));
    slog(LOG_DEBUG_VV, "Finished writing data write data to hairtunes\n");
  }
  else
  {
    slog(LOG_DEBUG, "\n\nUn-Handled recv: %s\n", pConn->recv.data);
    propogateCSeq(pConn);
  }
/*
  if (gnDigest==1){
      memset(tCStr,0,sizeof(tCStr));
//      sprintf(tCStr, sizeof(tCStr), "%s%s%s\n", RAOP_AUTHENTICATE,gShairport.pName, RAOP_NONCE);
      snprintf(tCStr, sizeof(tCStr), "%s%s%s\n", RAOP_AUTHENTICATE,gShairport.pName, RAOP_NONCE);
      //addToShairBuffer(&(pConn->resp), "Server: AirTunes/105.1\n");

      addToShairBuffer(&(pConn->resp), tCStr);
  }
  */
//  malloc_stats();    
  
  addToShairBuffer(&(pConn->resp), "\r\n");
  return tReturn;
}

// Copies CSeq value from request, and adds standard header values in.
static void propogateCSeq(struct connection *pConn) //char *pRecvBuffer, struct shairbuffer *pConn->recp.data)
{
  int tSize=0;
  char *tRecPtr = getFromHeader(pConn->recv.data, "CSeq", &tSize);
  addToShairBuffer(&(pConn->resp), "Audio-Jack-Status: connected; type=analog\r\n");
  addToShairBuffer(&(pConn->resp), "CSeq: ");
  addNToShairBuffer(&(pConn->resp), tRecPtr, tSize);
  addToShairBuffer(&(pConn->resp), "\r\n");
}

void cleanupBuffers(struct connection *pConn)
{
  if(pConn->recv.data != NULL)
  {
    free(pConn->recv.data);
    pConn->recv.data = NULL;
  }
  if(pConn->resp.data != NULL)
  {
    free(pConn->resp.data);
    pConn->resp.data = NULL;
  }
}

static void cleanup(struct connection *pConn)
{
  cleanupBuffers(pConn);
  if(pConn->hairtunes != NULL)
  {

    closePipe(&(pConn->hairtunes->in[0]));
    closePipe(&(pConn->hairtunes->in[1]));
    closePipe(&(pConn->hairtunes->out[0]));
    closePipe(&(pConn->hairtunes->out[1]));
  }
  if(pConn->keys != NULL)
  {
    if(pConn->keys->aesiv != NULL)
    {
      free(pConn->keys->aesiv);
    }
    if(pConn->keys->aeskey != NULL)
    {
      free(pConn->keys->aeskey);
    }
    if(pConn->keys->fmt != NULL)
    {
      free(pConn->keys->fmt);
    }
    pConn->keys = NULL;
  }
  if(pConn->clientSocket != -1)
  {
    close(pConn->clientSocket);
    pConn->clientSocket = -1;
  }
}


extern int PosixDaemon_main(int argc, char **argv);

static DNSServiceRef service;
static DNSServiceRef serviceHttp;

#if 0
extern int mDNSResponder_main(int argc, char **argv);
static void* mDNSResponderFunc(void* arg)
{
    static char* args[] = 
    { 
        "mDNSResponder",
        "-n",
        NULL,
        "-t",
        "_raop._tcp",
        "-p",
        NULL,
    #ifndef NDEBUG
        "-v2",
    #endif
        "-x",
        "tp=UDP",
        "sm=false",
        "sv=false",
        "ek=1",
        "et=0,1",
        "cn=0,1",
        "ch=2",
        "ss=16",
        "sr=44100",
        "pw=false",
        "vn=3",
        "txtvers=1",
    };
    char** args2 = (char**)arg;
    args[2] = args2[0];
    args[6] = args2[1];

    return (void*)mDNSResponder_main(sizeof(args) / sizeof(args[0]), args);
}
#endif

static void  MyDNSServiceBrowseReply
							(
								DNSServiceRef                       sdRef,
								DNSServiceFlags                     flags,
								uint32_t                            interfaceIndex,
								DNSServiceErrorType                 errorCode,
								const char                          *serviceName,
								const char                          *regtype,
								const char                          *replyDomain,
								void                                *context
							)
{
    printf("MyDNSServiceBrowseReply %d \n",flags);
}



static void* PosixDaemonFunc(void* arg)
{
    static char* args[] = 
    {
        "-debug"
    };

    return (void*)PosixDaemon_main(ITH_COUNT_OF(args), args);
}


static void
	_BonjourRegistrationHandler( 
		DNSServiceRef		inRef, 
		DNSServiceFlags		inFlags, 
		DNSServiceErrorType	inError, 
		const char *		inName,
		const char *		inType,
		const char *		inDomain,
		void *				inContext )
{
    printf("BonjourRegistrationHandler %d \n",inFlags);

}


static int startAvahi(const char *pHWStr, const char *pServerName, int pPort)
{
  int tMaxServerName = 25; // Something reasonable?  iPad showed 21, iphone 25
  int tPid = fork();
 // test 
  DNSServiceErrorType error;
  const char *record[] = { MDNS_RECORD, NULL };  
  uint16_t length = 0;
  const char **field;    // Concatenate string contained i record into buf.
  char buf[256];
  char *p;
  char * newp;
  int err;
    TXTRecordRef		txtRec;
    uint8_t				txtBuf[ 256 ];

	TXTRecordCreate( &txtRec, (uint16_t) sizeof( txtBuf ), txtBuf );

    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_CompressionTypes, 3, "0,1" );
	err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_RFC2617DigestAuth, 4, "true" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_EncryptionTypes, 3, "0,1" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_FirmwareSourceVersion, 3, "2.3" );
	err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_AppleModel, 9, "audiolink" );
	err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_StatusFlags, 3 , "0x6");
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_Password, 5, "false" );

    err = TXTRecordSetValue( &txtRec, "ss", 2, "16" );
	err = TXTRecordSetValue( &txtRec, "sr", 5, "44100" );
    err = TXTRecordSetValue( &txtRec, "sm", 5, "false" );
    
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_Channels, 1, "2" );
	err = TXTRecordSetValue( &txtRec, "txtvers", 1, "1" );
    err = TXTRecordSetValue( &txtRec, "pw", 5, "false" );
	err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_TransportTypes, 3, "UDP" );
	err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_ProtocolVersion, 5, "65537" );
	err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_SourceVersion, 3, "2.2" );
////////////////////////////////////////////////////////////////////////////////////////////////////////////




  if(tPid == 0)
  {
    //static char tName[100 + HWID_SIZE + 3];
    static char tName[256];
    static char tPort[SERVLEN];
    static char* args[] = { tName, tPort };
    pthread_t task;
    if(strlen(pServerName) > tMaxServerName)
    {
      slog(LOG_INFO,"Hey dog, we see you like long server names, "
              "so we put a strncat in our command so we don't buffer overflow, while you listen to your flow.\n"
              "We just used the first %d characters.  Pick something shorter if you want\n", tMaxServerName);
    }
    
    tName[0] = '\0';
    sprintf(tPort, "%d", pPort);
    strcat(tName, pHWStr);
    strcat(tName, "@");
    strncat(tName, pServerName, tMaxServerName);
    slog(AVAHI_LOG_LEVEL, "Avahi/DNS-SD Name: %s\n", tName);
#if 0    
    execlp("avahi-publish-service", "avahi-publish-service", tName,
         "_raop._tcp", tPort, "tp=UDP","sm=false","sv=false","ek=1","et=0,1",
         "cn=0,1","ch=2","ss=16","sr=44100","pw=false","vn=3","txtvers=1", NULL);
    execlp("dns-sd", "dns-sd", "-R", tName,
         "_raop._tcp", ".", tPort, "tp=UDP","sm=false","sv=false","ek=1","et=0,1",
         "cn=0,1","ch=2","ss=16","sr=44100","pw=false","vn=3","txtvers=1", NULL);
    execlp("mDNSPublish", "mDNSPublish", tName,
         "_raop._tcp", tPort, "tp=UDP","sm=false","sv=false","ek=1","et=0,1",
         "cn=0,1","ch=2","ss=16","sr=44100","pw=false","vn=3","txtvers=1", NULL);
    if(errno == -1) {
            perror("error");
    }

    slog(LOG_INFO, "Bad error... couldn't find or failed to run: avahi-publish-service OR dns-sd OR mDNSPublish\n");
    exit(1);
#endif // 0

    //pthread_create(&task, NULL, mDNSResponderFunc, args);
    pthread_create(&task, NULL, PosixDaemonFunc, NULL);
    usleep(500000);
    
    error = DNSServiceRegister (&service, 0, kDNSServiceInterfaceIndexAny, 
        tName, kRAOPBonjourServiceType, kRAOPBonjourServiceDomain, NULL, htons( pPort ), 
        TXTRecordGetLength( &txtRec ), TXTRecordGetBytesPtr( &txtRec ), _BonjourRegistrationHandler, NULL);

    if (error == kDNSServiceErr_NoError) {
        printf(" dns-sd DNSServiceRegister   \n");
    } else {
        printf("dns-sd: DNSServiceRegister error %d", error);
//        error = DNSServiceRegister (&service, 0, kDNSServiceInterfaceIndexAny, 
//        tName, kRAOPBonjourServiceType, kRAOPBonjourServiceDomain, NULL, htons( pPort ), 
//        length, buf, NULL, NULL);

    }
/*    
    printf("Starting DNSServiceBrowse \n");
    error = DNSServiceBrowse(&service,0,0,kDACPBonjourServiceType,kRAOPBonjourServiceDomain,MyDNSServiceBrowseReply, NULL);
    printf("DNSServiceBrowse nResult %d \n",error);*/
  }
  else
  {
    slog(LOG_DEBUG_VV, "Avahi/DNS-SD started on PID: %d\n", tPid);
  }
  return tPid;
}

int shairportClose()
{
    //  raop close
    if( service ) {
        
        printf("[shairport] close raop \n");
        DNSServiceRefDeallocate( service );
    }
    gnDigest = 0;
}

int shairportRegistRAOP(int nPassword)
{
    DNSServiceErrorType error;

    int err;
    TXTRecordRef		txtRec;
    uint8_t				txtBuf[ 4096*2 ];
    TXTRecordCreate( &txtRec, (uint16_t) sizeof( txtBuf ), txtBuf );
    
#if 1
    //tp=UDP?sm=false?sv=false?ek=1?et=0,1?cn=0,1?cn=0,1?ch=2?ss=16?sr=44100?pw=false?vn=3?txtvers=1
    if (nPassword==0) {
        err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_TransportTypes, 3, "UDP" );
        err = TXTRecordSetValue( &txtRec, "am", 9, "AudioLink" );
        err = TXTRecordSetValue( &txtRec, "da", 4, "true" );    
        err = TXTRecordSetValue( &txtRec, "sm", 5, "false" );
        err = TXTRecordSetValue( &txtRec, "sv", 5, "false" );
        err = TXTRecordSetValue( &txtRec, "ek", 1, "1" );
        err = TXTRecordSetValue( &txtRec, "et", 3, "0,1" );
        err = TXTRecordSetValue( &txtRec, "fv", 4, "5.72" );
        err = TXTRecordSetValue( &txtRec, "cn", 3, "0,1" );
        err = TXTRecordSetValue( &txtRec, "ch", 1, "2" );
        err = TXTRecordSetValue( &txtRec, "ss", 2, "16" );
        err = TXTRecordSetValue( &txtRec, "sr", 5, "44100" );
        err = TXTRecordSetValue( &txtRec, "pw", 5, "false" );
        err = TXTRecordSetValue( &txtRec, "vn", 1, "3" );
        err = TXTRecordSetValue( &txtRec, "txtvers", 1, "1" );
        err = TXTRecordSetValue( &txtRec, "vs", 5, "100.8" );
    } else {
        err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_TransportTypes, 3, "UDP" );
        err = TXTRecordSetValue( &txtRec, "am", 9, "AudioLink" );
        err = TXTRecordSetValue( &txtRec, "da", 4, "true" );    
        err = TXTRecordSetValue( &txtRec, "sm", 5, "false" );
        err = TXTRecordSetValue( &txtRec, "sv", 5, "false" );
        err = TXTRecordSetValue( &txtRec, "ek", 1, "1" );
        err = TXTRecordSetValue( &txtRec, "et", 3, "0,1" );
        err = TXTRecordSetValue( &txtRec, "fv", 4, "5.72" );
        err = TXTRecordSetValue( &txtRec, "cn", 3, "0,1" );
        err = TXTRecordSetValue( &txtRec, "ch", 1, "2" );
        err = TXTRecordSetValue( &txtRec, "ss", 2, "16" );
        err = TXTRecordSetValue( &txtRec, "sr", 5, "44100" );
        err = TXTRecordSetValue( &txtRec, "pw", 4, "true" );
        err = TXTRecordSetValue( &txtRec, "vn", 1, "3" );
        err = TXTRecordSetValue( &txtRec, "txtvers", 1, "1" );
        err = TXTRecordSetValue( &txtRec, "vs", 5, "100.8" );
        printf("[Shairport DACP] shairportRegistRAOP set password \n");
//        gnDigest = 1;

    }

#else
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_TransportTypes, 3, "UDP" );
    err = TXTRecordSetValue( &txtRec, "sm", 5, "false" );
    err = TXTRecordSetValue( &txtRec, "sv", 5, "false" );
    err = TXTRecordSetValue( &txtRec, "ek", 1, "1" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_CompressionTypes, 3, "0,1" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_RFC2617DigestAuth, 4, "true" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_EncryptionTypes, 3, "0,1" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_FirmwareSourceVersion, 3, "2.3" );
    //err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_AppleModel, 9, "audiolink" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_StatusFlags, 3 , "0x6");
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_Password, 5, "false" );

    err = TXTRecordSetValue( &txtRec, "ss", 2, "16" );
    err = TXTRecordSetValue( &txtRec, "sr", 5, "44100" );


    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_Channels, 1, "2" );
    err = TXTRecordSetValue( &txtRec, "txtvers", 1, "1" );
    err = TXTRecordSetValue( &txtRec, "pw", 5, "false" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_ProtocolVersion, 5, "65537" );
    err = TXTRecordSetValue( &txtRec, kRAOPTXTKey_SourceVersion, 3, "2.2" );
#endif

    printf("[Shairport] DNSServiceRegister \n");
    error = DNSServiceRegister (&service, 0, kDNSServiceInterfaceIndexAny, 
    gtName, kRAOPBonjourServiceType, kRAOPBonjourServiceDomain, NULL, htons( gShairport.nPort ), 
    TXTRecordGetLength( &txtRec ), TXTRecordGetBytesPtr( &txtRec ), _BonjourRegistrationHandler, NULL);

    if (error == kDNSServiceErr_NoError) {
        printf("[Shairport] dns-sd DNSServiceRegister  no error  \n");
    } else {
        printf("[Shairport]dns-sd: DNSServiceRegister error %d", error);
    }

/*
    usleep(200000);

    error = DNSServiceRegister (&serviceHttp, 0, kDNSServiceInterfaceIndexAny, 
    gShairport.pName, "_http._tcp", kRAOPBonjourServiceDomain, NULL, htons( 80), 
    NULL, NULL, NULL, NULL);

    if (error == kDNSServiceErr_NoError) {
        printf("[Shairport] dns-sd DNSServiceRegister  no error \n");
    } else {
        printf("[Shairport] dns-sd: DNSServiceRegister error %d", error);
    }
*/
    return 0;
    
}

#if 1
int startmDNSResponder()
{
    int tMaxServerName = 25; // Something reasonable?  iPad showed 21, iphone 25
    int tPid = fork();
    DNSServiceErrorType error;

    int err;
    TXTRecordRef		txtRec;
    uint8_t				txtBuf[ 4096*2 ];

    //  printf("startDNSResponder \n");
    if(tPid == 0)
    {
        static char tPort[SERVLEN];
        static char* args[] = { gtName, tPort };
        pthread_t task;
        if(strlen(gShairport.pName) > tMaxServerName)
        {
            slog(LOG_INFO,"Hey dog, we see you like long server names, "
            "so we put a strncat in our command so we don't buffer overflow, while you listen to your flow.\n"
            "We just used the first %d characters.  Pick something shorter if you want\n", tMaxServerName);
        }

        gtName[0] = '\0';
        sprintf(tPort, "%d", gShairport.nPort);
        strcat(gtName, gShairport.pHWIDHex);
        strcat(gtName, "@");
        strncat(gtName, gShairport.pName, tMaxServerName);
        usleep(2000);
        printf("[Shairport] PosixDaemonFunc %s %d \n",gtName,gShairport.nPort );

        slog(AVAHI_LOG_LEVEL, "Avahi/DNS-SD Name: %s\n", gtName);
        gShairport.taskmDNSResponder = task;
        //    pthread_create(&task, NULL, mDNSResponderFunc, args);
        pthread_create(&task, NULL, PosixDaemonFunc, args);
        usleep(600000);

        //
        printf("[Shairport] gnDigest %d \n",gnDigest);
        if (gnDigest==1){
            shairportRegistRAOP(1);
        } else {
            shairportRegistRAOP(0);
        }
        
        usleep(200000);

        error = DNSServiceRegister (&serviceHttp, 0, kDNSServiceInterfaceIndexAny, 
        gShairport.pName, "_http._tcp", kRAOPBonjourServiceDomain, NULL, htons(80), 
        NULL, NULL, NULL, NULL);

        if (error == kDNSServiceErr_NoError) {
            printf("[Shairport] dns-sd DNSServiceRegister  no error \n");
        } else {
            printf("[Shairport] dns-sd: DNSServiceRegister error %d", error);
        }

    }
    else
    {
        slog(LOG_DEBUG_VV, "Avahi/DNS-SD started on PID: %d\n", tPid);
    }
    return tPid;
}
#else
int startmDNSResponder()
{
  int tMaxServerName = 25; // Something reasonable?  iPad showed 21, iphone 25
  int tPid = fork();

//  printf("startDNSResponder \n");
  if(tPid == 0)
  {
    static char tName[100 + HWID_SIZE + 3];
    static char tPort[SERVLEN];
    static char* args[] = { tName, tPort };
    pthread_t task;
    if(strlen(gShairport.pName) > tMaxServerName)
    {
      slog(LOG_INFO,"Hey dog, we see you like long server names, "
              "so we put a strncat in our command so we don't buffer overflow, while you listen to your flow.\n"
              "We just used the first %d characters.  Pick something shorter if you want\n", tMaxServerName);
    }
    
    tName[0] = '\0';
    sprintf(tPort, "%d", gShairport.nPort);
    strcat(tName, gShairport.pHWIDHex);
    strcat(tName, "@");
    strncat(tName, gShairport.pName, tMaxServerName);
    
    slog(AVAHI_LOG_LEVEL, "Avahi/DNS-SD Name: %s\n", tName);
    gShairport.taskmDNSResponder = task;
//    pthread_create(&task, NULL, mDNSResponderFunc, args);
  }
  else
  {
    slog(LOG_DEBUG_VV, "Avahi/DNS-SD started on PID: %d\n", tPid);
  }
  return tPid;
}
#endif

static int getAvailChars(struct shairbuffer *pBuf)
{
  return (pBuf->maxsize / sizeof(char)) - pBuf->current;
}

static void addToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf)
{
  addNToShairBuffer(pBuf, pNewBuf, strlen(pNewBuf));
}

static void addNToShairBuffer(struct shairbuffer *pBuf, char *pNewBuf, int pNofNewBuf)
{
  int tAvailChars = getAvailChars(pBuf);
  if(pNofNewBuf > tAvailChars)
  {
    int tNewSize = pBuf->maxsize * 2 + MAX_SIZE + sizeof(char);
    char *tTmpBuf = malloc(tNewSize);
    // check  out of memory 
    if(tTmpBuf!= NULL){
        tTmpBuf[0] = '\0';
        memset(tTmpBuf, 0, tNewSize/sizeof(char));
        memcpy(tTmpBuf, pBuf->data, pBuf->current);
    } else {
        printf("[Shairport ]addNToShairBuffer tTmpBuf null %d \n",__LINE__);
        return;
    }
    
    free(pBuf->data);

    pBuf->maxsize = tNewSize;
    pBuf->data = tTmpBuf;
  }
  memcpy(pBuf->data + pBuf->current, pNewBuf, pNofNewBuf);
  pBuf->current += pNofNewBuf;
  if(getAvailChars(pBuf) > 1)
  {
    pBuf->data[pBuf->current] = '\0';
  }
}

static char *getTrimmedMalloc(char *pChar, int pSize, int pEndStr, int pAddNL)
{
  int tAdditionalSize = 0;
  char *tTrimDest;
  if(pEndStr)
    tAdditionalSize++;
  if(pAddNL)
    tAdditionalSize++;
  tTrimDest = malloc(sizeof(char) * (pSize + tAdditionalSize));

  // check  out of memory
  if(tTrimDest== NULL){
      printf("[Shairport ]getTrimmedMalloc tTrimDest null %d \n",__LINE__);
      return NULL;
  }
  
  return getTrimmed(pChar, pSize, pEndStr, pAddNL, tTrimDest);
}

// Must free returned ptr
static char *getTrimmed(char *pChar, int pSize, int pEndStr, int pAddNL, char *pTrimDest)
{
  int tSize = pSize;
  if(pEndStr)
  {
    tSize++;
  }
  if(pAddNL)
  {
    tSize++;
  }
  
  memset(pTrimDest, 0, tSize);
  memcpy(pTrimDest, pChar, pSize);
  if(pAddNL)
  {
    pTrimDest[pSize] = '\n';
  }
  if(pEndStr)
  {
    pTrimDest[tSize-1] = '\0';
  }
  return pTrimDest;
}

static void slog(int pLevel, char *pFormat, ...)
{
  #ifdef SHAIRPORT_LOG
  if(isLogEnabledFor(pLevel))
  {
    va_list argp;
    va_start(argp, pFormat);
    vprintf(pFormat, argp);
    va_end(argp);
  }
  #endif
}

static int isLogEnabledFor(int pLevel)
{
  if(pLevel <= kCurrentLogLevel)
  {
    return TRUE;
  }
  return FALSE;
}

static void initConnection(struct connection *pConn, struct keyring *pKeys,
                    struct comms *pComms, int pSocket, char *pPassword)
{
  pConn->hairtunes = pComms;
  if(pKeys != NULL)
  {
    pConn->keys = pKeys;
    pConn->keys->aesiv = NULL;
    pConn->keys->aeskey = NULL;
    pConn->keys->fmt = NULL;
  }
  pConn->recv.data = NULL;  // Pre-init buffer expected to be NULL
  pConn->resp.data = NULL;  // Pre-init buffer expected to be NULL
  pConn->clientSocket = pSocket;
  if(strlen(pPassword) >0)
  {
    pConn->password = pPassword;
  }
  else
  {
    pConn->password = NULL;
  }
}

static void closePipe(int *pPipe)
{
  if(*pPipe != -1)
  {
    close(*pPipe);
    *pPipe = -1;
  }
}

static void initBuffer(struct shairbuffer *pBuf, int pNumChars)
{
  if(pBuf->data != NULL)
  {
    slog(LOG_DEBUG_VV, "Hrm, buffer wasn't cleaned up....trying to free\n");
    free(pBuf->data);
    slog(LOG_DEBUG_VV, "Free didn't seem to seg fault....huzzah\n");
  }
  pBuf->current = 0;
  pBuf->marker = 0;
  pBuf->maxsize = sizeof(char) * pNumChars;
  pBuf->data = malloc(pBuf->maxsize);
  memset(pBuf->data, 0, pBuf->maxsize);
}

double shairportGetVolume()
{

    return gVolume;
}


static void setKeys(struct keyring *pKeys, char *pIV, char* pAESKey, char *pFmtp)
{
  if(pKeys->aesiv != NULL)
  {
    free(pKeys->aesiv);
  }
  if(pKeys->aeskey != NULL)
  {
    free(pKeys->aeskey);
  }
  if(pKeys->fmt != NULL)
  {
    free(pKeys->fmt);
  }
  pKeys->aesiv = pIV;
  pKeys->aeskey = pAESKey;
  pKeys->fmt = pFmtp;
}

#define AIRPORT_PRIVATE_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\n" \
"wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\n" \
"wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\n" \
"/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\n" \
"UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\n" \
"BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\n" \
"LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\n" \
"NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\n" \
"lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\n" \
"aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\n" \
"a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\n" \
"oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\n" \
"oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\n" \
"k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\n" \
"AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\n" \
"cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\n" \
"54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\n" \
"17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\n" \
"1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\n" \
"LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\n" \
"2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKY=\n" \
"-----END RSA PRIVATE KEY-----"

static RSA *loadKey(void)
{
  BIO *tBio = BIO_new_mem_buf(AIRPORT_PRIVATE_KEY, -1);
  RSA *rsa = PEM_read_bio_RSAPrivateKey(tBio, NULL, NULL, NULL); //NULL, NULL, NULL);
  BIO_free(tBio);
  slog(RSA_LOG_LEVEL, "RSA Key: %d\n", RSA_check_key(rsa));
  return rsa;
}
