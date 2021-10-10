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
 * Implementation of functions defined in portable.h for the Atmel AT91R40008
 * port.
 *
 * Components that can be compiled to either ARM or THUMB mode are
 * contained in this file.  The ISR routines, which can only be compiled
 * to ARM mode are contained in portISR.c.
 *----------------------------------------------------------*/

/* Standard includes. */
#include <stdlib.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "ite/ith.h"

#ifndef configSYSTICK_CLOCK_HZ
    #define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
#endif

/* Constants required to setup the task context. */
#define portINITIAL_SPSR                ( ( StackType_t ) 0x1f ) /* System mode, ARM mode, interrupts enabled. */
#define portTHUMB_MODE_BIT              ( ( StackType_t ) 0x20 )
#define portINSTRUCTION_SIZE            ( ( StackType_t ) 4 )
#define portNO_CRITICAL_SECTION_NESTING ( ( StackType_t ) 0 )
/*-----------------------------------------------------------*/

/* Setup the timer to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/*
 * The scheduler can only be started from ARM mode, so
 * vPortISRStartFirstSTask() is defined in portISR.c.
 */
extern void vPortISRStartFirstTask( void );

/*-----------------------------------------------------------*/

/*
 * The number of SysTick increments that make up one tick period.
 */
#if configUSE_TICKLESS_IDLE == 1
    static unsigned long ulTimerReloadValueForOneTick = 0;
#endif

/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 24 bit resolution of the SysTick timer.
 */
#if configUSE_TICKLESS_IDLE == 1
    static unsigned long xMaximumPossibleSuppressedTicks = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Compensate for the CPU cycles that pass while the SysTick is stopped (low
 * power functionality only.
 */
#if configUSE_TICKLESS_IDLE == 1
    static unsigned long ulStoppedTimerCompensation = 0;
#endif /* configUSE_TICKLESS_IDLE */

/* The tick interrupt has already executed. */
#if configUSE_TICKLESS_IDLE == 1
    volatile unsigned BaseType_t uxTickInterruptExecuted = ( unsigned BaseType_t ) 0U;
#endif /* configUSE_TICKLESS_IDLE */

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
StackType_t *pxOriginalTOS;

    pxOriginalTOS = pxTopOfStack;

    /* To ensure asserts in tasks.c don't fail, although in this case the assert
    is not really required. */
    pxTopOfStack--;

    /* Setup the initial stack of the task.  The stack is set exactly as
    expected by the portRESTORE_CONTEXT() macro. */

    /* First on the stack is the return address - which in this case is the
    start of the task.  The offset is added to make the return address appear
    as it would within an IRQ ISR. */
    *pxTopOfStack = ( StackType_t ) pxCode + portINSTRUCTION_SIZE;
    pxTopOfStack--;

    *pxTopOfStack = ( StackType_t ) 0xaaaaaaaa;  /* R14 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pxOriginalTOS; /* Stack used when task starts goes in R13. */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x12121212;  /* R12 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x00000000;  /* R11 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x10101010;  /* R10 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x09090909;  /* R9 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x08080808;  /* R8 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x07070707;  /* R7 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x06060606;  /* R6 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x05050505;  /* R5 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x04040404;  /* R4 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x03030303;  /* R3 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x02020202;  /* R2 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x01010101;  /* R1 */
    pxTopOfStack--;

    /* When the task starts is will expect to find the function parameter in
    R0. */
    *pxTopOfStack = ( StackType_t ) pvParameters; /* R0 */
    pxTopOfStack--;

    /* The last thing onto the stack is the status register, which is set for
    system mode, with interrupts enabled. */
    *pxTopOfStack = ( StackType_t ) portINITIAL_SPSR;

    #ifdef THUMB_INTERWORK
    {
        /* We want the task to start in thumb mode. */
        *pxTopOfStack |= portTHUMB_MODE_BIT;
    }
    #endif

    pxTopOfStack--;

    /* Some optimisation levels use the stack differently to others.  This
    means the interrupt flags cannot always be stored on the stack and will
    instead be stored in a variable, which is then saved as part of the
    tasks context. */
    *pxTopOfStack = portNO_CRITICAL_SECTION_NESTING;

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    /* Start the timer that generates the tick ISR.  Interrupts are disabled
    here already. */
    prvSetupTimerInterrupt();

    /* Start the first task. */
    vPortISRStartFirstTask();

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* It is unlikely that the ARM port will require this function as there
    is nothing to return to.  */
}
/*-----------------------------------------------------------*/

