#include <string.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "gfx/gfx.h"
#include "itu_private.h"

static LcdSurface lcdSurface;

#define SMTK_RGB565(r, g, b) \
    (((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11))

static ITUSurface *LcdGetDisplaySurface(void)
{
    return (ITUSurface *)&lcdSurface;
}

void ituLcdInit(void)
{
    ituGetDisplaySurface = LcdGetDisplaySurface;

    if (lcdSurface.surf.addr == 0)
    {
        ITUPixelFormat format;

        memset(&lcdSurface, 0, sizeof(lcdSurface));

        switch (ithLcdGetFormat())
        {
        case ITH_LCD_RGB565: format = ITU_RGB565;   break;
        case ITH_LCD_ARGB1555: format = ITU_ARGB1555; break;
        case ITH_LCD_ARGB4444: format = ITU_ARGB4444; break;
        case ITH_LCD_ARGB8888: format = ITU_ARGB8888; break;
        default: // ITH_LCD_YUV
            format = ITU_MONO;
        }

        // Get width, height, pitch setting from LCD register
        // [ToDo] Could we set the UI size different from the LCD width and
        //        height?
        lcdSurface.surf.width  = ithLcdGetWidth();
        lcdSurface.surf.height = ithLcdGetHeight();
        lcdSurface.surf.pitch  = ithLcdGetPitch();
        lcdSurface.surf.format = format;
        lcdSurface.surf.flags  = ITU_STATIC;

#ifndef CFG_M2D_ENABLE
        lcdSurface.surf.addr   = ithLcdGetBaseAddrA();
#else

  #if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        if (ithLcdGetFlip() == 0)
            lcdSurface.surf.addr = ithLcdGetBaseAddrB();
        else if (ithLcdGetFlip() == 1)
            lcdSurface.surf.addr = ithLcdGetBaseAddrC();
        else if (ithLcdGetFlip() == 2)
            lcdSurface.surf.addr = ithLcdGetBaseAddrA();
  #else
        lcdSurface.surf.addr = (ithLcdGetFlip() == 0) ? ithLcdGetBaseAddrB() : ithLcdGetBaseAddrA();
  #endif

#endif


      
        ituSetColor(&lcdSurface.surf.fgColor, 255, 255, 255, 255);
    }

#ifdef CFG_M2D_ENABLE
	//create m2d surface
    if (!lcdSurface.m2dSurf)
    {
        GFXColorFormat gfxformat = GFX_COLOR_UNKNOWN;

        switch (ithLcdGetFormat())
        {
        case ITH_LCD_RGB565: gfxformat = GFX_COLOR_RGB565;   break;
        case ITH_LCD_ARGB1555: gfxformat = GFX_COLOR_ARGB1555; break;
        case ITH_LCD_ARGB4444: gfxformat = GFX_COLOR_ARGB4444; break;
        case ITH_LCD_ARGB8888: gfxformat = GFX_COLOR_ARGB8888; break;
        default: // ITH_LCD_YUV
            gfxformat = GFX_COLOR_UNKNOWN;
        }

        lcdSurface.m2dSurf = gfxCreateSurfaceByPointer(ithLcdGetWidth(), 
                                                       ithLcdGetHeight(), 
                                                       ithLcdGetPitch(), 
                                                       gfxformat,
                                                       lcdSurface.surf.addr, 
                                                       ithLcdGetHeight()*ithLcdGetPitch());
    }
#endif	
}

void ituLcdExit(void)
{
    // TODO: IMPLEMENT
#ifdef CFG_M2D_ENABLE
    gfxDestroySurface(lcdSurface.m2dSurf);
#endif
}
