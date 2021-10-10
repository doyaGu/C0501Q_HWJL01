/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Header file of brush functions.
 *
 * @author Mandy Wu
 * @version 0.1
 */

#ifndef __BRUSH_H
#define __BRUSH_H

#include "m2d/m2d_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

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
//                              Function Declaration
//=============================================================================

/**
 * Create brush object.
 */
M2D_BRUSHOBJ *
M2D_CreateBrush(void);

#ifndef SHRINK_CODE_SIZE
/**
 * Check if the origin position of the brush is changed.
 *
 * @param display   handle to display context.
 *
 * @author Mandy Wu
 */
MMP_BOOL
M2D_BrushOriginPos(
    M2D_SURFACE *display);
#endif

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __BRUSH_H