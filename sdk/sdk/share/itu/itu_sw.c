#include <assert.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include "itu_cfg.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "itu_private.h"

#ifdef CFG_WIN32_SIMULATOR
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

static HWND    win;
static HBITMAP bitmap;
static BYTE    *bits;
#else
    #define SPEED_UP
#endif // CFG_WIN32_SIMULATOR

static ITURotation swRotation;
static uint32_t    swLcdAddr;

#define OUT_OF_RANGE(p, start, length) (((p) < (start)) || ((p) >= ((start) + (length))))

const static unsigned char DITHER_TABLE[16] = {
    1, 13,  2, 14,
    9,  5, 10,  6,
    3, 15,  0, 12,
   11,  7,  8,  4
};

#define RGB565_DITHER(a,r,g,b,x,y) \
    if (a != 0) { \
        int d = ((y & 0x3) << 2) | (x & 0x3); \
        if (((r & 7) > (uint32_t)(DITHER_TABLE[d] >> 1)) && ((r&0xf8)!=0xf8)) r += 0x08; \
        if (((g & 3) > (uint32_t)(DITHER_TABLE[d] >> 2)) && ((g&0xfc)!=0xfc)) g += 0x04; \
        if (((b & 7) > (uint32_t)(DITHER_TABLE[d] >> 1)) && ((b&0xf8)!=0xf8)) b += 0x08; \
    }

static ITUSurface *SWCreateSurface(int w, int h, int pitch, ITUPixelFormat format, const uint8_t *bitmap, unsigned int flags)
{
    ITUSurface *surf = calloc(1, sizeof(ITUSurface));
    if (!surf)
    {
        LOG_ERR "Out of memory: %d\n", sizeof(ITUSurface)LOG_END
        return NULL;
    }

    surf->width  = w;
    surf->height = h;

    if (pitch)
    {
        surf->pitch = pitch;
    }
    else
    {
        surf->pitch = w * ituFormat2Bpp(format) / 8;
    }

    surf->format = format;
    surf->flags  = flags;
    ituSetColor(&surf->fgColor, 255, 255, 255, 255);

#ifndef _WIN32
    if (flags & ITU_STATIC)
    {
        surf->addr = (uint32_t) bitmap;
    }
    else
#endif // !_WIN32
    {
        uint8_t      *ptr;
        unsigned int size;

        if (format == ITU_RGB565A8)
            size = surf->pitch * h + w * h;
        else
            size = surf->pitch * h;

        surf->addr = itpVmemAlloc(size);
        if (!surf->addr)
        {
            LOG_ERR "Out of VRAM: %d\n", size LOG_END
            return NULL;
        }

        if (bitmap)
        {
            ptr = ithMapVram(surf->addr, size, ITH_VRAM_WRITE);
            memcpy(ptr, bitmap, size);
#ifndef SPEED_UP
            ithUnmapVram(ptr, size);
#endif
            if (!(flags & ITU_STATIC))
                free((uint8_t*)bitmap);
        }
    }

    return surf;
}

static void SWDestroySurface(ITUSurface *surf)
{
    if (surf)
    {
#ifndef _WIN32
        if (!(surf->flags & ITU_STATIC))
#endif
        itpVmemFree(surf->addr);
        free(surf);
    }
}

static uint8_t *SWLockSurface(ITUSurface *surf, int x, int y, int w, int h)
{
    uint32_t vram_addr;

    if (surf->format == ITU_RGB565A8)
        surf->lockSize = surf->pitch * (surf->height - y) + surf->width * h;
    else
        surf->lockSize = surf->pitch * h;

    if (surf->format == ITU_ARGB8888)
    {
        vram_addr = surf->addr + surf->pitch * y + x * 4;
    }
    else if (surf->format == ITU_A8)
    {
        vram_addr = surf->addr + surf->pitch * y + x;
    }
    else if (surf->format == ITU_MONO)
    {
        vram_addr = surf->addr + surf->pitch * y + (x + 7) / 8;
    }
    else
    {
        vram_addr = surf->addr + surf->pitch * y + x * 2;
    }
#ifndef SPEED_UP
    surf->lockAddr = ithMapVram(vram_addr, surf->lockSize, ITH_VRAM_READ | ITH_VRAM_WRITE);
#else
    surf->lockAddr = ithMapVram(vram_addr, surf->lockSize, /*ITH_VRAM_READ |*/ ITH_VRAM_WRITE);
#endif
    return surf->lockAddr;
}

static void SWUnlockSurface(ITUSurface *surf)
{
#ifndef SPEED_UP
    ithUnmapVram(surf->lockAddr, surf->lockSize);
#endif
}

static void SWSetRotation(ITURotation rot)
{
    ITUSurface *lcdSurf;

    if (swRotation == rot)
        return;

    lcdSurf = ituGetDisplaySurface();
    assert(lcdSurf);

    if (swRotation != ITU_ROT_0)
        itpVmemFree(lcdSurf->addr);

    if (rot == ITU_ROT_0)
    {
        lcdSurf->width  = ithLcdGetWidth();
        lcdSurf->height = ithLcdGetHeight();
        lcdSurf->pitch  = ithLcdGetPitch();
        lcdSurf->addr   = swLcdAddr;
    }
    else
    {
        if (rot == ITU_ROT_180)
        {
            lcdSurf->width  = ithLcdGetWidth();
            lcdSurf->height = ithLcdGetHeight();
            lcdSurf->pitch  = ithLcdGetPitch();
            lcdSurf->addr   = swLcdAddr;
        }
        else
        {
            lcdSurf->width  = ithLcdGetHeight();
            lcdSurf->height = ithLcdGetWidth();
            lcdSurf->pitch  = ithLcdGetPitch();
            lcdSurf->addr   = swLcdAddr;
        }
        lcdSurf->addr = itpVmemAlloc(ithLcdGetPitch() * ithLcdGetHeight());
    }
    swRotation = rot;
}

static void SWDrawGlyph(ITUSurface *surf, int x, int y, ITUGlyphFormat format, const uint8_t *bitmap, int w, int h)
{
    int i, j;

    if (surf->format == ITU_RGB565)
    {
        uint16_t* base = (uint16_t *)ituLockSurface(surf, x, y, w, h);

        if (format == ITU_GLYPH_1BPP)
        {
            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    int     index;
                    uint8_t c, mask, v;

                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    index = ((w + 7) / 8) * i + j / 8;
                    c = bitmap[index];
                    mask = (0x80 >> (j % 8));
                    v = c & mask;
                    if (v)
                        base[surf->width * i + j] = ITH_RGB565(surf->fgColor.red, surf->fgColor.green, surf->fgColor.blue);
                }
            }
        }
        else
        {
            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < (int)w; j++)
                {
                    int c, bc, r, g, b;

                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    c = bitmap[w * i + j];
                    bc = base[surf->width * i + j];
                    r = ((((bc & 0xf800) >> 11) << 3) * (255 - c) + surf->fgColor.red * c) / 255;
                    g = ((((bc & 0x07e0) >> 5) << 2) * (255 - c) + surf->fgColor.green * c) / 255;
                    b = (((bc & 0x001f) << 3) * (255 - c) + surf->fgColor.blue * c) / 255;
                    base[surf->width * i + j] = ITH_RGB565(r, g, b);
                }
            }
        }
        ituUnlockSurface(surf);
    }
    else if (surf->format == ITU_ARGB8888)
    {
        uint32_t* base = (uint32_t *)ituLockSurface(surf, x, y, w, h);

        for (i = 0; i < h; i++)
        {
            if (surf->flags & ITU_CLIPPING)
            {
                if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                    continue;
            }

            for (j = 0; j < (int)w; j++)
            {
                int c, bc, a, r, g, b;

                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                        continue;
                }
                c = bitmap[w * i + j];
                bc = base[surf->width * i + j];
                a = (((bc & 0xFF000000) >> 24) * (255 - c) + surf->fgColor.alpha * c) / 255;
                r = (((bc & 0x00FF0000) >> 16) * (255 - c) + surf->fgColor.red * c) / 255;
                g = (((bc & 0x0000FF00) >> 8) * (255 - c) + surf->fgColor.green * c) / 255;
                b = ((bc & 0x000000FF) * (255 - c) + surf->fgColor.blue * c) / 255;
                base[surf->width * i + j] = ITH_ARGB8888(a, r, g, b);
            }
        }
        ituUnlockSurface(surf);
    }
}

