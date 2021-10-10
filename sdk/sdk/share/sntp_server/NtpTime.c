#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include "NtpTime.h"
#include "sntp.h"

#define FALSE 0 

/************************************************************************/
/*  函数原型：get_last_error()a
    参数类型：
    返回值：  
    基本原理： 调用各个平台上的报错函数，获取错误代码       */
/************************************************************************/
long  get_last_error(void)
{
	return errno;   
}
/************************************************************************/
/*  函数原型：set_last_error()
    参数类型：
    返回值：  
    基本原理： 根据错误代码设置出错后操作       */
/************************************************************************/
void set_last_error(ulong dwerror)
{
   errno = dwerror;
   return ;
}
/************************************************************************/
/*  函数原型：NTPTIMEPACKET host2network(NTPTIMEPACKET ntp) 
    参数类型： NTPTIMEPACKET ntp  本地字节序的时间包 
    返回值：   ntp：时间的网络字节序
    基本原理： 在客户端发包之前将时间的本地字节序转换为网络字节序，网络字节序表示整数时，最高位字节在前。                                                    */
/************************************************************************/
NTPTIMEPACKET host2network(NTPTIMEPACKET ntp)  //transform the host byte order to network byte order,before filling the packet
{
	ntp.m_dwInteger = htonl(ntp.m_dwInteger);
	ntp.m_dwFractional = htonl(ntp.m_dwFractional);
	return ntp;
}

/************************************************************************/
/*  函数原型：NTPTIMEPACKET network2host(NTPTIMEPACKET packet)
    参数类型：NTPTIMEPACKET packet  网络字节序的时间包 
    返回值：  packet：时间的网络字节序
    基本原理： 在客户端收包后将时间的网络字节序转换为本地字节序。       */
/************************************************************************/
NTPTIMEPACKET network2host(NTPTIMEPACKET packet) //transform the network byte order to host byte order,after receiving the packet
{ 
	NTPTIMEPACKET hosttime;
	
	hosttime.m_dwInteger = ntohl(packet.m_dwInteger);
    hosttime.m_dwFractional = ntohl(packet.m_dwFractional);

	return hosttime;
}

/************************************************************************/
/*  函数原型：NTPTIMEPACKET Systemtime2CNtpTimePacket(struct tm st)
    参数类型：struct tm st  从本机上获取的系统时间，由年月日时分秒微妙构成。
    返回值：  t：2个32bit的数据包
    基本原理：首先将系统时间的年月日转换成为1个32bit的整数,减去1900年的时间，
              获得秒数赋给包的整数部分，微妙转换为小数部分。            */
/************************************************************************/

NTPTIMEPACKET Systemtime2CNtpTimePacket(struct tm st)  
{  
	NTPTIMEPACKET t;
	//Currently this function only operates correctly in 
	//the 1900 - 2036 primary epoch defined by NTP
 
	long Seconds;
	long JD = GetJulianDay(st.tm_year + 1900, st.tm_mon + 1, st.tm_mday);
	JD -= JAN_1ST_1900;   

	assert(JD >= 0);  
	Seconds = JD;
	Seconds = (Seconds * 24) + st.tm_hour;
	Seconds = (Seconds * 60) + st.tm_min;
	Seconds = (Seconds * 60) + st.tm_sec;
	assert(Seconds <= 0xFFFFFFFF); //NTP Only supports up to 2036
	t.m_dwInteger=Seconds;
	t.m_dwFractional=Ms2NtpFraction(0);
    //t.m_dwFractional=st.wMilliseconds/NTP_TO_SECOND;  //16bit的ms值
	
	return t;
}

/************************************************************************/
/*  函数原型：struct tm NtpTimePacket2SystemTime(NTPTIMEPACKET pNtpTime)
    参数类型：NTPTIMEPACKET pNtpTime  2个32bit组成的时间包 
    返回值：  st：系统时间
    基本原理： 时间包转换成年月日时分秒的系统时间。                     */
/************************************************************************/

struct tm NtpTimePacket2SystemTime(NTPTIMEPACKET pNtpTime) 
{
	//Currently this function only operates correctly in 
	//the 1900 - 2036 primary epoch defined by NTP

	struct tm st;
    ulong s = pNtpTime.m_dwInteger;  
	long      JD;
	long      d;
	long      m;
	long      j;
	long      y;
	st.tm_sec = (u16_t)(s % 60);
	s /= 60;
	st.tm_min = (u16_t)(s % 60);
	s /= 60;

	st.tm_hour = (u16_t)(s % 24);
	s /= 24;
	JD = s + JAN_1ST_1900;
	st.tm_wday = (u16_t)((JD + 1) % 7);

    j = JD - 1721119;
    y = (4 * j - 1) / 146097;
    j = 4 * j - 1 - 146097 * y;
    d = j / 4;
    j = (4 * d + 3) / 1461;
    d = 4 * d + 3 - 1461 * j;
    d = (d + 4) / 4;
    m = (5 * d - 3) / 153; 
    d = 5 * d - 3 - 153 * m;
    d = (d + 5) / 5;
    y = 100 * y + j;
    if (m < 10)
        m = m + 3;
    else
    {
        m = m - 9;
        y = y + 1;
    }

  	st.tm_year = (u16_t) y - 1900;
	st.tm_mon= (u16_t) m - 1;
	st.tm_mday = (u16_t) d;
     
	return st;
}

/************************************************************************/
/*  函数原型：GetJulianDay(u16_t Year, u16_t Month, u16_t Day)
    参数类型：u16_t Year, u16_t Month, u16_t Day：系统时间中用的16bit无符号整数表示年月日
    返回值：  j：1个32bit的整数
    基本原理： 年月日3个数表示成1个32bit的天数                          */
