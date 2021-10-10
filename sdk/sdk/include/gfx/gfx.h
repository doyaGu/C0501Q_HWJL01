/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file gfx.h
 *  GFX driver API header file.
 *
 * @author Awin Huang
 * @version 1.0
 */
#ifndef __GFX_H__
#define __GFX_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * DLL export API declaration for Win32.
 */

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef enum _GFX_ROP3
{
    GFX_ROP3_BLACKNESS      = 0x00,
    GFX_ROP3_NOTSRCERASE    = 0x11,
    GFX_ROP3_NOTSRCCOPY     = 0x33,
    GFX_ROP3_SRCERASE       = 0x44,
    GFX_ROP3_DSTINVERT      = 0x55,
    GFX_ROP3_SRCINVERT      = 0x66,
    GFX_ROP3_SECAND         = 0x88,
    GFX_ROP3_PAINTINVERT    = 0x5A,
    GFX_ROP3_MERGEPAINT     = 0xBB,
    GFX_ROP3_MERGECOPY      = 0xC0,
    GFX_ROP3_SRCCOPY        = 0xCC,
    GFX_ROP3_SRCPAINT       = 0xEE,
    GFX_ROP3_PATCOPY        = 0xF0,
    GFX_ROP3_PATPAINT       = 0xFB,
    GFX_ROP3_WHITENESS      = 0xFF,
} GFX_ROP3;

typedef enum
{
    GFX_COLOR_RGBA8888,     ///< 0x00
    GFX_COLOR_ARGB8888,     ///< 0x01
    GFX_COLOR_BGRA8888,     ///< 0x02
    GFX_COLOR_ABGR8888,     ///< 0x03
    GFX_COLOR_RGBA5551,     ///< 0x04
    GFX_COLOR_ARGB1555,     ///< 0x05
    GFX_COLOR_BGRA5551,     ///< 0x06
    GFX_COLOR_ABGR1555,     ///< 0x07
    GFX_COLOR_RGBA4444,     ///< 0x08
    GFX_COLOR_ARGB4444,     ///< 0x09
    GFX_COLOR_BGRA4444,     ///< 0x0a
    GFX_COLOR_ABGR4444,     ///< 0x0b
    GFX_COLOR_RGB565,       ///< 0x0c
    GFX_COLOR_BGR565,       ///< 0x0d
    GFX_COLOR_A_8,          ///< 0x0e
    GFX_COLOR_A_4,          ///< 0x0f
    GFX_COLOR_A_2,          ///< 0x10
    GFX_COLOR_A_1,          ///< 0x11
    GFX_COLOR_UNKNOWN,      ///< Some Error!
} GFXColorFormat;

typedef enum
{
    GFX_MASK_A_8,           ///< 0x00
    GFX_MASK_A_4,           ///< 0x01
    GFX_MASK_A_2,           ///< 0x02
    GFX_MASK_A_1,           ///< 0x03
    GFX_MASK_UNKNOWN,       ///< Some Error!
} GFXMaskFormat;

typedef enum
{
    GFX_GRAD_H,             ///< 0
    GFX_GRAD_V,             ///< 1
    GFX_GRAD_BOTH,          ///< 2
    GFX_GRAD_UNKNOWN,       ///< Error
} GFXGradDir;

typedef enum
{
    GFX_TILE_FILL,          ///< 0
    GFX_TILE_PAD,           ///< 1
    GFX_TILE_REPEAT,        ///< 2
    GFX_TILE_REFLECT,       ///< 3
    GFX_TILE_UNKNOWN,       ///< Error
} GFXTileMode;

typedef enum
{
    GFX_QUALITY_BETTER,     ///< 0
    GFX_QUALITY_FASTER      ///< 1
} GFXQualityMode;

/**
 * PageFlow type definition.
 */
typedef enum
{
    GFX_PAGEFLOW_FOLD,  ///< Fold effect
    GFX_PAGEFLOW_FLIP,  ///< Flip effect
    GFX_PAGEFLOW_FLIP2, ///< Flip 2 pieces effect
    GFX_PAGEFLOW_FOLD2  ///< Fold 2 effect, no clipping destWidth
} GFXPageFlowType;


