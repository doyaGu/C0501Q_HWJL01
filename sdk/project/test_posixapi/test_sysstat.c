#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "ite/itp.h"

//   This example gets status information for the file called temp.file.
void Test_fstat()
{
  char fn[]="temp.file";
  struct stat info;
  int fd;

  if ((fd = open(fn, O_WRONLY | O_APPEND)) < 0)
    perror("creat() error");
  else {
    if (fstat(fd, &info) != 0)
      perror("fstat() error");
    else {
      puts("fstat() returned:");
      printf("  inode:   %d\n",   (int) info.st_ino);
      printf(" dev id:   %d\n",   (int) info.st_dev);
      printf("   mode:   %08x\n",       info.st_mode);
      printf("  links:   %d\n",         info.st_nlink);
      printf("    uid:   %d\n",   (int) info.st_uid);
      printf("    gid:   %d\n",   (int) info.st_gid);
      printf("created:   %s",           ctime(&info.st_ctime));
    }
    close(fd);
    unlink(fn);
  }
}


//   This example changes the permission from the file owner to the file's group.
void Test_chmod()
{
  char fn[]="./temp.file";
  FILE *stream;
  struct stat info;

  if ((stream = fopen(fn, "w")) == NULL)
    perror("fopen() error");
  else {
    fclose(stream);
    stat(fn, &info);
    printf("original permissions were: %08x\n", info.st_mode);
    if (chmod(fn, S_IRWXU) != 0)
      perror("chmod() error");
    else {
      stat(fn, &info);
      printf("after chmod(), permissions are: %08x\n", info.st_mode);
    }
    unlink(fn);
  }
}


//  The following example creates a new directory.
void Test_mkdir()
{
  char new_dir[]="new_dir";

  if (mkdir(new_dir,S_IRWXU | S_IRWXG) != 0)
    perror("mkdir() error");
  else if (chdir(new_dir) != 0)
    perror("first chdir() error");
  else if (chdir("..") != 0)
    perror("second chdir() error");
  else if (rmdir(new_dir) != 0)
    perror("rmdir() error");
  else
    puts("success!");
}

//   This example gets status information about a file.
void Test_stat()
{
  struct stat info;
  int fd;
  char fn[]="test_stat_file";
  if ((fd = open(fn, O_WRONLY | O_APPEND)) < 0)
    perror("creat() error");
  else
  {	
  	if (stat(fn, &info) != 0)
    	perror("stat() error");
  	else 
	{
    	puts("stat() returned the following information about root f/s:");
    	printf("  inode:   %d\n",   (int) info.st_ino);
    	printf(" dev id:   %d\n",   (int) info.st_dev);
    	printf("   mode:   %08x\n",       info.st_mode);
    	printf("  links:   %d\n",         info.st_nlink);
    	printf("    uid:   %d\n",   (int) info.st_uid);
    	printf("    gid:   %d\n",   (int) info.st_gid);
    	printf("created:   %s",           ctime(&info.st_ctime));
	}
	close(fd);
    unlink(fn);
  }
}

void* TestFunc(void* arg)
{
	itpInit();
	
    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif			
	
	printf("\nTest: fstat()\n-------------\n");
	Test_fstat();
	printf("\nTest: chmod()\n-------------\n");
	Test_chmod();
	printf("\nTest: mkdir()\n-------------\n");
	Test_mkdir();
	printf("\nTest: stat()\n-------------\n");
	Test_stat();	
	
	printf("\nEnd the test\n");
}
