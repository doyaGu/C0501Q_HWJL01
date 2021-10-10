/*
 * sys/mman.h
 */

#ifndef __SYS_MMAN_H__
#define __SYS_MMAN_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROT_NONE       0
#define PROT_READ       1
#define PROT_WRITE      2
#define PROT_EXEC       4

#define MAP_FILE        0
#define MAP_SHARED      1
#define MAP_PRIVATE     2
#define MAP_TYPE        0xf
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_ANON        MAP_ANONYMOUS

//FIXME This block is from /usr/include/bits/mman.h on an Ubuntu 11.04 machine.
/* These are Linux-specific.  */
//#ifdef __USE_MISC
# define MAP_GROWSDOWN  0x00100   /* Stack-like segment.  */
# define MAP_DENYWRITE  0x00800   /* ETXTBSY */
# define MAP_EXECUTABLE 0x01000   /* Mark it as an executable.  */
# define MAP_LOCKED 0x02000   /* Lock the mapping.  */
# define MAP_NORESERVE  0x04000   /* Don't check for reservations.  */
# define MAP_POPULATE 0x08000   /* Populate (prefault) pagetables.  */
# define MAP_NONBLOCK 0x10000   /* Do not block on IO.  */
# define MAP_STACK  0x20000   /* Allocation is for a stack.  */
# define MAP_HUGETLB  0x40000   /* Create huge page mapping.  */
//#endif


#define MAP_FAILED      ((void *)-1)

/* Flags for msync. */
#define MS_ASYNC        1
#define MS_SYNC         2
#define MS_INVALIDATE   4

void*   mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int     munmap(void *addr, size_t len);
int     mprotect(void *addr, size_t len, int prot);
int     msync(void *addr, size_t len, int flags);
int     mlock(const void *addr, size_t len);
int     munlock(const void *addr, size_t len);

#ifdef __cplusplus
};
#endif

#endif /*  __SYS_MMAN_H__ */

