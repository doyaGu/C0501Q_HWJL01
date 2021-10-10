#include <sys/time.h>
#include <errno.h>     
#include <pthread.h>
#include <stdio.h>  
#include <stdlib.h>   
#include <signal.h>
#include <time.h>
#include "ite/itp.h"


/*  This example prints the time elapsed since the sleep() was invoked.  */
void test_clock()
{
	double time1, timedif;   /* use doubles to show small values */
	
	time1 = (double) clock();            /* get initial time */
    time1 = time1 / CLOCKS_PER_SEC;      /*    in seconds    */
    
	sleep(1);
    
	/* call clock a second time */
    timedif = ( ((double) clock()) / CLOCKS_PER_SEC) - time1;
    printf("The elapsed time is %f seconds\n", timedif);
}

/*
   This example shows a timing application using &diff..                        
   The example calculates how long, on average, it takes
   the sleep()                                                                       
*/
void test_difftime()                                                                  
{                                                                               
	time_t start, finish;                                                                                                                      
                                                                                                         
   	time(&start); 
	                                                                  
	sleep(1);
	                                                            
   	time(&finish);                                                               
    printf("The elapsed time is %f seconds\n",difftime(finish,start));                                             
}

/*                   
   This example prints the day of the week that is 40 days and                  
   16 hours from the current date.
*/                                                                                              
void test_mktime()                                                                  
{                                                                               
	char *wday[] = { "Sunday", "Monday", "Tuesday", "Wednesday",                    
                 	 "Thursday", "Friday", "Saturday" }; 
  	time_t t1, t3;                                                                
  	struct tm *t2;                                                                
                                                                                
  	t1 = time(NULL);                                                              
  	t2 = localtime(&t1);                                                          
  	t2 -> tm_mday += 40;                                                          
  	t2 -> tm_hour += 16;                                                          
  	t3 = mktime(t2);                                                              
                                                                                
  	printf("40 days and 16 hours from now, it will be a %s \n" ,
	  		wday[t2 -> tm_wday]);                                                 
}


/*                                    
   This example gets the time and assigns it to ltime, then uses                
   the &ctime. function to convert the number of seconds to the                 
   current date and time.                                                       
   Finally, it prints a message giving the current time.                                                                                                      
*/
void test_time()                                                                  
{                                                                               
   time_t ltime;                                                                
                                                                                
   time(&ltime);                                                                
   printf("The time is %s\n", ctime(&ltime));                                   
}


void my_handler( timer_t timerid,int myarg )
{
   printf("\nHello World\n");
}

void Test_timer(void)
{
   timer_t timerid;
   struct itimerspec	value;

   value.it_value.tv_sec = 1;
   value.it_value.tv_nsec = 0;

   value.it_interval.tv_sec = 1;
   value.it_interval.tv_nsec = 0;

   timer_create (CLOCK_REALTIME, NULL, &timerid);

   timer_connect (timerid, my_handler,0);

   timer_settime (timerid, 0, &value, NULL);
   
   sleep(5);
   timer_delete(timerid);
}


void* TestFunc(void* arg)
{
  	itpInit();
	
	printf("\nTest : clock()\n----------------\n");
	test_clock();
	
	printf("\nTest : difftime()\n----------------\n");
    test_difftime();
	
	printf("\nTest : mktime()\n----------------\n");
    test_mktime();
	
	printf("\nTest : time()\n----------------\n");
    test_time();
    
	printf("\nTest : timer()\n----------------\n");
    Test_timer();
    
    printf("\nEnd test\n");
}

