#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "itu_cfg.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "ite/itv.h"
//#include "ite/ite_m2d.h"
#include "gfx/gfx.h"
#include "itu_private.h"

#define MAX_ROTATE_BUFFER_COUNT    2

#define SMTK_RGB565(r, g, b) \
    (((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11))

#define OUT_OF_RANGE(p, start, length) (((p) < (start)) || ((p) >= ((start) + (length))))

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static ITURotation m2dRotation = ITU_ROT_0;
static uint32_t m2dLcdAddr;

static bool        gM2dInit = false;
static uint32_t    gRotateBuffer[MAX_ROTATE_BUFFER_COUNT];
static uint8_t     gRotateBufIdx = 0;
static ITUSurface  *gRoateSurf; 

void
_SetClipRectRegion(
    ITUSurface *surf,
    int        x,
    int        y,
    int        w,
    int        h)
{        
    int left   = surf->clipping.x;
    int top    = surf->clipping.y;
    int right  = (surf->clipping.x + surf->clipping.width - 1);
    int bottom = (surf->clipping.y + surf->clipping.height - 1);
    M2dSurface *m2dSurf = (M2dSurface *)surf;

    if (right >= (x + w))
        right = x + w - 1;
    if (bottom >= (y + h))
        bottom = y + h - 1;
    if (left < x)
        left = x;
    if (top < y)
        top = y;
    
    gfxSurfaceSetClip(m2dSurf->m2dSurf, left, top, right, bottom);
}

static void 
_SWColorFill(
    ITUSurface *surf,
    int x,
    int y,
    int w,
    int h,
    ITUColor *color)
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
                uint16_t dc, r, g, b;

                if (surf->flags & ITU_CLIPPING)
                {
                    if (OUT_OF_RANGE(x + j, surf->clipping.x, surf->clipping.width))
                        continue;
                }
                dc = ptr[surf->width * i + j];
                r = ((((dc & 0xf800) >> 11) << 3) * (255 - color->alpha) + color->red * color->alpha) / 255;
                g = ((((dc & 0x07e0) >> 5) << 2) * (255 - color->alpha) + color->green * color->alpha) / 255;
                b = (((dc & 0x001f) << 3) * (255 - color->alpha) + color->blue * color->alpha) / 255;
                ptr[surf->width * i + j] = ITH_RGB565(r, g, b);
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

static ITUSurface*
M2dCreateSurface(
    int w,
    int h,
    int pitch,
    ITUPixelFormat format,
    const uint8_t* bitmap,
    unsigned int flags)
{
    GFXColorFormat gfxformat = GFX_COLOR_UNKNOWN;
    GFXMaskFormat maskFormat;
    M2dSurface *surface = calloc(1, sizeof(M2dSurface));

    if (!surface)
    {
        LOG_ERR "Out of memory: %d\n", sizeof(M2dSurface)LOG_END
        return NULL;
    }

    surface->surf.width  = w;
    surface->surf.height = h;
    surface->surf.pitch  = (pitch) ? pitch : (w * ituFormat2Bpp(format) / 8);
    surface->surf.format = format;
    surface->surf.flags  = flags;
    ituSetColor(&surface->surf.fgColor, 255, 255, 255, 255);

    if(format == ITU_RGB565)
    {
        gfxformat = GFX_COLOR_RGB565;
    }
    else if(format == ITU_ARGB1555)
    {
        gfxformat = GFX_COLOR_ARGB1555;
    }
    else if(format == ITU_ARGB4444)
    {
        gfxformat = GFX_COLOR_ARGB4444;
    }
    else if(format == ITU_ARGB8888)
    {
        gfxformat = GFX_COLOR_ARGB8888;
    }
    else if (format == ITU_A8)
    {
        gfxformat = GFX_COLOR_ARGB8888;
        maskFormat = GFX_MASK_A_8;
    }
    else
    {
        gfxformat = GFX_COLOR_RGB565;
    }
    
    if (bitmap)
    {    	  
        surface->surf.addr = (uint32_t) bitmap;
    }
    else
    {
        uint8_t      *ptr;
        unsigned int size = (surface->surf.pitch + 4) * (h + 1);

        surface->surf.addr = itpVmemAlloc(size);
        if (!surface->surf.addr)
        {
            LOG_ERR "Out of VRAM: %d\n", size LOG_END
            free(surface);
            return NULL;
        }
#ifdef CFG_CPU_WB
        //The mapped memory area may be used by CPU and related cache may be dirty before.
        //Therefore, we need to invalidate the cache range to ensure no related cache flush while engine
        //writing this area simultaneously.
        else
        {
            ithInvalidateDCacheRange((void*)surface->surf.addr, size);
        }
#endif
    }
    
    if (format == ITU_A8)
    {
        surface->m2dSurf = gfxCreateSurface(
            (w + 3) / 4,
            h,
            (int)((w + 3) / 4) * 4,
            gfxformat);

        surface->m2dSurf->format = GFX_COLOR_A_8;
        surface->m2dSurf->maskSurface = gfxCreateMaskSurface(
            w,
            h,
            (int)((w + 3) / 4) * 4,
            maskFormat);
    }
    else
    {
        surface->m2dSurf = gfxCreateSurfaceByPointer(
            w,
            h,
            surface->surf.pitch,
            gfxformat,
            surface->surf.addr,
            h*surface->surf.pitch);
    }

    return (ITUSurface*)surface;
}

static void M2dDestroySurface(ITUSurface* surf)
{
    M2dSurface *m2dSurf = (M2dSurface *)surf;

    if (m2dSurf->m2dSurf)
    {        
        gfxDestroySurface(m2dSurf->m2dSurf);
    }

    if (surf && !(surf->flags & ITU_STATIC))
    {
        itpVmemFree(surf->addr);
    }
    free(surf);
}

static ITUMaskSurface*
M2dCreateMaskSurface(
    int w,
    int h,
    int pitch,
    ITUMaskFormat format,
    uint8_t* buffer,
    uint32_t bufferLength)
{
    GFXMaskFormat maskFormat = GFX_MASK_UNKNOWN;
    GFXMaskSurface* imgSurf = 0;

    if(format == ITU_MASK_A_8)
    {
        maskFormat = GFX_MASK_A_8;;
    }
    else if(format == ITU_MASK_A_4)
    {
        maskFormat = GFX_MASK_A_4;
    }
    else if(format == ITU_MASK_A_2)
    {
        maskFormat = GFX_MASK_A_2;
    }
    else if(format == ITU_MASK_A_1)
    {
        maskFormat = GFX_MASK_A_1;
    }

    imgSurf = gfxCreateMaskSurfaceByBuffer(
        w,
        h,
        pitch,
        maskFormat,
        buffer,
        bufferLength);

    return (ITUMaskSurface*)imgSurf;
}

static void M2dDestroyMaskSurface(ITUMaskSurface* surf)
{
    gfxDestroyMaskSurface((GFXMaskSurface*)surf);
}

static void M2dSetMaskSurface(ITUSurface* surface, ITUMaskSurface* maskSurface, bool enable)
{
    gfxSetMaskSurface((GFXSurface*)surface, (GFXMaskSurface*)maskSurface, enable);
}

static uint8_t* M2dLockSurface(ITUSurface* surf, int x, int y, int w, int h)
{
    uint32_t vram_addr;

    gfxwaitEngineIdle(); 

    surf->lockSize = surf->pitch * h;

    switch (surf->format)
    {
    default:
        vram_addr = surf->addr + surf->pitch * y + x * 2;
        break;

    case ITU_ARGB8888:
        vram_addr = surf->addr + surf->pitch * y + x * 4;
        break;

    case ITU_MONO:
        vram_addr = surf->addr + surf->pitch * y + (x + 7) / 8;
        break;
    }

#ifndef SPEED_UP
    surf->lockAddr = ithMapVram(vram_addr, surf->lockSize, ITH_VRAM_READ | ITH_VRAM_WRITE);
#else
    surf->lockAddr = ithMapVram(vram_addr, surf->lockSize, /*ITH_VRAM_READ |*/ ITH_VRAM_WRITE);
#endif
    return surf->lockAddr;
}

static void M2dUnlockSurface(ITUSurface* surf)
{
#ifndef SPEED_UP
    ithUnmapVram(surf->lockAddr, surf->lockSize);
#endif
}

static void M2dSetRotation(ITURotation rot)
{
    ITUSurface* lcdSurf;
    M2dSurface *screenSurf;

#ifdef CFG_VIDEO_ENABLE
    itv_set_rotation(rot);
#endif
    
    if (m2dRotation == rot)   
        return;

    lcdSurf = ituGetDisplaySurface();
    assert(lcdSurf);
    screenSurf = (M2dSurface *)lcdSurf;

    switch (rot)
    {
    default:
        lcdSurf->width  = ithLcdGetWidth();
        lcdSurf->height = ithLcdGetHeight();
        lcdSurf->pitch  = ithLcdGetPitch();
        break;

    case ITU_ROT_90:
    case ITU_ROT_270:
        lcdSurf->width  = ithLcdGetHeight();
        lcdSurf->height = ithLcdGetWidth();
        lcdSurf->pitch  = ithLcdGetPitch() * ithLcdGetHeight() / ithLcdGetWidth();
        break;
    }

    m2dRotation = rot;
    
    // create rotation buffer and sruface
    if (!gRotateBuffer[0] && m2dRotation != ITU_ROT_0)
    {
        uint32_t i;
        uint32_t size;
        uint32_t pitch;
        uint32_t tiling_block;
        
        if (ithIsTilingModeOn() == 0)
        {
            pitch = ithTilingPitch();
            tiling_block = 2048;
        } else {
            pitch = lcdSurf->pitch;
            tiling_block = 16;
        }
                       
        size = pitch * lcdSurf->height * MAX_ROTATE_BUFFER_COUNT; 

        gRotateBuffer[0] = itpVmemAlignedAlloc(tiling_block, size);

        assert(gRotateBuffer[0]);
        
        for (i = 1; i < MAX_ROTATE_BUFFER_COUNT; ++i)
        {
            gRotateBuffer[i] = gRotateBuffer[i - 1] + pitch * lcdSurf->height; 
        }
        
        lcdSurf->addr = screenSurf->surf.addr = gRotateBuffer[0];
        
        gfxSurfaceSetSurfaceBaseAddress(
            screenSurf->m2dSurf,
            screenSurf->surf.addr);
            
        gRoateSurf = M2dCreateSurface(ithLcdGetWidth(), ithLcdGetHeight(), ithLcdGetPitch(), ITU_RGB565, ithLcdGetBaseAddrA(), ITU_STATIC);       
        gfxSurfaceSetWidth(screenSurf->m2dSurf, lcdSurf->width);
        gfxSurfaceSetHeight(screenSurf->m2dSurf, lcdSurf->height);
        gfxSurfaceSetPitch(screenSurf->m2dSurf, pitch);
    }    
}

static void
M2dDrawGlyph(
    ITUSurface* surf,
    int x,
    int y,
    ITUGlyphFormat format,
    const uint8_t *bitmap,
    int w,
    int h)
{
    M2dSurface *destSurf  = (M2dSurface *)surf;
    GFXColor color = {0};
    GFXSurfaceSrc srcSurface = {0};
    GFXSurface tmpSrcSurf = {0};

    color.a = surf->fgColor.alpha;
    color.r = surf->fgColor.red;
    color.g = surf->fgColor.green;
    color.b = surf->fgColor.blue;

    tmpSrcSurf.imageData = bitmap;
    tmpSrcSurf.width = w;
    tmpSrcSurf.height = h;
    tmpSrcSurf.quality = GFX_QUALITY_BETTER;
    
    if (format == ITU_GLYPH_1BPP)
    {
        tmpSrcSurf.pitch = (w + 7) / 8;
        tmpSrcSurf.format = GFX_COLOR_A_1;
    }
    else if (format == ITU_GLYPH_4BPP)
    {
    	  tmpSrcSurf.pitch = (w + 1) / 2;
    	  tmpSrcSurf.format = GFX_COLOR_A_4;
    }
    else if (format == ITU_GLYPH_8BPP)
    {
    	  tmpSrcSurf.pitch = w;
    	  tmpSrcSurf.format = GFX_COLOR_A_8;
    }
    
    tmpSrcSurf.imageDataLength = tmpSrcSurf.pitch*tmpSrcSurf.height;
#ifdef CFG_CPU_WB
    ithFlushDCacheRange(tmpSrcSurf.imageData, tmpSrcSurf.imageDataLength);
    ithFlushMemBuffer();
#endif

    srcSurface.srcSurface = &tmpSrcSurf; //bitmap;
    srcSurface.srcX = 0;
    srcSurface.srcY = 0;
    srcSurface.srcWidth = w;
    srcSurface.srcHeight = h;

    //printf("x:%d,y:%d,w:%d,h:%d\n",x,y,w,h);
    //printf("a:%d,r:%d,g:%d,b:%d\n",color.a,color.r,color.g,color.b);

    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);
    }

    gfxSurfaceDrawGlyph(
        destSurf->m2dSurf,
        x,
        y,
        &srcSurface, 
        color);

    if (surf->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

static void
M2dBitBlt(
    ITUSurface* dest,
    int dx,
    int dy,
    int w,
    int h,
    ITUSurface* src,
    int sx,
    int sy)
{
    GFXSurfaceSrc srcSurface = {0};
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf  = (M2dSurface *)src;

    int        width, height;
    int        destX, destY;
    int        srcX, srcY;

    width  = (w > src->width)  ? src->width  : w;
    height = (h > src->height) ? src->height : h;

    if (dx < 0)
    {
        srcX  = sx - dx;
        destX = 0;
    }
    else
    {
        srcX  = sx;
        destX = dx;
    }

    if (dy < 0)
    {
        srcY  = sy - dy;
        destY = 0;
    }
    else
    {
        srcY  = sy;
        destY = dy;
    }

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, destX, destY, width, height);
    }

    srcSurface.srcSurface = srcSurf->m2dSurf;
    srcSurface.srcX = srcX;
    srcSurface.srcY = srcY;
    srcSurface.srcWidth = width;
    srcSurface.srcHeight = height;

    gfxSurfaceAlphaBlendEx(
        destSurf->m2dSurf,
        destX,
        destY,
        &srcSurface,
        false,
        0xFF);

    if (dest->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

static void
M2dStretchBlt(
    ITUSurface *dest,
    int dx,
    int dy,
    int dw,
    int dh,
    ITUSurface *src,
    int sx,
    int sy,
    int sw,
    int sh)
{
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf  = (M2dSurface *)src;    
    GFXSurfaceDst dstSurface = {0};
    GFXSurfaceSrc srcSurface = {0};

    dstSurface.dstSurface = destSurf->m2dSurf;
    dstSurface.dstX = dx;
    dstSurface.dstY = dy;
    dstSurface.dstWidth = dw;
    dstSurface.dstHeight = dh;

    srcSurface.srcSurface = srcSurf->m2dSurf;
    srcSurface.srcX = sx;
    srcSurface.srcY = sy;
    srcSurface.srcWidth = sw;
    srcSurface.srcHeight = sh;

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, dx, dy, dw, dh);
    }

    gfxSurfaceStrectch(&dstSurface, &srcSurface);

    if (dest->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

static void
M2dAlphaBlend(
    ITUSurface* dest,
    int dx,
    int dy,
    int w,
    int h,
    ITUSurface* src,
    int sx,
    int sy,
    uint8_t alpha)
{
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf  = (M2dSurface *)src;
    GFXSurfaceSrc srcSurface = {0};

    srcSurface.srcSurface = srcSurf->m2dSurf;
    srcSurface.srcX = sx;
    srcSurface.srcY = sy;
    srcSurface.srcWidth = w;
    srcSurface.srcHeight = h;

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, dx, dy, w, h);     
    }

    if(alpha != 0xFF)
    {
        gfxSurfaceAlphaBlendEx(
            destSurf->m2dSurf,
            dx,
            dy,
            &srcSurface,
            true,
            alpha);
    }
    else
    {
        gfxSurfaceAlphaBlendEx(
            destSurf->m2dSurf,
            dx,
            dy,
            &srcSurface,
            false,
            0xFF);
    }

    if (dest->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

static void
M2dColorFill(
    ITUSurface* surf,
    int x,
    int y,
    int w,
    int h,
    ITUColor* color)
{
    M2dSurface     *destSurf   = (M2dSurface *)surf;
    GFXColor fillColor = {0};
    GFXSurfaceDst dstSurface = {0};

    if (w == 1 && h == 1)
    {
        _SWColorFill(surf, x, y, w, h, color);
    }
    else
    {
    fillColor.a = color->alpha;
    fillColor.r = color->red;
    fillColor.g = color->green;
    fillColor.b = color->blue;

    dstSurface.dstSurface = destSurf->m2dSurf;
    dstSurface.dstX = x;
    dstSurface.dstY = y;
    dstSurface.dstWidth = w;
    dstSurface.dstHeight = h;

    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);       
    }

    //printf("surf->format:%d\n",surf->format);
    //printf("surface->width:%d,surface->height:%d,x:%d,y:%d,w:%d,h:%d\n",surface.width,surface.height,x,y,w,h);
    gfxSurfaceFillColor(
        &dstSurface,
        fillColor);

    if (surf->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}
}

