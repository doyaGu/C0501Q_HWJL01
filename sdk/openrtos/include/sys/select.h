#ifndef ITP_SYS_SELECT_H
#define ITP_SYS_SELECT_H

#include <sys/types.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern int itpSocketSelect(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                struct timeval *timeout);

#define select(a,b,c,d,e)     itpSocketSelect(a,b,c,d,e)

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_SELECT_H
