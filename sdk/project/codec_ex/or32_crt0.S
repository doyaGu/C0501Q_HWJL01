
#define __CRT0__

//#include "or32.h"
#include "spr_defs.h"
//#include "version.h"

        .extern main
        .extern _stack
        .extern risc1_start_addr
        .extern risc2_start_addr
        .global __exit_halt
        .global __bus_error_halt
        .global __data_page_fault_halt
        .global __inst_page_fault_halt
        .global __tick_halt
        .global __alignment_halt
        .global __illegal_inst_halt
        .global __int_halt
        .global __dtlb_miss_halt
        .global __itlb_miss_halt
        .global __range_halt
        .global __sys_call_halt
        .global __trap_halt

###########################################################################
## Macro
###########################################################################

#define ENTRY(symbol) \
        .global symbol ; \
symbol:

#define FUNC_BEGIN(symbol) \
        .global symbol; \
        .proc symbol; \
        .type symbol, @function ; \
symbol:

#define FUNC_END(symbol) \
        .size   symbol, .-symbol

###########################################################################
## Begin of RAM
###########################################################################
        .section .reset, "ax"
        .org    0x000

1:      l.j     1b      # forever loop
        l.nop   0x02

########################################
# Pointer of Print Buffer
########################################
        .org    0xcc
        .global __dbgmsg_buf_ptr
__dbgmsg_buf_ptr:
        .long   0
        .global __dbgmsg_buf_len
__dbgmsg_buf_len:
        .long   0
        .global __dbgmsg_idx_ptr
__dbgmsg_idx_ptr:
        .long   0

###########################################################################
## Reset section
###########################################################################
        .org    0x100
ENTRY(_start)
ENTRY(_reset)
        l.movhi r0, 0x0
        l.ori   r0,r0,0x0
        l.addi  r1,r0,0x0
        l.addi  r2,r0,0x0
        l.addi  r3,r0,0x0
        l.addi  r4,r0,0x0
        l.addi  r5,r0,0x0
        l.addi  r6,r0,0x0
        l.addi  r7,r0,0x0
        l.addi  r8,r0,0x0
        l.addi  r9,r0,0x0
        l.addi  r10,r0,0x0
        l.addi  r11,r0,0x0
        l.addi  r12,r0,0x0
        l.addi  r13,r0,0x0
        l.addi  r14,r0,0x0
        l.addi  r15,r0,0x0
        l.addi  r16,r0,0x0
        l.addi  r17,r0,0x0
        l.addi  r18,r0,0x0
        l.addi  r19,r0,0x0
        l.addi  r20,r0,0x0
        l.addi  r21,r0,0x0
        l.addi  r22,r0,0x0
        l.addi  r23,r0,0x0
        l.addi  r24,r0,0x0
        l.addi  r25,r0,0x0
        l.addi  r26,r0,0x0
        l.addi  r27,r0,0x0
        l.addi  r28,r0,0x0
        l.addi  r29,r0,0x0
        l.addi  r30,r0,0x0
        l.addi  r31,r0,0x0

        # Normal Function
        l.movhi r3,hi(_start_proc)
        l.ori   r3,r3,lo(_start_proc)
        l.jr    r3
        l.addi  r3, r0, 0x0

###########################################################################
## Exit Procedure
###########################################################################
.global _exit
FUNC_BEGIN(_exit)
        l.trap  0xf     # Software break point

        # SHOULD NOT REACH HERE
__exit_halt:
        l.j     __exit_halt
        l.nop   0x01
FUNC_END(_exit)

###########################################################################
## Bus Error
###########################################################################
        .org    0x200
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_bus_error)
__bus_error_halt:
1:      l.j     1b
        l.nop   0x02
FUNC_END(_bus_error)

###########################################################################
## Data Page Fault
###########################################################################
        .org    0x300
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_data_page_fault)
__data_page_fault_halt:
        l.j     __data_page_fault_halt
        l.nop   0x03
FUNC_END(_data_page_fault)

###########################################################################
## Instructions Page Fault
###########################################################################
        .org    0x400
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_inst_page_fault)
__inst_page_fault_halt:
        l.j     __inst_page_fault_halt
        l.nop   0x04
FUNC_END(_inst_page_fault)

###########################################################################
## Tick Timer Interrupt
###########################################################################
        .org    0x500
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_tick_timer)
__tick_halt:
        l.j     __tick_halt
        l.nop   0x05
FUNC_END(_tick_timer)

###########################################################################
## Alignment
###########################################################################
        .org    0x600
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_alignment)
__alignment_halt:
1:      l.j     1b
        l.nop   0x02
FUNC_END(_alignment)