static void
M2dColorFillBlend(
    ITUSurface* surf,
    int x,
    int y,
    int w,
    int h,
    ITUColor* color,
    bool enableAlpha,
    bool enableConstantAlpha,
    uint8_t constantAlphaValue)
{
    M2dSurface     *destSurf   = (M2dSurface *)surf;
    GFXColor fillColor = {0};
    GFXAlphaBlend alphaBlend = {0};
    GFXSurfaceDst dstSurface = {0};

    fillColor.a = color->alpha;
    fillColor.r = color->red;
    fillColor.g = color->green;
    fillColor.b = color->blue;

    dstSurface.dstSurface = destSurf->m2dSurf;
    dstSurface.dstX = x;
    dstSurface.dstY = y;
    dstSurface.dstWidth = w;
    dstSurface.dstHeight = h;

    alphaBlend.enableAlpha = enableAlpha;
    alphaBlend.enableConstantAlpha = enableConstantAlpha;
    alphaBlend.constantAlphaValue = constantAlphaValue;

    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);
    }

    gfxSurfaceFillColorWithBlend(
        &dstSurface,
        fillColor,
        &alphaBlend);

    if (surf->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
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
    float          scaleX,
    float          scaleY)
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
    dst_half_width_scaled  = dstWidth / 2 / scaleX;
    dst_half_height_scaled = dstHeight / 2 / scaleY;
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
    float        scaleX,
    float        scaleY)
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
    dst_half_width_scaled  = dstWidth / 2 / scaleX;
    dst_half_height_scaled = dstHeight / 2 / scaleY;
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

