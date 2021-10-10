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
 * Implementation of functions defined in portable.h for the OpenRISC port.
 *----------------------------------------------------------*/

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Processor constants. */
#include "port_spr_defs.h"

/* Jump buffer */
#include <setjmp.h>
static jmp_buf jmpbuf;

#ifndef configSYSTICK_CLOCK_HZ
    #define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
#endif

/* Tick Timer Interrupt handler */
void vTickHandler( void );

/* Setup the timer to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/* forward decleation */
void vPortDisableInterrupts( void );
void vPortEnableInterrupts( void );

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
 * portSAVE_CONTEXT had been called. Context layout is described in
 * portmarco.h
 *
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    unsigned portLONG uTaskSR = mfspr(SPR_SR);
    uTaskSR &= ~SPR_SR_SM;                  // User mode
    uTaskSR |= (SPR_SR_TEE | SPR_SR_IEE);   // Tick interrupt enable, All External interupt enable

    // allocate redzone
    pxTopOfStack -= REDZONE_SIZE/4;

    /* Setup the initial stack of the task.  The stack is set exactly as
    expected by the portRESTORE_CONTEXT() macro. */
    *(--pxTopOfStack) = ( StackType_t ) pxCode;         // SPR_EPCR_BASE(0)
    *(--pxTopOfStack) = ( StackType_t ) uTaskSR;        // SPR_ESR_BASE(0)

    *(--pxTopOfStack) = ( StackType_t ) 0x00000031;     // r31
    *(--pxTopOfStack) = ( StackType_t ) 0x00000030;     // r30
    *(--pxTopOfStack) = ( StackType_t ) 0x00000029;     // r29
    *(--pxTopOfStack) = ( StackType_t ) 0x00000028;     // r28
    *(--pxTopOfStack) = ( StackType_t ) 0x00000027;     // r27
    *(--pxTopOfStack) = ( StackType_t ) 0x00000026;     // r26
    *(--pxTopOfStack) = ( StackType_t ) 0x00000025;     // r25
    *(--pxTopOfStack) = ( StackType_t ) 0x00000024;     // r24
    *(--pxTopOfStack) = ( StackType_t ) 0x00000023;     // r23
    *(--pxTopOfStack) = ( StackType_t ) 0x00000022;     // r22
    *(--pxTopOfStack) = ( StackType_t ) 0x00000021;     // r21
    *(--pxTopOfStack) = ( StackType_t ) 0x00000020;     // r20
    *(--pxTopOfStack) = ( StackType_t ) 0x00000019;     // r19
    *(--pxTopOfStack) = ( StackType_t ) 0x00000018;     // r18
    *(--pxTopOfStack) = ( StackType_t ) 0x00000017;     // r17
    *(--pxTopOfStack) = ( StackType_t ) 0x00000016;     // r16
    *(--pxTopOfStack) = ( StackType_t ) 0x00000015;     // r15
    *(--pxTopOfStack) = ( StackType_t ) 0x00000014;     // r14
    *(--pxTopOfStack) = ( StackType_t ) 0x00000013;     // r13
    *(--pxTopOfStack) = ( StackType_t ) 0x00000012;     // r12
    *(--pxTopOfStack) = ( StackType_t ) 0x00000011;     // r11
    *(--pxTopOfStack) = ( StackType_t ) 0x00000010;     // r10
    *(--pxTopOfStack) = ( StackType_t ) 0x00000008;     // r8
    *(--pxTopOfStack) = ( StackType_t ) 0x00000007;     // r7
    *(--pxTopOfStack) = ( StackType_t ) 0x00000006;     // r6
    *(--pxTopOfStack) = ( StackType_t ) 0x00000005;     // r5
    *(--pxTopOfStack) = ( StackType_t ) 0x00000004;     // r4
    *(--pxTopOfStack) = ( StackType_t ) pvParameters;   // task argument
    *(--pxTopOfStack) = ( StackType_t ) 0x00000002;     // r2
    *(--pxTopOfStack) = ( StackType_t ) pxCode;         // PC

    return pxTopOfStack;
}

