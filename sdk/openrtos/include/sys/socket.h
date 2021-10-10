#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Protocol families.  */
#define	PF_LOCAL	1	/* Local to host (pipes and file-domain).  */
#define	PF_UNIX		PF_LOCAL /* Old BSD name for PF_LOCAL.  */
#define	PF_INET6	10	/* IP version 6.  */
#define	PF_NETLINK	16
#define	PF_ROUTE	PF_NETLINK /* Alias to emulate 4.4BSD.  */


/* Address families.  */
#define	AF_LOCAL	PF_LOCAL
#define	AF_UNIX		PF_UNIX
#define	AF_INET6	PF_INET6
#define	AF_ROUTE	PF_ROUTE
#define	AF_NETLINK	PF_NETLINK


#define	MSG_DONTROUTE	0x4     /* send without using routing tables */
#define	MSG_CTRUNC	    0x20    /* control data lost before delivery */
#define MSG_MCAST       0x0200  /* this message was rcvd using link-level mcast */

/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SOL_IP		0

/*
 * Maximum queue length specifiable by listen.
 */
#ifndef	SOMAXCONN
#define	SOMAXCONN	128
#endif

/*
 * Message header for recvmsg and sendmsg calls.
 * Used value-result for recvmsg, value only for sendmsg.
 */
struct msghdr {
	void		*msg_name;		/* optional address */
	socklen_t	 msg_namelen;		/* size of address */
	struct iovec	*msg_iov;		/* scatter/gather array */
	int		 msg_iovlen;		/* # elements in msg_iov */
	void		*msg_control;		/* ancillary data, see below */
	socklen_t	 msg_controllen;	/* ancillary data buffer len */
	int		 msg_flags;		/* flags on received message */
};

/*
 * Header for ancillary data objects in msg_control buffer.
 * Used for additional information with/about a datagram
 * not expressible by flags.  The format is a sequence
 * of message elements headed by cmsghdr structures.
 */
struct cmsghdr {
	socklen_t	cmsg_len;		/* data byte count, including hdr */
	int		cmsg_level;		/* originating protocol */
	int		cmsg_type;		/* protocol-specific type */
/* followed by	unsigned char  cmsg_data[]; */
};

/* given pointer to struct cmsghdr, return pointer to data */
#define	CMSG_DATA(cmsg)		((unsigned char *)(cmsg) + \
				 _ALIGN(sizeof(struct cmsghdr)))

/* given pointer to struct cmsghdr, return pointer to next cmsghdr */
#define	CMSG_NXTHDR(mhdr, cmsg)	\
	(((caddr_t)(cmsg) + _ALIGN((cmsg)->cmsg_len) + \
	  _ALIGN(sizeof(struct cmsghdr)) > \
	    (caddr_t)(mhdr)->msg_control + (mhdr)->msg_controllen) ? \
	    (struct cmsghdr *)NULL : \
	    (struct cmsghdr *)((caddr_t)(cmsg) + _ALIGN((cmsg)->cmsg_len)))

#define	CMSG_FIRSTHDR(mhdr)	((struct cmsghdr *)(mhdr)->msg_control)

/* RFC 2292 additions */
	
#define	CMSG_SPACE(l)		(_ALIGN(sizeof(struct cmsghdr)) + _ALIGN(l))
#define	CMSG_LEN(l)		(_ALIGN(sizeof(struct cmsghdr)) + (l))

#define	SCM_RIGHTS	0x01		/* access rights (array of int) */

struct sockaddr_storage {
  u8_t ss_len;
  u8_t ss_family;
  char ss_data[14];
};

static inline int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    return lwip_accept(s, addr, addrlen);
}

static inline int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
    return lwip_bind(s, name, namelen);
}

#define shutdown(a,b)         lwip_shutdown(a,b)
#define closesocket(s)        lwip_close(s)

static inline int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
    return lwip_connect(s, name, namelen);
}

#define getsockname(a,b,c)    lwip_getsockname(a,b,c)
#define getpeername(a,b,c)    lwip_getpeername(a,b,c)
#define setsockopt(a,b,c,d,e) lwip_setsockopt(a,b,c,d,e)
#define getsockopt(a,b,c,d,e) lwip_getsockopt(a,b,c,d,e)
#define listen(a,b)           lwip_listen(a,b)

static inline int recv(int s, void *mem, size_t len, int flags)
{
    return lwip_recv(s, mem, len, flags);
}

#define recvfrom(a,b,c,d,e,f) lwip_recvfrom(a,b,c,d,e,f)

static inline int send(int s, const void *dataptr, size_t size, int flags)
{
	return lwip_send(s, dataptr, size, flags);
}

#define sendto(a,b,c,d,e,f)   lwip_sendto(a,b,c,d,e,f)
#define socket(a,b,c)         lwip_socket(a,b,c)
#define select(a,b,c,d,e)     lwip_select(a,b,c,d,e)
#define ioctlsocket(a,b,c)    lwip_ioctl(a,b,c)

ssize_t recvmsg(int s, struct msghdr *msg, int flags);
ssize_t sendmsg(int s, const struct msghdr *msg, int flags);
int socketpair(int __domain, int __type, int __protocol, int *__socket_vec);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SOCKET_H */
