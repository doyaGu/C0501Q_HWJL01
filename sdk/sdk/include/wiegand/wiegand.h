#ifndef _WIEGAND_H
#define _WIEGAND_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "ite/itp.h"

#define WIEGAND_API


#ifdef __cplusplus
extern "C" {
#endif

typedef enum WIEGANDID_TAG
{
    wiegand_0,
    wiegand_1,
    wiegand_max,
}WIEGANDID;

WIEGAND_API void init_wiegand_controller(WIEGANDID id);
WIEGAND_API void wiegand_controller_enable(WIEGANDID id, int d0pin, int d1pin);
WIEGAND_API void wiegand_set_bitcnt(WIEGANDID id, int bitcnt);
WIEGAND_API void wiegand_suspend(WIEGANDID id);
WIEGAND_API void wiegand_resume(WIEGANDID id);

#ifdef __cplusplus
}
#endif


#endif
