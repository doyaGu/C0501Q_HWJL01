#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "ite/itp.h"

#define TEST_STACK_SIZE 102400*2

extern void* TestFunc(void* arg);


static void* main_task(void* arg)
{
    pthread_t task;
    pthread_attr_t attr;
    
    // create test task
    pthread_attr_init(&attr);
    attr.stacksize = TEST_STACK_SIZE;

    pthread_create(&task, &attr, TestFunc, NULL);
    
    // do the test
    for (;;)
    {
        usleep(1*1000*1000);
    }
    
    return NULL;
}

int main(void)
{
    pthread_t task;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    attr.stacksize = TEST_STACK_SIZE;

    pthread_create(&task, &attr, main_task, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
