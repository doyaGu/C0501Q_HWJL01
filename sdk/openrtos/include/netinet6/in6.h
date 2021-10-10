#ifndef ITP_NETINET6_IN6_H
#define ITP_NETINET6_IN6_H

#include <sys/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * IPv6 address
 */
struct in6_addr {
	union {
		uint8_t   __u6_addr8[16];
		uint16_t  __u6_addr16[8];
		uint32_t  __u6_addr32[4];
	} __u6_addr;			/* 128-bit IP6 address */
};

#define s6_addr   __u6_addr.__u6_addr8

/*
 * Unicast Scope
 * Note that we must check topmost 10 bits only, not 16 bits (see RFC2373).
 */
#define IN6_IS_ADDR_LINKLOCAL(a)	\
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0x80))
#define IN6_IS_ADDR_SITELOCAL(a)	\
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0xc0))

/*
 * Options for use with [gs]etsockopt at the IPV6 level.
 * First word of comment is data type; bool is stored in int.
 */
#define IPV6_SOCKOPT_RESERVED1	3  /* reserved for future use */
#define IPV6_UNICAST_HOPS	4  /* int; IP6 hops */
#define IPV6_MULTICAST_IF	9  /* u_char; set/get IP6 multicast i/f  */
#define IPV6_MULTICAST_HOPS	10 /* u_char; set/get IP6 multicast hops */
#define IPV6_MULTICAST_LOOP	11 /* u_char; set/get IP6 multicast loopback */
#define IPV6_JOIN_GROUP		12 /* ip6_mreq; join a group membership */
#define IPV6_LEAVE_GROUP	13 /* ip6_mreq; leave a group membership */
#define IPV6_PORTRANGE		14 /* int; range to choose for unspec port */
#define ICMP6_FILTER		18 /* icmp6_filter; icmp6 filter */
/* RFC2292 options */
#define IPV6_PKTINFO		19 /* bool; send/recv if, src/dst addr */
#define IPV6_HOPLIMIT		20 /* bool; hop limit */
#define IPV6_NEXTHOP		21 /* bool; next hop addr */
#define IPV6_HOPOPTS		22 /* bool; hop-by-hop option */
#define IPV6_DSTOPTS		23 /* bool; destination option */
#define IPV6_RTHDR		24 /* bool; routing header */
#define IPV6_PKTOPTIONS		25 /* buf/cmsghdr; set/get IPv6 options */

#define IPV6_CHECKSUM		26 /* int; checksum offset for raw socket */
#define IPV6_V6ONLY		27 /* bool; only bind INET6 at wildcard bind */

#define INET6_ADDRSTRLEN	46

/*
 * Socket address for IPv6
 */
struct sockaddr_in6 {
	uint8_t	sin6_family;	/* AF_INET6 (sa_family_t) */
	uint16_t	sin6_port;	/* Transport layer port # (in_port_t)*/
	uint32_t	sin6_flowinfo;	/* IP6 flow information */
	struct in6_addr	sin6_addr;	/* IP6 address */
	uint32_t	sin6_scope_id;	/* scope zone index */
};

#define IN6ADDR_ANY_INIT	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
#define IN6ADDR_LOOPBACK_INIT	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

#ifdef __cplusplus
}
#endif

#endif /* ITP_NETINET6_IN6_H */
