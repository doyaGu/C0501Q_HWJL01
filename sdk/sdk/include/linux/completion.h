#ifndef _COMPLETION_H_
#define _COMPLETION_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "ite/itp.h"

struct completion {
	unsigned int done;
	sem_t wait;
};

/**
 * init_completion - Initialize a dynamically allocated completion
 * @x:  completion structure that is to be initialized
 *
 * This inline function will initialize a dynamically created completion
 * structure.
 */
static inline void init_completion(struct completion *x)
{
	x->done = 0;
    sem_init(&x->wait, 0, 0);
}

/**
 * reinit_completion - reinitialize a completion structure
 * @x:  pointer to completion structure that is to be reinitialized
 *
 * This inline function should be used to reinitialize a completion structure so it can
 * be reused. This is especially important after complete_all() is used.
 */
static inline void reinit_completion(struct completion *x)
{
	x->done = 0;
}

/* !!!! IMPORTMENT !!!! Need add by yourself for memory leak */
static inline void destroy_completion(struct completion *x)
{
    sem_destroy(&x->wait);
}

extern unsigned long wait_for_completion_timeout(struct completion *x,
						   unsigned long timeout);

extern void complete(struct completion *x);


#ifdef __cplusplus
}
#endif

#endif // _COMPLETION_H_
