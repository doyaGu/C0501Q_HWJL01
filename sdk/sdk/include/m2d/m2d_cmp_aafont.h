/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Header file of anti-aliasing font functions.
 *
 * @author Mandy Wu
 * @version 0.1
 */

#ifndef __AAFONT_H
#define __AAFONT_H

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
 * Create font object.
 */
M2D_FONTOBJ *
M2D_CreateFont(void);

/**
 * Write the string on the specified display context.
 *
 * @param display     handle to destination display context.
 * @param startX      x-coordinate of starting position.
 * @param startY      y-coordinate of starting position.
 * @param string      array of character string.
 * @param charCount   the word count of the string.
 * @return MMP_TRUE if succeed, error codes of MMP_FALSE
 *         otherwise.
 */

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __AAFONT_H