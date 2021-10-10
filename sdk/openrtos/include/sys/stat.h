#ifndef ITP_SYS_STAT_H
#define ITP_SYS_STAT_H

#include_next <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

int	_EXFUN(lstat,( const char *__path, struct stat *__buf ));

#ifdef __cplusplus
}
#endif
#endif /* ITP_SYS_STAT_H */
