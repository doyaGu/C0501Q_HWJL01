#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

extern void* TestFunc_count(void* arg);
extern void* TestFunc_IRQ(void* arg);
extern void* TestFunc_timeout(void* arg);
extern void* TestFunc_PWM(void* arg);

int main(void)
{
    pthread_t task;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
#ifdef CFG_TIMER_BASIC_TEST    
    pthread_create(&task, &attr, TestFunc_count, NULL);
#endif
#ifdef CFG_TIMER_TIMEOUT_TEST
    pthread_create(&task, &attr, TestFunc_timeout, NULL);
#endif
#ifdef CFG_TIMER_IRQ_TEST
    pthread_create(&task, &attr, TestFunc_IRQ, NULL);
#endif
#ifdef CFG_TIMER_PWM_TEST    
    pthread_create(&task, &attr, TestFunc_PWM, NULL);
#endif
    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
