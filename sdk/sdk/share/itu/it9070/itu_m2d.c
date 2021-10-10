#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include "itu_cfg.h"
#include "ite/itp.h"
#include "ite/itu.h"
#include "ite/ite_m2d.h"
#include "itu_private.h"

#define MAX_ROTATE_BUFFER_COUNT 2

typedef struct
{
    ITUSurface      surf;
    MMP_M2D_SURFACE m2dSurf;
} M2dSurface;

#define SMTK_RGB565(r, g, b) \
    (((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11))

static ITURotation m2dRotation;
static uint32_t    m2dLcdAddr;
static bool        m2dInited;

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
    MMP_M2D_RECT rect;

    rect.left   = surf->clipping.x;
    rect.top    = surf->clipping.y;
    rect.right  = (surf->clipping.x + surf->clipping.width - 1);
    rect.bottom = (surf->clipping.y + surf->clipping.height - 1);

    if (rect.right >= (x + w))
        rect.right = x + w - 1;
    if (rect.bottom >= (y + h))
        rect.bottom = y + h - 1;
    if (rect.left < x)
        rect.left = x;
    if (rect.top < y)
        rect.top = y;

    mmpM2dSetClipRectRegion(
        ((M2dSurface *)surf)->m2dSurf,
        &rect,
        1);
}

static ITUSurface *
M2dCreateSurface(
    int            w,
    int            h,
    int            pitch,
    ITUPixelFormat format,
    const uint8_t  *bitmap,
    unsigned int   flags)
{
    M2dSurface *surface = calloc(1, sizeof(M2dSurface));
    MMP_RESULT result;

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

    if (bitmap)
    {
#ifndef WIN32
        surface->surf.addr = (uint32_t) bitmap;
#else
        uint8_t      *ptr;
        unsigned int size = surface->surf.pitch * h;

        surface->surf.addr = itpVmemAlloc(size);
        if (!surface->surf.addr)
        {
            LOG_ERR "Out of VRAM: %d\n", size LOG_END
            free(surface);
            return NULL;
        }

        ptr = ithMapVram(surface->surf.addr, size, ITH_VRAM_WRITE);
        memcpy(ptr, bitmap, size);
        ithUnmapVram(ptr, size);
        if (!(flags & ITU_STATIC))
            free(bitmap);
#endif
    }
    else
    {
        uint8_t      *ptr;
        unsigned int size = surface->surf.pitch * h;

        surface->surf.addr = itpVmemAlloc(size);
        if (!surface->surf.addr)
        {
            LOG_ERR "Out of VRAM: %d\n", size LOG_END
            free(surface);
            return NULL;
        }
    }

    {
        int m2dformat = MMP_M2D_IMAGE_FORMAT_RGB565;
        switch (surface->surf.format)
        {
        case ITU_RGB565:
            m2dformat = MMP_M2D_IMAGE_FORMAT_RGB565;
            break;

        case ITU_ARGB1555:
            m2dformat = MMP_M2D_IMAGE_FORMAT_ARGB1555;
            break;

        case ITU_ARGB4444:
            m2dformat = MMP_M2D_IMAGE_FORMAT_ARGB4444;
            break;

        case ITU_ARGB8888:
            m2dformat = MMP_M2D_IMAGE_FORMAT_ARGB8888;
            break;
        }
        //create m2d surface
        result = mmpM2dCreateVirtualSurface(w,
                                            h,
                                            m2dformat,
                                            surface->surf.addr,
                                            &surface->m2dSurf);
        if (MMP_SUCCESS != result)
        {
            LOG_ERR "mmpM2dCreateVirtualSurface failed: %d\n", result LOG_END

            if (!(flags & ITU_STATIC))
                itpVmemFree(surface->surf.addr);

            free(surface);
            return NULL;
        }
    }
    //printf("M2dCreateSurface 0x%X\n",surface);
    return (ITUSurface *)surface;
}

