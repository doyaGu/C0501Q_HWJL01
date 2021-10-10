#ifndef	ITE_WIFI_TIMER_H
#define	ITE_WIFI_TIMER_H

typedef void (*ITE_TIMER_PROC_F) (void *data);
typedef struct ITE_TIMER_S {
  	unsigned 	_scheduled;					/* Active timer */

#define SM_TIMER_MAX_TOCLOCK 		    0x7fffffff      /*max allow time-out value.*/
#define SM_TIMER_MAX_CLOCK              0xffffffff
  	unsigned long  expires;    				/* Tick Count To Wake Up   */  
    unsigned long  delayT;
  	ITE_TIMER_PROC_F function;				/* Expire call-back function */
  	unsigned long data;					    /* Data pointer */
} ITE_TIMER_T;

typedef struct ITE_TIMER_CTRL_S {
#define SM_TIMER_MAX_TIMER  1024
    ITE_TIMER_T *timer[SM_TIMER_MAX_TIMER]; 

    int timerCnt;
    pid_t timerTask;
    spinlock_t  lock;
    int kill;
    struct completion	threadNotify;
} ITE_TIMER_CTRL_T;

#define ITE_GET_TIME()           xTaskGetTickCount()

int iteTimerInit(ITE_TIMER_T *this);
int iteTimerAdd(ITE_TIMER_T *this);
int iteTimerDel(ITE_TIMER_T *this);
int iteTimerPending(ITE_TIMER_T *this);
int iteTimerCtrlInit(void);
int iteTimerCtrlTerminate(void);

#endif

