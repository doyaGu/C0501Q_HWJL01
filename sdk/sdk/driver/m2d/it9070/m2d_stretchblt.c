/*
 * Copyright (c) 2011 ITE technology Systems Corp. All Rights Reserved.
 */
/** @file
 * Linear frame buffer stretch bitblt.
 *
 * @author James Lin
 * @version 1.0
 */

#include "m2d/m2d_graphics.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_rotate.h"

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
 * Transfer a block of image from the source rectangle to the destination
 * rectangle, stretch or shrink to fit the dimensions of the destination
 * rectangle.
 *
 * @param destSurface   handle to destination display context.
 * @param destX         x-coordinate of destination upper-left corner.
 * @param destY         y-coordinate of destination upper-left corner.
 * @param destWidth     width of destination rectangle.
 * @param destHeight    height of destination rectangle.
 * @param srcSurface    handle to source display context.
 * @param srcX          x-coordinate of source upper-left corner.
 * @param srcY          y-coordinate of source upper-left corner.
 * @param srcWidth      width of source rectangle.
 * @param srcHeight     height of source rectangle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
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
    MMP_UINT        srcHeight)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(destSurface) || IS_INVALIDATE_SURFACE(srcSurface))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(destSurface))
    {
        return MMP_RESULT_ERROR_DEST_DISP_FORMAT;
    }

    //
    // HW acceleration
    //
    return m2dStretchSrcCopy((M2D_SURFACE *)destSurface,
                               destX,
                               destY,
                               destWidth,
                               destHeight,
                               (M2D_SURFACE *)srcSurface,
                               srcX,
                               srcY,
                               srcWidth,
                               srcHeight);
} //mmpM2dStretchSrcCopy()