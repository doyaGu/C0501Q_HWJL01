/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file surface.c
 *  GFX application layer API function file.
 *
 * @author Awin Huang
 * @version 1.0
 * @date 2014-05-16
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "ite/itp.h"
#include "gfx.h"
#include "driver.h"
#include "gfx_math.h"
#include "gfx_mem.h"
#include "msg.h"

//=============================================================================
//                              Extern Reference
//=============================================================================
//extern GFX_DRIVER* gfxGetDriver();

//=============================================================================
//                              Macro Definition
//=============================================================================
#undef MIN
#undef MAX
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define REG_FILL_SRC_SURFACE(regs, srcSurface, srcX, srcY, srcWidth, srcHeight) \
    {   \
        regs->regSRC.data   = (uint32_t)srcSurface->imageData;    \
        regs->regSPR.data   = srcSurface->pitch;  \
        regs->regSRCXY.data = (((srcX) & GFX_REG_SRCXY_SRCX_MASK) << GFX_REG_SRCXY_SRCX_OFFSET) | (((srcY) & GFX_REG_SRCXY_SRCY_MASK) << GFX_REG_SRCXY_SRCY_OFFSET);    \
        regs->regSHWR.data  = (((srcWidth) & GFX_REG_SHWR_SRCWIDTH_MASK) << GFX_REG_SHWR_SRCWIDTH_OFFSET) | (((srcHeight) & GFX_REG_SHWR_SRCHEIGHT_MASK) << GFX_REG_SHWR_SRCHEIGHT_OFFSET);  \
        regs->regCR2.data  &= ~GFX_REG_CR2_SRCFORMAT_MASK;   \
        regs->regCR2.data  |= (srcSurface->format << GFX_REG_CR2_SRCFORMAT_OFFSET);  \
    }

#define REG_FILL_DST_SURFACE(regs, dstSurface, dstX, dstY, dstWidth, dstHeight) \
    {   \
        regs->regDST.data   = (uint32_t)dstSurface->imageData;    \
        regs->regDPR.data   = dstSurface->pitch;  \
        regs->regDSTXY.data = (((dstX) & GFX_REG_SRCXY_SRCX_MASK) << GFX_REG_SRCXY_SRCX_OFFSET) | (((dstY) & GFX_REG_SRCXY_SRCY_MASK) << GFX_REG_SRCXY_SRCY_OFFSET);    \
        regs->regDHWR.data  = (((dstWidth) & GFX_REG_SHWR_SRCWIDTH_MASK) << GFX_REG_SHWR_SRCWIDTH_OFFSET) | (((dstHeight) & GFX_REG_SHWR_SRCHEIGHT_MASK) << GFX_REG_SHWR_SRCHEIGHT_OFFSET);  \
        regs->regCR2.data  &= ~GFX_REG_CR2_DSTFORMAT_MASK;   \
        regs->regCR2.data  |= (dstSurface->format << GFX_REG_CR2_DSTFORMAT_OFFSET);  \
    }

#define REG_FILL_MASK_SURFACE(regs, dstSurface, maskX, maskY, maskWidth, maskHeight) \
    {   \
        if (dstSurface->maskEnable && dstSurface->maskSurface) \
        {   \
            regs->regMASK.data   = (uint32_t)dstSurface->maskSurface->imageData; \
            regs->regMPR.data    = dstSurface->maskSurface->pitch; \
            regs->regMASKXY.data = (((maskX) & GFX_REG_MASKXY_MASKX_MASK) << GFX_REG_MASKXY_MASKX_OFFSET) | (((maskY) & GFX_REG_MASKXY_MASKY_MASK) << GFX_REG_MASKXY_MASKY_OFFSET);    \
            regs->regMHWR.data   = (((maskWidth) & GFX_REG_MHWR_MASKWIDTH_MASK) << GFX_REG_MHWR_MASKWIDTH_OFFSET) | (((maskHeight) & GFX_REG_MHWR_MASKHEIGHT_MASK) << GFX_REG_MHWR_MASKHEIGHT_OFFSET);  \
            regs->regCR2.data   &= ~GFX_REG_CR2_MSKFORMAT_MASK; \
            regs->regCR2.data   |= (dstSurface->maskSurface->format << GFX_REG_CR2_MSKFORMAT_OFFSET); \
            regs->regCR1.data   |= GFX_REG_CR1_MASK_EN; \
        }   \
    }

#define REG_FILL_DST_CLIP(regs, clipSX, clipSY, clipEX, clipEY) \
    {   \
        regs->regPXCR.data = (((clipSX) & GFX_REG_PXCR_CLIPXSTART_MASK) << GFX_REG_PXCR_CLIPXSTART_OFFSET) | (((clipEX) & GFX_REG_PXCR_CLIPXEND_MASK) << GFX_REG_PXCR_CLIPXEND_OFFSET); \
        regs->regPYCR.data = (((clipSY) & GFX_REG_PYCR_CLIPYSTART_MASK) << GFX_REG_PYCR_CLIPYSTART_OFFSET) | (((clipEY) & GFX_REG_PYCR_CLIPYEND_MASK) << GFX_REG_PYCR_CLIPYEND_OFFSET); \
    }

#if CFG_CHIP_FAMILY == 9850
#define REG_FILL_SET_ITM(regs, mtx) \
    {   \
        regs->regITMR00.data = (int32_t)(mtx.m[0][0] * (1 << 16) + 1); \
        regs->regITMR01.data = (int32_t)(mtx.m[0][1] * (1 << 16) + 1); \
        regs->regITMR02.data = (int32_t)(mtx.m[0][2] * (1 << 16) + 1); \
        regs->regITMR10.data = (int32_t)(mtx.m[1][0] * (1 << 16) + 1); \
        regs->regITMR11.data = (int32_t)(mtx.m[1][1] * (1 << 16) + 1); \
        regs->regITMR12.data = (int32_t)(mtx.m[1][2] * (1 << 16) + 1); \
        regs->regITMR20.data = (int32_t)(mtx.m[2][0] * (1 << 16) + 1); \
        regs->regITMR21.data = (int32_t)(mtx.m[2][1] * (1 << 16) + 1); \
        regs->regITMR22.data = (int32_t)(mtx.m[2][2] * (1 << 16) + 1); \
    }
#else
#define REG_FILL_SET_ITM(regs, mtx) \
    {   \
        regs->regITMR00.data = (int32_t)(mtx.m[0][0] * (1 << 19)); \
        regs->regITMR01.data = (int32_t)(mtx.m[0][1] * (1 << 19)); \
        regs->regITMR02.data = (int32_t)(mtx.m[0][2] * (1 << 19)); \
        regs->regITMR10.data = (int32_t)(mtx.m[1][0] * (1 << 19)); \
        regs->regITMR11.data = (int32_t)(mtx.m[1][1] * (1 << 19)); \
        regs->regITMR12.data = (int32_t)(mtx.m[1][2] * (1 << 19)); \
        regs->regITMR20.data = (int32_t)(mtx.m[2][0] * (1 << 19)); \
        regs->regITMR21.data = (int32_t)(mtx.m[2][1] * (1 << 19)); \
        regs->regITMR22.data = (int32_t)(mtx.m[2][2] * (1 << 19)); \
    }
#endif

#define REG_FILL_FGCOLOR(regs, color)           \
    {                                               \
        regs->regFGCOLOR.data =                     \
            (color.a << GFX_REG_FGCOLOR_A_OFFSET) \
            | (color.r << GFX_REG_FGCOLOR_R_OFFSET) \
            | (color.g << GFX_REG_FGCOLOR_G_OFFSET) \
            | (color.b << GFX_REG_FGCOLOR_B_OFFSET); \
    }

#define REG_FILL_BGCOLOR(regs, color)           \
    {                                               \
        regs->regBGCOLOR.data =                     \
            (color.a << GFX_REG_BGCOLOR_A_OFFSET) \
            | (color.r << GFX_REG_BGCOLOR_R_OFFSET) \
            | (color.g << GFX_REG_BGCOLOR_G_OFFSET) \
            | (color.b << GFX_REG_BGCOLOR_B_OFFSET); \
    }

#define REG_FILL_ROP3(regs, ropValue)                   \
    {                                                       \
        regs->regCAR.data &= ~GFX_REG_CAR_ROP_ROP3_MASK;    \
        regs->regCAR.data |= ropValue;                      \
    }

#define REG_FILL_BLENDING(regs, enableAlpha, enableConstantAlpha, constantAlphaValue)   \
    {                                                           \
        if (enableAlpha == true) {                              \
            regs->regCR1.data |= GFX_REG_CR1_ALPHABLEND_EN;     \
        }                                                       \
        if (enableConstantAlpha == true) {                      \
            regs->regCR1.data |= GFX_REG_CR1_ALPHABLEND_EN;     \
            regs->regCR1.data |= GFX_REG_CR1_CONSTALPHA_EN;     \
            regs->regCAR.data |= (constantAlphaValue & GFX_REG_CAR_CONSTALPHA_MASK) << GFX_REG_CAR_CONSTALPHA_OFFSET;    \
        }                                                       \
    }

#define REG_FILL_LINEDRAW_STARTXY(x, y) \
    {                                                           \
        regs->regSRCXY.data = (((x) & GFX_REG_SRCXY_SRCX_MASK) << GFX_REG_SRCXY_SRCX_OFFSET) | ((y) & GFX_REG_SRCXY_SRCY_MASK); \
    }

#define REG_FILL_LINEDRAW_ENDXY(x, y) \
    {                                                           \
        regs->regLNER.data = (((x) & GFX_REG_LNEXY_LNEX_MASK) << GFX_REG_LNEXY_LNEX_OFFSET) | ((y) & GFX_REG_LNEXY_LNEY_MASK); \
    }

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static bool
_SurfaceBoundaryCheck(
    GFXSurface *surface,
    int        clipX,
    int        clipY,
    int        clipWidth,
    int        clipHeight);

static GFXSurface *
_gfxCreateSurface(
    uint32_t       width,
    uint32_t       height,
    uint32_t       pitch,
    GFXColorFormat format);

static GFXMaskSurface *
_gfxCreateMaskSurface(
    uint32_t      width,
    uint32_t      height,
    uint32_t      pitch,
    GFXMaskFormat format);

// TODO
#if 0
static bool
_SurfaceDrawSurface(
    GFXSurface *dstSurface,
    int32_t    dstX,
    int32_t    dstY,
    int32_t    dstWidth,
    int32_t    dstHeight,
    GFXSurface *srcSurface,
    int32_t    srcX,
    int32_t    srcY,
    int32_t    srcWidth,
    int32_t    srcHeight,
    GFXSurface *maskSurface,
    bool       enableMask,
    int32_t    maskX,
    int32_t    maskY,
    int32_t    maskWidth,
    int32_t    maskHeight,
    bool       enableSrcColorKey,
    GFXColor   srcColorKey,
    bool       enableDstColorKey,
    GFXColor   dstColorKey,
    bool       enableAlphablending,
    bool       enableConstantAlpha,
    GFXColor   contantAlphaValue,
    float      rotateDegree);
#endif

static bool
_SurfaceBitblt(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface,
    GFX_ROP3      rop,
    GFXAlphaBlend *alphaBlend);

static bool
_SurfaceTransform(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    int32_t       refX,
    int32_t       refY,
    float         scaleWidth,
    float         scaleHeight,
    float         degree,
    GFXTileMode   mode,
    GFX_ROP3      rop,
    GFXAlphaBlend *alphaBlend);

static bool
_SurfaceFillColor(
    GFXSurfaceDst *surface,
    GFXColor      color,
    GFXAlphaBlend *alphaBlend);

static bool
_SurfaceGradientFill(
    GFXSurfaceDst *surface,
    GFXColor      initcolor,
    GFXColor      endcolor,
    GFXGradDir    dir,
    GFXAlphaBlend *alphaBlend);

static bool
_SurfaceProjection(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    float         scaleWidth,
    float         scaleHeight,
    float         degree,
    float         FOV,
    float         pivotX,
    GFX_ROP3      rop,
    GFXAlphaBlend *alphaBlend);