static void
SWRotate(
    ITUSurface *dest,
    int        dx,
    int        dy,
    ITUSurface *src,
    int        cx,
    int        cy,
    float      angle,
    float      scaleX,
    float      scaleY)
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

    switch (src->format)
    {
    case ITU_RGB565:
    case ITU_ARGB1555:
    case ITU_ARGB4444:
        {
            ITUSurface *surf = M2dCreateSurface(dstwidth, dstheight, 0, src->format, NULL, 0);
            if (surf)
            {
                uint8_t *srcPtr  = M2dLockSurface(src, 0, 0, src->width, src->height);
                uint8_t *destPtr = M2dLockSurface(surf, 0, 0, surf->width, surf->height);

                RotateImage16((unsigned short *)destPtr, (unsigned short *)srcPtr, dstwidth, dstheight, dstwidth, src->width, src->height, src->width, cx, cy, angle, scaleX, scaleY);

                M2dUnlockSurface(src);
                M2dUnlockSurface(surf);

                M2dBitBlt(dest, dx - dstwidth / 2, dy - dstheight / 2, surf->width, surf->height, surf, 0, 0);
                gfxwaitEngineIdle();

                M2dDestroySurface(surf);
            }
        }
        break;

    case ITU_ARGB8888:
        {
            ITUSurface *surf = M2dCreateSurface(dstwidth, dstheight, 0, ITU_ARGB8888, NULL, 0);
            if (surf)
            {
                uint8_t *srcPtr  = M2dLockSurface(src, 0, 0, src->width, src->height);
                uint8_t *destPtr = M2dLockSurface(surf, 0, 0, surf->width, surf->height);

                RotateImage32((unsigned int *)destPtr, (unsigned int *)srcPtr, dstwidth, dstheight, dstwidth, src->width, src->height, src->width, cx, cy, angle, scaleX, scaleY);

                M2dUnlockSurface(src);
                M2dUnlockSurface(surf);

                M2dBitBlt(dest, dx - dstwidth / 2, dy - dstheight / 2, surf->width, surf->height, surf, 0, 0);

                gfxwaitEngineIdle();

                M2dDestroySurface(surf);
            }
        }
        break;

    case ITU_MONO:  // TODO: IMPLEMENT
        break;
    }
}

