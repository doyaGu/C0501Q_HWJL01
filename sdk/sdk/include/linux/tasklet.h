#ifndef _TASKLET_H_
#define _TASKLET_H_

#include <stdio.h>
#include <linux/list.h>
#include <linux/atomic.h>

struct tasklet_struct {
    struct list_head list;
    atomic_t count;
    void(*func)(unsigned long);
    unsigned long data;
};


#define tasklet_init        ite_tasklet_init
#define tasklet_kill        ite_tasklet_kill
#define tasklet_schedule    ite_tasklet_schedule

void ite_tasklet_init(struct tasklet_struct *t,
    void(*func)(unsigned long), unsigned long data);
void ite_tasklet_kill(struct tasklet_struct *t);
void ite_tasklet_schedule(struct tasklet_struct *t);



#endif // _TASKLET_H_