/**
* Transform type definition.
*/
typedef enum
{
    GFX_TRANSFORM_NONE = 0,        ///< No transform
    GFX_TRANSFORM_TURN_LEFT = 1,   ///< Turn to left side
    GFX_TRANSFORM_TURN_TOP = 2,    ///< Turn to top side
    GFX_TRANSFORM_TURN_RIGHT = 3,  ///< Turn to right side
    GFX_TRANSFORM_TURN_BOTTOM = 4  ///< Turn to bottom side
} GFXTransformType;

typedef struct _GFXColor
{
    uint8_t         a;
    uint8_t         r;
    uint8_t         g;
    uint8_t         b;
} GFXColor;

typedef struct _GFXRectangle
{
    int32_t         left;       ///< the x-coordinate of upper-left corner of the rectangle
    int32_t         top;        ///< the y-coordiante of upper-left corner of the rectangle
    int32_t         right;      ///< the x-coordinate of lower-right corner of the rectangle
    int32_t         bottom;     ///< the y-coordinate of lower-right corner of the rectangle
} GFXRectangle;

typedef struct _GFXMaskSurface
{
    int32_t         width;           ///< the width of mask surface
    int32_t         height;          ///< the height of mask surface
    uint32_t        pitch;           ///< the pitch of mask surface
    GFXMaskFormat   format;          ///< the format of mask surface
    uint8_t*        imageData;       ///< the pointer of image data
    uint32_t        imageDataLength; ///< the length of image data
    bool            imageDataOwner;  ///< the enable flag of image data
} GFXMaskSurface;

typedef struct _GFXSurface
{
    int32_t         width;           ///< the width of surface
    int32_t         height;          ///< the height of surface
    uint32_t        pitch;           ///< the pitch of surface
    GFXColorFormat  format;          ///< the format of surface
    unsigned int    flags;           ///< Flags; can be union of ITU_STATIC and ITU_CLIPPING
    GFXColor        fgColor;         ///< the foreground color of surface
    GFXRectangle    clipSet;         ///< the area of clipping
    uint8_t*        imageData;       ///< the pointer of image data
    uint32_t        imageDataLength; ///< the length of image data
    bool            imageDataOwner;  ///< the enable flag of image data
    bool            maskEnable;      ///< the enable flag of mask surface
    GFXMaskSurface* maskSurface;     ///< the pointer of mask surface
    bool            clipEnable;      ///< the enable flag of clipping    
    GFXQualityMode  quality;         ///< the quality mode
} GFXSurface;

typedef struct _GFXSurfaceSrc
{
	GFXSurface*   srcSurface; ///< the pointer of source surface
    int32_t       srcX;       ///< the x-coordinate of source surface
    int32_t       srcY;       ///< the y-coordinate of source surface
    int32_t       srcWidth;   ///< the width of source surface
    int32_t       srcHeight;  ///< the height of source surface
} GFXSurfaceSrc;

typedef struct _GFXSurfaceDst
{
	GFXSurface*   dstSurface; ///< the pointer of destination surface
    int32_t       dstX;       ///< the x-coordinate of destination surface
    int32_t       dstY;       ///< the y-coordinate of destination surface
    int32_t       dstWidth;   ///< the width of destination surface
    int32_t       dstHeight;  ///< the height of destination surface
} GFXSurfaceDst;

typedef struct _GFXAlphaBlend
{
	bool    enableAlpha;          ///< the enable flag of alpha blending
    bool    enableConstantAlpha;  ///< the enable flag of constant alpha
    uint8_t constantAlphaValue;   ///< the constant alpha value
} GFXAlphaBlend;

typedef struct _GFXCoordinates
{
    int32_t       x;  ///< the x-coordinate
    int32_t       y;  ///< the y-coordinate
}GFXCoordinates;

//=============================================================================
//                              Function Declaration
//=============================================================================
/**
 * This routine is used to create surface with specified width, height, pitch and format.
 *
 * @param width           The image width of buffer.
 * @param height          The image height of buffer.
 * @param pitch           The image pitch of buffer.
 * @param format          The image format of buffer.
 *
 * @return a valid pointer of GFXSurface if succeed, otherwise return NULL.
 */
GFXSurface*
gfxCreateSurface(
    uint32_t        width,
    uint32_t        height,
    uint32_t        pitch,
    GFXColorFormat  format);

