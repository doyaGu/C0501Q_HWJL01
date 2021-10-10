#ifndef ITE_WAIT_H
#define ITE_WAIT_H

#include "ite/ith.h"
#include "ite/itp.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct __wait_queue_head {
	volatile int is_wait; 
	sem_t sem;
};
typedef struct __wait_queue_head wait_queue_head_t;


#define init_waitqueue_head(q)	    do { (q)->is_wait = 0; } while (0)



#define wake_up(x)      \
    do {        \
        if ((x)->is_wait)  \
            sem_post(&(x)->sem); \
    } while(0); 

#define WQ_SLEEP_TIME   10  // ms
/**
 * wait_event_timeout - sleep until a condition gets true or a timeout elapses
 * @wq: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 * @timeout: timeout, in jiffies
 *
 * The process is put to sleep until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * The function returns 0 if the @timeout elapsed, or the remaining
 * jiffies (at least 1) if the @condition evaluated to %true before
 * the @timeout elapsed.
 */
#define wait_event_timeout(wq, condition, timeout)			\
({									\
	long __ret = timeout;						\
    sem_init(&wq.sem, 0, 0);    \
    wq.is_wait = 1;    \
    do {    \
        if (!(condition))	{			\
            if (itpSemWaitTimeout(&wq.sem, WQ_SLEEP_TIME)) {	\
                __ret -= WQ_SLEEP_TIME;  \
                if (__ret<=0) { __ret=0; ithPrintf("%s:%d wait_event_timeout(%d) \n", __FILE__, __LINE__, timeout); break; } \
            }                                       \
        } else                                          \
            break;  \
    } while(1); \
    wq.is_wait = 0;    \
    sem_destroy(&wq.sem);   \
	__ret;								\
})

#define wait_event(wq, condition)   wait_event_timeout(wq, condition, 10*1000*1000)


#ifdef __cplusplus
}
#endif

#endif // ITE_WAIT_H