static void
M2dRotate(
    ITUSurface *dest,
    int dx,
    int dy,
    ITUSurface *src,
    int cx,
    int cy,
    float angle,
    float scaleX,
    float scaleY)
{
    M2dSurface   *destSurf = (M2dSurface *)dest;
    M2dSurface   *srcSurf  = (M2dSurface *)src;    
    GFXSurfaceDst dstSurface = {0};
    GFXSurfaceSrc srcSurface = {0};
    GFXAlphaBlend alphaBlend = {0};

#if 0
    gfxwaitEngineIdle();    
    SWRotate(dest, dx, dy, src, cx, cy, angle, scaleX, scaleY);
#else
    dstSurface.dstSurface = destSurf->m2dSurf;
    dstSurface.dstX = dx;
    dstSurface.dstY = dy;
    dstSurface.dstWidth = dest->width;
    dstSurface.dstHeight = dest->height;

    srcSurface.srcSurface = srcSurf->m2dSurf;
    srcSurface.srcX = 0;
    srcSurface.srcY = 0;
    srcSurface.srcWidth = src->width;
    srcSurface.srcHeight = src->height;

    alphaBlend.enableAlpha = true;
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue = 0;

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, dest->clipping.x, dest->clipping.y, dest->width, dest->height);
    }

    gfxSurfaceTransform(
        &dstSurface,
        &srcSurface,
        cx,
        cy,
        scaleX,
        scaleY,
        angle,
        ITU_TILE_FILL,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);

    if (dest->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }

#endif        
}

