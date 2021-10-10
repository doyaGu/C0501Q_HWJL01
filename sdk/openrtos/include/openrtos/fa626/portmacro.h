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

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  uint32_t
#define portBASE_TYPE   long

#ifndef __ASSEMBLER__
typedef portSTACK_TYPE  StackType_t;
typedef long            BaseType_t;
typedef unsigned long   UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL

    /* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
    not need to be guarded with a critical section. */
    #define portTICK_TYPE_IS_ATOMIC 1
#endif
#endif // __ASSEMBLER__
/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH            ( -1 )
#define portTICK_PERIOD_MS          ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          8
#define portYIELD()                 asm volatile ( "SWI 0" )
#define portNOP()                   asm volatile ( "NOP" )

/*-----------------------------------------------------------*/

/* Task utilities. */

/*
 * portRESTORE_CONTEXT, portRESTORE_CONTEXT, portENTER_SWITCHING_ISR
 * and portEXIT_SWITCHING_ISR can only be called from ARM mode, but
 * are included here for efficiency.  An attempt to call one from
 * THUMB mode code will result in a compile time error.
 */
#ifdef CFG_GCC_LTO
#define portRESTORE_CONTEXT()                                           \
{                                                                       \
extern volatile void * volatile pxCurrentTCB;                           \
extern volatile unsigned portLONG ulCriticalNesting;                    \
                                                                        \
    /* Set the LR to the task stack. */                                 \
    asm volatile (                                                      \
    "LDR    R0, 1f /* =pxCurrentTCB */                          \n\t"   \
    "LDR    R0, [R0]                                            \n\t"   \
    "LDR    LR, [R0]                                            \n\t"   \
                                                                        \
    /* The critical nesting depth is the first item on the stack. */    \
    /* Load it into the ulCriticalNesting variable. */                  \
    "LDR    R0, 2f /* =ulCriticalNesting */                     \n\t"   \
    "LDMFD  LR!, {R1}                                           \n\t"   \
    "STR    R1, [R0]                                            \n\t"   \
                                                                        \
    /* Get the SPSR from the stack. */                                  \
    "LDMFD  LR!, {R0}                                           \n\t"   \
    "MSR    SPSR, R0                                            \n\t"   \
                                                                        \
    /* Restore all system mode registers for the task. */               \
    "LDMFD  LR, {R0-R14}^                                       \n\t"   \
    "NOP                                                        \n\t"   \
                                                                        \
    /* Restore the return address. */                                   \
    "LDR    LR, [LR, #+60]                                      \n\t"   \
                                                                        \
    /* And return - correcting the offset in the LR to obtain the */    \
    /* correct address. */                                              \
    "SUBS   PC, LR, #4                                          \n\t"   \
    /* Place the pool behind the code. This is never reached. */        \
    "1:     .word pxCurrentTCB                                  \n\t"   \
    "2:     .word ulCriticalNesting                             \n\t"   \
    );                                                                  \
    ( void ) ulCriticalNesting;                                         \
    ( void ) pxCurrentTCB;                                              \
}
#else // CFG_GCC_LTO
#define portRESTORE_CONTEXT()                                           \
{                                                                       \
extern volatile void * volatile pxCurrentTCB;                           \
extern volatile unsigned portLONG ulCriticalNesting;                    \
                                                                        \
    /* Set the LR to the task stack. */                                 \
    asm volatile (                                                      \
    "LDR    R0, =pxCurrentTCB                                  \n\t"   \
    "LDR    R0, [R0]                                            \n\t"   \
    "LDR    LR, [R0]                                            \n\t"   \
                                                                        \
    /* The critical nesting depth is the first item on the stack. */    \
    /* Load it into the ulCriticalNesting variable. */                  \
    "LDR    R0, =ulCriticalNesting                              \n\t"   \
    "LDMFD  LR!, {R1}                                           \n\t"   \
    "STR    R1, [R0]                                            \n\t"   \
                                                                        \
    /* Get the SPSR from the stack. */                                  \
    "LDMFD  LR!, {R0}                                           \n\t"   \
    "MSR    SPSR, R0                                            \n\t"   \
                                                                        \
    /* Restore all system mode registers for the task. */               \
    "LDMFD  LR, {R0-R14}^                                       \n\t"   \
    "NOP                                                        \n\t"   \
                                                                        \
    /* Restore the return address. */                                   \
    "LDR    LR, [LR, #+60]                                      \n\t"   \
                                                                        \
    /* And return - correcting the offset in the LR to obtain the */    \
    /* correct address. */                                              \
    "SUBS   PC, LR, #4                                          \n\t"   \
    );                                                                  \
    ( void ) ulCriticalNesting;                                         \
    ( void ) pxCurrentTCB;                                              \
}
#endif // CFG_GCC_LTO
/*-----------------------------------------------------------*/

