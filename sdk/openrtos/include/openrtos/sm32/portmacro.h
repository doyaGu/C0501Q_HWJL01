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
    #effor "configUSE_16_BIT_TICKS must be 0"
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL

    /* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
    not need to be guarded with a critical section. */
    #define portTICK_TYPE_IS_ATOMIC 1
#endif
#endif // __ASSEMBLER__

/*-----------------------------------------------------------*/
#define portSTACK_GROWTH                -1
#define portTICK_PERIOD_MS              ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT              4
#define portCRITICAL_NESTING_IN_TCB     1
#define portINSTRUCTION_SIZE            ( ( StackType_t ) 4 )
#define portNO_CRITICAL_SECTION_NESTING ( ( StackType_t ) 0 )

#define portYIELD_FROM_ISR( xHigherPriorityTaskWoken ) if( xHigherPriorityTaskWoken != pdFALSE ) vTaskSwitchContext()
#define portYIELD()     \
    __asm__ __volatile__ (  "l.nop       \n\t"  \
                            "l.sys 0x0FCC\n\t"  \
                            "l.nop       \n\t"  \
    );
#define portNOP()       __asm__ __volatile__ ( "l.nop" )

/*-----------------------------------------------------------*/
#define portDISABLE_INTERRUPTS()    { extern inline void vPortDisableInterrupts( void ); vPortDisableInterrupts(); }
#define portENABLE_INTERRUPTS()     { extern inline void vPortEnableInterrupts( void );  vPortEnableInterrupts();  }

#define portSAVEDISABLE_INTERRUPTS()                                        \
    asm volatile (                                                          \
        "l.addi   r1, r1, 0xfffffffc   \n\t"    /* SP -4.                */ \
        "l.addi   r21, r0, 0x11        \n\t"    /* Get SPR_SR address.   */ \
        "l.mfspr  r23, r21, 0x0        \n\t"    /* Get SPR_SR.           */ \
        "l.sw     0x0(r1), r23         \n\t"    /* Push SPR_SR.          */ \
        "l.addi   r25, r0, 0xfffffffb  \n\t"    /* Disable IEE.          */ \
        "l.and    r27, r23, r25        \n\t"    /* Disable IEE.          */ \
        "l.mtspr  r21, r27, 0x0        \n\t"    /* Write Back SPR_SR.    */ \
        : : :"r1", "r21", "r23", "r25", "r27", "memory" )

#define portRESTORE_INTERRUPTS()                                            \
    asm volatile (                                                          \
        "l.lwz   r23, 0x0(r1)         \n\t"    /* Pop SPR_SR.           */  \
        "l.addi  r21, r0, 0x11        \n\t"    /* Get SPR_SR address.   */  \
        "l.mtspr r21, r23, 0x0        \n\t"    /* Write back SPR_SR.    */  \
        "l.addi  r1, r1, 0x4          \n\t"    /* SP +4.                */  \
        : : :"r1", "r21", "r23" )

#define portENTER_CRITICAL()        { extern void vTaskEnterCritical( void ); vTaskEnterCritical();  }
#define portEXIT_CRITICAL()         { extern void vTaskExitCritical( void );  vTaskExitCritical();   }
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

/*
    Context layout
    0x00    r9
    0x04    r2
    0x08    r3
    0x0C    r4
    0x10    r5
    0x14    r6
    0x18    r7
    0x1C    r8
    0x20    r10
    0x24    r11
    0x28    r12
    0x2C    r13
    0x30    r14
    0x34    r15
    0x38    r16
    0x3C    r17
    0x40    r18
    0x44    r19
    0x48    r20
    0x4C    r21
    0x50    r22
    0x54    r23
    0x58    r24
    0x5C    r25
    0x60    r26
    0x64    r27
    0x68    r28
    0x6C    r29
    0x70    r30
    0x74    r31
    0x78    ESR
    0x7C    EPCR
*/

#define REDZONE_SIZE        (128)
#define CONTEXT_SIZE        (128)
#define STACKFRAME_SIZE     (CONTEXT_SIZE + REDZONE_SIZE)

