#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <pthread.h>
#include "ite/itp.h"


void traverse(char *fn, int indent)
{
	DIR *dir;
  	struct dirent *entry;
  	int count;
  	char path[1025];
  	struct stat info;

  	for (count=0; count<indent; count++)
		printf("  ");
  	printf("%s\n", fn);

  	if ((dir = opendir(fn)) == NULL)
    	perror("opendir() error");
  	else 
	{
    	while ((entry = readdir(dir)) != NULL) 
		{
      		if (entry->d_name[0] != '.') 
			{
        		strcpy(path, fn);
        		strcat(path, "/");
        		strcat(path, entry->d_name);
        		if (stat(path, &info) != 0)
          			fprintf(stderr, "stat() error on %s: %s\n", path,strerror(errno));
        		else if (S_ISDIR(info.st_mode))
               		traverse(path, indent+1);
      		}
    	}
    	closedir(dir);
  	}
}

/*	This example opens a directory.	*/
void test_opendir()
{
	puts("Directory structure:");
  	traverse("/", 0);
}

/*	This example closes a directory.	*/
void test_closedir()
{
	DIR *dir;
  	struct dirent *entry;
  	int count;

  	if ((dir = opendir("/")) == NULL)
    	perror("opendir() error");
  	else
  	{
    	count = 0;
    	while ((entry = readdir(dir)) != NULL)
    		printf("directory entry %03d: %s\n", ++count, entry->d_name);
    	if (closedir(dir) < 0)
			perror("closedir() error");
    	else
			printf("closedir() success!!\n");
  	}
}

/*	This example reads the contents of a root directory.	*/
void test_readdir()
{
	DIR *dir;
  	struct dirent *entry;

  	if ((dir = opendir("/")) == NULL)
    	perror("opendir() error");
  	else
  	{
    	puts("contents of root:");
    	while ((entry = readdir(dir)) != NULL)
      		printf("  %s\n", entry->d_name);
    	closedir(dir);
  	}
}

/* 
   This example produces the contents of a directory by opening it,
   rewinding it, and closing it.
*/
void test_rewinddir()
{
	DIR *dir;
  	struct dirent *entry;

  	if ((dir = opendir("/")) == NULL)
    	perror("opendir() error");
  	else
  	{
    	puts("contents of root:");
    	while ((entry = readdir(dir)) != NULL)
      		printf("%s \n", entry->d_name);
      
    	rewinddir(dir);   
    	puts("");   

		puts("After rewinddir()...");
		puts("contents of root:");
    	while ((entry = readdir(dir)) != NULL)
      		printf("%s \n", entry->d_name);
    	closedir(dir); 
    	puts("");
  	}
}

void* TestFunc(void* arg)
{
    itpInit();
	
    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif		
	
	printf("\nTest : opendir()\n------------------\n");
	test_opendir();
	printf("\nTest : readdir()\n------------------\n");
    test_readdir();
	printf("\nTest : rewinddir()\n------------------\n");   
	test_rewinddir();
	printf("\nTest : closedir()\n------------------\n"); 
    test_closedir();
    
    printf("\nEnd Test\n");
}