/**
 * This routine is used to create surface with already exist buffer,
 * and the buffer "WILL BE COPIED" into the internal buffer of created surface.
 *
 * @width           The image width of buffer.
 * @height          The image height of buffer.
 * @pitch           The image pitch of buffer.
 * @format          The image format of buffer.
 * @buffer          The buffer pointer which will be copied into surface. This buffer will be released in gfxDestroySurface().
 * @bufferLength    The buffer length of buffer. Counted in byte.
 *
 * @return a valid pointer of GFXSurface if succeed, otherwise return NULL.
 */
GFXSurface*
gfxCreateSurfaceByBuffer(
    uint32_t        width,
    uint32_t        height,
    uint32_t        pitch,
    GFXColorFormat  format,
    uint8_t*        buffer,
    uint32_t        bufferLength);

/**
 * This routine is used to create surface with already exist buffer,
 * and "WON'T" COPY the buffer.
 *
 * @width           The image width of buffer.
 * @height          The image height of buffer.
 * @pitch           The image pitch of buffer.
 * @format          The image format of buffer.
 * @alreadyExistPtr The buffer pointer which will be assigned to surface. This pointer will "NOT" be released in gfxDestroySurface().
 * @ptrLength       The buffer length of alreadyExistPtr. Counted in byte.
 *
 * @return a valid pointer of GFXSurface if succeed, otherwise return NULL.
 */
GFXSurface*
gfxCreateSurfaceByPointer(
    uint32_t        width,
    uint32_t        height,
    uint32_t        pitch,
    GFXColorFormat  format,
    uint8_t*        alreadyExistPtr,
    uint32_t        ptrLength);

/**
 * This routine is used to destroy surface.
 *
 * @param surface   The valid pointer of GFXSurface.
 *
 * @return void.
 */
void
gfxDestroySurface(
    GFXSurface* surface);

/**
 * This routine is used to get width of surface.
 *
 * @param surface   The valid pointer of GFXSurface.
 *
 * @return width of surface.
 */
int
gfxSurfaceGetWidth(
    GFXSurface* surface);

/**
 * This routine is used to get height of surface.
 *
 * @param surface   The valid pointer of GFXSurface.
 *
 * @return height of surface.
 */
int
gfxSurfaceGetHeight(
    GFXSurface* surface);

/**
 * This routine is used to get color format of surface.
 *
 * @param surface   The valid pointer of GFXSurface.
 *
 * @return color format of surface.
 */
GFXColorFormat
gfxSurfaceGetFormat(
    GFXSurface* surface);

/**
 * This routine is used to set surface address.
 *
 * @param surface   The valid pointer of GFXSurface.
 * @param addr      The address.
 *
 * @return void.
 */
void
gfxSurfaceSetSurfaceBaseAddress(
    GFXSurface* surface,
    uint32_t    addr);

/**
 * This routine is used to set surface width.
 *
 * @param surface   The valid pointer of GFXSurface.
 * @param addr      The width.
 *
 * @return void.
 */
void
gfxSurfaceSetWidth(
    GFXSurface* surface,
    uint32_t    width);

/**
 * This routine is used to set surface height.
 *
 * @param surface   The valid pointer of GFXSurface.
 * @param addr      The height.
 *
 * @return void.
 */
void
gfxSurfaceSetHeight(
    GFXSurface* surface,
    uint32_t    height);

/**
 * This routine is used to set surface pitch.
 *
 * @param surface   The valid pointer of GFXSurface.
 * @param addr      The pitch.
 *
 * @return void.
 */
void
gfxSurfaceSetPitch(
    GFXSurface* surface,
    uint32_t    pitch);

