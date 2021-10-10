#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
                                                           
#define THREADNUM 3                                                                                
void *HelloFunc(void *arg) 
{
   printf("hello from the thread %d\n",*(int*)arg);
   sleep(1);
   printf("End thread %d\n",*(int*)arg);
   pthread_exit(NULL);
}

void Test_pthread_detach()
{
	int            rc, stat,i,num[THREADNUM];
	pthread_t      thid[THREADNUM];            
	pthread_attr_t attr; 				
                                                                                
	if (pthread_attr_init(&attr) == -1)    //Test for pthread_attr_init
    	perror("error in pthread_attr_init");
    
	for(i=0;i<THREADNUM;i++)
	{
		num[i] = i;
		//Test for pthread_create
   		if (pthread_create(&thid[i], &attr, HelloFunc, (void*)&num[i] ) == -1)
   	  		perror("error in pthread_create");
	}
	for(i=0;i<THREADNUM;i++)
		if (pthread_detach(thid[i]) == -1)        
      		perror("error in pthread_detach");

   	
   	printf("hello from the Main thread\n");
    sleep(3);
			       	
   	if (pthread_attr_destroy(&attr) == -1)
      	perror("error in pthread_attr_destroy");
    printf("End Main Thread\n");
}

void Test_pthread_join()
{
	int            rc, stat,i,num[THREADNUM];
	pthread_t      thid[THREADNUM];            
	pthread_attr_t attr; 
                                                                                
	if (pthread_attr_init(&attr) == -1)
    	perror("error in pthread_attr_init");
    
	for(i=0;i<THREADNUM;i++)
	{
		num[i] = i;
   		if (pthread_create(&thid[i], &attr, HelloFunc, (void*)&num[i] ) == -1)
   	  		perror("error in pthread_create");
	}

	for(i=0;i<THREADNUM;i++)		                                                                   
   		rc = pthread_join(thid[i], (void *)&stat);
   	
   	printf("hello from the Main thread\n");
    sleep(3);
			       	
   	if (pthread_attr_destroy(&attr) == -1)
      	perror("error in pthread_attr_destroy");
    printf("End Main Thread\n");	
}

/***************************************************************************/

pthread_mutex_t m_mtx;
int m_goahead;

void dosleep(int millis)
{
    usleep(millis*1000);
}

void domsg(const char *msg)
{
    pthread_mutex_lock(&m_mtx);
    printf("%s\n",msg);
    pthread_mutex_unlock(&m_mtx);
}

void dowait() 
{
    while (!m_goahead) 
        dosleep(1);
}

void *fn1(void *param)
{
    domsg("in fn1...waiting");
    dowait();
    while (m_goahead) 
	{
        dosleep(1000);
        domsg("in fn1 loop");
    }
}

void *fn2(void *param)
{
    domsg("in fn2...waiting");
    dowait();
}

