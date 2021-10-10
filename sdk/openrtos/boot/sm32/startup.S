/* Support file for c based tests */
#include "spr_defs.h"

//#define IC_ENABLE       1
//#define IC_SIZE         (8192-16)
//#define DC_ENABLE       1
//#define DC_SIZE         (8192-16)
#define CODEC_ENTRY_OFFSET  0x10

    .extern __printbuf_base
    .weak   codec_api
    .weak   codec_start_addr

    .global _stack_top

    .section .reset, "ax"

    .align 4             /* Align to the word boundary */

/*****************************************************************************
* Version Information
*/
//<<<<< DON'T MOVED!!!
    .org    0x8
    /* Disable IC */
    l.addi  r1,r0,SPR_SR
    l.mfspr r3,r1,0
    l.addi  r4,r0,~SPR_SR_ICE
    l.and   r2,r3,r4
    l.mtspr r1,r2,0

    #Remap Memory
    l.movhi r1,hi(0xd0000000) # AHB base
    l.ori   r1,r1,lo(0x0088)  # AHB remap
    l.addi  r2,r0,0x1
    l.sw    0x0(r1),r2
    l.nop   0x168           # 3 delay slots to remap memory.
    l.nop   0x168           # Fetch+decode+execute pipeline statge.
    l.nop   0x168
    l.nop   0x168           # one more delay slot for safty.

    l.j     _reset_vector
    l.nop   0x0
//>>>>> DON'T MOVED!!!

    .org    0x54
    .global __version_major
__version_major:
    .word   CFG_VERSION_MAJOR
    .global __version_minor
__version_minor:
    .word   CFG_VERSION_MINOR
    .global __version_patch
__version_patch:
    .word   CFG_VERSION_PATCH
    .global __version_custom
__version_custom:
    .word   CFG_VERSION_CUSTOM
    .global __version_tweak
__version_tweak:
    .word   CFG_VERSION_TWEAK

    .org    0x100
_reset_vector:
    l.nop   0x15
    l.nop   0x15
#if defined(__SM32_LITTLE_ENDIAN)
    .byte   0x18,0x60,0x00,0x00     # l.movhi   r3,0x0
    .byte   0xa8,0x63,0x00,0x00     # l.ori     r3,r3,0x0
    .byte   0x9c,0x83,0x00,0x81     # l.addi    r4,r3,(SPR_SR_LEE|SPR_SR_SM)
    .byte   0xc0,0x03,0x20,0x40     # l.mtspr   r3,r4,(SPR_ESR_BASE)
    .byte   0xc0,0x03,0x20,0x11     # l.mtspr   r3,r4,(SPR_SR)
    .byte   0x15,0x00,0x00,0x15     # l.nop     0x15
    .byte   0x15,0x00,0x00,0x15     # l.nop     0x15
    .byte   0x15,0x00,0x00,0x15     # l.nop     0x15
    .byte   0x15,0x00,0x00,0x15     # l.nop     0x15
#endif
    l.movhi r0,0x0
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

    l.movhi r3,hi(_start)
    l.ori   r3,r3,lo(_start)
    l.jr    r3
    l.nop


.org 0x200
_except_200:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0x200
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0x300
_except_300:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0x300
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0x400
_except_400:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0x400
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0x500
_except_500:
    l.nop
    l.j vPortTickHandler
    l.nop


.org 0x600
_except_600:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0x600
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0x700
_except_700:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0x700
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0x800
_except_800:
    l.nop
    l.j vPortExtIntHandler
    l.nop


.org 0x900
_except_900:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0x900
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0xa00
_except_a00:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0xa00
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0xb00
_except_b00:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0xb00
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0xc00
_except_c00:
    .global     vPortSystemCall
    l.nop
    l.j vPortSystemCall
    l.nop


.org 0xd00
_except_d00:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0xd00
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0xe00
_except_e00:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0xe00
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop


.org 0xf00
_except_f00:
    l.nop
    l.sw    -4(r1), r3
    l.addi  r3, r0, 0xf00
    l.sw    -260(r1), r3
    l.lwz   r3, -4(r1)
    l.j     vPortMiscIntHandler
    l.nop

/*****************************************************************************
* Pointers of Print Buffer
*/
    .org    0x1000 - 12
    .global __printbuf_addr
__printbuf_addr:
    .word   __printbuf_base
    .global __printbuf_size
__printbuf_size:
    .word   CFG_DBG_PRINTBUF_SIZE
    .global __printbuf_ptr
__printbuf_ptr:
    .word   0

    .section .text

_start:
# Check the processor number
    l.addi  r10,r0,SPR_VR
    l.mfspr r11,r10,0
    l.andi  r12,r11,SPR_VER_N
    l.sfeqi r12,0
    l.bf    .proc1
    l.nop
    l.sfeqi r12,0x80
    l.bf    .proc3
    l.nop

# processor 2
.proc2:
    # Enable Cache
    l.jal   ic_enable
    l.nop
    l.jal   dc_enable
    l.nop

    # Set the return address to _exit
    l.movhi r9,hi(exit)
    l.ori   r9,r9,lo(exit)

    l.movhi r1,hi(codec_stack)
    l.ori   r1,r1,lo(codec_stack)
    l.addi  r2, r1, 0x0

    l.movhi r5,hi(codec_start_addr)
    l.ori   r5,r5,lo(codec_start_addr)
    l.movhi r3,hi(codec_api)
    l.ori   r3,r3,lo(codec_api)
    l.lwz   r4,CODEC_ENTRY_OFFSET(r5)

    l.jalr  r4
    l.nop

1:  l.j     1b
    l.nop

# processor 3
.proc3:
    # Enable Cache
    l.jal   ic_enable
    l.nop
    l.jal   dc_enable
    l.nop
    l.movhi r9,hi(_exit)
    l.ori   r9,r9,lo(_exit)

    l.movhi r1,hi(codec_ex_stack)
    l.ori   r1,r1,lo(codec_ex_stack)
    l.addi  r2, r1, 0x0

    l.movhi r5,hi(codec_ex_start_addr)
    l.ori   r5,r5,lo(codec_ex_start_addr)
    l.movhi r3,hi(codec_api)
    l.ori   r3,r3,lo(codec_api)
    l.lwz   r4,CODEC_ENTRY_OFFSET(r5)

    l.jalr  r4
    l.nop

1:  l.j     1b
    l.nop

# processor 1
.proc1:
    # Enable Cache
    l.jal   ic_enable
    l.nop
    l.jal   dc_enable
    l.nop

    /* Set stack pointer */
    l.movhi r1, hi(__stack_end__-4)
    l.ori   r1, r1, lo(__stack_end__-4)
    l.sw    0x4(r1), r0

    /* clear BSS */
    l.movhi r2, hi(__bss_start__)
    l.ori   r2, r2, lo(__bss_start__)
    l.movhi r3, hi(__bss_end__)
    l.ori   r3, r3, lo(__bss_end__)
1:
    l.sfeq  r2, r3
    l.bf    __main
    l.nop

    l.sw    0x0(r2), r0
    l.addi  r2, r2, 0x4
    l.j     1b
    l.nop

    /* Jump to main */
__main:
    l.movhi r2,hi(BootInit)
    l.ori   r2,r2,lo(BootInit)
    l.jalr  r2
    l.nop

    l.movhi r2,hi(main)
    l.ori   r2,r2,lo(main)
    l.jr    r2
    l.nop


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
