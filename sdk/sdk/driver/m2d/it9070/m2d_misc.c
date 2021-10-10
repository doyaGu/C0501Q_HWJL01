/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Miscellaneous display context operation functions.
 *
 * @author Erica Chang
 * @version 0.1
 */
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_dispctxt.h"

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
 * Get the current position for the specified display context.
 *
 * @param display       display context handle.
 * @param position      return the coordinates of current position.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *         otherwise.
 * @see mmpM2dSetCurrentPosition().
 */
MMP_RESULT
mmpM2dGetCurrentPosition(
    MMP_M2D_SURFACE    display,
    MMP_M2D_POINT*     position)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }
    
    *position = ((M2D_SURFACE*)display)->currPos;

    return MMP_RESULT_SUCCESS;
        
} // End of mmpGetCurrentPosition()

/**
 * Set the current position to the specified position.
 *
 * @param display   display context handle.
 * @param newX      x-coordinate of new current position.
 * @param newY      y-coordinate of new current position.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *         otherwise.
 * @see mmpGetCurrentPosition(). 
 */
MMP_RESULT
mmpM2dSetCurrentPosition(
    MMP_M2D_SURFACE		display,
    MMP_INT				newX,
    MMP_INT				newY)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE*)display)->currPos.x = newX;
    ((M2D_SURFACE*)display)->currPos.y = newY;

    return MMP_RESULT_SUCCESS;   

} // End of mmpM2dSetCurrentPosition()

/**
 * Get the current background mode for the specified display context.
 *
 * @param display         display context handle.
 * @param backgroundMode  return the current background mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpSetBackgroundMode().
 */
MMP_RESULT
mmpM2dGetBackgroundMode(
    MMP_M2D_SURFACE				display,
    MMP_M2D_BACKGROUND_MODE*    backgroundMode)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *backgroundMode = ((M2D_SURFACE*)display)->backgroundMode;

    return MMP_RESULT_SUCCESS;

} // End of mmpGetBackgroundMode()

/**
 * Set the current background mode to the specified mode.
 *
 * @param display           display context handle.
 * @param backgroundMode    background mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *         otherwise.
 * @see mmpGetBackgroundMode().
 */
MMP_RESULT
mmpM2dSetBackgroundMode(
    MMP_M2D_SURFACE				display,
    MMP_M2D_BACKGROUND_MODE		backgroundMode)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE*)display)->backgroundMode = backgroundMode;

    return MMP_RESULT_SUCCESS;

} // End of mmpSetBackgroundMode()

#ifndef SHRINK_CODE_SIZE
/**
 * Get the current background color for the specified display context.
 *
 * @param display           display context handle.
 * @param backgroundColor   return the current background color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpSetBackgroundColor().
 */
MMP_RESULT
mmpM2dGetBackgroundColor(
    MMP_M2D_SURFACE         display,
    MMP_M2D_COLOR*          backgroundColor)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *backgroundColor = ((M2D_SURFACE*)display)->backgroundColor;

    return MMP_RESULT_SUCCESS;

} // End of mmpGetBackgroundColor()
#endif    

#ifndef SHRINK_CODE_SIZE
/**
 * Set the current background color to the specified color value.
 *
 * @param display           display context handle.
 * @param backgroundColor   background color value.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *         otherwise.
 * @see mmpGetBackgroundColor().
 */
MMP_RESULT
mmpM2dSetBackgroundColor(
    MMP_M2D_SURFACE      display,
    MMP_M2D_COLOR        backgroundColor)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE*)display)->backgroundColor = backgroundColor;

    return MMP_RESULT_SUCCESS;

} // End of mmpSetBackgroundColor() 
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Get the current foreground mix mode.
 *
 * @param display   display context handle.
 * @param rop2      return the current foreground mix mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpSetRop2();
 */
MMP_RESULT
mmpM2dGetRop2(
    MMP_M2D_SURFACE    display,
    MMP_M2D_ROP2*      rop2)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *rop2 = ((M2D_SURFACE*)display)->rop2;

    return MMP_RESULT_SUCCESS;
} // End of mmpGetRop2()     
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Set the current foreground mix mode. Use the foreground mix mode to combine 
 * pens and interiors of filled objects with the colors already on the screen. 
 * The foreground mix mode defines how colors from the brush or pen and the 
 * colors in the existing image are to be combined. 
 *
 * @param display   display context handle.
 * @param rop2      foreground mix mode.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *         otherwise.
 * @see mmpGetRop2();
 */
MMP_RESULT
mmpM2dSetRop2(
    MMP_M2D_SURFACE     display,
    MMP_M2D_ROP2        rop2)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE*)display)->rop2 = rop2;

    return MMP_RESULT_SUCCESS;

} // End of mmpSetRop2()    
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Get the brush origin of the specified display context.
 *
 * @param display       display context handle.
 * @param originPos     return the coordinates of brush origin.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *         otherwise.
 * @see mmpSetBrushOrigin().
 */
MMP_RESULT
mmpM2dGetBrushOrigin(
    MMP_M2D_SURFACE        display,
    MMP_M2D_POINT*         originPos)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *originPos = ((M2D_SURFACE*)display)->brushOriginPos;

    return MMP_RESULT_SUCCESS;
} // End of mmpGetBrushOrigin()         
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Set the brush origin to the specified position.
 *
 * @param display         display context handle.
 * @param newOriginX      x-coordinate of new brush origin position.
 * @param newOriginY      y-coordinate of new brush origin position.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR 
 *         otherwise.
 * @see mmpGetBrushOrigin(). 
 */
MMP_RESULT
mmpM2dSetBrushOrigin(
    MMP_M2D_SURFACE		display,
    MMP_INT				newOriginX,
    MMP_INT				newOriginY)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    ((M2D_SURFACE*)display)->brushOriginPos.x = newOriginX;
    ((M2D_SURFACE*)display)->brushOriginPos.y = newOriginY;

    return MMP_RESULT_SUCCESS;

} // End of mmpSetBrushOrigin()
#endif
