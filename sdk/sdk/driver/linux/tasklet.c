#include <linux/tasklet.h>
#include <pthread.h>
#include "ite/itp.h"

struct tasklet_ctrl {
    struct list_head    list;
    sem_t               sem;
    sem_t               thread_complete;
    atomic_t            count;
    pthread_t           thread;
};

static struct tasklet_ctrl  iteTaskletCtrl;
static bool inited = false;


static void* ite_tasklet_thread(void* arg)
{
    struct tasklet_ctrl *ctrl = (struct tasklet_ctrl *)arg;
    struct list_head *entry, *tmp;
    struct tasklet_struct *t;

    sem_post(&ctrl->thread_complete);

    while (1){
        sem_wait(&ctrl->sem);

        atomic_set(&ctrl->count, 0);
        list_for_each_safe(entry, tmp, &ctrl->list) {
            t = list_entry(entry, struct tasklet_struct, list);
            if (atomic_read(&t->count)) {
                atomic_set(&t->count, 0);
                if (t->func)
                    t->func(t->data);
            }
        }
    }

}

static void ite_tasklet_start(void)
{
    struct tasklet_ctrl *ctrl = &iteTaskletCtrl;
    pthread_attr_t attr;
    struct sched_param param;
    int res;

    INIT_LIST_HEAD(&ctrl->list);
    sem_init(&ctrl->sem, 0, 0);
    sem_init(&ctrl->thread_complete, 0, 0);

    pthread_attr_init(&attr);
    param.sched_priority = 4;
    pthread_attr_setschedparam(&attr, &param);
    res = pthread_create(&ctrl->thread, &attr, ite_tasklet_thread, (void*)ctrl);
    if (res) {
        printf("\n\n[ERR] create ite_tasklet_thread fail! \n\n");
        return;
    }

    sem_wait(&ctrl->thread_complete);
    sem_destroy(&ctrl->thread_complete);
    inited = true;
}

void ite_tasklet_init(struct tasklet_struct *t,
    void(*func)(unsigned long), unsigned long data)
{
    struct tasklet_ctrl *ctrl = &iteTaskletCtrl;

    if (inited == false)
        ite_tasklet_start();

    atomic_set(&t->count, 0);
    INIT_LIST_HEAD(&t->list);
    t->func = func;
    t->data = data;

    ithEnterCritical();
    list_add_tail(&t->list, &ctrl->list);
    ithExitCritical();
}

void ite_tasklet_kill(struct tasklet_struct *t)
{
    atomic_set(&t->count, 0);
    t->func = NULL;
    t->data = 0;

    ithEnterCritical();
    list_del(&t->list);
    ithExitCritical();
}

void ite_tasklet_schedule(struct tasklet_struct *t)
{
    struct tasklet_ctrl *ctrl = &iteTaskletCtrl;

    atomic_inc(&t->count);

    if (atomic_read(&ctrl->count) == 0) {
        atomic_inc(&ctrl->count);
        sem_post(&ctrl->sem);
    }
}