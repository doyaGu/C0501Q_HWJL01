/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Rotate functions.
 *
 * @author Erica Chang
 * @version 0.1
 */
#include "m2d/m2d_rotate.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_engine.h"

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
//#ifndef SHRINK_CODE_SIZE
/**
 * Get the current coordinate rotation operation for the specified display
 * context.
 *
 * @param display       display context handle.
 * @param rotateOp      return the current coordinate rotation operation.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetRotateOp().
 */
MMP_RESULT
mmpM2dGetRotateOp(
    MMP_M2D_SURFACE   display,
    MMP_M2D_ROTATE_OP *rotateOp)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *rotateOp = ((M2D_SURFACE *)display)->rotateOp;

    return MMP_RESULT_SUCCESS;
} // End of mmpGetRotateOp()

//#endif

//#ifndef SHRINK_CODE_SIZE
/**
 * Set the current coordinate rotation function to the specified rotate
 * operation.
 *
 * @param display       display context handle.
 * @param rotateOp      coordinate rotate operation.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpGetRotateOp().
 */
MMP_RESULT
mmpM2dSetRotateOp(
    MMP_M2D_SURFACE   display,
    MMP_M2D_ROTATE_OP rotateOp)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE *)display)->rotateOp = rotateOp;

    return MMP_RESULT_SUCCESS;
} // End of mmpM2dSetRotateOp()

MMP_RESULT
mmpM2dTransformations(
    MMP_M2D_SURFACE dest,
    MMP_INT         dx,
    MMP_INT         dy,
    MMP_M2D_SURFACE src,
    MMP_INT         cx,
    MMP_INT         cy,
    float           angle,
    float           scale)
{
    return m2dTransformations(
        (M2D_SURFACE *)dest,
        dx,
        dy,
        (M2D_SURFACE *)src,
        cx,
        cy,
        angle,
        scale);
}
//#endif