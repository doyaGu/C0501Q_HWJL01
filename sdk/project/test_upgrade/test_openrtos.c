#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

#define TEST_STACK_SIZE 102400

extern void* TestFunc(void* arg);

int main(void)
{
    pthread_t task;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    attr.stacksize = TEST_STACK_SIZE;

    pthread_create(&task, &attr, TestFunc, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
