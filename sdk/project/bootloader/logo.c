#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"
#include "config.h"
#include "ite/ith.h"

__attribute__((aligned(4)))
static const uint8_t logo[] =
{
#include "logo.inc"
};

void ShowLogo(void)
{
    int size = ithLcdGetPitch() * ithLcdGetHeight();
    uint8_t* lcdBase = (uint8_t*)ithMapVram(ithLcdGetBaseAddrA(), size, ITH_VRAM_WRITE);
    int *header = (int*)logo;

    int shift_x = 0;
    int shift_y = 0;
    int logo_size = header[0];
    int logo_width = header[1];
    int logo_height = header[2];
    int logo_bpp = header[3];

    uint32_t* ptr = lcdBase;
    uint32_t color, i;

    //ithPrintf("logo_width:%d logo_height:%d logo_bpp:%d\n", logo_width, logo_height, logo_bpp);

#if CFG_LCD_BPP == 4
    color = CFG_LCD_BOOT_BGCOLOR;
#else
    color = ITH_RGB565((CFG_LCD_BOOT_BGCOLOR >> 16) & 0xFF, (CFG_LCD_BOOT_BGCOLOR >> 8) & 0xFF, CFG_LCD_BOOT_BGCOLOR & 0xFF);
    color |= color << 16;
#endif

    for (i = 0; i < size / (sizeof(uint32_t)* 8); i++)
    {
        *ptr++ = color;
        *ptr++ = color;
        *ptr++ = color;
        *ptr++ = color;
        *ptr++ = color;
        *ptr++ = color;
        *ptr++ = color;
        *ptr++ = color;
    }

    logo_bpp = (logo_bpp == 3) ? 4 : 2;
    
    if (logo_width == ithLcdGetWidth() && logo_height == ithLcdGetHeight() && ithLcdGetWidth()*CFG_LCD_BPP == ithLcdGetPitch())
    {
        memcpy(lcdBase, logo + 16, size);
    }
    else
    {
        int i;
        char *logoAddr;
        char *lcdAddr;
        int new_logo_width = ITH_MIN(logo_width, ithLcdGetWidth());
        int new_logo_height = ITH_MIN(logo_height, ithLcdGetHeight());

        shift_x = (ithLcdGetWidth() - new_logo_width) / 2;
        shift_y = (ithLcdGetHeight() - new_logo_height) / 2;

        logoAddr = logo + 16;
        lcdAddr = (char*)(lcdBase + shift_y * ithLcdGetPitch() + shift_x * logo_bpp);

        for (i = 0; i < new_logo_height; i++)
        {
            memcpy(lcdAddr, logoAddr, new_logo_width*logo_bpp);
            lcdAddr += ithLcdGetPitch();
            logoAddr += logo_width*logo_bpp;
        }
    }

    ithFlushDCacheRange(lcdBase, size);
    ithFlushMemBuffer();
    ithUnmapVram(lcdBase, size);
}