void Test_pthread_setschedparam()
{	
    // min prio = -2, max prio = 2
	int t1_pri = 2, t2_pri = 0, main_pri = 1;
    //SCHED_RR, SCHED_FIFO, SCHED_OTHER (POSIX scheduling policies)
    int sched = SCHED_OTHER; // standard
    // get the range between min and max and set the priorities base on split range
    int min = sched_get_priority_min(sched);
    int max = sched_get_priority_max(sched);
    int skip = (max - min) / 5; // 5 since -2...2
    struct sched_param main_param, t1_param, t2_param;
    
    memset(&main_param, 0, sizeof(struct sched_param));
    memset(&t1_param, 0, sizeof(struct sched_param));
    memset(&t2_param, 0, sizeof(struct sched_param));
    
    main_param.sched_priority = (min + ((main_pri+2) * (skip+1))) + (skip / 2);
    t1_param.sched_priority = (min + ((t1_pri+2) * (skip+1))) + (skip / 2);
    t2_param.sched_priority = (min + ((t2_pri+2) * (skip+1))) + (skip / 2);
    
	printf("main thread will have a prio of %d\n",main_param.sched_priority);
    printf("t1 thread will have a prio of %d\n",t1_param.sched_priority);
    printf("t2 thread will have a prio of %d\n",t2_param.sched_priority);
    
	m_goahead = 0;
    pthread_mutex_init(&m_mtx, NULL);
    pthread_t t1, t2;
    // Create the threads 
    if (pthread_create(&t1, NULL, fn1, NULL) != 0) 
        printf("couldn't create t1\n");

    if (pthread_create(&t2, NULL, fn2, NULL) != 0) 
        printf("couldn't create t2\n");
    
    //Test for pthread_getschedparam()
    int Start_sched;
    struct sched_param Start_main_param , Start_t1_param , Start_t2_param;
   	if (pthread_getschedparam(pthread_self(), &Start_sched, &Start_main_param) != 0) 
    	printf("error getting priority for main thread: (%d), %s\n",errno,strerror(errno));

   	if (pthread_getschedparam(t1, &Start_sched, &Start_t1_param) != 0) 
    	printf("error getting priority for main thread: (%d), %s\n",errno,strerror(errno));
    	
   	if (pthread_getschedparam(t1, &Start_sched, &Start_t2_param) != 0) 
    	printf("error getting priority for main thread: (%d), %s\n",errno,strerror(errno));    
    
	printf("Initial prio Value\n");
	printf("main thread have a prio = %d\n",Start_main_param.sched_priority);
	printf("t1 thread have a prio = %d\n",Start_t1_param.sched_priority);
	printf("t2 thread have a prio = %d\n",Start_t2_param.sched_priority);
	
	
	dosleep(1000); // sleep a second before setting priorities
    // --main thread--
    if (pthread_setschedparam(pthread_self(), sched, &main_param) != 0) 
        printf("error setting priority for main thread: (%d), %s\n",errno,strerror(errno));
    
    // --t1 thread--
    if (pthread_setschedparam(t1, sched, &t1_param) != 0) 
        printf("error setting priority for T1: (%d), %s\n",errno,strerror(errno));
    
    // --t2 thread--
    if (pthread_setschedparam(t2, sched, &t2_param) != 0) 
        printf("error setting priority for T2: (%d), %s\n",errno,strerror(errno));
    
    m_goahead = 1; // all start
    // loop until user interupt

    pthread_mutex_destroy(&m_mtx);
}

////////////////////////////////////////////
#define threads 3
#define BUFFSZ  48
pthread_key_t   key;

void *threadfunc(void *parm)
{
	int        status;
 	void      *value;
 	int        threadnum;
 	int       *tnum;
 	void      *getvalue;
 	char       Buffer[BUFFSZ];

 	tnum = parm;
 	threadnum = *tnum;

 	printf("Thread %d executing\n", threadnum);

 	if (!(value = malloc(sizeof(Buffer))))
     	printf("Thread %d could not allocate storage, errno = %d\n",
                                                  threadnum, errno);
 	status = pthread_setspecific(key, (void *) value);
 	if ( status <  0) 
 	{
    	printf("pthread_setspecific failed, thread %d, errno %d",threadnum, errno);
    	pthread_exit((void *)12);
 	}
 	printf("Thread %d setspecific value: %d\n", threadnum, value);

 	getvalue = 0;
 	status = pthread_getspecific(key);
 	if ( status <  0) 
 	{
    	printf("pthread_getspecific failed, thread %d, errno %d",threadnum, errno);
    	pthread_exit((void *)13);
 	}	

 	if (getvalue != value) 
 	{
   		printf("getvalue not valid, getvalue=%d", (int)getvalue);
   		pthread_exit((void *)68);
 	}

 	pthread_exit((void *)0);
}

void  destr_fn(void *parm)
{
	printf("Destructor function invoked\n");
   	free(parm);
}

void Test_pthread_setspecific()
{
 	int          getvalue;
 	int          status;
 	int          i;
 	int          threadparm[threads];
 	pthread_t    threadid[threads];
 	int          thread_stat[threads];

 	if ((status = pthread_key_create(&key, destr_fn )) < 0) 
 	{
    	printf("pthread_key_create failed, errno=%d", errno);
    	exit(1);
 	}

 	for (i=0; i<threads; i++) 
 	{
    	threadparm[i] = i+1;
    	status = pthread_create( &threadid[i] , NULL , threadfunc , (void *)&threadparm[i]);
    	
		if ( status <  0) 
		{
       		printf("pthread_create failed, errno=%d", errno);
       		exit(2);
    	}
  	}

 	for ( i=0; i<threads; i++) 
 	{
    	status = pthread_join( threadid[i],
    	(void *)&thread_stat[i]);
    	
    	if ( status <  0) 
       		printf("pthread_join failed, thread %d, errno=%d\n", i+1, errno);
    
    	if (thread_stat[i] != 0)   
        	printf("bad thread status, thread %d, status=%d\n", i+1 , thread_stat[i]);     
  	}
}

