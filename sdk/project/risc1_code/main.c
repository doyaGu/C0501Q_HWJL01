#include <stdarg.h>

#include "ite/itp.h"

#define ENDIAN_SWAP16(x) \
        (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))

#define ENDIAN_SWAP32(x) \
        (((x & 0x000000FF) << 24) | \
        ((x & 0x0000FF00) <<  8) | \
        ((x & 0x00FF0000) >>  8) | \
        ((x & 0xFF000000) >> 24))

int main(int argc, char **argv)
{
    ithWriteRegH(0x1698, 0x0000);
    while(1) 
    {
        ithWriteRegH(0x1698, ithReadRegH(0x1698)+1);
    }
}