/**
 * This routine is used to bitblt source surface to destination surface.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param dstX         The X coordinates of destination GFXSurface.
 * @param dstY         The Y coordinates of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceBitblt(
    GFXSurface*    dstSurface,
    int32_t        dstX,
    int32_t        dstY,
    GFXSurfaceSrc* srcSurface);

/**
 * This routine is used to bitblt source surface to destination surface with ROP3.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param dstX         The X coordinates of destination GFXSurface.
 * @param dstY         The Y coordinates of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param rop          The ternary raster-operation codes.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceBitbltWithRop(
    GFXSurface*    dstSurface,
    int32_t        dstX,
    int32_t        dstY,
    GFXSurfaceSrc* srcSurface,
    GFX_ROP3       rop);

/**
 * This routine is used to draw glyph.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param dstX         The X coordinates of destination GFXSurface.
 * @param dstY         The Y coordinates of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param color        The a,r,g,b color.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceDrawGlyph(
    GFXSurface*    dstSurface,
    int32_t        dstX,
    int32_t        dstY,
    GFXSurfaceSrc* srcSurface,
    GFXColor       color);

/**
 * This routine is used to transform effects.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param refX         The X coordinates of rotate.
 * @param refY         The Y coordinates of rotate.
 * @param scaleWidth   The ratio of width scaling.
 * @param scaleHeight  The ratio of height scaling.
 * @param degree       The degree of rotate.
 * @param tilemode     The tile mode.
 * @param rop          The ternary raster-operation codes.
 * @param alphaBlend   The alpha blending effects.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceTransform(
    GFXSurfaceDst* dstSurface,   
    GFXSurfaceSrc* srcSurface,
    int32_t        refX,
    int32_t        refY,
    float          scaleWidth,
    float          scaleHeight,
    float          degree,
    GFXTileMode    tilemode,
    GFX_ROP3       rop,
    GFXAlphaBlend* alphaBlend);

/**
 * This routine is used to bitblt source surface to destination surface with rotate.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param dstX         The X coordinates of destination GFXSurface.
 * @param dstY         The Y coordinates of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param refX         The X coordinates of rotate.
 * @param refY         The Y coordinates of rotate.
 * @param degree       The degree of rotate.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceBitbltWithRotate(
    GFXSurface* dstSurface,
    int32_t     dstX,
    int32_t     dstY,
    GFXSurfaceSrc* srcSurface,
    int32_t     refX,
    int32_t     refY,
    float       degree);

/**
 * This routine is used to source surface copy to destination surface.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceStrectch(
    GFXSurfaceDst* dstSurface,
    GFXSurfaceSrc* srcSurface);

/**
 * This routine is used to source surface copy to destination surface with rotate.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param refX         The X coordinates of rotate.
 * @param refY         The Y coordinates of rotate.
 * @param degree       The degree of rotate.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceStrectchWithRotate(
    GFXSurfaceDst* dstSurface,
    GFXSurfaceSrc* srcSurface,
    int32_t     refX,
    int32_t     refY,
    float       degree);

/**
 * This routine is used to bitblt source surface to destination surface with alpha blending.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param dstX         The X coordinates of destination GFXSurface.
 * @param dstY         The Y coordinates of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceAlphaBlend(
    GFXSurface*    dstSurface,
    int32_t        dstX,
    int32_t        dstY,
    GFXSurfaceSrc* srcSurface);

/**
 * This routine is used to bitblt source surface to destination surface with constant alpha value.
 *
 * @param dstSurface           The valid pointer of destination GFXSurface.
 * @param dstX                 The X coordinates of destination GFXSurface.
 * @param dstY                 The Y coordinates of destination GFXSurface.
 * @param srcSurface           The valid pointer of source GFXSurface.
 * @param enableConstantAlpha  Enable constant alpha or not.
 * @param constantAlphaValue   The constant alpha value.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceAlphaBlendEx(
    GFXSurface*    dstSurface,
    int32_t        dstX,
    int32_t        dstY,
    GFXSurfaceSrc* srcSurface,
    bool           enableConstantAlpha,
    uint8_t        constantAlphaValue);

/**
 * This routine is used to bitblt source surface to destination surface with alpha blending and rotate.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param dstX         The X coordinates of destination GFXSurface.
 * @param dstY         The Y coordinates of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param refX         The X coordinates of rotate.
 * @param refY         The Y coordinates of rotate.
 * @param degree       The degree of rotate.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceAlphaBlendWithRotate(
    GFXSurface*    dstSurface,
    int32_t        dstX,
    int32_t        dstY,
    GFXSurfaceSrc* srcSurface,
    int32_t        refX,
    int32_t        refY,
    float          degree);

/**
 * This routine is used to source surface copy to destination surface with alpha blending.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceStrectchAlphaBlend(
    GFXSurfaceDst* dstSurface,
    GFXSurfaceSrc* srcSurface);

/**
 * This routine is used to source surface copy to destination surface with alpha blending and rotate.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param refX         The X coordinates of rotate.
 * @param refY         The Y coordinates of rotate.
 * @param degree       The degree of rotate.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceStrectchAlphaBlendWithRotate(
    GFXSurfaceDst* dstSurface,
    GFXSurfaceSrc* srcSurface,
    int32_t        refX,
    int32_t        refY,
    float          degree);

/**
 * This routine is used to fill a rectangular area with color.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param color        The a,r,g,b color.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceFillColor(
    GFXSurfaceDst* dstSurface,
    GFXColor       color);

/**
 * This routine is used to fill a rectangular area with color and alpha blending.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param color        The a,r,g,b color.
 * @param alphaBlend   The alpha blending effects.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceFillColorWithBlend(
    GFXSurfaceDst* dstSurface,
    GFXColor       color,
    GFXAlphaBlend* alphaBlend);

/**
 * This routine is used to fill gradient color a rectangular area.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param initcolor    The a,r,g,b color.
 * @param endcolor     The a,r,g,b color.
 * @param dir          The gradient fill effects.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceGradientFill(
    GFXSurfaceDst* dstSurface,
    GFXColor       initcolor,
    GFXColor       endcolor,
    GFXGradDir     dir);

/**
 * This routine is used to fill gradient color a rectangular area with alpha blending.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param initcolor    The a,r,g,b color.
 * @param endcolor     The a,r,g,b color.
 * @param dir          The gradient fill effects.
 * @param alphaBlend   The alpha blending effects.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceGradientFillWithBlend(
    GFXSurfaceDst*    dstSurface,
    GFXColor          initcolor,
    GFXColor          endcolor,
    GFXGradDir        dir,
    GFXAlphaBlend*    alphaBlend);

/**
 * This routine is used to draw line.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param fromX        The X coordinates of line start.
 * @param fromY        The Y coordinates of line start.
 * @param toX          The X coordinates of line end.
 * @param toY          The Y coordinates of line end.
 * @param lineColor    The a,r,g,b color.
 * @param lineWidth    The width of line.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceDrawLine(
    GFXSurface* dstSurface,
    int32_t     fromX,
    int32_t     fromY,
    int32_t     toX,
    int32_t     toY,
    GFXColor*   lineColor,
    int32_t     lineWidth);

/**
 * This routine is used to draw curve.
 *
 * @param surface      The valid pointer of destination GFXSurface.
 * @param point1       The XY coordinates of curve point1.
 * @param point2       The XY coordinates of curve point2.
 * @param point3       The XY coordinates of curve point3.
 * @param point4       The XY coordinates of curve point4.
 * @param lineColor    The a,r,g,b color.
 * @param lineWidth    The width of line.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceDrawCurve(
    GFXSurface*      surface,
    GFXCoordinates*  point1,
    GFXCoordinates*  point2,
    GFXCoordinates*  point3,
    GFXCoordinates*  point4,
    GFXColor*        lineColor,
    int32_t          lineWidth);

/**
 * This routine is used to projection.
 *
 * @param dstSurface   The valid pointer of destination GFXSurface.
 * @param srcSurface   The valid pointer of source GFXSurface.
 * @param scaleWidth   The ratio of width scaling.
 * @param scaleHeight  The ratio of height scaling.
 * @param degree       The angle in radius on which the image is rotating (image rotates on the Y axis).
 * @param FOV          The field of view, ratio between the projection plane and the actual object.
 * @param pivotX       The Pivot point in X, indicates which point in x to use for rotation.
 * @param alphaBlend   The alpha blending effects.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxSurfaceProjection(
    GFXSurfaceDst* dstSurface,
    GFXSurfaceSrc* srcSurface,
    float          scaleWidth,
    float          scaleHeight,
    float          degree,
    float          FOV,
    float          pivotX,
    GFXAlphaBlend* alphaBlend);

/**
* This routine is used to transform page flow effects.
*
* @param dstSurface   The valid pointer of destination GFXSurface.
* @param dx           The X coordinates of destination.
* @param dy           The Y coordinates of destination.
* @param srcSurface   The valid pointer of source GFXSurface.
* @param sx           The X coordinates of source.
* @param sy           The Y coordinates of source.
* @param sw           The width of source.
* @param sh           The height of source.
* @param x0           The X coordinates of transform ponit0 (left top).
* @param y0           The Y coordinates of transform ponit0 (left top).
* @param x1           The X coordinates of transform ponit1 (right top).
* @param y1           The Y coordinates of transform ponit1 (right top).
* @param x2           The X coordinates of transform ponit2 (right bottom).
* @param y2           The Y coordinates of transform ponit2 (right bottom).
* @param x3           The X coordinates of transform ponit3 (left bottom).
* @param y3           The Y coordinates of transform ponit3 (left bottom).
* @param bInverse     inverse true or not, true is left small right big, false is left big right small.
* @param type         The type of transform page flow effects.
*
* @return a bool value true if succeed, otherwise return false.
*/
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
    GFXTransformType transformType);

