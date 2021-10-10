/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Font functions.
 *
 * @author Mandy Wu
 * @version 0.1
 */
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_font.h"
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
 * Get the current font color of the specified display context.
 *
 * @param display       display context handle.
 * @param fontColor     return the current font color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetFontColor();
 */
MMP_RESULT
mmpM2dGetFontColor(
    MMP_M2D_SURFACE display,
    MMP_M2D_COLOR   *fontColor)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *fontColor = ((M2D_SURFACE *)display)->font->fontColor;

    return MMP_SUCCESS;
} // mmpGetFontColor()

/**
 * Set the current font color to the specified color value.
 *
 * @param display             display context handle.
 * @param fontColor           font color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpGetFontColor();
 */
MMP_RESULT
mmpM2dSetFontColor(
    MMP_M2D_SURFACE display,
    MMP_M2D_COLOR   fontColor)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE *)display)->font->fontColor = fontColor;

    return MMP_SUCCESS;
} // mmpM2dSetFontColor()

M2D_API MMP_RESULT
mmpM2dDrawBmpTextTtx2(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    void            *srcPtr,
    MMP_UINT        srcWidth,
    MMP_UINT        srcHeight,
    MMP_UINT        srcPitch)
{
    M2D_SURFACE *destDisplay = (M2D_SURFACE *)destSurface;
    MMP_UINT32  fontW        = srcWidth;
    MMP_UINT32  fontH        = srcHeight;

    //hw register
    m2dDrawBmpTextTtx2(destDisplay,
                       destX,
                       destY,
                       srcPtr,
                       fontW,
                       fontH,
                       srcPitch);

    return MMP_SUCCESS;
} // mmpM2dDrawBmpTextTtx2()

M2D_API MMP_RESULT
mmpM2dDrawTrueTypeText(
    MMP_M2D_SURFACE destSurface,
    MMP_UINT        destX,
    MMP_UINT        destY,
    void            *path)
{
    M2D_SURFACE *destDisplay = (M2D_SURFACE *)destSurface;

    //hw register
    m2dDrawFont(
        destDisplay,
        path,
        destX,
        destY);

    return MMP_SUCCESS;
}