###########################################################################
## Illegal instructions
###########################################################################
        .org    0x700
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_illegal_inst)
__illegal_inst_halt:
1:      l.j     1b
        l.nop   0x02
FUNC_END(_illegal_inst)

###########################################################################
## External Interrupt handler
###########################################################################
        .org    0x800
        l.nop   0x0         # Two of NOPs is required, it can not be removed.
        l.nop   0x0
FUNC_BEGIN(_int)
__int_halt:
        l.j     __int_halt
        l.nop   0x08
FUNC_END(_int)

###########################################################################
## D-TLB Miss
###########################################################################
        .org    0x900
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_dtlb_miss)
__dtlb_miss_halt:
        l.j     __dtlb_miss_halt
        l.nop   0x09
FUNC_END(_dtlb_miss)

###########################################################################
## I-TLB Miss
###########################################################################
        .org    0xa00
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_itlb_miss)
__itlb_miss_halt:
        l.j     __itlb_miss_halt
        l.nop   0x0a
FUNC_END(_itlb_miss)

###########################################################################
## Arithmetic overflow
###########################################################################
        .org    0xb00
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_range)
__range_halt:
        l.j     __range_halt
        l.nop   0x0b
FUNC_END(_range)

###########################################################################
## System call
###########################################################################
        .org    0xc00
        # HACK: These two nop's are required. So far nobody knows why,
        # keep them here!
        l.nop
        l.nop
FUNC_BEGIN(_sys_call)
__sys_call_halt:
        l.j     __sys_call_halt
        l.nop   0x0c
FUNC_END(_sys_call)

###########################################################################
## Instruction l.trap or debug unit exeception
###########################################################################
        .org    0xe00
        l.nop   0x0
        l.nop   0x0
FUNC_BEGIN(_trap)
__trap_halt:
1:      l.j     1b
        l.nop   0x02
FUNC_END(_trap)

        #.section .text
###########################################################################
## Start Procedure
###########################################################################
ENTRY(_start_proc)
        # Check CPU ID
        l.addi  r5,r0,SPR_VR
        l.mfspr r4,r5,0
        l.andi  r3,r4,SPR_VER_N
        l.sfeqi r3,0
        l.bf    .proc0
        l.nop

        # Set the return address to _exit
        l.movhi r9,hi(_exit)
        l.ori   r9,r9,lo(_exit)

        # Jump to _main procedure
        l.movhi r4,hi(risc2_start_addr)
        l.ori   r4,r4,lo(risc2_start_addr)
        l.lwz   r5,0x10(r4) 
        l.jr    r5
        l.nop

.proc0:
        # Set the return address to _exit
        l.movhi r9,hi(_exit)
        l.ori   r9,r9,lo(_exit)

        # Jump to _main procedure
        l.movhi r4,hi(risc1_start_addr)
        l.ori   r4,r4,lo(risc1_start_addr)
        l.lwz   r5,0x10(r4) 
        l.jr    r5
        l.nop

###########################################################################
## Enable I-Cache, and invalidate all of I-Cache entry.
##
## Code generate by GCC 3.4.4. with -O3 -fomit-frame-pointer options.
##
## void or32_ic_invalidate(void) {
##     int ncs, bs;
##     int cache_size, cache_line_size;
##     int i = 0;
##
##     // Disable IC
##     mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_ICE);
##
##     // Number of cache sets
##     ncs = (mfspr(SPR_ICCFGR) >> 3) & 0xf;
##
##     // Number of cache block size
##     bs  = ((mfspr(SPR_ICCFGR) >> 7) & 0x1) + 4;
##
##     // Calc Cache size
##     cache_line_size = 1 << bs;
##     cache_size      = 1 << (ncs+bs);
##
##     // Flush IC
##     do {
##         mtspr(SPR_ICBIR, i);
##         i += cache_line_size;
##     } while (i < cache_size);
##
##     // Enable IC
##     mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_ICE);
##     asm volatile("l.nop 0x0\nl.nop 0x0\nl.nop 0x0\nl.nop 0x0\n");
## }
###########################################################################
.section .text
.global ic_invalidate
.global ic_enable
.align  0x100
FUNC_BEGIN(ic_invalidate)
FUNC_BEGIN(ic_enable)
        l.addi  r3,r0,0

        /* Disable IC */
        l.addi  r21,r0,SPR_SR
        l.mfspr r25,r21,0
        l.addi  r27,r0,~SPR_SR_ICE
        l.and   r23,r25,r27
        l.mtspr r21,r23,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        /* Calc Cache size */
        l.addi  r15,r0,SPR_ICCFGR
        l.mfspr r19,r15,0
        l.srli  r17,r19,3
        l.andi  r5,r17,0xf
        l.mfspr r6,r15,0
        l.srli  r13,r6,7
        l.addi  r8,r0,1
        l.and   r11,r13,r8
        l.addi  r6,r0,SPR_ICBIR
        l.addi  r4,r11,4
        l.add   r7,r5,r4
        l.sll   r5,r8,r4
        l.sll   r4,r8,r7
        l.j     1f
        l.nop

        /* For the NAND booting, the NAND controller prefetch 64 bytes as buffering. */
        /* So it would be 64 bytes boundary on the loop to prevent multi-access the  */
        /* NAND data if it cross 64 bytes boundary                                   */
        .align  0x40

        /* Flush IC */
