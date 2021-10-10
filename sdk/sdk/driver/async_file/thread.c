#include "async_file/pal.h"
#include <unistd.h>

void
PalSleep(
    MMP_ULONG ms)
{
    usleep(ms * 1000);
}