static void M2dDestroySurface(ITUSurface *surf)
{
    M2dSurface *m2dSurf = (M2dSurface *)surf;

    //printf("M2dDestroySurface 0x%X\n",surf);

    if (m2dSurf->m2dSurf)
    {
        mmpM2dDeleteVirtualSurface(m2dSurf->m2dSurf);
    }

    if (surf && !(surf->flags & ITU_STATIC))
    {
        itpVmemFree(surf->addr);
    }
    free(surf);
    //printf("M2dDestroySurface out\n");
}

static uint8_t *M2dLockSurface(ITUSurface *surf, int x, int y, int w, int h)
{
    uint32_t vram_addr;
    surf->lockSize = surf->pitch * h;

    vram_addr      = surf->addr + surf->pitch * y + x * ituFormat2Bpp(surf->format) / 8;
#ifndef SPEED_UP
    surf->lockAddr = ithMapVram(vram_addr, surf->lockSize, ITH_VRAM_READ | ITH_VRAM_WRITE);
#else
    surf->lockAddr = ithMapVram(vram_addr, surf->lockSize, /*ITH_VRAM_READ |*/ ITH_VRAM_WRITE);
#endif
    return surf->lockAddr;
}

static void M2dUnlockSurface(ITUSurface *surf)
{
#ifndef SPEED_UP
    ithUnmapVram(surf->lockAddr, surf->lockSize);
#endif
}

static void M2dSetRotation(ITURotation rot)
{
    ITUSurface *lcdSurf;
    M2dSurface *screenSurf;

    if (m2dRotation == rot)
        return;

    lcdSurf    = ituGetDisplaySurface();
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
    if (!gRotateBuffer[0])
    {
        int i;
        int size = lcdSurf->pitch * lcdSurf->height * 2;

        gRotateBuffer[0] = itpVmemAlloc(size);

        assert(gRotateBuffer[0]);

        for (i = 1; i < MAX_ROTATE_BUFFER_COUNT; ++i)
        {
            gRotateBuffer[i] = gRotateBuffer[i - 1] + lcdSurf->pitch * lcdSurf->height;
        }

        lcdSurf->addr = screenSurf->surf.addr = gRotateBuffer[0];

        mmpM2dSetSurfaceBaseAddress(
            screenSurf->m2dSurf,
            screenSurf->surf.addr);

        gRoateSurf = M2dCreateSurface(ithLcdGetWidth(), ithLcdGetHeight(), ithLcdGetPitch(), ITU_RGB565, ithLcdGetBaseAddrA(), ITU_STATIC);
        mmpM2dSetSurfaceBaseWidth(screenSurf->m2dSurf, lcdSurf->width);
        mmpM2dSetSurfaceBaseHeight(screenSurf->m2dSurf, lcdSurf->height);
        mmpM2dSetSurfaceBasePitch(screenSurf->m2dSurf, lcdSurf->pitch);
    }
}

static void
M2dDrawGlyph(
    ITUSurface     *surf,
    int            x,
    int            y,
    ITUGlyphFormat format,
    const uint8_t  *bitmap,
    int            w,
    int            h)
{
    M2dSurface *m2dSurf = (M2dSurface *)surf;
    int        pitch    = 0;
    int        result   = 0;

    if (surf == NULL || bitmap == NULL || w == 0 || h == 0 || surf->width <= 0 || surf->height <= 0)
    {
        printf("M2dDrawGlyph error handle[null source]\n");
        goto end;
    }

    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);
    }

    mmpM2dSetFontColor(m2dSurf->m2dSurf, SMTK_RGB565(surf->fgColor.red, surf->fgColor.green, surf->fgColor.blue));

    if (format == ITU_GLYPH_1BPP)
    {
        pitch = (w + 7) / 8;
        mmpM2dDrawBmpTextTtx2(
            m2dSurf->m2dSurf,
            x,
            y,
            bitmap,
            w,
            h,
            pitch);
    }
    else if (format == ITU_GLYPH_4BPP)
    {
        mmpM2dDrawAABmpTtx2(
            m2dSurf->m2dSurf,
            bitmap,
            x,
            y,
            w,
            h);
    }
    else if (format == ITU_GLYPH_8BPP)
    {
        mmpM2dDrawAABmpTtx3(
            m2dSurf->m2dSurf,
            bitmap,
            x,
            y,
            w,
            h);
    }

    if (surf->flags & ITU_CLIPPING)
    {
        mmpM2dResetClipRegion(m2dSurf->m2dSurf);
    }

