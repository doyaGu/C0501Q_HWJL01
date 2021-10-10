/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * MMP Graphic 2D header file.
 *
 * @author Erica Chang
 * @version 0.1
 */
#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include "ite/ite_m2d.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define     M2D_12BPP        12
#define     M2D_16BPP        16
#define     M2D_32BPP        32

#define     MAX_PARTSOF_FONT 10    // Limitation of loading the font files

//=============================================================================
//                              Macro Definition
//=============================================================================
//#define     M2D_BYTES_ALIGN(x,y)              (((x) + (y - 1)) & (~(y - 1)))
#define M2D_PITCH_ALIGN(x)     (((x + 7 ) / 8 + 3 ) & ~3L)
#define M2D_02_BYTES_ALIGN(x)  (((x) + 0x00000001) & 0xFFFFFFFE)
#define M2D_16K_BYTES_ALIGN(x) (((x) + 0x00003FFF) & 0xFFFFC000)

#define IS_INVALIDATE_SURFACE(surface)      ((!(surface)) || (((M2D_SURFACE *)(surface))->dispalyStructSize != sizeof(M2D_SURFACE)))
#define IS_INVALIDATE_PENOBJ(penobj)        ((!(penobj )) || (((M2D_PENOBJ  *)(penobj))->penStructSize      != sizeof(M2D_PENOBJ )))
#define IS_INVALIDATE_BRUSHOBJ(brushobj)    ((!(brushobj)) || (((M2D_BRUSHOBJ*)(brushobj))->brushStructSize   != sizeof(M2D_BRUSHOBJ)))
#define IS_INVALIDATE_IMAGE_FORMAT(surface) ((((M2D_SURFACE*)(surface))->imageFormat != MMP_M2D_IMAGE_FORMAT_RGB565)    \
                                          && (((M2D_SURFACE*)(surface))->imageFormat != MMP_M2D_IMAGE_FORMAT_ARGB4444)   \
                                          && (((M2D_SURFACE*)(surface))->imageFormat != MMP_M2D_IMAGE_FORMAT_ARGB8888)   \
                                          && (((M2D_SURFACE*)(surface))->imageFormat != MMP_M2D_IMAGE_FORMAT_ARGB1555))

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct M2D_BRUSH_TAG
{
    MMP_UINT      flag;                 /**< Flag for brush. */

    MMP_M2D_COLOR foregroundColor;      /**< Foreground color for 2 color brush. */
    MMP_M2D_COLOR backgroundColor;      /**< Background color for 2 color brush. */
    MMP_UINT      mask3210;             /**< Mask0 ~ Mask3 for hatch brush>*/
    MMP_UINT      mask7654;             /**< Mask4 ~ Mask7 for hatch brush>*/

    MMP_UINT8     *pattern;             /**< Point to the pattern. */
} M2D_BRUSH;

typedef struct M2D_FONT_TAG
{
    MMP_UINT          fontPartsSize; /**< Structure size. */

    MMP_M2D_FONT_CODE fontCode;      /**< ASCII, Big-5..... */

    MMP_UINT          fontWidth;
    MMP_UINT          fontHeight;

    MMP_UINT16        limitL;       /**< The lower limit of the font array */
    MMP_UINT16        limitH;       /**< The higher limit of the font array>*/

    MMP_UINT8         *fontPtr;     /**< Point to the font array. */
} M2D_FONT;

typedef struct M2D_PENOBJ_TAG
{
    MMP_UINT          penStructSize;    /**< Structure size. */
    MMP_M2D_COLOR     penColor;
    MMP_M2D_PEN_STYLE penStyle;
    MMP_UINT32        userDefinedStyle;
    MMP_UINT          penWidth;
} M2D_PENOBJ;

typedef struct M2D_BRUSHOBJ_TAG
{
    MMP_UINT  brushStructSize;    /**< Structure size. */
    M2D_BRUSH *realizedBrush;     /**< Point to the realized brush. */
} M2D_BRUSHOBJ;

