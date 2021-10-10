//#include "rt_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#include "ports/ite_port.h"
void*	timer_event;
/* Simple Linux timer implement. */
static ITE_TIMER_CTRL_T smTimerCtrl;

sem_t* timer_create_sem(int cnt)
{
    sem_t* x = malloc(sizeof(sem_t)); 
    sem_init(x, 0, cnt); 
    return x;
}

#define SYS_CreateEvent()               timer_create_sem(0) 
#define SYS_WaitEvent                   itpSemWaitTimeout
#define SYS_SetEvent                    sem_post
#define SYS_DelEvent(a)                 do { sem_destroy(a); free(a); } while(0)
#define SYS_SetEventFromIsr				itpSemPostFromISR


int iteTimerInit(ITE_TIMER_T *this) 
{
    ITE_TIMER_CTRL_T *timerCtrl = &smTimerCtrl;
    int i;
    
   	if ((timerCtrl->timerTask <= 0) || (this == NULL))
   	{
		return -1;
   	}

	if (timerCtrl->timerCnt >= SM_TIMER_MAX_TIMER)
	{
		printf("\n@@@@SM Timer : Timer so much!");	
		return -2;
	}
		
	for (i = 0; i < timerCtrl->timerCnt; i++)
	{
		if (timerCtrl->timer[i] == this)
		{
			this->_scheduled = 0;  
			return 0;
		}
	}
	
	this->_scheduled = 0;  
	
	timerCtrl->timer[timerCtrl->timerCnt++] = this;

		
	return 0;
}

int iteTimerAdd(ITE_TIMER_T *this) 
{
    ITE_TIMER_CTRL_T *timerCtrl = &smTimerCtrl;
    
   	if ((timerCtrl->timerTask <= 0) || (this == NULL))
   		return -1;

   	if (this->expires == 0)  /* avoid very small touts to be 0 */
   		this->expires = 1;
        
	this->_scheduled = 1;

	SYS_SetEvent(timer_event);
			 
	return 0;
}

int iteTimerDel(ITE_TIMER_T *this) 
{
	if (this)
		this->_scheduled = 0;

	return 1;  /*always return TRUE*/
}

int iteTimerPending(ITE_TIMER_T *this) 
{
	if (this->_scheduled)
        return 1;
	else
    	return 0;
}

void smTimerCtrlProc(void *p)
{
    ITE_TIMER_CTRL_T *timerCtrl = &smTimerCtrl;
	ITE_TIMER_T *run = NULL;
   	unsigned long i, now;
   	timer_event = SYS_CreateEvent();

    for(;;)
    {
      SYS_WaitEvent(timer_event, 1);
		  if(timerCtrl->kill)
      {
			    goto end;
      }  

      spin_lock(&timerCtrl->lock);

    	now = ITE_GET_TIME();

    	for (i = 0; i < (unsigned long)timerCtrl->timerCnt; i++)
    	{

			run = timerCtrl->timer[i]; 
    		if (run == NULL)
    		{
    			printf("\rITE Timer : NULL timer !!");
    			continue;
    		}


#if 1
			if((run->_scheduled == 1) &&
				((run->expires < now) && 
				((now - run->expires) >= run->delayT)) ||
				((run->expires > now) &&
                ((run->expires - now) >= SM_TIMER_MAX_TOCLOCK) &&
                ((now >= run->delayT) || ((now + (SM_TIMER_MAX_CLOCK - run->expires)) >= run->delayT ))))
#else
    		if ((run->_scheduled == 1) && ( 
                 ((run->expires <= now) &&
                 ((now - run->expires) < SM_TIMER_MAX_TOCLOCK)) ||
                 ((run->expires > now) &&
                 ((run->expires - now) >= SM_TIMER_MAX_TOCLOCK))
                 ) )  
 #endif                
    		{
    			if  (run->function != NULL ) 
    			{
					run->_scheduled = 0;
    				(run->function)(run->data);
    			}
    			else
    			{
    				printf("\rITE Timer : function is NULL , timer = %08x \n",(unsigned long)run);
    				continue;
    			}
    		}
        }  	
		
        spin_unlock(&timerCtrl->lock);

    }
    
end:
    complete_and_exit(&timerCtrl->threadNotify, 0);
	return;
}

int iteTimerCtrlInit(void) 
{
    ITE_TIMER_CTRL_T *timerCtrl = &smTimerCtrl;
	int i;
    /* init lock */
    spin_lock_init(&timerCtrl->lock);

    init_completion(&timerCtrl->threadNotify);
    timerCtrl->timerCnt = 0;
    
	/* Clear all timer */
	for(i = 0; i < SM_TIMER_MAX_TIMER; i++)
		timerCtrl->timer[i] = NULL;

    /* Start timer task. */
	if (timerCtrl->timerTask > 0) 
   	{
	    printf("\rITE Timer : Already init.");   	
   	}
    else
        timerCtrl->timerTask = kernel_thread(smTimerCtrlProc, NULL, 0);

	return 0;	
}

int iteTimerCtrlTerminate(void) 
{
    ITE_TIMER_CTRL_T *timerCtrl = &smTimerCtrl;
	int i;


   	if (timerCtrl->timerTask > 0) 
   	{
        timerCtrl->kill = 1;
        wait_for_completion(&timerCtrl->threadNotify);
        if(kill_proc (timerCtrl->timerTask, SIGTERM, 1))
		    printk (" unable to kill timer thread, pid=%d\n", timerCtrl->timerTask);

        timerCtrl->timerTask = -1;
   	}

    timerCtrl->timerCnt = 0;
    
	/* Clear all timer */
	for(i = 0; i < SM_TIMER_MAX_TIMER; i++)
		timerCtrl->timer[i] = NULL;

	return 0;	
}

