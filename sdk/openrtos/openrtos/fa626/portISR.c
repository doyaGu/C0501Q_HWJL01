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
 * Components that can be compiled to either ARM or THUMB mode are
 * contained in port.c  The ISR routines, which can only be compiled
 * to ARM mode, are contained in this file.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "ite/itp.h"

/* Constants required to handle critical sections. */
#define portNO_CRITICAL_NESTING     ( ( unsigned long ) 0 )
volatile uint32_t ulCriticalNesting = 9999UL;

/* The tick interrupt has already executed. */
#if configUSE_TICKLESS_IDLE == 1
    extern volatile unsigned portBASE_TYPE uxTickInterruptExecuted;
#endif /* configUSE_TICKLESS_IDLE */

#if configCHECK_CRITICAL_TIME == 1
    static uint32_t uxTimeCriticalEntered = 0UL;
#endif

/*-----------------------------------------------------------*/

/* ISR to handle manual context switches (from a call to taskYIELD()). */
void vPortYieldProcessor( void ) __attribute__((interrupt("SWI"), naked));

/*
 * The scheduler can only be started from ARM mode, hence the inclusion of this
 * function here.
 */
void vPortISRStartFirstTask( void );
/*-----------------------------------------------------------*/

void vPortISRStartFirstTask( void )
{
    /* Simply start the scheduler.  This is included here as it can only be
    called from ARM mode. */
    portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * Called by portYIELD() or taskYIELD() to manually force a context switch.
 *
 * When a context switch is performed from the task level the saved task
 * context is made to look as if it occurred from within the tick ISR.  This
 * way the same restore context function can be used when restoring the context
 * saved from the ISR or that saved from a call to vPortYieldProcessor.
 */
void vPortYieldProcessor( void )
{
    /* Within an IRQ ISR the link register has an offset from the true return
    address, but an SWI ISR does not.  Add the offset manually so the same
    ISR return code can be used in both cases. */
    asm volatile ( "ADD     LR, LR, #4" );

    /* Perform the context switch.  First save the context of the current task. */
    portSAVE_CONTEXT();

    /* Find the highest priority task that is ready to run. */
    vTaskSwitchContext();

    /* Restore the context of the new task. */
    portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * The ISR used for the scheduler tick depends on whether the cooperative or
 * the preemptive scheduler is being used.
 */

#if configUSE_PREEMPTION == 0

    /* The cooperative scheduler requires a normal IRQ service routine to
    simply increment the system tick. */
    void vNonPreemptiveTick( void* arg )
    {
    #if configUSE_TICKLESS_IDLE == 1
        uxTickInterruptExecuted = 1UL;
    #endif /* configUSE_TICKLESS_IDLE */

        /* Clear tick timer interrupt indication. */
        ithTimerClearIntr(portTIMER);

        xTaskIncrementTick();
    }

#else  /* else preemption is turned on */

    /* The preemptive scheduler is defined as "naked" as the full context is
    saved on entry as part of the context switch. */
    void vPreemptiveTick( void* arg ) __attribute__((naked));
    void vPreemptiveTick( void* arg )
    {
        /* WARNING - Do not use local (stack) variables here.  Use globals
                     if you must! */

    #if configUSE_TICKLESS_IDLE == 1
        uxTickInterruptExecuted = 1UL;
    #endif /* configUSE_TICKLESS_IDLE */

        /* Clear tick timer interrupt indication. */
        ithTimerClearIntr(portTIMER);

        /* Increment the RTOS tick count, then look for the highest priority
        task that is ready to run. */
        if( xTaskIncrementTick() != pdFALSE )
        {
            vTaskSwitchContext();
        }

        /* Restore the context of the new task. */
        portRESTORE_CONTEXT();
    }

#endif
/*-----------------------------------------------------------*/

/*
 * The interrupt management utilities can only be called from ARM mode.  When
 * THUMB_INTERWORK is defined the utilities are defined as functions here to
 * ensure a switch to ARM mode.  When THUMB_INTERWORK is not defined then
 * the utilities are defined as macros in portmacro.h - as per other ports.
 */
#ifdef THUMB_INTERWORK

    void vPortDisableInterruptsFromThumb( void ) __attribute__ ((naked));
    void vPortEnableInterruptsFromThumb( void ) __attribute__ ((naked));

    void vPortDisableInterruptsFromThumb( void )
    {
        asm volatile (
            "STMDB  SP!, {R0}       \n\t"   /* Push R0.                                 */
            "MRS    R0, CPSR        \n\t"   /* Get CPSR.                                */
            "ORR    R0, R0, #0xC0   \n\t"   /* Disable IRQ, FIQ.                        */
            "MSR    CPSR, R0        \n\t"   /* Write back modified value.               */
            "LDMIA  SP!, {R0}       \n\t"   /* Pop R0.                                  */
            "BX     R14" );                 /* Return back to thumb.                    */
    }

    void vPortEnableInterruptsFromThumb( void )
    {
        asm volatile (
            "STMDB  SP!, {R0}       \n\t"   /* Push R0.                                 */
            "MRS    R0, CPSR        \n\t"   /* Get CPSR.                                */
            "BIC    R0, R0, #0xC0   \n\t"   /* Enable IRQ, FIQ.                         */
            "MSR    CPSR, R0        \n\t"   /* Write back modified value.               */
            "LDMIA  SP!, {R0}       \n\t"   /* Pop R0.                                  */
            "BX     R14" );                 /* Return back to thumb.                    */
    }

#endif /* THUMB_INTERWORK */

/* The code generated by the GCC compiler uses the stack in different ways at
different optimisation levels.  The interrupt flags can therefore not always
be saved to the stack.  Instead the critical section nesting level is stored
in a variable, which is then saved as part of the stack context. */
void vPortEnterCritical( void )
{
    /* Disable interrupts as per portDISABLE_INTERRUPTS();                          */
    asm volatile (
        "STMDB  SP!, {R0}           \n\t"   /* Push R0.                             */
        "MRS    R0, CPSR            \n\t"   /* Get CPSR.                            */
    #if configCHECK_CRITICAL_TIME == 2
        "ORR    R0, R0, #0x80       \n\t"   /* Disable IRQ.                         */
    #else
        "ORR    R0, R0, #0xC0       \n\t"   /* Disable IRQ, FIQ.                    */
    #endif
        "MSR    CPSR, R0            \n\t"   /* Write back modified value.           */
        "LDMIA  SP!, {R0}" );               /* Pop R0.                              */

#if configCHECK_CRITICAL_TIME == 1
    if (ulCriticalNesting == portNO_CRITICAL_NESTING)
    {
        uxTimeCriticalEntered = ithTimerGetCounter(ITH_TIMER5);
        //ithPrintf("uxTimeCriticalEntered in vPortEnterCritical() = 0x%X\n", uxTimeCriticalEntered);
    }
#endif // configCHECK_CRITICAL_TIME == 1

    /* Now interrupts are disabled ulCriticalNesting can be accessed
    directly.  Increment ulCriticalNesting to keep a count of how many times
    portENTER_CRITICAL() has been called. */
    ulCriticalNesting++;
}

void vPortExitCritical( void )
{
    if( ulCriticalNesting > portNO_CRITICAL_NESTING )
    {
        /* Decrement the nesting count as we are leaving a critical section. */
        ulCriticalNesting--;

        /* If the nesting level has reached zero then interrupts should be
        re-enabled. */
        if( ulCriticalNesting == portNO_CRITICAL_NESTING )
        {
        #if configCHECK_CRITICAL_TIME == 1
            uint32_t diff, time = ithTimerGetCounter(ITH_TIMER5);
            if (time >= uxTimeCriticalEntered)
            {
                diff = time - uxTimeCriticalEntered;
            }
            else
            {
                diff = (0xFFFFFFFF - uxTimeCriticalEntered) + 1 + time;
            }

        #endif // configCHECK_CRITICAL_TIME == 1

            /* Enable interrupts as per portEXIT_CRITICAL().                */
            asm volatile (
                "STMDB  SP!, {R0}       \n\t"   /* Push R0.                     */
                "MRS    R0, CPSR        \n\t"   /* Get CPSR.                    */
            #if configCHECK_CRITICAL_TIME == 2
                "BIC    R0, R0, #0x80   \n\t"   /* Enable IRQ.                  */
            #else
                "BIC    R0, R0, #0xC0   \n\t"   /* Enable IRQ, FIQ.             */
            #endif
                "MSR    CPSR, R0        \n\t"   /* Write back modified value.   */
                "LDMIA  SP!, {R0}" );           /* Pop R0.                      */

        #if configCHECK_CRITICAL_TIME == 1
            diff = (uint64_t)diff * 1000000 / ithGetBusClock();
            if (diff >= 1000)
            {
                ithPrintf("Critical time too long: %lu us (0x%X ~ 0x%X)\n", diff, uxTimeCriticalEntered, time);
            #ifndef NDEBUG
                itpPrintBacktrace();
            #endif
            }
        #endif // configCHECK_CRITICAL_TIME == 1
        }
    }
}

#if configCHECK_CRITICAL_TIME == 2

void vCheckCriticalTime( void* arg )
{
    if ( ulCriticalNesting > 0UL )
    {
        ithPrintf("ulCriticalNesting: %d\n", ulCriticalNesting);
    }
    ithTimerClearIntr(ITH_TIMER5);
    ithTimerSetCounter(ITH_TIMER5, 0);  // reset counter
}

#endif // configCHECK_CRITICAL_TIME == 2