typedef struct M2D_FONTOBJ_TAG
{
    MMP_UINT      fontStructSize;                   /**< Structure size. */
    MMP_M2D_COLOR fontColor;
    MMP_UINT      fontIndex;
    M2D_FONT      *PartsofFont[MAX_PARTSOF_FONT];   /**< Point to the parts of font. */
} M2D_FONTOBJ;

typedef struct M2D_CLIP_TAG
{
    MMP_M2D_RECT *clipRegion;
    MMP_UINT     clipRegionCount;
} M2D_CLIP;

typedef struct M2D_SURFACE_TAG
{
    MMP_UINT                dispalyStructSize; /**< Structure size. */
    MMP_UINT                displayID;         /**< ID of the dispaly. */
    MMP_UINT                engineVersion;     /**< Version of 2D engine.*/
    MMP_BOOL                WCE_isCreateByOS;  /**< Check if the surface is created by OS, for WinCE.*/

    MMP_UINT8               *baseScanPtr;      /**< Point to the first scan line of the image. */

    MMP_UINT                baseAddrOffset;    /**< Base address offset. */
    MMP_UINT                width;             /**< The width of dispaly, in pixel. */
    MMP_UINT                height;            /**< The height of dispaly, in pixel. */
    MMP_UINT                bitsPerPixel;      /**< Bits per pixel. */
    MMP_UINT                pitch;             /**< The stride from one scan to the next, in bytes. */

    MMP_M2D_IMAGE_FORMAT    imageFormat;       /**< Data image format. */

    MMP_M2D_BACKGROUND_MODE backgroundMode;    /**< Background mode. */
    MMP_M2D_COLOR           backgroundColor;   /**< Background color. */

    MMP_M2D_POINT           currPos;           /**< Current pen position. */
    MMP_M2D_POINT           brushOriginPos;    /**< The origin point of the brush. */

    MMP_M2D_FONT_STYLE      fontStyle;

    MMP_M2D_ROP2            rop2;              /**< Binary raster operation. */

    MMP_M2D_ROTATE_OP       rotateOp;          /**< Rotation operation. */
    MMP_M2D_POINT           rotateRefPixelPos; /**< The reference pixel of the rotation. */

    M2D_PENOBJ              *pen;              /**< Point to the pen object. */
    M2D_BRUSHOBJ            *brush;            /**< Point to the brush object. */
    M2D_FONTOBJ             *font;             /**< Point to the font object. */

    MMP_UINT8               isResetLineStyle;
    MMP_UINT8               isEnableScreenRotate;

    M2D_CLIP                clipSet;        /**< Point to the clipping set. */
    MMP_UINT32              bitbltId2;      /**< Record the ID of last bitblt operation related with the surface */
} M2D_SURFACE;

//=============================================================================
//                              Extern Reference
//=============================================================================
extern MMP_UINT8 *m2d_vramBasePtr;
extern MMP_INT   inited;
//extern MMP_UINT8*     m2d_currDecodeBuf;
//extern MMP_UINT32     bitbltId2Value;
//=============================================================================
//                              Function Declaration
//=============================================================================
/**
 * Bit-block transfer capabilities with alpha blending.
 *
 * @param destDisplay        handle to destination display context.
 * @param destX              x-coordinate of destination upper-left corner.
 * @param destY              y-coordinate of destination upper-left corner.
 * @param rectWidth          width of destination rectangle.
 * @param rectHeight         height of destination rectangle.
 * @param srcDisplay         handle to source display context.
 * @param srcX               x-coordinate of source upper-left corner.
 * @param srcY               y-coordinate of source upper-left corner.
 * @param alphaFormat        the way the source and destination images are
 *                           interpreted.
 * @param srcConstAlpha      transparency value to be used on the entire source
 *                           image.
 * @author Erica Chang
 */
MMP_BOOL
M2D_AlphaBlend(
    M2D_SURFACE           *destDisplay,
    MMP_INT               destX,
    MMP_INT               destY,
    MMP_INT               rectWidth,
    MMP_INT               rectHeight,
    M2D_SURFACE           *srcDisplay,
    MMP_INT               srcX,
    MMP_INT               srcY,
    MMP_M2D_ABLEND_FORMAT alphaFormat,
    MMP_INT               srcConstAlpha);