static bool
_SurfaceTransformBlt(
    GFXSurface* dest,
    int32_t dx,
    int32_t dy, 
    GFXSurface* src, 
    int32_t sx, 
    int32_t sy,
    int32_t sw, 
    int32_t sh, 
    int32_t x0, int32_t y0, 
    int32_t x1, int32_t y1, 
    int32_t x2, int32_t y2, 
    int32_t x3, int32_t y3,
    bool    bInverse,
    GFXPageFlowType type,
    GFXTransformType transformType);

static void
_setFGColor(
    GFXSurface *dstSurface,
    GFXColor   color);

static bool
_SurfaceDrawLine(
    GFXSurface *surface,
    int32_t    fromX,
    int32_t    fromY,
    int32_t    toX,
    int32_t    toY,
    GFXColor   *lineColor,
    int32_t    lineWidth);

#if CFG_CHIP_FAMILY == 9850
static bool
_SurfaceDrawPixel(
    GFXSurface *surface,
    int        x,
    int        y,
    GFXColor   *color,
    int        lineWidth);
#endif

static bool
_SurfaceDrawPixelAA(
    GFXSurface *surface,
    int        x,
    int        y,
    GFXColor   *color,
    int        lineWidth);

static bool
_SurfaceReflected(
    GFXSurface* dest,
    int32_t dx,
    int32_t dy,
    GFXSurface* src,
    int32_t sx,
    int32_t sy,
    int32_t reflectedWidth,
    int32_t reflectedHeight);

//=============================================================================
//                              Public Function Definition
//=============================================================================
GFXSurface *
gfxCreateSurface(
    uint32_t       width,
    uint32_t       height,
    uint32_t       pitch,
    GFXColorFormat format)
{
    GFXSurface *surface = NULL;

    GFX_FUNC_ENTRY;
    gfxLock();

    surface = _gfxCreateSurface(width, height, pitch, format);
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Create surface fail.\n");
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return surface;
}

GFXSurface *
gfxCreateSurfaceByBuffer(
    uint32_t       width,
    uint32_t       height,
    uint32_t       pitch,
    GFXColorFormat format,
    uint8_t        *buffer,
    uint32_t       bufferLength)
{
    GFXSurface *surface = NULL;

    GFX_FUNC_ENTRY;
    gfxLock();
    do
    {
        uint8_t *mappedSysRam = NULL;
        surface = _gfxCreateSurface(width, height, pitch, format);
        if (surface == NULL)
        {
            GFX_ERROR_MSG("Create surface fail.\n");
            break;
        }

        mappedSysRam = ithMapVram((uint32_t)surface->imageData, bufferLength, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, buffer, surface->imageDataLength);
        ithFlushDCacheRange(mappedSysRam, bufferLength);
        ithFlushMemBuffer();
        ithUnmapVram(mappedSysRam, bufferLength);
    } while (0);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return surface;
}

GFXSurface *
gfxCreateSurfaceByPointer(
    uint32_t       width,
    uint32_t       height,
    uint32_t       pitch,
    GFXColorFormat format,
    uint8_t        *alreadyExistPtr,
    uint32_t       ptrLength)
{
    GFXSurface *surface = NULL;

    GFX_FUNC_ENTRY;
    //gfxLock();

    do
    {
        surface = _gfxCreateSurface(width, height, pitch, format);
        if (surface == NULL)
        {
            GFX_ERROR_MSG("Create surface fail.\n");
            break;
        }
        surface->imageDataOwner  = false;
        gfxVmemFree(surface->imageData);
        surface->imageData       = alreadyExistPtr;
        surface->imageDataLength = ptrLength;
    } while (0);

    //gfxUnlock();
    GFX_FUNC_LEAVE;

    return surface;
}

