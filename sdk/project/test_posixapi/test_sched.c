#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ite/itp.h"


#define            LOOPCONSTANT        1000
#define            THREADS             3
#define 		   PRIORITY_ADJUSTMENT 5

pthread_mutex_t    mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t     attr;
int                i,j,k,l,policy;

static void checkResults(char *string, int rc) 
{
	if (rc) 
	{
		printf("Error on : %s, rc=%d\n",string, rc);
		exit(EXIT_FAILURE);
	}
	return;
}

void *threadfunc(void *parm)
{
  int   loop = 0;
  int   localProcessingCompleted = 0;
  int   numberOfLocalProcessingBursts = 0;
  int   processingCompletedThisBurst = 0;
  int   rc;

  printf("Entered secondary thread\n");
  for (loop=0; loop<LOOPCONSTANT; ++loop)
  {
    rc = pthread_mutex_lock(&mutex);
    checkResults("pthread_mutex_lock()\n", rc);
    /* Perform some not so important processing */
    i++, j++, k++, l++;

    rc = pthread_mutex_unlock(&mutex);
    checkResults("pthread_mutex_unlock()\n", rc);
    /* This work is not too important. Also, we just released a lock
       and would like to ensure that other threads get a chance in
       a more co-operative manner. This is an admittedly contrived
       example with no real purpose for doing the sched_yield().
       */
    sched_yield();
  }
  printf("Finished secondary thread\n");
  return NULL;
}

void Test_sched_yield()
{
  pthread_t             threadid[THREADS];
  int                   rc=0;
  int                   loop=0;


  rc = pthread_mutex_lock(&mutex);
  checkResults("pthread_mutex_lock()\n", rc);

  printf("Creating %d threads\n", THREADS);
  for (loop=0; loop<THREADS; ++loop) {
    rc = pthread_create(&threadid[loop], NULL, threadfunc, NULL);
    checkResults("pthread_create()\n", rc);
  }

  rc = pthread_mutex_unlock(&mutex);
  checkResults("pthread_mutex_unlock()\n", rc);

  printf("Wait for results\n");
  for (loop=0; loop<THREADS; ++loop) {
    rc = pthread_join(threadid[loop], NULL);
    checkResults("pthread_join()\n", rc);
  }

  pthread_mutex_destroy(&mutex);

  printf("Main completed\n");

}

static void Test_show_thread_priority( pthread_attr_t attr, int policy )
{
        int priority = sched_get_priority_max( policy );
        assert( priority != -1 );
        printf("max_priority = %d\n",priority);

        priority = sched_get_priority_min( policy );
        assert( priority != -1 );
        printf("min_priority = %d\n",priority);
}

void* TestFunc(void* arg)
{
  	itpInit();
  	
  	printf("Test: sched_yield()\n------------\n");
	Test_sched_yield();
	printf("\nTest: get_priority()\n------------\n");
	Test_show_thread_priority(attr,policy);
	
	system("PAUSE");
}