/**
 * Create temp display in CPU-Blt buffer for virtual source display.
 *
 * @param destDisplay        handle to destination display context.
 * @param destX              x-coordinate of destination upper-left corner.
 * @param destY              y-coordinate of destination upper-left corner.
 * @param rectWidth          width of destination rectangle.
 * @param rectHeight         height of destination rectangle.
 * @param srcDisplay         handle to source display context.
 * @param srcX               x-coordinate of source upper-left corner.
 * @param srcY               y-coordinate of source upper-left corner.
 * @param alphaFormat        the way the source and destination images are
 *                           interpreted.
 * @param srcConstAlpha      transparency value to be used on the entire source
 *                           image.
 * @author Erica Chang
 */
MMP_BOOL
M2D_SrcSysMem_AlphaBlend(
    M2D_SURFACE           *destDisplay,
    MMP_INT               destX,
    MMP_INT               destY,
    MMP_INT               rectWidth,
    MMP_INT               rectHeight,
    M2D_SURFACE           *srcDisplay,
    MMP_INT               srcX,
    MMP_INT               srcY,
    MMP_M2D_ABLEND_FORMAT alphaFormat,
    MMP_INT               srcConstAlpha);

/**
 * Transfer a block of color data from the source to the destination with
 * transpancy.
 *
 * @param destDisplay       handle to destination display context.
 * @param destX             x-coordinate of destination upper-left corner.
 * @param destY             y-coordinate of destination upper-left corner.
 * @param destWidth         width of destination rectangle.
 * @param destHeight        height of destination rectangle.
 * @param srcDisplay        handle to source display context.
 * @param srcX              x-coordinate of source upper-left corner.
 * @param srcY              y-coordinate of source upper-left corner.
 * @param destHighColor     the high value of destination color key (inclusive).
 * @param destLowColor      the low value of destination color key (inclusive).
 * @param srcHighColor      the high value of source color key (inclusive).
 * @param srcLowColor       the low value of source color key (inclusive).
 * @param transparentRop    raster operation for transparency.
 * @author Erica Chang
 */
MMP_BOOL
M2D_TransparentBlt(
    M2D_SURFACE   *destDisplay,
    MMP_INT       destX,
    MMP_INT       destY,
    MMP_INT       rectWidth,
    MMP_INT       rectHeight,
    M2D_SURFACE   *srcDisplay,
    MMP_INT       srcX,
    MMP_INT       srcY,
    MMP_M2D_COLOR destHighColor,
    MMP_M2D_COLOR destLowColor,
    MMP_M2D_COLOR srcHighColor,
    MMP_M2D_COLOR srcLowColor,
    MMP_M2D_TROP  transparentRop);

/**
 * Create temp display in CPU-Blt buffer for virtual source display.
 *
 * @param destDisplay       handle to destination display context.
 * @param destX             x-coordinate of destination upper-left corner.
 * @param destY             y-coordinate of destination upper-left corner.
 * @param destWidth         width of destination rectangle.
 * @param destHeight        height of destination rectangle.
 * @param srcDisplay        handle to source display context.
 * @param srcX              x-coordinate of source upper-left corner.
 * @param srcY              y-coordinate of source upper-left corner.
 * @param destHighColor     the high value of destination color key (inclusive).
 * @param destLowColor      the low value of destination color key (inclusive).
 * @param srcHighColor      the high value of source color key (inclusive).
 * @param srcLowColor       the low value of source color key (inclusive).
 * @param transparentRop    raster operation for transparency.
 * @author Erica Chang
 */
MMP_BOOL
M2D_SrcSysMem_TransparentBlt(
    M2D_SURFACE   *destDisplay,
    MMP_INT       destX,
    MMP_INT       destY,
    MMP_INT       rectWidth,
    MMP_INT       rectHeight,
    M2D_SURFACE   *srcDisplay,
    MMP_INT       srcX,
    MMP_INT       srcY,
    MMP_M2D_COLOR destHighColor,
    MMP_M2D_COLOR destLowColor,
    MMP_M2D_COLOR srcHighColor,
    MMP_M2D_COLOR srcLowColor,
    MMP_M2D_TROP  transparentRop);

