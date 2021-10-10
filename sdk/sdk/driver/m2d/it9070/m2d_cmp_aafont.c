/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Font functions.
 *
 * @author Page Luo
 * @version 0.1
 */
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_font.h"
#include "m2d/m2d_rotate.h"
#include "m2d/m2d_aafont.h"

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
MMP_RESULT
mmpM2dDrawCmpAABmpTtx2(
    MMP_M2D_SURFACE destSurface,
    void            *pCmpGlyph,//MMP_UINT8*  fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight)
{
    MMP_RESULT     result           = MMP_RESULT_SUCCESS;
    M2D_SURFACE    *destDisplay     = (M2D_SURFACE *)destSurface;
    M2D_COMP_GLYPH *ptCmpGlyph      = (M2D_COMP_GLYPH *)pCmpGlyph;
    MMP_UINT16     cmdFontMPSTable  = 0;
    MMP_UINT16     cmdFontHuffTable = 0;
    MMP_UINT16     cmdFontDataSize  = 0;
    MMP_UINT16     cmdFontSetting   = 0;
    MMP_UINT8      *srcBase;
    MMP_UINT32     fontW            = ((fontWidth + 1) >> 1) << 1;
    MMP_UINT32     fontH            = fontHeight;
    MMP_UINT32     srcPitch         = (fontW) / 2;
    MMP_UINT32     srcWidth         = srcPitch / 2;
    MMP_UINT32     data             = 0;
    MMP_UINT16     tempId           = 0;
    MMP_UINT32     compBpp          = 4 - ptCmpGlyph->compBPP;
    MMP_UINT32     i;

    srcBase = ptCmpGlyph->pCmpGlyphData;//fontPtr;

//hw register
//VG not implement

    return (result);
}