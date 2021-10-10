/*
 * Copyright (c) 2011 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Linear frame buffer bitblt.
 *
 * @author James Lin
 * @version 1.0
 */
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_rotate.h"
#include "m2d/m2d_brush.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

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
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
//                              Function Definition
//=============================================================================
/**
 * Copies the source rectangle directly to the destination rectangle.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param destWidth     width of destination rectangle.
 * @param destHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dSourceCopy(
    MMP_M2D_SURFACE destDisplay,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        destWidth,
    MMP_UINT        destHeight,
    MMP_M2D_SURFACE srcDisplay,
    MMP_INT         srcX,
    MMP_INT         srcY)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(destDisplay) || IS_INVALIDATE_SURFACE(srcDisplay))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(destDisplay))
    {
        return MMP_RESULT_ERROR_DEST_DISP_FORMAT;
    }

    return m2dSourceCopy(
        (M2D_SURFACE *)destDisplay,
        destX,
        destY,
        destWidth,
        destHeight,
        (M2D_SURFACE *)srcDisplay,
        srcX,
        srcY);
} //End of mmpM2dSourceCopy()

#ifndef SHRINK_CODE_SIZE
/**
 * Fill a rectangle by the current brush.
 *
 * @param display   display context handle.
 * @param startX    x-coordinate of the upper-left corner of the rectangle.
 * @param startY    y-coordinate of the upper-left corner of the rectangle.
 * @param width     width of the rectangle.
 * @param height    height of the rectangle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dFillRectangle(
    MMP_M2D_SURFACE display,
    MMP_INT         startX,
    MMP_INT         startY,
    MMP_UINT        width,
    MMP_UINT        height)
{
    MMP_M2D_SURFACE srcDisplay = 0;

    // Use pattern copy to fill a rectangle with the brush color
    return mmpM2dTransferBlock(
        display,
        startX,
        startY,
        width,
        height,
        srcDisplay,
        0,
        0,
        0xF0);
}//End of mmpFillRectangle()

/**
 * Fill a series of rectangles in the specified rectangle array by the current
 * brush.
 *
 * @param display       display context handle.
 * @param rects         array of the rectangles.
 * @param rectCount     the amount of the rectangles in array.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dFillPolyRectangle(
    MMP_M2D_SURFACE    display,
    const MMP_M2D_RECT *rects,
    MMP_UINT           rectCount)
{
    MMP_UINT32 i;
    MMP_INT32  startX, startY;
    MMP_UINT32 width, height;

    for (i = 0; i < rectCount; i++)
    {
        startX = rects[i].left;
        startY = rects[i].top;
        width  = rects[i].right - rects[i].left;
        height = rects[i].bottom - rects[i].top;

        if (mmpM2dFillRectangle(
            display,
            startX,
            startY,
            width,
            height)
            != MMP_SUCCESS)
        {
            return MMP_RESULT_ERROR;
        }
    }
    return MMP_SUCCESS;
}//End of mmpFillPolyRectangle()
#endif

/**
 * Transfer a block of color data from the source to the destination according
 * to the ROP.
 *
 * @param destDisplay   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param destWidth     width of destination rectangle.
 * @param destHeight    height of destination rectangle.
 * @param srcDisplay    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param rop           raster operation.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dTransferBlock(
    MMP_M2D_SURFACE destDisplay,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        destWidth,
    MMP_UINT        destHeight,
    MMP_M2D_SURFACE srcDisplay,
    MMP_INT         srcX,
    MMP_INT         srcY,
    MMP_INT         rop)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(destDisplay))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    // Check if the rop needs source information
    if ((rop ^ (rop >> 2)) & 0x33)
    {
        if (IS_INVALIDATE_SURFACE(srcDisplay))
        {
            return MMP_RESULT_ERROR_INVALID_DISP;
        }
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(destDisplay))
    {
        return MMP_RESULT_ERROR_DEST_DISP_FORMAT;
    }

    //
    // HW acceleration
    //
    return m2dTransferBlock(
        (M2D_SURFACE *)destDisplay,
        destX,
        destY,
        destWidth,
        destHeight,
        srcDisplay,
        srcX,
        srcY,
        rop);
} // mmpM2dTransferBlock()