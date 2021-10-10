#ifndef ITP_SYS_WAIT_H
#define ITP_SYS_WAIT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WNOHANG 1
#define WUNTRACED 2
#define WEXITSTATUS(w)	(((w) >> 8) & 0xff)

pid_t waitpid(pid_t, int *, int);

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_WAIT_H
