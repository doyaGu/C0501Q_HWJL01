#ifndef ITP_SYS_TIMES_H
#define ITP_SYS_TIMES_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tms {
	clock_t	tms_utime;		/* user time */
	clock_t	tms_stime;		/* system time */
	clock_t	tms_cutime;		/* user time, children */
	clock_t	tms_cstime;		/* system time, children */
};

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_TIMES_H