static void SWBitBlt(ITUSurface *dest, int dx, int dy, int w, int h, ITUSurface *src, int sx, int sy)
{
    int dw = dest->width - dx;
    int dh = dest->height - dy;
    int sw = src->width - sx;
    int sh = src->height - sy;
    int x, y;

    if (w > dw)
        w = dw;
    if (w > sw)
        w = sw;
    if (h > dh)
        h = dh;
    if (h > sh)
        h = sh;

    if (dest->format == ITU_RGB565)
    {
        uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);

        if (src->format == ITU_RGB565)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    destPtr[dest->width * y + x] = srcPtr[src->width * y + x];
                }
            }
        }
        else if (src->format == ITU_ARGB1555)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = (sc & 0x8000) ? 255 : 0;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x7C00) >> 10) << 3) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x03E0) >> 5) << 3) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001F) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }
        else if (src->format == ITU_ARGB4444)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = (sc & 0xF000) >> 12;
                    a                           |= a << 4;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x0F00) >> 8) << 4) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x00F0) >> 4) << 4) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x000F) << 4) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }
        else if (src->format == ITU_ARGB8888)
        {
            uint32_t *srcPtr = (uint32_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = sc >> 24;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + ((sc & 0x00FF0000) >> 16) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + ((sc & 0x0000FF00) >> 8) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + (sc & 0x000000FF) * a) / 255;
                    RGB565_DITHER(a, r, g, b, x, y);
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }
        else if (src->format == ITU_MONO)
        {
            uint8_t *srcPtr = ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < w; x++)
                {
                    uint8_t value, mask;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    value  = srcPtr[(src->width + 7) / 8 * y + x / 8];
                    mask   = (0x80 >> (x % 8));
                    value &= mask;
                    if (value)
                        destPtr[dest->width * y + x] = ITH_RGB565(src->fgColor.red, src->fgColor.green, src->fgColor.blue);
                }
            }
        }
        else if (src->format == ITU_RGB565A8)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = *((uint8_t*)srcPtr + src->pitch * src->height + src->width * y + x);
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0xf800) >> 11) << 3) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x07e0) >> 5) << 2) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001F) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }

        ituUnlockSurface(src);
        ituUnlockSurface(dest);
    }
    else if (dest->format == ITU_ARGB8888)
    {
        if (src->format == ITU_ARGB8888)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint32_t *srcPtr = (uint32_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc = srcPtr[src->width * y + x];
                    dc = destPtr[dest->width * y + x];
                    a = sc >> 24;
                    r = (((dc & 0x00FF0000) >> 16) * (255 - a) + ((sc & 0x00FF0000) >> 16) * a) / 255;
                    g = (((dc & 0x0000FF00) >> 8) * (255 - a) + ((sc & 0x0000FF00) >> 8) * a) / 255;
                    b = ((dc & 0x000000FF) * (255 - a) + (sc & 0x000000FF) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_RGB565A8)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc = srcPtr[src->width * y + x];
                    dc = destPtr[dest->width * y + x];
                    a = *((uint8_t*)srcPtr + src->pitch * src->height + src->width * y + x);
                    r = (((dc & 0x00FF0000) >> 16) * (255 - a) + (((sc & 0xf800) >> 11) << 3) * a) / 255;
                    g = (((dc & 0x0000FF00) >> 8) * (255 - a) + (((sc & 0x07e0) >> 5) << 2) * a) / 255;
                    b = ((dc & 0x000000FF) * (255 - a) + ((sc & 0x001f) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_RGB565)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    int sc, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc = srcPtr[src->width * y + x];
                    r = ((sc & 0xf800) >> 11) << 3;
                    g = ((sc & 0x07e0) >> 5) << 2;
                    b = (sc & 0x001f) << 3;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(255, r, g, b);
                }
            }
        }
    }
}

static void SWStretchBlt(ITUSurface *dest, int dx, int dy, int dw, int dh, ITUSurface *src, int sx, int sy, int sw, int sh)
{
    int w = dw;
    int h = dh;
    int x, y;

    if (dest->format == ITU_RGB565)
    {
        uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);

        if (src->format == ITU_RGB565)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    int xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx                           = x * sw / dw;
                    yy                           = y * sh / dh;
                    destPtr[dest->width * y + x] = srcPtr[src->width * yy + xx];
                }
            }
        }
        else if (src->format == ITU_ARGB1555)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;
                    int      xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx                           = x * sw / dw;
                    yy                           = y * sh / dh;
                    sc                           = srcPtr[src->width * yy + xx];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = (sc & 0x8000) ? 255 : 0;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x7C00) >> 10) << 3) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x03E0) >> 5) << 3) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001F) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }
        else if (src->format == ITU_ARGB4444)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;
                    int      xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx                           = x * sw / dw;
                    yy                           = y * sh / dh;
                    sc                           = srcPtr[src->width * yy + xx];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = (sc & 0xF000) >> 12;
                    a                           |= a << 4;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x0F00) >> 8) << 4) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x00F0) >> 4) << 4) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x000F) << 4) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }
        else if (src->format == ITU_ARGB8888)
        {
            uint32_t *srcPtr = (uint32_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;
                    int      xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx                           = x * sw / dw;
                    yy                           = y * sh / dh;
                    sc                           = srcPtr[src->width * yy + xx];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = sc >> 24;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + ((sc & 0x00FF0000) >> 16) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + ((sc & 0x0000FF00) >> 8) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + (sc & 0x000000FF) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }
        else if (src->format == ITU_MONO)
        {
            uint8_t *srcPtr = ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < w; x++)
                {
                    uint8_t value, mask;
                    int     xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx     = x * sw / dw;
                    yy     = y * sh / dh;
                    value  = srcPtr[(src->width + 7) / 8 * yy + xx / 8];
                    mask   = (0x80 >> (x % 8));
                    value &= mask;
                    if (value)
                        destPtr[dest->width * y + x] = ITH_RGB565(src->fgColor.red, src->fgColor.green, src->fgColor.blue);
                }
            }
        }
        else if (src->format == ITU_RGB565A8)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;
                    int xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx                           = x * sw / dw;
                    yy                           = y * sh / dh;
                    sc                           = srcPtr[src->width * yy + xx];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = *((uint8_t*)srcPtr + src->pitch * src->height + src->width * yy + xx);
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0xf800) >> 11) << 3) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x07e0) >> 5) << 2) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001F) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
        }
        ituUnlockSurface(src);
        ituUnlockSurface(dest);
    }
    else if (dest->format == ITU_ARGB8888)
    {
        if (src->format == ITU_ARGB8888)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint32_t *srcPtr = (uint32_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;
                    int      xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx = x * sw / dw;
                    yy = y * sh / dh;
                    sc = srcPtr[src->width * yy + xx];
                    dc = destPtr[dest->width * y + x];
                    a = sc >> 24;
                    r = (((dc & 0x00FF0000) >> 16) * (255 - a) + ((sc & 0x00FF0000) >> 16) * a) / 255;
                    g = (((dc & 0x0000FF00) >> 8) * (255 - a) + ((sc & 0x0000FF00) >> 8) * a) / 255;
                    b = ((dc & 0x000000FF) * (255 - a) + (sc & 0x000000FF) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_RGB565A8)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;
                    int xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx = x * sw / dw;
                    yy = y * sh / dh;
                    sc = srcPtr[src->width * yy + xx];
                    dc = destPtr[dest->width * y + x];
                    a = *((uint8_t*)srcPtr + src->pitch * src->height + src->width * y + x);
                    r = (((dc & 0x00FF0000) >> 16) * (255 - a) + (((sc & 0xf800) >> 11) << 3) * a) / 255;
                    g = (((dc & 0x0000FF00) >> 8) * (255 - a) + (((sc & 0x07e0) >> 5) << 2) * a) / 255;
                    b = ((dc & 0x000000FF) * (255 - a) + ((sc & 0x001f) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_RGB565)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    int sc, r, g, b;
                    int xx, yy;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    xx = x * sw / dw;
                    yy = y * sh / dh;
                    sc = srcPtr[src->width * yy + xx];
                    r = ((sc & 0xf800) >> 11) << 3;
                    g = ((sc & 0x07e0) >> 5) << 2;
                    b = (sc & 0x001f) << 3;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(255, r, g, b);
                }
            }
        }
    }
}

