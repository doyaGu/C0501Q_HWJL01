/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * ITE Graphic 2D Driver API header file.
 *
 * @author Erica Chang
 * @version 2.0
 */
#ifndef __MMP_GRAPHICS_H
#define __MMP_GRAPHICS_H

#include "ite/mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M2D_API extern

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct M2D_COMP_GLYPH_TAG
{
    MMP_UINT8  outputBPP;
    MMP_UINT8  compBPP;
    MMP_UINT8  xorWithOtherPlane;
    MMP_UINT8  reserved1;               // DWORD 0
    MMP_INT16  width;
    MMP_INT16  rows;                    // DWORD 1
    MMP_INT16  top;
    MMP_INT16  left;                    // DWORD 2
    MMP_INT16  xAdvance;
    MMP_INT16  reserved2;               // DWORD 3
    MMP_UINT16 pitch;
    MMP_UINT16 compGlyphSize;           // DWORD 4
    MMP_UINT8  mps[4];                  // DWORD 5
    MMP_UINT8  huffTable[4];            // DWORD 6
    MMP_UINT8  *pCmpGlyphData;          // DWORD 7
} M2D_COMP_GLYPH;
//=============================
//  Type Definition
//=============================

/**
 * < 4-byte format:0000 0000 0000 0000 RRRR RGGG GGGB BBBB.
 */
typedef MMP_UINT32 MMP_M2D_COLOR;

/**
 * Diplay handle.
 */
//typedef MMP_UINT MMP_M2D_SURFACE;
typedef MMP_SURFACE MMP_M2D_SURFACE;

/**
 * A image, brush, pen, or font object handle.
 */
typedef MMP_UINT MMP_M2D_HANDLE;

//=============================
//  Structure Type Definition
//=============================
/**
 * The structure defines LCD type
 */
typedef enum MMP_M2D_LCD_TYPE_TAG
{
    MMP_M2D_LCDA,   /**< LCDA, default LCD */
    MMP_M2D_LCDB,   /**< LCDB */
    MMP_M2D_LCDC    /**< LCDC */
} MMP_M2D_LCD_TYPE;

/**
 * The structure defines the x- and y- coordinates of a point.
 */
typedef struct MMP_M2D_POINT_TAG
{
    MMP_INT x; /**< Specifies the x-coordinate of the point */
    MMP_INT y; /**< Specifies the Y-coordinate of the point */
} MMP_M2D_POINT;

/**
 * The structure defines the coordinates of the upper-left and lower-right
 * corners of a rectangle.
 */
typedef struct MMP_M2D_RECT_TAG
{
    /**
     * The x-coordinate of the upper-left corner of the rectangle
     */
    MMP_INT left;

    /**
     * The y-coordinate of the upper-left corner of the rectangle
     */
    MMP_INT top;

    /**
     * The x-coordinate of the lower-right corner of the rectangle
     */
    MMP_INT right;

    /**
     * The y-coordinate of the lower-right corner of the rectangle
     */
    MMP_INT bottom;
} MMP_M2D_RECT;

//=============================
//  Enumeration Type Definition
//=============================
/**
 * @see mmpM2dAlphaBlend()
 */
typedef enum MMP_M2D_ABLEND_FORMAT_TAG
{
    /**
     * The source image use an constant alpha value
     */
    MMP_M2D_ABLEND_FORMAT_SRC_CONST,

    /**
     * The source image with an Alpha channel
     */
    MMP_M2D_ABLEND_FORMAT_SRC_ALPHA,

    MMP_M2D_ABLEND_FORMAT_SRC_PREMULTIPLY
} MMP_M2D_ABLEND_FORMAT;

/**
 * @see mmpM2dDrawTransparentBlock()
 */
typedef enum MMP_M2D_TROP_TAG
{
    /**
     * The source pixels will never be drawn to the destination.
     */
    MMP_M2D_TROP_NEVER,

    /**
     * Only the source pixels within the source color key range and its pixels
     * on the destination are within the destination color key range will be
     * drawn.
     */
    MMP_M2D_TROP_SRCKEY_AND_DESTKEY,

    /**
     * Only the source pixels beyond the source color key range and its pixels
     * on the destination are within the destination color key range will be
     * drawn.
     */
    MMP_M2D_TROP_NOT_SRCKEY_AND_DESTKEY,

    /**
     * The source pixels will only be drawn to the pixels on the destination
     * that are within the destination color key range.
     */
    MMP_M2D_TROP_DESTKEY,

    /**
     * Only the source pixels within the source color key range and its pixels
     * on the destination are beyond the destination color key range will be
     * drawn.
     */
    MMP_M2D_TROP_SRCKEY_AND_NOT_DESTKEY,

    /**
     * Only the source pixels within the source color key range will be drawn
     * on the destinaiton.
     */
    MMP_M2D_TROP_SRCKEY,

    /**
     * The source pixels within the source color key range and its pixels on
     * the destination are beyond the destination color key range will be
     * drawn. And, the source pixels beyond the source color key range and its
     * pixels on the destination are within the destination color key range
     * will also be drawn.
     */
    MMP_M2D_TROP_SRCKEY_XOR_DESTKEY,

    /**
     * The source pixels within the source color key range will be drawn. And,
     * the source pixels that its pixels on the destination are within the
     * destination color key range will also be drawn.
     */
    MMP_M2D_TROP_SRCKEY_OR_DESTKEY,

    /**
     * Only the source pixels beyond the source color key range and its pixels
     * on the destination are beyond the destination color key range will be
     * drawn.
     */
    MMP_M2D_TROP_NOT_SRCKEY_AND_NOT_DESTKEY,

    /**
     * The source pixels within the source color key range and its pixels on
     * the destination are within the destination color key range will be
     * drawn. And, the source pixels beyond the source color key range and its
     * pixels on the destination are beyond the destination color key range
     * will also be drawn.
     */
    MMP_M2D_TROP_SRCKEY_XNOR_DESTKEY,

    /**
     * Only the source pixels beyond the source color key range will be drawn
     * on the destinaiton.
     */
    MMP_M2D_TROP_NOT_SRCKEY,

    /**
     * The source pixels beyond the source color key range will be drawn. And,
     * the source pixels that its pixels on the destination are within the
     * destination color key range will also be drawn.
     */
    MMP_M2D_TROP_NOT_SRCKEY_OR_DESTKEY,

    /**
     * The source pixels will only be drawn to the pixels on the destination
     * that are beyond the destination color key range.
     */
    MMP_M2D_TROP_NOT_DESTKEY,

    /**
     * The source pixels within the source color key range will be drawn. And,
     * the source pixels that its pixels on the destination are beyond the
     * destination color key range will also be drawn.
     */
    MMP_M2D_TROP_SRCKEY_OR_NOT_DESTKEY,

    /**
     * The source pixels beyond the source color key range will be drawn. And,
     * the source pixels that its pixels on the destination are beyond the
     * destination color key range will also be drawn.
     */
    MMP_M2D_TROP_NOT_SRCKEY_OR_NOT_DESTKEY,

    /**
     * The source pixels will be always drawn to the destination.
     */
    MMP_M2D_TROP_ALWAYS
} MMP_M2D_TROP;