/************************************************************************/

long GetJulianDay(u16_t Year, u16_t Month, u16_t Day) 
{
	long    c;
	long    ya;
	long    j;
	long    y = (long) Year;
	long    m = (long) Month;
	long    d = (long) Day;

    if (m > 2)
        m = m - 3;
    else
    {
        m = m + 9;
        y = y - 1;
    }
    c = y / 100;
    ya = y - 100 * c;
    j = (146097L * c) / 4 + (1461L * ya) / 4 + (153L * m + 2) / 5 + d + 1721119L;
    return j;
}

/************************************************************************/
/*  函数原型：NtpTimePacket2Double(NTPTIMEPACKET pNtpTime)
    参数类型：NTPTIMEPACKET pNtpTime：2个32bit的包
    返回值：  t：双精度时间
    基本原理： 2个32bit的包转换成双精度时间，由整数部分左移32bit后加上分数部分转换成的秒数组成*/
/************************************************************************/
double NtpTimePacket2Double(NTPTIMEPACKET pNtpTime) 
{
	double t;  
	t = (pNtpTime.m_dwInteger)+NtpFraction2Second (pNtpTime.m_dwFractional);
	return t;
}

/************************************************************************/
/*  函数原型：Double2CNtpTimePacket(double dt)
    参数类型：double dt：双精度时间
    返回值：  t：2个32bit的包
    基本原理： 双精度时间转换成2个32bit的包，将双精度数强制转换成32bit无符号长整型，
               构成包的整数部分，将双精度数的后32bit转换成包的小数部分。*/
/************************************************************************/
NTPTIMEPACKET Double2CNtpTimePacket(double dt) //transform double packet Round trip delay and c-s clock offset to 2 packets of 32 bit
{                                          
	NTPTIMEPACKET  t;
	t.m_dwInteger= (ulong) (dt);    //force dt to 1 integer of 32 bit，drop the fractional part
	dt-=(uint64_t)t.m_dwInteger<<32;
	t.m_dwFractional=(ulong) (dt /NTP_TO_SECOND);    
	return t;
}

/************************************************************************/
/*  函数原型：Ms2NtpFraction(u16_t wMilliSeconds)
    参数类型：u16_t wMilliSeconds：16bit无符号整数表示微秒
    返回值：  m_Ms2NTP：包的分数部分
    基本原理： 由微秒到m_Ms2NTP数组中查找对应的32bit数据。              */
/************************************************************************/
ulong Ms2NtpFraction(u16_t wMilliSeconds)
{
    assert(wMilliSeconds < 1000);
    return m_Ms2NTP[wMilliSeconds];   
}

/************************************************************************/
/*  函数原型：NtpFraction2Ms(ulong dwFraction)
    参数类型：ulong dwFraction：时间包的32bit分数部分
    返回值：  微秒值
    基本原理： 由分数部分的值转换成微秒值                               */
/************************************************************************/   
u16_t NtpFraction2Ms(ulong dwFraction)
{
	return (u16_t) ((((double) dwFraction) * NTP_FRACTIONAL_TO_MS) + 0.5);
} 

/************************************************************************/
/*  函数原型：NtpFraction2Second(ulong dwFraction)
    参数类型：ulong dwFraction：时间包的32bit分数部分
    返回值：  秒值，0.xxxxxx s
    基本原理： 由分数部分的值转换成秒值                                 */
/************************************************************************/ 
double NtpFraction2Second(ulong dwFraction)  
{
    double   d = (double) dwFraction;  //这里就有问题，0.40600000005355102--1743756722      
    d *= NTP_TO_SECOND;  //NTP_TO_SECOND十进制的1代表多少秒，乘上十进制的值就得到秒值。0.xxxs
	return d;
}
 
/************************************************************************/
/*  函数原型：NTPTIMEPACKET myGetCurrentTime()
    参数类型：无
    返回值：  时间包
    基本原理：                                                          */
/************************************************************************/ 
NTPTIMEPACKET MyGetCurrentTime(void)
{
    struct timeval tv;
    struct tm st;

    gettimeofday(&tv, NULL);
    st = *gmtime(&tv.tv_sec);

    return Systemtime2CNtpTimePacket(st);
}

/************************************************************************/
/*  函数原型：set_client_time(NTPTIMEPACKET *NewTime)
    参数类型：NTPTIMEPACKET *NewTime：指向时间包类型的指针
    返回值：  bSuccess：FALSE表示获取新时间失败，!FALSE则表示成功
    基本原理：                                                          */
/************************************************************************/  
int  set_client_time(NTPTIMEPACKET *NewTime)   
{
	int bSuccess = FALSE;
	struct tm st =NtpTimePacket2SystemTime(*NewTime);
    struct timeval systime;
    
    systime.tv_sec = mktime(&st);
    systime.tv_usec = 0;

    bSuccess = !settimeofday(&systime, NULL);
	if (!bSuccess)
	     printf("Failed in call to set the system time\n");

    return bSuccess;
}

/************************************************************************/
/*  函数原型：NTPTIMEPACKET NewTime()
    参数类型： 
    返回值：  time: 更新后的时间，2个32bit数
    基本原理：将客户端当前时间变换成双精度时间值加上计算出的偏移量即得准确时间                                                          */
/************************************************************************/
NTPTIMEPACKET NewTime(double nOffset)
{    
	NTPTIMEPACKET time;
	double a;
	NTPTIMEPACKET t=MyGetCurrentTime();
	a=NtpTimePacket2Double(t) +  nOffset;
	
	time=Double2CNtpTimePacket(a);  //问题在这个函数处
	return time;
}




 