end:
    mmpM2dWaitIdle(); // FIXME: WORKAROUND TEXT WITH GARBAGE BUG
    //printf("M2dDrawGlyph\n");
    result = -1;
}

static void
M2dBitBlt(
    ITUSurface *dest,
    int        dx,
    int        dy,
    int        w,
    int        h,
    ITUSurface *src,
    int        sx,
    int        sy)
{
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf  = (M2dSurface *)src;
    int        result    = 0;
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

    if (dest == NULL || src == NULL || w == 0 || h == 0 || dest->width <= 0 || dest->height <= 0 || src->width <= 0 || src->height <= 0)
    {
        printf("M2dBitBlt error handle[null source]\n");
        goto end;
    }

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, destX, destY, width, height);
    }

    mmpM2dAlphaBlend(
        destSurf->m2dSurf,
        destX,
        destY,
        width,
        height,
        srcSurf->m2dSurf,
        srcX,
        srcY,
        MMP_M2D_ABLEND_FORMAT_SRC_ALPHA,
        0xFF);

    if (dest->flags & ITU_CLIPPING)
    {
        mmpM2dResetClipRegion(destSurf->m2dSurf);
    }

end:
    //printf("M2dBitBlt\n");
    result = -1;
}

static void
M2dStretchBlt(
    ITUSurface *dest,
    int        dx,
    int        dy,
    int        dw,
    int        dh,
    ITUSurface *src,
    int        sx,
    int        sy,
    int        sw,
    int        sh)
{
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf  = (M2dSurface *)src;
    int        result    = 0;

    if (dest == NULL || src == NULL)
    {
        printf("M2dStretchBlt error handle[null source]\n");
        goto end;
    }

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, dx, dy, dw, dh);
    }

    mmpM2dStretchSrcCopy(
        destSurf->m2dSurf,
        dx,
        dy,
        dw,
        dh,
        srcSurf->m2dSurf,
        sx,
        sy,
        sw,
        sh);

    if (dest->flags & ITU_CLIPPING)
    {
        mmpM2dResetClipRegion(destSurf->m2dSurf);
    }

end:
    //printf("M2dStretchBlt\n");
    result = -1;
}

