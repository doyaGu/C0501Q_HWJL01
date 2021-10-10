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

#if __riscv_xlen == 64
# define STORE    sd
# define LOAD     ld
# define REGBYTES 8
#else
# define STORE    sw
# define LOAD     lw
# define REGBYTES 4
#endif

#define MSTATUS_PRV1 0x1800

.global portSAVE_CONTEXT
.global portRESTORE_CONTEXT
.global TIMER_CMP_INT
.global xPortStartScheduler
.global vPortYield
.global vTaskIncrementTick
.global vPortEndScheduler
.global xExitStack

/* Macro for saving task context */
.macro portSAVE_CONTEXT
    .global pxCurrentTCB
    /* make room in stack */
    addi    sp, sp, -REGBYTES * 32

    /* Save Context */
    STORE   x1, 0x0(sp)
    STORE   x2, 1 * REGBYTES(sp)
    STORE   x3, 2 * REGBYTES(sp)
    STORE   x4, 3 * REGBYTES(sp)
    STORE   x5, 4 * REGBYTES(sp)
    STORE   x6, 5 * REGBYTES(sp)
    STORE   x7, 6 * REGBYTES(sp)
    STORE   x8, 7 * REGBYTES(sp)
    STORE   x9, 8 * REGBYTES(sp)
    STORE   x10, 9 * REGBYTES(sp)
    STORE   x11, 10 * REGBYTES(sp)
    STORE   x12, 11 * REGBYTES(sp)
    STORE   x13, 12 * REGBYTES(sp)
    STORE   x14, 13 * REGBYTES(sp)
    STORE   x15, 14 * REGBYTES(sp)
    STORE   x16, 15 * REGBYTES(sp)
    STORE   x17, 16 * REGBYTES(sp)
    STORE   x18, 17 * REGBYTES(sp)
    STORE   x19, 18 * REGBYTES(sp)
    STORE   x20, 19 * REGBYTES(sp)
    STORE   x21, 20 * REGBYTES(sp)
    STORE   x22, 21 * REGBYTES(sp)
    STORE   x23, 22 * REGBYTES(sp)
    STORE   x24, 23 * REGBYTES(sp)
    STORE   x25, 24 * REGBYTES(sp)
    STORE   x26, 25 * REGBYTES(sp)
    STORE   x27, 26 * REGBYTES(sp)
    STORE   x28, 27 * REGBYTES(sp)
    STORE   x29, 28 * REGBYTES(sp)
    STORE   x30, 29 * REGBYTES(sp)
    STORE   x31, 30 * REGBYTES(sp)

    /* Store current stackpointer in task control block (TCB) */
    LOAD    t0, pxCurrentTCB    //pointer
    STORE   sp, 0x0(t0)
    .endm

/* Saves current error program counter (EPC) as task program counter */
.macro portSAVE_EPC
    csrr    t0, mepc
    STORE   t0, 31 * REGBYTES(sp)
    .endm

/* Saves current return adress (RA) as task program counter */
.macro portSAVE_RA
    STORE   ra, 31 * REGBYTES(sp)
    .endm

/* Macro for restoring task context */
.macro portRESTORE_CONTEXT

    .global pxCurrentTCB
    /* Load stack pointer from the current TCB */
    LOAD    sp, pxCurrentTCB
    LOAD    sp, 0x0(sp)

    /* Load task program counter */
    LOAD    t0, 31 * REGBYTES(sp)
    csrw    mepc, t0

    /* Run in machine mode */
    li      t0, MSTATUS_PRV1
    csrs    mstatus, t0

    /* Restore registers,
       Skip global pointer because that does not change */
    LOAD    x1, 0x0(sp)
    LOAD    x4, 3 * REGBYTES(sp)
    LOAD    x5, 4 * REGBYTES(sp)
    LOAD    x6, 5 * REGBYTES(sp)
    LOAD    x7, 6 * REGBYTES(sp)
    LOAD    x8, 7 * REGBYTES(sp)
    LOAD    x9, 8 * REGBYTES(sp)
    LOAD    x10, 9 * REGBYTES(sp)
    LOAD    x11, 10 * REGBYTES(sp)
    LOAD    x12, 11 * REGBYTES(sp)
    LOAD    x13, 12 * REGBYTES(sp)
    LOAD    x14, 13 * REGBYTES(sp)
    LOAD    x15, 14 * REGBYTES(sp)
    LOAD    x16, 15 * REGBYTES(sp)
    LOAD    x17, 16 * REGBYTES(sp)
    LOAD    x18, 17 * REGBYTES(sp)
    LOAD    x19, 18 * REGBYTES(sp)
    LOAD    x20, 19 * REGBYTES(sp)
    LOAD    x21, 20 * REGBYTES(sp)
    LOAD    x22, 21 * REGBYTES(sp)
    LOAD    x23, 22 * REGBYTES(sp)
    LOAD    x24, 23 * REGBYTES(sp)
    LOAD    x25, 24 * REGBYTES(sp)
    LOAD    x26, 25 * REGBYTES(sp)
    LOAD    x27, 26 * REGBYTES(sp)
    LOAD    x28, 27 * REGBYTES(sp)
    LOAD    x29, 28 * REGBYTES(sp)
    LOAD    x30, 29 * REGBYTES(sp)
    LOAD    x31, 30 * REGBYTES(sp)

    addi    sp, sp, REGBYTES * 32
    mret
    .endm

