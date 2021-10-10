#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

void start_clock(void);
void end_clock(char *msg);

static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

void test_start_clock()
{
    st_time = times(&st_cpu);
}

/* This example assumes that the result of each subtraction
   is within the range of values that can be represented in
   an integer type. */
void test_end_clock(char *msg)
{
    en_time = times(&en_cpu);
    
    fputs(msg,stdout);
    printf("\nReal Time: %d, User Time %d, System Time %d\n",
        (intmax_t)(en_time - st_time),
        (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
        (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
}

void* TestFunc(void* arg)
{
    itpInit();
	
	printf("Test : times()\n******************\n");
	test_start_clock();
	sleep(1);
	test_end_clock("test_times");
}

