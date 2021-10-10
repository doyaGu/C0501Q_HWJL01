#ifndef _MSUDP_H_
#define _MSUDP_H_

#include <mediastreamer2/msfilter.h>

typedef enum{
	AUDIO_INPUT = 0,
	AUDIO_OUTPUT,
	VIDEO_INPUT,
	VIDEO_OUTPUT,
}MEDIA_TYPE;

typedef struct udp_config{
	MEDIA_TYPE c_type;
	struct ip_mreq mreq;
	unsigned char group_ip[16];
	unsigned char remote_ip[16];
	unsigned short remote_port;
	int enable_multicast;
	int cur_socket;
	MSFilter *cur_filter;
}udp_config_t;


#define MS_UDP_RECV_SET_PARA             MS_FILTER_METHOD(MS_UDP_RECV_ID,0,udp_config_t*)

#define MS_UDP_RECV_GET_SOCKET           MS_FILTER_METHOD(MS_UDP_RECV_ID,1,int*)

#define MS_UDP_SEND_SET_PARA             MS_FILTER_METHOD(MS_UDP_SEND_ID,0,udp_config_t*)

#define MS_UDP_SEND_GET_SOCKET           MS_FILTER_METHOD(MS_UDP_SEND_ID,1,int*)

extern MSFilterDesc ms_udp_send_desc;
extern MSFilterDesc ms_udp_recv_desc;

#endif
