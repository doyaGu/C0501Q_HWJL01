#ifndef _NETDB_H
#define _NETDB_H

#include <sys/types.h>
#include "lwip/netdb.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define	NI_MAXHOST	1025
#define	NI_NUMERICHOST	0x00000002
#define	NI_NUMERICSERV	0x00000008
#define	NETDB_INTERNAL	-1	/* see errno */
#define	AI_PASSIVE	0x00000001 /* get address to use bind() */

struct servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

#define gethostbyname(name) lwip_gethostbyname(name)
#define gethostbyname_r(name, ret, buf, buflen, result, h_errnop) \
       lwip_gethostbyname_r(name, ret, buf, buflen, result, h_errnop)
#define freeaddrinfo(addrinfo) lwip_freeaddrinfo(addrinfo)
#define getaddrinfo(nodname, servname, hints, res) \
       lwip_getaddrinfo(nodname, servname, hints, res)

int getnameinfo(const struct sockaddr *sa, socklen_t addrlen, char *host,
	     socklen_t hostlen, char *serv, socklen_t servlen, unsigned int flags);
const char *gai_strerror(int ecode);
struct servent *getservbyport(int port, const char *proto);

#ifdef __cplusplus
}
#endif

#endif /* _NETDB_H */
