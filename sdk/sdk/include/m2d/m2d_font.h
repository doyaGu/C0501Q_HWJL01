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
 * Create font object.
 */
M2D_FONTOBJ *
M2D_CreateFont(void);

/**
 * Draw a text to the specified surface.
 *
 * @param destSurface   handle to destination surface.
 * @param startX        the x-coordinate of starting position of the destSurface to be drawn.
 * @param startY        the y-coordinate of starting position of the destSurface to be drawn.
 * @param srcPtr        source address in vram.
 * @param srcWidth      width of the source bitmap.
 * @param srcHeight     height of the source bitmap.
 * @return MMP_TRUE if succeed, error codes of MMP_FALSE otherwise.
 */
M2D_API MMP_BOOL
M2d_DrawBmpText(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_UINT8   *srcPtr,
    MMP_UINT    srcWidth,
    MMP_UINT    srcHeight,
    MMP_UINT    srcPitch);

/**
 * Switch font style from font bitmap
 *
 * @param destSurface   handle to destination surface.
 * @param startX        the x-coordinate of starting position of the destSurface to be drawn.
 * @param startY        the y-coordinate of starting position of the destSurface to be drawn.
 * @param srcPtr        source address in vram.
 * @param srcWidth      width of the source bitmap.
 * @param srcHeight     height of the source bitmap.
 * @return MMP_TRUE if succeed, error codes of MMP_FALSE otherwise.
 */
void
M2d_DrawBmpFontSwitchSytle(
    M2D_SURFACE *destSurface,
    MMP_INT     destX,
    MMP_INT     destY,
    MMP_UINT8   *srcPtr,
    MMP_UINT    srcWidth,
    MMP_UINT    srcHeight,
    MMP_UINT    srcPitch);

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
M2D_DrawText(
    M2D_SURFACE     *display,
    MMP_UINT        startX,
    MMP_UINT        startY,
    const MMP_UINT8 *string,
    MMP_UINT        wordCount);

/**
 * Handle different font style.
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
M2D_SwitchFontStyle(
    M2D_SURFACE     *display,
    MMP_UINT        startX,
    MMP_UINT        startY,
    const MMP_UINT8 *string,
    MMP_UINT        wordCount);

/**
 * Transform Big-5 code to serial number.
 *
 * @param wordPtr     the pointer of the input word.
 * @return the serial number of the word.
 *
 */
MMP_UINT16
M2D_TransWordptrSer(
    MMP_UINT8 *wordPtr);

/**
 * Generate the specific font in to VRAM
 *
 * @param offset        the offset in font file.
 * @param fontWidth     font width.
 * @param fontHeight    font height.
 * @param fontFilePtr   the pointer of the font in font file.
 * @return the pointer of the font in VRAM.
 *
 */
MMP_UINT8 *
M2D_GeneFontBytes(
    MMP_UINT32 fontWidth,
    MMP_UINT32 fontHeight,
    MMP_UINT32 fontPitch,
    MMP_UINT8  *fontFilePtr);

/**
 * Generate the specific font in to VRAM
 *
 * @param offset        the offset in font file.
 * @param fontWidth     font width.
 * @param fontHeight    font height.
 * @param fontFilePtr   the pointer of the font in font file.
 * @return the pointer of the font in VRAM.
 *
 */
MMP_UINT8 *
M2D_GeneItalFont(
    MMP_UINT32 fontWidth,
    MMP_UINT32 fontHeight,
    MMP_UINT8  *fontFilePtr);

/**
 * Convert hiByte and lowByte of the big5 code.
 *
 * @param big5Code     Big5 code.
 * @return the converted big5 code.
 *
 */
MMP_UINT16
M2D_ConvertBig5(
    MMP_UINT16 big5Code);

/**
 * Transform higher limit and lowerl limit code to serial number.
 *
 * @param big5Code     Big5 code.
 * @return the serial number of the word.
 *
 */
MMP_UINT16
M2D_TransBig5Ser(
    MMP_UINT16 big5Code);
/**
 * Show text for normal style.
 *
 * @param big5Code     Big5 code.
 * @return the serial number of the word.
 *
 */
void
M2D_ShowText(
    M2D_SURFACE *destDisplay,
    MMP_UINT8   *srcBase,
    MMP_UINT32  fontWidth,
    MMP_UINT32  fontHeight,
    MMP_UINT32  fontPitch,
    MMP_UINT32  startX,
    MMP_UINT32  startY);

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __FONT_H