void
gfxDestroySurface(
    GFXSurface *surface)
{
    GFX_FUNC_ENTRY;
    gfxLock();

    if (surface)
    {
        gfxwaitEngineIdle();
        if (surface->imageDataOwner == true
            && surface->imageData)
        {
            //printf("Gfx free %x %d\n", surface->imageData, surface->imageDataOwner);
            gfxVmemFree(surface->imageData);
            surface->imageData = NULL;
        }
        free(surface);
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;
}

int
gfxSurfaceGetWidth(
    GFXSurface *surface)
{
    int w = 0;

    GFX_FUNC_ENTRY;

    if (surface)
    {
        w = surface->width;
    }

    GFX_FUNC_LEAVE;

    return w;
}

int
gfxSurfaceGetHeight(
    GFXSurface *surface)
{
    int h = 0;

    GFX_FUNC_ENTRY;

    if (surface)
    {
        h = surface->height;
    }

    GFX_FUNC_LEAVE;

    return h;
}

GFXColorFormat
gfxSurfaceGetFormat(
    GFXSurface *surface)
{
    GFXColorFormat format = GFX_COLOR_UNKNOWN;

    GFX_FUNC_ENTRY;

    if (surface)
    {
        format = surface->format;
    }

    GFX_FUNC_LEAVE;

    return format;
}

void
gfxSurfaceSetSurfaceBaseAddress(
    GFXSurface *surface,
    uint32_t   addr)
{
    surface->imageData = (void*)addr;
}

void
gfxSurfaceSetWidth(
    GFXSurface *surface,
    uint32_t   width)
{
    surface->width = width;
}

void
gfxSurfaceSetHeight(
    GFXSurface *surface,
    uint32_t   height)
{
    surface->height = height;
}

void
gfxSurfaceSetPitch(
    GFXSurface *surface,
    uint32_t   pitch)
{
    surface->pitch = pitch;
}

bool
gfxSurfaceDrawGlyph(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface,
    GFXColor      color)
{
    bool          result     = true;
    GFXAlphaBlend alphaBlend = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface, dstX, dstY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    do
    {
        int32_t actualSrcWidth  = srcSurface->srcWidth;
        int32_t actualSrcHeight = srcSurface->srcHeight;

        if (srcSurface->srcSurface)
        {
            if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
            {
                actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
            }

            if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
            {
                actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
            }
        }
        if (dstSurface)
        {
            if ((dstX + actualSrcWidth) > dstSurface->width)
            {
                actualSrcWidth = dstSurface->width - dstX;
            }

            if ((dstY + actualSrcHeight) > dstSurface->height)
            {
                actualSrcHeight = dstSurface->height - dstY;
            }
        }

        _setFGColor(dstSurface, color);
        //printf("dstX:%d,dstY:%d\n",dstX,dstY);
        srcSurface->srcWidth           = actualSrcWidth;
        srcSurface->srcHeight          = actualSrcHeight;

        alphaBlend.enableAlpha         = true; // Enable Alpha Blending
        alphaBlend.enableConstantAlpha = false;
        alphaBlend.constantAlphaValue  = 0;    // No constant alpha, 0 for don't care

        result                         = _SurfaceBitblt(
            dstSurface,
            dstX, dstY,
            srcSurface,
            0xCC,
            &alphaBlend);
    } while (0);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceBitblt(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface)
{
    return gfxSurfaceBitbltWithRop(
        dstSurface,
        dstX,
        dstY,
        srcSurface,
        GFX_ROP3_SRCCOPY);
}

bool
gfxSurfaceBitbltWithRop(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface,
    GFX_ROP3      rop)
{
    bool          result     = true;
    GFXAlphaBlend alphaBlend = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface, dstX, dstY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    do
    {
        int32_t actualSrcWidth  = srcSurface->srcWidth;
        int32_t actualSrcHeight = srcSurface->srcHeight;

        if (srcSurface->srcSurface)
        {
            if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
            {
                actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
            }

            if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
            {
                actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
            }
        }
        if (dstSurface)
        {
            if ((dstX + actualSrcWidth) > dstSurface->width)
            {
                actualSrcWidth = dstSurface->width - dstX;
            }

            if ((dstY + actualSrcHeight) > dstSurface->height)
            {
                actualSrcHeight = dstSurface->height - dstY;
            }
        }

        srcSurface->srcWidth           = actualSrcWidth;
        srcSurface->srcHeight          = actualSrcHeight;

        alphaBlend.enableAlpha         = false; // No alpha blending
        alphaBlend.enableConstantAlpha = false;
        alphaBlend.constantAlphaValue  = 0;     // No constant alpha, 0 for don't care

        result                         = _SurfaceBitblt(
            dstSurface,
            dstX, dstY,
            srcSurface,
            rop,
            &alphaBlend);
    } while (0);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceTransform(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    int32_t       refX,
    int32_t       refY,
    float         scaleWidth,
    float         scaleHeight,
    float         degree,
    GFXTileMode   tilemode,
    GFX_ROP3      rop,
    GFXAlphaBlend *alphaBlend)
{
    bool    result          = false;
    int32_t actualSrcWidth  = srcSurface->srcWidth;
    int32_t actualSrcHeight = srcSurface->srcHeight;

    GFX_FUNC_ENTRY;

    if (dstSurface->dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface->dstSurface, dstSurface->dstX, dstSurface->dstY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    if (srcSurface->srcSurface)
    {
        if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
        {
            actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
        }

        if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
        {
            actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
        }
    }

    srcSurface->srcWidth  = actualSrcWidth;
    srcSurface->srcHeight = actualSrcHeight;

    result                = _SurfaceTransform(
        dstSurface,
        srcSurface,
        refX,
        refY,
        scaleWidth,
        scaleHeight,
        degree,
        tilemode,
        rop,
        alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceBitbltWithRotate(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface,
    int32_t       refX,
    int32_t       refY,
    float         degree)
{
    bool          result          = false;
    int32_t       actualSrcWidth  = srcSurface->srcWidth;
    int32_t       actualSrcHeight = srcSurface->srcHeight;
    GFXSurfaceDst *DstSurface     = 0;
    GFXAlphaBlend alphaBlend      = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface, dstX, dstY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    if (srcSurface->srcSurface)
    {
        if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
        {
            actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
        }

        if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
        {
            actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
        }
    }

    srcSurface->srcWidth           = actualSrcWidth;
    srcSurface->srcHeight          = actualSrcHeight;

    DstSurface->dstSurface         = dstSurface;
    DstSurface->dstX               = dstX;
    DstSurface->dstY               = dstY;
    DstSurface->dstWidth           = dstSurface->width;
    DstSurface->dstHeight          = dstSurface->height;

    alphaBlend.enableAlpha         = false; // No alpha blending
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;     // No constant alpha, 0 for don't care

    result                         = _SurfaceTransform(
        DstSurface,
        srcSurface,
        refX,
        refY,
        1.0,
        1.0,
        degree,
        GFX_TILE_FILL,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceStrectch(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface)
{
    bool          result          = false;
    int32_t       actualSrcWidth  = srcSurface->srcWidth;
    int32_t       actualSrcHeight = srcSurface->srcHeight;
    int32_t       actualDstWidth  = dstSurface->dstWidth;
    int32_t       actualDstHeight = dstSurface->dstHeight;

    float         scaleWidth      = (float) dstSurface->dstWidth / srcSurface->srcWidth;
    float         scaleHeight     = (float) dstSurface->dstHeight / srcSurface->srcHeight;

    GFXAlphaBlend alphaBlend      = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface->dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface->dstSurface, dstSurface->dstX, dstSurface->dstY, dstSurface->dstWidth, dstSurface->dstHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    if (srcSurface->srcSurface)
    {
        if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
        {
            actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
        }

        if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
        {
            actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
        }
    }

    if (dstSurface->dstSurface)
    {
        if ((dstSurface->dstX + dstSurface->dstWidth) > dstSurface->dstSurface->width)
        {
            actualDstWidth = dstSurface->dstSurface->width - dstSurface->dstX;
        }

        if ((dstSurface->dstY + dstSurface->dstHeight) > dstSurface->dstSurface->height)
        {
            actualDstHeight = dstSurface->dstSurface->height - dstSurface->dstY;
        }
    }

    srcSurface->srcWidth           = actualSrcWidth;
    srcSurface->srcHeight          = actualSrcHeight;
    dstSurface->dstWidth           = actualDstWidth;
    dstSurface->dstHeight          = actualDstHeight;

    alphaBlend.enableAlpha         = true; // No alpha blending
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;     // No constant alpha, 0 for don't care

    result                         = _SurfaceTransform(
        dstSurface,
        srcSurface,
        0,
        0,
        (float)scaleWidth,
        (float)scaleHeight,
        0.0,
        GFX_TILE_FILL,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);   // no alpha & const alpha

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceStrectchWithRotate(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    int32_t       refX,
    int32_t       refY,
    float         degree)
{
    bool          result          = false;
    int32_t       actualSrcWidth  = srcSurface->srcWidth;
    int32_t       actualSrcHeight = srcSurface->srcHeight;
    int32_t       actualDstWidth  = dstSurface->dstWidth;
    int32_t       actualDstHeight = dstSurface->dstHeight;

    float         scaleWidth      = (float) dstSurface->dstWidth / srcSurface->srcWidth;
    float         scaleHeight     = (float) dstSurface->dstHeight / srcSurface->srcHeight;

    GFXAlphaBlend alphaBlend      = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface->dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface->dstSurface, dstSurface->dstX, dstSurface->dstY, dstSurface->dstWidth, dstSurface->dstHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    if (srcSurface->srcSurface)
    {
        if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
        {
            actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
        }

        if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
        {
            actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
        }
    }

    if (dstSurface->dstSurface)
    {
        if ((dstSurface->dstX + dstSurface->dstWidth) > dstSurface->dstSurface->width)
        {
            actualDstWidth = dstSurface->dstSurface->width - dstSurface->dstX;
        }

        if ((dstSurface->dstY + dstSurface->dstHeight) > dstSurface->dstSurface->height)
        {
            actualDstHeight = dstSurface->dstSurface->height - dstSurface->dstY;
        }
    }

    srcSurface->srcWidth           = actualSrcWidth;
    srcSurface->srcHeight          = actualSrcHeight;
    dstSurface->dstWidth           = actualDstWidth;
    dstSurface->dstHeight          = actualDstHeight;

    alphaBlend.enableAlpha         = true; // No alpha blending
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;     // No constant alpha, 0 for don't care

    result                         = _SurfaceTransform(
        dstSurface,
        srcSurface,
        0,
        0,
        (float)scaleWidth,
        (float)scaleHeight,
        degree,
        GFX_TILE_FILL,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);   // no alpha & const alpha

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceAlphaBlend(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface)
{
    bool          result     = true;
    GFXAlphaBlend alphaBlend = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface, dstX, dstY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    do
    {
        int32_t actualSrcWidth  = srcSurface->srcWidth;
        int32_t actualSrcHeight = srcSurface->srcHeight;

        if (srcSurface->srcSurface)
        {
            if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
            {
                actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
            }

            if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
            {
                actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
            }
        }
        if (dstSurface)
        {
            if ((dstX + actualSrcWidth) > dstSurface->width)
            {
                actualSrcWidth = dstSurface->width - dstX;
            }

            if ((dstY + actualSrcHeight) > dstSurface->height)
            {
                actualSrcHeight = dstSurface->height - dstY;
            }
        }

        srcSurface->srcWidth           = actualSrcWidth;
        srcSurface->srcHeight          = actualSrcHeight;

        alphaBlend.enableAlpha         = true; // use alpha blending
        alphaBlend.enableConstantAlpha = false;
        alphaBlend.constantAlphaValue  = 0;    // No constant alpha, 0 for don't care

        result                         = _SurfaceBitblt(
            dstSurface,
            dstX, dstY,
            srcSurface,
            GFX_ROP3_SRCCOPY,
            &alphaBlend);
    } while (0);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceAlphaBlendEx(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface,
    bool          enableConstantAlpha,
    uint8_t       constantAlphaValue)
{
    bool          result     = true;
    GFXAlphaBlend alphaBlend = {0};

    GFX_FUNC_ENTRY;   
    
    if (dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface, dstX, dstY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    do
    {
        int32_t actualSrcWidth  = srcSurface->srcWidth;
        int32_t actualSrcHeight = srcSurface->srcHeight;

        if (srcSurface->srcSurface)
        {
            if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
            {
                actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
            }

            if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
            {
                actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
            }
        }
        if (dstSurface)
        {
            if ((dstX + actualSrcWidth) > dstSurface->width)
            {
                actualSrcWidth = dstSurface->width - dstX;
            }

            if ((dstY + actualSrcHeight) > dstSurface->height)
            {
                actualSrcHeight = dstSurface->height - dstY;
            }
        }

        srcSurface->srcWidth           = actualSrcWidth;
        srcSurface->srcHeight          = actualSrcHeight;

        alphaBlend.enableAlpha         = true; // use alpha blending
        alphaBlend.enableConstantAlpha = enableConstantAlpha;
        alphaBlend.constantAlphaValue  = constantAlphaValue;

        result                         = _SurfaceBitblt(
            dstSurface,
            dstX, dstY,
            srcSurface,
            GFX_ROP3_SRCCOPY,
            &alphaBlend);
    } while (0);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceAlphaBlendWithRotate(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface,
    int32_t       refX,
    int32_t       refY,
    float         degree)
{
    bool          result          = false;
    int32_t       actualSrcWidth  = srcSurface->srcWidth;
    int32_t       actualSrcHeight = srcSurface->srcHeight;
    GFXSurfaceDst *DstSurface     = 0;
    GFXAlphaBlend alphaBlend      = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface, dstX, dstY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    if (srcSurface->srcSurface)
    {
        if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
        {
            actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
        }

        if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
        {
            actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
        }
    }

    srcSurface->srcWidth           = actualSrcWidth;
    srcSurface->srcHeight          = actualSrcHeight;

    DstSurface->dstSurface         = dstSurface;
    DstSurface->dstX               = dstX;
    DstSurface->dstY               = dstY;
    DstSurface->dstWidth           = dstSurface->width;
    DstSurface->dstHeight          = dstSurface->height;

    alphaBlend.enableAlpha         = true; // enable alpha blending
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;    // No constant alpha, 0 for don't care

    result                         = _SurfaceTransform(
        DstSurface,
        srcSurface,
        refX,
        refY,
        1.0,
        1.0,
        degree,
        GFX_TILE_FILL,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceStrectchAlphaBlend(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface)
{
    bool          result          = false;
    int32_t       actualSrcWidth  = srcSurface->srcWidth;
    int32_t       actualSrcHeight = srcSurface->srcHeight;
    int32_t       actualDstWidth  = dstSurface->dstWidth;
    int32_t       actualDstHeight = dstSurface->dstHeight;

    float         scaleWidth      = (float) dstSurface->dstWidth / srcSurface->srcWidth;
    float         scaleHeight     = (float) dstSurface->dstHeight / srcSurface->srcHeight;
    GFXAlphaBlend alphaBlend      = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface->dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface->dstSurface, dstSurface->dstX, dstSurface->dstY, dstSurface->dstWidth, dstSurface->dstHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    if (srcSurface->srcSurface)
    {
        if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
        {
            actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
        }

        if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
        {
            actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
        }
    }

    if (dstSurface->dstSurface)
    {
        if ((dstSurface->dstX + dstSurface->dstWidth) > dstSurface->dstSurface->width)
        {
            actualDstWidth = dstSurface->dstSurface->width - dstSurface->dstX;
        }

        if ((dstSurface->dstY + dstSurface->dstHeight) > dstSurface->dstSurface->height)
        {
            actualDstHeight = dstSurface->dstSurface->height - dstSurface->dstY;
        }
    }

    srcSurface->srcWidth           = actualSrcWidth;
    srcSurface->srcHeight          = actualSrcHeight;
    dstSurface->dstWidth           = actualDstWidth;
    dstSurface->dstHeight          = actualDstHeight;

    alphaBlend.enableAlpha         = true; // use alpha blending
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;    // No constant alpha, 0 for don't care

    result                         = _SurfaceTransform(
        dstSurface,
        srcSurface,
        0,
        0,
        (float)scaleWidth,
        (float)scaleHeight,
        0.0,
        GFX_TILE_FILL,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceStrectchAlphaBlendWithRotate(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    int32_t       refX,
    int32_t       refY,
    float         degree)
{
    bool          result          = false;
    int32_t       actualSrcWidth  = srcSurface->srcWidth;
    int32_t       actualSrcHeight = srcSurface->srcHeight;
    int32_t       actualDstWidth  = dstSurface->dstWidth;
    int32_t       actualDstHeight = dstSurface->dstHeight;

    float         scaleWidth      = (float) dstSurface->dstWidth / srcSurface->srcWidth;
    float         scaleHeight     = (float) dstSurface->dstHeight / srcSurface->srcHeight;

    GFXAlphaBlend alphaBlend      = {0};

    GFX_FUNC_ENTRY;

    if (dstSurface->dstSurface)
    {
        // Check
        if (_SurfaceBoundaryCheck(dstSurface->dstSurface, dstSurface->dstX, dstSurface->dstY, dstSurface->dstWidth, dstSurface->dstHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (srcSurface->srcSurface)
    {
        if (_SurfaceBoundaryCheck(srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    gfxLock();

    if (srcSurface->srcSurface)
    {
        if ((srcSurface->srcX + srcSurface->srcWidth) > srcSurface->srcSurface->width)
        {
            actualSrcWidth = srcSurface->srcSurface->width - srcSurface->srcX;
        }

        if ((srcSurface->srcY + srcSurface->srcHeight) > srcSurface->srcSurface->height)
        {
            actualSrcHeight = srcSurface->srcSurface->height - srcSurface->srcY;
        }
    }

    if (dstSurface->dstSurface)
    {
        if ((dstSurface->dstX + dstSurface->dstWidth) > dstSurface->dstSurface->width)
        {
            actualDstWidth = dstSurface->dstSurface->width - dstSurface->dstX;
        }

        if ((dstSurface->dstY + dstSurface->dstHeight) > dstSurface->dstSurface->height)
        {
            actualDstHeight = dstSurface->dstSurface->height - dstSurface->dstY;
        }
    }

    srcSurface->srcWidth           = actualSrcWidth;
    srcSurface->srcHeight          = actualSrcHeight;
    dstSurface->dstWidth           = actualDstWidth;
    dstSurface->dstHeight          = actualDstHeight;

    alphaBlend.enableAlpha         = true; // use alpha blending
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;    // No constant alpha, 0 for don't care

    result                         = _SurfaceTransform(
        dstSurface,
        srcSurface,
        0,
        0,
        (float)scaleWidth,
        (float)scaleHeight,
        degree,
        GFX_TILE_FILL,
        GFX_ROP3_SRCCOPY,
        &alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceFillColor(
    GFXSurfaceDst *surface,
    GFXColor      color)
{
    bool          result     = false;
    GFXAlphaBlend alphaBlend = {0};

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    alphaBlend.enableAlpha         = false;
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;

    result                         = _SurfaceFillColor(surface,
                                                       color,
                                                       &alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceFillColorWithBlend(
    GFXSurfaceDst *surface,
    GFXColor      color,
    GFXAlphaBlend *alphaBlend)
{
    bool result = false;

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    result = _SurfaceFillColor(surface,
                               color,
                               alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceGradientFill(
    GFXSurfaceDst *surface,
    GFXColor      initcolor,
    GFXColor      endcolor,
    GFXGradDir    dir)
{
    bool          result     = false;
    GFXAlphaBlend alphaBlend = {0};

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    alphaBlend.enableAlpha         = false;
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;

    result                         = _SurfaceGradientFill(surface,
                                                          initcolor,
                                                          endcolor,
                                                          dir,
                                                          &alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceGradientFillWithBlend(
    GFXSurfaceDst *surface,
    GFXColor      initcolor,
    GFXColor      endcolor,
    GFXGradDir    dir,
    GFXAlphaBlend *alphaBlend)
{
    bool result = false;

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    result = _SurfaceGradientFill(surface,
                                  initcolor,
                                  endcolor,
                                  dir,
                                  alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceProjection(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    float         scaleWidth,
    float         scaleHeight,
    float         degree,
    float         FOV,
    float         pivotX,
    GFXAlphaBlend *alphaBlend)
{
    bool result = false;

    if (!dstSurface || !srcSurface)
    {
        return false;
    }

    result = _SurfaceProjection(
        dstSurface,
        srcSurface,
        scaleWidth,
        scaleHeight,
        degree,
        FOV,
        pivotX,
        GFX_ROP3_SRCCOPY,
        alphaBlend);

    return result;
}

bool
gfxSurfaceTransformBlt(
    GFXSurface* dest,
    int32_t dx,
    int32_t dy, 
    GFXSurface* src, 
    int32_t sx, 
    int32_t sy,
    int32_t sw, 
    int32_t sh, 
    int32_t x0, int32_t y0, 
    int32_t x1, int32_t y1, 
    int32_t x2, int32_t y2, 
    int32_t x3, int32_t y3,
    bool    bInverse,
    GFXPageFlowType type,
    GFXTransformType transformType)
{
    bool result = false;

    if (!dest || !src)
    {
        return false;
    }

    result = _SurfaceTransformBlt(
        dest,
        dx,
        dy, 
        src, 
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

    return result;
}

bool
gfxSurfaceDrawLine(
    GFXSurface *surface,
    int32_t    fromX,
    int32_t    fromY,
    int32_t    toX,
    int32_t    toY,
    GFXColor   *lineColor,
    int32_t    lineWidth)
{
    bool result = false;

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    result = _SurfaceDrawLine(surface,
                              fromX,
                              fromY,
                              toX,
                              toY,
                              lineColor,
                              lineWidth);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxSurfaceDrawCurve(
    GFXSurface     *surface,
    GFXCoordinates *point1,
    GFXCoordinates *point2,
    GFXCoordinates *point3,
    GFXCoordinates *point4,
    GFXColor       *lineColor,
    int32_t        lineWidth)
{
    bool  result = false;
    float x, y = 0.0;
    float t      = 0.0;

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    for (t = 0.0; t <= 1.0; t += 0.01)
    {
        x = (-t * t * t + 3 * t * t - 3 * t + 1) * point1->x
            + (3 * t * t * t - 6 * t * t + 3 * t) * point2->x
            + (-3 * t * t * t + 3 * t * t) * point3->x
            + (t * t * t) * point4->x;

        y = (-t * t * t + 3 * t * t - 3 * t + 1) * point1->y
            + (3 * t * t * t - 6 * t * t + 3 * t) * point2->y
            + (-3 * t * t * t + 3 * t * t) * point3->y
            + (t * t * t) * point4->y;

        //x = -4.5*(t-1/3)*(t-2/3)*(t-1) * point1->x
        //  + 13.5*t*(t-2/3)*(t-1)       * point2->x
        //  + -13.5*t*(t-1/3)*(t-1)      * point3->x
        //  + 4.5*t*(t-1/3)*(t-2/3)      * point4->x;

        //y = -4.5*(t-1/3)*(t-2/3)*(t-1) * point1->y
        //  + 13.5*t*(t-2/3)*(t-1)       * point2->y
        //  + -13.5*t*(t-1/3)*(t-1)      * point3->y
        //  + 4.5*t*(t-1/3)*(t-2/3)      * point4->y;
        //cout << '(' << x << ',' << y << ')';

        result = _SurfaceDrawPixelAA(surface, (int)round(x), (int)round(y), lineColor, lineWidth);
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

void
gfxSurfaceSetColor(
    GFXSurface *surface,
    GFXColor   color)
{
    _setFGColor(surface, color);
}

void
gfxSurfaceSetQuality(
    GFXSurface     *surface,
    GFXQualityMode quality)
{
    if (surface)
    {
        surface->quality = quality;
    }
}

void
gfxSurfaceSetClip(
    GFXSurface *surface,
    int        x0,
    int        y0,
    int        x1,
    int        y1)
{
    surface->clipSet.left   = x0;
    surface->clipSet.top    = y0;
    surface->clipSet.right  = x1;
    surface->clipSet.bottom = y1;
    surface->clipEnable     = true;
}

void
gfxSurfaceUnSetClip(
    GFXSurface *surface)
{
    surface->clipEnable = false;
}

GFXMaskSurface *
gfxCreateMaskSurface(
    uint32_t      width,
    uint32_t      height,
    uint32_t      pitch,
    GFXMaskFormat format)
{
    GFXMaskSurface *surface = NULL;

    GFX_FUNC_ENTRY;
    gfxLock();

    surface = _gfxCreateMaskSurface(width, height, pitch, format);
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Create surface fail.\n");
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return surface;
}

GFXMaskSurface *
gfxCreateMaskSurfaceByBuffer(
    uint32_t      width,
    uint32_t      height,
    uint32_t      pitch,
    GFXMaskFormat format,
    uint8_t       *buffer,
    uint32_t      bufferLength)
{
    GFXMaskSurface *surface = NULL;

    GFX_FUNC_ENTRY;
    gfxLock();

    do
    {
        uint8_t *mappedSysRam = NULL;
        surface = _gfxCreateMaskSurface(width, height, pitch, format);
        if (surface == NULL)
        {
            GFX_ERROR_MSG("Create surface fail.\n");
            break;
        }

        mappedSysRam = ithMapVram((uint32_t)surface->imageData, bufferLength, ITH_VRAM_WRITE);
        memcpy(mappedSysRam, buffer, surface->imageDataLength);
        ithFlushDCacheRange(mappedSysRam, bufferLength);
        ithFlushMemBuffer();
        ithUnmapVram(mappedSysRam, bufferLength);
    } while (0);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return surface;
}

void
gfxDestroyMaskSurface(
    GFXMaskSurface *surface)
{
    GFX_FUNC_ENTRY;
    gfxLock();

    if (surface)
    {
        gfxwaitEngineIdle();

        if (surface->imageDataOwner == true
            && surface->imageData)
        {
            gfxVmemFree(surface->imageData);
            surface->imageData = NULL;
        }
        free(surface);
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;
}

void
gfxSetMaskSurface(
    GFXSurface     *surface,
    GFXMaskSurface *maskSurface,
    bool           enable)
{
    GFX_FUNC_ENTRY;
    gfxLock();

    if (surface)
    {
        surface->maskEnable  = enable;
        surface->maskSurface = maskSurface;
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;
}

bool
gfxSurfaceReflected(
    GFXSurface* dest,
    int32_t dx,
    int32_t dy,
    GFXSurface* src,
    int32_t sx,
    int32_t sy,
    int32_t reflectedWidth,
    int32_t reflectedHeight)
{
    bool          result = true;

    GFX_FUNC_ENTRY;

    if (dest == NULL || src == NULL)
    {
        return false;
    }

    gfxLock();

    result = _SurfaceReflected(
        dest,
        dx,
        dy,
        src,
        sx,
        sy,
        reflectedWidth,
        reflectedHeight);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;

}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static bool
_SurfaceBoundaryCheck(
    GFXSurface *surface,
    int        clipX,
    int        clipY,
    int        clipWidth,
    int        clipHeight)
{
    bool result = false;

#if 0
    do
    {
        if (surface == NULL)
        {
            GFX_ERROR_MSG("Invalud destination surface.\n");
            break;
        }
        if (clipX >= surface->width)
        {
            GFX_ERROR_MSG("Invalud start X(%d). Over surface's width(%d).\n", clipX, surface->width);
            break;
        }
        if (clipY >= surface->height)
        {
            GFX_ERROR_MSG("Invalud start Y(%d). Over surface's height(%d).\n", clipY, surface->height);
            break;
        }
        if ((clipX + clipWidth) <= 0)
        {
            GFX_ERROR_MSG("Selected X(%d) and width(%d) out of boundary.\n", clipX, clipWidth);
            break;
        }
        if ((clipY + clipHeight) <= 0)
        {
            GFX_ERROR_MSG("Selected Y(%d) and height(%d) out of boundary.\n", clipY, clipHeight);
            break;
        }

        result = true;
    } while (0);
#else
    result = true;
#endif
    return result;
}

static GFXSurface *
_gfxCreateSurface(
    uint32_t       width,
    uint32_t       height,
    uint32_t       pitch,
    GFXColorFormat format)
{
    GFXSurface *surface = NULL;
    bool       result   = false;

    do
    {
#ifdef WIN32
        surface = (GFXSurface *)malloc(sizeof(GFXSurface));
#else
        surface = (GFXSurface *)memalign(sizeof(int), sizeof(GFXSurface));
#endif
        if (surface == NULL)
        {
            GFX_ERROR_MSG("Out of memory.\n");
            break;
        }

        surface->width           = width;
        surface->height          = height;
        surface->pitch           = pitch;
        surface->format          = format;
        surface->imageDataLength = pitch * height;
        surface->imageData       = gfxVmemAlloc(surface->imageDataLength);
        surface->imageDataOwner  = true;
        surface->maskEnable      = false;
        surface->maskSurface     = NULL;
        surface->clipEnable      = false;
        surface->quality         = GFX_QUALITY_BETTER;
        if (surface->imageData == NULL)
        {
            GFX_ERROR_MSG("Out of memory.\n");
            break;
        }
        result = true;
    } while (0);

    if (result == false)
    {
        if (surface)
        {
            free(surface);
            surface = NULL;
        }
    }

    return surface;
}

static GFXMaskSurface *
_gfxCreateMaskSurface(
    uint32_t      width,
    uint32_t      height,
    uint32_t      pitch,
    GFXMaskFormat format)
{
    GFXMaskSurface *surface = NULL;
    bool           result   = false;

    do
    {
#ifdef WIN32
        surface = (GFXMaskSurface * )malloc(sizeof(GFXMaskSurface));
#else
        surface = (GFXMaskSurface * )memalign(sizeof(int), sizeof(GFXMaskSurface));
#endif
        if (surface == NULL)
        {
            GFX_ERROR_MSG("Out of memory.\n");
            break;
        }

        surface->width           = width;
        surface->height          = height;
        surface->pitch           = pitch;
        surface->format          = format;
        surface->imageDataLength = pitch * height;
        surface->imageData       = gfxVmemAlloc(surface->imageDataLength);
        surface->imageDataOwner  = true;
        if (surface->imageData == NULL)
        {
            GFX_ERROR_MSG("Out of memory.\n");
            break;
        }
        result = true;
    } while (0);

    if (result == false)
    {
        if (surface)
        {
            free(surface);
            surface = NULL;
        }
    }

    return surface;
}

// TODO
#if 0
static bool
_SurfaceDrawSurface(
    GFXSurface *dstSurface,
    int32_t    dstX,
    int32_t    dstY,
    int32_t    dstWidth,
    int32_t    dstHeight,
    GFXSurface *srcSurface,
    int32_t    srcX,
    int32_t    srcY,
    int32_t    srcWidth,
    int32_t    srcHeight,
    GFXSurface *maskSurface,
    bool       enableMask,
    int32_t    maskX,
    int32_t    maskY,
    int32_t    maskWidth,
    int32_t    maskHeight,
    bool       enableSrcColorKey,
    GFXColor   srcColorKey,
    bool       enableDstColorKey,
    GFXColor   dstColorKey,
    bool       enableAlphablending,
    bool       enableConstantAlpha,
    GFXColor   contantAlphaValue,
    float      rotateDegree)
{
    bool        result  = false;
    GFX_DRIVER  *driver = gfxGetDriver();
    GFX_HW_REGS *regs   = &driver->hwDev->regs;

    regs->regSRC.data   = (uint32_t)srcSurface->imageData;
    regs->regMASK.data  = (enableMask == true) ? (uint32_t)maskSurface->imageData : 0;
    regs->regSPR.data   = srcSurface->pitch;
    regs->regMPR.data   = (enableMask == true) ? maskSurface->pitch : 0;
    regs->regSRCXY.data = ((srcX & GFX_REG_SRCXY_SRCX_MASK) << GFX_REG_SRCXY_SRCX_OFFSET) | (srcY & GFX_REG_SRCXY_SRCY_MASK);
    regs->regSHWR.data  = ((srcWidth & GFX_REG_SHWR_SRCWIDTH_MASK) << GFX_REG_SHWR_SRCWIDTH_OFFSET) | (srcHeight & GFX_REG_SHWR_SRCHEIGHT_MASK);
    regs->regDST.data   = (uint32_t)dstSurface->imageData;
    regs->regDSTXY.data = ((dstX & GFX_REG_DSTXY_DSTX_MASK) << GFX_REG_DSTXY_DSTX_OFFSET) | (dstY & GFX_REG_DSTXY_DSTY_MASK);
    regs->regDHWR.data  = ((dstWidth & GFX_REG_DHWR_DSTWIDTH_MASK) << GFX_REG_DHWR_DSTWIDTH_OFFSET) | (dstHeight & GFX_REG_DHWR_DSTHEIGHT_MASK);
    regs->regDPR.data   = dstSurface->pitch;
    regs->regPXCR.data  = (dstWidth - 1) & GFX_REG_PXCR_CLIPXEND_MASK;
    regs->regPYCR.data  = (dstHeight - 1) & GFX_REG_PYCR_CLIPYEND_MASK;

    // Set src & dst surface format

    // Enable clipping here!

    if (enableMask == true)
    {
        // Enable mask

        // Setup mask data format
    }

    if (rotateDegree > 0)
    {
        // Setup ITMR

        // Calculate rotate boundary, reset clipping range here!

        // Enable Transform here!

        // Enable perspective here, if needed!

        // Set interpolation mode

        // Set texture tile mode
    }

    // Setup Color key
    if (enableSrcColorKey == true)
    {
        // Set transparent mode
    }
    if (enableDstColorKey == true)
    {
        // Set transparent mode
    }

    if (enableAlphablending == true)
    {
        // Enable alpha blending
    }

    if (enableConstantAlpha == true)
    {
        // Enable constant alpha
    }

    // Setup constant alpha
    if (enableConstantAlpha == true)
    {
    }

    // Enable dithering

    return result;
}
#endif

static bool
_SurfaceBitblt(
    GFXSurface    *dstSurface,
    int32_t       dstX,
    int32_t       dstY,
    GFXSurfaceSrc *srcSurface,
    GFX_ROP3      rop,
    GFXAlphaBlend *alphaBlend)
{
    bool        result  = false;
    GFX_DRIVER  *driver = gfxGetDriver();
    GFX_HW_REGS *regs   = &driver->hwDev->regs;

    gfxHwReset(driver->hwDev);

    if (srcSurface->srcSurface)
    {
        REG_FILL_SRC_SURFACE(regs, srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight);
    }

    if (dstSurface)
    {
        REG_FILL_DST_SURFACE(regs, dstSurface, dstX, dstY, srcSurface->srcWidth, srcSurface->srcHeight);
    }

    // Clipping
    if (dstSurface->clipEnable)
    {
        REG_FILL_DST_CLIP(regs,
                          dstSurface->clipSet.left,
                          dstSurface->clipSet.top,
                          dstSurface->clipSet.right,
                          dstSurface->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
    }

    // ROP3
    REG_FILL_ROP3(regs, rop);

    // About blending
    REG_FILL_BLENDING(regs, alphaBlend->enableAlpha, alphaBlend->enableConstantAlpha, alphaBlend->constantAlphaValue);

    // Foreground Color
    REG_FILL_FGCOLOR(regs, dstSurface->fgColor);

    // Mask
    REG_FILL_MASK_SURFACE(regs, dstSurface, dstX, dstY, dstSurface->width, dstSurface->height);

    // FIXME
/*
    if ((srcSurface->format == GFX_COLOR_A_8) ||
        (srcSurface->format == GFX_COLOR_A_4) ||
        (srcSurface->format == GFX_COLOR_A_2) ||
        (srcSurface->format == GFX_COLOR_A_1) ||
        (srcSurface->format != dstSurface->format)) {
        regs->regSAFE.data |= GFX_REG_SAFE_PIXELIZE_EN;
    }
 */

    // Fire
    regs->regCMD.data = GFX_REG_CMD_ROP3;

    result            = gfxHwEngineFire(driver->hwDev);

    return result;
}

static bool
_SurfaceTransform(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    int32_t       refX,
    int32_t       refY,
    float         scaleWidth,
    float         scaleHeight,
    float         degree,
    GFXTileMode   tilemode,
    GFX_ROP3      rop,
    GFXAlphaBlend *alphaBlend)
{
    bool        result  = false;
    GFX_DRIVER  *driver = gfxGetDriver();
    GFX_HW_REGS *regs   = &driver->hwDev->regs;
    GFX_MATRIX  mtx;
    GFX_MATRIX  inverseMtx;
    int32_t     degree_i = degree;
    int32_t     new_dstX, new_dstY, new_dstWidth, new_dstHeight;
    int32_t     clip_x0 = 0, clip_y0 = 0, clip_x1 = 0, clip_y1 = 0;
    bool        extra_clip = false;

    //printf("dest (%d, %d)(%d, %d)\n", dstSurface->dstX, dstSurface->dstY, dstSurface->dstWidth, dstSurface->dstHeight);
    //printf("src (%d, %d)(%d, %d)\n", srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight);

    gfxHwReset(driver->hwDev);

    memset((void *)&inverseMtx, 0, sizeof(GFX_MATRIX));

    gfxMatrixIdentify(&mtx);

    gfxMatrixTranslate(&mtx, dstSurface->dstX, dstSurface->dstY);

    if (scaleWidth != 1.0 || scaleHeight != 1.0)
        gfxMatrixScale(&mtx, scaleWidth, scaleHeight);

    if (refX != 0 || refY != 0)
        gfxMatrixTranslate(&mtx, refX, refY);

    if (degree != 0.0)
        gfxMatrixRotate(&mtx, degree);

    if (refX != 0 || refY != 0)
        gfxMatrixTranslate(&mtx, -refX, -refY);

    if (tilemode == GFX_TILE_FILL)   // fill mode
    {
        float   topLX, topLY, topRX, topRY;
        float   botLX, botLY, botRX, botRY;
        int32_t topX, topY;
        int32_t botX, botY;

        // Find the top left and bottom right coordinate
        {
            gfxMatrixTransform(&mtx, srcSurface->srcX, srcSurface->srcY, &topLX, &topLY);
            gfxMatrixTransform(&mtx, srcSurface->srcX + srcSurface->srcWidth, srcSurface->srcY, &topRX, &topRY);
            gfxMatrixTransform(&mtx, srcSurface->srcX, srcSurface->srcY + srcSurface->srcHeight, &botLX, &botLY);
            gfxMatrixTransform(&mtx, srcSurface->srcX + srcSurface->srcWidth, srcSurface->srcY + srcSurface->srcHeight, &botRX, &botRY);

            topX = (int32_t)floorf(MIN(MIN(topLX, topRX), MIN(botLX, botRX))) - 1;
            topY = (int32_t)floorf(MIN(MIN(topLY, topRY), MIN(botLY, botRY))) - 1;
            botX = (int32_t)ceilf(MAX(MAX(topLX, topRX), MAX(botLX, botRX))) + 1;
            botY = (int32_t)ceilf(MAX(MAX(topLY, topRY), MAX(botLY, botRY))) + 1;

            //printf("topL (%f, %f), topR (%f, %f)\n", topLX, topLY, topRX, topRY);
            //printf("botL (%f, %f), botR (%f, %f)\n", botLX, botLY, botRX, botRY);
            //printf("top (%d, %d), bot (%d, %d)\n", topX, topY, botX, botY);
        }

        gfxMatrixIdentify(&mtx);

        // Adjust the coordinate
        gfxMatrixTranslate(&mtx, dstSurface->dstX - topX, dstSurface->dstY - topY);

        if (scaleWidth != 1.0 || scaleHeight != 1.0)
            gfxMatrixScale(&mtx, scaleWidth, scaleHeight);

        if (refX != 0 || refY != 0)
            gfxMatrixTranslate(&mtx, refX, refY);

        if (topX < 0 || topY < 0 || botX > dstSurface->dstSurface->width || botY > dstSurface->dstSurface->height)
        {
            clip_x0    = (topX < 0) ? 0 : topX;
            clip_y0    = (topY < 0) ? 0 : topY;
            clip_x1    = (botX > dstSurface->dstSurface->width) ? dstSurface->dstSurface->width : botX;
            clip_y1    = (botY > dstSurface->dstSurface->height) ? dstSurface->dstSurface->height :  botY;
            extra_clip = true;
        }

        if (degree != 0.0)
            gfxMatrixRotate(&mtx, degree);

        if (refX != 0 || refY != 0)
            gfxMatrixTranslate(&mtx, -refX, -refY);
        
        //botX = botX >= dstSurface->dstWidth ? dstSurface->dstX + dstSurface->dstWidth - 1 : botX;       
        //botY = botY >= dstSurface->dstHeight ? dstSurface->dstY + dstSurface->dstHeight - 1 : botY;

        new_dstX      = topX;
        new_dstY      = topY;
        new_dstWidth  = botX - topX + 1;
        new_dstHeight = botY - topY + 1;
    }
    else     // pad, repeat, reflect mode
    {
        new_dstX      = dstSurface->dstX;
        new_dstY      = dstSurface->dstY;
        new_dstWidth  = dstSurface->dstWidth;
        new_dstHeight = dstSurface->dstHeight;
    }

    //printf("new dst (%d, %d)(%d, %d)\n", new_dstX, new_dstY, new_dstWidth, new_dstHeight);
    if (dstSurface->dstSurface)
    {
        REG_FILL_DST_SURFACE(regs, dstSurface->dstSurface, new_dstX, new_dstY, new_dstWidth, new_dstHeight);
    }

    if (srcSurface->srcSurface)
    {
        REG_FILL_SRC_SURFACE(regs, srcSurface->srcSurface, srcSurface->srcX, srcSurface->srcY, srcSurface->srcWidth, srcSurface->srcHeight);
    }

    // Clipping
    if (extra_clip)
    {
        if (dstSurface->dstSurface->clipEnable)
        {
            clip_x0 = MAX(clip_x0, dstSurface->dstSurface->clipSet.left);
            clip_y0 = MAX(clip_y0, dstSurface->dstSurface->clipSet.top);
            clip_x1 = MIN(clip_x1, dstSurface->dstSurface->clipSet.right);
            clip_y1 = MIN(clip_y1, dstSurface->dstSurface->clipSet.bottom);
        }
        REG_FILL_DST_CLIP(regs,
                          clip_x0,
                          clip_y0,
                          clip_x1,
                          clip_y1);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
        //printf("Clip top (%d, %d), bot (%d, %d)\n", clip_x0, clip_y0, clip_x1, clip_y1);
    }
    else if (dstSurface->dstSurface->clipEnable)
    {
        REG_FILL_DST_CLIP(regs,
                          dstSurface->dstSurface->clipSet.left,
                          dstSurface->dstSurface->clipSet.top,
                          dstSurface->dstSurface->clipSet.right,
                          dstSurface->dstSurface->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
    }

    gfxMatrixInverse(&mtx, &inverseMtx);
    REG_FILL_SET_ITM(regs, inverseMtx);

    // ROP3
    REG_FILL_ROP3(regs, rop);

    // About blending
    REG_FILL_BLENDING(regs, alphaBlend->enableAlpha, alphaBlend->enableConstantAlpha, alphaBlend->constantAlphaValue);

    // Foreground Color
    REG_FILL_FGCOLOR(regs, dstSurface->dstSurface->fgColor);

    // Mask
    REG_FILL_MASK_SURFACE(regs, dstSurface->dstSurface, new_dstX, new_dstY, new_dstWidth, new_dstHeight);

    // Control 1
    regs->regCR1.data |= GFX_REG_CR1_TRANSFORM_EN
                         | ((tilemode << GFX_REG_CR1_TILEMODE_OFFSET) & GFX_REG_CR1_TILEMODE_MASK);

    // Disable cache for test
    //regs->regCR1.data |= GFX_REG_CR1_MANUALCACHE_EN;

    if (dstSurface->dstSurface)
    {
        if (dstSurface->dstSurface->quality == GFX_QUALITY_BETTER)
        {
            regs->regCR1.data |= GFX_REG_CR1_INTERPOLATE_EN;
    
            if (tilemode == GFX_TILE_FILL)
                regs->regCR1.data |= GFX_REG_CR1_ALPHABLEND_EN;
        }
    }

    // Temp solution for rotate LCD application to speed up the performance
    // if quality set faster mode, disable alphablending
    if (dstSurface->dstSurface->quality == GFX_QUALITY_FASTER)
	{        
		regs->regCR1.data = regs->regCR1.data & ~(GFX_REG_CR1_ALPHABLEND_EN);

        // disable cache
        #if CFG_CHIP_FAMILY == 9850
		regs->regCR1.data |= GFX_REG_CR1_MANUALCACHE_EN;
        #endif
    }

    // Change the scan direction to increase the hit rate of cache
    // (it only supports after IT9920 project)
    if ((degree_i > 45*1 && degree_i < 45*3) || (degree_i > 45*5 && degree_i < 45*7))
    {
        regs->regCR1.data |= GFX_REG_CR1_ROWMAJOR;
	}
       
    // Fire
    regs->regCMD.data = GFX_REG_CMD_ROP3;

    result            = gfxHwEngineFire(driver->hwDev);

    return result;
}

static bool
_SurfaceFillColor(
    GFXSurfaceDst *surface,
    GFXColor      color,
    GFXAlphaBlend *alphaBlend)
{
    bool        result  = false;
    GFX_DRIVER  *driver = gfxGetDriver();
    GFX_HW_REGS *regs   = &driver->hwDev->regs;

    gfxHwReset(driver->hwDev);

    // Surface
    if (surface->dstSurface)
    {
        REG_FILL_DST_SURFACE(regs, surface->dstSurface, surface->dstX, surface->dstY, surface->dstWidth, surface->dstHeight);
    }

    // Clipping
    if (surface->dstSurface->clipEnable)
    {
        REG_FILL_DST_CLIP(regs,
                          surface->dstSurface->clipSet.left,
                          surface->dstSurface->clipSet.top,
                          surface->dstSurface->clipSet.right,
                          surface->dstSurface->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
    }

    // About blending
    REG_FILL_BLENDING(regs, alphaBlend->enableAlpha, alphaBlend->enableConstantAlpha, alphaBlend->constantAlphaValue);

    // Foreground Color
    REG_FILL_FGCOLOR(regs, color);

    // Mask
    REG_FILL_MASK_SURFACE(regs, surface->dstSurface, surface->dstX, surface->dstY, surface->dstWidth, surface->dstHeight);

    // ROP3
    REG_FILL_ROP3(regs, GFX_ROP3_PATCOPY);

    if (surface->dstSurface)
    {
        // Control Bits
        regs->regCR1.data |= GFX_REG_CR1_TILEMODE_FILL;
        regs->regCR2.data |=
            (surface->dstSurface->format << GFX_REG_CR2_DSTFORMAT_OFFSET)
            | (surface->dstSurface->format << GFX_REG_CR2_SRCFORMAT_OFFSET);
    }

    // Fire
    regs->regCMD.data = GFX_REG_CMD_ROP3;

    result            = gfxHwEngineFire(driver->hwDev);

    return result;
}

static bool
_SurfaceGradientFill(
    GFXSurfaceDst *surface,
    GFXColor      initcolor,
    GFXColor      endcolor,
    GFXGradDir    dir,
    GFXAlphaBlend *alphaBlend)
{
    bool        result  = false;
    GFX_DRIVER  *driver = gfxGetDriver();
    GFX_HW_REGS *regs   = &driver->hwDev->regs;

    if (surface->dstWidth <= 0 || surface->dstHeight <= 0)
        return true;

    gfxHwReset(driver->hwDev);

    if (surface->dstSurface)
    {
        // Surface
        REG_FILL_DST_SURFACE(regs, surface->dstSurface, surface->dstX, surface->dstY, surface->dstWidth, surface->dstHeight);
    }

    // Clipping
    if (surface->dstSurface->clipEnable)
    {
        REG_FILL_DST_CLIP(regs,
                          surface->dstSurface->clipSet.left,
                          surface->dstSurface->clipSet.top,
                          surface->dstSurface->clipSet.right,
                          surface->dstSurface->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
    }

    // About blending
    REG_FILL_BLENDING(regs, alphaBlend->enableAlpha, alphaBlend->enableConstantAlpha, alphaBlend->constantAlphaValue);

    // Mask
    REG_FILL_MASK_SURFACE(regs, surface->dstSurface, surface->dstX, surface->dstY, surface->dstWidth, surface->dstHeight);

    // Initial Color
    REG_FILL_FGCOLOR(regs, initcolor);

    // Gradient Direction
    switch (dir)
    {
    case GFX_GRAD_H: regs->regCR2.data    |= GFX_REG_CR2_GRADMODE_H;
        break;
    case GFX_GRAD_V: regs->regCR2.data    |= GFX_REG_CR2_GRADMODE_V;
        break;
    case GFX_GRAD_BOTH: regs->regCR2.data |= GFX_REG_CR2_GRADMODE_BI;
        break;
    default: result                        = false;
        goto end;
    }

    // Color Delta
    if (dir == GFX_GRAD_H)
    {
        regs->regITMR00.data = ((((int32_t)endcolor.a - (int32_t)initcolor.a) * (1 << 12)) / surface->dstWidth) * (1 << 4);
        regs->regITMR01.data = ((((int32_t)endcolor.r - (int32_t)initcolor.r) * (1 << 12)) / surface->dstWidth) * (1 << 4);
        regs->regITMR02.data = ((((int32_t)endcolor.g - (int32_t)initcolor.g) * (1 << 12)) / surface->dstWidth) * (1 << 4);
        regs->regITMR10.data = ((((int32_t)endcolor.b - (int32_t)initcolor.b) * (1 << 12)) / surface->dstWidth) * (1 << 4);
    }

    if (dir == GFX_GRAD_V)
    {
        regs->regITMR11.data = ((((int32_t)endcolor.a - (int32_t)initcolor.a) * (1 << 12)) / surface->dstHeight) * (1 << 4);
        regs->regITMR12.data = ((((int32_t)endcolor.r - (int32_t)initcolor.r) * (1 << 12)) / surface->dstHeight) * (1 << 4);
        regs->regITMR20.data = ((((int32_t)endcolor.g - (int32_t)initcolor.g) * (1 << 12)) / surface->dstHeight) * (1 << 4);
        regs->regITMR21.data = ((((int32_t)endcolor.b - (int32_t)initcolor.b) * (1 << 12)) / surface->dstHeight) * (1 << 4);
    }

    if (dir == GFX_GRAD_BOTH)
    {
        regs->regITMR00.data = ((((int32_t)endcolor.a - (int32_t)initcolor.a) * (1 << 11)) / surface->dstWidth) * (1 << 4);
        regs->regITMR01.data = ((((int32_t)endcolor.r - (int32_t)initcolor.r) * (1 << 11)) / surface->dstWidth) * (1 << 4);
        regs->regITMR02.data = ((((int32_t)endcolor.g - (int32_t)initcolor.g) * (1 << 11)) / surface->dstWidth) * (1 << 4);
        regs->regITMR10.data = ((((int32_t)endcolor.b - (int32_t)initcolor.b) * (1 << 11)) / surface->dstWidth) * (1 << 4);

        regs->regITMR11.data = ((((int32_t)endcolor.a - (int32_t)initcolor.a) * (1 << 11)) / surface->dstHeight) * (1 << 4);
        regs->regITMR12.data = ((((int32_t)endcolor.r - (int32_t)initcolor.r) * (1 << 11)) / surface->dstHeight) * (1 << 4);
        regs->regITMR20.data = ((((int32_t)endcolor.g - (int32_t)initcolor.g) * (1 << 11)) / surface->dstHeight) * (1 << 4);
        regs->regITMR21.data = ((((int32_t)endcolor.b - (int32_t)initcolor.b) * (1 << 11)) / surface->dstHeight) * (1 << 4);
    }

    // Mask
    REG_FILL_MASK_SURFACE(regs, surface->dstSurface, surface->dstX, surface->dstY, surface->dstWidth, surface->dstHeight);

    // Fire
    regs->regCMD.data = GFX_REG_CMD_GRADIENT_FILL;

    result            = gfxHwEngineFire(driver->hwDev);

end:
    return result;
}

static bool
_SurfaceProjection(
    GFXSurfaceDst *dstSurface,
    GFXSurfaceSrc *srcSurface,
    float         scaleWidth,
    float         scaleHeight,
    float         degree,
    float         FOV,
    float         pivotX,
    GFX_ROP3      rop,
    GFXAlphaBlend *alphaBlend)
{
    // degree: angle in radius on which the image is rotating (image rotates on the Y axis)
    // FOV: Field of view, ratio between the projection plane and the actual object
    // pivotX: Pivot point in X; indicates which point in x to use for rotation. 

    bool          result  = false;
    GFX_DRIVER    *driver = gfxGetDriver();
    GFX_HW_REGS   *regs   = &driver->hwDev->regs;

    GFX_RECTANGLE d;
    GFX_RECTANGLE s;
    GFX_MATRIX    mtx;
    GFX_MATRIX    inverseMtx;

    float         Xw;      // Holds the proportional width due to the rotation
    float         Yo;      // Holds the proportional height on the reduced side
    float         Rx;      // Delta width ( Rx + Xw = width)
    float         Po, Pon; // ratio of reduction affected by X pivot point offset
    float         rad = (degree * GFX_PI / 180.0f);

    gfxHwReset(driver->hwDev);

    memset((void *)&inverseMtx, 0, sizeof(GFX_MATRIX));
    gfxMatrixIdentify(&mtx);

    Xw   = cos(rad) * srcSurface->srcWidth;                                  // transformed width

    if(Xw >= 0)                                                              // width delta
        Rx = srcSurface->srcWidth - Xw;
    else
        Rx = srcSurface->srcWidth + Xw;

    Yo   = sin(rad) * (srcSurface->srcHeight / 2) * (FOV * GFX_PI / 180.0f); // height reduction due to horizontal angle
    Po   = pivotX / srcSurface->srcWidth;                                    // ratio of reduction affected by X pivot point offset
    Pon  = 1.0f - Po;                                                        // inverse ..

    //printf("cos(%f):%f\n",degree,cos(rad));
    //printf("sin(%f):%f\n",degree,sin(rad));
    //printf("FOV:%f\n",(FOV * GFX_PI / 180.0f));
    //printf("Xw:%f Rx:%f Yo:%f Po:%f Pon:%f\n",Xw,Rx,Yo,Po,Pon);

#if 1
    s.x0 = 0;
    s.y0 = srcSurface->srcHeight;
    s.x1 = srcSurface->srcWidth;
    s.y1 = srcSurface->srcHeight;
    s.x2 = srcSurface->srcWidth;
    s.y2 = 0;
    s.x3 = 0;
    s.y3 = 0;

    d.x0 = (Po * Rx);
    d.y0 = srcSurface->srcHeight + (Po * Yo);
    d.x1 = (Po * Rx) + Xw;
    d.y1 = srcSurface->srcHeight - (Pon * Yo);
    d.x2 = (Po * Rx) + Xw;
    d.y2 = (Pon * Yo);
    d.x3 = (Po * Rx);
    d.y3 = (Po * -Yo);
#else
    d.x0 = 0;
    d.y0 = dstSurface->dstHeight;
    d.x1 = dstSurface->dstWidth;
    d.y1 = dstSurface->dstHeight;
    d.x2 = dstSurface->dstWidth;
    d.y2 = 0;
    d.x3 = 0;
    d.y3 = 0;

    s.x0 = (Po * Rx);
    s.y0 = srcSurface->srcHeight + (Po * Yo);
    s.x1 = (Po * Rx) + Xw;
    s.y1 = srcSurface->srcHeight - (Pon * Yo);
    s.x2 = (Po * Rx) + Xw;
    s.y2 = (Pon * Yo);
    s.x3 = (Po * Rx);
    s.y3 = (Po * -Yo);
#endif

    //printf("s (%f,%f) (%f,%f) (%f,%f) (%f,%f)\n",s.x0,s.y0,s.x1,s.y1,s.x2,s.y2,s.x3,s.y3);
    //printf("d (%f,%f) (%f,%f) (%f,%f) (%f,%f)\n",d.x0,d.y0,d.x1,d.y1,d.x2,d.y2,d.x3,d.y3);
    gfxMatrixWarpQuadToQuad(d, s, &mtx, false);

    mtx.m[0][2] = dstSurface->dstX;
    mtx.m[1][2] = dstSurface->dstY;
    //gfxMatrixTranslate(&mtx, dstSurface->dstX, dstSurface->dstY);

    if (scaleWidth != 1.0 || scaleHeight != 1.0)
    {
        mtx.m[0][0] = mtx.m[0][0]*scaleWidth;
        mtx.m[1][1] = mtx.m[1][1]*scaleHeight;
        //gfxMatrixScale(&mtx, scaleWidth, scaleHeight);
    }

    if (dstSurface->dstSurface)
    {
        // TODO
        //REG_FILL_DST_SURFACE(regs, dstSurface->dstSurface, dstSurface->dstX+(int32_t)d.x0, dstSurface->dstY+(int32_t)d.y0, dstSurface->dstWidth, dstSurface->dstHeight);
        //REG_FILL_DST_SURFACE(regs, dstSurface->dstSurface, (int32_t)0, (int32_t)0, dstSurface->dstWidth, dstSurface->dstHeight);
        REG_FILL_DST_SURFACE(regs, dstSurface->dstSurface, dstSurface->dstX, dstSurface->dstY, dstSurface->dstWidth, dstSurface->dstHeight);
    }

    if (srcSurface->srcSurface)
    {
        // TODO
        //REG_FILL_SRC_SURFACE(regs, srcSurface->srcSurface, (int32_t)s.x3, (int32_t)s.y3, srcSurface->srcWidth, srcSurface->srcHeight);
        REG_FILL_SRC_SURFACE(regs, srcSurface->srcSurface, 0, 0, srcSurface->srcWidth, srcSurface->srcHeight);
    }

    gfxMatrixInverse(&mtx, &inverseMtx);
    REG_FILL_SET_ITM(regs, inverseMtx);

    // ROP3
    REG_FILL_ROP3(regs, rop);

    // About blending
    REG_FILL_BLENDING(regs, alphaBlend->enableAlpha, alphaBlend->enableConstantAlpha, alphaBlend->constantAlphaValue);

    // Foreground Color
    REG_FILL_FGCOLOR(regs, dstSurface->dstSurface->fgColor);

    // Mask
    REG_FILL_MASK_SURFACE(regs, dstSurface->dstSurface, dstSurface->dstX, dstSurface->dstY, dstSurface->dstWidth, dstSurface->dstHeight);
    //REG_FILL_MASK_SURFACE(regs, dstSurface->dstSurface, dstSurface->dstX+(int32_t)d.x0, dstSurface->dstY+(int32_t)d.y0, dstSurface->dstWidth, dstSurface->dstHeight);

    // Control 1
    regs->regCR1.data |= GFX_REG_CR1_TRANSFORM_EN
        | ((GFX_TILE_FILL << GFX_REG_CR1_TILEMODE_OFFSET) & GFX_REG_CR1_TILEMODE_MASK);

    regs->regCR1.data |= GFX_REG_CR1_PERSPECTIVE_EN;

    if (dstSurface->dstSurface)
    {
        if (dstSurface->dstSurface->quality == GFX_QUALITY_BETTER)
        {
            regs->regCR1.data |= GFX_REG_CR1_INTERPOLATE_EN;
            regs->regCR1.data |= GFX_REG_CR1_ALPHABLEND_EN;
        }
    }

    // Clipping
    if (dstSurface->dstSurface->clipEnable)
    {
        REG_FILL_DST_CLIP(regs,
                          dstSurface->dstSurface->clipSet.left,
                          dstSurface->dstSurface->clipSet.top,
                          dstSurface->dstSurface->clipSet.right,
                          dstSurface->dstSurface->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
    }

    // Fire
    regs->regCMD.data = GFX_REG_CMD_ROP3;

    result            = gfxHwEngineFire(driver->hwDev);

    return result;
}

static bool
_SurfaceTransformBlt(
    GFXSurface* dest,
    int32_t dx,
    int32_t dy,
    GFXSurface* src,
    int32_t sx,
    int32_t sy,
    int32_t sw,
    int32_t sh,
    int32_t x0, int32_t y0,
    int32_t x1, int32_t y1,
    int32_t x2, int32_t y2,
    int32_t x3, int32_t y3,
    bool    bInverse,
    GFXPageFlowType type,
    GFXTransformType transformType)
{
    bool          result = false;
    GFX_DRIVER    *driver = gfxGetDriver();
    GFX_HW_REGS   *regs = &driver->hwDev->regs;

    GFX_MATRIX    mtx;
    GFX_MATRIX    inverseMtx;

    float         Xw, w, h, offsetx, offsety;
    int32_t       minX, minY;
    int32_t       newDestWidth = dest->width - dx;
    int32_t       newDestheight = dest->height - dy;

    gfxHwReset(driver->hwDev);

    memset((void *)&inverseMtx, 0, sizeof(GFX_MATRIX));
    gfxMatrixIdentify(&mtx);

    //printf("(%d %d) (%d %d) (%d %d) (%d %d)\n", x0, y0, x1, y1, x2, y2, x3, y3);

    w = (float)src->width;
    h = (float)src->height;

    offsetx = (float)(src->width - abs(x2));
    if (offsetx == 0)
    {
        Xw = (float)x1 - (float)x3;

        if(Xw >= 0)
            offsetx = (float)src->width - Xw;
        else
            offsetx = (float)src->width + Xw;
    }

    minX = MIN(x0, x1);
    minY = MIN(y0, y1);

    if (transformType == GFX_TRANSFORM_TURN_LEFT || transformType == GFX_TRANSFORM_TURN_RIGHT) // horizontal
    {
        if (bInverse)
        {
            if ((type == GFX_PAGEFLOW_FOLD || type == GFX_PAGEFLOW_FOLD2) && y1 == 0)
            {
                offsety = (float)y0;

                //printf("offset 1(%f,%f)\n", offsetx, offsety);
                //printf("bInverse minX:%d, minY:%d\n", minX, minY);
                mtx.m[0][0] = 1 - (offsetx / w) - ((2 * offsety * (w - offsetx)) / (w * h));
                mtx.m[0][1] = 0;
                mtx.m[0][2] = 0;
                mtx.m[1][0] = (-offsety) / w;
                mtx.m[1][1] = 1 - ((2 * offsety) / h);
                mtx.m[1][2] = offsety - (minY*mtx.m[1][1]);
                mtx.m[2][0] = (-2 * offsety) / (w * h);
                mtx.m[2][1] = 0;
                mtx.m[2][2] = 1;
            }
            else
            {
                offsety = (float)(-y1);

                //printf("offset 2(%f,%f)\n", offsetx, offsety);
                //printf("bInverse minX:%d, minY:%d\n", minX, minY);
                mtx.m[0][0] = 1 - (offsetx / w) - ((2 * offsety * (w - offsetx)) / (w * (h + (2 * offsety))));
                mtx.m[0][1] = 0;
                mtx.m[0][2] = 0;
                mtx.m[1][0] = ((2 * offsety * offsety) / (w * (h + (2 * offsety)))) - (offsety / w);
                mtx.m[1][1] = 1;
                mtx.m[1][2] = -(minY*mtx.m[1][1]);
                mtx.m[2][0] = -(2 * offsety) / (w * (h + (2 * offsety)));
                mtx.m[2][1] = 0;
                mtx.m[2][2] = 1;

                if (type == GFX_PAGEFLOW_FOLD)
                    newDestWidth = x1 / 2;
            }
        }
        else
        {
            if ((type == GFX_PAGEFLOW_FOLD || type == GFX_PAGEFLOW_FOLD2) && y0 == 0)
            {
                offsety = (float)y1;

                //printf("offset 3(%f,%f)\n", offsetx, offsety);
                //printf("minX:%d, minY:%d\n", minX, minY);
                mtx.m[0][0] = 1 - (offsetx / w) - ((2 * offsety) / ((2 * offsety) - h));
                mtx.m[0][1] = 0;
                mtx.m[0][2] = offsetx;
                mtx.m[1][0] = (offsety / w) - ((2 * offsety * offsety) / (w * ((2 * offsety) - h)));
                mtx.m[1][1] = 1;
                mtx.m[1][2] = -(minY*mtx.m[1][1]);
                mtx.m[2][0] = -(2 * offsety) / (w * ((2 * offsety) - h));
                mtx.m[2][1] = 0;
                mtx.m[2][2] = 1;

                if (type == GFX_PAGEFLOW_FOLD)
                    newDestWidth = (x1 - x0) / 2 + x0;
            }
            else
            {
                offsety = (float)(-y0);

                //printf("offset 4(%f,%f)\n", offsetx, offsety);
                //printf("minX:%d, minY:%d\n", minX, minY);
                mtx.m[0][0] = 1 - (offsetx / w) + ((2 * offsety) / h);
                mtx.m[0][1] = 0;
                mtx.m[0][2] = offsetx;
                mtx.m[1][0] = (offsety / w);
                mtx.m[1][1] = 1 + ((2 * offsety) / h);
                mtx.m[1][2] = (-offsety) - (minY*mtx.m[1][1]);
                mtx.m[2][0] = (2 * offsety) / (w * h);
                mtx.m[2][1] = 0;
                mtx.m[2][2] = 1;
            }
        }
    }
    else if (transformType == GFX_TRANSFORM_TURN_TOP || transformType == GFX_TRANSFORM_TURN_BOTTOM) // vertical
    {
        if (bInverse) //Turn to top side
        {
            offsety = (float)y0;

            //printf("vertical offset 0(%f,%f)\n", offsetx, offsety);
            //printf("bInverse minX:%d, minY:%d\n", minX, minY);
            mtx.m[0][0] = 1 - (2 * offsetx / w);
            mtx.m[0][1] = -(offsetx / h);
            mtx.m[0][2] = offsetx;
            mtx.m[1][0] = 0;
            mtx.m[1][1] = 1 - ((2 * offsety) / h) - ((2 * offsetx * (h - offsety)) / (w * h));
            mtx.m[1][2] = offsety;
            mtx.m[2][0] = 0;
            mtx.m[2][1] = -((2 * offsetx) / (w * h));
            mtx.m[2][2] = 1;
        }
        else //Turn to bottom side
        {
            offsety = (float)y1;

            //printf("vertical offset 1(%f,%f)\n", offsetx, offsety);
            //printf("minX:%d, minY:%d\n", minX, minY);
            mtx.m[0][0] = 1;
            mtx.m[0][1] = (offsetx / h) + ((2 * offsetx * offsetx) / (h * (w - (2 * offsetx))));
            mtx.m[0][2] = 0;
            mtx.m[1][0] = 0;
            mtx.m[1][1] = 1 - ((2 * offsety) / h) + ((2 * offsetx * (h - offsety)) / (h * (w - (2 * offsetx))));
            mtx.m[1][2] = offsety;
            mtx.m[2][0] = 0;
            mtx.m[2][1] = ((2 * offsetx) / (h * (w - (2 * offsetx))));
            mtx.m[2][2] = 1;
        }

        minX = minY = 0;
    }


    if (type == GFX_PAGEFLOW_FOLD)
        sw = src->width;

    if (!offsetx && !offsety)
    {
        newDestWidth = w;
        newDestheight = h;
    }

    if (dest)
    {
        //printf("dest(%d %d) (%d %d) (%d %d)\n", dx, dy, newDestWidth, newDestheight, dest->width, dest->height);
        REG_FILL_DST_SURFACE(regs, dest, dx, dy + minY, newDestWidth, newDestheight - minY);
    }

    if (src)
    {
        //printf("src(%d %d)(%d %d) (%d %d)\n\n",sx, sy, sw, sh, src->width, src->height);
        REG_FILL_SRC_SURFACE(regs, src, sx, sy, sw, sh);
    }

    //printf("++++Last++\n");
    //printf("%f %f %f\n",mtx.m[0][0],mtx.m[0][1],mtx.m[0][2]);
    //printf("%f %f %f\n",mtx.m[1][0],mtx.m[1][1],mtx.m[1][2]);
    //printf("%f %f %f\n\n",mtx.m[2][0],mtx.m[2][1],mtx.m[2][2]);

    gfxMatrixInverse(&mtx, &inverseMtx);
    REG_FILL_SET_ITM(regs, inverseMtx);

    // ROP3
    REG_FILL_ROP3(regs, GFX_ROP3_SRCCOPY);

    // About blending
    REG_FILL_BLENDING(regs, 0, 0, 0);

    //// Foreground Color
    //REG_FILL_FGCOLOR(regs, dstSurface->dstSurface->fgColor);

    // Mask
    REG_FILL_MASK_SURFACE(regs, dest, dx, dy + minY, newDestWidth, newDestheight - minY);
    
    // Control 1
    regs->regCR1.data |= GFX_REG_CR1_TRANSFORM_EN
        | ((GFX_TILE_FILL << GFX_REG_CR1_TILEMODE_OFFSET) & GFX_REG_CR1_TILEMODE_MASK);

    regs->regCR1.data |= GFX_REG_CR1_PERSPECTIVE_EN;

    if (dest)
    {
        if (dest->quality == GFX_QUALITY_BETTER)
        {
            regs->regCR1.data |= GFX_REG_CR1_INTERPOLATE_EN;
            regs->regCR1.data |= GFX_REG_CR1_ALPHABLEND_EN;
        }
    }

    // Clipping
    if (dest->clipEnable)
    {
        dest->clipSet.top = dest->clipSet.top + minY;

        if (dest->clipSet.top < 0)
            dest->clipSet.top = 0;

        REG_FILL_DST_CLIP(regs,
            dest->clipSet.left,
            dest->clipSet.top,
            dest->clipSet.right,
            dest->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
        //printf("clipSet(%d %d) (%d %d)\n\n", dest->clipSet.left, dest->clipSet.top, dest->clipSet.right, dest->clipSet.bottom);
    }

    // Fire
    regs->regCMD.data = GFX_REG_CMD_ROP3;

    result            = gfxHwEngineFire(driver->hwDev);

    return result;
}

static void
_setFGColor(
    GFXSurface *dstSurface,
    GFXColor   color)
{
    if (dstSurface)
    {
        dstSurface->fgColor.a = color.a;
        dstSurface->fgColor.r = color.r;
        dstSurface->fgColor.g = color.g;
        dstSurface->fgColor.b = color.b;
    }
}

static bool
_SurfaceDrawLine(
    GFXSurface *surface,
    int32_t    fromX,
    int32_t    fromY,
    int32_t    toX,
    int32_t    toY,
    GFXColor   *lineColor,
    int32_t    lineWidth)
{
    bool        result  = false;

#if CFG_CHIP_FAMILY != 9850
    GFX_DRIVER  *driver = gfxGetDriver();
    GFX_HW_REGS *regs   = &driver->hwDev->regs;

    gfxHwReset(driver->hwDev);

    // FIXME
    if (surface)
    {
        REG_FILL_DST_SURFACE(regs, surface, (int32_t)0, (int32_t)0, surface->width, surface->height);
    }

    // Clipping
    if (surface->clipEnable)
    {
        REG_FILL_DST_CLIP(regs,
                          surface->clipSet.left,
                          surface->clipSet.top,
                          surface->clipSet.right,
                          surface->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
    }

    // Start x,y
    REG_FILL_LINEDRAW_STARTXY(fromX, fromY);

    // End x,y
    REG_FILL_LINEDRAW_ENDXY(toX, toY);

    // Line Color
    REG_FILL_FGCOLOR(regs, (*lineColor));

    // Mask
    REG_FILL_MASK_SURFACE(regs, surface, 0, 0, surface->width, surface->height);

    // Fire
    regs->regCMD.data = GFX_REG_CMD_LINE_DRAW;

    result            = gfxHwEngineFire(driver->hwDev);
#else

    int32_t dx = toX - fromX;
    int32_t dy = toY - fromY;
    int32_t steps, k = 0;

    float   xincrement, yincrement = 0.0;
    float   x = fromX;
    float   y = fromY;

    if (abs(dx) > abs(dy))
        steps = abs(dx);
    else
        steps = abs(dy);

    xincrement = dx / (float)steps;
    yincrement = dy / (float)steps;

    result     = _SurfaceDrawPixel(surface, (int)round(x), (int)round(y), lineColor, lineWidth);

    for (k = 0; k < steps; k++)
    {
        x     += xincrement;
        y     += yincrement;
        result = _SurfaceDrawPixel(surface, (int)round(x), (int)round(y), lineColor, lineWidth);
    }

    //result = false;
#endif

    return result;
}

#if CFG_CHIP_FAMILY == 9850
static bool
_SurfaceDrawPixel(
    GFXSurface *surface,
    int        x,
    int        y,
    GFXColor   *color,
    int        lineWidth)
{
    bool          result     = false;
    GFXAlphaBlend alphaBlend = {0};
    GFXSurfaceDst dstSurface = {0};
    GFXColor      pixelcolor = {0};

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    dstSurface.dstSurface          = surface;
    dstSurface.dstX                = x;
    dstSurface.dstY                = y;
    dstSurface.dstWidth            = lineWidth;
    dstSurface.dstHeight           = lineWidth;

    alphaBlend.enableAlpha         = true;
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;

    pixelcolor.a                   = color->a;
    pixelcolor.r                   = color->r;
    pixelcolor.g                   = color->g;
    pixelcolor.b                   = color->b;

    result                         = _SurfaceFillColor(&dstSurface, pixelcolor, &alphaBlend);

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}
#endif

static bool
_SurfaceDrawPixelAA(
    GFXSurface *surface,
    int        x,
    int        y,
    GFXColor   *color,
    int        lineWidth)
{
    bool          result     = false;
    GFXAlphaBlend alphaBlend = {0};
    GFXSurfaceDst dstSurface = {0};
    GFXColor      pixelcolor = {0};
    float         d, alpha;
    int32_t       x1, y1, x2, y2;
    int32_t       px, py;
    int32_t       cr;

    GFX_FUNC_ENTRY;
    if (surface == NULL)
    {
        GFX_ERROR_MSG("Invalid destination surface.\n");
        return false;
    }

    gfxLock();

    alphaBlend.enableAlpha         = true;
    alphaBlend.enableConstantAlpha = false;
    alphaBlend.constantAlphaValue  = 0;

    alpha                          = 2 * (color->a) / lineWidth;
    if (alpha > 255)
        alpha = 255;

    pixelcolor.r = color->r;
    pixelcolor.g = color->g;
    pixelcolor.b = color->b;

    //radius = 1.1;
    cr           = floor(lineWidth / 1.5);

    x1           = floor(x - lineWidth + 0.3);
    y1           = floor(y - lineWidth + 0.3);
    x2           = floor(x + lineWidth - 0.3);
    y2           = floor(y + lineWidth - 0.3);

    //printf("cr:%d, x1:%d,y1:%d,x2:%d,y2:%d\n", cr, x1, y1, x2, y2);
    for (py = y1, px = x1; py <= y2 && px <= x2; py++, px++)
    {
        d = sqrt(pow(px - x, 2) + pow(py - y, 2));
        d = d / lineWidth;
        //printf("d:%f\n",d);
        if (d < 1)
        {
            d                     = pow(d, cr);

            dstSurface.dstSurface = surface;
            dstSurface.dstX       = px;
            dstSurface.dstY       = py;
            dstSurface.dstWidth   = lineWidth;
            dstSurface.dstHeight  = lineWidth;

            pixelcolor.a          = floor(alpha * (1 - d));
            //printf("pixelcolor.a:%d\n", pixelcolor.a);
            result                = _SurfaceFillColor(&dstSurface, pixelcolor, &alphaBlend);
        }
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return result;
}

static bool
_SurfaceReflected(
    GFXSurface* dest,
    int32_t dx,
    int32_t dy,
    GFXSurface* src,
    int32_t sx,
    int32_t sy,
    int32_t reflectedWidth,
    int32_t reflectedHeight)
{
    bool          result = false;
    GFX_DRIVER    *driver = gfxGetDriver();
    GFX_HW_REGS   *regs = &driver->hwDev->regs;

    GFX_MATRIX    mtx;
    GFX_MATRIX    inverseMtx;

    gfxHwReset(driver->hwDev);

    memset((void *)&inverseMtx, 0, sizeof(GFX_MATRIX));
    gfxMatrixIdentify(&mtx);
  
    mtx.m[0][0] = 1;
    mtx.m[0][1] = 0;
    mtx.m[0][2] = 0;
    mtx.m[1][0] = 0;
    mtx.m[1][1] = -1;
    mtx.m[1][2] = reflectedHeight;
    mtx.m[2][0] = 0;
    mtx.m[2][1] = 0;
    mtx.m[2][2] = 1;
       
    if (dest)
    {
        //printf("dest(%d %d) (%d %d)\n", dx, dy, dest->width, dest->height);
        REG_FILL_DST_SURFACE(regs, dest, dx, dy, reflectedWidth, reflectedHeight);
    }

    if (src)
    {
        //printf("src(%d %d) (%d %d)\n", sx, sy, src->width, src->height);
        REG_FILL_SRC_SURFACE(regs, src, sx, sy, reflectedWidth, reflectedHeight);
    }

    //printf("++++Last++\n");
    //printf("%f %f %f\n", mtx.m[0][0], mtx.m[0][1], mtx.m[0][2]);
    //printf("%f %f %f\n", mtx.m[1][0], mtx.m[1][1], mtx.m[1][2]);
    //printf("%f %f %f\n\n", mtx.m[2][0], mtx.m[2][1], mtx.m[2][2]);

    gfxMatrixInverse(&mtx, &inverseMtx);
    REG_FILL_SET_ITM(regs, inverseMtx);

    // ROP3
    REG_FILL_ROP3(regs, GFX_ROP3_SRCCOPY);

    // About blending
    REG_FILL_BLENDING(regs, 0, 0, 0);

    //// Foreground Color
    //REG_FILL_FGCOLOR(regs, dest->fgColor);

    // Mask
    REG_FILL_MASK_SURFACE(regs, dest, 0, 0, reflectedWidth, reflectedHeight);

    // Control 1
    regs->regCR1.data |= GFX_REG_CR1_TRANSFORM_EN
        | ((GFX_TILE_FILL << GFX_REG_CR1_TILEMODE_OFFSET) & GFX_REG_CR1_TILEMODE_MASK);

    if (dest)
    {
        if (0 && dest->quality == GFX_QUALITY_BETTER)
        {
            regs->regCR1.data |= GFX_REG_CR1_INTERPOLATE_EN;
            regs->regCR1.data |= GFX_REG_CR1_ALPHABLEND_EN;
        }
    }

    // Clipping
    if (dest->clipEnable)
    {
        REG_FILL_DST_CLIP(regs,
            dest->clipSet.left,
            dest->clipSet.top,
            dest->clipSet.right,
            dest->clipSet.bottom);
        regs->regCR1.data |= GFX_REG_CR1_CLIP_EN;
    }

    // Fire
    regs->regCMD.data = GFX_REG_CMD_ROP3;

    result = gfxHwEngineFire(driver->hwDev);

    return result;
}
