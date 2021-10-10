#include <errno.h>   
#include <pthread.h>                                                            
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>                                                             

                                                         
/*	Test Function : pthread_mutex_trylock()	*/                                                                                 
pthread_mutex_t mutex;                                            
                                                                     
void *thread_trylock(void *arg) 
{    
	int ERRNO = pthread_mutex_trylock(&mutex);                                                    
	if ( ERRNO != 0)                                 
    	if ( ERRNO == EBUSY)                                                         
      		puts("thread was denied access to the mutex");                            
   		else                                                                    
      		perror("pthread_mutex_trylock() error");                                                                                             
	else	puts("thread was granted the mutex");                                   
}      
void Test_pthread_mutex_trylock()
{
	pthread_t thid;                                                                
                                                                                
	if (pthread_mutex_init(&mutex, NULL) != 0)
	{	perror("pthread_mutex_init() error");	exit(2);	}                                                                             
                                                                                
  	if (pthread_create(&thid, NULL, thread_trylock, NULL) != 0) 
  	{	perror("pthread_create() error");	exit(3);	}                                                                      
  
  	int ERRNO = pthread_mutex_trylock(&mutex); 
	if ( ERRNO != 0)                                       
    	if ( ERRNO == EBUSY)                                                         
      		puts("IPT was denied access to the mutex");                               
    	else 
		{	perror("pthread_mutex_trylock() error");	exit(4);	}                                                                        
  	else	puts("IPT was granted the mutex");                                       
                                                                                
  	if (pthread_join(thid, NULL) != 0)
	{	perror("pthread_mutex_trylock() error");	exit(5);	}    
}    



//Test Function : pthread_mutex_unlock()
void *thread_unlock(void *arg) 
{                                                       
	if (pthread_mutex_lock(&mutex) != 0) //Test Function : pthread_mutex_lock()
  	{	perror("pthread_mutex_lock() error");	exit(1);	}                                                                             
                                                                                
  	puts("thread was granted the mutex");                                         
                                                                                
  	if (pthread_mutex_unlock(&mutex) != 0) 
  	{	perror("pthread_mutex_unlock() error");	exit(2);	}
	else	puts("thread was unlocked the mutex");                                                   
} 
   
void Test_pthread_mutex_unlock()
{
	pthread_t thid;                                                               
                                                                                
  	if (pthread_mutex_init(&mutex, NULL) != 0)
	{	perror("pthread_mutex_init() error");	exit(3);	}                                                                          
                                                                                
  	if (pthread_create(&thid, NULL, thread_unlock, NULL) != 0)
  	{	perror("pthread_create() error");	exit(4);	}                                                                             
  
  	if (pthread_mutex_lock(&mutex) != 0)
  	{	perror("pthread_mutex_lock() error");	exit(5);	}                                                                             
                                                                                
  	puts("IPT was granted the mutex");                                            
                                                                                
  	if (pthread_mutex_unlock(&mutex) != 0)
	{	perror("pthread_mutex_unlock() error");	exit(6);	}
	else	puts("IPT was unlocked the mutex");                                                                           
                                                                                
  	if (pthread_join(thid, NULL) != 0)
	{	perror("pthread_mutex_lock() error");	exit(7);	}
} 


                                                           

void Test_Mutex_Others()
{
	pthread_mutex_t mutex; 
  	pthread_mutexattr_t attr; 
  	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);  //Test Function :pthread_mutexattr_settype()
  
  	if (pthread_mutexattr_init(&attr) != 0)    //Test Function :pthread_mutexattr_init()
  	{	perror("pthread_mutex_attr_init() error");	exit(1);	}                                                      
    else	printf("pthread_mutex_attr_init() success!!\n");                                                                           
  	
	if (pthread_mutex_init(&mutex, NULL) != 0) //Test Function :pthread_mutex_init()
  	{	perror("pthread_mutex_init() error");	exit(2);	}                                                                            
    else	printf("pthread_mutex_init() success!!\n");                                                                           
                                                                                
  	if (pthread_mutex_destroy(&mutex) != 0)  //Test Function :pthread_mutex_destroy()   
  	{	perror("pthread_mutex_destroy() error");	exit(3);	} 
    else	printf("pthread_mutex_destroy() success!!\n");                                                                           
  
  	if (pthread_mutexattr_destroy(&attr) != 0) 
  	{	perror("pthread_mutex_attr_destroy() error");	exit(4);	}       
    else	printf("pthread_mutex_attr_destroy() success!!\n");
}

void* TestFunc(void* arg)
{
    itpInit();
    
	printf("\nTest :\n");
  	printf("pthread_mutexattr_init()\n");
	printf("pthread_mutex_init()\n");
	printf("pthread_mutex_destroy()\n");
	printf("pthread_mutexattr_destroy()\n");
	printf("------------------\n");                                                               	
	Test_Mutex_Others();
	    
	printf("\nTest: pthread_mutex_unlock()\n------------------\n");                                                             
 	Test_pthread_mutex_unlock(); 
    
	printf("\nTest: pthread_mutex_trylock()\n------------------\n");                                                             
	Test_pthread_mutex_trylock();    
	                                                    
	printf("\nEnd Test\n");
}