/* Macro for restoring task context */
TIMER_CMP_INT:
    portSAVE_CONTEXT
    portSAVE_EPC
    jal vPortSysTickHandler
    portRESTORE_CONTEXT

xPortStartScheduler:
#ifdef __gracefulExit
    /* Stores context when starting the scheduler in xStartContext.
       This is used for when you want to gracefully exit the scheduler.
       For example if you want to test multiple instances after each other in one test suite.
     */
    la      t0, xStartContext
    STORE   x1, 0x0(t0)
    STORE   x2, 1 * REGBYTES(t0)
    STORE   x3, 2 * REGBYTES(t0)
    STORE   x4, 3 * REGBYTES(t0)
    STORE   x5, 4 * REGBYTES(t0)
    STORE   x6, 5 * REGBYTES(t0)
    STORE   x7, 6 * REGBYTES(t0)
    STORE   x8, 7 * REGBYTES(t0)
    STORE   x9, 8 * REGBYTES(t0)
    STORE   x10, 9 * REGBYTES(t0)
    STORE   x11, 10 * REGBYTES(t0)
    STORE   x12, 11 * REGBYTES(t0)
    STORE   x13, 12 * REGBYTES(t0)
    STORE   x14, 13 * REGBYTES(t0)
    STORE   x15, 14 * REGBYTES(t0)
    STORE   x16, 15 * REGBYTES(t0)
    STORE   x17, 16 * REGBYTES(t0)
    STORE   x18, 17 * REGBYTES(t0)
    STORE   x19, 18 * REGBYTES(t0)
    STORE   x20, 19 * REGBYTES(t0)
    STORE   x21, 20 * REGBYTES(t0)
    STORE   x22, 21 * REGBYTES(t0)
    STORE   x23, 22 * REGBYTES(t0)
    STORE   x24, 23 * REGBYTES(t0)
    STORE   x25, 24 * REGBYTES(t0)
    STORE   x26, 25 * REGBYTES(t0)
    STORE   x27, 26 * REGBYTES(t0)
    STORE   x28, 27 * REGBYTES(t0)
    STORE   x29, 28 * REGBYTES(t0)
    STORE   x30, 29 * REGBYTES(t0)
    STORE   x31, 30 * REGBYTES(t0)
#endif
    jal     vPortSetupTimer
    portRESTORE_CONTEXT

vPortEndScheduler:
#ifdef __gracefulExit
    /* Load current context from xStartContext */
    la      t0, xStartContext
    LOAD    x1, 0x0(t0)
    LOAD    x2, 1 * REGBYTES(t0)
    LOAD    x3, 2 * REGBYTES(t0)
    LOAD    x4, 3 * REGBYTES(t0)
    LOAD    x5, 4 * REGBYTES(t0)
    LOAD    x6, 5 * REGBYTES(t0)
    LOAD    x7, 6 * REGBYTES(t0)
    LOAD    x8, 7 * REGBYTES(t0)
    LOAD    x9, 8 * REGBYTES(t0)
    LOAD    x10, 9 * REGBYTES(t0)
    LOAD    x11, 10 * REGBYTES(t0)
    LOAD    x12, 11 * REGBYTES(t0)
    LOAD    x13, 12 * REGBYTES(t0)
    LOAD    x14, 13 * REGBYTES(t0)
    LOAD    x15, 14 * REGBYTES(t0)
    LOAD    x16, 15 * REGBYTES(t0)
    LOAD    x17, 16 * REGBYTES(t0)
    LOAD    x18, 17 * REGBYTES(t0)
    LOAD    x19, 18 * REGBYTES(t0)
    LOAD    x20, 19 * REGBYTES(t0)
    LOAD    x21, 20 * REGBYTES(t0)
    LOAD    x22, 21 * REGBYTES(t0)
    LOAD    x23, 22 * REGBYTES(t0)
    LOAD    x24, 23 * REGBYTES(t0)
    LOAD    x25, 24 * REGBYTES(t0)
    LOAD    x26, 25 * REGBYTES(t0)
    LOAD    x27, 26 * REGBYTES(t0)
    LOAD    x28, 27 * REGBYTES(t0)
    LOAD    x29, 28 * REGBYTES(t0)
    LOAD    x30, 39 * REGBYTES(t0)
    LOAD    x31, 30 * REGBYTES(t0)
#endif
    ret

vPortYield:
    portSAVE_CONTEXT
    portSAVE_RA
    jal vTaskSwitchContext
    portRESTORE_CONTEXT
