/*
 * Copyright (c) 2009 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * ITE atomic Library.
 *
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef ITE_ATOMIC_H
#define ITE_ATOMIC_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct { 
    volatile int counter; 
} atomic_t;


#ifndef atomic_read
#define atomic_read(v)	(*(volatile int *)&(v)->counter)
#endif
#ifndef atomic_set
#define atomic_set(v,i)	(((v)->counter) = (i))
#endif

#ifndef atomic_add_return
static inline int atomic_add_return(int i, atomic_t *v)
{
	int temp;

	ithEnterCritical();
	temp = v->counter;
	temp += i;
	v->counter = temp;
    ithExitCritical();

	return temp;
}
#endif

#ifndef atomic_sub_return
static inline int atomic_sub_return(int i, atomic_t *v)
{
	int temp;

	ithEnterCritical();
	temp = v->counter;
	temp -= i;
	v->counter = temp;
	ithExitCritical();

	return temp;
}
#endif

static inline int atomic_add_negative(int i, atomic_t *v)
{
	return atomic_add_return(i, v) < 0;
}

static inline void atomic_add(int i, atomic_t *v)
{
	atomic_add_return(i, v);
}

static inline void atomic_sub(int i, atomic_t *v)
{
	atomic_sub_return(i, v);
}

static inline void atomic_inc(atomic_t *v)
{
	atomic_add_return(1, v);
}

static inline void atomic_dec(atomic_t *v)
{
	atomic_sub_return(1, v);
}

#define atomic_dec_return(v)		atomic_sub_return(1, (v))
#define atomic_inc_return(v)		atomic_add_return(1, (v))

#define atomic_sub_and_test(i, v)	(atomic_sub_return((i), (v)) == 0)
#define atomic_dec_and_test(v)		(atomic_dec_return(v) == 0)
#define atomic_inc_and_test(v)		(atomic_inc_return(v) == 0)



#ifdef __cplusplus
}
#endif

#endif // ITE_ATOMIC_H