static void SWAlphaBlend(ITUSurface *dest, int dx, int dy, int w, int h, ITUSurface *src, int sx, int sy, uint8_t alpha)
{
    int x, y;
    int dw = dest->width - dx;
    int dh = dest->height - dy;
    int sw = src->width - sx;
    int sh = src->height - sy;

    if (w > dw)
        w = dw;
    if (w > sw)
        w = sw;
    if (h > dh)
        h = dh;
    if (h > sh)
        h = sh;

    if (dest->format == ITU_RGB565)
    {
        if (src->format == ITU_RGB565)
        {
            uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr  = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - alpha) + (((sc & 0xf800) >> 11) << 3) * alpha) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - alpha) + (((sc & 0x07e0) >> 5) << 2) * alpha) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - alpha) + ((sc & 0x001f) << 3) * alpha) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_ARGB1555)
        {
            uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr  = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, r, g, b, a;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = (sc & 0x8000) ? 255 : 0;
                    a                            = a * alpha / 255;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x7C00) >> 10) << 3) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x03E0) >> 5) << 3) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001F) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_ARGB4444)
        {
            uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr  = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, r, g, b, a;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = (sc & 0xF000) >> 12;
                    a                           |= a << 4;
                    a                            = a * alpha / 255;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x0F00) >> 8) << 4) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x00F0) >> 4) << 4) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x000F) << 4) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_ARGB8888)
        {
            uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);
            uint32_t *srcPtr  = (uint32_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, r, g, b, a;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = sc >> 24;
                    a                            = a * alpha / 255;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + ((sc & 0x00FF0000) >> 16) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + ((sc & 0x0000FF00) >> 8) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + (sc & 0x000000FF) * a) / 255;
                    RGB565_DITHER(a, r, g, b, x, y);
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_RGB565A8)
        {
            uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr  = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = *((uint8_t*)srcPtr + src->pitch * src->height + src->width * y + x);
                    a                            = a * alpha / 255;
                    r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0xf800) >> 11) << 3) * a) / 255;
                    g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x07e0) >> 5) << 2) * a) / 255;
                    b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001f) << 3) * a) / 255;
                    destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
    }
    else if (dest->format == ITU_ARGB1555)
    {
        if (src->format == ITU_ARGB1555)
        {
            uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr  = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = ((((dc & 0x8000) >> 15) << 7) * (255 - alpha) + (((sc & 0x8000) >> 15) << 7) * alpha) / 255;
                    r                            = ((((dc & 0x7C00) >> 10) << 3) * (255 - alpha) + (((sc & 0x7C00) >> 10) << 3) * alpha) / 255;
                    g                            = ((((dc & 0x03E0) >> 5) << 3) * (255 - alpha) + (((sc & 0x03E0) >> 5) << 3) * alpha) / 255;
                    b                            = (((dc & 0x001F) << 3) * (255 - alpha) + ((sc & 0x001F) << 3) * alpha) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB1555(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
    }
    else if (dest->format == ITU_ARGB4444)
    {
        if (src->format == ITU_ARGB4444)
        {
            uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr  = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = ((((dc & 0xF000) >> 12) << 4) * (255 - alpha) + (((sc & 0xF000) >> 12) << 4) * alpha) / 255;
                    r                            = ((((dc & 0x0F00) >> 8) << 4) * (255 - alpha) + (((sc & 0x0F00) >> 8) << 4) * alpha) / 255;
                    g                            = ((((dc & 0x00F0) >> 4) << 4) * (255 - alpha) + (((sc & 0x00F0) >> 4) << 4) * alpha) / 255;
                    b                            = (((dc & 0x000F) << 4) * (255 - alpha) + ((sc & 0x000F) << 4) * alpha) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB4444(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
    }
    else if (dest->format == ITU_ARGB8888)
    {
        if (src->format == ITU_ARGB8888)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint32_t *srcPtr  = (uint32_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = (((dc & 0xFF000000) >> 24) * (255 - alpha) + ((sc & 0xFF000000) >> 24) * alpha) / 255;
                    r                            = (((dc & 0x00FF0000) >> 16) * (255 - alpha) + ((sc & 0x00FF0000) >> 16) * alpha) / 255;
                    g                            = (((dc & 0x0000FF00) >> 8) * (255 - alpha) + ((sc & 0x0000FF00) >> 8) * alpha) / 255;
                    b                            = ((dc & 0x000000FF) * (255 - alpha) + (sc & 0x000000FF) * alpha) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
        else if (src->format == ITU_RGB565A8)
        {
            uint32_t *destPtr = (uint32_t *)ituLockSurface(dest, dx, dy, w, h);
            uint16_t *srcPtr  = (uint16_t *)ituLockSurface(src, sx, sy, w, h);
            for (y = 0; y < h; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < w; x++)
                {
                    uint32_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }
                    sc                           = srcPtr[src->width * y + x];
                    dc                           = destPtr[dest->width * y + x];
                    a                            = *((uint8_t*)srcPtr + src->pitch * src->height + src->width * y + x);
                    a                            = (((dc & 0xFF000000) >> 24) * (255 - alpha) + a * alpha) / 255;
                    r                            = (((dc & 0x00FF0000) >> 16) * (255 - alpha) + (((sc & 0xf800) >> 11) << 3) * alpha) / 255;
                    g                            = (((dc & 0x0000FF00) >> 8) * (255 - alpha) + (((sc & 0x07e0) >> 5) << 2) * alpha) / 255;
                    b                            = ((dc & 0x000000FF) * (255 - alpha) + ((sc & 0x001f) << 3) * alpha) / 255;
                    destPtr[dest->width * y + x] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(src);
            ituUnlockSurface(dest);
        }
    }
}

static void SWColorFill(ITUSurface *surf, int x, int y, int w, int h, ITUColor *color)
{
    if (surf->format == ITU_RGB565)
    {
        int      i, j;
        uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

        for (i = 0; i < h; i++)
        {
            if (surf->flags & ITU_CLIPPING)
            {
                if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                    continue;
            }

            for (j = 0; j < w; j++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                        continue;
                }
                ptr[surf->width * i + j] = ITH_RGB565(color->red, color->green, color->blue);
            }
        }
        ituUnlockSurface(surf);
    }
    else if (surf->format == ITU_ARGB1555)
    {
        int      i, j;
        uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

        for (i = 0; i < h; i++)
        {
            if (surf->flags & ITU_CLIPPING)
            {
                if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                    continue;
            }

            for (j = 0; j < w; j++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                        continue;
                }
                ptr[surf->width * i + j] = ITH_ARGB1555(color->alpha, color->red, color->green, color->blue);
            }
        }
        ituUnlockSurface(surf);
    }
    else if (surf->format == ITU_ARGB4444)
    {
        int      i, j;
        uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

        for (i = 0; i < h; i++)
        {
            if (surf->flags & ITU_CLIPPING)
            {
                if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                    continue;
            }

            for (j = 0; j < w; j++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                        continue;
                }
                ptr[surf->width * i + j] = ITH_ARGB4444(color->alpha, color->red, color->green, color->blue);
            }
        }
        ituUnlockSurface(surf);
    }
    else if (surf->format == ITU_ARGB8888)
    {
        int      i, j;
        uint32_t *ptr = (uint32_t *)ituLockSurface(surf, x, y, w, h);

        for (i = 0; i < h; i++)
        {
            if (surf->flags & ITU_CLIPPING)
            {
                if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                    continue;
            }

            for (j = 0; j < w; j++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                        continue;
                }
                ptr[surf->width * i + j] = ITH_ARGB8888(color->alpha, color->red, color->green, color->blue);
            }
        }
        ituUnlockSurface(surf);
    }
}

