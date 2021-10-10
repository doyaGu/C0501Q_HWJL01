#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"

void Test_write()
{
  int fd;
  char out[20]="Test string";
  if ((fd = open("myfile", O_WRONLY | O_APPEND)) < 0)
	perror("creat error");
  else
  {	 
    if (write(fd, out, strlen(out)+1) == -1)   //Test write()
      perror("write() error");

    if (fd == 0)
	  perror("write() error");
    else
    { 
	printf("Write the \"Test string\" to the file named \"myfile\"\n");
    close(fd);  //Test close()
    printf("close() success\n");
    }
  }	
}

/* This example opens a file and reads input.*/
void Test_read()
{
  int fd,ret;
  char buf[1024];
  if ((fd = open("myfile", O_RDONLY)) < 0)
    perror("open() error");
  else
  {
    while ((ret = read(fd, buf, sizeof(buf)-1)) > 0)
	{
      buf[ret] = 0x00;
      printf("Read from the file \"myfile\", block read: \n<%s>\n", buf);
    }
    close(fd);		//Test close()
    printf("close() success\n");
  }
}

int Test_lseek()
{
        int file=0;
        if((file=open("myfile",O_RDONLY)) < -1)  return 1;
 
        char buffer[11];
        if(read(file,buffer,11) != 11)  return 1;
          printf("%s\n",buffer);
 
        if(lseek(file,6,SEEK_SET) < 0) return 1;
 
        if(read(file,buffer,6) != 6)  return 1;
          printf("%s\n",buffer);

		close(file);		//Test close()
		printf("close() success\n"); 
		return 0;
}

/*   This example determines the working directory.*/
void Test_getcwd(char new_dir[])
{
  char cwd[256];

  if (getcwd(cwd, sizeof(cwd)) == NULL)
    perror("getcwd() error");
  else
    printf("Original working directory is: \n%s\n", cwd);

  //New the directory "tmp".
  if (mkdir(new_dir, S_IRWXU|S_IRGRP|S_IXGRP ) != 0)
    perror("mkdir() error");
  printf("New a directory \"tmp\"\n\n");


  if (chdir("./tmp") != 0)   //Test chdir()
    perror("chdir() error()");
  else
  {
    if (getcwd(cwd, sizeof(cwd)) == NULL)
      perror("getcwd() error");
    else
      printf("Current working directory is: \n%s\n", cwd);
  }
}

/*	This example removes a directory.	*/
void Test_rmdir(char new_dir[])
{         
  	char cwd[256];
  	
	unlink("myfile"); //Delete the file "myfile"
	
	//Go back the origin working directory
  	if (chdir("../") != 0)   //Test chdir()
    	perror("chdir() error()");
  	else 
	{
    	if (getcwd(cwd, sizeof(cwd)) == NULL)
      		perror("getcwd() error");
    	else
      		printf("Go back the origin working directory: \n%s\n", cwd);
  	}

	printf("Wait 1 sec to remove the directory \"tmp\"\n");
    sleep(1);

  	if (rmdir(new_dir) != 0)
    	perror("rmdir() error");
  	else
    	puts("removed!");
}

void Test_usleep()
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

//   This example suspends execution for a specified time.
void Test_sleep()
{
  unsigned int ret;
  time_t t;
  time(&t);
  printf("starting sleep at %s", ctime(&t));
  ret = sleep(10);
  time(&t);
  printf("naptime over at %s", ctime(&t));
  printf("sleep() returned %d\n", ret);

}

void* TestFunc(void* arg)
{
	char new_dir[]="tmp";
	
  	itpInit();
	  
    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif			  

	printf("\nTest: getcwd(),chdir()\n-----------------\n");
	Test_getcwd(new_dir);

	printf("\nTest: write(),close()\n-----------------\n");
	Test_write();
	
	printf("\nTest: read(),close()\n-----------------\n");
	Test_read();
	
	printf("\nTest: lseek()\n-----------------\n");
	Test_lseek();

	printf("\nTest: rmdir()\n-----------------\n");
	Test_rmdir(new_dir);
	
	printf("\nTest: usleep()\n---------------\n");
	Test_usleep();
	
	printf("\nTest: sleep()\n---------------\n");
	Test_sleep();
	
	printf("\nEnd the test\n");
}