#ifdef CFG_GCC_LTO
#define portSAVE_CONTEXT()                                              \
{                                                                       \
extern volatile void * volatile pxCurrentTCB;                           \
extern volatile unsigned portLONG ulCriticalNesting;                    \
                                                                        \
    /* Push R0 as we are going to use the register. */                  \
    asm volatile (                                                      \
    "b      3f /* skip data */                                  \n\t"   \
    "1:     .word pxCurrentTCB                                  \n\t"   \
    "2:     .word ulCriticalNesting                             \n\t"   \
    "3:                                                         \n\t"   \
                                                                        \
    "STMDB  SP!, {R0}                                           \n\t"   \
                                                                        \
    /* Set R0 to point to the task stack pointer. */                    \
    "STMDB  SP,{SP}^                                            \n\t"   \
    "NOP                                                        \n\t"   \
    "SUB    SP, SP, #4                                          \n\t"   \
    "LDMIA  SP!,{R0}                                            \n\t"   \
                                                                        \
    /* Push the return address onto the stack. */                       \
    "STMDB  R0!, {LR}                                           \n\t"   \
                                                                        \
    /* Now we have saved LR we can use it instead of R0. */             \
    "MOV    LR, R0                                              \n\t"   \
                                                                        \
    /* Pop R0 so we can save it onto the system mode stack. */          \
    "LDMIA  SP!, {R0}                                           \n\t"   \
                                                                        \
    /* Push all the system mode registers onto the task stack. */       \
    "STMDB  LR,{R0-LR}^                                         \n\t"   \
    "NOP                                                        \n\t"   \
    "SUB    LR, LR, #60                                         \n\t"   \
                                                                        \
    /* Push the SPSR onto the task stack. */                            \
    "MRS    R0, SPSR                                            \n\t"   \
    "STMDB  LR!, {R0}                                           \n\t"   \
                                                                        \
    "LDR    R0, 2b /* =ulCriticalNesting */                     \n\t"   \
    "LDR    R0, [R0]                                            \n\t"   \
    "STMDB  LR!, {R0}                                           \n\t"   \
                                                                        \
    /* Store the new top of stack for the task. */                      \
    "LDR    R0, 1b /* =pxCurrentTCB */                          \n\t"   \
    "LDR    R0, [R0]                                            \n\t"   \
    "STR    LR, [R0]                                            \n\t"   \
    );                                                                  \
    ( void ) ulCriticalNesting;                                         \
    ( void ) pxCurrentTCB;                                              \
}
#else // CFG_GCC_LTO
#define portSAVE_CONTEXT()                                              \
{                                                                       \
extern volatile void * volatile pxCurrentTCB;                           \
extern volatile unsigned portLONG ulCriticalNesting;                    \
                                                                        \
    /* Push R0 as we are going to use the register. */                  \
    asm volatile (                                                      \
    "STMDB  SP!, {R0}                                           \n\t"   \
                                                                        \
    /* Set R0 to point to the task stack pointer. */                    \
    "STMDB  SP,{SP}^                                            \n\t"   \
    "NOP                                                        \n\t"   \
    "SUB    SP, SP, #4                                          \n\t"   \
    "LDMIA  SP!,{R0}                                            \n\t"   \
                                                                        \
    /* Push the return address onto the stack. */                       \
    "STMDB  R0!, {LR}                                           \n\t"   \
                                                                        \
    /* Now we have saved LR we can use it instead of R0. */             \
    "MOV    LR, R0                                              \n\t"   \
                                                                        \
    /* Pop R0 so we can save it onto the system mode stack. */          \
    "LDMIA  SP!, {R0}                                           \n\t"   \
                                                                        \
    /* Push all the system mode registers onto the task stack. */       \
    "STMDB  LR,{R0-LR}^                                         \n\t"   \
    "NOP                                                        \n\t"   \
    "SUB    LR, LR, #60                                         \n\t"   \
                                                                        \
    /* Push the SPSR onto the task stack. */                            \
    "MRS    R0, SPSR                                            \n\t"   \
    "STMDB  LR!, {R0}                                           \n\t"   \
                                                                        \
    "LDR    R0, =ulCriticalNesting                              \n\t"   \
    "LDR    R0, [R0]                                            \n\t"   \
    "STMDB  LR!, {R0}                                           \n\t"   \
                                                                        \
    /* Store the new top of stack for the task. */                      \
    "LDR    R0, =pxCurrentTCB                                   \n\t"   \
    "LDR    R0, [R0]                                            \n\t"   \
    "STR    LR, [R0]                                            \n\t"   \
    );                                                                  \
    ( void ) ulCriticalNesting;                                         \
    ( void ) pxCurrentTCB;                                              \
}
#endif // CFG_GCC_LTO

