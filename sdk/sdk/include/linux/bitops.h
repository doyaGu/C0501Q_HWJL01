#ifndef BITOPS_H
#define BITOPS_H


static inline int
test_bit(int nr, const volatile void * addr)
{
    return (1UL & (((const int *)addr)[nr >> 5] >> (nr & 31))) != 0UL;
}

/*
* WARNING: non atomic version.
*/
static inline void
set_bit(unsigned long nr, volatile void * addr)
{
    int *m = ((int *)addr) + (nr >> 5);

    *m |= 1 << (nr & 31);
}

/*
* WARNING: non atomic version.
*/
static inline void
clear_bit(unsigned long nr, volatile void * addr)
{
    int *m = ((int *)addr) + (nr >> 5);

    *m &= ~(1 << (nr & 31));
}

/*
* WARNING: non atomic version.
*/
static inline int
test_and_set_bit(unsigned long nr, volatile void * addr)
{
    unsigned long mask = 1 << (nr & 0x1f);
    int *m = ((int *)addr) + (nr >> 5);
    int old = *m;

    *m = old | mask;
    return (old & mask) != 0;
}

/*
* WARNING: non atomic version.
*/
static inline int
test_and_clear_bit(unsigned long nr, volatile void * addr)
{
    unsigned long mask = 1 << (nr & 0x1f);
    int *m = ((int *)addr) + (nr >> 5);
    int old = *m;

    *m = old & ~mask;
    return (old & mask) != 0;
}


#endif // BITOPS_H
