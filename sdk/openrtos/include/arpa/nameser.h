#ifndef _ARPA_NAMESER_H_
#define _ARPA_NAMESER_H_

#define BIND_4_COMPAT

#include <sys/types.h>
#include <sys/cdefs.h>

#define NS_INADDRSZ	4	/* IPv4 T_A */

typedef enum __ns_class {
	ns_c_in = 1,		/* Internet. */
				/* Class 2 unallocated/unsupported. */
	ns_c_chaos = 3,		/* MIT Chaos-net. */
	ns_c_hs = 4,		/* MIT Hesiod. */
	/* Query class values which do not appear in resource records */
	ns_c_none = 254,	/* for prereq. sections in update requests */
	ns_c_any = 255,		/* Wildcard match. */
	ns_c_max = 65536
} ns_class;

#include <arpa/nameser_compat.h>

#endif /* !_ARPA_NAMESER_H_ */
