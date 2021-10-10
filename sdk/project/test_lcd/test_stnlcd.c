#include <sys/ioctl.h>
#include <unistd.h>
#include "ite/itp.h"

void* TestFunc(void* arg)
{
    itpInit();

    while (1)
    {
        int x;
        for (x = 0; x < 64; x++)
        {
            ioctl(ITP_DEVICE_STNLCD, ITP_IOCTL_CLEAR, NULL);
            itpStnLcdDrawText(x, 0, "Hello world!");
            ioctl(ITP_DEVICE_STNLCD, ITP_IOCTL_FLUSH, NULL);

            sleep(1000);
        }
    }

    return NULL;
}
