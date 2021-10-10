#ifndef _SYS_UN_H
#define _SYS_UN_H

#include <string.h>		/* for strlen */
#include <sys/socket.h>

/* POSIX requires only at least 100 bytes */
#define UNIX_PATH_LEN   108

struct sockaddr_un {
  sa_family_t	 sun_family;              /* address family AF_LOCAL/AF_UNIX */
  char	         sun_path[UNIX_PATH_LEN]; /* 108 bytes of socket address     */
};

/* Evaluates the actual length of `sockaddr_un' structure. */
#define SUN_LEN(p) ((size_t)(((struct sockaddr_un *) NULL)->sun_path) \
		   + strlen ((p)->sun_path))

#endif