/**
 * This routine is used to set surface color.
 *
 * @param surface      The valid pointer of destination GFXSurface.
 * @param color        The a,r,g,b color.
 *
 * @return void.
 */
void
gfxSurfaceSetColor(
    GFXSurface* surface,
    GFXColor    color);

/**
 * This routine is used to set surface quality.
 *
 * @param surface      The valid pointer of destination GFXSurface.
 * @param quality      The quality mode.
 *
 * @return void.
 */
void
gfxSurfaceSetQuality(
    GFXSurface*     surface,
    GFXQualityMode  quality);

/**
 * This routine is used to set clipping area.
 *
 * @param surface      The valid pointer of destination GFXSurface.
 * @param x0           The X coordinates of left/top.
 * @param y0           The Y coordinates of left/top.
 * @param x1           The X coordinates of right/bottom.
 * @param y1           The Y coordinates of right/bottom.
 *
 * @return void.
 */
void
gfxSurfaceSetClip(
    GFXSurface* surface,
    int         x0,
    int         y0,
    int         x1,
    int         y1);

/**
 * This routine is used to un-set clipping area.
 *
 * @param surface      The valid pointer of destination GFXSurface.
 *
 * @return void.
 */
void
gfxSurfaceUnSetClip(
    GFXSurface* surface);

