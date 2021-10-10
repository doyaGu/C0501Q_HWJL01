/* This example flushes all buffers, closes any open files, and ends the
   program if it cannot open the file myfile.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ite/itp.h"

FILE *stream;

void test_stgt_env()
{
   char *x;

   /* set environment variable _EDC_ANSI_OPEN_DEFAULT to "Y" */
   setenv("_EDC_ANSI_OPEN_DEFAULT","Y",1);

   /* set x to the current value of the _EDC_ANSI_OPEN_DEFAULT*/
   x = getenv("_EDC_ANSI_OPEN_DEFAULT");

   printf("program1 _EDC_ANSI_OPEN_DEFAULT = %s\n",
      (x != NULL) ? x : "undefined");

   /* clear the Environment Variables Table */
   unsetenv("_EDC_ANSI_OPEN_DEFAULT");

   /* set x to the current value of the _EDC_ANSI_OPEN_DEFAULT*/
   x = getenv("_EDC_ANSI_OPEN_DEFAULT");

   printf("program1 _EDC_ANSI_OPEN_DEFAULT = %s\n",
      (x != NULL) ? x : "undefined");
}


void test_exit()
{
   exit(0);
}

void* TestFunc(void* arg)
{
    itpInit();
    
	printf("\nTest : setenv() , getenv()\n--------------------\n");    
	test_stgt_env();
	
	printf("\nTest : exit()\n--------------------\n");
	test_exit();
	
	printf("\nEnd the test\n");
}