/**
 * @see mmpM2dGradientFill()
 */
typedef enum MMP_M2D_GF_DIRECTION_TAG
{
    MMP_M2D_GF_HORIZONTAL,    /**< the direction of horizontal */
    MMP_M2D_GF_VERTICAL,      /**< the direction of vertical */
    MMP_M2D_GF_DIAGONAL_LT,   /**< the direction of diagonal, left-top to right-bottom */
    MMP_M2D_GF_DIAGONAL_RT,   /**< the direction of diagonal, right-top to left-bottom */
    MMP_M2D_GF_DIAGONAL_LB,   /**< the direction of diagonal, left-bottom to right-top */
    MMP_M2D_GF_DIAGONAL_RB    /**< the direction of diagonal, right-bottom to left-top */
} MMP_M2D_GF_DIRECTION;

/**
 * @see mmpM2dSetRotateOP()
 */
typedef enum MMP_M2D_ROTATE_OP_TAG
{
    MMP_M2D_ROTATE_OP_NOT,           /**< Coordinates not rotate */
    MMP_M2D_ROTATE_OP_90,            /**< Coordinates rotate  90 degree */
    MMP_M2D_ROTATE_OP_180,           /**< Coordinates rotate 180 degree */
    MMP_M2D_ROTATE_OP_270,           /**< Coordinates rotate 270 degree */
    MMP_M2D_ROTATE_OP_MIRROR,        /**< Coordinates mirror */
    MMP_M2D_ROTATE_OP_MIRROR_90,     /**< Coordinates mirror and rotate 90 degree */
    MMP_M2D_ROTATE_OP_MIRROR_180,    /**< Coordinates mirror and rotate 180 degree*/
    MMP_M2D_ROTATE_OP_MIRROR_270     /**< Coordinates mirror and rotate 270 degree*/
} MMP_M2D_ROTATE_OP;

/**
 * @see mmpM2dSetBackgroundMode()
 */
typedef enum MMP_M2D_BACKGROUND_MODE_TAG
{
    /**
     * Background is filled with the current background color before the text,
     * hatched brush , or pen is drawn.
     */
    MMP_M2D_BACKGROUND_MODE_OPAQUE,

    /**
     * Background remains untouched.
     */
    MMP_M2D_BACKGROUND_MODE_TRANSPARENT
} MMP_M2D_BACKGROUND_MODE;

/**
 * @see mmpM2dCreateSurfaceFromImage()
 */
typedef enum MMP_M2D_IMAGE_FORMAT_TAG
{
    MMP_M2D_IMAGE_FORMAT_ARGB8888 = MMP_PIXEL_FORMAT_ARGB8888,  /**< Image format is ARGB8888 */
    MMP_M2D_IMAGE_FORMAT_ARGB1555 = MMP_PIXEL_FORMAT_ARGB1555,  /**< Image format is ARGB1555 */
    MMP_M2D_IMAGE_FORMAT_ARGB4444 = MMP_PIXEL_FORMAT_ARGB4444,  /**< Image format is ARGB4444 */
    MMP_M2D_IMAGE_FORMAT_RGB565   = MMP_PIXEL_FORMAT_RGB565     /**< Image format is RGB565 */
} MMP_M2D_IMAGE_FORMAT;

/**
 * @see mmpM2dSelectObject()
 */
typedef enum MMP_M2D_OBJECT_TYPE_TAG
{
    MMP_M2D_PENOBJ,
    MMP_M2D_BRUSHOBJ,
    MMP_M2D_FONTOBJ
} MMP_M2D_OBJECT_TYPE;

/**
 * @see mmpM2dCreatePen()
 */
typedef enum MMP_M2D_PEN_STYLE_TAG
{
    MMP_M2D_PEN_STYLE_SOLID,        /**< default pen          */
    MMP_M2D_PEN_STYLE_DASH,         /**< DASH         ------- */
    MMP_M2D_PEN_STYLE_DOT,          /**< DOT                  */
    MMP_M2D_PEN_STYLE_DASH_DOT,     /**< DASHDOT      _._._._ */
    MMP_M2D_PEN_STYLE_DASH_DOT_DOT, /**< DASHDOTDOT   _.._.._ */
    MMP_M2D_PEN_STYLE_USER_DEFINED
} MMP_M2D_PEN_STYLE;

/**
 * @see mmpM2dCreateHatchBrush()
 */
typedef enum MMP_M2D_HATCH_STYLE_TAG
{
    MMP_M2D_HS_HORIZONTAL,  /**< Horizontal hatch */
    MMP_M2D_HS_VERTICAL,    /**< Vertical hatch */
    MMP_M2D_HS_FDIAGONAL,   /**< 45-degree upward left-to-right hatch*/
    MMP_M2D_HS_BDIAGONAL,   /**< 45-degree downward left-to-right hatch*/
    MMP_M2D_HS_CROSS,       /**< Horizontal and vertical crosshatch */
    MMP_M2D_HS_DIAGCROSS,   /**< 45-degree crosshatch */
    MMP_M2D_HS_CUBE,        /**< Cube */
    MMP_M2D_HS_USER_DEFINED
} MMP_M2D_HATCH_STYLE;

/**
 * @see mmpM2dLoadFont()
 */
typedef enum MMP_M2D_FONT_CODE_TAG
{
    MMP_M2D_FC_ASCII,       /**< A font file with ASCII code */
    MMP_M2D_FC_BIG5         /**< A font file with Big-5 code */
} MMP_M2D_FONT_CODE;

/**
 * @see mmpM2dGetFontStyle(), mmpM2dSetFontStyle().
 */
typedef enum MMP_M2D_FONT_STYLE_TAG
{
    MMP_M2D_FS_NORMAL,      /**< default font */
    MMP_M2D_FS_BOLD,        /**< Bold font style */
    MMP_M2D_FS_ITALIC,      /**< Italic font style */
    MMP_M2D_FS_UNDERLINE,   /**< Underline font style */
    MMP_M2D_FS_SHADOW,      /**< Shadow font style */
    MMP_M2D_FS_EMBOSSMENT   /**< Embossment font style */
} MMP_M2D_FONT_STYLE;

/**
 * @see mmpM2dGetRop2(), mmpM2dSetRop2().
 */
