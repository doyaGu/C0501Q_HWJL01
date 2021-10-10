/*
    OpenRTOS V8.2.3 - Copyright (C) Wittenstein High Integrity Systems.

    OpenRTOS is distributed exclusively by Wittenstein High Integrity Systems,
    and is subject to the terms of the License granted to your organization,
    including its warranties and limitations on distribution.  It cannot be
    copied or reproduced in any way except as permitted by the License.

    Licenses are issued for each concurrent user working on a specified product
    line.

    WITTENSTEIN high integrity systems is a trading name of WITTENSTEIN
    aerospace & simulation ltd, Registered Office: Brown's Court, Long Ashton
    Business Park, Yanley Lane, Long Ashton, Bristol, BS41 9LB, UK.
    Tel: +44 (0) 1275 395 600, fax: +44 (0) 1275 393 630.
    E-mail: info@HighIntegritySystems.com
    Registered in England No. 3711047; VAT No. GB 729 1583 15

    http://www.HighIntegritySystems.com
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the RISC-V port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"

/* A variable is used to keep track of the critical section nesting.  This
variable has to be stored as part of the task context and must be initialised to
a non zero value to ensure interrupts don't inadvertently become unmasked before
the scheduler starts.  As it is stored as part of the task context it will
automatically be set to 0 when the first task is started. */
static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/* Contains context when starting scheduler, save all 31 registers */
#ifdef __gracefulExit
BaseType_t xStartContext[31] = {0};
#endif

volatile uint64_t* mtime;
volatile uint64_t* timecmp;

/*
 * Handler for timer interrupt
 */
void vPortSysTickHandler( void );

/*
 * Setup the timer to generate the tick interrupts.
 */
void vPortSetupTimer( void );

/*
 * Set the next interval for the timer
 */
static void prvSetNextTimerInterrupt( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/

/* Sets the next timer interrupt
 * Reads previous timer compare register, and adds tickrate */
static void prvSetNextTimerInterrupt(void)
{
    // TODO: RISCV
    //*timecmp += configTICK_CLOCK_HZ / configTICK_RATE_HZ;
}
/*-----------------------------------------------------------*/

/* Sets and enable the timer interrupt */
void vPortSetupTimer(void)
{
    // TODO: RISCV
    //*timecmp += *mtime+(configTICK_CLOCK_HZ / configTICK_RATE_HZ);

    /* Enable timer interupt */
    __asm volatile("csrs mie,%0"::"r"(0x80));
}
/*-----------------------------------------------------------*/

void prvTaskExitError( void )
{
    /* A function that implements a task must not exit or attempt to return to
    its caller as there is nothing to return to.  If a task wants to exit it
    should instead call vTaskDelete( NULL ).

    Artificially force an assert() to be triggered if configASSERT() is
    defined, then stop here so application writers can catch the error. */
    configASSERT( uxCriticalNesting == ~0UL );
    portDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

/* Clear current interrupt mask and set given mask */
void vPortClearInterruptMask(int mask)
{
    __asm volatile("csrw mie, %0"::"r"(mask));
}
/*-----------------------------------------------------------*/

/* Set interrupt mask and return current interrupt enable register */
int vPortSetInterruptMask(void)
{
    int ret;
    __asm volatile("csrr %0,mie":"=r"(ret));
    __asm volatile("csrc mie,%0"::"i"(7));
    return ret;
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    /* Simulate the stack frame as it would be created by a context switch
    interrupt. */
    register int *tp asm("x3");
    pxTopOfStack--;
    *pxTopOfStack = (portSTACK_TYPE)pxCode;         /* Start address */
    pxTopOfStack -= 22;
    *pxTopOfStack = (portSTACK_TYPE)pvParameters;   /* Register a0 */
    pxTopOfStack -= 6;
    *pxTopOfStack = (portSTACK_TYPE)tp; /* Register thread pointer */
    pxTopOfStack -= 3;
    *pxTopOfStack = (portSTACK_TYPE)prvTaskExitError; /* Register ra */

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

void vPortSysTickHandler( void )
{
    prvSetNextTimerInterrupt();

    /* Increment the RTOS tick. */
    if( xTaskIncrementTick() != pdFALSE )
    {
        vTaskSwitchContext();
    }
}

/*-----------------------------------------------------------*/
/*
 * malloc, realloc and free are meant to be called through respectively
 * pvPortMalloc, pvPortRealloc and vPortFree.
 * The latter functions call the former ones from within sections where tasks
 * are suspended, so the latter functions are task-safe. __malloc_lock and
 * __malloc_unlock use the same mechanism to also keep the former functions
 * task-safe as they may be called directly from Newlib's functions.
 * However, all these functions are interrupt-unsafe and SHALL THEREFORE NOT BE
 * CALLED FROM WITHIN AN INTERRUPT, because __malloc_lock and __malloc_unlock do
 * not call portENTER_CRITICAL and portEXIT_CRITICAL in order not to disable
 * interrupts during memory allocation management as this may be a very time-
 * consuming process.
 */

#if 1
/*
 * Lock routine called by Newlib on malloc / realloc / free entry to guarantee a
 * safe section as memory allocation management uses global data.
 * See the aforementioned details.
 */
void __malloc_lock(struct _reent *ptr)
{
    vTaskSuspendAll();
}

/*
 * Unlock routine called by Newlib on malloc / realloc / free exit to guarantee
 * a safe section as memory allocation management uses global data.
 * See the aforementioned details.
 */
void __malloc_unlock(struct _reent *ptr)
{
    xTaskResumeAll();
}
#endif // 0
/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats( void )
{
    /* This function configures a timer that is used as the time base when
    collecting run time statistical information - basically the percentage
    of CPU time that each task is utilising.  It is called automatically when
    the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
    to 1). */
}

unsigned long ulGetRunTimeCounterValue( void )
{
    /* Just return 32 bits. */
    return ( unsigned long ) xTaskGetTickCountFromISR() * 1000 * portTICK_PERIOD_MS;
}
/*-----------------------------------------------------------*/

