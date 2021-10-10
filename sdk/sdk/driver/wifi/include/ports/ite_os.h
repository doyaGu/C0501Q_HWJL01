#ifndef	ITE_WIFI_OS_H
#define	ITE_WIFI_OS_H

#include "ite/ith.h"
#include "ite/itp.h"
#include <pthread.h>
#include "linux/spinlock.h"
#include "linux/atomic.h"

//=====================================
// mutex global variable => Irene TODO : still need moidfy
//=====================================
//extern int wifi_spinlock;
//extern int wifi_mutex;

#define in_interrupt()      (0)     /*no need in FreeRTOS*/

#define ITE_INT_SAVE()		      portENTER_CRITICAL() //James : openrtos task.h
#define ITE_INT_RESTORE()		  portEXIT_CRITICAL() //James : openrtos task.h
#if 0
//=====================================
// Spin lock related
//=====================================
typedef struct {
	volatile unsigned int lock;
} spinlock_t;

#define spin_lock_init(x)	
#define spin_lock(x)						  ITE_INT_SAVE()
#define spin_unlock(x)						  ITE_INT_RESTORE()
#define spin_lock_bh(x)                       spin_lock(x)
#define spin_unlock_bh(x)                     spin_unlock(x)
#define local_irq_save(x)					
#define local_irq_restore(x)				
#define spin_lock_irqsave(lock, flags)		do { local_irq_save(flags);       spin_lock(lock); } while (0)
#define spin_unlock_irqrestore(lock, flags)	do { spin_unlock(lock);  local_irq_restore(flags); } while (0)


// Irene TBD : atomic may no need : win32 may can't work
//=====================================
// atomic
//=====================================
typedef struct { 
    volatile int counter; 
} atomic_t;

#define atomic_set(v,i)         (((v)->counter) = (i))
#define atomic_add(i, v)        (int) atomic_add_return(i, v)
#define atomic_inc(v)           (int) atomic_add_return(1, v)
#define atomic_sub(i, v)        (int) atomic_sub_return(i, v)
#define atomic_dec(v)           (int) atomic_sub_return(1, v)
#define atomic_read(v)          ((v)->counter)

int atomic_add_return(int i, atomic_t *v);
int atomic_sub_return(int i, atomic_t *v);
int atomic_dec_and_test(atomic_t *v);
int atomic_inc_return(atomic_t *v);
int atomic_dec_return(atomic_t *v);
#endif

//=====================================
// Mutex related
//=====================================
         
#define up(x)                   pthread_mutex_unlock(x)
#define down(x)                 pthread_mutex_lock(x)
#define init_MUTEX(x)           pthread_mutex_init(x, PTHREAD_MUTEX_NORMAL)
#define init_MUTEX_LOCKED(x)    do { init_MUTEX((x));  down((x)); } while(0)
#define down_trylock(x)         pthread_mutex_trylock(x)
#define MUTEX_destroy(x)        pthread_mutex_destroy(x)

//=====================================
// tasklet related
//=====================================
//James Todo : used softirq implement

struct tasklet_struct {
	unsigned long state;
	int count;
	void (*func)(unsigned long);
	unsigned long data;
	pthread_mutex_t* mutex;
};

void start_tasklet_timer(void);

void tasklet_init(struct tasklet_struct *t,
		   void (*func)(unsigned long), unsigned long data);

void tasklet_schedule(struct tasklet_struct *t);
void tasklet_hi_schedule(struct tasklet_struct *t);

 
#define tasklet_kill(_bh)


//=====================================
// thread related
//=====================================
 static inline pid_t kernel_thread(void* proc, 
                                   void* data, 
                                   int flags)
{
     pthread_t task;
     pthread_attr_t attr;
	 struct sched_param param;
     pthread_attr_init(&attr);
     param.sched_priority = flags+1;
	 pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
     pthread_attr_setschedparam(&attr, &param);
	 //#define WIFI_STACK_SIZE   102400
     //pthread_attr_setstacksize(&attr, WIFI_STACK_SIZE);
	 pthread_create(&task, &attr, proc, data);
#ifdef _WIN32
    return 0;
#else
     return task;
#endif
}	



//#define kernel_thread(_proc, _data, _flags)     ((pid_t)pthread_create(_proc, _data, 8000, PAL_THREAD_PRIORITY_NORMAL))
#define kill_proc(_pid, _signal, _flags)        0
#if 0
// Irene TBD, check all use it
//=====================================
// wait queue related
//=====================================
typedef struct {
    int nothing;
} wait_queue_head_t;

#define DECLARE_WAIT_QUEUE_HEAD(_a)    
#define DECLARE_WAITQUEUE(_a,_b)        
#define add_wait_queue(_a,_b)
#define remove_wait_queue(_a,_b)
#define init_waitqueue_head(_a) 
#endif


// Irene TBD : completion may no need
//=====================================
// completion related
//=====================================
struct completion {
    int isFinish;
};

#define init_completion(_comp)          ((_comp)->isFinish = 0)
#define wait_for_completion(_comp)      while(1){ \
                                            mdelay(1);  \
                                            if ((_comp)->isFinish){     \
                                                (_comp)->isFinish = 0;  \
                                                break;                  \
                                            }                           \
                                        }
#define complete(_comp)                 ((_comp)->isFinish = 1)
#define complete_and_exit(_comp, _flag) ((_comp)->isFinish = 1)

unsigned long RT_wait_for_completion_timeout(struct completion *x, unsigned long timeout);
#define wait_for_completion_timeout	RT_wait_for_completion_timeout

#endif