/****************************************************************************/
#define threads 3
#define BUFFSZ  48
pthread_key_t   key;                                                                          
int             once_counter=0;                                                 
pthread_once_t  once_control = PTHREAD_ONCE_INIT;                               
                                                                                
void  once_fn()                                                             
{                                                                              
	puts("in once_fn");                                                            
 	once_counter++;                                                                
}                                                                               
                                                                                
void *thread_pthread_once(void *parm)                                         
{                                                                              
 	int        status;                                                             
 	int        threadnum;                                                          
 	int        *tnum;                                                              
                                                                                
 	tnum = parm;                                                                   
 	threadnum = *tnum;                                                             
                                                                                
 	printf("Thread %d executing\n", threadnum);                                    
                                                                                
 	status = pthread_once(&once_control, once_fn);                                 
 	if ( status <  0)                                                              
    	printf("pthread_once failed, thread %d, errno=%d\n", threadnum , errno);             
                                                                          
 	pthread_exit((void *)0);                                                      
}
     
void Test_pthread_once()
{
 	int          status;                                                           
 	int          i;                                                                
 	int          threadparm[threads];                                              
 	pthread_t    threadid[threads];                                                
 	int          thread_stat[threads];                                             
                                                                                
 	for (i=0; i<threads; i++) 
	{                                                    
    	threadparm[i] = i+1;                                                        
    	status = pthread_create( &threadid[i], NULL, thread_pthread_once, (void *)&threadparm[i]);                           
    	if ( status <  0) 
		{	printf("pthread_create failed, errno=%d", errno);	exit(2);	}                                                                                                                                      
	}                                                                             
                                                                                
 	for ( i=0; i<threads; i++) 
	{                                                   
    	status = pthread_join( threadid[i], (void *)&thread_stat[i]);               
    	if ( status <  0)                                                           
       		printf("pthread_join failed, thread %d, errno=%d\n", i+1, errno);        
                                                                                
    	if (thread_stat[i] != 0)                                                    
        	printf("bad thread status, thread %d, status=%d\n", i+1, thread_stat[i]);             
  	}                                                                             
                                                                                
 	if (once_counter != 1)                                                         
   		printf("once_fn did not get control once, counter=%d",once_counter);           
}
             
void *thread_getspecific(void *parm)
{
 	int        status;
 	void      *value;
 	int        threadnum;
 	int       *tnum;
 	void      *getvalue;
 	char       Buffer[BUFFSZ];

 	tnum = parm;
 	threadnum = *tnum;

 	printf("Thread %d executing\n", threadnum);

 	if (!(value = malloc(sizeof(Buffer))))
     	printf("Thread %d could not allocate storage, errno = %d\n",threadnum, errno);
 	
	status = pthread_setspecific(key, (void *) value);
 	if ( status <  0) 
	{
    	printf("pthread_setspecific failed, thread %d, errno %d",threadnum, errno);
    	pthread_exit((void *)12);
 	}
 	printf("Thread %d setspecific value: %d\n", threadnum, value);

 	getvalue = 0;
 	status = pthread_getspecific(key);
 	if ( status <  0) 
	{
    	printf("pthread_getspecific failed, thread %d, errno %d",threadnum, errno);
    	pthread_exit((void *)13);
 	}

 	if (getvalue != value)
 	{
  	 	printf("getvalue not valid, getvalue=%d", (int)getvalue);
   		pthread_exit((void *)68);
 	}

 	pthread_exit((void *)0);
}

	
void Test_pthread_getspecific()
{
 	int          getvalue;
 	int          status;
 	int          i;
 	int          threadparm[threads];
 	pthread_t    threadid[threads];
 	int          thread_stat[threads];


 	if ((status = pthread_key_create(&key, destr_fn )) < 0)    //destr_fn() is defined before the Test_pthread_setspecific
	{	printf("pthread_key_create failed, errno=%d", errno);	exit(1);	}

 /* create 3 threads, pass each its number */
 	for (i=0; i<threads; i++) 
	{
    	threadparm[i] = i+1;
    	status = pthread_create( &threadid[i], NULL, thread_getspecific, (void *)&threadparm[i]);
    	if ( status <  0) 
		{	printf("pthread_create failed, errno=%d", errno);	exit(2);	}
  	}

 	for ( i=0; i<threads; i++) 
	{
    	status = pthread_join( threadid[i], (void *)&thread_stat[i]);
    	
		if ( status <  0) 
       		printf("pthread_join failed, thread %d, errno=%d\n", i+1, errno);

    	if (thread_stat[i] != 0)
        	printf("bad thread status, thread %d, status=%d\n", i+1, thread_stat[i]);
  	}
}


