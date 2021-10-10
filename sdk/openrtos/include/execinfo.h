#ifndef ITP_EXECINFO_H
#define ITP_EXECINFO_H

#include "ite/ith.h"
#ifdef __cplusplus
extern "C"
{
#endif

/* ARM replacement for unsupported gcc __builtin_return_address(n)
 * where 0 < n.  n == 0 is supported here as well.
 *
 * Walk up the stack frame until the desired frame is found or a NULL
 * fp is encountered, return NULL in the latter case.
 *
 * Note: it is possible under code optimization for the stack invocation
 * of an ancestor function (level N) to be removed before calling a
 * descendant function (level N+1).  No easy means is available to deduce
 * this scenario with the result being [for example] caller_addr(0) when
 * called from level N+1 returning level N-1 rather than the expected
 * level N.  This optimization issue appears isolated to the case of
 * a call to a level N+1 routine made at the tail end of a level N
 * routine -- the level N frame is deleted and a simple branch is made
 * to the level N+1 routine.
 */
#ifdef __arm__
#define itpReturnAddress(i, addr)                           \
{                                                           \
   asm volatile(                                            \
"  mov     ip, %1                                    \n\t"	\
"  mov     r0, fp                                    \n\t"	\
"  cmp     ip, #0                                    \n\t"	\
"  beq     1f                                        \n\t"	\
"  sub     ip, ip, #1                                \n\t"	\
"3:                                                  \n\t"	\
"  cmp     ip, #0                                    \n\t"	\
"  beq     2f                                        \n\t"	\
"  ldr     r0, [r0, #-4]                             \n\t"	\
"  sub     ip, ip, #1                                \n\t"	\
"  b       3b                                        \n\t"	\
"2:                                                  \n\t"	\
"  ldr     %0, [r0, #0]                              \n\t"	\
"  b       0f                                        \n\t"	\
"1:                                                  \n\t"	\
"  mov     %0, lr                                    \n\t"	\
"0:                                                  \n\t"	\
   : "=r"(addr)                                             \
   : "r"(i)                                                 \
   : "ip", "r0", "lr"                                       \
   );                                                       \
}

#define itpReturnAddressIrq(i, addr)                        \
{                                                           \
   asm volatile(                                            \
"  mov     ip, %1                                    \n\t"	\
"  mov     r0, #0xf00                                \n\t"	\
"  orr     r0, r0, #0xec                             \n\t"	\
"  ldr     lr, [r0, #0x0]                            \n\t"	\
"  mov     r0, #0xff0                                \n\t"	\
"  ldr     r0, [r0, #0x0]                            \n\t"	\
"  cmp     ip, #0                                    \n\t"	\
"  beq     1f                                        \n\t"	\
"  sub     ip, ip, #1                                \n\t"	\
"  cmp     ip, #0                                    \n\t"	\
"  stmdb   sp!, {lr}^                                \n\t"	\
"  ldmia   sp!, {lr}                                 \n\t"	\
"  beq     1f                                        \n\t"	\
"  sub     ip, ip, #1                                \n\t"	\
"3:                                                  \n\t"	\
"  cmp     ip, #0                                    \n\t"	\
"  beq     2f                                        \n\t"	\
"  ldr     r0, [r0, #-4]                             \n\t"	\
"  sub     ip, ip, #1                                \n\t"	\
"  b       3b                                        \n\t"	\
"2:                                                  \n\t"	\
"  ldr     %0, [r0, #0]                              \n\t"	\
"  b       0f                                        \n\t"	\
"1:                                                  \n\t"	\
"  mov     %0, lr                                    \n\t"	\
"0:                                                  \n\t"	\
   : "=r"(addr)                                             \
   : "r"(i)                                                 \
   : "ip", "r0", "lr"                                       \
   );                                                       \
}
#elif defined(__SM32__)
#define itpReturnAddress(i, addr)                           \
{                                                           \
   asm volatile(                                            \
"  l.ori    r21, %1, 0x0                             \n\t"  \
"  l.addi   r23, r1, 0x8                             \n\t"  \
"  l.sfeqi  r21, 0x0                                 \n\t"  \
"  l.bf     1f                                       \n\t"  \
"  l.nop    0x0                                      \n\t"  \
"  l.addi   r21, r21, 0xffffffff                     \n\t"  \
"3:                                                  \n\t"  \
"  l.sfeqi  r21, 0x0                                 \n\t"  \
"  l.bf     2f                                       \n\t"  \
"  l.nop    0x0                                      \n\t"  \
"  l.lwz    r23, 0x4(r23)                            \n\t"  \
"  l.addi   r21, r21, 0xffffffff                     \n\t"  \
"  l.j      3b                                       \n\t"  \
"  l.nop    0x0                                      \n\t"  \
"2:                                                  \n\t"  \
"  l.lwz    %0, 0x0(r23)                             \n\t"  \
"  l.j      0f                                       \n\t"  \
"  l.nop    0x0                                      \n\t"  \
"1:                                                  \n\t"  \
"  l.ori    %0, r9, 0x0                              \n\t"  \
"0:                                                  \n\t"  \
   : "=r"(addr)                                             \
   : "r"(i)                                                 \
   : "r21", "r23"                                           \
   );                                                       \
}
#elif defined(__riscv)
// TODO: RISCV
#define itpReturnAddress(i, addr)                           \
{                                                           \
}
#elif defined(__NDS32__)
// TODO: NDS32
#define itpReturnAddress(i, addr)                           \
{                                                           \
}
#endif // __arm__

/* Store up to SIZE return address of the current program state in
   ARRAY and return the exact number of values stored.  */
/* compatibility functions for libexecinfo and glibc */
static inline int backtrace(void **bt, int depth) __attribute__((always_inline)); 
static inline int backtrace(void **bt, int depth)
{
#ifdef __arm__
extern int _text_end;
extern caddr_t __heap_end__;

    void *addr;
    int i   = 0;
    unsigned int fpReg, lrReg;
    int bTopFunctionCall = 1;
    
    asm volatile ("mov %0, fp" : "=r" (fpReg));
    asm volatile ("mov %0, lr" : "=r" (lrReg));

    if (depth <= 0)
        return(-1);

    addr = lrReg;
    bt[i] = (void*) addr;
    i++;
    lrReg = 0;

    while(i < depth)
    {
        int prevFpReg = fpReg;
        int addr = 0;

        if (prevFpReg & 0x3 || prevFpReg > (unsigned int)&__heap_end__)
        {
            break;
        }

        if (lrReg)
        {
            addr = lrReg;
        }
        else if (prevFpReg > (unsigned int)&_text_end)
        {
            addr = *((unsigned int*)prevFpReg);
        }
        else
        {
            addr = prevFpReg;
        }

        // alignment and valid boundary check
        if (addr == 0 || (unsigned int)addr & 0x3 || (unsigned int)addr > (unsigned int)&__heap_end__) 
            break;

        bt[i] = (void*) addr;
        if (lrReg) // case of no push lr
        {
            fpReg = *((int*)(prevFpReg));
            lrReg = 0;
        }
        else
        {
            fpReg = *((int*)(prevFpReg - 4));
        }

        if (bTopFunctionCall)
        {
            if (fpReg & 0x3 || fpReg > (unsigned int)&__heap_end__)
            {
                break;
            }

            if (fpReg > (unsigned int)&_text_end && *((unsigned int*)fpReg) > (unsigned int)&_text_end)
            {
                //switch to user mode to grap lr of user mode.
                asm volatile("msr cpsr, %0\n" : : "r"(0x10) : "cc");
                asm volatile ("mov %0, lr" : "=r" (lrReg));
            }
            bTopFunctionCall = 0;
        }
        i++;
    }

    return(i);
#else
    void *addr;
    int i   = 0;
    void **p = 0;

    if (depth <= 0)
        return(-1);

    p = bt;
        
    do
    {               
        itpReturnAddress(i, addr);

        if (addr == 0)
            break;

        if ((unsigned int)addr & 0x3) // alignment check 
            break;

        p[i] = addr;

    } while (++i < depth);

    return(i+1);
#endif
}

static inline int itpBacktraceIrq(void **bt, int depth) __attribute__((always_inline)); 
static inline int itpBacktraceIrq(void **bt, int depth)
{
    void *addr;
    int i   = 0;
    void **p = 0;

    if (depth <= 0)
        return(-1);

    p = bt;
        
    do
    {
    #ifdef __arm__
        itpReturnAddressIrq(i, addr);
    #else
        itpReturnAddress(i, addr);
    #endif

        if (addr == 0)
            break;

        if ((unsigned int)addr & 0x3) // alignment check 
            break;

        p[i] = addr;

    } while (++i < depth);

    return(i+1);
}

/* Return names of functions from the backtrace list in ARRAY in a newly
   malloc()ed memory block.  */
extern char **backtrace_symbols (void *__const *__array, int __size);

/* This function is similar to backtrace_symbols() but it writes the result
   immediately to a file.  */
extern void backtrace_symbols_fd (void *__const *__array, int __size, int __fd);

#ifdef __cplusplus
}
#endif

#endif // ITP_EXECINFO_H
