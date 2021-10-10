#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "ite/itp.h"

void test_gettimeofday()
{
  struct timeval tv, tv2;
  unsigned long long start_utime, end_utime;

  gettimeofday(&tv,NULL);
  start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
 
  usleep(1000);
  gettimeofday(&tv2,NULL);
  end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;
 
  printf(" runtime = %llu\n", end_utime - start_utime );
}

void test_settimeofday()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	
	printf("tv_sec:%d,tv_usec:%d.\n",tv.tv_sec,tv.tv_usec);
	printf("tz_minuteswest:%d,tz_dsttime:%d.\n",tz.tz_minuteswest,tz.tz_dsttime);
	
	if(settimeofday(&tv,&tz) == -1)
	{
		if(errno == EPERM)
			printf("The calling process has insufficient privilege to call settimeofday()\n");
		else if( errno == EINVAL)
			printf("Timezone (or something else) is invalid.\n");
		else if(errno == EFAULT)
			printf("One of tv or tz pointed outside the accessible address space.\n");
		else
			printf("Unknow Error.\n");
	}
}

void* TestFunc(void* arg)
{
	itpInit();
	
	printf("\nTest : gettimeofday()\n-------------------\n");
	test_gettimeofday();
	printf("\nTest : settimeofday()\n-------------------\n");
    test_settimeofday();
    
    printf("\nEnd the test\n");
}

