/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Header file of font functions.
 *
 * @author Mandy Wu
 * @version 0.1
 */

#ifndef __FONT_H
#define __FONT_H

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
 * Generate the specific font in to VRAM
 *
 * @param offset		the offset in font file.
 * @param fontWidth		font width.
 * @param fontHeight	font height.
 * @param fontFilePtr   the pointer of the font in font file.
 * @return the pointer of the font in VRAM.
 *
 */
MMP_UINT8 *
M2D_GeneCmpFontBytes(
    MMP_UINT32 fontWidth,
    MMP_UINT32 fontHeight,
    MMP_UINT32 fontPitch,
    MMP_UINT8  *fontFilePtr);

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __FONT_H