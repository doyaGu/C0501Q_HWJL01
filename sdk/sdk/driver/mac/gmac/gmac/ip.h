
#ifndef MAC_IP_H
#define MAC_IP_H

#include "skb.h"

#ifdef __cplusplus
extern "C" {
#endif

struct iphdr {
	u8	reserved1[2];
	__be16	tot_len;
	u8	reserved2[5];
	u8	protocol;
	u16	check;
	__be32	saddr;
	__be32	daddr;
	/*The options start here. */
};

static inline struct iphdr *ip_hdr(const struct sk_buff *skb)
{
	return (struct iphdr *)(skb->data+ETH_HLEN);
}

#if 0 /* in lwip/sockets.h  */
enum {
	  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
#define IPPROTO_IP		IPPROTO_IP
	  IPPROTO_ICMP = 1, 	/* Internet Control Message Protocol	*/
#define IPPROTO_ICMP		IPPROTO_ICMP
	  IPPROTO_IGMP = 2, 	/* Internet Group Management Protocol	*/
#define IPPROTO_IGMP		IPPROTO_IGMP
	  IPPROTO_IPIP = 4, 	/* IPIP tunnels (older KA9Q tunnels use 94) */
#define IPPROTO_IPIP		IPPROTO_IPIP
	  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
#define IPPROTO_TCP		IPPROTO_TCP
	  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
#define IPPROTO_EGP		IPPROTO_EGP
	  IPPROTO_PUP = 12, 	/* PUP protocol 			*/
#define IPPROTO_PUP		IPPROTO_PUP
	  IPPROTO_UDP = 17, 	/* User Datagram Protocol		*/
#define IPPROTO_UDP		IPPROTO_UDP
};
#endif


#ifdef __cplusplus
}
#endif

#endif //MAC_IP_H
