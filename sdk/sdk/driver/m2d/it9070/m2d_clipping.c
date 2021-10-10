/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Clipping functions.
 *
 * @author Erica Chang
 * @version 0.1
 */
#include <malloc.h>
#include "m2d/m2d_graphics.h"
#include "../../include/ite/ith.h"

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
MMP_BOOL    M2D_ResetClipRegion(M2D_SURFACE *display);

//=============================================================================
//                              Function Definition
//=============================================================================
#ifndef SHRINK_CODE_SIZE
/**
 * Get the current clipping region(s) for the specified display context.
 * If the clipping region returned is NULL and the amount of region is zero,
 * the clipping region is the default clipping region which is the boundary of
 * a display.
 *
 * @param display            display context handle.
 * @param clipRegion         return the clipping region(s).
 * @param clipRegionCount    return the amount of the clipping region(s).
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpSetClipRectRegion();
 */
MMP_RESULT
mmpM2dGetClipRectRegion(
    MMP_M2D_SURFACE display,
    MMP_M2D_RECT    **clipRegion,
    MMP_UINT        *clipRegionCount)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *clipRegion      = ((M2D_SURFACE *)display)->clipSet.clipRegion;
    *clipRegionCount = ((M2D_SURFACE *)display)->clipSet.clipRegionCount;

    return MMP_RESULT_SUCCESS;
} // End of mmpGetClipRegion()

#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Set the clipping region of the display. No intersection is allowed between
 * the clip rectangles.
 *
 * @param display           display context handle.
 * @param clipRegion        the clipping region(s) and intersection is not
 *                          allowed. If clipRegion is NULL, the region of the
 *                          display is the boundary of the display context.
 * @param clipRegionCount   the amount of the clipping region(s). If clipRegion
 *                          is NULL, clipRegionCount should be zero.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpGetClipRegion();
 */
MMP_RESULT
mmpM2dSetClipRectRegion(
    MMP_M2D_SURFACE display,
    MMP_M2D_RECT    *clipRegion,
    MMP_UINT        clipRegionCount)
{
    MMP_UINT32   clipSetSize;
    MMP_M2D_RECT *clipSetPtr = NULL;
    MMP_UINT32   i           = 0;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    if (((clipRegion == NULL) && (clipRegionCount != 0))
     || ((clipRegion != NULL) && (clipRegionCount == 0)))
    {
        return MMP_RESULT_ERROR;
    }

    if (M2D_ResetClipRegion((M2D_SURFACE *)display) != MMP_TRUE)
    {
        return MMP_RESULT_ERROR;
    }

    if (clipRegionCount != 0)
    {
        clipSetSize = (sizeof(MMP_M2D_RECT)) * clipRegionCount;
        clipSetPtr  = (MMP_M2D_RECT *)malloc(clipSetSize);
        if (clipSetPtr == NULL)
        {
            return MMP_RESULT_ERROR;
        }

        for (i = 0; i < clipRegionCount; i++)
        {
            *(clipSetPtr + i) = *(clipRegion + i);
        }

        ((M2D_SURFACE *)display)->clipSet.clipRegion      = clipSetPtr;
        ((M2D_SURFACE *)display)->clipSet.clipRegionCount = clipRegionCount;
    } // if (clipRegionCount != 0)

    return MMP_RESULT_SUCCESS;
} // End of mmpSetClipRectRegion()

#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Reset the clipping region of the specified display to be NULL clipping.
 *
 * @param display           display context.
 * @author Erica Chang
 */
MMP_BOOL
M2D_ResetClipRegion(
    M2D_SURFACE *display)
{
    if (display->clipSet.clipRegion == NULL)
    {
        return MMP_TRUE;
    }

    free(display->clipSet.clipRegion);

    display->clipSet.clipRegion      = NULL;
    display->clipSet.clipRegionCount = 0;

    return MMP_TRUE;
} // End of M2D_ResetClipRegion()

#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Reset the clipping region of the specified display to be NULL clipping.
 *
 * @param display           display context.
 * @author Mandy Wu
 */
MMP_RESULT
mmpM2dResetClipRegion(
    MMP_M2D_SURFACE display)
{
    if (!(M2D_ResetClipRegion((M2D_SURFACE *)display)))
        return MMP_RESULT_ERROR;
    else
        return MMP_RESULT_SUCCESS;
} // End of mmpM2dResetClipRegion()

#endif