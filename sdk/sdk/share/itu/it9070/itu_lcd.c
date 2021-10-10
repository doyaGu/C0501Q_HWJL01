#include <string.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "ite/ite_m2d.h"
#include "itu_private.h"

typedef struct
{
    ITUSurface      surf;
#ifdef CFG_M2D_ENABLE
    MMP_M2D_SURFACE m2dSurf;
#endif
} LcdSurface;

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
        memset(&lcdSurface, 0, sizeof(lcdSurface));

        // Get width, height, pitch setting from LCD register
        // [ToDo] Could we set the UI size different from the LCD width and
        //        height?
        lcdSurface.surf.width  = ithLcdGetWidth();
        lcdSurface.surf.height = ithLcdGetHeight();
        lcdSurface.surf.pitch  = ithLcdGetPitch();
        lcdSurface.surf.format = ITU_RGB565;
        lcdSurface.surf.flags  = ITU_STATIC;
        lcdSurface.surf.addr   =
#ifdef CFG_M2D_ENABLE
            (ithLcdGetFlip() == 0) ? ithLcdGetBaseAddrB() :
#endif
            ithLcdGetBaseAddrA();
        ituSetColor(&lcdSurface.surf.fgColor, 255, 255, 255, 255);
    }

#ifdef CFG_M2D_ENABLE
    //create m2d surface
    if (!lcdSurface.m2dSurf)
    {
        mmpM2dCreateVirtualSurface(lcdSurface.surf.width,
                                   lcdSurface.surf.height,
                                   MMP_M2D_IMAGE_FORMAT_RGB565,
                                   lcdSurface.surf.addr,
                                   &lcdSurface.m2dSurf);

        mmpM2dSetFontColor(lcdSurface.m2dSurf, SMTK_RGB565(0, 0, 0));
    }
#endif
}

void ituLcdExit(void)
{
#ifdef CFG_M2D_ENABLE
    mmpM2dDeleteVirtualSurface(lcdSurface.m2dSurf);
#endif
}
