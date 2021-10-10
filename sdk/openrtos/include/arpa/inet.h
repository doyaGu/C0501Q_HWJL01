#ifndef ITP_ARPA_INET_H
#define ITP_ARPA_INET_H

#include <stdint.h>
#include "lwip/inet.h"
#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef	uint32_t	in_addr_t;

const char	*inet_ntop(int af, const void *src, char *dst, socklen_t size);

#ifdef __cplusplus
}
#endif

#endif // ITP_ARPA_INET_H
