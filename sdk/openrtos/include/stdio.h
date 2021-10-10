#ifndef ITP_STDIO_H
#define ITP_STDIO_H

#include_next <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

size_t itpFread(void* buf, size_t size, size_t count, FILE* fp);
size_t itpFwrite(const void* buf, size_t size, size_t count, FILE* fp);
int itpFseek(FILE* fp, long offset, int whence);
long itpFtell(FILE* fp);
int itpFflush(FILE* fp);
int itpFeof(FILE* fp);

#ifdef fread
#undef fread
#endif

#ifdef fwrite
#undef fwrite
#endif

#ifdef fseek
#undef fseek
#endif

#ifdef ftell
#undef ftell
#endif

#ifdef fflush
#undef fflush
#endif

#ifdef feof
#undef feof
#endif

#define fread itpFread
#define fwrite itpFwrite
#define fseek itpFseek
#define ftell itpFtell
#define fflush itpFflush
#define feof itpFeof

ssize_t getline(char **bufptr, size_t *n, FILE *fp);

static ssize_t getdelim(char **bufptr, size_t *n, int delim, FILE *fp)
{
    return __getdelim(bufptr, n, delim, fp);
}

#ifdef __cplusplus
}
#endif

#endif // ITP_STDIO_H
