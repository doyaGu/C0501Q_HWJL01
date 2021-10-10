#ifndef _SYS_RESOURCE_H_
#define _SYS_RESOURCE_H_

#include_next <sys/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PRIO_PROCESS	0

int setpriority (int which, id_t who, int value);

#ifdef __cplusplus
}
#endif

#endif

