#ifndef ITP_LIMITS_H
#define ITP_LIMITS_H

#include_next <limits.h>
#include <sys/syslimits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PTHREAD_KEYS_MAX        8
#define SSIZE_MAX               32767

#ifdef __cplusplus
}
#endif

#endif // ITP_LIMITS_H
