#define _POSIX_SOURCE
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"

void test_statvfs()
{

  struct statvfs info;
  
  if (statvfs("A:/", &info) == -1)	//Error here!!
  {
    perror("statvfs() error");
  }
  else
  {
    puts("statvfs() returned the following information");
    puts("about the root (/) file system:");
    printf("  f_bsize    : %u\n", info.f_bsize);
    printf("  f_files    : %u\n", info.f_files);
    printf("  f_ffree    : %u\n", info.f_ffree);
    printf("  f_fsid     : %u\n", info.f_fsid);
    printf("  f_flag     : %X\n", info.f_flag);
    printf("  f_namemax  : %u\n", info.f_namemax);
  }
}

void *TestFunc(void *arg)
{
    itpInit();
	
    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif			
	
	printf("Test: statvfs()\n*******************\n");
	test_statvfs();
}

