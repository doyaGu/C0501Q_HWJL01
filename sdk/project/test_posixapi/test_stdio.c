#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"


/* This example is to print the answer of variables */
void test_printf()
{
    int a,b;
    float c,d;

    a = 15;
    b = a / 2;
    printf("%d\n",b);
    printf("%3d\n",b);
    printf("%03d\n",b);

    c = 15.3;
    d = c / 3;
    printf("%3.2f\n",d);
}



/*
   This example opens a file myfile.dat for reading as a stream and then
   closes the file. 
*/
void test_fclose()
{
   FILE *stream;

   stream = fopen("myfile.dat", "r");

   if (fclose(stream))   // Close the stream. 
       printf("fclose error\n");
   else
       printf("fclose successful\n");
}



/*
   This example attempts to read NUM_ALPHA characters from the
   file myfile.dat.
   If there are any errors with either &fread. or &fopen., a
   message is printed.
*/
void test_fread()
{
  int NUM_ALPHA = 50;
  	
  FILE * stream;
  int num;       // number of characters read from stream 

  // Do not forget that the '\0' char occupies one character too! 
  char buffer[NUM_ALPHA + 1];
  buffer[NUM_ALPHA] = '\0';

  if (( stream = fopen("myfile.dat", "r"))!= NULL )
  {
    num = fread( buffer, sizeof( char ), NUM_ALPHA, stream );
    if (num == NUM_ALPHA) {  // fread success 
      printf( "Number of characters read = %i\n", num );
      printf( "buffer = %s\n", buffer );
      fclose( stream );
    }
    else 
    {                               // fread failed 
      if ( ferror(stream) )         // possibility 1 
        printf( "Error reading myfile.dat" );
      else if ( feof(stream))
      {                             // possibility 2 
        printf( "EOF found\n" );
        printf( "Number of characters read %d\n", num );
        printf( "buffer = %.*s\n", num, buffer);
      }
    }
  }
  else
    printf( "Error opening myfile.dat\n" );
}


/*
   This example opens a file myfile.dat for reading.
   After performing input operations (not shown), it moves the file
   pointer to the beginning of the file.
*/
void test_fseek()
{
   FILE *stream;
   int result;

   if (stream = fopen("myfile.dat", "r"))
   { // successful 
   		if (fseek(stream, 0L, SEEK_SET))  // moves pointer to the beginning of the file
       		printf("if not equal to 0, then error ...\n");
  		else
       		printf("fseek() successful\n");

        fclose(stream);
   }
}


/*
   This example writes NUM characters to a stream.                                                                         
   It checks that the &fopen. function is successful and that                   
   100 items are written to the stream.                                         
*/
void test_fwrite()
{
   int NUM =100;
   
   FILE *stream;
   char list[NUM];
   int numwritten, number;

   if((stream = fopen("myfile.dat", "w")) != NULL )
   {
     for (number = 0; number < NUM; ++number)
       list[number] = (char)(number+32);
     numwritten = fwrite(list, sizeof(char), NUM, stream);
     printf("number of characters written is %d\n",numwritten);
     fclose(stream);
   }
   else
     printf("fopen error\n");
}



/*
   This example gets a line of input from the stdin stream.                     
   You can also use getc(stdin) instead of &getchar. in the for                 
   statement to get a line of input from stdin.                                 
*/
void test_getchar()
{                                                                               
	int LINE = 80;

	char buffer[LINE+1];                                                          
	int i;                                                                        
	int ch;                                                                       

	printf( "Please enter string and wait 10s to show the result.\n" );                                            

   //Keep reading until either:                                                 
   //1. the length of LINE is exceeded  or                                      
   //2. the input character is EOF  or                                          
   //3. the input character is a new-line character                                                                                                         
	
	sleep(10);		//Wait for User input...10 seconds
  	
	for ( i = 0; ( i < LINE ) && (( ch = getchar()) != EOF) &&                    
               ( ch !='\n' ); ++i )                                             
    buffer[i] = ch;                                                             
                                                                                
  	buffer[i] = '\0';  // a string should always end with '\0' !              
                                                                                
  	printf( "The string is %s\n", buffer );                                       
}


/*   This example writes "Hello World" to stdout.  */
void test_puts()                                                                  
{                                                                               
  if ( puts("Hello World") == EOF )                                             
    printf( "Error in puts\n" );                                                
}


/*                                      
   When you invoke this example with a file name, the program attempts to       
   remove that file.                                                            
   It issues a message if an error occurs.                                      
*/
void test_remove()
{                                                                               
  char *s1;
  s1="Nenamed.dat"; //remove file name

  if ( remove( s1 ) != 0 )
     printf( "Could not remove file\n" );
  else
     printf( "Remove successful!\n" );
}


/*                                     
   This example takes two file names as input and uses rename() to change       
   the file name from the first name to the second name.                        
*/                                                                                                                                                             
void test_rename()
{                                                                               
  char *s1,*s2;
  
  s1="myfile.dat"; //original file name
  s2="Nenamed.dat"; //rename file name
  
  if ( rename( s1, s2 ) == 0 )                                                              
    printf( "Usage: %s old_fn new_fn %s\n", s1, s2 );                    
  else
    printf( "Could not rename file\n" );
}

void* TestFunc(void* arg)
{
	itpInit();
	
    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif	
	
	printf("\nTest : printf()\n-----------------\n");	
	test_printf();
	
	printf("\nTest : fwrite()\n-----------------\n");
    test_fwrite();

	printf("\nTest : fclose()\n-----------------\n");
    test_fclose();

	printf("\nTest : fread()\n-----------------\n");
    test_fread();

	printf("\nTest : fseek()\n-----------------\n");
    test_fseek();

	printf("\nTest : puts()\n-----------------\n");
    test_puts();
								 
	printf("\nTest : rename()\n-----------------\n");
    test_rename();    

	printf("\nTest : remove()\n-----------------\n");
    test_remove();
    
	printf("\nTest : getchar()\n-----------------\n");  
  	test_getchar();	
    
    printf("\nEnd the test\n");
}