typedef enum MMP_M2D_ROP2_TAG
{
    /**
     * Pixel is always 0.
     */
    MMP_M2D_ROP2_BLACK = 1,

    /**
     * Pixel is the inverse of the ROP2_MERGEPEN color.
     */
    MMP_M2D_ROP2_NOTMERGEPEN,

    /**
     * Pixel is a combination of the colors common to both the screen and the
     * inverse of the pen.
     */
    MMP_M2D_ROP2_MASKNOTPEN,

    /**
     * Pixel is the inverse of the pen color.
     */
    MMP_M2D_ROP2_NOTCOPYPEN,

    /**
     * Pixel is a combination of the colors common to both the pen and the
     * inverse of the screen.
     */
    MMP_M2D_ROP2_MASKPENNOT,

    /**
     * Pixel is the inverse of the screen color.
     */
    MMP_M2D_ROP2_NOT,

    /**
     * Pixel is a combination of the colors in the pen and in the screen, but
     * not in both.
     */
    MMP_M2D_ROP2_XORPEN,

    /**
     * Pixel is the inverse of the ROP2_MASKPEN color.
     */
    MMP_M2D_ROP2_NOTMASKPEN,

    /**
     * Pixel is a combination of the colors common to both the pen and the
     * screen.
     */
    MMP_M2D_ROP2_MASKPEN,

    /**
     * Pixel is the inverse of the ROP2_XORPEN color.
     */
    MMP_M2D_ROP2_NOTXORPEN,

    /**
     * Pixel remains unchanged.
     */
    MMP_M2D_ROP2_NOP,

    /**
     * Pixel is a combination of the screen color and the inverse of the pen
     * color.
     */
    MMP_M2D_ROP2_MERGENOTPEN,

    /**
     * Pixel is the pen color.
     */
    MMP_M2D_ROP2_COPYPEN,

    /**
     * Pixel is a combination of the pen color and the inverse of the screen
     * color.
     */
    MMP_M2D_ROP2_MERGEPENNOT,

    /**
     * Pixel is a combination of the pen color and the screen color.
     */
    MMP_M2D_ROP2_MERGEPEN,

    /**
     * Pixel is always 1.
     */
    MMP_M2D_ROP2_WHITE
} MMP_M2D_ROP2;

//=============================================================================
//                              Extern Reference
//=============================================================================

extern MMP_BOOL m2d_trueColor;

//=============================================================================
//                              Function Declaration
//=============================================================================
/** @defgroup group1 ITE 2D Driver API
 *  The supported API for 2d graphic.
 *  @{
 */

/**
 * Initial 2D engine.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dInitialize(
    void);

/**
 * Terminate 2D engine.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dTerminate(
    void);

//=============================
//  2D Drawing Primitives
//=============================
/**
 * Display images that have transparent or semitransparent pixels.
 *
 * @param destSurface        handle to destination surface.
 * @param destX              x-coordinate of destination upper-left corner.
 * @param destY              y-coordinate of destination upper-left corner.
 * @param destWidth          width of destination rectangle.
 * @param destHeight         height of destination rectangle.
 * @param srcSurface         handle to source surface.
 * @param srcX               x-coordinate of source upper-left corner.
 * @param srcY               y-coordinate of source upper-left corner.
 * @param alphaFormat        the way the source and destination images are
 *                           interpreted.
 * @param srcConstAlpha      transparency value to be used on the entire source
 *                           image.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark There are two alpha formats, constant alpha and per-pixel alpha.
 * Three kinds of bitmap format are supported by per-pixel alpha: ARGB1555, ARGB4444, and ARGB8888.
 * Constant alpha can support four kinds of bitmap format: RGB565, ARGB1555, ARGB4444, and ARGB8888.
 */
M2D_API MMP_RESULT
mmpM2dAlphaBlend(
    MMP_M2D_SURFACE       destSurface,
    MMP_INT               destX,
    MMP_INT               destY,
    MMP_UINT              destWidth,
    MMP_UINT              destHeight,
    MMP_M2D_SURFACE       srcSurface,
    MMP_INT               srcX,
    MMP_INT               srcY,
    MMP_M2D_ABLEND_FORMAT alphaFormat,
    MMP_INT               srcConstAlpha);
/*
 * @example alphablend_const.c
 * This is an example of how to apply const alphablend effect to a bitmap.
 */

/**
 * Draw a line from current position up to the specified point.
 *
 * @param surface       surface handle.
 * @param endX          x-coordinate of ending point.
 * @param endY          y-coordinate of ending point.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark The default start coordinate for line drawing is (0,0).
 * One can change the start coordinate by mmpM2dSetCurrentPosition().
 */
M2D_API MMP_RESULT
mmpM2dDrawLine(
    MMP_M2D_SURFACE surface,
    MMP_INT         endX,
    MMP_INT         endY);
/*
 * @example line.c
 * This is an example of how to draw lines.
 */

/**
 * Draw a series of lines by connecting the points in the specified point
 * array.
 *
 * @param surface       surface handle.
 * @param points        array of endpoints.
 * @param pointCount    the amount of the points in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark The default start coordinate for line drawing is (0,0).
 * One can change the start coordinate by mmpM2dSetCurrentPosition().
 */
M2D_API MMP_RESULT
mmpM2dDrawPolyLines(
    MMP_M2D_SURFACE surface,
    MMP_M2D_POINT   *points,
    MMP_UINT        pointCount);
/*
 * @example polyline.c
 * This is an example of how to draw a series of connected lines.
 */

/**
 * Transfer a block of image from the source rectangle to the destination
 * rectangle, stretch or shrink to fit the dimensions of the destination
 * rectangle.
 *
 * @param destSurface   handle to destination surface.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param destWidth     width of destination rectangle.
 * @param destHeight    height of destination rectangle.
 * @param srcSurface    handle to source surface.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param srcWidth      width of source rectangle.
 * @param srcHeight     height of source rectangle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark This API is used to stretch/shrink a bitmap by assigning different destWidth and destHeight.
 */
M2D_API MMP_RESULT
mmpM2dStretchSrcCopy(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        destWidth,
    MMP_UINT        destHeight,
    MMP_M2D_SURFACE srcSurface,
    MMP_INT         srcX,
    MMP_INT         srcY,
    MMP_UINT        srcWidth,
    MMP_UINT        srcHeight);
/*
 * @example stretchsrccopy.c
 * This is an example of how to stretch/shrink a bitmap.
 */

/**
 * Copies the source rectangle directly to the destination rectangle.
 *
 * @param destSurface   handle to destination surface.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param destWidth     width of destination rectangle.
 * @param destHeight    height of destination rectangle.
 * @param srcSurface    handle to source surface.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dSourceCopy(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        destWidth,
    MMP_UINT        destHeight,
    MMP_M2D_SURFACE srcSurface,
    MMP_INT         srcX,
    MMP_INT         srcY);
/*
 * @example srccopy.c
 * This is an example of how to draw a bitmap to LCD.
 */

