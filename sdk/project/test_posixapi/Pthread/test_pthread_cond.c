#include <sys/time.h>
#include <errno.h>
#include <pthread.h>                                                                                                                    
#include <stdio.h>                                                              
#include <stdlib.h>      
#include <time.h>                                                               
#include <unistd.h>       
                                             
                                                                                
static pthread_cond_t cond;                                                            
static pthread_mutex_t mutex;                                                          
                                                                                
static int footprint = 0;                                                              
               
void Test_pthread_cond_timedwait(void)
{
	pthread_cond_t cond;                                                          
  	pthread_mutex_t mutex;                                                        
  	time_t T;                                                                     
  	struct timespec t;                                                            
                                                                                
  	if (pthread_mutex_init(&mutex, NULL) != 0) 
	{	perror("pthread_mutex_init() error");	exit(1);	}
	                                                                                                                                                           
  	if (pthread_cond_init(&cond, NULL) != 0) 
  	{	perror("pthread_cond_init() error");	exit(2);	}                                                                    
                                                                                                                                                            
  	if (pthread_mutex_lock(&mutex) != 0)                                         
    {	perror("pthread_mutex_lock() error");	exit(3);	}                                                                                                                                                 
                                                                                
  	time(&T);                                                                     
  	t.tv_sec = T + 2;
	                                                               
  	printf("starting timedwait at %s", ctime(&T));                                
  	if (pthread_cond_timedwait(&cond, &mutex, &t) != 0)
	{                           
    	if (errno == EAGAIN)
			puts("wait timed out");                                                   
	}
	else 
	{	perror("pthread_cond_timedwait() error");	exit(4);	}                                                                        
                                                                                
  	time(&T);                                                                     
  	printf("timedwait over at %s", ctime(&T));   
}			   
			   
			                                                                    
void *thread(void *arg) 
{                                                       
	time_t T;                                                                     
                                                                                
  	if (pthread_mutex_lock(&mutex) != 0)
	{	perror("pthread_mutex_lock() error");	exit(6);	}                                                                             
  	
	time(&T);	                                                                       
  	printf("starting wait at %s", ctime(&T));                                     
  	footprint++;                                                                  
                                                                                
  	if (pthread_cond_wait(&cond, &mutex) != 0) 
	{	perror("pthread_cond_timedwait() error");	exit(7);	}                                                                             
  	
	time(&T);                                                                     
  	printf("wait over at %s", ctime(&T));

    pthread_mutex_unlock(&mutex);
}   


void Test_pthread_cond_wait(void)
{
	pthread_t thid;                                                               
  	time_t T;                                                                     
  	struct timespec t;                                                            
                                                                                
  	if (pthread_mutex_init(&mutex, NULL) != 0) 
  	{	perror("pthread_mutex_init() error");	exit(1);	}                                                                             
                                                                                
  	if (pthread_cond_init(&cond, NULL) != 0) 
	{	perror("pthread_cond_init() error");	exit(2);	}                                                                             
                                                                                
  	if (pthread_create(&thid, NULL, thread, NULL) != 0)
	{	perror("pthread_create() error");	exit(3);	}                                                                             
                                                                                
  	while (footprint == 0)
		sleep(1);                                                                   
                                                                                
  	puts("IPT is about ready to release the thread");                             
  	sleep(2);                                                                     
                                                                                
  	if (pthread_cond_signal(&cond) != 0) 
	{	perror("pthread_cond_signal() error");	exit(4);	}                                                                             
                                                                                
  	if (pthread_join(thid, NULL) != 0)
	{	perror("pthread_join() error");	exit(5);	}

     pthread_cond_destroy(&cond);
     pthread_mutex_destroy(&mutex);
}
   
void Test_Cond_Others(void)
{
	pthread_cond_t cond;                                                          
                                                                                
  	if (pthread_cond_init(&cond, NULL) != 0) 
	{	perror("pthread_cond_init() error");	exit(1);	}
	else	printf("pthread_cond_init() success!!\n");                                                                             
                                                                                
  	if (pthread_cond_broadcast(&cond) != 0) 
  	{	perror("pthread_cond_broadcast() error");	exit(2);	}   
	else	printf("pthread_cond_broadcast() success!!\n");
	  
  	if (pthread_cond_signal(&cond) != 0) 
	{	perror("pthread_cond_broadcast() error");	exit(3);	}                                                                                    
	else	printf("pthread_cond_signal() success!!\n");
	                                                                                
  	if (pthread_cond_destroy(&cond) != 0) 
  	{	perror("pthread_cond_destroy() error");	exit(4);	}
	else	printf("pthread_cond_destroy() success!!\n");
}

void* TestFunc(void* arg)
{
	itpInit();
	
	printf("\nTest :\n");
  	printf("pthread_cond_init()\n");
	printf("pthread_cond_broadcast()\n");
	printf("pthread_cond_signal()\n");
	printf("pthread_cond_destroy()\n");
	printf("------------------\n");
  	Test_Cond_Others();
  	
	printf("\nTest: pthread_cond_wait()\n------------------\n");
  	Test_pthread_cond_wait();
  	
	printf("\nTest: pthread_cond_timedwait()\n------------------\n");
  	Test_pthread_cond_timedwait();  
	
	printf("\nEnd test\n");                                                                    	
}
