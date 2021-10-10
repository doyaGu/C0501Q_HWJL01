#ifndef ITP_DIRENT_H
#define ITP_DIRENT_H

#include_next <dirent.h>

#ifdef __cplusplus
extern "C"
{
#endif

int alphasort (const struct dirent **__a, const struct dirent **__b);

#ifdef __cplusplus
}
#endif

#endif // ITP_DIRENT_H