/**
 * Copies the source rectangle directly to the destination rectangle.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param rectWidth     width of destination rectangle.
 * @param rectHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @author Erica Chang
 */
M2D_API MMP_BOOL
M2D_SourceCopy(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_INT     rectWidth,
    MMP_INT     rectHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY);

/**
 * Create temp display in CPU-Blt buffer for virtual source display.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param rectWidth     width of destination rectangle.
 * @param rectHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @author Erica Chang
 */
MMP_BOOL
M2D_SrcSysMem_SourceCopy(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_INT     rectWidth,
    MMP_INT     rectHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY);

/**
 * Transfer a block of color data from the source to the destination according
 * to the ROP.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param rectWidth     width of destination rectangle.
 * @param rectHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param rop           raster operation.
 * @author Erica Chang
 */
MMP_BOOL
M2D_Bitblt(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_INT     rectWidth,
    MMP_INT     rectHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY,
    MMP_INT     rop);

/**
 * Create temp display in CPU-Blt buffer for virtual source display.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param rectWidth     width of destination rectangle.
 * @param rectHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param rop           raster operation.
 * @author Erica Chang
 */
MMP_BOOL
M2D_SrcSysMem_Bitblt(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_INT     rectWidth,
    MMP_INT     rectHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY,
    MMP_INT     rop);

/**
 * Transfer a block of image from the source rectangle to the destination
 * rectangle, stretch or shrink to fit the dimensions of the destination
 * rectangle.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param rectWidth     width of destination rectangle.
 * @param rectHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param srcWidth      width of source rectangle.
 * @param srcHeight     height of source rectangle.
 * @author Erica Chang
 */
MMP_BOOL
M2D_StretchBlt(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_INT     rectWidth,
    MMP_INT     rectHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY,
    MMP_INT     srcWidth,
    MMP_INT     srcHeight);

/**
 * Create temp display in CPU-Blt buffer for virtual source display.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param rectWidth     width of destination rectangle.
 * @param rectHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param srcWidth      width of source rectangle.
 * @param srcHeight     height of source rectangle.
 * @author Erica Chang
 */
MMP_BOOL
M2D_SrcSysMem_StretchBlt(
    M2D_SURFACE *destDisplay,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_INT     rectWidth,
    MMP_INT     rectHeight,
    M2D_SURFACE *srcDisplay,
    MMP_INT     srcX,
    MMP_INT     srcY,
    MMP_INT     srcWidth,
    MMP_INT     srcHeight);

/**
 * Draw a line from current position up to the specified point.
 *
 * @param display       display context handle.
 * @param endX          x-coordinate of ending point.
 * @param endY          y-coordinate of ending point.
 * @author Mandy Wu
 */
MMP_BOOL
M2D_DrawLine(
    M2D_SURFACE       *display,
    MMP_M2D_PEN_STYLE penStyle,
    MMP_INT           endX,
    MMP_INT           endY);

/**
 * Draw a series of lines by connecting the points in the specified point
 * array.
 *
 * @param display       display context handle.
 * @param points        array of endpoints.
 * @param pointCount    the amount of the points in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_BOOL
M2D_DrawPolyLines(
    M2D_SURFACE   *display,
    MMP_M2D_POINT *points,
    MMP_UINT      pointCount);

/**
 * For MM365, Fill a rectangle in the specified gradient color.
 *
 * @param destDisplay       handle to destination display context.
 * @param destX             x-coordinate of destination upper-left corner.
 * @param destY             y-coordinate of destination upper-left corner.
 * @param destWidth         width of destination rectangle.
 * @param destHeight        height of destination rectangle.
 * @param initColor         the initial color(RGB565)
 * @param endColor          the end color(RGB565)
 * @param direction         the direction of gradient fill
 * @author Mandy Wu
 */

MMP_BOOL
M2D_GradientFill(
    M2D_SURFACE          *destDisplay,
    MMP_UINT             destX,
    MMP_UINT             destY,
    MMP_UINT             destWidth,
    MMP_UINT             destHeight,
    MMP_M2D_COLOR        initColor,
    MMP_M2D_COLOR        endColor,
    MMP_M2D_GF_DIRECTION direction);

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __GRAPHICS_H