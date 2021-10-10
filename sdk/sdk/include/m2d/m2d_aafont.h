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
MMP_BOOL
M2D_DrawAAText(
    M2D_SURFACE     *display,
    MMP_UINT        startX,
    MMP_UINT        startY,
    const MMP_UINT8 *string,
    MMP_UINT        wordCount);

/**
 * Generate the specific AA font in to VRAM
 *
 * @param fontWidth		font width.
 * @param fontHeight	font height.
 * @param fontFilePtr   the pointer of the font in font file.
 * @return the pointer of the font in VRAM.
 *
 */
MMP_UINT8 *
M2D_GeneAAFontBytes(
    MMP_UINT32 fontWidth,
    MMP_UINT32 fontHeight,
    MMP_UINT8  *fontFilePtr);

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __AAFONT_H