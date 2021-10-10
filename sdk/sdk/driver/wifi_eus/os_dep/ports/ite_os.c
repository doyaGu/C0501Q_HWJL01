//#include "rt_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#include "ports/ite_port.h"

#define NETLINK_NUM 24


sem_t* wifi_create_sem(int cnt)
{
    sem_t* x = malloc(sizeof(sem_t)); 
    sem_init(x, 0, cnt); 
    return x;
}

#define SYS_CreateEvent()               wifi_create_sem(0) 
#define SYS_WaitEvent                   itpSemWaitTimeout
#define SYS_SetEvent                    sem_post
#define SYS_DelEvent(a)                 do { sem_destroy(a); free(a); } while(0)
#define SYS_SetEventFromIsr				itpSemPostFromISR


struct netlink {
	int setflags;
	int recvflags;
	char buffer[NETLINK_NUM][1024];
    int messageLen[NETLINK_NUM];
};


struct netlink netlinkbuffer;

//MMP_INT wifi_spinlock = PAL_MUTEX_WIFI_SPINLOCK;  /** currently has 25 spinlock */
//MMP_INT wifi_mutex = PAL_MUTEX_WIFI_MUTEX;
#if 0
/*Atomic Operation*/ /*These functions can't be interrupted.*/
int atomic_add_return(int i, atomic_t *v)
{
	int val;

	ITE_INT_SAVE();
	val = v->counter;
	v->counter = val += i;
	ITE_INT_RESTORE();
	
	return val;
}

int atomic_sub_return(int i, atomic_t *v)
{
	int val;

	ITE_INT_SAVE();
	val = v->counter;
	v->counter = val -= i;
	ITE_INT_RESTORE();

	return val;
}

int atomic_inc_return(atomic_t *v)
{
	int val;

	ITE_INT_SAVE();
	val = v->counter;
	v->counter = val += 1;
	ITE_INT_RESTORE();
	
	return val;
}

int atomic_dec_return(atomic_t *v)
{
	int val;

	ITE_INT_SAVE();
	val = v->counter;
	v->counter = val -= 1;
	ITE_INT_RESTORE();

	return val;
}


int atomic_dec_and_test(atomic_t *v)
{
    int val;
	val = atomic_dec(v);

	if(val == 0)
		return true;
	else
		return false;
	
}
#endif
unsigned long
RT_wait_for_completion_timeout(
   struct completion *x, 
   unsigned long timeout)
{

   unsigned long result,now;

   for(;;)
   {
   		 now = ITE_GET_TIME();
	     if (x->isFinish)
		 {  
	        x->isFinish = 0;
	        return 1;
	     }	

		 if (    ((timeout <= now) &&
                 ((now - timeout) < 0x7fffffff)) ||
                 ((timeout > now) &&
                 ((timeout - now) >= 0x7fffffff))
                 )
		 {
             return 0;
		 }
		 usleep(1);
   }	
   


}

//#define STATIC_CONTROL
struct tasklet_struct statictasklet[2];
static int statictaskletCount = 0; 
struct tasklet_struct tasklet[1024];
struct tasklet_struct tasklethi[1024];
static int taskletR =0,taskletW =0;
static int tasklethiR =0,tasklethiW =0;
static unsigned int   taskletCount = 0;
static unsigned int   tasklethiCount = 0;
void*	rw_event;
extern bool txframespending;


void tasklet_doprocess(void *p)
{	 
	rw_event = SYS_CreateEvent();

	while(1)
	{
#ifndef STATIC_CONTROL
         SYS_WaitEvent(rw_event, 100);

		 if(tasklethiCount)
		 {
           //printf("hiR = %d,hiC = %d\n",tasklethiR,tasklethiCount);
           //printf("--\n"); 
		   (tasklethi[tasklethiR].func)(tasklethi[tasklethiR].data);
           //printf("++\n");
			
			tasklethiCount--;
			tasklethiR++;
			if(tasklethiR >= 1024)
				tasklethiR = 0;
		 }
	     else if(taskletCount)
		 {
			//printf("R = %d,C = %d\n",taskletR,taskletCount);
            //printf("**\n");  
			(tasklet[taskletR].func)(tasklet[taskletR].data);
            //printf("&&\n");
			taskletCount--;
			taskletR++;
			if(taskletR >= 1024)
				taskletR = 0;

            if(taskletCount > 5)
            {
				do
				{
				    //printf("R = %d,C = %d\n",taskletR,taskletCount);
		            //printf("**\n");  
					(tasklet[taskletR].func)(tasklet[taskletR].data);
		            //printf("&&\n");
					taskletCount--;
					taskletR++;
					if(taskletR >= 1024)
						taskletR = 0;
					
				}while(taskletCount);
            }
			


		 }	 	 
#else
if(statictaskletCount > 0)
{
    if(statictasklet[0].count > 0)
    {
    	(statictasklet[0].func)(statictasklet[0].data);
   
    	(statictasklet[1].func)(statictasklet[1].data);
		
		statictasklet[0].count--;
    }
}	
#endif
}


}
 

void start_tasklet_timer(void)
{
#if 1
	pthread_t task;
	pthread_attr_t attr;
	struct sched_param param;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 102400);
	param.sched_priority = 5;
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&task, &attr, tasklet_doprocess, NULL);
#endif
}
 
void tasklet_init(struct tasklet_struct *t,
		   void (*func)(unsigned long), unsigned long data)
 {
	 t->func = func;
	 t->data = data;
#ifdef STATIC_CONTROL	 
	 statictasklet[statictaskletCount].func = t->func;
	 statictasklet[statictaskletCount].data = t->data;
	 statictasklet[statictaskletCount].count = 0;
     statictaskletCount++;
#endif	 
 }


