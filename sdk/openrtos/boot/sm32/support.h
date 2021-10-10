/* Support file for or32 tests.  This file should is included
   in each test. It calls main() function and add support for
   basic functions */

#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdarg.h>
#include <stddef.h>
#include <limits.h>

/* Register access macros */
#define REG8(add)	*((volatile unsigned char *)(add))
#define REG16(add) 	*((volatile unsigned short *)(add))
#define REG32(add) 	*((volatile unsigned long *)(add))

/* Function to be called at entry point - not defined here.  */
int main(int argc, char **argv);

/* Prints out a value */
void report(unsigned long value);

/* return value by making a syscall */
extern void or32_exit (int i) __attribute__ ((__noreturn__));

/* cache related functions */
int cache_init(void);
void icache_disable(void);
void icache_enable(void);
void dcache_disable(void);
void dcache_enable(void);
int checkdcache(void);
int dcache_status(void);
int checkicache(void);
int icache_status(void);
void flush_cache(unsigned long addr, unsigned long size);
void invalidate_dcache_range(unsigned long addr, unsigned long stop);
void flush_dcache_range(unsigned long addr, unsigned long stop);

#endif /* SUPPORT_H */
