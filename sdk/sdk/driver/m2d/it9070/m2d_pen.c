/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * create brush object.
 *
 * @author Mandy Wu
 * @version 0.1
 */
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_pen.h"
#include "m2d/m2d_line.h"

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
 * Create a logical pen with the specified ilne attributes.
 *
 * @param penColor          pen color.
 * @param penStyle          pen style - SOLID, DASH, DOT, DASHDOT, DASHDOTDOT.
 * @param userDefinedStyle	The pen style of user defined.
 * @param penWidth          pen width, sets to 1 if not solid pen.
 * @param penObject         return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpSeleteObject(), mmpM2dDeleteObject().
 */
MMP_RESULT
mmpM2dCreatePen(
    MMP_M2D_COLOR     penColor,
    MMP_M2D_PEN_STYLE penStyle,
    MMP_UINT32        userDefinedStyle,
    MMP_UINT          penWidth,
    MMP_M2D_HANDLE    *penObject)
{
    M2D_PENOBJ *newPenobj = NULL;

    newPenobj = M2D_CreatePen();

    if (newPenobj == NULL)
        return MMP_RESULT_ERROR;

    switch (penStyle)
    {
    case MMP_M2D_PEN_STYLE_DASH:
        userDefinedStyle = M2D_PEN_STYLE_DASH;
        break;

    case MMP_M2D_PEN_STYLE_DOT:
        userDefinedStyle = M2D_PEN_STYLE_DOT;
        break;

    case MMP_M2D_PEN_STYLE_DASH_DOT:
        userDefinedStyle = M2D_PEN_STYLE_DASH_DOT;
        break;

    case MMP_M2D_PEN_STYLE_DASH_DOT_DOT:
        userDefinedStyle = M2D_PEN_STYLE_DASH_DOT_DOT;
        break;
    }//End of switch(penStyle)

    newPenobj->penColor         = penColor;
    newPenobj->penStyle         = penStyle;
    newPenobj->userDefinedStyle = userDefinedStyle;
    newPenobj->penWidth         = penWidth;

    *penObject                  = (MMP_M2D_HANDLE)newPenobj;
    return MMP_RESULT_SUCCESS;
}// End of mmpM2dCreatePen

#ifndef SHRINK_CODE_SIZE
/**
 * Get the current pen color for the specified display context.
 *
 * @param display       display context handle.
 * @param penColor      return the current pen color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dGetPenColor(
    MMP_M2D_SURFACE display,
    MMP_M2D_COLOR   *penColor)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *penColor = ((M2D_SURFACE *)display)->pen->penColor;

    return MMP_RESULT_SUCCESS;
}//End of mmpGetPenColor()
#endif

/**
 * Set the current pen color to the specified color value.
 *
 * @param display            display context handle.
 * @param penColor           pen color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dSetPenColor(
    MMP_M2D_SURFACE display,
    MMP_M2D_COLOR   penColor)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE *)display)->pen->penColor = penColor;

    return MMP_RESULT_SUCCESS;
}//End of mmpSetPenColor()

/**
 * Create an empty memory space for brush.
 *
 * @param newBrushobj    create a brush object.
 *
 */

M2D_PENOBJ *
M2D_CreatePen(void)
{
    M2D_PENOBJ *newPenobj = NULL;

    newPenobj = (M2D_PENOBJ *)malloc(sizeof(M2D_PENOBJ));
    if (newPenobj == NULL)
        return NULL;

    newPenobj->penStructSize = sizeof(M2D_PENOBJ);

    // Default pen object
    newPenobj->penColor      = 0;
    newPenobj->penStyle      = MMP_M2D_PEN_STYLE_SOLID;
    newPenobj->penWidth      = 1;

    return newPenobj;
} // End of M2D_CreatePen

#ifndef SHRINK_CODE_SIZE
/**
 * Get the current pen style.
 *
 * @param surface			surface handle.
 * @param penStyle			return the type of pen style.
 * @param userDefinedStyle	return the pen style of user defined.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
M2D_API MMP_RESULT
mmpM2dGetPenStyle(
    MMP_M2D_SURFACE   display,
    MMP_M2D_PEN_STYLE *penStyle,
    MMP_UINT32        *userDefinedStyle)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *penStyle = ((M2D_SURFACE *)display)->pen->penStyle;

    if (((M2D_SURFACE *)display)->pen->penStyle == MMP_M2D_PEN_STYLE_USER_DEFINED)
        *userDefinedStyle = ((M2D_SURFACE *)display)->pen->userDefinedStyle;
    else
        *userDefinedStyle = 0;

    return MMP_RESULT_SUCCESS;
}//End of mmpM2dGetPenStyle()
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Set the current pen style to a specified style.
 *
 * @param surface			surface handle.
 * @param penStyle			Type of pen style.
 * @param userDefinedStyle	The pen style of user defined.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dSetPenStyle(
    MMP_M2D_SURFACE   display,
    MMP_M2D_PEN_STYLE penStyle,
    MMP_UINT32        userDefinedStyle)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    switch (penStyle)
    {
    case MMP_M2D_PEN_STYLE_DASH:
        userDefinedStyle = M2D_PEN_STYLE_DASH;
        break;

    case MMP_M2D_PEN_STYLE_DOT:
        userDefinedStyle = M2D_PEN_STYLE_DOT;
        break;

    case MMP_M2D_PEN_STYLE_DASH_DOT:
        userDefinedStyle = M2D_PEN_STYLE_DASH_DOT;
        break;

    case MMP_M2D_PEN_STYLE_DASH_DOT_DOT:
        userDefinedStyle = M2D_PEN_STYLE_DASH_DOT_DOT;
        break;
    }//End of switch(penStyle)

    ((M2D_SURFACE *)display)->pen->penStyle         = penStyle;
    ((M2D_SURFACE *)display)->pen->userDefinedStyle = userDefinedStyle;

    return MMP_RESULT_SUCCESS;
}//End of mmpM2dSetPenStyle()
#endif