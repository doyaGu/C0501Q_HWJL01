#ifndef _ITE_TIMER_H_
#define _ITE_TIMER_H_

#include <time.h>

/*************** Importmant *********************
 *  
 *  We need to call destroy_timer() to release resource!
 *
 *********************************************/

struct timer_list {
    void(*function)(unsigned long);
    unsigned long data;

    timer_t timerId;
    struct itimerspec value;
};

void ite_init_timer(struct timer_list *timer);
int ite_del_timer(struct timer_list *timer); /** deactivate a timer */ 
int ite_mod_timer(struct timer_list *timer, unsigned long expires);
void ite_add_timer(struct timer_list *timer);
int ite_destroy_timer(struct timer_list *timer);

#define ite_setup_timer(_timer, _fn, _data)			\
	do {								\
		ite_init_timer((_timer));			\
		(_timer)->function = (_fn);				\
		(_timer)->data = (_data);				\
	} while (0)


#define init_timer(t)           ite_init_timer((t))
#define del_timer(t)            ite_del_timer((t))
#define mod_timer(t, expire)    ite_mod_timer((t), (expire-jiffies))
#define add_timer(t)            ite_add_timer((t))
#define del_timer_sync(t)		del_timer(t)
#define setup_timer(t, fn, data)	ite_setup_timer((t), (fn), (data))
#define destroy_timer(t)        ite_destroy_timer((t))

#endif // _ITE_TIMER_H_
