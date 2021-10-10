/*
 * filename: crt0.c
 * date    : 2013/03/28
 * desc    : startup file
 */

extern int _bss_start;
extern int _bss_end;
extern int _stack;
int main(void);
void clear_bss(void);

void reset(void) __attribute__ ((section(".reset"), naked));
void reset(void) {
    register int i;

    asm volatile("        l.j     2f");         // 0x00: reset vector
    asm volatile("1:      l.j     1b");         // 0x04: illegal inst
    asm volatile("1:      l.j     1b");         // 0x08: external interrupt
    asm volatile("1:      l.j     1b");         // 0x0C: system call
    asm volatile("1:      l.j     1b");         // 0x10: trap
    asm volatile("1:      l.j     1b");         // 0x14: NMI
    asm volatile("1:      l.j     1b");         // 0x18: Bus Error

    // setup stack point
    asm volatile("2:      l.movhi r1, hi(_stack)");
    asm volatile("        l.ori   r1, r1, lo(_stack)");
    asm volatile("        l.add   r2, r0, r1");

    // clear BSS
    for (i = (int)&_bss_start; i < (int)&_bss_end; i += 4)
        *(volatile int*)(i) = 0;

    main();

    while(1);
}