static void SWGradientFill(ITUSurface *surf, int x, int y, int w, int h, ITUColor *startColor, ITUColor *endColor, ITUGradientMode mode)
{
    if (mode == ITU_GF_NONE)
    {
        SWColorFill(surf, x, y, w, h, startColor);
    }
    else if (mode == ITU_GF_HORIZONTAL)
    {
        if (surf->format == ITU_RGB565)
        {
            int      i, j, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    r                        = (endColor->red * j / w) + (startColor->red - startColor->red * j / w);
                    g                        = (endColor->green * j / w) + (startColor->green - startColor->green * j / w);
                    b                        = (endColor->blue * j / w) + (startColor->blue - startColor->blue * j / w);
                    ptr[surf->width * i + j] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB1555)
        {
            int      i, j, a, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    a                        = (endColor->alpha * j / w) + (startColor->alpha - startColor->alpha * j / w);
                    r                        = (endColor->red * j / w) + (startColor->red - startColor->red * j / w);
                    g                        = (endColor->green * j / w) + (startColor->green - startColor->green * j / w);
                    b                        = (endColor->blue * j / w) + (startColor->blue - startColor->blue * j / w);
                    ptr[surf->width * i + j] = ITH_ARGB1555(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB4444)
        {
            int      i, j, a, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    a                        = (endColor->alpha * j / w) + (startColor->alpha - startColor->alpha * j / w);
                    r                        = (endColor->red * j / w) + (startColor->red - startColor->red * j / w);
                    g                        = (endColor->green * j / w) + (startColor->green - startColor->green * j / w);
                    b                        = (endColor->blue * j / w) + (startColor->blue - startColor->blue * j / w);
                    ptr[surf->width * i + j] = ITH_ARGB4444(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB8888)
        {
            int      i, j, a, r, g, b;
            uint32_t *ptr = (uint32_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    a                        = (endColor->alpha * j / w) + (startColor->alpha - startColor->alpha * j / w);
                    r                        = (endColor->red * j / w) + (startColor->red - startColor->red * j / w);
                    g                        = (endColor->green * j / w) + (startColor->green - startColor->green * j / w);
                    b                        = (endColor->blue * j / w) + (startColor->blue - startColor->blue * j / w);
                    ptr[surf->width * i + j] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
    }
    else if (mode == ITU_GF_VERTICAL)
    {
        if (surf->format == ITU_RGB565)
        {
            int      i, j, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                r = (endColor->red * i / h) + (startColor->red - startColor->red * i / h);
                g = (endColor->green * i / h) + (startColor->green - startColor->green * i / h);
                b = (endColor->blue * i / h) + (startColor->blue - startColor->blue * i / h);

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    ptr[surf->width * i + j] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB1555)
        {
            int      i, j, a, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                a = (endColor->alpha * i / h) + (startColor->alpha - startColor->alpha * i / h);
                r = (endColor->red * i / h) + (startColor->red - startColor->red * i / h);
                g = (endColor->green * i / h) + (startColor->green - startColor->green * i / h);
                b = (endColor->blue * i / h) + (startColor->blue - startColor->blue * i / h);

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    ptr[surf->width * i + j] = ITH_ARGB1555(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB4444)
        {
            int      i, j, a, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                a = (endColor->alpha * i / h) + (startColor->alpha - startColor->alpha * i / h);
                r = (endColor->red * i / h) + (startColor->red - startColor->red * i / h);
                g = (endColor->green * i / h) + (startColor->green - startColor->green * i / h);
                b = (endColor->blue * i / h) + (startColor->blue - startColor->blue * i / h);

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    ptr[surf->width * i + j] = ITH_ARGB4444(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB8888)
        {
            int      i, j, a, r, g, b;
            uint32_t *ptr = (uint32_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                a = (endColor->alpha * i / h) + (startColor->alpha - startColor->alpha * i / h);
                r = (endColor->red * i / h) + (startColor->red - startColor->red * i / h);
                g = (endColor->green * i / h) + (startColor->green - startColor->green * i / h);
                b = (endColor->blue * i / h) + (startColor->blue - startColor->blue * i / h);

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    ptr[surf->width * i + j] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_A8)
        {
            int      i, j, a;
            uint8_t *ptr = ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                a = (endColor->alpha * i / h) + (startColor->alpha - startColor->alpha * i / h);

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    ptr[surf->width * i + j] = a;
                }
            }
            ituUnlockSurface(surf);
        }
    }
    else if (mode == ITU_GF_FORWARD_DIAGONAL)
    {
        if (surf->format == ITU_RGB565)
        {
            int      i, j, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    r                        = (endColor->red * (i + j) / (w + h)) + (startColor->red - startColor->red * (i + j) / (w + h));
                    g                        = (endColor->green * (i + j) / (w + h)) + (startColor->green - startColor->green * (i + j) / (w + h));
                    b                        = (endColor->blue * (i + j) / (w + h)) + (startColor->blue - startColor->blue * (i + j) / (w + h));
                    ptr[surf->width * i + j] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB1555)
        {
            int      i, j, a, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    a                        = (endColor->alpha * (i + j) / (w + h)) + (startColor->alpha - startColor->alpha * (i + j) / (w + h));
                    r                        = (endColor->red * (i + j) / (w + h)) + (startColor->red - startColor->red * (i + j) / (w + h));
                    g                        = (endColor->green * (i + j) / (w + h)) + (startColor->green - startColor->green * (i + j) / (w + h));
                    b                        = (endColor->blue * (i + j) / (w + h)) + (startColor->blue - startColor->blue * (i + j) / (w + h));
                    ptr[surf->width * i + j] = ITH_ARGB1555(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB4444)
        {
            int      i, j, a, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    a                        = (endColor->alpha * (i + j) / (w + h)) + (startColor->alpha - startColor->alpha * (i + j) / (w + h));
                    r                        = (endColor->red * (i + j) / (w + h)) + (startColor->red - startColor->red * (i + j) / (w + h));
                    g                        = (endColor->green * (i + j) / (w + h)) + (startColor->green - startColor->green * (i + j) / (w + h));
                    b                        = (endColor->blue * (i + j) / (w + h)) + (startColor->blue - startColor->blue * (i + j) / (w + h));
                    ptr[surf->width * i + j] = ITH_ARGB4444(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB8888)
        {
            int      i, j, a, r, g, b;
            uint32_t *ptr = (uint32_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    a                        = (endColor->alpha * (i + j) / (w + h)) + (startColor->alpha - startColor->alpha * (i + j) / (w + h));
                    r                        = (endColor->red * (i + j) / (w + h)) + (startColor->red - startColor->red * (i + j) / (w + h));
                    g                        = (endColor->green * (i + j) / (w + h)) + (startColor->green - startColor->green * (i + j) / (w + h));
                    b                        = (endColor->blue * (i + j) / (w + h)) + (startColor->blue - startColor->blue * (i + j) / (w + h));
                    ptr[surf->width * i + j] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
    }
    else if (mode == ITU_GF_BACKWARD_DIAGONAL)
    {
        int   i, j, a, r, g, b, yy, x1, y1;
        float k, k1, n, d, n1, radius;

        k      = (float)h / w;
        k1     = -1.0f / k;
        n      = 0.0f;
        radius = sqrtf(pow(h / 2, 2.0f) + pow(w / 2, 2.0f));

        if (surf->format == ITU_RGB565)
        {
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    yy = (int)(k * (j - w / 2) + n);
                    n1 = (int)((i - h / 2) - k1 * (j - w / 2));
                    x1 = (int)((n1 - n) / (k - k1));
                    y1 = (int)(k1 * x1 + n1);
                    d  = sqrtf(powf((j - w / 2) - x1, 2.0f) + powf((i - h / 2) - y1, 2.0f)) / (2.0f * radius);
                    if ((i - h / 2) <= yy)
                    {
                        r = (int)((0.5f - d) * endColor->red + (0.5f + d) * startColor->red);
                        g = (int)((0.5f - d) * endColor->green + (0.5f + d) * startColor->green);
                        b = (int)((0.5f - d) * endColor->blue + (0.5f + d) * startColor->blue);
                    }
                    else
                    {
                        r = (int)((0.5f - d) * startColor->red + (0.5 + d) * endColor->red);
                        g = (int)((0.5f - d) * startColor->green + (0.5 + d) * endColor->green);
                        b = (int)((0.5f - d) * startColor->blue + (0.5 + d) * endColor->blue);
                    }
                    ptr[surf->width * i + j] = ITH_RGB565(r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB1555)
        {
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    yy = (int)(k * (j - w / 2) + n);
                    n1 = (int)((i - h / 2) - k1 * (j - w / 2));
                    x1 = (int)((n1 - n) / (k - k1));
                    y1 = (int)(k1 * x1 + n1);
                    d  = sqrtf(powf((j - w / 2) - x1, 2.0f) + powf((i - h / 2) - y1, 2.0f)) / (2.0f * radius);
                    if ((i - h / 2) <= yy)
                    {
                        a = (int)((0.5f - d) * endColor->alpha + (0.5f + d) * startColor->alpha);
                        r = (int)((0.5f - d) * endColor->red + (0.5f + d) * startColor->red);
                        g = (int)((0.5f - d) * endColor->green + (0.5f + d) * startColor->green);
                        b = (int)((0.5f - d) * endColor->blue + (0.5f + d) * startColor->blue);
                    }
                    else
                    {
                        a = (int)((0.5f - d) * startColor->alpha + (0.5 + d) * endColor->alpha);
                        r = (int)((0.5f - d) * startColor->red + (0.5 + d) * endColor->red);
                        g = (int)((0.5f - d) * startColor->green + (0.5 + d) * endColor->green);
                        b = (int)((0.5f - d) * startColor->blue + (0.5 + d) * endColor->blue);
                    }
                    ptr[surf->width * i + j] = ITH_ARGB1555(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB4444)
        {
            int      i, j, a, r, g, b;
            uint16_t *ptr = (uint16_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    yy = (int)(k * (j - w / 2) + n);
                    n1 = (int)((i - h / 2) - k1 * (j - w / 2));
                    x1 = (int)((n1 - n) / (k - k1));
                    y1 = (int)(k1 * x1 + n1);
                    d  = sqrtf(powf((j - w / 2) - x1, 2.0f) + powf((i - h / 2) - y1, 2.0f)) / (2.0f * radius);
                    if ((i - h / 2) <= yy)
                    {
                        a = (int)((0.5f - d) * endColor->alpha + (0.5f + d) * startColor->alpha);
                        r = (int)((0.5f - d) * endColor->red + (0.5f + d) * startColor->red);
                        g = (int)((0.5f - d) * endColor->green + (0.5f + d) * startColor->green);
                        b = (int)((0.5f - d) * endColor->blue + (0.5f + d) * startColor->blue);
                    }
                    else
                    {
                        a = (int)((0.5f - d) * startColor->alpha + (0.5 + d) * endColor->alpha);
                        r = (int)((0.5f - d) * startColor->red + (0.5 + d) * endColor->red);
                        g = (int)((0.5f - d) * startColor->green + (0.5 + d) * endColor->green);
                        b = (int)((0.5f - d) * startColor->blue + (0.5 + d) * endColor->blue);
                    }
                    ptr[surf->width * i + j] = ITH_ARGB4444(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
        else if (surf->format == ITU_ARGB8888)
        {
            int      i, j, a, r, g, b;
            uint32_t *ptr = (uint32_t *)ituLockSurface(surf, x, y, w, h);

            for (i = 0; i < h; i++)
            {
                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(y + i, surf->clipping.y, surf->clipping.height))
                        continue;
                }

                for (j = 0; j < w; j++)
                {
                    if (surf->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                            continue;
                    }
                    yy = (int)(k * (j - w / 2) + n);
                    n1 = (int)((i - h / 2) - k1 * (j - w / 2));
                    x1 = (int)((n1 - n) / (k - k1));
                    y1 = (int)(k1 * x1 + n1);
                    d  = sqrtf(powf((j - w / 2) - x1, 2.0f) + powf((i - h / 2) - y1, 2.0f)) / (2.0f * radius);
                    if ((i - h / 2) <= yy)
                    {
                        a = (int)((0.5f - d) * endColor->alpha + (0.5f + d) * startColor->alpha);
                        r = (int)((0.5f - d) * endColor->red + (0.5f + d) * startColor->red);
                        g = (int)((0.5f - d) * endColor->green + (0.5f + d) * startColor->green);
                        b = (int)((0.5f - d) * endColor->blue + (0.5f + d) * startColor->blue);
                    }
                    else
                    {
                        a = (int)((0.5f - d) * startColor->alpha + (0.5 + d) * endColor->alpha);
                        r = (int)((0.5f - d) * startColor->red + (0.5 + d) * endColor->red);
                        g = (int)((0.5f - d) * startColor->green + (0.5 + d) * endColor->green);
                        b = (int)((0.5f - d) * startColor->blue + (0.5 + d) * endColor->blue);
                    }
                    ptr[surf->width * i + j] = ITH_ARGB8888(a, r, g, b);
                }
            }
            ituUnlockSurface(surf);
        }
    }
}

static void RotateImage16(
    unsigned short *dst,
    unsigned short *src,
    int            dstWidth,
    int            dstHeight,
    int            dstPitch,
    int            srcWidth,
    int            srcHeight,
    int            srcPitch,
    int            centerX,
    int            centerY,
    float          angle,
    float          scale)
{
    float          dst0X_o, dst0Y_o, dst1X_o, dst1Y_o, dst2X_o, dst2Y_o, width_cosT, width_sinT, height_cosT, height_sinT;
    float          sinT, cosT, dst_half_width_scaled, dst_half_height_scaled;
    int            leftDist, rightDist, upDist, downDist, h, v;
    int            row_dX, row_dY, col_dX, col_dY, dstColX, dstColY, dstX, dstY, leftX, upY;
    unsigned short pixel, *dstCol, *dstRow, *pixPtr, newA, newR, newG, newB;
    unsigned int   ulPixel, urPixel, dlPixel, drPixel, ulValue, urValue, dlValue, drValue;

    // prepare dst0X, dst0Y, row_dx, row_dy, col_dx, col_dy
    ulPixel                = urPixel = dlPixel = drPixel = 0;
    angle                  = -angle;
    sinT                   = sinf(angle * (3.14159265f / 180.0f));
    cosT                   = cosf(angle * (3.14159265f / 180.0f));
    dst_half_width_scaled  = dstWidth / 2 / scale;
    dst_half_height_scaled = dstHeight / 2 / scale;
    width_cosT             = dst_half_width_scaled * cosT;
    width_sinT             = dst_half_width_scaled * sinT;
    height_cosT            = dst_half_height_scaled * cosT;
    height_sinT            = dst_half_height_scaled * sinT;
    dst0X_o                = -width_cosT - -height_sinT;
    dst0Y_o                = -width_sinT + -height_cosT;
    dst1X_o                = width_cosT - -height_sinT;
    dst1Y_o                = width_sinT + -height_cosT;
    dst2X_o                = -width_cosT - height_sinT;
    dst2Y_o                = -width_sinT + height_cosT;
    row_dX                 = (int)(((dst1X_o - dst0X_o) / dstWidth) * 65536);
    row_dY                 = (int)(((dst1Y_o - dst0Y_o) / dstWidth) * 65536);
    col_dX                 = (int)(((dst2X_o - dst0X_o) / dstHeight) * 65536);
    col_dY                 = (int)(((dst2Y_o - dst0Y_o) / dstHeight) * 65536);

    // start to generate rotated image
    dstCol                 = dst;
    dstColX                = (int)(dst0X_o * 65536) + (centerX << 16);
    dstColY                = (int)(dst0Y_o * 65536) + (centerY << 16);

    for (v = 0; v < dstHeight; v++)
    {
        dstRow = dstCol;
        dstX   = dstColX;
        dstY   = dstColY;
        for (h = 0; h < dstWidth; h++)
        {
            if (dstX < 0 || dstX >= ((srcWidth - 1) << 16) || dstY < 0 || dstY >= ((srcHeight - 1) << 16))
            {
                // out of source boundary
                pixel = 0;
            }
            else
            {
                leftX     = dstX >> 16;
                upY       = dstY >> 16;

                pixPtr    = src + upY * srcPitch + leftX;
                ulPixel   = *(pixPtr);
                urPixel   = *(pixPtr + 1);
                dlPixel   = *(pixPtr + srcPitch);
                drPixel   = *(pixPtr + srcPitch + 1);

                leftDist  = (dstX & 0x0FFFF) >> 8;
                rightDist = 256 - leftDist;
                upDist    = (dstY & 0x0FFFF) >> 8;
                downDist  = 256 - upDist;

                ulValue   = ulPixel >> 12;
                urValue   = urPixel >> 12;
                dlValue   = dlPixel >> 12;
                drValue   = drPixel >> 12;
                newA      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = (ulPixel >> 8) & 0x0F;
                urValue   = (urPixel >> 8) & 0x0F;
                dlValue   = (dlPixel >> 8) & 0x0F;
                drValue   = (drPixel >> 8) & 0x0F;
                newR      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = (ulPixel >> 4) & 0x0F;
                urValue   = (urPixel >> 4) & 0x0F;
                dlValue   = (dlPixel >> 4) & 0x0F;
                drValue   = (drPixel >> 4) & 0x0F;
                newG      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = ulPixel & 0x0F;
                urValue   = urPixel & 0x0F;
                dlValue   = dlPixel & 0x0F;
                drValue   = drPixel & 0x0F;
                newB      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                pixel     = (newA << 12) | (newR << 8) | (newG << 4) | newB;
            }
            *(dstRow) = pixel;
            dstRow++;
            dstX     += row_dX;
            dstY     += row_dY;
        }
        dstCol  += dstPitch;
        dstColX += col_dX;
        dstColY += col_dY;
    }
}

static void RotateImage32(
    unsigned int *dst,
    unsigned int *src,
    int          dstWidth,
    int          dstHeight,
    int          dstPitch,
    int          srcWidth,
    int          srcHeight,
    int          srcPitch,
    int          centerX,
    int          centerY,
    float        angle,
    float        scale)
{
    float        dst0X_o, dst0Y_o, dst1X_o, dst1Y_o, dst2X_o, dst2Y_o, width_cosT, width_sinT, height_cosT, height_sinT;
    float        sinT, cosT, dst_half_width_scaled, dst_half_height_scaled;
    int          leftDist, rightDist, upDist, downDist, h, v;
    int          row_dX, row_dY, col_dX, col_dY, dstColX, dstColY, dstX, dstY, leftX, upY;
    unsigned int pixel, *dstCol, *dstRow, *pixPtr, newA, newR, newG, newB;
    unsigned int ulPixel, urPixel, dlPixel, drPixel, ulValue, urValue, dlValue, drValue;

    // prepare dst0X, dst0Y, row_dx, row_dy, col_dx, col_dy
    ulPixel                = urPixel = dlPixel = drPixel = 0;
    angle                  = -angle;
    sinT                   = sinf(angle * (3.14159265f / 180.0f));
    cosT                   = cosf(angle * (3.14159265f / 180.0f));
    dst_half_width_scaled  = dstWidth / 2 / scale;
    dst_half_height_scaled = dstHeight / 2 / scale;
    width_cosT             = dst_half_width_scaled * cosT;
    width_sinT             = dst_half_width_scaled * sinT;
    height_cosT            = dst_half_height_scaled * cosT;
    height_sinT            = dst_half_height_scaled * sinT;
    dst0X_o                = -width_cosT - -height_sinT;
    dst0Y_o                = -width_sinT + -height_cosT;
    dst1X_o                = width_cosT - -height_sinT;
    dst1Y_o                = width_sinT + -height_cosT;
    dst2X_o                = -width_cosT - height_sinT;
    dst2Y_o                = -width_sinT + height_cosT;
    row_dX                 = (int)(((dst1X_o - dst0X_o) / dstWidth) * 65536);
    row_dY                 = (int)(((dst1Y_o - dst0Y_o) / dstWidth) * 65536);
    col_dX                 = (int)(((dst2X_o - dst0X_o) / dstHeight) * 65536);
    col_dY                 = (int)(((dst2Y_o - dst0Y_o) / dstHeight) * 65536);

    // start to generate rotated image
    dstCol                 = dst;
    dstColX                = (int)(dst0X_o * 65536) + (centerX << 16);
    dstColY                = (int)(dst0Y_o * 65536) + (centerY << 16);

    for (v = 0; v < dstHeight; v++)
    {
        dstRow = dstCol;
        dstX   = dstColX;
        dstY   = dstColY;
        for (h = 0; h < dstWidth; h++)
        {
            if (dstX < 0 || dstX >= ((srcWidth - 1) << 16) || dstY < 0 || dstY >= ((srcHeight - 1) << 16))
            {
                // out of source boundary
                pixel = 0;
            }
            else
            {
                leftX     = dstX >> 16;
                upY       = dstY >> 16;

                pixPtr    = src + upY * srcPitch + leftX;
                ulPixel   = *(pixPtr);
                urPixel   = *(pixPtr + 1);
                dlPixel   = *(pixPtr + srcPitch);
                drPixel   = *(pixPtr + srcPitch + 1);

                leftDist  = (dstX & 0x0FFFF) >> 8;
                rightDist = 256 - leftDist;
                upDist    = (dstY & 0x0FFFF) >> 8;
                downDist  = 256 - upDist;

                ulValue   = ulPixel >> 24;
                urValue   = urPixel >> 24;
                dlValue   = dlPixel >> 24;
                drValue   = drPixel >> 24;
                newA      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = (ulPixel >> 16) & 0xFF;
                urValue   = (urPixel >> 16) & 0xFF;
                dlValue   = (dlPixel >> 16) & 0xFF;
                drValue   = (drPixel >> 16) & 0xFF;
                newR      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = (ulPixel >> 8) & 0xFF;
                urValue   = (urPixel >> 8) & 0xFF;
                dlValue   = (dlPixel >> 8) & 0xFF;
                drValue   = (drPixel >> 8) & 0xFF;
                newG      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = ulPixel & 0xFF;
                urValue   = urPixel & 0xFF;
                dlValue   = dlPixel & 0xFF;
                drValue   = drPixel & 0xFF;
                newB      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                pixel     = (newA << 24) | (newR << 16) | (newG << 8) | newB;
            }
            *(dstRow) = pixel;
            dstRow++;
            dstX     += row_dX;
            dstY     += row_dY;
        }
        dstCol  += dstPitch;
        dstColX += col_dX;
        dstColY += col_dY;
    }
}

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static void SWRotate(ITUSurface *dest, int dx, int dy, ITUSurface *src, int cx, int cy, float angle, float scaleX, float scaleY)
{
    int   dstwidth, dstheight;
    float r0   = sqrtf((float)((0 - cx) * (0 - cx) + (0 - cy) * (0 - cy)));
    float r1   = sqrtf((float)((src->width - cx) * (src->width - cx) + (0 - cy) * (0 - cy)));
    float r2   = sqrtf((float)((0 - cx) * (0 - cx) + (src->height - cy) * (src->height - cy)));
    float r3   = sqrtf((float)((src->width - cx) * (src->width - cx) + (src->height - cy) * (src->height - cy)));
    float rmax = r0;

    if (r1 > rmax)
        rmax = r1;
    if (r2 > rmax)
        rmax = r2;
    if (r3 > rmax)
        rmax = r3;

    rmax    += 2; // guard band

    dstwidth = dstheight = (int)rmax * 2;

    if (src->format == ITU_RGB565)
    {
        ITUSurface *surf = SWCreateSurface(dstwidth, dstheight, 0, ITU_RGB565, NULL, 0);
        if (surf)
        {
            uint8_t *srcPtr  = ituLockSurface(src, 0, 0, src->width, src->height);
            uint8_t *destPtr = SWLockSurface(surf, 0, 0, surf->width, surf->height);

            RotateImage16((unsigned short *)destPtr, (unsigned short *)srcPtr, dstwidth, dstheight, dstwidth, src->width, src->height, src->width, cx, cy, angle, scaleX);

            SWUnlockSurface(src);
            SWUnlockSurface(surf);

            ituBitBlt(dest, dx - dstwidth / 2, dy - dstheight / 2, surf->width, surf->height, surf, 0, 0);

            SWDestroySurface(surf);
        }
    }
    else if (src->format == ITU_ARGB1555)
    {
        ITUSurface *surf = SWCreateSurface(dstwidth, dstheight, 0, ITU_ARGB1555, NULL, 0);
        if (surf)
        {
            uint8_t *srcPtr  = ituLockSurface(src, 0, 0, src->width, src->height);
            uint8_t *destPtr = SWLockSurface(surf, 0, 0, surf->width, surf->height);

            RotateImage16((unsigned short *)destPtr, (unsigned short *)srcPtr, dstwidth, dstheight, dstwidth, src->width, src->height, src->width, cx, cy, angle, scaleX);

            SWUnlockSurface(src);
            SWUnlockSurface(surf);

            ituBitBlt(dest, dx - dstwidth / 2, dy - dstheight / 2, surf->width, surf->height, surf, 0, 0);

            SWDestroySurface(surf);
        }
    }
    else if (src->format == ITU_ARGB4444)
    {
        ITUSurface *surf = SWCreateSurface(dstwidth, dstheight, 0, ITU_ARGB4444, NULL, 0);
        if (surf)
        {
            uint8_t *srcPtr  = ituLockSurface(src, 0, 0, src->width, src->height);
            uint8_t *destPtr = SWLockSurface(surf, 0, 0, surf->width, surf->height);

            RotateImage16((unsigned short *)destPtr, (unsigned short *)srcPtr, dstwidth, dstheight, dstwidth, src->width, src->height, src->width, cx, cy, angle, scaleX);

            SWUnlockSurface(src);
            SWUnlockSurface(surf);

            ituBitBlt(dest, dx - dstwidth / 2, dy - dstheight / 2, surf->width, surf->height, surf, 0, 0);

            SWDestroySurface(surf);
        }
    }
    else if (src->format == ITU_ARGB8888)
    {
        ITUSurface *surf = SWCreateSurface(dstwidth, dstheight, 0, ITU_ARGB8888, NULL, 0);
        if (surf)
        {
            uint8_t *srcPtr  = ituLockSurface(src, 0, 0, src->width, src->height);
            uint8_t *destPtr = SWLockSurface(surf, 0, 0, surf->width, surf->height);

            RotateImage32((unsigned int *)destPtr, (unsigned int *)srcPtr, dstwidth, dstheight, dstwidth, src->width, src->height, src->width, cx, cy, angle, scaleX);

            SWUnlockSurface(src);
            SWUnlockSurface(surf);

            ituBitBlt(dest, dx - dstwidth / 2, dy - dstheight / 2, surf->width, surf->height, surf, 0, 0);

            SWDestroySurface(surf);
        }
    }
    else if (src->format == ITU_MONO)
    {
        // TODO: IMPLEMENT
    }
    else if (src->format == ITU_RGB565A8)
    {
        ITUSurface *srcSurf = SWCreateSurface(src->width, src->height, 0, ITU_ARGB8888, NULL, 0);
        if (srcSurf)
        {
            ITUSurface *surf;

            ituAlphaBlend(srcSurf, 0, 0, srcSurf->width, srcSurf->height, src, 0, 0, 255);

            surf = SWCreateSurface(dstwidth, dstheight, 0, ITU_ARGB8888, NULL, 0);
            if (surf)
            {
                uint8_t *srcPtr  = ituLockSurface(srcSurf, 0, 0, srcSurf->width, srcSurf->height);
                uint8_t *destPtr = SWLockSurface(surf, 0, 0, surf->width, surf->height);

                RotateImage32((unsigned int *)destPtr, (unsigned int *)srcPtr, dstwidth, dstheight, dstwidth, src->width, src->height, src->width, cx, cy, angle, scaleX);

                SWUnlockSurface(srcSurf);
                SWUnlockSurface(surf);

                ituBitBlt(dest, dx - dstwidth / 2, dy - dstheight / 2, surf->width, surf->height, surf, 0, 0);

                SWDestroySurface(surf);
            }
            SWDestroySurface(srcSurf);
        }
    }
}

static void SWFlip(ITUSurface *surf)
{
    if (swRotation == ITU_ROT_90)
    {
        ITUSurface *lcdSurf = ituGetDisplaySurface();
        int        x, y, i;

        if (lcdSurf->format == ITU_RGB565 || ITU_ARGB1555 || ITU_ARGB4444)
        {
            uint16_t *src  = (uint16_t *)ituLockSurface(lcdSurf, 0, 0, lcdSurf->width, lcdSurf->height);
            uint16_t *dest = (uint16_t *)ithMapVram(swLcdAddr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_WRITE);

            i = 0;
            for (x = 0; x < lcdSurf->width; x++)
            {
                for (y = lcdSurf->height - 1; y >= 0; y--)
                {
                    dest[i++] = src[lcdSurf->width * y + x];
                }
            }
            ithUnmapVram(dest, ithLcdGetPitch() * ithLcdGetHeight());
            ituUnlockSurface(lcdSurf);
        }
    }
    else if (swRotation == ITU_ROT_180)
    {
        ITUSurface *lcdSurf = ituGetDisplaySurface();
        int        x, y, i;

        if (lcdSurf->format == ITU_RGB565 || ITU_ARGB1555 || ITU_ARGB4444)
        {
            uint16_t *src  = (uint16_t *)ituLockSurface(lcdSurf, 0, 0, lcdSurf->width, lcdSurf->height);
            uint16_t *dest = (uint16_t *)ithMapVram(swLcdAddr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_WRITE);

            i = 0;
            for (y = lcdSurf->height - 1; y >= 0; y--)
            {
                for (x = lcdSurf->width - 1; x >= 0; x--)
                {
                    dest[i++] = src[lcdSurf->width * y + x];
                }
            }
            ithUnmapVram(dest, ithLcdGetPitch() * ithLcdGetHeight());
            ituUnlockSurface(lcdSurf);
        }
    }
    else if (swRotation == ITU_ROT_270)
    {
        ITUSurface *lcdSurf = ituGetDisplaySurface();
        int        x, y, i;

        if (lcdSurf->format == ITU_RGB565 || ITU_ARGB1555 || ITU_ARGB4444)
        {
            uint16_t *src  = (uint16_t *)ituLockSurface(lcdSurf, 0, 0, lcdSurf->width, lcdSurf->height);
            uint16_t *dest = (uint16_t *)ithMapVram(swLcdAddr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_WRITE);

            i = 0;
            for (x = lcdSurf->width - 1; x >= 0; x--)
            {
                for (y = 0; y < lcdSurf->height; y++)
                {
                    dest[i++] = src[lcdSurf->width * y + x];
                }
            }
            ithUnmapVram(dest, ithLcdGetPitch() * ithLcdGetHeight());
            ituUnlockSurface(lcdSurf);
        }
    }
#ifdef SPEED_UP
    else
    {
        uint16_t *dest = (uint16_t *)ithMapVram(swLcdAddr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_WRITE);
        ithUnmapVram(dest, ithLcdGetPitch() * ithLcdGetHeight());
    }
#endif

#ifdef CFG_WIN32_SIMULATOR
    {
        HDC     dc, bitmapDc;
        uint8_t *ptr;

        dc       = GetDC(win);
        assert(dc);
        bitmapDc = CreateCompatibleDC(dc);

        SelectObject(bitmapDc, bitmap);

        ptr = ithMapVram(swLcdAddr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_READ | ITH_VRAM_WRITE);
        memcpy(bits, ptr, ithLcdGetPitch() * ithLcdGetHeight());
        ithUnmapVram(ptr, ithLcdGetPitch() * ithLcdGetHeight());

        BitBlt(dc, 0, 0, ithLcdGetWidth(), ithLcdGetHeight(), bitmapDc, 0, 0, SRCCOPY);

        DeleteDC(bitmapDc);
        ReleaseDC(win, dc);
    }
#endif // CFG_WIN32_SIMULATOR
}

static float CrossProduct(float v0x, float v0y, float v1x, float v1y)
{
    return v0x * v1y - v1x * v0y;
}

static bool IsCCW(int v1x, int v1y, int v2x, int v2y, int v3x, int v3y)
{
    int v21x = v2x - v1x;
    int v21y = v2y - v1y;
    int v23x = v2x - v3x;
    int v23y = v2y - v3y;
    return CrossProduct(v21x, v21y, v23x, v23y) > 0;
}

static bool IsOnPlaneABCD(int x, int y, int v0x, int v0y, int v1x, int v1y, int v2x, int v2y, int v3x, int v3y)
{
    if (!IsCCW(x, y, v0x, v0y, v1x, v1y))
    {
        if (!IsCCW(x, y, v1x, v1y, v2x, v2y))
        {
            if (!IsCCW(x, y, v2x, v2y, v3x, v3y))
            {
                if (!IsCCW(x, y, v3x, v3y, v0x, v0y))
                    return true;
            }
        }
    }
    return false;
}

#define MIN(a,b) (((a)<(b))?(a):(b))

static void SWTransformBlt(ITUSurface* dest, int dx, int dy, ITUSurface* src, int sx, int sy, int sw, int sh, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, bool bInverse, ITUPageFlowType type, ITUTransformType transformType)
{
	int minx = MIN(x0,x3);
	int miny = MIN(y0,y1);
	int maxx = MAX(x1,x2);
	int maxy = MAX(y2,y3);
    if (dest->format == ITU_RGB565)
    {
        int      x, y;
        uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, 0, 0, dest->width, dest->height);

        if (src->format == ITU_RGB565)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, 0, 0, src->width, src->height);
            float len;
            float ABx, ABy, BCx, BCy, CDx, CDy, DAx, DAy;
            
            ABx = x1 - x0;
            ABy = y1 - y0;
            len = sqrtf(ABx * ABx + ABy * ABy);
            ABx /= len;
            ABy /= len;

            BCx = x2 - x1;
            BCy = y2 - y1;
            len = sqrtf(BCx * BCx + BCy * BCy);
            BCx /= len;
            BCy /= len;

            CDx = x3 - x2;
            CDy = y3 - y2;
            len = sqrtf(CDx * CDx + CDy * CDy);
            CDx /= len;
            CDy /= len;

            DAx = x0 - x3;
            DAy = y0 - y3;
            len = sqrtf(DAx * DAx + DAy * DAy);
            DAx /= len;
            DAy /= len;

            for (y = miny; y < maxy; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = minx; x < maxx; x++)
                {
                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }

                    if (x == src->width-1)
                    {
                        x = src->width-1;
                    }

                    if (IsOnPlaneABCD(x, y, x0, y0, x1, y1, x2, y2, x3, y3))
                    {
                        int v0x = x - x0;
                        int v0y = y - y0;
                        int v1x = x - x1;
                        int v1y = y - y1;
                        int v2x = x - x2;
                        int v2y = y - y2;
                        int v3x = x - x3;
                        int v3y = y - y3;
                        float dab = fabsf(CrossProduct(v0x, v0y, ABx, ABy));
                        float dbc = fabsf(CrossProduct(v1x, v1y, BCx, BCy));
                        float dcd = fabsf(CrossProduct(v2x, v2y, CDx, CDy));
                        float dda = fabsf(CrossProduct(v3x, v3y, DAx, DAy));
                        int xx = src->width * dda / (dda + dbc);
                        int yy = src->height * dab / (dab + dcd);

                        if (xx >= sx && xx < sw && yy >= sy && yy < sh)
                        {
                            destPtr[dest->width * (dy + y) + (dx + x)] = srcPtr[src->width * yy + xx];
                        }
                    }
                }
            }
        }
        else if (src->format == ITU_ARGB1555)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, 0, 0, src->width, src->height);
            float len;
            float ABx, ABy, BCx, BCy, CDx, CDy, DAx, DAy;
            
            ABx = x1 - x0;
            ABy = y1 - y0;
            len = sqrtf(ABx * ABx + ABy * ABy);
            ABx /= len;
            ABy /= len;

            BCx = x2 - x1;
            BCy = y2 - y1;
            len = sqrtf(BCx * BCx + BCy * BCy);
            BCx /= len;
            BCy /= len;

            CDx = x3 - x2;
            CDy = y3 - y2;
            len = sqrtf(CDx * CDx + CDy * CDy);
            CDx /= len;
            CDy /= len;

            DAx = x0 - x3;
            DAy = y0 - y3;
            len = sqrtf(DAx * DAx + DAy * DAy);
            DAx /= len;
            DAy /= len;

            for (y = 0; y < src->height; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }

                for (x = 0; x < src->width; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }

                    if (IsOnPlaneABCD(x, y, x0, y0, x1, y1, x2, y2, x3, y3))
                    {
                        int v0x = x - x0;
                        int v0y = y - y0;
                        int v1x = x - x1;
                        int v1y = y - y1;
                        int v2x = x - x2;
                        int v2y = y - y2;
                        int v3x = x - x3;
                        int v3y = y - y3;
                        float dab = fabsf(CrossProduct(v0x, v0y, ABx, ABy));
                        float dbc = fabsf(CrossProduct(v1x, v1y, BCx, BCy));
                        float dcd = fabsf(CrossProduct(v2x, v2y, CDx, CDy));
                        float dda = fabsf(CrossProduct(v3x, v3y, DAx, DAy));
                        int xx = src->width * dda / (dda + dbc);
                        int yy = src->height * dab / (dab + dcd);

                        if (xx >= sx && xx < sw && yy >= sy && yy < sh)
                        {
                            sc                           = srcPtr[src->width * yy + xx];
                            dc                           = destPtr[dest->width * y + x];
                            a                            = (sc & 0x8000) ? 255 : 0;
                            r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x7C00) >> 10) << 3) * a) / 255;
                            g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x03E0) >> 5) << 3) * a) / 255;
                            b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001F) << 3) * a) / 255;
                            destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                        }
                    }
                }
            }
        }
        else if (src->format == ITU_ARGB4444)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, 0, 0, src->width, src->height);
            float len;
            float ABx, ABy, BCx, BCy, CDx, CDy, DAx, DAy;
            
            ABx = x1 - x0;
            ABy = y1 - y0;
            len = sqrtf(ABx * ABx + ABy * ABy);
            ABx /= len;
            ABy /= len;

            BCx = x2 - x1;
            BCy = y2 - y1;
            len = sqrtf(BCx * BCx + BCy * BCy);
            BCx /= len;
            BCy /= len;

            CDx = x3 - x2;
            CDy = y3 - y2;
            len = sqrtf(CDx * CDx + CDy * CDy);
            CDx /= len;
            CDy /= len;

            DAx = x0 - x3;
            DAy = y0 - y3;
            len = sqrtf(DAx * DAx + DAy * DAy);
            DAx /= len;
            DAy /= len;

            for (y = 0; y < src->height; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < src->width; x++)
                {
                    uint16_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }

                    if (IsOnPlaneABCD(x, y, x0, y0, x1, y1, x2, y2, x3, y3))
                    {
                        int v0x = x - x0;
                        int v0y = y - y0;
                        int v1x = x - x1;
                        int v1y = y - y1;
                        int v2x = x - x2;
                        int v2y = y - y2;
                        int v3x = x - x3;
                        int v3y = y - y3;
                        float dab = fabsf(CrossProduct(v0x, v0y, ABx, ABy));
                        float dbc = fabsf(CrossProduct(v1x, v1y, BCx, BCy));
                        float dcd = fabsf(CrossProduct(v2x, v2y, CDx, CDy));
                        float dda = fabsf(CrossProduct(v3x, v3y, DAx, DAy));
                        int xx = src->width * dda / (dda + dbc);
                        int yy = src->height * dab / (dab + dcd);

                        if (xx >= sx && xx < sw && yy >= sy && yy < sh)
                        {
                            sc                           = srcPtr[src->width * yy + xx];
                            dc                           = destPtr[dest->width * y + x];
                            a                            = (sc & 0xF000) >> 12;
                            a                           |= a << 4;
                            r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0x0F00) >> 8) << 4) * a) / 255;
                            g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x00F0) >> 4) << 4) * a) / 255;
                            b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x000F) << 4) * a) / 255;
                            destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                        }
                    }
                }
            }
        }
        else if (src->format == ITU_ARGB8888)
        {
            uint32_t *srcPtr = (uint32_t *)ituLockSurface(src, 0, 0, src->width, src->height);
            float len;
            float ABx, ABy, BCx, BCy, CDx, CDy, DAx, DAy;
            
            ABx = x1 - x0;
            ABy = y1 - y0;
            len = sqrtf(ABx * ABx + ABy * ABy);
            ABx /= len;
            ABy /= len;

            BCx = x2 - x1;
            BCy = y2 - y1;
            len = sqrtf(BCx * BCx + BCy * BCy);
            BCx /= len;
            BCy /= len;

            CDx = x3 - x2;
            CDy = y3 - y2;
            len = sqrtf(CDx * CDx + CDy * CDy);
            CDx /= len;
            CDy /= len;

            DAx = x0 - x3;
            DAy = y0 - y3;
            len = sqrtf(DAx * DAx + DAy * DAy);
            DAx /= len;
            DAy /= len;

            for (y = 0; y < src->height; y++)
            {
                if (dest->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                        continue;
                }
                for (x = 0; x < src->width; x++)
                {
                    uint32_t sc, dc, a, r, g, b;

                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                            continue;
                    }

                    if (IsOnPlaneABCD(x, y, x0, y0, x1, y1, x2, y2, x3, y3))
                    {
                        int v0x = x - x0;
                        int v0y = y - y0;
                        int v1x = x - x1;
                        int v1y = y - y1;
                        int v2x = x - x2;
                        int v2y = y - y2;
                        int v3x = x - x3;
                        int v3y = y - y3;
                        float dab = fabsf(CrossProduct(v0x, v0y, ABx, ABy));
                        float dbc = fabsf(CrossProduct(v1x, v1y, BCx, BCy));
                        float dcd = fabsf(CrossProduct(v2x, v2y, CDx, CDy));
                        float dda = fabsf(CrossProduct(v3x, v3y, DAx, DAy));
                        int xx = src->width * dda / (dda + dbc);
                        int yy = src->height * dab / (dab + dcd);

                        if (xx >= sx && xx < sw && yy >= sy && yy < sh)
                        {
                            sc                           = srcPtr[src->width * yy + xx];
                            dc                           = destPtr[dest->width * y + x];
                            a                            = sc >> 24;
                            r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + ((sc & 0x00FF0000) >> 16) * a) / 255;
                            g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + ((sc & 0x0000FF00) >> 8) * a) / 255;
                            b                            = (((dc & 0x001f) << 3) * (255 - a) + (sc & 0x000000FF) * a) / 255;
                            destPtr[dest->width * y + x] = ITH_RGB565(r, g, b);
                        }
                    }
                }
            }
        }
        ituUnlockSurface(src);
        ituUnlockSurface(dest);
    }
}