static void M2dTransform(
    ITUSurface* dest, int dx, int dy, int dw, int dh,
    ITUSurface* src, int sx, int sy, int sw, int sh,
    int cx,
    int cy,
    float scaleWidth,
    float scaleHeight,
    float angle,
    ITUTileMode tilemode,
    bool enableAlpha,
    bool enableConstantAlpha,
    uint8_t constantAlphaValue)
{
    M2dSurface   *destSurf = (M2dSurface *)dest;
    M2dSurface   *srcSurf  = (M2dSurface *)src;
    GFXSurfaceDst dstSurface = {0};
    GFXSurfaceSrc srcSurface = {0};
    GFXAlphaBlend alphaBlend = {0};

    dstSurface.dstSurface = destSurf->m2dSurf;
    dstSurface.dstX = dx;
    dstSurface.dstY = dy;
    dstSurface.dstWidth = dw;
    dstSurface.dstHeight = dh;

    srcSurface.srcSurface = srcSurf->m2dSurf;
    srcSurface.srcX = sx;
    srcSurface.srcY = sy;
    srcSurface.srcWidth = sw;
    srcSurface.srcHeight = sh;

    alphaBlend.enableAlpha = enableAlpha;
    alphaBlend.enableConstantAlpha = enableConstantAlpha;
    alphaBlend.constantAlphaValue = constantAlphaValue;

    if (dest->clipping.x + dest->clipping.width >= dest->width || ((dest->flags & ITU_CLIPPING) && angle == 0))
    {
        _SetClipRectRegion(dest, dx, dy, dw, dh);
    }

    gfxSurfaceTransform(
        &dstSurface,
        &srcSurface,
        cx,
        cy,
        scaleWidth,
        scaleHeight,
        angle,
        tilemode,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);

    if (dest->clipping.x + dest->clipping.width >= dest->width || ((dest->flags & ITU_CLIPPING) && angle == 0))
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

/**
 * Fill a rectangle in the specified gradient color.
 *
 * @param destDisplay       handle to destination display context.
 * @param destX             x-coordinate of destination upper-left corner.
 * @param destY             y-coordinate of destination upper-left corner.
 * @param destWidth         width of destination rectangle.
 * @param destHeight        height of destination rectangle.
 * @param initColor         the initial color(RGB565)
 * @param endColor          the end color(RGB565)
 * @param direction         the direction of gradient fill
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */

static void
M2dGradientFill(
    ITUSurface* surf,
    int x,
    int y,
    int w,
    int h,
    ITUColor* initcolor,
    ITUColor* endcolor,
    ITUGradientMode  mode)
{
    M2dSurface   *destSurf = (M2dSurface *)surf;
    GFXColor color = {0};
    GFXColor color_end = {0};
    GFXSurfaceDst dstSurface = {0};
	
    color.a = initcolor->alpha;
    color.r = initcolor->red;
    color.g = initcolor->green;
    color.b = initcolor->blue;

    color_end.a = endcolor->alpha;
    color_end.r = endcolor->red;
    color_end.g = endcolor->green;
    color_end.b = endcolor->blue;

    dstSurface.dstSurface = destSurf->m2dSurf;
    dstSurface.dstX = x;
    dstSurface.dstY = y;
    dstSurface.dstWidth = w;
    dstSurface.dstHeight = h;

    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);
    }

    if(mode == ITU_GF_NONE)
    {
        gfxSurfaceFillColor(
            &dstSurface,
            color);
    }
    else
    {
        GFXGradDir gfxMode = GFX_GRAD_UNKNOWN;

        if(mode == ITU_GF_HORIZONTAL)
        {
            gfxMode = GFX_GRAD_H;
        }
        else if(mode == ITU_GF_VERTICAL)
        {
            gfxMode = GFX_GRAD_V;
        }
        else if(mode == ITU_GF_FORWARD_DIAGONAL || mode == ITU_GF_BACKWARD_DIAGONAL)
        {
            gfxMode = GFX_GRAD_BOTH;
        }


        if (destSurf->m2dSurf->format == GFX_COLOR_A_8)
        {
            GFXSurface* surface = dstSurface.dstSurface;
           
            surface->format = GFX_COLOR_ARGB8888;
            dstSurface.dstWidth = surface->width = (w + 3) / 4;
            gfxSurfaceGradientFill(
                &dstSurface,
                color,
                color_end,
                gfxMode);

            surface->maskSurface = (GFXMaskSurface*)surface;

            surface->maskSurface->width = w;
            surface->maskSurface->height = surface->height;
            surface->maskSurface->pitch = surface->pitch;
            surface->maskSurface->format = GFX_MASK_A_8;
            surface->maskSurface->imageData = surface->imageData;
            surface->maskSurface->imageDataLength = surface->imageDataLength;
            surface->maskSurface->imageDataOwner = surface->imageDataOwner;
        }
        else
        {
            gfxSurfaceGradientFill(
                &dstSurface,
                color,
                color_end,
                gfxMode);
        }
    }

    if (surf->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

static void
M2dGradientFillBlend(
    ITUSurface* surf,
    int x,
    int y,
    int w,
    int h,
    ITUColor* initcolor,
    ITUColor* endcolor,
    ITUGradientMode  mode,
    bool enableAlpha,
    bool enableConstantAlpha,
    uint8_t constantAlphaValue)
{
    M2dSurface   *destSurf = (M2dSurface *)surf;
    GFXColor color = {0};
    GFXColor color_end = {0};
    GFXAlphaBlend alphaBlend = {0};
    GFXSurfaceDst dstSurface = {0};

    color.a = initcolor->alpha;
    color.r = initcolor->red;
    color.g = initcolor->green;
    color.b = initcolor->blue;

    color_end.a = endcolor->alpha;
    color_end.r = endcolor->red;
    color_end.g = endcolor->green;
    color_end.b = endcolor->blue;

    dstSurface.dstSurface = destSurf->m2dSurf;
    dstSurface.dstX = x;
    dstSurface.dstY = y;
    dstSurface.dstWidth = w;
    dstSurface.dstHeight = h;

    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);
    }

    if(mode == ITU_GF_NONE)
    {
        gfxSurfaceFillColor(
            &dstSurface,
            color);
    }
    else
    {
        GFXGradDir gfxMode = GFX_GRAD_UNKNOWN;

        if(mode == ITU_GF_HORIZONTAL)
        {
            gfxMode = GFX_GRAD_H;
        }
        else if(mode == ITU_GF_VERTICAL)
        {
            gfxMode = GFX_GRAD_V;
        }
        else if(mode == ITU_GF_FORWARD_DIAGONAL || mode == ITU_GF_BACKWARD_DIAGONAL)
        {
            gfxMode = GFX_GRAD_BOTH;
        }

        alphaBlend.enableAlpha = enableAlpha;
        alphaBlend.enableConstantAlpha = enableConstantAlpha;
        alphaBlend.constantAlphaValue = constantAlphaValue;

        gfxSurfaceGradientFillWithBlend(
            &dstSurface,
            color,
            color_end,
            gfxMode,
            &alphaBlend);
    }

    if (surf->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

static void M2dHWFlip(ITUSurface *surf)
{
    ITUSurface *lcdSurf;
    M2dSurface *srcSurf = (M2dSurface *)surf;
    lcdSurf = ituGetDisplaySurface();    
    
    if (m2dRotation != ITU_ROT_0)
    {
        M2dSurface  *m2dtempSurf;
        float       angle;
        int         dX;
        int         dY;
        int         cX;
        int         cY;
    
        switch (m2dRotation)
        {
            case ITU_ROT_90:
                angle = 90.0f;
                dX = ithLcdGetWidth();
                dY = 0;
                cX = 0;
                cY = 0;
                break;
            case ITU_ROT_180:
                angle = 180.0f;    
                dX = ithLcdGetWidth();
                dY = ithLcdGetHeight();
                cX = 0;
                cY = 0;
                break;
            case ITU_ROT_270:
                angle = 270.0f;    
                dX = 0;
                dY = ithLcdGetHeight();
                cX = 0;
                cY = 0;
                break;
            default:
                angle = 0;
                break;
        }        	      
    
        m2dtempSurf = (M2dSurface *)gRoateSurf;

        if (gfxGetDispSurfIndex() == 0)
        {
            gfxSurfaceSetSurfaceBaseAddress(
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrA());
        } else if (gfxGetDispSurfIndex() == 1) {
            gfxSurfaceSetSurfaceBaseAddress(
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrB());
        } else {
            gfxSurfaceSetSurfaceBaseAddress(
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrC());
        }        

        gfxSurfaceSetQuality(
            m2dtempSurf->m2dSurf,
            GFX_QUALITY_FASTER);
        
        M2dRotate(
            m2dtempSurf,
            dX,
            dY,
            srcSurf,
            cX,
            cY,
            angle,
            1.0f,
            1.0f);
            
        gfxwaitEngineIdle();
        
        //flip
        gfxFlip();
        
        gRotateBufIdx = (gRotateBufIdx + 1) % MAX_ROTATE_BUFFER_COUNT;
        lcdSurf->addr = gRotateBuffer[gRotateBufIdx];
                             
        gfxSurfaceSetSurfaceBaseAddress(
            srcSurf->m2dSurf,
            lcdSurf->addr);        
    } 
    else 
    {
        gfxFlip();
        //printf("HWFlip %d\n", gfxGetDispSurfIndex());
        if (gfxGetDispSurfIndex() == 0)
        {
        	lcdSurf->addr = ithLcdGetBaseAddrA();
        }
        else if (gfxGetDispSurfIndex() == 1)
        {
        	lcdSurf->addr = ithLcdGetBaseAddrB();
        }
        else if (gfxGetDispSurfIndex() == 2)
        {
        	lcdSurf->addr = ithLcdGetBaseAddrC();
        }
        
        gfxSurfaceSetSurfaceBaseAddress(
            srcSurf->m2dSurf,
            lcdSurf->addr);            	               
    }
}

static void M2dSWFlip(ITUSurface *surf)
{
    ITUSurface *lcdSurf;
    M2dSurface *srcSurf = (M2dSurface *)surf;
    lcdSurf = ituGetDisplaySurface();    
    
    if (m2dRotation != ITU_ROT_0)
    {
        M2dSurface  *m2dtempSurf;
        float       angle;
        int         dX;
        int         dY;
        int         cX;
        int         cY;
        uint8_t     flipIndex;
    
        switch (m2dRotation)
        {
            case ITU_ROT_90:
                angle = 90.0f;
                dX = ithLcdGetWidth();
                dY = 0;
                cX = 0;
                cY = 0;
                break;
            case ITU_ROT_180:
                angle = 180.0f;    
                dX = ithLcdGetWidth();
                dY = ithLcdGetHeight();
                cX = 0;
                cY = 0;
                break;
            case ITU_ROT_270:
                angle = 270.0f;    
                dX = 0;
                dY = ithLcdGetHeight();
                cX = 0;
                cY = 0;
                break;
            default:
                angle = 0;
                break;
        }        	      
    
        m2dtempSurf = (M2dSurface *)gRoateSurf;
        
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        if (ithLcdGetFlip() == 0)
        {    	     
            flipIndex = 1;
            gfxSurfaceSetSurfaceBaseAddress(    	      
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrB());
        } else if (ithLcdGetFlip() == 1) {  
        	  flipIndex = 2;
            gfxSurfaceSetSurfaceBaseAddress(    	  	 
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrC());
        } else {
        	  flipIndex = 0;
            gfxSurfaceSetSurfaceBaseAddress(    	  	 
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrA());
        }  
#else
        if (ithLcdGetFlip() == 0)
        {    	     
            flipIndex = 1;
            gfxSurfaceSetSurfaceBaseAddress(    	      
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrB());       
        } else {   
        	  flipIndex = 0;
            gfxSurfaceSetSurfaceBaseAddress(    	  	 
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrA());
        }
#endif                

        gfxSurfaceSetQuality(
            m2dtempSurf->m2dSurf,
            GFX_QUALITY_FASTER);
        
        M2dRotate(
            m2dtempSurf,
            dX,
            dY,
            srcSurf,
            cX,
            cY,
            angle,
            1.0f,
            1.0f);
            
        gfxwaitEngineIdle();
        
        //flip
        //gfxFlip();
        ithLcdSwFlip(flipIndex);
        
        gRotateBufIdx = (gRotateBufIdx + 1) % MAX_ROTATE_BUFFER_COUNT;
        lcdSurf->addr = gRotateBuffer[gRotateBufIdx];
                             
        gfxSurfaceSetSurfaceBaseAddress(
            srcSurf->m2dSurf,
            lcdSurf->addr);        
    } 
    else 
    {
        gfxwaitEngineIdle(); 
        //flip
        //gfxFlip();
   
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)        
        if (lcdSurf->addr == ithLcdGetBaseAddrA())
        {
            ithLcdSwFlip(0);
            lcdSurf->addr = ithLcdGetBaseAddrB();
        }
        else if (lcdSurf->addr == ithLcdGetBaseAddrB())
        {
            ithLcdSwFlip(1);
            lcdSurf->addr = ithLcdGetBaseAddrC();
        }
        else if (lcdSurf->addr == ithLcdGetBaseAddrC())
        {
            ithLcdSwFlip(2);
            lcdSurf->addr = ithLcdGetBaseAddrA();
        }
#else
        if (lcdSurf->addr == ithLcdGetBaseAddrA())
        {    	     
            ithLcdSwFlip(0);
            while(ithLcdGetFlip() != 0)
            {
                usleep(500);
            }
            lcdSurf->addr = ithLcdGetBaseAddrB();
        } else {
            ithLcdSwFlip(1);
            while(ithLcdGetFlip() != 1)
            {
                usleep(500);
            }
            lcdSurf->addr = ithLcdGetBaseAddrA();
        }
#endif
        gfxSurfaceSetSurfaceBaseAddress(
            srcSurf->m2dSurf,
            lcdSurf->addr);        
    }
}

