#ifndef  _NTP_TIME
#define  _NTP_TIME 

#include <assert.h>
#include <time.h>
#include "datatype.h"


static const double NTP_FRACTIONAL_TO_MS = (((double)1000.0)/0xFFFFFFFF); //230ms->10位十进制,NTP_TO_SECOND表示十进制中的1代表多少秒，就是把1秒切割成32bit的数。

static const double NTP_TO_SECOND = (((double)1.0)/0xFFFFFFFF);

static const long JAN_1ST_1900 = 2415021;

//Representation of an NTP timestamp

typedef struct 
{
	ulong m_dwInteger;
	ulong m_dwFractional;  // 2 packets of 32bit，one is integer part,the other is frantional part 
}NTPTIMEPACKET;

//General functions

struct tm NtpTimePacket2SystemTime(NTPTIMEPACKET pNtpTime);
double NtpTimePacket2Double(NTPTIMEPACKET pNtpTime);
NTPTIMEPACKET Double2CNtpTimePacket(double f);
NTPTIMEPACKET Systemtime2CNtpTimePacket(struct tm st);
NTPTIMEPACKET MyGetCurrentTime();
NTPTIMEPACKET NewTime(double nOffset);

ulong Ms2NtpFraction(u16_t wMilliSeconds);
u16_t NtpFraction2Ms(ulong dwFraction);
double NtpFraction2Second(ulong dwFraction);
long GetJulianDay(u16_t Year, u16_t Month, u16_t Day);

NTPTIMEPACKET host2network(NTPTIMEPACKET ntp);
NTPTIMEPACKET network2host(NTPTIMEPACKET packet);

//////////////////////////////////////////////////////////////////////////

int send_sntp_packet(char* pszBuf, int nBuf);
int is_readible(int *bReadible);
int rcv_sntp_packet(char* pszBuf, int nBuf);
void close_socket();
int  set_client_time(NTPTIMEPACKET * NewTime);

//////////////////////////////////////////////////////////////////////////
void set_last_error(ulong dwerror);
long  get_last_error();

#endif 
