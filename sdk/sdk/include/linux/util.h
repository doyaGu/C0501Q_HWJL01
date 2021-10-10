#ifndef _LINUX_UTIL_H_
#define _LINUX_UTIL_H_

#if defined(WIN32)
#define max_t(type, x, y)  ((x) > (y) ? (x) : (y))
#define min_t(type, x, y)  ((x) < (y) ? (x) : (y))
#else
#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({			\
	type __max1 = (x);			\
	type __max2 = (y);			\
	__max1 > __max2 ? __max1: __max2; })
#endif

#ifndef ALIGN
#define ALIGN		ITH_ALIGN_UP
#endif

static inline int copy_from_user(void *des, void *src, int len) {
    memcpy(des, src, len);
    return 0;
}

static inline int copy_to_user(void *des, void *src, int len) {
    memcpy(des, src, len);
    return 0;
}



#endif // _LINUX_UTIL_H_
