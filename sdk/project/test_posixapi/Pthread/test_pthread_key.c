#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


pthread_key_t   tlsKey = 0;

static void checkResults(char *string, int rc) 
{
	if (rc) 
	{
		printf("Error on : %s, rc=%d\n",string, rc);
		exit(EXIT_FAILURE);
	}
}

void globalDestructor(void *value)
{
	printf("In global data destructor\n");
	free(value);
	pthread_setspecific(tlsKey, NULL);
}

void Test_pthread_key()
{
	int                   rc=0;
  	int                   i=0;

  	printf("Create a thread local storage key\n");
  	rc = pthread_key_create(&tlsKey, globalDestructor);
  	checkResults("pthread_key_create()\n", rc);
  	/* The key can now be used from all threads */

  	printf("Delete a thread local storage key\n");
  	rc = pthread_key_delete(tlsKey);
  	checkResults("pthread_key_delete()\n", rc);

  	printf("- The key should not be used from any thread\n");
  	printf("- after destruction.\n");
  	/* The key and any remaining values are now gone. */
  	printf("Main completed\n");
}

void* TestFunc(void* arg)
{
	itpInit();
	
	printf("\nTest: pthread_key_create() , pthread_key_delete()\n------------------\n");
	Test_pthread_key();
    printf("\nEnd Test\n");
}