/**
 * Transfer a block of color data from the source to the destination according
 * to the ROP.
 *
 * @param destSurface   handle to destination surface.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param destWidth     width of destination rectangle.
 * @param destHeight    height of destination rectangle.
 * @param srcSurface    handle to source surface.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param rop           raster operation.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark ROP is a numerical raster operation code that describes how to draw the source bitmap to the destination.
 * Please refers <a href="../tutorial/m2d_API_tutorial_2.htm#bitblt">here</a> to see the common used rop3 codes.
 */
M2D_API MMP_RESULT
mmpM2dTransferBlock(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        destWidth,
    MMP_UINT        destHeight,
    MMP_M2D_SURFACE srcSurface,
    MMP_INT         srcX,
    MMP_INT         srcY,
    MMP_INT         rop);
/*
 * @example bitblt.c
 * This is an example of how to apply brush to show the effect of bitblt.
 */

/**
 * Transfer a block of color data from the source to the destination with
 * transpancy.
 *
 * @param destSurface       handle to destination surface.
 * @param destX             x-coordinate of destination upper-left corner.
 * @param destY             y-coordinate of destination upper-left corner.
 * @param destWidth         width of destination rectangle.
 * @param destHeight        height of destination rectangle.
 * @param srcSurface        handle to source surface.
 * @param srcX              x-coordinate of source upper-left corner.
 * @param srcY              y-coordinate of source upper-left corner.
 * @param destHighColor     the high value of destination color key (inclusive).
 * @param destLowColor      the low value of destination color key (inclusive).
 * @param srcHighColor      the high value of source color key (inclusive).
 * @param srcLowColor       the low value of source color key (inclusive).
 * @param transparentRop    raster operation for transparency.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark There supports 16 types of transparentRop in ITE 2D graphic accelerator. Please refers
 * MMP_TROP to see the detailed information.
 */
M2D_API MMP_RESULT
mmpM2dDrawTransparentBlock(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        destWidth,
    MMP_UINT        destHeight,
    MMP_M2D_SURFACE srcSurface,
    MMP_INT         srcX,
    MMP_INT         srcY,
    MMP_M2D_COLOR   destHighColor,
    MMP_M2D_COLOR   destLowColor,
    MMP_M2D_COLOR   srcHighColor,
    MMP_M2D_COLOR   srcLowColor,
    MMP_M2D_TROP    transparentRop);
/*
 * @example transparent.c
 * This is an example of how to apply transparent bitblt effect.
 */

/**
 * Write the string on the specified surface.
 *
 * @param surface     handle to destination surface.
 * @param startX      x-coordinate of starting position.
 * @param startY      y-coordinate of starting position.
 * @param string      array of character string.
 * @param charCount   the character count of the string.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dDrawText(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         startX,
    MMP_INT         startY,
    MMP_UINT8       *string,
    MMP_UINT        wordCount);
/*
 * @example font.c
 * This is an example of how to draw text to LCD.
 */

/**
 * Draw a 1bpp bitmap (a text) to the specified surface.
 *
 * @param destSurface   handle to destination surface.
 * @param destX         the x-coordinate of starting position of the destSurface to be drawn.
 * @param destY         the y-coordinate of starting position of the destSurface to be drawn.
 * @param srcPtr        source address.
 * @param srcWidth      width of the source bitmap.
 * @param srcHeight     height of the source bitmap.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Source must be 1bpp bitmap!
 */
M2D_API MMP_RESULT
mmpM2dDrawBmpText(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    void            *srcPtr,
    MMP_UINT        srcWidth,
    MMP_UINT        srcHeight,
    MMP_UINT        srcPitch);

/**
 * Only for teletext using
 * Draw a 1bpp bitmap (a text) to the specified surface.
 *
 * @param destSurface   handle to destination surface.
 * @param destX         the x-coordinate of starting position of the destSurface to be drawn.
 * @param destY         the y-coordinate of starting position of the destSurface to be drawn.
 * @param srcPtr        source address.
 * @param srcWidth      width of the source bitmap.
 * @param srcHeight     height of the source bitmap.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Source must be 1bpp bitmap!
 */
M2D_API MMP_RESULT
mmpM2dDrawBmpTextTtx(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    void            *srcPtr,   //MMP_UINT8*          srcPtr,
    MMP_UINT        srcWidth,
    MMP_UINT        srcHeight,
    MMP_UINT        srcPitch);

/**
 * add wait 2d idle for font buffer reuse
 */
M2D_API MMP_RESULT
mmpM2dDrawBmpTextTtx2(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    void            *srcPtr,
    MMP_UINT        srcWidth,
    MMP_UINT        srcHeight,
    MMP_UINT        srcPitch);

M2D_API MMP_RESULT
mmpM2dDrawCmpBmpTextTtx2(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    void            *ptCmpGlyph,
    MMP_UINT        srcWidth,
    MMP_UINT        srcHeight,
    MMP_UINT        srcPitch);

/*
 * @example bmpfont.c
 * This is an example of how to draw a bitmap font to LCD.
 */

/**
 * Draw a hollow rectangle by the current pen.
 *
 * @param surface   surface handle.
 * @param startX    x-coordinate of the upper-left corner of the rectangle.
 * @param startY    y-coordinate of the upper-left corner of the rectangle.
 * @param width     width of the rectangle.
 * @param height    height of the rectangle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dDrawRectangle(
    MMP_M2D_SURFACE surface,
    MMP_INT         startX,
    MMP_INT         startY,
    MMP_UINT        width,
    MMP_UINT        height);

/**
 * Draw a series of hollow rectangles in the specified rectangle array by the
 * current pen.
 *
 * @param surface       surface handle.
 * @param rects         array of the rectangles.
 * @param rectCount     the amount of the rectangles in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dDrawPolyRectangle(
    MMP_M2D_SURFACE surface,
    MMP_M2D_RECT    *rects,
    MMP_UINT        rectCount);

/**
 * Draw a rectangle and fill it by the current brush.
 *
 * @param surface   surface handle.
 * @param startX    x-coordinate of the upper-left corner of the rectangle.
 * @param startY    y-coordinate of the upper-left corner of the rectangle.
 * @param width     width of the rectangle.
 * @param height    height of the rectangle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dFillRectangle(
    MMP_M2D_SURFACE surface,
    MMP_INT         startX,
    MMP_INT         startY,
    MMP_UINT        width,
    MMP_UINT        height);

/**
 * Fill a series of rectangles in the specified rectangle array by the current
 * brush.
 *
 * @param surface       surface handle.
 * @param rects         array of the rectangles.
 * @param rectCount     the amount of the rectangles in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dFillPolyRectangle(
    MMP_M2D_SURFACE    surface,
    const MMP_M2D_RECT *rects,
    MMP_UINT           rectCount);

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

M2D_API MMP_RESULT
mmpM2dGradientFill(
    MMP_M2D_SURFACE      destSurface,
    MMP_UINT             destX,
    MMP_UINT             destY,
    MMP_UINT             destWidth,
    MMP_UINT             destHeight,
    MMP_M2D_COLOR        initColor,
    MMP_M2D_COLOR        endColor,
    MMP_M2D_GF_DIRECTION direction);

//=============================
//  Miscellaneous Surface Operation Functions
//=============================

/**
 * Get the current coordinate rotation operation for the specified surface
 * context.
 *
 * @param surface       surface handle.
 * @param rotateOp      return the current coordinate rotation operation.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetRotateOp().
 */
