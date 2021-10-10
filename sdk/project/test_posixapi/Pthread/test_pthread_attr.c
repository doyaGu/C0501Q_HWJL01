#include <sys/time.h>                                                          
#include <stdio.h>                                                              
#include <pthread.h>  
                                                                                
void *thread1(void *arg)                                                        
{                                                                               
	printf("hello from the thread\n");                                           
    pthread_exit(NULL);                                                          
} 

void Test_pthread_attr_setstacksize()
{
	int            rc, stat;                                                     
   	size_t         s1;                                                           
   	pthread_attr_t attr;                                                         
   	pthread_t      thid;        
                                                      
   	rc = pthread_attr_init(&attr);                                               
   	if (rc == -1) 
	{	printf("error in pthread_attr_init");	exit(1);	}                                                                            
                                                                                
   	s1 = 4096;                                                                   
   	rc = pthread_attr_setstacksize(&attr, s1);                                   
   	if (rc == -1)
   	{	printf("error in pthread_attr_setstacksize");	exit(2);	}                                                                            
                                                                                
   	rc = pthread_create(&thid, &attr, thread1, NULL);                            
   	if (rc == -1) 
   	{	printf("error in pthread_create");	exit(3);	}                                                                            
                                                                                
	rc = pthread_join(thid, (void *)&stat);
}

void Test_pthread_attr_setschedparam()
{
	pthread_attr_t attr;                                                         
   	int              rc; 
   	struct sched_param param;

   	param.sched_priority = 999;

   	if (pthread_attr_init(&attr) == -1) 
	{	printf("error in pthread_attr_init");	exit(1);	}                                                                            
                                                                                
   	rc = pthread_attr_setschedparam(&attr, &param);
   	if (rc != 0)                                            
		printf("pthread_attr_setschedparam returned: %d\n", rc); 
	else 
      	printf("Set schedpriority to %d\n", param.sched_priority);
   
   rc = pthread_attr_destroy(&attr);
   if (rc != 0) 
   {	printf("error in pthread_attr_destroy");	exit(2);	}
}         


int thread_finished=0;					
void mythread(void)						
{
	printf("Thread is running ...\n");
	sleep(3);
	printf("Thread is exiting now.\n");
  	thread_finished = 1;					
  	pthread_exit(NULL);
}
void Test_pthread_attr_setdetachstate()
{
	pthread_t id; 						
  	pthread_attr_t thread_attr;				
  	int ret;
  	
  	ret = pthread_attr_init(&thread_attr);		
  	if(ret!=0) 							
  	{	printf ("Attribute creation failed.\n");	exit (1);	}

  	ret=pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
  	if(ret!=0)
	{	printf ("Setting detached attribute failed.\n");	exit (1);	}
  	
	ret=pthread_create(&id, &thread_attr, (void *)mythread, NULL);	
  	if(ret!=0)
	{	printf ("Thread creation failed.\n");	exit (1);	}
	
  	while(!thread_finished)					
  	{
    	printf("Waiting for thread finished ...\n");
    	sleep(1);
  	}
  	printf("Thread finished.\n");				
}   



void Test_pthread_attr_getschedparam()
{
	pthread_attr_t attr;                                                         
   	int              rc; 
   	struct sched_param param;

   	param.sched_priority = 999;

   	if (pthread_attr_init(&attr) == -1)
	{	printf("error in pthread_attr_init");	exit(1);	}                                                                            
                                                                                
   	rc = pthread_attr_setschedparam(&attr, &param);
   	if (rc != 0)                                         
      printf("pthread_attr_setschedparam returned: %d\n", rc);                                                             
    else 
      printf("Set schedpriority to %d\n", param.sched_priority);

   	param.sched_priority = 0;

   	rc = pthread_attr_getschedparam(&attr, &param);
   	if (rc != 0)                                            
    	printf("pthread_attr_getschedparam returned: %d\n", rc);                                                            
    else 
      	printf("Retrieved schedpriority of %d\n", param.sched_priority);
   
   rc = pthread_attr_destroy(&attr);
   if (rc != 0) 
   {	printf("error in pthread_attr_destroy");	exit(4);	}                                                                            
}                                                           
  
  
#define MAX_THREADS	256            
int total_hits, total_misses, hits[MAX_THREADS][1],
	sample_points, sample_points_per_thread, num_threads;
void *compute_pi (void *s)
{
	int thread_no, i, *hit_pointer;
	double rand_no_x, rand_no_y;
	int hits;

	hit_pointer = (int *) s;
	thread_no = *hit_pointer;

	hits = 0;

	srand48(thread_no);
	for (i = 0; i < sample_points_per_thread; i++)
	{
		rand_no_x = (double)(rand_r(&thread_no))/(double)((2 <<30) - 1);
		rand_no_y = (double)(rand_r(&thread_no))/(double)((2 <<30) - 1);

		if (((rand_no_x - 0.5) * (rand_no_x - 0.5) +
			(rand_no_y - 0.5) * (rand_no_y - 0.5)) < 0.25)
			hits ++;
	}
	*hit_pointer = hits;
}

void* TestFunc(void* arg)
{
	itpInit();
	
	printf("\nTest : pthread_attr_getschedparam()\n------------------\n");
    Test_pthread_attr_getschedparam(); 
	
	printf("\nTest : pthread_attr_setdetachstate()\n------------------\n");
    Test_pthread_attr_setdetachstate();   
		
	printf("\nTest : pthread_attr_setstacksize()\n------------------\n");
    Test_pthread_attr_setstacksize();     
	
	printf("\nTest : pthread_attr_setschedparam()\n------------------\n");
    Test_pthread_attr_setschedparam();     
    
	printf("\nEnd Test\n");   	
}