BaseType_t xPortStartScheduler( void )
{
    if(setjmp((void *)jmpbuf) == 0) {
    /* Start the timer that generates the tick ISR.  Interrupts are disabled
    here already. */
    prvSetupTimerInterrupt();

    /* Start the first task. */
        asm volatile (
        " .global   pxCurrentTCB    \n\t"
        /*   restore stack pointer          */
        "   l.movhi r3, hi(pxCurrentTCB)        \n\t"
        "   l.ori   r3, r3, lo(pxCurrentTCB)    \n\t"
        "   l.lwz   r3, 0x0(r3)     \n\t"
        "   l.lwz   r1, 0x0(r3)     \n\t"
        /*   restore context                */
        "   l.lwz   r9,  0x00(r1)   \n\t"
        "   l.lwz   r2,  0x04(r1)   \n\t"
        "   l.lwz   r6,  0x14(r1)   \n\t"
        "   l.lwz   r7,  0x18(r1)   \n\t"
        "   l.lwz   r8,  0x1C(r1)   \n\t"
        "   l.lwz   r10, 0x20(r1)   \n\t"
        "   l.lwz   r11, 0x24(r1)   \n\t"
        "   l.lwz   r12, 0x28(r1)   \n\t"
        "   l.lwz   r13, 0x2C(r1)   \n\t"
        "   l.lwz   r14, 0x30(r1)   \n\t"
        "   l.lwz   r15, 0x34(r1)   \n\t"
        "   l.lwz   r16, 0x38(r1)   \n\t"
        "   l.lwz   r17, 0x3C(r1)   \n\t"
        "   l.lwz   r18, 0x40(r1)   \n\t"
        "   l.lwz   r19, 0x44(r1)   \n\t"
        "   l.lwz   r20, 0x48(r1)   \n\t"
        "   l.lwz   r21, 0x4C(r1)   \n\t"
        "   l.lwz   r22, 0x50(r1)   \n\t"
        "   l.lwz   r23, 0x54(r1)   \n\t"
        "   l.lwz   r24, 0x58(r1)   \n\t"
        "   l.lwz   r25, 0x5C(r1)   \n\t"
        "   l.lwz   r26, 0x60(r1)   \n\t"
        "   l.lwz   r27, 0x64(r1)   \n\t"
        "   l.lwz   r28, 0x68(r1)   \n\t"
        "   l.lwz   r29, 0x6C(r1)   \n\t"
        "   l.lwz   r30, 0x70(r1)   \n\t"
        "   l.lwz   r31, 0x74(r1)   \n\t"
        /*  restore SPR_ESR_BASE(0), SPR_EPCR_BASE(0) */
        "   l.lwz   r3,  0x78(r1)   \n\t"
        "   l.lwz   r4,  0x7C(r1)   \n\t"
        "   l.mtspr r0,  r3, %1     \n\t"
        "   l.mtspr r0,  r4, %2     \n\t"
        /*   restore clobber register     */
        "   l.lwz   r3,  0x08(r1)   \n\t"
        "   l.lwz   r4,  0x0C(r1)   \n\t"
        "   l.lwz   r5,  0x10(r1)   \n\t"
        "   l.addi  r1,  r1, %0     \n\t"
        "   l.rfe                   \n\t"
        "   l.nop                   \n\t"
        :
        :   "n"(STACKFRAME_SIZE),
            "n"(SPR_ESR_BASE),
            "n"(SPR_EPCR_BASE)
        );

    /* Should not get here! */
    } else {
        /* Retrun by vPortEndScheduler */
    }

    return 0;
}

void vPortEndScheduler( void )
{
    mtspr(SPR_SR, mfspr(SPR_SR) & (~SPR_SR_TEE));   // Tick stop
    //longjmp((void *)jmpbuf, 1);                       // return to xPortStartScheduler
}

/*
 * Setup the tick timer to generate the tick interrupts at the required frequency.
 */
