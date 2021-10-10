#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <linux/os.h>

#ifdef __cplusplus
extern "C"
{
#endif


struct device_type {
	const char *name;
};

struct device {
    char *name;
};

static inline void device_initialize(struct device *dev)
{
}

static inline int dev_to_node(struct device *dev)
{
	return -1;
}
static inline void set_dev_node(struct device *dev, int node)
{
}

static inline void device_lock(struct device *dev)
{
}

static inline int device_trylock(struct device *dev)
{
}

static inline void device_unlock(struct device *dev)
{
}

#define device_del(x)   do {} while(0)
#define put_device(x)   do {} while(0)
#ifndef get_device
#define get_device(x)	(x)
#endif




#ifdef __cplusplus
}
#endif

#endif // _DEVICE_H_