/*
 * Setup the tick timer to generate the tick interrupts at the required frequency.
 */
static void prvSetupTimerInterrupt( void )
{
    /* Calculate the constants required to configure the tick interrupt. */
    #if configUSE_TICKLESS_IDLE == 1
    {
        ulTimerReloadValueForOneTick = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
        xMaximumPossibleSuppressedTicks = 0xfffffffUL / ( ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL );
        ulStoppedTimerCompensation = 45UL / ( configCPU_CLOCK_HZ / configSYSTICK_CLOCK_HZ );
    }
    #endif /* configUSE_TICKLESS_IDLE */

    /* Enable clock to the tick timer... */
    // Enable WCLK, Nothing TODO

    /* Stop the tick timer... */
    ithTimerDisable(portTIMER);

    /* Start with tick timer interrupts disabled... */
    ithIntrDisableIrq(portTIMER_INTR);

    /* Clear any pending tick timer interrupts... */
    ithTimerClearIntr(portTIMER);
    ithIntrClearIrq(portTIMER_INTR);

    /* Store interrupt handler function address in tick timer vector register...
    The ISR installed depends on whether the preemptive or cooperative
    scheduler is being used. */
    #if configUSE_PREEMPTION == 1
    {
        extern void ( vPreemptiveTick )( void* arg );
        ithIntrRegisterHandlerIrq(portTIMER_INTR, vPreemptiveTick, NULL);
    }
    #else  // else use cooperative scheduler
    {
        extern void ( vNonPreemptiveTick )( void* arg );
        ithIntrRegisterHandlerIrq(portTIMER_INTR, vNonPreemptiveTick, NULL);
    }
    #endif

    /* Tick timer interrupt level-sensitive */
    ithIntrSetTriggerModeIrq(portTIMER_INTR, ITH_INTR_EDGE);
    ithTimerCtrlEnable(portTIMER, ITH_TIMER_UPCOUNT);
    ithTimerCtrlEnable(portTIMER, ITH_TIMER_PERIODIC);

    /* Enable the tick timer interrupt... */
    ithIntrEnableIrq(portTIMER_INTR);

    /* Calculate timer compare value to achieve the desired tick rate... */
    ithTimerSetCounter(portTIMER, 0);
    ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);

    /* Start tick timer... */
    ithTimerEnable(portTIMER);

#if (configCHECK_CRITICAL_TIME == 1) || (configCHECK_CRITICAL_TIME == 3)
    ithTimerCtrlEnable(ITH_TIMER5, ITH_TIMER_UPCOUNT);
    ithTimerSetCounter(ITH_TIMER5, 0);
    ithTimerEnable(ITH_TIMER5);

#elif configCHECK_CRITICAL_TIME == 2
    extern void ( vCheckCriticalTime )( void* arg );
    ithIntrRegisterHandlerFiq(ITH_INTR_TIMER5, vCheckCriticalTime, NULL);
    ithIntrSetTriggerModeFiq(ITH_INTR_TIMER5, ITH_INTR_EDGE);
    ithTimerCtrlEnable(ITH_TIMER5, ITH_TIMER_UPCOUNT);
    ithTimerCtrlEnable(ITH_TIMER5, ITH_TIMER_PERIODIC);
    ithIntrEnableFiq(ITH_INTR_TIMER5);
    ithTimerSetCounter(ITH_TIMER5, 0);
    ithTimerSetMatch(ITH_TIMER5, configCPU_CLOCK_HZ * 5);
    ithTimerEnable(ITH_TIMER5);

#endif // (configCHECK_CRITICAL_TIME == 1) || (configCHECK_CRITICAL_TIME == 3)
}
/*-----------------------------------------------------------*/

extern void itpErrorStackOverflow(void) __attribute__ ((naked));

