#ifndef ITP_ASM_UNISTD_H
#define ITP_ASM_UNISTD_H

#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define __NR_gettid getpid

#ifdef __cplusplus
}
#endif

#endif // ITP_ASM_UNISTD_H