M2D_API MMP_RESULT
mmpM2dGetRotateOp(
    MMP_M2D_SURFACE   surface,
    MMP_M2D_ROTATE_OP *rotateOp);

/**
 * Set the current coordinate rotation function to the specified rotate
 * operation.
 *
 * @param surface       surface handle.
 * @param rotateOp      coordinate rotate operation.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark ITE 2D graphic accelerator supports 8 types of rotation operation. Please refers
 * MMP_ROTATE_OP to see the detail.
 * @remark If one wants to draw a rotated image to a surface, one should call mmpM2dSetRotateOp()
 * before drawing functions.
 * @see mmpM2dGetRotateOp().
 */
M2D_API MMP_RESULT
mmpM2dSetRotateOp(
    MMP_M2D_SURFACE   surface,
    MMP_M2D_ROTATE_OP rotateOp);

/**
 * Get the current position for the specified surface.
 *
 * @param surface       surface handle.
 * @param position      return the coordinates of current position.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetCurrentPosition(), mmpM2dDrawLine(), mmpM2dDrawPolyLines().
 */
M2D_API MMP_RESULT
mmpM2dGetCurrentPosition(
    MMP_M2D_SURFACE surface,
    MMP_M2D_POINT   *position);

/**
 * Set the current position to the specified position.
 *
 * @param surface   surface handle.
 * @param newX      x-coordinate of new current position.
 * @param newY      y-coordinate of new current position.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark One can change the start coordinate before line drawing by mmpM2dSetCurrentPosition().
 * @see mmpM2dGetCurrentPosition().
 */
M2D_API MMP_RESULT
mmpM2dSetCurrentPosition(
    MMP_M2D_SURFACE surface,
    MMP_INT         newX,
    MMP_INT         newY);

/**
 * Get the current background mode for the specified surface.
 *
 * @param surface         surface handle.
 * @param backgroundMode  return the current background mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetBackgroundMode().
 */
M2D_API MMP_RESULT
mmpM2dGetBackgroundMode(
    MMP_M2D_SURFACE         surface,
    MMP_M2D_BACKGROUND_MODE *backgroundMode);

/**
 * Set the current background mode to the specified mode.
 *
 * @param surface           surface handle.
 * @param backgroundMode    background mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dGetBackgroundMode().
 */
M2D_API MMP_RESULT
mmpM2dSetBackgroundMode(
    MMP_M2D_SURFACE         surface,
    MMP_M2D_BACKGROUND_MODE backgroundMode);

/**
 * Get the current background color for the specified surface.
 *
 * @param surface           surface handle.
 * @param backgroundColor   return the current background color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetBackgroundColor().
 */
M2D_API MMP_RESULT
mmpM2dGetBackgroundColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   *backgroundColor);

/**
 * Set the current background color to the specified color value.
 *
 * @param surface           surface handle.
 * @param backgroundColor   background color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dGetBackgroundColor().
 */
M2D_API MMP_RESULT
mmpM2dSetBackgroundColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   backgroundColor);

//=============================
//  Surface Funcitons
//=============================
/**
 * Get the surface of the LCD screen.
 *
 * @param   return the screen surface handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dGetScreenSurface(
    MMP_M2D_SURFACE *surface);

/**
 * Create the surface of LCD_B or LCD_C.
 *
 * @param   surface      return the screen surface handle.
 * @param   surfaceType  fill LCDB or LCDC.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dGetScreenSurface to get the handle of LCD A
 */
M2D_API MMP_RESULT
mmpM2dCreateScreenSurface(
    MMP_M2D_SURFACE  *surface,
    MMP_M2D_LCD_TYPE LCDType);

/**
 * Create a surface in memory that is compatible with the existing
 * surface.
 *
 * @param targetSurface    the existing specified surface handle. If NULL, the
 *                         target surface will be the default screen surface.
 * @param surface          return the created surface handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark This API can create a surface which is compatible with an existing targetSurface.
 * All attributes of the created surface are the same with the targetSurface but with null pixel data.
 * @remark When you no longer need the surface, call mmpM2dDeleteSurface() to delete it.
 * @see mmpM2dDeleteSurface().
 */
M2D_API MMP_RESULT
mmpM2dCreateSurface(
    MMP_M2D_SURFACE targetSurface,
    MMP_M2D_SURFACE *surface);

/**
 * Create a virtual surface structure with the existing image
 * data in memory. A virtual surface can't be used as
 * destination.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageFormat    image format.
 * @param imageDataBits  point to the first pixel data of the image.
 * @param surface        return the created surface handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark The mmpM2dCreateVirtualSurface() can be used to create a surface structure with
 * the existing image data.
 * @remark Notice that virtual surface can't be used as the destination surface when calling drawing functions.
 * @remark When you no longer need the surface, call mmpM2dDeleteVirtualSurface() to delete it.
 * @see mmpM2dDeleteVirtualSurface().
 */
M2D_API MMP_RESULT
mmpM2dCreateVirtualSurface(
    MMP_UINT             imageWidth,
    MMP_UINT             imageHeight,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    MMP_UINT8            *imageDataBits,
    MMP_M2D_SURFACE      *surface);

/**
 * Create a surface in video memory from the existing image data in memory.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageDelta     image width stride (row pitch), in bytes.
 * @param imageFormat    image format.
 * @param imageDataBits  point to the first pixel data of the image.
 * @param surface        return the created surface handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark The mmpM2dCreateSurfaceFromImage() can be used to create a surface in video memory
 * from the existing image data.
 * @remark When you no longer need the surface, call mmpM2dDeleteSurface() to delete it.
 * @see mmpM2dDeleteSurface().
 */
M2D_API MMP_RESULT
mmpM2dCreateSurfaceFromImage(
    MMP_UINT             imageWidth,
    MMP_UINT             imageHeight,
    MMP_UINT             imageDelta,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    const void           *imageDataBits,
    MMP_M2D_SURFACE      *surface);

/**
 * Create a surface in video memory with no image data.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageFormat    image format.
 * @param surface        return the created surface handle.
 * @return The allocated address of the surface if succeed, error codes of NULL otherwise.
 * @remark The mmpM2dCreateNullSurface() can be used to create a surface in video memory
 * with null data.
 * @remark When you no longer need the surface, call mmpM2dDeleteSurface() to delete it.
 * @see mmpM2dDeleteSurface().
 */