static void SWReflected(ITUSurface* dest, int dx, int dy, ITUSurface* src, int sx, int sy, int reflectedWidth, int reflectedHeight, ITUSurface* masksurf)
{
    int w, h;
    int dw = dest->width - dx;
    int dh = dest->height - dy;
    int sw = src->width - sx;
    int sh = src->height - sy;

    w = reflectedWidth;
    h = reflectedHeight;

    if (w > dw)
        w = dw;
    if (w > sw)
        w = sw;
    if (h > dh)
        h = dh;
    if (h > sh)
        h = sh;

    if (dest->format == ITU_RGB565)
    {
        int      x, y;
        uint16_t *destPtr = (uint16_t *)ituLockSurface(dest, dx, dy, w, h);

        if (src->format == ITU_RGB565)
        {
            uint16_t *srcPtr = (uint16_t *)ituLockSurface(src, sx, sy, w, h);

            if (masksurf)
            {
                uint8_t *maskPtr = (uint8_t *)ituLockSurface(masksurf, sx, sy, w, h);

                for (y = 0; y < h; y++)
                {
                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                            continue;
                    }

                    for (x = 0; x < w; x++)
                    {
                        uint16_t sc, dc, a, r, g, b;

                        if (dest->flags & ITU_CLIPPING)
                        {
                            if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                                continue;
                        }
                        sc                           = srcPtr[src->width * y + x];
                        dc                           = destPtr[dest->width * (h - y - 1) + x];
                        a                            = maskPtr[masksurf->width * (h - y - 1) + x];
                        r                            = ((((dc & 0xf800) >> 11) << 3) * (255 - a) + (((sc & 0xf800) >> 11) << 3) * a) / 255;
                        g                            = ((((dc & 0x07e0) >> 5) << 2) * (255 - a) + (((sc & 0x07e0) >> 5) << 2) * a) / 255;
                        b                            = (((dc & 0x001f) << 3) * (255 - a) + ((sc & 0x001f) << 3) * a) / 255;
                        destPtr[dest->width * (h - y - 1) + x] = ITH_RGB565(r, g, b);
                    }
                }
                ituUnlockSurface(masksurf);
            }
            else
            {
                for (y = 0; y < h; y++)
                {
                    if (dest->flags & ITU_CLIPPING)
                    {
                        if (OUT_OF_RANGE(dy + y, dest->clipping.y, dest->clipping.height))
                            continue;
                    }

                    for (x = 0; x < w; x++)
                    {
                        if (dest->flags & ITU_CLIPPING)
                        {
                            if (OUT_OF_RANGE(dx + x, dest->clipping.x, dest->clipping.width))
                                continue;
                        }
                        destPtr[dest->width * (h - y - 1) + x] = srcPtr[src->width * y + x];
                    }
                }
            }
        }
        ituUnlockSurface(src);
        ituUnlockSurface(dest);
    }
}

