#ifndef __STDEFS_H__
#define __STDEFS_H__

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y) ?(x):(y))
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y) ?(x):(y))
#endif

#ifndef ABS
#define ABS(x)   ((x)<0   ?(-(x)):(x))
#endif

#ifndef SGN
#define SGN(x)   ((x)<0   ?(-1):((x)==0?(0):(1)))
#endif

typedef short          HWORD;
typedef unsigned short UHWORD;
typedef int            WORD;
typedef unsigned int   UWORD;

#if defined(WIN32)
    typedef __int64 int64;
    typedef unsigned __int64 uint64;
#else
    typedef long long int64;
    typedef unsigned long long uint64;
#endif

#define MAX_HWORD (32767)
#define MIN_HWORD (-32768)

#endif // __STDEFS_H__