1:
        l.mtspr r6,r3,0
        l.add   r3,r3,r5
        l.sfges r3,r4
        l.bnf   1b

        /* Enable IC */
        l.addi  r29,r0,SPR_SR
        l.mfspr r31,r29,0
        l.ori   r3,r31,SPR_SR_ICE
        l.mtspr r29,r3,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(ic_enable)

###########################################################################
## Disable I Cache
###########################################################################
.global ic_disable
FUNC_BEGIN(ic_disable)
        /* Disable IC */
        l.mfspr r4,r0,SPR_SR
        l.addi  r3,r0,-1
        l.xori  r3,r3,SPR_SR_ICE
        l.and   r3,r4,r3
        l.mtspr r0,r3,SPR_SR
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(ic_disable)

###########################################################################
## Enable D-Cache, and invalidate all of D-Cache entry.
##
## Code generate by GCC 3.4.4. with -O3 -fomit-frame-pointer options.
##
## void or32_dc_invalidate(void) {
##     int ncs, bs;
##     int cache_size, cache_line_size;
##     int i = 0;
##
##     // Disable DC
##     mtspr(SPR_SR, mfspr(SPR_SR) & ~SPR_SR_DCE);
##
##     // Number of cache sets
##     ncs = ((mfspr(SPR_DCCFGR) >> 3) & 0xf);
##
##     // Number of cache block size
##     bs  = ((mfspr(SPR_DCCFGR) >> 7) & 0x1) + 4;
##
##     // Calc Cache size
##     cache_line_size = 1 << bs;
##     cache_size      = 1 << (ncs+bs);
##
##     // Flush DC
##     do {
##         mtspr(SPR_DCBIR, i);
##         i += cache_line_size;
##     } while(i < cache_size);
##
##     // Enable DC
##     mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_DCE);
##     asm volatile("l.nop 0x0\nl.nop 0x0\nl.nop 0x0\nl.nop 0x0\n");
## }
###########################################################################
.global dc_invalidate
.global dc_enable
FUNC_BEGIN(dc_invalidate)
FUNC_BEGIN(dc_enable)
        l.addi  r3,r0,0

        /* Disable DC */
        l.addi  r21,r0,SPR_SR
        l.mfspr r25,r21,0
        l.addi  r27,r0,~SPR_SR_DCE
        l.and   r23,r25,r27
        l.mtspr r21,r23,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        /* Calc Cache size */
        l.addi  r15,r0,SPR_DCCFGR
        l.mfspr r19,r15,0
        l.srli  r17,r19,3
        l.andi  r5,r17,0xf
        l.mfspr r6,r15,0
        l.srli  r13,r6,7
        l.addi  r8,r0,1
        l.and   r11,r13,r8
        l.addi  r6,r0,SPR_DCBIR
        l.addi  r4,r11,4
        l.add   r7,r5,r4
        l.sll   r5,r8,r4
        l.sll   r4,r8,r7

        /* Flush IC */
1:
        l.mtspr r6,r3,0
        l.add   r3,r3,r5
        l.sfges r3,r4
        l.bnf   1b

        /* Enable DC */
        l.addi  r29,r0,SPR_SR
        l.mfspr r31,r29,0
        l.ori   r3,r31,SPR_SR_DCE
        l.mtspr r29,r3,0
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(dc_invalidate)
FUNC_END(dc_enable)

###########################################################################
## Disable D Cache
###########################################################################
.global dc_disable
FUNC_BEGIN(dc_disable)
        /* Disable DC */
        l.mfspr r4,r0,SPR_SR
        l.addi  r3,r0,-1
        l.xori  r3,r3,SPR_SR_DCE
        l.and   r3,r4,r3
        l.mtspr r0,r3,SPR_SR
        l.nop   # insert sets of l.nop after changing the cache state
        l.nop
        l.nop
        l.nop

        l.jr    r9
        l.nop
FUNC_END(dc_disable)
