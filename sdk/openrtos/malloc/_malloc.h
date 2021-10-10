#ifndef __MALLOC_H__
#define __MALLOC_H__

#ifdef calloc
# undef calloc
#endif

#ifdef realloc
# undef realloc
#endif

#ifdef malloc
# undef malloc
#endif

#ifdef free
# undef free
#endif

#ifdef memalign
# undef memalign
#endif

#endif // __MALLOC_H__