static void
M2dAlphaBlend(
    ITUSurface *dest,
    int        dx,
    int        dy,
    int        w,
    int        h,
    ITUSurface *src,
    int        sx,
    int        sy,
    uint8_t    alpha)
{
    M2dSurface *destSurf = (M2dSurface *)dest;
    M2dSurface *srcSurf  = (M2dSurface *)src;
    int        result    = 0;

    if (dest == NULL || src == NULL || w == 0 || h == 0 || dest->width <= 0 || dest->height <= 0 || src->width <= 0 || src->height <= 0)
    {
        printf("M2dAlphaBlend error handle[null source]\n");
        goto end;
    }

    if (alpha == 0)
    {
        printf("const alpha 0\n");
        goto end;
    }

    if (dest->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(dest, dx, dy, w, h);
    }

    if (alpha != 0xFF)
    {
        ITUSurface *tempSurf;
        M2dSurface *m2dtempSurf;
        uint8_t    *tempbuffer;
        tempbuffer  = malloc(src->width * src->height * (ituFormat2Bpp(dest->format) / 8) + 10);
        tempSurf    = M2dCreateSurface(src->width, src->height, 0, dest->format, tempbuffer, 0);
        m2dtempSurf = (M2dSurface *)tempSurf;

        mmpM2dStretchSrcCopy(
            m2dtempSurf->m2dSurf,
            0,
            0,
            w,
            h,
            destSurf->m2dSurf,
            dx,
            dy,
            w,
            h);

        mmpM2dAlphaBlend(
            m2dtempSurf->m2dSurf,
            0,
            0,
            src->width,
            src->height,
            srcSurf->m2dSurf,
            0,
            0,
            MMP_M2D_ABLEND_FORMAT_SRC_ALPHA,
            0xFF);

        mmpM2dAlphaBlend(
            destSurf->m2dSurf,
            dx,
            dy,
            w,
            h,
            m2dtempSurf->m2dSurf,
            sx,
            sy,
            MMP_M2D_ABLEND_FORMAT_SRC_CONST,
            alpha);

        mmpM2dWaitIdle();

        M2dDestroySurface(tempSurf);
    }
    else
    {
        mmpM2dAlphaBlend(
            destSurf->m2dSurf,
            dx,
            dy,
            w,
            h,
            srcSurf->m2dSurf,
            sx,
            sy,
            MMP_M2D_ABLEND_FORMAT_SRC_ALPHA,
            0xFF);
    }

    if (dest->flags & ITU_CLIPPING)
    {
        mmpM2dResetClipRegion(destSurf->m2dSurf);
    }

end:
    //printf("M2dAlphaBlend\n");
    result = -1;
}

static void
M2dColorFill(
    ITUSurface *surf,
    int        x,
    int        y,
    int        w,
    int        h,
    ITUColor   *color)
{
    M2dSurface     *m2dSurf   = (M2dSurface *)surf;
    MMP_M2D_HANDLE solidBrush = MMP_NULL;
    MMP_M2D_HANDLE prevBrush  = MMP_NULL;
    int            result     = 0;

    if (surf == NULL || w == 0 || h == 0 || surf->width <= 0 || surf->height <= 0)
    {
        printf("M2dColorFill error handle[null source]\n");
        goto end;
    }

    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);
    }

    //printf("mmpM2dCreateSolidBrush\n");

    mmpM2dCreateSolidBrush(SMTK_RGB565(color->red, color->green, color->blue), &solidBrush);

    mmpM2dSelectObject(
        MMP_M2D_BRUSHOBJ,
        m2dSurf->m2dSurf,
        solidBrush,
        &prevBrush);

    mmpM2dTransferBlock(
        m2dSurf->m2dSurf,
        x, y, w, h,
        MMP_NULL,
        0, 0,
        0xF0);

    mmpM2dSelectObject(
        MMP_M2D_BRUSHOBJ,
        m2dSurf->m2dSurf,
        prevBrush,
        &solidBrush);

    //printf("mmpM2dDeleteObject\n");

    mmpM2dDeleteObject(MMP_M2D_BRUSHOBJ, solidBrush);

    //printf("mmpM2dDeleteObject out\n");

    if (surf->flags & ITU_CLIPPING)
    {
        mmpM2dResetClipRegion(m2dSurf->m2dSurf);
    }

