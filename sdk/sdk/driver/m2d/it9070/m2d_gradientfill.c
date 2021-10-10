/*
 * Copyright (c) 2011 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Gradient Fill.
 *
 * @author James Lin
 * @version 1.0
 */
#include "ite/mmp_types.h"

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
 * Fill a rectangle in the specified gradient color.
 *
 * @param destDisplay       handle to destination display context.
 * @param destX             x-coordinate of destination upper-left corner.
 * @param destY             y-coordinate of destination upper-left corner.
 * @param destWidth         width of destination rectangle.
 * @param destHeight        height of destination rectangle.
 * @param initColor			the initial color(RGB565)
 * @param endColor			the end color(RGB565)
 * @param direction			the direction of gradient fill
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dGradientFill(
    MMP_M2D_SURFACE         destDisplay,
    MMP_UINT				destX,
    MMP_UINT				destY,
	MMP_UINT				destWidth,
	MMP_UINT				destHeight,
	MMP_M2D_COLOR			initColor,
	MMP_M2D_COLOR			endColor,
	MMP_M2D_GF_DIRECTION	direction)
{
    MMP_BOOL    result = MMP_FALSE;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(destDisplay))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(destDisplay))
    {
        return MMP_RESULT_ERROR_DEST_DISP_FORMAT;
    }

    //
    // HW acceleration
    //

	//vg linear gradients
    result = m2dGradientFill(
        (M2D_SURFACE*)destDisplay,
        destX,
        destY,
        destWidth,
        destHeight,
        initColor,
        endColor,
        direction);

    if (result == MMP_FALSE)
    {
        return MMP_RESULT_ERROR;
    }

    return MMP_RESULT_SUCCESS;

} // mmpGradientFill()