#if 1
void tasklet_schedule(struct tasklet_struct *t)
{
#ifndef STATIC_CONTROL

	 if(taskletCount < 1024)
     {
    	 tasklet[taskletW].func = t->func;
		 tasklet[taskletW].data = t->data;
	 
		 taskletCount++;	 
		 taskletW++;
		 if(taskletW >= 1024)
				taskletW = 0;

		 
		 SYS_SetEvent(rw_event);
	 
	 }
	 else
	 {
         printf("tasklet_schedule queue full\n");
	 } 
 
 #else
 statictasklet[0].count++;
 #endif
 }
 
void tasklet_hi_schedule(struct tasklet_struct *t)
{
#ifndef STATIC_CONTROL

#if 1

	if(tasklethiCount < 1024)
	{
		tasklethi[tasklethiW].func = t->func;
		tasklethi[tasklethiW].data = t->data;
	
		tasklethiCount++; 	
		tasklethiW++;
		if(tasklethiW >= 1024)
			   tasklethiW = 0;

		SYS_SetEvent(rw_event);
	
	}
	else
	{
		printf("tasklet_hi_schedule queue full\n");
	}
	
 #else
 
 tasklet_schedule(t);

 #endif
 #else
 statictasklet[0].count++;
 #endif
}
#else

void tasklet_schedule(struct tasklet_struct *t)
 
 {
	 //pthread_mutex_lock(t->mutex);
	 //atomic_inc(&t->count);

	 //ITE_INT_SAVE();	
 
	 (t->func)(t->data);

	 //ITE_INT_RESTORE();
	 
	 //atomic_dec(&t->count);
	 //pthread_mutex_unlock(t->mutex);
	 
 }

void tasklet_hi_schedule(struct tasklet_struct *t)
 
 {
	// pthread_mutex_lock(t->mutex);
	 //atomic_inc(&t->count);

	 //ITE_INT_SAVE();	
 
	 (t->func)(t->data);

	 //ITE_INT_RESTORE();
	 
	 //atomic_dec(&t->count);
	 //pthread_mutex_unlock(t->mutex);
	 
 }

 
#endif

//event

void*	netlink_event;


void netlinkInitial(void)
{
    int i = 0;
	
	netlinkbuffer.setflags = 0;
	netlinkbuffer.recvflags = 0;
    for(i=0; i<NETLINK_NUM; i++)
    {
		netlinkbuffer.messageLen[i] = 0;
		memset(netlinkbuffer.buffer[i],0,1024);
    }
}

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]


void wireless_send_event(void* dev,unsigned int cmd,unsigned int lens,char* buff,struct eventsockaddr ap_addr,struct eventsockaddr addr)
{
    
    struct iw_event iwe_buf;
	int len = sizeof(struct iw_event);
	int len2 = sizeof(struct eventsockaddr);
	//union iwreq_data* wrqu_data = (union iwreq_data*)wrqu;
    //union iwreq_data wrqu_data;

	ITE_INT_SAVE();

    //memcpy(&wrqu_data,wrqu,sizeof(union iwreq_data));
#if 1
	{	
		iwe_buf.cmd = cmd;
		iwe_buf.len =  lens + len + len2*2;
		iwe_buf.u.data.length = lens;
			
		//printf("+%s, %02x:%02x:%02x:%02x:%02x:%02x send\n", __func__, MAC2STR(addr.sa_data));
		netlinkbuffer.messageLen[netlinkbuffer.setflags] = iwe_buf.len;
		memcpy(netlinkbuffer.buffer[netlinkbuffer.setflags],&iwe_buf,len);
		memcpy(&netlinkbuffer.buffer[netlinkbuffer.setflags][len],&ap_addr,len2);
		memcpy(&netlinkbuffer.buffer[netlinkbuffer.setflags][len + len2],&addr,len2);
		if(buff != NULL && lens > 0)
			memcpy(&netlinkbuffer.buffer[netlinkbuffer.setflags][len + len2 + len2],buff,lens);

		netlinkbuffer.setflags++;

		if(netlinkbuffer.setflags >= NETLINK_NUM)
			netlinkbuffer.setflags = 0;

		printf("netlinkbuffer.setflags = %d\n",netlinkbuffer.setflags);

		SYS_SetEvent(netlink_event);
	}
#endif
    ITE_INT_RESTORE();
}


int mmpRtlWifiDriverNetlinkrecvfrom(char* buf, int size)
{
     int index = netlinkbuffer.recvflags;
	 ITE_INT_SAVE();

     if(netlinkbuffer.setflags != netlinkbuffer.recvflags)
     {	   
	   if((size >= netlinkbuffer.messageLen[index]) && (buf!=NULL))
       {
            if(netlinkbuffer.messageLen[index]>0)
            {
				memcpy(buf,netlinkbuffer.buffer[index],netlinkbuffer.messageLen[index]);
           

				netlinkbuffer.recvflags++;
				if(netlinkbuffer.recvflags >= NETLINK_NUM)
					netlinkbuffer.recvflags = 0;

				printf("netlinkbuffer.recvflags = %d\n",netlinkbuffer.recvflags);

				ITE_INT_RESTORE();

				return netlinkbuffer.messageLen[index]; 
			}
			else
			{
				ITE_INT_RESTORE();
				return -1;
			}
       }
	   else
	   {
            ITE_INT_RESTORE();
			return -1;
	   }
    }
	else
	{
       ITE_INT_RESTORE();
	   return -1; 
	}

}


int copy_from_user(void *_des,void *_src,int _len)
{
    memcpy((void *)(_des), (void *)(_src), (int)(_len));
	return 0;
}

int copy_to_user(void *_des,void *_src,int _len)
{
    memcpy((void *)(_des), (void *)(_src), (int)(_len));
	return 0;
}


