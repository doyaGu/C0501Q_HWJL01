/*
 * Copyright (c) 2011 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Linear frame buffer alpha blending.
 *
 * @author James Lin
 * @version 1.0
 */
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_rotate.h"
#include "m2d/m2d_alphablend.h"

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
 * Display images that have transparent or semitransparent pixels.
 *
 * @param destSurface        handle to destination display context.
 * @param destX              x-coordinate of destination upper-left corner.
 * @param destY              y-coordinate of destination upper-left corner.
 * @param destWidth          width of destination rectangle.
 * @param destHeight         height of destination rectangle.
 * @param srcSurface         handle to source display context.
 * @param srcX               x-coordinate of source upper-left corner.
 * @param srcY               y-coordinate of source upper-left corner.
 * @param alphaFormat        the way the source and destination images are
 *                           interpreted.
 * @param srcConstAlpha      transparency value to be used on the entire source
 *                           image.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @author Erica Chang
 */
MMP_RESULT
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
    MMP_INT               srcConstAlpha)
{
    MMP_BOOL      result            = MMP_FALSE;
    MMP_M2D_POINT srcPos            = {0, 0};
    MMP_BOOL      isSetRotateRefPos = MMP_FALSE;
    HWBlendMode   blendMode         = HW_BLEND_SRC;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(destSurface) || IS_INVALIDATE_SURFACE(srcSurface))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(destSurface))
    {
        return MMP_RESULT_ERROR_DEST_DISP_FORMAT;
    }

    if (IS_INVALIDATE_IMAGE_FORMAT(srcSurface))
    {
        return MMP_RESULT_ERROR_SRC_DISP_FORMAT;
    }

    // Check source format for premultiply alpha
    if ((alphaFormat == MMP_M2D_ABLEND_FORMAT_SRC_PREMULTIPLY)
        && (((M2D_SURFACE *)srcSurface)->imageFormat != MMP_M2D_IMAGE_FORMAT_ARGB8888))
    {
        return MMP_RESULT_ERROR_SRC_DISP_FORMAT;
    }

    //todo:more blending mode
    switch (alphaFormat)
    {
    case MMP_M2D_ABLEND_FORMAT_SRC_CONST:
        blendMode = HW_BLEND_CONSTANT_Alpha;
        break;

    case MMP_M2D_ABLEND_FORMAT_SRC_ALPHA:
        blendMode = HW_BLEND_SRC_OVER;
        break;

    case MMP_M2D_ABLEND_FORMAT_SRC_PREMULTIPLY:
        blendMode = HW_BLEND_MULTIPLY;
        break;
    }

    return m2dAlphaBlend(
        (M2D_SURFACE *)destSurface,
        destX,
        destY,
        destWidth,
        destHeight,
        (M2D_SURFACE *)srcSurface,
        srcX,
        srcY,
        blendMode,
        srcConstAlpha);
} // End of mmpM2dAlphaBlend()