end:
    //printf("M2dColorFill\n");
    result = -1;
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

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

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

                mmpM2dWaitIdle();

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

                mmpM2dWaitIdle();

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
    int        dx,
    int        dy,
    ITUSurface *src,
    int        cx,
    int        cy,
    float      angle,
    float      scaleX,
    float      scaleY)
{
#if 0
    M2dSurface   *destSurf = (M2dSurface *)dest;
    M2dSurface   *srcSurf  = (M2dSurface *)src;
    MMP_M2D_RECT rect      = {0};
    int          result    = 0;

    if (dest == NULL || src == NULL || dest->width <= 0 || dest->height <= 0 || src->width <= 0 || src->height <= 0)
    {
        printf("M2dRotate error handle[null source]\n");
        goto end;
    }

    //printf("destx = %d,desty = %d,destw = %d,desth = %d\n",dx,dy,dest->width,dest->height);
    //printf("cx = %d,cy = %d,srcw = %d,srch = %d\n",cx,cy,src->width,src->height);

    /*
       if (dest->flags & ITU_CLIPPING)
       {

       rect.left = dest->clipping.x;
       rect.top = dest->clipping.y;
       rect.right = (dest->clipping.x + dest->clipping.width -1);
       rect.bottom = (dest->clipping.y + dest->clipping.height -1);



       if(rect.right >= dest->width)
       rect.right = dest->width -1;


       if(rect.bottom >= dest->height)
       rect.bottom = dest->height -1;

       if(rect.left < dx)
       rect.left = dx;

       if(rect.top < dy)
       rect.top = dy;


       mmpM2dSetClipRectRegion(
       destSurf->m2dSurf,
       &rect,
       1);
       }*/

    mmpM2dTransformations(
        destSurf->m2dSurf,
        dx,
        dy,
        srcSurf->m2dSurf,
        cx,
        cy,
        angle,
        scaleX);

    /*  
    if (dest->flags & ITU_CLIPPING)
    {
       mmpM2dSetClipRectRegion(
       destSurf->m2dSurf,
       &rect,
       0);
    }
    */

end:
    //printf("M2dRotate\n");
    result = -1;
#else
    mmpM2dWaitIdle();
    SWRotate(dest, dx, dy, src, cx, cy, angle, scaleX, scaleY);
#endif
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
    ITUSurface      *surf,
    int             x,
    int             y,
    int             w,
    int             h,
    ITUColor        *initcolor,
    ITUColor        *endcolor,
    ITUGradientMode mode)
{
    M2dSurface   *m2dSurf = (M2dSurface *)surf;
    int          result   = 0;

    if (surf == NULL || w == 0 || h == 0 || surf->width <= 0 || surf->height <= 0)
    {
        printf("M2dGradientFill error handle[null source]\n");
        goto end;
    }
    if (surf->flags & ITU_CLIPPING)
    {
        _SetClipRectRegion(surf, x, y, w, h);
    }

    if (mode == 0)
    {
        M2dColorFill(surf, x, y, w, h, initcolor);
    }
    else
    {
        mmpM2dGradientFill(
            m2dSurf->m2dSurf,
            x, y, w, h,
            SMTK_RGB565(initcolor->red, initcolor->green, initcolor->blue),
            SMTK_RGB565(endcolor->red, endcolor->green, endcolor->blue),
            mode - 1);
    }

    if (surf->flags & ITU_CLIPPING)
    {
        mmpM2dResetClipRegion(m2dSurf->m2dSurf);
    }

end:
    //printf("M2dGradientFill\n");
    result = -1;
}

