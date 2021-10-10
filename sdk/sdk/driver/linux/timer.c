#include <stdio.h>
#include <linux/timer.h>
#include "ite/itp.h"


static void timer_cb(timer_t timerid, int arg)
{
    struct timer_list *timer = (struct timer_list *)arg;
    if (timer->function)
        timer->function(timer->data);
}

void ite_init_timer(struct timer_list *timer)
{
    timer_create(CLOCK_REALTIME, NULL, &timer->timerId);
    timer_connect(timer->timerId, timer_cb, (int)timer);
}

/** deactivate a timer */
int ite_del_timer(struct timer_list *timer)
{
    int rc;

    timer->value.it_value.tv_sec = 0;
    timer->value.it_value.tv_nsec = 0;
    timer->value.it_interval.tv_sec = 0;
    timer->value.it_interval.tv_nsec = 0;
    rc = timer_settime(timer->timerId, 0, &timer->value, NULL);
    if (rc)
        printf("ite_del_timer(0x%X,%d) fail!!!!!!\n", timer);

    return rc;
}

/**
* mod_timer - modify a timer's timeout
* @timer: the timer to be modified
* @expires: new timeout in jiffies
*/
int ite_mod_timer(struct timer_list *timer, unsigned long expires)
{
    int rc;

    timer->value.it_value.tv_sec = expires / 1000;
    timer->value.it_value.tv_nsec = (expires % 1000) * 1000000;
    timer->value.it_interval.tv_sec = 0;
    timer->value.it_interval.tv_nsec = 0;
    rc = timer_settime(timer->timerId, 0, &timer->value, NULL);
    if (rc)
        printf("ite_mod_timer(0x%X,%d) fail!!!!!!\n", timer, expires);

    return rc;
}

void ite_add_timer(struct timer_list *timer)
{
    timer_settime(timer->timerId, 0, &timer->value, NULL);
}

int ite_destroy_timer(struct timer_list *timer)
{
	int rc = timer_delete(timer->timerId);
	if (rc)
		printf("ite_destroy_timer(0x%X) fail!!!! \n", timer);

	return rc;
}
