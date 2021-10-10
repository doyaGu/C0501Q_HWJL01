#ifndef KTHREAD_H
#define KTHREAD_H

#include <pthread.h>
#include <linux/os.h>
#include "ite/itp.h"


struct task_struct {
    pthread_t   task;
    sem_t       sem;
    volatile uint32_t    stop;
};

static inline int wake_up_process(struct task_struct *task)
{
    return sem_post(&task->sem);
}

static inline struct task_struct *kthread_creat(void*(*threadfn)(void *data), void *data)
{
    struct task_struct *task;
    pthread_attr_t attr;
	struct sched_param param;

    task = kzalloc(sizeof(struct task_struct), 0);
    sem_init(&task->sem, 0, 0);

    pthread_attr_init(&attr);
	param.sched_priority = 2;
	pthread_attr_setschedparam(&attr, &param);
    pthread_create(&task->task, &attr, threadfn, data);

    return task;
}

/** 
 * kthread_run - create and wake a thread.
 *
 * Description: Convenient wrapper for kthread_create() followed by
 * wake_up_process().  Returns the kthread or -1.
 */
static inline struct task_struct *kthread_run(void*(*threadfn)(void *data), void *data)
{
    struct task_struct *task = kthread_creat(threadfn, data);
    //if (task)
    //    wake_up_process(task);

    return task;
}

static inline bool kthread_should_stop(struct task_struct *task)
{
    if (task)
        return task->stop;
    else
        return 0;
}

static inline int schedule_timeout(struct task_struct *task, int timeout)
{
    if (task)
        return itpSemWaitTimeout(&task->sem, timeout);
    else {
        usleep(timeout * 1000);
        return 0;
    }
}

static inline int kthread_stop(struct task_struct *task)
{
    void *res;

    task->stop = 1;
    wake_up_process(task);
    pthread_join(task->task, &res);

    sem_destroy(&task->sem);
    kfree(task);

    return *((int*)res);
}

#if defined(_WIN32)
#define kthread_creat(fn, data, fmt, ...)   kthread_creat(fn, data)
#define kthread_run(threadfn, data, namefmt, ...) kthread_run(threadfn, data)
#else
#define kthread_creat(fn, data, fmt, arg...)   kthread_creat(fn, (void*)data)
#define kthread_run(threadfn, data, namefmt, arg...) kthread_run(threadfn, (void*)data)
#endif



#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2

#define set_current_state(x)


#endif // KTHREAD_H