M2D_API MMP_RESULT
mmpM2dCreateNullSurface(
    MMP_UINT             width,
    MMP_UINT             height,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    MMP_M2D_SURFACE      *surface);

/**
 * Create a surface with no image data by assigned outside buffer.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageFormat    image format.
 * @param pBuffer        scan ptr start address
 * @param bufferSize     buffer size of ouside buffer
 * @param display        the output of the created surface.
 * @return Whether the function call is success or not.
 * @remark When you no longer need the surface, call mmpM2dDeleteSurface() to delete it.
 * @see mmpM2dDeleteSurface().
 */
M2D_API MMP_RESULT
mmpM2dCreateNullSurfaceByAddr(
    MMP_UINT             imageWidth,
    MMP_UINT             imageHeight,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    MMP_UINT8            *pBuffer,
    MMP_INT32            bufferSize,
    MMP_M2D_SURFACE      *display);

/**
 * Read a block of pixels from the surface.
 *
 * @param   surface     assign a surface handle.
 * @param   destX       The x coordinate of the rectangle you want to write.
 * @param   destY       The y coordinate of the rectangle you want to write.
 * @param   rectWidth   The width of the rectangle you want to write.
 * @param   rectHeight  The height of the rectangle you want to write.
 * @param   bufPtr      The point of the buffer.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dDrawSurfaceRegion().
 * @remark
 */
M2D_API MMP_RESULT
mmpM2dReadSurfaceRegion(
    MMP_M2D_SURFACE surface,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        rectWidth,
    MMP_UINT        rectHeight,
    MMP_UINT8       *bufPtr);

/**
 * Write a block of pixels to the surface.
 *
 * @param   surface     assign a surface handle.
 * @param   destX       The x coordinate of the rectangle you want to write.
 * @param   destY       The y coordinate of the rectangle you want to read.
 * @param   rectWidth   The width of the rectangle you want to read.
 * @param   rectHeight  The height of the rectangle you want to read.
 * @param   bufPtr      The point of the buffer.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dReadSurfaceRegion().
 */
M2D_API MMP_RESULT
mmpM2dDrawSurfaceRegion(
    MMP_M2D_SURFACE surface,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        rectWidth,
    MMP_UINT        rectHeight,
    MMP_UINT8       *bufPtr);

/**
 * Delete a logical surface, and frees all system resources accociated
 * with the surface.
 *
 * @param object    the surface handle that is being deleted.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark This API is used to release the memory for a source surface you created before,
 * if you don't need to use the surface any more.
 * @see mmpM2dCreateSurface(), mmpM2dCreateSurfaceFromImage().
 */
M2D_API MMP_RESULT
mmpM2dDeleteSurface(
    MMP_M2D_SURFACE surface);

/**
 * Delete a logical surface, and frees all system resources accociated
 * with the surface.
 *
 * @param object    the surface handle that is being deleted.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark This API is used to release the structure for a source surface you created before,
 * if you don't need to use the surface any more.
 * @see mmpM2dCreateVirtualSurface().
 */
M2D_API MMP_RESULT
mmpM2dDeleteVirtualSurface(
    MMP_M2D_SURFACE surface);

/**
 * Select a logical object into the specified surface.
 *
 * @param surface           surface handle.
 * @param object            selected object handle.
 * @param prevObject        return the previously selected object of the
 *                          specified type.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Brush, pen and font are regarded as objects in our 2D API.
 * Before using a certain object, one should select it into a surface to be drawn.
 * @remark When you no longer need the object, call mmpM2dDeleteObject() to delete it.
 */
M2D_API MMP_RESULT
mmpM2dSelectObject(
    MMP_M2D_OBJECT_TYPE objectType,
    MMP_M2D_SURFACE     surface,
    MMP_M2D_HANDLE      object,
    MMP_M2D_HANDLE      *prevObject);

/**
 * Delete a logical object, and frees all system resources accociated with the
 * object.
 *
 * @param object        the object that is being deleted.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark This API is used to release the memory for a object you created before,
 * if you don't need to use this object any more.
 * @see mmpM2dSelectObject().
 */
M2D_API MMP_RESULT
mmpM2dDeleteObject(
    MMP_M2D_OBJECT_TYPE objectType,
    MMP_M2D_HANDLE      object);

//=============================
//  Pen Funcitons
//=============================

/**
 * Create a logical pen with the specified ilne attributes.
 *
 * @param penColor          pen color.
 * @param penStyle          pen style - SOLID, DASH, DOT, DASHDOT, DASHDOTDOT.
 * @param userDefinedStyle  The pen style of user defined.
 * @param penWidth          pen width, sets to 1 if not solid pen.
 * @param penObject         return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Pen object should be created before drawing line by mmpM2dDrawLine() or mmpM2dDrawPolyLines().
 * Otherwise, the default pen object (solid pen with black) is used.
 * @remark When you no longer need the pen, call mmpM2dDeleteObject() to delete it.
 * @see mmpM2dSelectObject(), mmpM2dDeleteObject().
 */
M2D_API MMP_RESULT
mmpM2dCreatePen(
    MMP_M2D_COLOR     penColor,
    MMP_M2D_PEN_STYLE penStyle,
    MMP_UINT32        userDefinedStyle,
    MMP_UINT          penWidth,
    MMP_M2D_HANDLE    *penObject);

/**
 * Get the current pen color for the specified surface.
 *
 * @param surface       surface handle.
 * @param penColor      return the current pen color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetPenColor();
 */
M2D_API MMP_RESULT
mmpM2dGetPenColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   *penColor);

/**
 * Set the current pen color to a specified color.
 *
 * @param surface            surface handle.
 * @param penColor           pen color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark The format of penColor is RGB565, please refers to MMP_M2D_COLOR.
 * @see mmpM2dGetPenColor();
 */
M2D_API MMP_RESULT
mmpM2dSetPenColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   penColor);

/**
 * Get the current pen style for the specified surface.
 *
 * @param surface       surface handle.
 * @param penStyle      return the current pen style.
 * @param userDefinedStyle  return the pen style of user defined.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetPenStyle();
 */
M2D_API MMP_RESULT
mmpM2dGetPenStyle(
    MMP_M2D_SURFACE   surface,
    MMP_M2D_PEN_STYLE *penStyle,
    MMP_UINT32        *userDefinedStyle);

/**
 * Set the current pen style to the specified pen style.
 *
 * @param surface            surface handle.
 * @param penStyle           pen style.
 * @param userDefinedStyle  The pen style of user defined.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Threre are four kinds of pen style supported by our 2D API. Please refers to
 * MMP_PEN_STYLE to see the supported pen styles.
 * @remark The default pen style is set to MMP_M2D_PEN_STYLE_SOLID.
 * @see mmpM2dGetPenStyle();
 */
