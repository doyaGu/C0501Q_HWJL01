#include "ite/ith.h"

#define REMAP_ADDR 0x80000000

// _start is default function name of entry point.
void _start(void)
{
    uint32_t* ptr;
    uint32_t size;
    uint32_t color, i;

    asm volatile("mcr p15, 0, %0, c7, c14, 0" : : "r"(0));  // clean and invalidate D-Cache all
    asm volatile("mcr p15, 0, %0, c7, c5, 0" : : "r"(0));   // invalidate I-Cache all

    ptr = (uint32_t*)(ithLcdGetBaseAddrA() + REMAP_ADDR);
    size = ithLcdGetPitch() * ithLcdGetHeight();
    
#if CFG_LCD_BPP == 2
    color = ITH_RGB565((CFG_LCD_BOOT_BGCOLOR >> 16) & 0xFF, (CFG_LCD_BOOT_BGCOLOR >> 8) & 0xFF, CFG_LCD_BOOT_BGCOLOR & 0xFF);
    color |= color << 16;
#elif CFG_LCD_BPP == 4
    color = CFG_LCD_BOOT_BGCOLOR;
#elif CFG_LCD_BPP == 0
    #error "0 LCD BPP"
#else
    #error "Unknown LCD BPP"
#endif

    for (i = 0; i < size / (sizeof(uint32_t)*8); i++)
    {
       *ptr++ = color;
       *ptr++ = color;
       *ptr++ = color;
       *ptr++ = color;
       *ptr++ = color;
       *ptr++ = color;
       *ptr++ = color;
       *ptr++ = color;

       // FIXME: workaround for IT9850
       #if (CFG_CHIP_FAMILY == 9850)
       {
           asm volatile("mcr p15, 0, %0, c7, c10, 4" : : "r"(0));  // sync (drain write buffer)
       }
       #endif
    }
}
