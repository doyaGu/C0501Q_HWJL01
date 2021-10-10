/*
 * Copyright (c) 2011 SMedia technology Systems Corp. All Rights Reserved.
 */
/** @file
 * Linear frame buffer transparent bitblit.
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
 * Transfer a block of color data from the source to the destination with
 * transpancy.
 *
 * @param destSurface       handle to destination display context.
 * @param destX             x-coordinate of destination upper-left corner. 
 * @param destY             y-coordinate of destination upper-left corner.
 * @param destWidth         width of destination rectangle.
 * @param destHeight        height of destination rectangle.
 * @param srcSurface        handle to source display context.
 * @param srcX              x-coordinate of source upper-left corner.
 * @param srcY              y-coordinate of source upper-left corner.
 * @param destHighColor     the high value of destination color key (inclusive).
 * @param destLowColor      the low value of destination color key (inclusive).
 * @param srcHighColor      the high value of source color key (inclusive).
 * @param srcLowColor       the low value of source color key (inclusive).
 * @param transparentRop    raster operation for transparency.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @author Erica Chang
 */
MMP_RESULT
mmpM2dDrawTransparentBlock(
    MMP_M2D_SURFACE		destSurface,
    MMP_INT				destX,
    MMP_INT				destY,
    MMP_UINT			destWidth,
    MMP_UINT			destHeight,
    MMP_M2D_SURFACE     srcSurface,
    MMP_INT				srcX,
    MMP_INT				srcY,
    MMP_M2D_COLOR       destHighColor,
    MMP_M2D_COLOR       destLowColor,
    MMP_M2D_COLOR       srcHighColor,
    MMP_M2D_COLOR       srcLowColor,
    MMP_M2D_TROP        transparentRop)
{	
    MMP_RESULT      result = MMP_RESULT_SUCCESS;
	//M2D_SURFACE*    srcDisplay = (M2D_SURFACE*)srcSurface;
	//M2D_SURFACE*    destDisplay = (M2D_SURFACE*)destSurface;

    //MMP_UINT32      srcPitch, srcBase;
	//MMP_UINT32      dstPitch, dstBase; 

	//srcPitch = srcDisplay->pitch;
	//srcBase = srcDisplay->baseAddrOffset;

	//dstPitch = destDisplay->pitch;
	//dstBase = destDisplay->baseAddrOffset;

	result = m2dDrawTransparentBlock(
			    (M2D_SURFACE*)destSurface,
				destX,
				destY,
				destWidth,
				destHeight,
				(M2D_SURFACE*)srcSurface,
				srcX,
				srcY,
				destHighColor,
				destLowColor,
				srcHighColor,
				srcLowColor,
				transparentRop);


    return MMP_RESULT_SUCCESS;

}