M2D_API MMP_RESULT
mmpM2dSetPenStyle(
    MMP_M2D_SURFACE   surface,
    MMP_M2D_PEN_STYLE penStyle,
    MMP_UINT32        userDefinedStyle);

/**
 * Get the current foreground mix mode.
 *
 * @param surface   surface handle.
 * @param rop2      return the current foreground mix mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetRop2();
 */
M2D_API MMP_RESULT
mmpM2dGetRop2(
    MMP_M2D_SURFACE surface,
    MMP_M2D_ROP2    *rop2);

/**
 * Set the current foreground mix mode. Use the foreground mix mode to combine
 * pens and interiors of filled objects with the colors already on the screen.
 * The foreground mix mode defines how colors from the brush or pen and the
 * colors in the existing image are to be combined.
 *
 * @param surface   surface handle.
 * @param rop2      foreground mix mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark ROP2 can be applied to show various effects of line drawing.
 * Therefore, if one attends to use ROP2 to show the specified special effect, it should be assigned
 * before mmpM2dDrawLine().
 * @see mmpM2dGetRop2();
 */
M2D_API MMP_RESULT
mmpM2dSetRop2(
    MMP_M2D_SURFACE surface,
    MMP_M2D_ROP2    rop2);

//=============================
//  Brush Functions
//=============================

/**
 * Create a logical brush with the specified solid color.
 *
 * @param brushColor    brush color value.
 * @param brushObject   return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark A solid brush is an object that is used to paint the interiors of filled shapes.
 * After an application creates a brush by calling mmpM2dCreateSolidBrush(),
 * it can select that brush into any surface by calling mmpM2dSelectObject().
 * @remark When you no longer need the brush, call mmpM2dDeleteObject() to delete it.
 */
M2D_API MMP_RESULT
mmpM2dCreateSolidBrush(
    MMP_M2D_COLOR  brushColor,
    MMP_M2D_HANDLE *brushObject);

/**
 * Create a logical brush with specified hatch pattern and color.
 *
 * @param hatchStyle            hatch style.
 * @param hatchForegroundColor  foreground color of the hatch brush.
 * @param brushObject           return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark A hatch brush is an object that is used to paint the interiors of filled shapes.
 * There are six default styles for hatch brush, please refers to MMP_HATCH_STYLE.
 * After an application creates a brush by calling mmpM2dCreateHatchBrush(),
 * it can select that brush into any surface by calling mmpM2dSelectObject().
 * @remark When you no longer need the brush you created before, call mmpM2dDeleteObject() to delete it.
 */
M2D_API MMP_RESULT
mmpM2dCreateHatchBrush(
    MMP_M2D_HATCH_STYLE hatchStyle,
    MMP_UINT8           *userDefinedStyle,
    MMP_M2D_COLOR       hatchForegroundColor,
    MMP_M2D_HANDLE      *brushObject);

/**
 * Create a logical brush with the specified bitmap pattern.
 *
 * @param imagePattern    image handle to be used to creat the logical brush.
 * @param brushObject     return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark A pattern brush is an object that is used to paint the interiors of filled shapes.
 * The pattern brush created from a bitmap which is created by mmpM2dCreateSurfaceFromImage() or mmpM2dCreateVirtualSurface().
 * After an application creates a brush by calling mmpM2dCreatePatternBrush(),
 * it can select that brush into any surface by calling mmpM2dSelectObject().
 * @remark When you no longer need the brush you created before, call mmpM2dDeleteObject() to delete it.
 */
M2D_API MMP_RESULT
mmpM2dCreatePatternBrush(
    MMP_UINT       pitch,
    const void     *imageDataPtr,
    MMP_M2D_HANDLE *brushObject);

/**
 * Get the brush origin of the specified surface.
 *
 * @param surface       surface handle.
 * @param originPos     return the coordinates of brush origin.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetBrushOrigin().
 */
M2D_API MMP_RESULT
mmpM2dGetBrushOrigin(
    MMP_M2D_SURFACE surface,
    MMP_M2D_POINT   *originPos);

/**
 * Set the brush origin to the specified position.
 *
 * @param surface         surface handle.
 * @param newOriginX      x-coordinate of new brush origin position.
 * @param newOriginY      y-coordinate of new brush origin position.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark One can change the origin of a brush after created it to show different effect.
 * @see mmpM2dGetBrushOrigin().
 */
M2D_API MMP_RESULT
mmpM2dSetBrushOrigin(
    MMP_M2D_SURFACE surface,
    MMP_INT         newOriginX,
    MMP_INT         newOriginY);

/**
 * Get brush color of the specified surface.
 *
 * @param surface       surface handle.
 * @param fgColor       return foreground color.
 * @param bgColor       return background color.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetBrushColor().
 */
M2D_API MMP_RESULT
mmpM2dGetBrushColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   *fgColor,
    MMP_M2D_COLOR   *bgColor);

/**
 * Set brush color to the specified position.
 *
 * @param surface       surface.
 * @param fgColor       foreground color.
 * @param bgColor       background color.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dGetBrushColor().
 */
M2D_API MMP_RESULT
mmpM2dSetBrushColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   fgColor,
    MMP_M2D_COLOR   bgColor);

//=============================
//  Font Functions
//=============================

/**
 * Create a font object.
 *
 * @param fontObject    return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark A font object must be created before an application calls mmpM2dDrawText().
 * After an application creates a font object and load a font file by calling mmpM2dLoadFont(),
 * it can select the font object into any surface by calling mmpM2dSelectObject().
 * @remark When you no longer need the font object, call mmpM2dDeleteObject() to delete it.
 */
M2D_API MMP_RESULT
mmpM2dCreateFont(
    MMP_M2D_HANDLE *fontObject);

/**
 * Load a font file.
 *
 * @param fontObject    select the created font object.
 * @param fontPtr       pointer of the font array.
 * @param fontWidth     font width (size in bits).
 * @param fontHeight    font height.
 * @param limitL        the lower limit of the Big-5/ASCII code of a font file.
 * @param limitH        the higher limit of the Big-5/ASCII code of a font file.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark This API is used to assign a font file to a font object created before.
 */
M2D_API MMP_RESULT
mmpM2dLoadFont(
    MMP_M2D_HANDLE    fontObject,
    MMP_UINT8         *fontPtr,
    MMP_UINT          fontWidth,
    MMP_UINT          fontHeight,
    MMP_M2D_FONT_CODE fontCode,
    MMP_UINT16        limitL,
    MMP_UINT16        limitH);

/**
 * Load a font file.
 *
 * @param fontObject    select the created font object.
 * @param fontPtr       pointer of the font array.
 * @param fontWidth     font width (size in 4 bits).
 * @param fontHeight    font height.
 * @param limitL        the lower limit of the Big-5/ASCII code of a font file.
 * @param limitH        the higher limit of the Big-5/ASCII code of a font file.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark This API is used to assign a font file to a font object created before.
 */
