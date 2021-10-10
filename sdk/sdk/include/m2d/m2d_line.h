/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Header file of brush functions.
 *
 * @author Mandy Wu
 * @version 0.1
 */

#ifndef __LINE_H
#define __LINE_H

#include "m2d/m2d_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
//#ifdef MM360
////For the patch of line drawing (MM360)
//#define    LINE_PATCH_DIFF_RANGE            0x200 // 512
//#define    LINE_SHIFT_POSITIVE_ONE_POINT    0x0
//#define    LINE_SHIFT_POSITIVE_TWO_POINT    0x1
//#endif

typedef enum LINE_DIRECTION_TAG
{
    LINE_VERTICAL,
    LINE_HORIZONTAL,
    LINE_RIGHT_BOTTOM,
    LINE_LEFT_BOTTOM
} LINE_DIRECTION;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
/**
 * Calculate  essential parameters for drawing line.
 *
 * @param startPos          Start coordinate of a line.
 * @param endPos            End coordinate of a line.
 * @param majorAxialPixel   Return pixel count of the major axial.
 * @param k1Term            Return k1 term of a line (plz refer to the spec. of 2d engine).
 * @param k2Term            Return k2 term of a line (plz refer to the spec. of 2d engine).
 * @param errorTerm         Return error term of a line (plz refer to the spec. of 2d engine).
 * @param cmd1              Return command 1.
 */
MMP_BOOL
M2D_GetLineParam(
    MMP_M2D_POINT startPos,
    MMP_M2D_POINT endPos,
    MMP_M2D_POINT *newStartPos,
    MMP_UINT32    *majorAxialPixel,
    MMP_INT32     *k1Term,
    MMP_INT32     *k2Term,
    MMP_INT32     *errorTerm,
    MMP_UINT16    *cmd1);

/**
 * Set parameters for drawing line by HW acceleration.
 *
 * @param dstSurface        The handle of destination surface.
 * @param majorAxialPixel   pixel count of the major axial.
 * @param k1Term            k1 term of a line (plz refer to the spec. of 2d engine).
 * @param k2Term            k2 term of a line (plz refer to the spec. of 2d engine).
 * @param errorTerm         error term of a line (plz refer to the spec. of 2d engine).
 * @param cmd1              command 1.
 */
M2D_API MMP_BOOL
M2D_SetLineParam(
    M2D_SURFACE *dstSurface,
    MMP_UINT32  majorAxialPixel,
    MMP_INT32   k1Term,
    MMP_INT32   k2Term,
    MMP_INT32   errorTerm,
    MMP_UINT16  cmd1);

//#ifdef MM360
//#ifndef M2D_DRAW_NT_LINE
///**
// * MM360 line patch.
// *
// * @param oriSX         Original start x-coordinate.
// * @param oriSY         Original start y-coordinate.
// * @param oriEX         Original end x-coordinate.
// * @param oriEY         Original end x-coordinate.
// * @param newSX         New start x-coordinate.
// * @param newSY         New start y-coordinate.
// * @param newEX         New end x-coordinate.
// * @param newEY         New end y-coordinate.
// */
//MMP_BOOL
//M2D_PatchLine(
//  MMP_INT32       oriSX,
//  MMP_INT32       oriSY,
//  MMP_INT32       oriEX,
//  MMP_INT32       oriEY,
//  MMP_INT32*      newSX,
//  MMP_INT32*      newSY,
//  MMP_INT32*      newEX,
//  MMP_INT32*      newEY);
//
///**
// * MM360 line patch, shift x.y by Bresenham's algorithm.
// *
// * @param oriSX         Original start x-coordinate.
// * @param oriSY         Original start y-coordinate.
// * @param oriEX         Original end x-coordinate.
// * @param oriEY         Original end x-coordinate.
// * @param XPosOnAxisY   .
// * @param XPosOnAxisX   .
// * @param shiftOp       .
// */
//MMP_BOOL
//M2D_PatchLine2(
//  MMP_INT32       oriSX,
//  MMP_INT32       oriSY,
//  MMP_INT32       oriEX,
//  MMP_INT32       oriEY,
//  MMP_INT32*      XPosOnAxisY,
//  MMP_INT32*      XPosOnAxisX,
//  MMP_UINT32      shiftOp);
//
//#endif// #ifndef M2D_DRAW_NT_LINE
//#endif// #ifdef MM360

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __LINE_H