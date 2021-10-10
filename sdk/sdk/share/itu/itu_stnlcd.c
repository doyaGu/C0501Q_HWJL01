#include <sys/ioctl.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "itu_cfg.h"
#include "ite/itp.h"
#include "ite/itu.h"

static ITUSurface stnLcdSurface;

ITUSurface* StnLcdGetDisplaySurface(void)
{
    return &stnLcdSurface;
}

static ITUSurface* StnLcdCreateSurface(int w, int h, int pitch, ITUPixelFormat format, const uint8_t* bitmap, unsigned int flags)
{
    ITUSurface* surf = calloc(1, sizeof (ITUSurface));
    if (!surf)
    {
        LOG_ERR "Out of memory: %d\n", sizeof (ITUSurface) LOG_END
        return NULL;
    }

    assert(format == ITU_MONO);

    if (flags & ITU_STATIC)
    {
        surf->addr = (uint32_t) bitmap;
    }
    else
    {
        unsigned int size = pitch * h;

        surf->addr = (uint32_t) malloc(size);
        if (!surf->addr)
        {
            LOG_ERR "Out of memory: %d\n", size LOG_END
            return NULL;
        }

        if (surf->addr)
        {
            memcpy((void*)surf->addr, bitmap, size);
        }
    }

    surf->width    = w;
    surf->height   = h;
    surf->pitch    = pitch;
    surf->format   = ITU_MONO;
    surf->flags    = flags;
    ituSetColor(&surf->fgColor, 255, 255, 255, 255);

    return surf;
}

static void StnLcdDestroySurface(ITUSurface* surf)
{
    if (surf && !(surf->flags & ITU_STATIC))
    {
        itpVmemFree(surf->addr);
    }
    free(surf);
}

static uint8_t* StnLcdLockSurface(ITUSurface* surf, int x, int y, int w, int h)
{
    surf->lockSize   = surf->pitch * h;
    surf->lockAddr   = (uint8_t*)(surf->addr + surf->pitch * y + (x + 7) / 8);
    return surf->lockAddr;
}

static void StnLcdUnlockSurface(ITUSurface* surf)
{
    // DO NOTHING
}

static void StnLcdDrawGlyph(ITUSurface* surf, int x, int y, ITUGlyphFormat format, const uint8_t* bitmap, int w, int h)
{
    unsigned int i, j;

    if (format == ITU_GLYPH_1BPP)
    {
        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                int index = ((w + 7) / 8) * i + j / 8;
                uint8_t c = bitmap[index];
                uint8_t mask = (0x80 >> (j % 8));
                uint8_t v = c & mask;
                itpStnLcdDrawPixel(x + j, y + i, v);
            }
        }
    }
    else
    {
        for (i = 0; i < h; i++)
        {
            for (j = 0; j < w; j++)
            {
                int c = bitmap[w * i + j];
                itpStnLcdDrawPixel(x + j, y + i, c);
            }
        }
    }
}

static void StnLcdBitBlt(ITUSurface* dest, int dx, int dy, int w, int h, ITUSurface* src, int sx, int sy)
{

    assert(dest == &stnLcdSurface);
    assert(dest->format == ITU_MONO);
    assert(src->format == ITU_MONO);
    assert(sx == 0);
    assert(sy == 0);

    itpStnLcdDrawBitmap(dx, dy, (uint8_t*)src->addr, w, h);
}

static void StnLcdColorFill(ITUSurface* surf, int x, int y, int w, int h, ITUColor* color)
{
    assert(surf == &stnLcdSurface);
    assert(surf->format == ITU_MONO);

    itpStnLcdFillRect(x, y, w, h, ITH_RGB565(color->red, color->green, color->blue));
}

static void StnLcdFlip(ITUSurface* surf)
{
    ioctl(ITP_DEVICE_STNLCD, ITP_IOCTL_FLUSH, NULL);
}

void ituStnLcdInit(void)
{
    ITPStnLcdInfo info;

    memset(&stnLcdSurface, 0, sizeof (stnLcdSurface));

    ioctl(ITP_DEVICE_STNLCD, ITP_IOCTL_GET_INFO, &info);

    stnLcdSurface.width     = info.width;
    stnLcdSurface.height    = info.height;
    stnLcdSurface.pitch     = info.pitch;
    stnLcdSurface.format    = ITU_MONO;
    stnLcdSurface.flags     = ITU_STATIC;
    stnLcdSurface.addr      = (uint32_t)info.buf;
    ituSetColor(&stnLcdSurface.fgColor, 255, 255, 255, 255);

    ituGetDisplaySurface    = StnLcdGetDisplaySurface;
    ituCreateSurface        = StnLcdCreateSurface;
    ituDestroySurface       = StnLcdDestroySurface;
    ituLockSurface          = StnLcdLockSurface;
    ituUnlockSurface        = StnLcdUnlockSurface;
    ituDrawGlyph            = StnLcdDrawGlyph;
    ituBitBlt               = StnLcdBitBlt;
    ituColorFill            = StnLcdColorFill;
    ituFlip                 = StnLcdFlip;
}

void ituStnLcdExit(void)
{
    // DO NOTHING
}