void *thread_exit(void *arg) 
{
  	char *ret;

  	if ((ret = (char*) malloc(20)) == NULL) 
	{	perror("malloc() error");	exit(2);	}
  	
	strcpy(ret, "This is a test");
  	pthread_exit(ret);
}
void Test_pthread_exit()
{
  	pthread_t thid;
  	void *ret;

  	if (pthread_create(&thid, NULL, thread_exit, NULL) != 0) 
	{	perror("pthread_create() error");	exit(1);	}

  	if (pthread_join(thid, &ret) != 0) 
	{	perror("pthread_create() error");	exit(3);	}

  	printf("thread exited with '%s'\n", ret);
}

pthread_t thid, IPT;                                                            
                                                                                
void *thread_equal(void *arg) 
{                                                    
  	if (pthread_equal(IPT, thid))                                                 
    	puts("the thread is the IPT...?");                                          
  	else                                                                          
    	puts("the thread is not the IPT");                                          
}         
void Test_pthread_equal()
{                                                                            
	IPT = pthread_self();                                                         
                                                                                
	if (pthread_create(&thid, NULL, thread_equal, NULL) != 0) 
  	{	perror("pthread_create() error");	exit(1);	}                                                                             
                                                                                
  	if (pthread_join(thid, NULL) != 0) 
  	{	perror("pthread_create() error");	exit(3);	}   
}


void *thread_self(void *arg) 
{                                                     
	if (pthread_equal(IPT, pthread_self()))                                       
    	puts("the thread is the IPT...?");                                          
  	else                                                                          
    	puts("the thread is not the IPT");                                          
                                                                                
  	if (pthread_equal(thid, pthread_self()))                                      
    	puts("the thread is the one created by the IPT");                           
  	else                                                                          
    	puts("the thread is not the one created by the IPT...?");                   
}   
void Test_pthread_self()
{
	IPT = pthread_self();                                                         
  	if (pthread_create(&thid, NULL, thread_self, NULL) != 0) 
	{	perror("pthread_create() error");	exit(1);	}                                                                             
                                                                                
  	if (pthread_join(thid, NULL) != 0) 
	{	perror("pthread_create() error");	exit(3);	}	       
}


void* mythread(void *arg) 				
{
	int i,ret;
  	for(i=0; i<10; i++)	
  	{
    	printf("Thread is running (%d) ...\n",i);
    	sleep(1);
  	}
  	pthread_exit("Thank you for the CPU time.\n");
}
void Test_pthread_cancel()
{
	pthread_t id;
  	int i, ret;
  	void *thread_result;
  	ret = pthread_create(&id, NULL, mythread, NULL); 
  	if(ret != 0)
  	{	printf("Create pthread error.\n");	exit(1);	}
  	
	sleep(3);
  	printf("Canceling thread ...\n");
  	ret = pthread_cancel(id);	
  	if(ret != 0)
  	{	printf("Thread cancelation failed.\n");	exit(1);	}
  	else  
	printf("Cancel success\n");
}


void* TestFunc(void* arg)
{
    itpInit();

	printf("\nTest: pthread_cancel()\n--------------------\n");
	Test_pthread_cancel();      	
  
	printf("\nTest : pthread_detach()\n--------------------\n");
   	Test_pthread_detach();
   	
   	printf("\nTest : pthread_join()\n--------------------\n");                                                                             
   	Test_pthread_join();
   
   	printf("\nTest: pthread_setspecific()\n-------------------\n");
   	Test_pthread_setspecific();
   
   	printf("\nTest: pthread_setschedparam()\n-------------------\n");	
   	Test_pthread_setschedparam(); 
   
	printf("\nTest: pthread_once()\n--------------------\n");
	Test_pthread_once();
	
	printf("\nTest: pthread_getspecific()\n--------------------\n");
	Test_pthread_getspecific();
	
	printf("\nTest: pthread_exit()\n--------------------\n");
	Test_pthread_exit(); 
	
	printf("\nTest: pthread_equal()\n--------------------\n");
	Test_pthread_equal();
	
	printf("\nTest: pthread_self()\n--------------------\n");
	Test_pthread_self();
		   
   	printf("\nEnd test\n"); 	
}
