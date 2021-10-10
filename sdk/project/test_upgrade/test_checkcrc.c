#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/ug.h"

void* TestFunc(void* arg)
{
    int ret;

    itpInit();

    ret = ugCheckFilesCrc(CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
    printf("check files crc result: %d\n", ret);

    ret = ugSetFileCrc("/ctrlboard.ini", CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
    printf("set crc result: %d\n", ret);

    ret = ugCheckFilesCrc(CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
    printf("check files crc result: %d\n", ret);

    ret = ugUpgradeFilesCrc(CFG_PUBLIC_DRIVE ":", CFG_PUBLIC_DRIVE ":/ite_crc.dat");
    printf("upgrade files crc result: %d\n", ret);

    return NULL;
}