#define portYIELD_FROM_ISR( xHigherPriorityTaskWoken ) if( xHigherPriorityTaskWoken != pdFALSE ) vTaskSwitchContext()

/* Critical section handling. */

/*
 * The interrupt management utilities can only be called from ARM mode.  When
 * THUMB_INTERWORK is defined the utilities are defined as functions in
 * portISR.c to ensure a switch to ARM mode.  When THUMB_INTERWORK is not
 * defined then the utilities are defined as macros here - as per other ports.
 */

#ifdef THUMB_INTERWORK

    extern void vPortDisableInterruptsFromThumb( void ) __attribute__ ((naked));
    extern void vPortEnableInterruptsFromThumb( void ) __attribute__ ((naked));

    #define portDISABLE_INTERRUPTS()    vPortDisableInterruptsFromThumb()
    #define portENABLE_INTERRUPTS()     vPortEnableInterruptsFromThumb()

#else

    #define portDISABLE_INTERRUPTS()                                            \
        asm volatile (                                                          \
            "STMDB  SP!, {R0}       \n\t"   /* Push R0.                     */  \
            "MRS    R0, CPSR        \n\t"   /* Get CPSR.                    */  \
            "ORR    R0, R0, #0xC0   \n\t"   /* Disable IRQ, FIQ.            */  \
            "MSR    CPSR, R0        \n\t"   /* Write back modified value.   */  \
            "LDMIA  SP!, {R0}           " ) /* Pop R0.                      */

    #define portENABLE_INTERRUPTS()                                             \
        asm volatile (                                                          \
            "STMDB  SP!, {R0}       \n\t"   /* Push R0.                     */  \
            "MRS    R0, CPSR        \n\t"   /* Get CPSR.                    */  \
            "BIC    R0, R0, #0xC0   \n\t"   /* Enable IRQ, FIQ.             */  \
            "MSR    CPSR, R0        \n\t"   /* Write back modified value.   */  \
            "LDMIA  SP!, {R0}           " ) /* Pop R0.                      */

    #define portSAVEDISABLE_INTERRUPTS()                                        \
        asm volatile (                                                          \
            "SUB     SP, #4           \n\t"    /* SP -4                 */      \
            "STMDB   SP!, {R0}        \n\t"    /* Push R0.              */      \
            "MRS     R0, CPSR         \n\t"    /* Get CPSR.             */      \
            "STMIB   SP, {R0}         \n\t"    /* Push CPSR.            */      \
            "ORR     R0, R0, #0xC0    \n\t"    /* Disable IRQ, FIQ.     */      \
            "MSR     CPSR, R0         \n\t"    /* Write back modified value. */ \
            "LDMIA   SP!, {R0}        " )      /* Pop R0.               */

    #define portRESTORE_INTERRUPTS()                                            \
        asm volatile (                                                          \
            "STMDB   SP!, {R0}         \n\t"    /* Push R0.             */      \
            "LDMIB   SP, {R0}          \n\t"    /* Pop CPSR.            */      \
            "MSR     CPSR_c, R0        \n\t"    /* Write back modified value. */\
            "LDMIA   SP!, {R0}         \n\t"    /* Pop R0.              */      \
            "ADD     SP, #4           " )       /* SP +4                */

#endif /* THUMB_INTERWORK */

extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );

#define portENTER_CRITICAL()        vPortEnterCritical();
#define portEXIT_CRITICAL()         vPortExitCritical();

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not necessary for to use this port.  They are defined so the common demo files
(which build with all the ports) will build. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* Tickless idle/low power functionality. */
#ifndef portSUPPRESS_TICKS_AND_SLEEP
    extern void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );
    #define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime ) vPortSuppressTicksAndSleep( xExpectedIdleTime )
#endif

/*-----------------------------------------------------------*/

/* Architecture specific optimisations. */
#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

    /* Generic helper function. */
    __attribute__( ( always_inline ) ) static inline uint8_t ucPortCountLeadingZeros( uint32_t ulBitmap )
    {
    uint8_t ucReturn;

        __asm volatile ( "clz %0, %1" : "=r" ( ucReturn ) : "r" ( ulBitmap ) );
        return ucReturn;
    }

    /* Check the configuration. */
    #if( configMAX_PRIORITIES > 32 )
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
    #endif

    /* Store/clear the ready priorities in a bit map. */
    #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
    #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

    /*-----------------------------------------------------------*/

    #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31 - ucPortCountLeadingZeros( ( uxReadyPriorities ) ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/*-----------------------------------------------------------*/

#ifdef configASSERT
    void vPortValidateInterruptPriority( void );
    #define portASSERT_IF_INTERRUPT_PRIORITY_INVALID()  vPortValidateInterruptPriority()
#endif

#ifndef portFORCE_INLINE
    #define portFORCE_INLINE inline __attribute__(( always_inline))
#endif
/*-----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

