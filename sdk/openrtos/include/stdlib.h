#ifndef ITP_STDLIB_H
#define ITP_STDLIB_H

#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef free
#undef free
#endif

#ifdef memalign
#undef memalign
#endif

#include_next <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

static inline void srandom(unsigned int seed)
{
    srand(seed);
}

static inline int random(void)
{
    return rand();
}

#ifdef CFG_DBG_MEMLEAK

extern void *dbg_malloc(size_t size, char *file_name, unsigned long line_number);
extern void *dbg_realloc(void *ptr, size_t size, char *file_name, unsigned long line_number);
extern void *dbg_calloc(size_t num, size_t size, char *file_name, unsigned long line_number);
extern void dbg_free(void *ptr, char *file_name, unsigned long line_number);
extern void *dbg_memalign(size_t align, size_t size, char *file_name, unsigned long line_number);

#ifndef malloc
#define malloc(s) dbg_malloc(s, __FILE__, __LINE__)
#endif

#ifndef realloc
#define realloc(p, s) dbg_realloc(p, s, __FILE__, __LINE__)
#endif

#ifndef calloc
#define calloc(n, s) dbg_calloc(n, s, __FILE__, __LINE__)
#endif

#ifndef free
#define free(p) dbg_free(p, __FILE__, __LINE__)
#endif

#ifndef memalign
#define memalign(a, s) dbg_memalign(a, s, __FILE__, __LINE__)
#endif

#elif defined(CFG_DBG_RMALLOC)

#define FUNCTIONIZE(a,b)  a(b)
#define STRINGIZE(a)      #a
#define INT2STRING(i)     FUNCTIONIZE(STRINGIZE,i)
#define RM_FILE_POS       __FILE__ ":" INT2STRING(__LINE__)

void *Rmalloc(size_t size, const char *file);
void *Rcalloc(size_t nelem, size_t size, const char *file);
void *Rrealloc(void *p, size_t size, const char *file);
void  Rfree(void *p, const char *file);
void *Rmemalign(size_t align, size_t size, const char *file);

#  ifdef malloc
#    undef malloc
#  endif
#  define malloc(s)       Rmalloc((s), RM_FILE_POS)

#  ifdef calloc
#    undef calloc
#  endif
#  define calloc(n,s)     Rcalloc((n), (s), RM_FILE_POS)

#  ifdef realloc
#    undef realloc
#  endif
#  define realloc(p,s)    Rrealloc((p), (s), RM_FILE_POS)

#  ifdef free
#    undef free
#  endif
#  define free(p)         Rfree((p), RM_FILE_POS)

#  ifdef memalign
#    undef memalign
#  endif
#  define memalign(a, s)  Rmemalign((a), (s), RM_FILE_POS)

#endif // CFG_DBG_MEMLEAK

#ifdef __cplusplus
}
#endif

#endif // ITP_STDLIB_H
