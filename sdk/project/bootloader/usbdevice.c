#include <sys/ioctl.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "bootloader.h"
#include "config.h"

static ITCArrayStream arrayStream;

ITCStream* OpenUsbDevicePackage(void)
{
    // TODO: IMPLEMENT

    //itcArrayStreamOpen(&arrayStream, ftpBuf.buf, ftpBuf.pos);

    return &arrayStream.stream;

error:
    return NULL;
}
