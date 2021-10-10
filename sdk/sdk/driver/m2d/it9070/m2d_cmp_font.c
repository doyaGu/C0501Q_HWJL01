/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Font functions.
 *
 * @author Page Luo
 * @version 0.1
 */
#include <string.h>
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_cmp_font.h"
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
 * Search the specific font in a font file, and write to VRAM
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
    MMP_UINT32 srcPitch,
    MMP_UINT8  *fontPtr)
{
    MMP_UINT8 *newFontPtr;

    newFontPtr = (MMP_UINT8 *)malloc((srcPitch * fontHeight));
    if (newFontPtr != NULL)
    {
        memcpy((MMP_UINT32) newFontPtr,
               (MMP_UINT32) fontPtr,
               srcPitch * fontHeight);
    }// End of if (newFontPtr != NULL)

    return newFontPtr;
} // M2D_GeneFontBytes()

MMP_RESULT
mmpM2dDrawCmpBmpTextTtx2(
    MMP_M2D_SURFACE destSurface,
    MMP_INT         destX,
    MMP_INT         destY,
    void            *pCmpGlyph,
    MMP_UINT        srcWidth,
    MMP_UINT        srcHeight,
    MMP_UINT        srcPitch)
{
    M2D_SURFACE    *destDisplay     = (M2D_SURFACE *)destSurface;
    M2D_COMP_GLYPH *ptCmpGlyph      = (M2D_COMP_GLYPH *)pCmpGlyph;
    MMP_UINT8      *srcBase         = NULL;
    MMP_UINT16     cmdFontMPSTable  = 0;
    MMP_UINT16     cmdFontHuffTable = 0;
    MMP_UINT16     cmdFontDataSize  = 0;
    MMP_UINT16     tempId           = 0;
    MMP_UINT32     data             = 0;
    MMP_UINT32     fontW            = srcWidth;
    MMP_UINT32     fontH            = srcHeight;
    MMP_UINT32     tmpPitch         = srcPitch;
    MMP_UINT32     tmpWidth         = srcPitch >> 1;
    MMP_UINT32     i;

    MMP_UINT16     low              = 0;
    MMP_UINT16     high             = 0;
    MMP_UINT32     test             = 0;

    srcBase = ptCmpGlyph->pCmpGlyphData;//M2D_GeneFontBytes(srcWidth, srcHeight, ptCmpGlyph);

//hw register
//VG not implement

    return MMP_RESULT_SUCCESS;
} // mmpM2dDrawBmpTextTtx2()