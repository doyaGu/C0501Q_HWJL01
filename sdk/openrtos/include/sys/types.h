#ifndef ITP_SYS_TYPES_H
#define ITP_SYS_TYPES_H

#include_next <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef unsigned long long u_int64_t;
typedef unsigned long id_t;

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_TYPES_H