static void SWDrawLine(ITUSurface* dest, int32_t x0, int32_t y0, int32_t x1, int32_t y1, ITUColor* color, int32_t lineWidth)
{
#ifdef CFG_WIN32_SIMULATOR
    int dx = abs(x1-x0), sx = x0 < x1 ? 1 : -1; 
    int dy = abs(y1-y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx-dy, e2, x2, y2;                          /* error value e_xy */
    float wd = (float)lineWidth;
    float ed = dx+dy == 0 ? 1.0f : sqrtf((float)dx*dx+(float)dy*dy);
    float alpha;

    ITUSurface* surf = ituCreateSurface(1, 1, 0, dest->format, NULL, 0);
    if (surf)
    {
        ituColorFill(surf, 0, 0, 1, 1, color);

        for (wd = (wd+1)/2; ; )
        {                                   /* pixel loop */
            alpha = 255-max(0,255*(abs(err-dx+dy)/ed-wd+1));
            ituAlphaBlend(dest, x0, y0, 1, 1, surf, 0, 0, (uint8_t)alpha);

            e2 = err; x2 = x0;
            if (2*e2 >= -dx)
            {                                           /* x step */
                for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
                {
                    alpha = 255-max(0,255*(abs(e2)/ed-wd+1));
                    ituAlphaBlend(dest, x0, y2 += sy, 1, 1, surf, 0, 0, (uint8_t)alpha);
                }

                if (x0 == x1)
                    break;

                e2 = err; err -= dy; x0 += sx; 
            } 
            if (2*e2 <= dy)
            {                                            /* y step */
                for (e2 = dx-e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
                {
                    alpha = 255-max(0,255*(abs(e2)/ed-wd+1));
                    ituAlphaBlend(dest, x2 += sx, y0, 1, 1, surf, 0, 0, (uint8_t)alpha);
                }

                if (y0 == y1)
                    break;

                err += dx; y0 += sy; 
            }
        }
        ituDestroySurface(surf);
    }
#endif
}

void ituSWInit(void)
{
    ITUSurface *lcdSurf = ituGetDisplaySurface();
    assert(lcdSurf);

    ituCreateSurface  = SWCreateSurface;
    ituDestroySurface = SWDestroySurface;
    ituLockSurface    = SWLockSurface;
    ituUnlockSurface  = SWUnlockSurface;
    ituSetRotation    = SWSetRotation;
    ituDrawGlyph      = SWDrawGlyph;
    ituBitBlt         = SWBitBlt;
    ituStretchBlt     = SWStretchBlt;
    ituAlphaBlend     = SWAlphaBlend;
    ituColorFill      = SWColorFill;
    ituGradientFill   = SWGradientFill;
    ituRotate         = SWRotate;
    ituTransformBlt   = SWTransformBlt;
    ituReflected      = SWReflected;
    ituDrawLine       = SWDrawLine;
    ituFlip           = SWFlip;

    swRotation        = ITU_ROT_0;
    swLcdAddr         = lcdSurf->addr;

#ifdef CFG_WIN32_SIMULATOR
    if (!win)
    {
        HDC              dc;
        BITMAPINFOHEADER *bmiHeader;
        DWORD            *colors;

        win                        = GetForegroundWindow();
        assert(win);

        bmiHeader                  = (BITMAPINFOHEADER *) calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * 3);
        assert(bmiHeader);

        bmiHeader->biSize          = sizeof(BITMAPINFOHEADER);
        bmiHeader->biWidth         = ithLcdGetWidth();
        bmiHeader->biHeight        = -(LONG)ithLcdGetHeight();
        bmiHeader->biPlanes        = 1;
        bmiHeader->biBitCount      = 16;
        bmiHeader->biCompression   = BI_BITFIELDS;
        bmiHeader->biSizeImage     = 0;
        bmiHeader->biXPelsPerMeter = 0;
        bmiHeader->biYPelsPerMeter = 0;
        bmiHeader->biClrUsed       = 3;
        bmiHeader->biClrImportant  = 0;

        colors                     = (DWORD *) (bmiHeader + 1);
        colors[0]                  = 0xF800;
        colors[1]                  = 0x07E0;
        colors[2]                  = 0x001F;

        dc                         = GetDC(win);
        assert(dc);

        bitmap                     = CreateDIBSection(dc, (BITMAPINFO *) bmiHeader, DIB_RGB_COLORS, &bits, NULL, 0);
        assert(bitmap);
        assert(bits);

        ReleaseDC(win, dc);
        free(bmiHeader);
    }
#endif // CFG_WIN32_SIMULATOR
}

void ituSWExit(void)
{
#ifdef CFG_WIN32_SIMULATOR
    win = NULL;
#endif
}