// Macro to save all registers, stack pointer into the TCB.
#define portSAVE_CONTEXT()                      \
    asm volatile (                              \
    "   .global _pxCurrentTCB           \n\t"   \
    "   # make rooms in stack           \n\t"   \
    "   l.addi  r1, r1, -256            \n\t"   \
    "   # early save r3-r5, these are clobber register\n\t" \
    "   l.sw    0x08(r1), r3            \n\t"   \
    "   l.sw    0x0C(r1), r4            \n\t"   \
    "   l.sw    0x10(r1), r5            \n\t"   \
    "   # save SPR_ESR_BASE(0), SPR_EPCR_BASE(0)\n\t"   \
    "   l.mfspr r3, r0, (0<<11) + 64    \n\t"   \
    "   l.mfspr r4, r0, (0<<11) + 32    \n\t"   \
    "   l.sw    0x78(r1), r3            \n\t"   \
    "   l.sw    0x7C(r1), r4            \n\t"   \
    "   # Save Context                  \n\t"   \
    "   l.sw    0x00(r1), r9            \n\t"   \
    "   l.sw    0x04(r1), r2            \n\t"   \
    "   l.sw    0x14(r1), r6            \n\t"   \
    "   l.sw    0x18(r1), r7            \n\t"   \
    "   l.sw    0x1C(r1), r8            \n\t"   \
    "   l.sw    0x20(r1), r10           \n\t"   \
    "   l.sw    0x24(r1), r11           \n\t"   \
    "   l.sw    0x28(r1), r12           \n\t"   \
    "   l.sw    0x2C(r1), r13           \n\t"   \
    "   l.sw    0x30(r1), r14           \n\t"   \
    "   l.sw    0x34(r1), r15           \n\t"   \
    "   l.sw    0x38(r1), r16           \n\t"   \
    "   l.sw    0x3C(r1), r17           \n\t"   \
    "   l.sw    0x40(r1), r18           \n\t"   \
    "   l.sw    0x44(r1), r19           \n\t"   \
    "   l.sw    0x48(r1), r20           \n\t"   \
    "   l.sw    0x4C(r1), r21           \n\t"   \
    "   l.sw    0x50(r1), r22           \n\t"   \
    "   l.sw    0x54(r1), r23           \n\t"   \
    "   l.sw    0x58(r1), r24           \n\t"   \
    "   l.sw    0x5C(r1), r25           \n\t"   \
    "   l.sw    0x60(r1), r26           \n\t"   \
    "   l.sw    0x64(r1), r27           \n\t"   \
    "   l.sw    0x68(r1), r28           \n\t"   \
    "   l.sw    0x6C(r1), r29           \n\t"   \
    "   l.sw    0x70(r1), r30           \n\t"   \
    "   l.sw    0x74(r1), r31           \n\t"   \
    "   # Save the top of stack in TCB  \n\t"   \
    "   l.movhi r3, hi(pxCurrentTCB)    \n\t"   \
    "   l.ori   r3, r3, lo(pxCurrentTCB)\n\t"   \
    "   l.lwz   r3, 0x0(r3)             \n\t"   \
    "   l.sw    0x0(r3), r1             \n\t"   \
    "   # restore clobber register      \n\t"   \
    "   l.lwz   r3, 0x08(r1)            \n\t"   \
    "   l.lwz   r4, 0x0C(r1)            \n\t"   \
    "   l.lwz   r5, 0x10(r1)            \n\t"   \
    );

#define portRESTORE_CONTEXT()                   \
    asm volatile (                              \
    "   .global pxCurrentTCB            \n\t"   \
    "   # restore stack pointer         \n\t"   \
    "   l.movhi r3, hi(pxCurrentTCB)    \n\t"   \
    "   l.ori   r3, r3, lo(pxCurrentTCB)\n\t"   \
    "   l.lwz   r3, 0x0(r3)             \n\t"   \
    "   l.lwz   r1, 0x0(r3)             \n\t"   \
    "   # restore context               \n\t"   \
    "   l.lwz   r9, 0x00(r1)            \n\t"   \
    "   l.lwz   r2, 0x04(r1)            \n\t"   \
    "   l.lwz   r6, 0x14(r1)            \n\t"   \
    "   l.lwz   r7, 0x18(r1)            \n\t"   \
    "   l.lwz   r8, 0x1C(r1)            \n\t"   \
    "   l.lwz   r10, 0x20(r1)           \n\t"   \
    "   l.lwz   r11, 0x24(r1)           \n\t"   \
    "   l.lwz   r12, 0x28(r1)           \n\t"   \
    "   l.lwz   r13, 0x2C(r1)           \n\t"   \
    "   l.lwz   r14, 0x30(r1)           \n\t"   \
    "   l.lwz   r15, 0x34(r1)           \n\t"   \
    "   l.lwz   r16, 0x38(r1)           \n\t"   \
    "   l.lwz   r17, 0x3C(r1)           \n\t"   \
    "   l.lwz   r18, 0x40(r1)           \n\t"   \
    "   l.lwz   r19, 0x44(r1)           \n\t"   \
    "   l.lwz   r20, 0x48(r1)           \n\t"   \
    "   l.lwz   r21, 0x4C(r1)           \n\t"   \
    "   l.lwz   r22, 0x50(r1)           \n\t"   \
    "   l.lwz   r23, 0x54(r1)           \n\t"   \
    "   l.lwz   r24, 0x58(r1)           \n\t"   \
    "   l.lwz   r25, 0x5C(r1)           \n\t"   \
    "   l.lwz   r26, 0x60(r1)           \n\t"   \
    "   l.lwz   r27, 0x64(r1)           \n\t"   \
    "   l.lwz   r28, 0x68(r1)           \n\t"   \
    "   l.lwz   r29, 0x6C(r1)           \n\t"   \
    "   l.lwz   r30, 0x70(r1)           \n\t"   \
    "   l.lwz   r31, 0x74(r1)           \n\t"   \
    "   # restore SPR_ESR_BASE(0), SPR_EPCR_BASE(0)\n\t"    \
    "   l.lwz   r3, 0x78(r1)            \n\t"   \
    "   l.lwz   r4, 0x7C(r1)            \n\t"   \
    "   l.mtspr r0, r3, (0<<11) + 64    \n\t"   \
    "   l.mtspr r0, r4, (0<<11) + 32    \n\t"   \
    "   # restore clobber register      \n\t"   \
    "   l.lwz   r3, 0x08(r1)            \n\t"   \
    "   l.lwz   r4, 0x0C(r1)            \n\t"   \
    "   l.lwz   r5, 0x10(r1)            \n\t"   \
    "   l.addi  r1, r1, 256             \n\t"   \
    "   l.rfe                           \n\t"   \
    "   l.nop                           \n\t"   \
    );

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not necessary for to use this port.  They are defined so the common demo files
(which build with all the ports) will build. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

#ifndef __ASSEMBLER__

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

        __asm volatile ( "l.fl1 %0, %1" : "=r" ( ucReturn ) : "r" ( ulBitmap ) );
        return 32 - ucReturn;
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

/* For writing into SPR. */
static inline void mtspr(unsigned long spr, unsigned long value)
{
    asm("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));
}

/* For reading SPR. */
static inline unsigned long mfspr(unsigned long spr)
{
    unsigned long value;
    asm("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));
    return value;
}

#endif // __ASSEMBLER__

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