static void M2dProjection(ITUSurface* dest, int dx, int dy, int dw, int dh, ITUSurface* src, int sx, int sy, int sw, int sh, float scaleWidth, float scaleHeight, float degree, float FOV, float pivotX, bool enableAlpha, bool enableConstantAlpha, uint8_t constantAlphaValue)
{
    GFXSurfaceDst dstSurface = {0};
    GFXSurfaceSrc srcSurface = {0};
    GFXAlphaBlend alphaBlend = {0};

    dstSurface.dstSurface = dest;
    dstSurface.dstX = dx;
    dstSurface.dstY = dy;
    dstSurface.dstWidth = dw;
    dstSurface.dstHeight = dh;

    srcSurface.srcSurface = src;
    srcSurface.srcX = sx;
    srcSurface.srcY = sy;
    srcSurface.srcWidth = sw;
    srcSurface.srcHeight = sh;

    alphaBlend.enableAlpha = enableAlpha;
    alphaBlend.enableConstantAlpha = enableConstantAlpha;
    alphaBlend.constantAlphaValue = constantAlphaValue;

    gfxSurfaceProjection(
        &dstSurface,
        &srcSurface,
        scaleWidth,
        scaleHeight,
        degree,
        FOV,
        pivotX,
        &alphaBlend);
}

