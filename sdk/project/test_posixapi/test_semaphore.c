#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "ite/itp.h"

//The following example attempts to decrement a semaphore with a current value of zero.
void Test_sem_trywait()
{
  sem_t my_semaphore;
  int value;
  int rc;

  rc = sem_init(&my_semaphore, 0, 1);// Initializes an unnamed semaphore, my_semaphore, that will be used by threads of the current process. Its value is set to 1.

  sem_getvalue(&my_semaphore, &value);// Retrieves the value of a semaphore before and after it is decremented by sem_wait().
  printf("The initial value of the semaphore is %d\n", value);
  
  sem_wait(&my_semaphore);  // A semaphore with an initial value of 1. The value is decremented by calling sem_wait().
  
  sem_getvalue(&my_semaphore, &value);// Retrieves the value of a semaphore before and after it is decremented by sem_wait().
  printf("The value of the semaphore after the wait is %d\n", value);
  
  rc = sem_trywait(&my_semaphore);

  if ((rc == -1) && (errno == EAGAIN)) {
   printf("sem_trywait did not decrement the semaphore\n");
  }
  
   rc = sem_destroy(&my_semaphore);//The semaphore is destroyed using sem_destroy().
   printf("Destroy the semaphore success\n");
}

//The following example initializes an unnamed semaphore and posts to it, incrementing its value by 1.
void Test_sem_post()
{
  sem_t my_semaphore;
  int value;

  sem_init(&my_semaphore, 0, 10);
  sem_getvalue(&my_semaphore, &value);
  printf("The initial value of the semaphore is %d\n", value);
  sem_post(&my_semaphore);
  sem_getvalue(&my_semaphore, &value);
  printf("The value of the semaphore after the post is %d\n", value);
}

void* TestFunc(void* arg)
{
	itpInit();

	printf("\nTest: sem_init(),sem_getvalue(),sem_wait(),sem_trywait(),sem_destroy()\n---------------\n");
	Test_sem_trywait();
	printf("\nTest: sem_post()\n---------------\n");
	Test_sem_post();
	
	printf("\nEnd the test\n");	
}