void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char *pcTaskName )
{
    /* Check pcTaskName for the name of the offending task, or pxCurrentTCB
    if pcTaskName has itself been corrupted. */
    ( void ) pxTask;
    ( void ) pcTaskName;
    itpErrorStackOverflow();
    for( ;; );
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

/*
 * Lock routine called by Newlib on malloc / realloc / free entry to guarantee a
 * safe section as memory allocation management uses global data.
 * See the aforementioned details.
 */
void __malloc_lock(struct _reent *ptr)
{
    vTaskSuspendAll();
}
/*-----------------------------------------------------------*/

/*
 * Unlock routine called by Newlib on malloc / realloc / free exit to guarantee
 * a safe section as memory allocation management uses global data.
 * See the aforementioned details.
 */
void __malloc_unlock(struct _reent *ptr)
{
    xTaskResumeAll();
}
/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats( void )
{
#if defined(configCHECK_CRITICAL_TIME) && (configCHECK_CRITICAL_TIME != 2)
    /* This function configures a timer that is used as the time base when
    collecting run time statistical information - basically the percentage
    of CPU time that each task is utilising.  It is called automatically when
    the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
    to 1). */

    /* Reset Timer 0 */
    ithTimerSetCounter(ITH_TIMER5, 0);

    /* Just count up. */
    ithTimerCtrlEnable(ITH_TIMER5, ITH_TIMER_UPCOUNT);

    /* Start the counter. */
    ithTimerEnable(ITH_TIMER5);

#endif // defined(configCHECK_CRITICAL_TIME) && (configCHECK_CRITICAL_TIME != 2)
}
/*-----------------------------------------------------------*/

unsigned long ulGetRunTimeCounterValue( void )
{
    /* Just return 32 bits. */
    return ( unsigned long ) xTaskGetTickCountFromISR();
}
/*-----------------------------------------------------------*/

#if configUSE_TICKLESS_IDLE == 1

    __attribute__((weak)) void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
    {
    unsigned long ulReloadValue, ulCompleteTickPeriods, ulCompletedSysTickIncrements;
    TickType_t xModifiableIdleTime;

        /* Make sure the SysTick reload value does not overflow the counter. */
        if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
        {
            xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
        }

        /* Calculate the reload value required to wait xExpectedIdleTime
        tick periods.  -1 is used because this code will execute part way
        through one of the tick periods, and the fraction of a tick period is
        accounted for later. */
        ulReloadValue = ( ulTimerReloadValueForOneTick * ( xExpectedIdleTime - 1UL ) );
        if( ulReloadValue > ulStoppedTimerCompensation )
        {
            ulReloadValue -= ulStoppedTimerCompensation;
        }

        /* Stop the SysTick momentarily.  The time the SysTick is stopped for
        is accounted for as best it can be, but using the tickless mode will
        inevitably result in some tiny drift of the time maintained by the
        kernel with respect to calendar time. */
        ithTimerDisable(portTIMER);

        /* Adjust the reload value to take into account that the current time
        slice is already partially complete. */
        ulReloadValue += ulTimerReloadValueForOneTick - ithTimerGetCounter(portTIMER);

        /* Enter a critical section but don't use the taskENTER_CRITICAL()
        method as that will mask interrupts that should exit sleep mode. */
        portDISABLE_INTERRUPTS();

        /* If a context switch is pending or a task is waiting for the scheduler
        to be unsuspended then abandon the low power entry. */
        if( eTaskConfirmSleepModeStatus() == eAbortSleep )
        {
            /* Restart SysTick. */
            ithTimerEnable(portTIMER);

            /* Re-enable interrupts - see comments above the cpsid instruction()
            above. */
            portENABLE_INTERRUPTS();
        }
        else
        {
            /* Set the new reload value. */
            ithTimerSetMatch(portTIMER, ulReloadValue);

            /* Clear the SysTick count flag and set the count value back to
            zero. */
            ithTimerSetCounter(portTIMER, 0);

            /* Restart SysTick. */
            ithTimerEnable(portTIMER);

            /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
            set its parameter to 0 to indicate that its implementation contains
            its own wait for interrupt or wait for event instruction, and so wfi
            should not be executed again.  However, the original expected idle
            time variable must remain unmodified, so a copy is taken. */
            xModifiableIdleTime = xExpectedIdleTime;
            configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
            if( xModifiableIdleTime > 0 )
            {
                uxTickInterruptExecuted = 0UL;
                portENABLE_INTERRUPTS();
                ithCpuDoze();
                portDISABLE_INTERRUPTS();
            }
            configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

            /* Stop SysTick.  Again, the time the SysTick is stopped for is
            accounted for as best it can be, but using the tickless mode will
            inevitably result in some tiny drift of the time maintained by the
            kernel with respect to calendar time. */
            ithTimerDisable(portTIMER);

            /* Re-enable interrupts - see comments above the cpsid instruction()
            above. */
            portENABLE_INTERRUPTS();

            if( uxTickInterruptExecuted != 0 )
            {
                /* The tick interrupt handler will already have pended the tick
                processing in the kernel.  As the pending tick will be
                processed as soon as this function exits, the tick value
                maintained by the tick is stepped forward by one less than the
                time spent waiting. */
                ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
            }
            else
            {
                /* Something other than the tick interrupt ended the sleep.
                Work out how long the sleep lasted. */
                ulCompletedSysTickIncrements = ithTimerGetCounter(portTIMER);

                /* How many complete tick periods passed while the processor
                was waiting? */
                ulCompleteTickPeriods = ulCompletedSysTickIncrements / ulTimerReloadValueForOneTick;

                /* The reload value is set to whatever fraction of a single tick
                period remains. */
                ithTimerSetCounter(portTIMER, ulCompletedSysTickIncrements - ( ulCompleteTickPeriods * ulTimerReloadValueForOneTick ) );
            }

            /* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
            again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
            value. */
            ithTimerSetMatch(portTIMER, configCPU_CLOCK_HZ / configTICK_RATE_HZ);
            ithTimerEnable(portTIMER);

            vTaskStepTick( ulCompleteTickPeriods );
        }
    }

#endif /* #if configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

#if configCHECK_CRITICAL_TIME == 3

#define TASK_NAME_FOR_CHECK  "USBEX_ThreadFun"

static uint32_t uxTimeTaskSwitched;

void vTaskSwitchedOut( void )
{
    //ithPrintf("vTaskSwitchedOut: %s\n", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
    if (strcmp(pcTaskGetTaskName(xTaskGetCurrentTaskHandle()), TASK_NAME_FOR_CHECK) == 0)
    {
        uint32_t diff, time = ithTimerGetCounter(ITH_TIMER5);

        if (time >= uxTimeTaskSwitched)
        {
            diff = time - uxTimeTaskSwitched;
        }
        else
        {
            diff = (0xFFFFFFFF - uxTimeTaskSwitched) + 1 + time;
        }

        diff = (uint64_t)diff * 1000000 / ithGetBusClock();
        if (diff >= 20000UL)
        {
            ithPrintf("%s task too long to switch out: %d us (0x%X ~ 0x%X)\n", TASK_NAME_FOR_CHECK, diff, uxTimeTaskSwitched, time);
        }

        uxTimeTaskSwitched = time;
    }
}

void vTaskSwitchedIn( void )
{
    //ithPrintf("vTaskSwitchedIn: %s\n", pcTaskGetTaskName(xTaskGetCurrentTaskHandle()));
    if (strcmp(pcTaskGetTaskName(xTaskGetCurrentTaskHandle()), TASK_NAME_FOR_CHECK) == 0)
    {
        uint32_t diff, time = ithTimerGetCounter(ITH_TIMER5);

        if (time >= uxTimeTaskSwitched)
        {
            diff = time - uxTimeTaskSwitched;
        }
        else
        {
            diff = (0xFFFFFFFF - uxTimeTaskSwitched) + 1 + time;
        }

        diff = (uint64_t)diff * 1000000 / ithGetBusClock();
        if (diff >= 20000UL)
        {
            ithPrintf("%s task too long to switch in: %d us (0x%X ~ 0x%X)\n", TASK_NAME_FOR_CHECK, diff, uxTimeTaskSwitched, time);
        }

        uxTimeTaskSwitched = time;
    }
}

#endif // configCHECK_CRITICAL_TIME