static void M2dFlip(ITUSurface *surf)
{
    ITUSurface *lcdSurf;
    M2dSurface *srcSurf = (M2dSurface *)surf;
    lcdSurf = ituGetDisplaySurface();
    //printf("M2dFlip %x %x %x %x\n", srcSurf->m2dSurf, ithLcdGetBaseAddrA(), ithLcdGetBaseAddrB(), mmpM2dGetSurfaceBaseAddress(srcSurf->m2dSurf));

    if (m2dRotation != ITU_ROT_0)
    {
        M2dSurface *m2dtempSurf;
        float      angle;
        MMP_INT    dX;
        MMP_INT    dY;
        MMP_INT    cX;
        MMP_INT    cY;

        switch (m2dRotation)
        {
        case ITU_ROT_90:
            angle = 90.0f;
            dX    = ithLcdGetWidth() - 1;
            dY    = 0;
            cX    = 0;
            cY    = 0;
            break;

        case ITU_ROT_270:
            angle = 270.0f;
            dX    = 0;
            dY    = 0;
            cX    = ithLcdGetHeight() - 1;
            cY    = 0;
            break;

        default:
            angle = 0;
            break;
        }

        m2dtempSurf = (M2dSurface *)gRoateSurf;

        if (ithLcdGetFlip() == 1)
        {
            mmpM2dSetSurfaceBaseAddress(
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrA());
        }
        else
        {
            mmpM2dSetSurfaceBaseAddress(
                m2dtempSurf->m2dSurf,
                ithLcdGetBaseAddrB());
        }

        mmpM2dTransformations(
            m2dtempSurf->m2dSurf,
            dX,
            dY,
            srcSurf->m2dSurf,
            cX,
            cY,
            angle,
            1.0f);

        //flip
        mmpM2dWaitIdle();
        if (ithLcdGetFlip() == 1)
        {
            mmpM2dSetVisibleLCD(MMP_M2D_LCDA);
            gRotateBufIdx      = (gRotateBufIdx + 1) % MAX_ROTATE_BUFFER_COUNT;
            srcSurf->surf.addr = lcdSurf->addr = gRotateBuffer[gRotateBufIdx];
        }
        else
        {
            mmpM2dSetVisibleLCD(MMP_M2D_LCDB);
            gRotateBufIdx      = (gRotateBufIdx + 1) % MAX_ROTATE_BUFFER_COUNT;
            srcSurf->surf.addr = lcdSurf->addr = gRotateBuffer[gRotateBufIdx];
        }

        mmpM2dSetSurfaceBaseAddress(
            srcSurf->m2dSurf,
            lcdSurf->addr);
    }
    else
    {
        //flip
        mmpM2dWaitIdle();
        if (lcdSurf->addr == ithLcdGetBaseAddrA())
        {
            mmpM2dSetVisibleLCD(MMP_M2D_LCDA);
            lcdSurf->addr = ithLcdGetBaseAddrB();
        }
        else
        {
            mmpM2dSetVisibleLCD(MMP_M2D_LCDB);
            lcdSurf->addr = ithLcdGetBaseAddrA();
        }

        mmpM2dSetSurfaceBaseAddress(
            srcSurf->m2dSurf,
            lcdSurf->addr);
    }
}

void ituM2dInit(void)
{
    ITUSurface *lcdSurf = ituGetDisplaySurface();
    ituCreateSurface  = M2dCreateSurface;
    ituDestroySurface = M2dDestroySurface;

    ituLockSurface    = M2dLockSurface;
    ituUnlockSurface  = M2dUnlockSurface;
    ituDrawGlyph      = M2dDrawGlyph;

    ituBitBlt         = M2dBitBlt;
    ituStretchBlt     = M2dStretchBlt;
    ituAlphaBlend     = M2dAlphaBlend;
    ituColorFill      = M2dColorFill;
    ituFlip           = M2dFlip;

    ituSetRotation    = M2dSetRotation;
    ituRotate         = M2dRotate;

    ituGradientFill   = M2dGradientFill;

    m2dRotation       = ITU_ROT_0;
    m2dLcdAddr        = lcdSurf->addr;

    //todo : m2d initial & cmq initial
    if (!m2dInited)
    {
        mmpM2dSurfaceInitialize();
        m2dInited = true;
    }
}

void ituM2dExit(void)
{
    // TODO: IMPLEMENT
    ITUSurface *lcdSurf = ituGetDisplaySurface();
    M2dSurface *srcSurf = (M2dSurface *)lcdSurf;

    mmpM2dWaitIdle();

    if (lcdSurf->addr != ithLcdGetBaseAddrA())
    {
        mmpM2dSetVisibleLCD(MMP_M2D_LCDB);
        lcdSurf->addr = ithLcdGetBaseAddrA();
    }
    else
    {
        ithLcdSwFlip(0);
    }

    mmpM2dSetSurfaceBaseAddress(
        srcSurf->m2dSurf,
        lcdSurf->addr);

    mmpM2dTerminate();
}
