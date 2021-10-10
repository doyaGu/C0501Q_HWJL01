#ifndef ITP_SYS_CDEFS_H
#define ITP_SYS_CDEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS    }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

#define __const		const

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_CDEFS_H