static void prvSetupTimerInterrupt( void )
{
    const unsigned portLONG ulTickPeriod = configCPU_CLOCK_HZ / configTICK_RATE_HZ;

    /* Calculate the constants required to configure the tick interrupt. */
    #if configUSE_TICKLESS_IDLE == 1
    {
        ulTimerReloadValueForOneTick = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
        xMaximumPossibleSuppressedTicks = 0xfffffffUL / ( ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL );
        ulStoppedTimerCompensation = 45UL / ( configCPU_CLOCK_HZ / configSYSTICK_CLOCK_HZ );
    }
    #endif /* configUSE_TICKLESS_IDLE */

    // Disable tick timer exception recognition
    mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);

    // clears interrupt
    mtspr(SPR_TTMR, mfspr(SPR_TTMR) & ~(SPR_TTMR_IP));

    // Set period of one cycle, restartable mode
    mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT | (ulTickPeriod & SPR_TTMR_PERIOD));

    // Reset counter
    mtspr(SPR_TTCR, 0);

    // set OR1200 to accept exceptions
    mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);
}

void vPortDisableInterrupts( void )
{
    mtspr(SPR_SR, mfspr(SPR_SR) & ~(SPR_SR_TEE|SPR_SR_IEE));    // Tick, interrupt stop
}

void vPortEnableInterrupts( void )
{
    mtspr(SPR_SR, mfspr(SPR_SR) | (SPR_SR_TEE|SPR_SR_IEE));     // Tick, interrupt start
}

/*
 * naked attribute is ignored or32-elf-gcc 4.5.1-or32-1.0rc1
 * use assemble routines in portasm.S
 */
#if 0
void vTickHandler( void )
{
    // clears interrupt
    mtspr(SPR_TTMR, mfspr(SPR_TTMR) & ~(SPR_TTMR_IP));

    /* Increment the RTOS tick count, then look for the highest priority
       task that is ready to run. */
    vTaskIncrementTick();

    // The cooperative scheduler requires a normal simple Tick ISR to
    // simply increment the system tick.
#if configUSE_PREEMPTION == 0
    // nothing to do here
#else
    /* Save the context of the current task. */
    portSAVE_CONTEXT();

    /* Find the highest priority task that is ready to run. */
    vTaskSwitchContext();

    portRESTORE_CONTEXT();
#endif
}
#endif

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
        mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);

        /* Adjust the reload value to take into account that the current time
        slice is already partially complete. */
        ulReloadValue += ulTimerReloadValueForOneTick - mfspr(SPR_TTCR);

        /* Enter a critical section but don't use the taskENTER_CRITICAL()
        method as that will mask interrupts that should exit sleep mode. */
        portDISABLE_INTERRUPTS();

        /* If a context switch is pending or a task is waiting for the scheduler
        to be unsuspended then abandon the low power entry. */
        if( eTaskConfirmSleepModeStatus() == eAbortSleep )
        {
            /* Restart SysTick. */
            mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);

            /* Re-enable interrupts - see comments above the cpsid instruction()
            above. */
            portENABLE_INTERRUPTS();
        }
        else
        {
            /* Set the new reload value. */
            mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT | (ulReloadValue & SPR_TTMR_PERIOD));

            /* Clear the SysTick count flag and set the count value back to
            zero. */
            mtspr(SPR_TTCR, 0);

            /* Restart SysTick. */
            mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);

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
            mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_TEE);

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
                ulCompletedSysTickIncrements = mfspr(SPR_TTCR);

                /* How many complete tick periods passed while the processor
                was waiting? */
                ulCompleteTickPeriods = ulCompletedSysTickIncrements / ulTimerReloadValueForOneTick;

                /* The reload value is set to whatever fraction of a single tick
                period remains. */
                mtspr(SPR_TTCR, ulCompletedSysTickIncrements - ( ulCompleteTickPeriods * ulTimerReloadValueForOneTick ));
            }

            /* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
            again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
            value. */
            mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT | ((configCPU_CLOCK_HZ / configTICK_RATE_HZ) & SPR_TTMR_PERIOD));
            mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);

            vTaskStepTick( ulCompleteTickPeriods );
        }
    }

#endif /* #if configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/
