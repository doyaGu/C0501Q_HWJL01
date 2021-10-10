#include <pthread.h>
#include <malloc.h>
#include <stdio.h>
#include "ite/itp.h"

void test_callmalloc()
{
  	long * array;    /* start of the array */
  	long * index;    /* index variable     */
  	long * ptr;
  	int    i;        /* index variable     */

	long num1 = rand()%1024*16+1;
	long num2 = rand()%1024*16+1;

  	/* allocate num entries */
  	if ( (index = array = (long * )malloc( num1 * sizeof( long ))) != NULL )
  	{
    	for ( i = 0; i < num1; ++i )          /* put values in array    */
       	   *index++ = i;                      /* using pointer notation */
        printf("malloc success\n");
        free( array );                        /* deallocates array  */
    }
    else
    {                                         /* malloc error */
        printf( "Out of storage\n" );
        abort();
    }

//////////////////////////////////////////////////////////////////////////////////////////

    if ( (index = array = (long *)calloc( num1, sizeof( long ))) != NULL )
  	{
    	for ( i = 0; i < num1; ++i )           /* put values in array    */
       		*index++ = i;                      /* using pointer notation */
		printf("calloc success\n");
		free( array );                         /* deallocates array  */
  	}
  	else
  	{                                          /* out of storage */
    	printf( "Out of storage\n" );
    	abort();
  	}

//////////////////////////////////////////////////////////////////////////////////////////

	if ( (array = (long *)malloc( num1 * sizeof( long ))) != NULL )
	{
    	for ( ptr = array, i = 0; i < num1 ; ++i ) /* assign values */
        	*ptr++ = i;
     	printf("malloc for realloc test\n");
  	}	                                                                             
  	else
	{ /*  malloc error  */
    	printf( "Out of storage\n" );
    	abort();
  	}
  	/* Change the size of the array ... */
  	if ( (array = (long *)realloc( array, num2* sizeof( long ))) != NULL )
  	{                                                                             
     	for ( ptr = array + num1, i = num1; i < num2; ++i )                       
        	*ptr++ = i + 2000;  /* assign values to new elements */
		printf("realloc success\n");
		free( array );                     /* deallocates array  */
  	}
  	else
	{                                           /* realloc error */
    	printf( "Out of storage\n" );
    	abort();
    }
}

#define TABLE_SIZE 512
void test_mallinfo()
{
   size_t bufferSize;
   char *bufferAddr;
   struct mallinfo info;

   // what is the largest heap buffer currently available?
   info = mallinfo();

   // Is this large enough to hold our table?
   if (info.fordblks < TABLE_SIZE)
   {
       printf("No large enough to hold the table\n");
       printf("get the maximum allowable buffer size\n");
	   bufferSize = info.fordblks;
   }
   else
   {
	   printf("set the needed buffer size to size of table\n");
	   bufferSize = TABLE_SIZE;
   }

   printf("Allocate the buffer\n");
   
   bufferAddr = (char *) malloc(bufferSize);
}


//test_malloc_stats()
int func1(void)
{
	char *p=NULL;
	p=(char *)malloc(1024*1024);
	
	if(!p)
		printf("Malloc error\n");
	
	return 0;
}

void test_malloc_stats()
{
	char *p=NULL;
	p=(char *)malloc(100);
	if(!p)
	{
		printf("In main ,malloc fail\n");
		return;
	}
	printf("********before call func1 **********\n");
	malloc_stats();
	
	func1();
	printf("\n@@@@@@@@after call func1 @@@@@@@@@@@\n");
	malloc_stats();
	free(p);
}


void test_memalign()
{
	char *page = memalign(1024, 1024 * 4);
	if(	page != NULL )
	{
		printf("memalign success...\n");
		free(page);
	}
	else	printf( "Out of storage\n" );
}

void* TestFunc(void* arg)
{
  	itpInit();
	
	printf("\nTest: malloc() , free() , calloc() , realloc()\n-------------------\n");
	test_callmalloc();

	printf("\nTest: test_memalign()\n-------------------\n");	
	test_memalign();
	
	printf("\nTest: test_mallinfo()\n-------------------\n");
	test_mallinfo();
	
	printf("\nTest: test_malloc_stats()\n-------------------\n");
	test_malloc_stats();
	
    printf("\nEnd the test\n");
}