static void M2dDrawLine(ITUSurface* surf, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, ITUColor* lineColor, int32_t lineWidth)
{
#if 1
    M2dSurface *destSurf = (M2dSurface *)surf;
    GFXColor fillColor = { 0 };
    GFXAlphaBlend alphaBlend = { 0 };
    GFXSurfaceDst dstSurface = { 0 };

    fillColor.a = lineColor->alpha;
    fillColor.r = lineColor->red;
    fillColor.g = lineColor->green;
    fillColor.b = lineColor->blue;

    int dx = abs(toX - fromX), sx = fromX < toX ? 1 : -1;
    int dy = abs(toY - fromY), sy = fromY < toY ? 1 : -1;
    int err = dx - dy, e2, x2, y2;                          /* error value e_xy */
    float wd = (float)lineWidth;
    float ed = dx + dy == 0 ? 1.0f : sqrtf((float)dx*dx + (float)dy*dy);
    float alpha;

    for (wd = (wd + 1) / 2;;)
    {                                   /* pixel loop */
        alpha = 255 - MAX(0, 255 * (abs(err - dx + dy) / ed - wd + 1));

        dstSurface.dstSurface = destSurf->m2dSurf;
        dstSurface.dstX = fromX;
        dstSurface.dstY = fromY;
        dstSurface.dstWidth = 1;
        dstSurface.dstHeight = 1;

        alphaBlend.enableAlpha = true;
        alphaBlend.enableConstantAlpha = true;
        alphaBlend.constantAlphaValue = (uint8_t)alpha;

        gfxSurfaceFillColorWithBlend(
            &dstSurface,
            fillColor,
            &alphaBlend);

        e2 = err; x2 = fromX;
        if (2 * e2 >= -dx)
        {   /* x step */
            for (e2 += dy, y2 = fromY; e2 < ed*wd && (toY != y2 || dx > dy); e2 += dx)
            {
                alpha = 255 - MAX(0, 255 * (abs(e2) / ed - wd + 1));

                dstSurface.dstSurface = destSurf->m2dSurf;
                dstSurface.dstX = fromX;
                dstSurface.dstY = y2 += sy;
                dstSurface.dstWidth = 1;
                dstSurface.dstHeight = 1;

                alphaBlend.enableAlpha = true;
                alphaBlend.enableConstantAlpha = true;
                alphaBlend.constantAlphaValue = (uint8_t)alpha;

                gfxSurfaceFillColorWithBlend(
                    &dstSurface,
                    fillColor,
                    &alphaBlend);
            }

            if (fromX == toX)
                break;

            e2 = err; err -= dy; fromX += sx;
        }
        if (2 * e2 <= dy)
        {   /* y step */
            for (e2 = dx - e2; e2 < ed*wd && (toX != x2 || dx < dy); e2 += dy)
            {
                alpha = 255 - MAX(0, 255 * (abs(e2) / ed - wd + 1));

                dstSurface.dstSurface = destSurf->m2dSurf;
                dstSurface.dstX = x2 += sx;
                dstSurface.dstY = fromY;
                dstSurface.dstWidth = 1;
                dstSurface.dstHeight = 1;

                alphaBlend.enableAlpha = true;
                alphaBlend.enableConstantAlpha = true;
                alphaBlend.constantAlphaValue = (uint8_t)alpha;

                gfxSurfaceFillColorWithBlend(
                    &dstSurface,
                    fillColor,
                    &alphaBlend);
            }

            if (fromY == toY)
                break;

            err += dx; fromY += sy;
        }
    }
#else
    M2dSurface *destSurf = (M2dSurface *)surf;

    gfxSurfaceDrawLine(
        destSurf->m2dSurf,
        fromX,
        fromY,
        toX,
        toY,
        (GFXColor*)lineColor,
        lineWidth);
#endif
}

static void M2dDrawCurve(ITUSurface* surf, ITUPoint* point1, ITUPoint* point2, ITUPoint* point3, ITUPoint* point4, ITUColor* lineColor, int32_t lineWidth)
{
    M2dSurface *destSurf = (M2dSurface *)surf;

    gfxSurfaceDrawCurve(
        destSurf->m2dSurf,
        (GFXCoordinates*)point1,
        (GFXCoordinates*)point2,
        (GFXCoordinates*)point3,
        (GFXCoordinates*)point4,
        (GFXColor*)lineColor,
        lineWidth);
}

static ITUSurface* M2dGetDisplaySurface(void)
{
    return (ITUSurface*)gfxGetDisplaySurface(); 
}

