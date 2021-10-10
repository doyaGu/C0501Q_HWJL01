#include <sys/ioctl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "ite/itp.h"

void* TestFunc(void* arg)
{
    struct timeval tv;
    struct tm* tm;
    int i;

    for (i = 0; i < 5; i++)
    {
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        printf("RTC: sec=%02d, %s", tm->tm_sec, ctime(&tv.tv_sec));

        sleep(1);
    }
    
    tv.tv_sec += 10;
    
    settimeofday(&tv, NULL);
    
    for (;;)
    {
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        printf("RTC: sec=%02d, %s", tm->tm_sec, ctime(&tv.tv_sec));

        sleep(1);
    }    
    
    return NULL;
}