M2D_API MMP_RESULT
mmpM2dLoadAAFont(
    MMP_M2D_HANDLE    fontObject,
    MMP_UINT8         *fontPtr,
    MMP_UINT          fontWidth,
    MMP_UINT          fontHeight,
    MMP_M2D_FONT_CODE fontCode,
    MMP_UINT16        limitL,
    MMP_UINT16        limitH);

/**
 * Get the current font style from a specified surface.
 *
 * @param surface       the handle of surface.
 * @param fontStyle     return the current font style.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetFontStyle();
 */
M2D_API MMP_RESULT
mmpM2dGetFontStyle(
    MMP_M2D_SURFACE    surface,
    MMP_M2D_FONT_STYLE *fontStyle);

/**
 * Change the current font style of a specified surface.
 *
 * @param surface       the handle of surface.
 * @param fontStyle     font style.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark Our 2D API support five kinds of special effect for font drawing. Please refers to
 * MMP_FONT_STYLE to see the supported font styles.
 * @remark The default font style is set to MMP_M2D_FS_NORMAL.
 * @see mmpM2dGetFontStyle();
 */
M2D_API MMP_RESULT
mmpM2dSetFontStyle(
    MMP_M2D_SURFACE    surface,
    MMP_M2D_FONT_STYLE fontStyle);

/**
 * Get the current font color of the specified surface.
 *
 * @param surface       surface handle.
 * @param fontColor     return the current font color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetFontColor();
 */
M2D_API MMP_RESULT
mmpM2dGetFontColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   *fontColor);

/**
 * Set the current font color to the specified color value.
 *
 * @param surface             surface handle.
 * @param fontColor           font color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @remark The format of penColor is RGB565, please refers to MMP_M2D_COLOR.
 * @remark Note that the mmpM2dSetFontColor() function has to use after select a font object by mmpM2dSelectObject()
 * into a certain surface.
 * @see mmpM2dGetFontColor();
 */
M2D_API MMP_RESULT
mmpM2dSetFontColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   fontColor);

/**
 * Write the string on the specified display context.
 *
 * @param destSurface             surface handle.
 * @param startX                  start x.
 * @param startY                  start y.
 * @param string                  string.
 * @param wordCount               number of word be written.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dDrawAAText();
 */

M2D_API MMP_RESULT
mmpM2dDrawAAText(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         startX,
    MMP_INT         startY,
    MMP_UINT8       *string,
    MMP_UINT        wordCount);

/**
 * Write the string on the specified surface.
 *
 * @param surface     handle to destination surface.
 * @param fontPtr     font source address
 * @param startX      x-coordinate of starting position.
 * @param startY      y-coordinate of starting position.
 * @param fontWidth   width of font, width have to be even.
 * @param fontHeight  height of font.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dDrawAABmp(
    MMP_M2D_SURFACE destSurface,
    void            *fontPtr,//MMP_UINT8*  fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight);

/**
 * Write the string on the specified surface.
 *
 * @param surface     handle to destination surface.
 * @param fontPtr     font source address
 * @param startX      x-coordinate of starting position.
 * @param startY      y-coordinate of starting position.
 * @param fontWidth   width of font, width have to be even.
 * @param fontHeight  height of font.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dDrawAABmpTtx(
    MMP_M2D_SURFACE destSurface,
    void            *fontPtr,//MMP_UINT8*  fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight);

/**
 * add wait 2d idle for font buffer reuse
 */
M2D_API MMP_RESULT
mmpM2dDrawAABmpTtx2(
    MMP_M2D_SURFACE destSurface,
    void            *fontPtr,//MMP_UINT8*  fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight);

M2D_API MMP_RESULT
mmpM2dDrawCmpAABmpTtx2(
    MMP_M2D_SURFACE destSurface,
    void            *pCmpGlyph,//M2D_COMP_GLYPH*     ptCmpGlyph,//MMP_UINT8*  fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight);

M2D_API MMP_RESULT
mmpM2dDrawAABmpTtx3(
    MMP_M2D_SURFACE destSurface,
    void            *fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight);

M2D_API MMP_RESULT
mmpM2dDrawTrueTypeText(
    MMP_M2D_SURFACE destSurface,
    MMP_UINT        destX,
    MMP_UINT        destY,
    void            *path);
//=============================
//  Clipping Functions
//=============================
/**
 * Get the current clipping region(s) for the specified surface.
 * If the clipping region returned is NULL and the amount of region is zero,
 * the clipping region is the default clipping region which is the boundary of
 * a surface.
 *
 * @param surface            surface handle.
 * @param clipRegion         return the clipping region(s).
 * @param clipRegionCount    return the amount of the clipping region(s).
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetClipRectRegion();
 */
M2D_API MMP_RESULT
mmpM2dGetClipRectRegion(
    MMP_M2D_SURFACE surface,
    MMP_M2D_RECT    **clipRegion,
    MMP_UINT        *clipRegionCount);

/**
 * Set the clipping region of the surface. No intersection is allowed between
 * the clip rectangles.
 *
 * @param surface           surface handle.
 * @param clipRegion        the clipping region(s) and intersection is not
 *                          allowed. If clipRegion is NULL, the region of the
 *                          surface is the boundary of the surface.
 * @param clipRegionCount   the amount of the clipping region(s). If clipRegion
 *                          is NULL, clipRegionCount should be zero.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dGetClipRegion();
 */
M2D_API MMP_RESULT
mmpM2dSetClipRectRegion(
    MMP_M2D_SURFACE surface,
    MMP_M2D_RECT    *clipRegion,
    MMP_UINT        clipRegionCount);

/**
 * Reset the clipping region of the specified display to be NULL clipping.
 *
 * @param display           display context.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dResetClipRegion(
    MMP_M2D_SURFACE display);
/*
 * @example clipping.c
 * The example shows how to apply clipping API to a bitmap.
 */
//@}

M2D_API MMP_UINT32
mmpM2dGetSurfaceBaseAddress(
    MMP_SURFACE surface);

void
mmpM2dSetSurfaceBaseAddress(
    MMP_SURFACE surface,
    MMP_UINT32  addr);

M2D_API void
mmpM2dSetVisibleLCD(
    MMP_M2D_LCD_TYPE lcdType);

M2D_API void
mmpM2dWaitIdle();

M2D_API MMP_RESULT
mmpM2dTransformations(
    MMP_M2D_SURFACE dest,
    MMP_INT         dx,
    MMP_INT         dy,
    MMP_M2D_SURFACE src,
    MMP_INT         cx,
    MMP_INT         cy,
    float           angle,
    float           scale);

#ifdef __cplusplus
}
#endif

#endif /* __MMP_GRAPHICS_H */