static void M2dTransformBlt(
    ITUSurface* dest,
    int dx,
    int dy, 
    ITUSurface* src, 
    int sx, 
    int sy,
    int sw, 
    int sh, 
    int x0, int y0, 
    int x1, int y1, 
    int x2, int y2, 
    int x3, int y3,
    bool bInverse,
    ITUPageFlowType type,
    ITUTransformType transformType)
{
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf = (M2dSurface *)src;

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, dx, dy, dest->width, dest->height);
    }

    gfxSurfaceTransformBlt(
        destSurf->m2dSurf,
        dx,
        dy,
        srcSurf->m2dSurf,
        sx,
        sy,
        sw,
        sh,
        x0, y0,
        x1, y1,
        x2, y2,
        x3, y3,
        bInverse,
        type,
        transformType);

    if (dest->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

static void M2dReflected(
    ITUSurface* dest,
    int dx,
    int dy,
    ITUSurface* src,
    int sx,
    int sy,
    int reflectedWidth,
    int reflectedHeight,
    ITUSurface* masksurf)
{
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf = (M2dSurface *)src;
    M2dSurface *maskSurface = (M2dSurface *)masksurf;

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, dx, dy, dest->width, dest->height);
    }

    if (masksurf)
    {        
        if (maskSurface->m2dSurf->maskSurface)
        {
            gfxSetMaskSurface(destSurf->m2dSurf, maskSurface->m2dSurf->maskSurface, true);
        }            
    }

    gfxSurfaceReflected(
        destSurf->m2dSurf,
        dx,
        dy,
        srcSurf->m2dSurf,
        sx,
        sy,
        reflectedWidth,
        reflectedHeight);

    if (dest->flags & ITU_CLIPPING)
    {
        gfxSurfaceUnSetClip(destSurf->m2dSurf);
    }
}

void ituM2dInit(void)
{
    bool        gfxResult = false;
    ITUSurface *lcdSurf;
    M2dSurface *srcSurf;

    if (gM2dInit)
    	return;
    	
    gfxResult = gfxInitialize();
    lcdSurf = ituGetDisplaySurface();
    srcSurf = (M2dSurface *)lcdSurf;

    assert(gfxResult == true);
    
    //ituGetDisplaySurface  = M2dGetDisplaySurface; //add
    //ITUSurface* lcdSurf  = ituGetDisplaySurface();
    ituCreateSurface      = M2dCreateSurface;
    ituDestroySurface     = M2dDestroySurface;
    ituCreateMaskSurface  = M2dCreateMaskSurface; //add
    ituDestroyMaskSurface = M2dDestroyMaskSurface; //add
    ituSetMaskSurface     = M2dSetMaskSurface; //add

    ituLockSurface       = M2dLockSurface;
    ituUnlockSurface     = M2dUnlockSurface;
    ituDrawGlyph         = M2dDrawGlyph;
    ituDrawLine          = M2dDrawLine; //add
    ituDrawCurve         = M2dDrawCurve; //add

    ituBitBlt            = M2dBitBlt;
    ituStretchBlt        = M2dStretchBlt;  //add
    ituAlphaBlend        = M2dAlphaBlend;
    ituColorFill         = M2dColorFill;
    ituColorFillBlend    = M2dColorFillBlend; //add
    
    //A0: SWFlip, A1: HWFlip
    if (ithGetRevisionId() == 0)
        ituFlip          = M2dSWFlip;
    else
    	ituFlip          = M2dHWFlip;

    ituSetRotation       = M2dSetRotation;
    ituRotate            = M2dRotate;
    ituTransform         = M2dTransform;

    ituGradientFill      = M2dGradientFill;
    ituGradientFillBlend = M2dGradientFillBlend; //add

    ituProjection        = M2dProjection; //add
    ituTransformBlt      = M2dTransformBlt; //add
    ituReflected         = M2dReflected;

    m2dRotation          = ITU_ROT_0;
    //m2dLcdAddr           = lcdSurf->addr;

#if defined(CFG_LCD_TRIPLE_BUFFER)    
#if (CFG_CHIP_FAMILY == 9920)    
    ithCmdQSetTripleBuffer(ITH_CMDQ0_OFFSET);
#else
    ithCmdQSetTripleBuffer();
#endif  
#endif    
    
    if (ithGetRevisionId() > 0)
    {
    	if (gfxGetDispSurfIndex() == 0)
        {
            lcdSurf->addr = ithLcdGetBaseAddrA();
        }
        else if (gfxGetDispSurfIndex() == 1)
        {
            lcdSurf->addr = ithLcdGetBaseAddrB();
        }
        else if (gfxGetDispSurfIndex() == 2)
        {
            lcdSurf->addr = ithLcdGetBaseAddrC();
        }    	
    }
    else
    {
    	ithLcdDisableHwFlip();
    	
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        if (ithLcdGetFlip() == 0)
        {
            lcdSurf->addr = ithLcdGetBaseAddrB();
        }
        else if (ithLcdGetFlip() == 1)
        {
            lcdSurf->addr = ithLcdGetBaseAddrC();
        }
        else if (ithLcdGetFlip() == 2)
        {
            lcdSurf->addr = ithLcdGetBaseAddrA();
        }
#else
        if (ithLcdGetFlip() == 0)
        {
            lcdSurf->addr = ithLcdGetBaseAddrB();
        }
        else if (ithLcdGetFlip() == 1)
        {
            lcdSurf->addr = ithLcdGetBaseAddrA();
        }
#endif
    }
    
    gfxSurfaceSetSurfaceBaseAddress(
        srcSurf->m2dSurf,
        lcdSurf->addr);   
        
    gM2dInit = true;
}

void ituM2dExit(void)
{
    // TODO: IMPLEMENT
    ITUSurface *lcdSurf = ituGetDisplaySurface();
    //M2dSurface *srcSurf = (M2dSurface *)lcdSurf;

    //printf("lcdSurf->addr:0x%x  GetA:0x%x GetB:0x%x GetC:0x%x\n",lcdSurf->addr,ithLcdGetBaseAddrA(),ithLcdGetBaseAddrB(),ithLcdGetBaseAddrC());

    gfxwaitEngineIdle();

    ithLcdDisableHwFlip();
    
    if (lcdSurf->addr != ithLcdGetBaseAddrA())
        lcdSurf->addr = ithLcdGetBaseAddrA();
    else
    {
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        ithLcdSwFlip(0);
#else
        ithLcdSwFlip(0);
        while(ithLcdGetFlip() != 0)
        {
            usleep(500);
        }
#endif
    }
    //gfxSurfaceSetSurfaceBaseAddress(
    //    srcSurf->m2dSurf,
    //    lcdSurf->addr);

    //printf("lcdSurf->addr:0x%x\n",lcdSurf->addr);
    gfxTerminate();
    
    if (gRotateBuffer[0])
    {
    	  itpVmemFree(gRotateBuffer[0]);
    	  gRotateBuffer[0] = NULL;
    }
    gM2dInit = false;
}