/**
 * This routine is used to create mask surface with specified width, height, pitch and format.
 *
 * @param width           The image width of buffer.
 * @param height          The image height of buffer.
 * @param pitch           The image pitch of buffer.
 * @param format          The image format of buffer.
 *
 * @return a valid pointer of GFXMaskSurface if succeed, otherwise return NULL.
 */
GFXMaskSurface*
gfxCreateMaskSurface(
    uint32_t        width,
    uint32_t        height,
    uint32_t        pitch,
    GFXMaskFormat   format);

/**
 * This routine is used to create mask surface with already exist buffer,
 * and the buffer "WILL BE COPIED" into the internal buffer of created mask surface.
 *
 * @width           The image width of buffer.
 * @height          The image height of buffer.
 * @pitch           The image pitch of buffer.
 * @format          The image format of buffer.
 * @buffer          The buffer pointer which will be copied into surface. This buffer will be released in gfxDestroyMaskSurface().
 * @bufferLength    The buffer length of buffer. Counted in byte.
 *
 * @return a valid pointer of GFXMaskSurface if succeed, otherwise return NULL.
 */
GFXMaskSurface*
gfxCreateMaskSurfaceByBuffer(
    uint32_t        width,
    uint32_t        height,
    uint32_t        pitch,
    GFXMaskFormat   format,
    uint8_t*        buffer,
    uint32_t        bufferLength);

/**
 * This routine is used to destroy mask surface.
 *
 * @param surface   The valid pointer of GFXMaskSurface.
 *
 * @return void.
 */
void
gfxDestroyMaskSurface(
    GFXMaskSurface* surface);

/**
 * This routine is used to set mask surface.
 *
 * @param surface       The valid pointer of GFXSurface.
 * @param maskSurface   The valid pointer of GFXMaskSurface.
 * @param enable        Enable mask surface or not.
 *
 * @return void.
 */
void
gfxSetMaskSurface(
    GFXSurface* surface,
    GFXMaskSurface* maskSurface,
    bool enable);

bool
gfxSurfaceReflected(
    GFXSurface* dest,
    int32_t dx,
    int32_t dy,
    GFXSurface* src,
    int32_t sx,
    int32_t sy,
    int32_t reflectedWidth,
    int32_t reflectedHeight);

#ifdef __cplusplus
}
#endif

#endif // __GFX_H__
