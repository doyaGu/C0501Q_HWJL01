#ifndef _MG_LOCALS_H_
#define _MG_LOCALS_H_

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>  

#include <lwip/err.h>
#include <lwip/ip_addr.h>
#include <lwip/inet.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

#define LWIP_TIMEVAL_PRIVATE 0

#if LWIP_SOCKET
#include <lwip/sockets.h>
#else
    /* We really need the definitions from sockets.h. */
#undef LWIP_SOCKET
#define LWIP_SOCKET      1
#include <lwip/sockets.h>
#undef LWIP_SOCKET
#define LWIP_SOCKET      0
#endif

    typedef int sock_t;
#define INVALID_SOCKET       (-1)
#define SIZE_T_FMT           "u"
    typedef struct stat cs_stat_t;
#define DIRSEP               '/'
#define to64(x) strtoll(x, NULL, 10)
#define INT64_FMT            PRId64
#define INT64_X_FMT          PRIx64
#define __cdecl

#ifndef __func__
#define STRX(x) #x
#define STR(x)  STRX(x)
#define __func__    __FILE__ ":" STR(__LINE__)
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#define CS_PLATFORM 0
#define MG_DISABLE_CGI
#define MG_LWIP

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif