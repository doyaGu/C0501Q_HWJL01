/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * create brush object.
 *
 * @author Mandy Wu
 * @version 0.1
 */
#include <string.h>
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_brush.h"
#include "../../include/ite/ith.h"

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
 * Create a logical brush with the specified solid color.
 *
 * @param brushColor    brush color value.
 * @param brushObject   return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dCreateSolidBrush(
    MMP_M2D_COLOR  brushColor,
    MMP_M2D_HANDLE *brushObject)
{
    M2D_BRUSHOBJ *newBrushobj = NULL;

    newBrushobj = M2D_CreateBrush();
    if (newBrushobj == NULL)
        return MMP_RESULT_ERROR;

    newBrushobj->realizedBrush->foregroundColor = brushColor;
    *brushObject                                = (MMP_M2D_HANDLE)newBrushobj;

    return MMP_SUCCESS;
}

/**
 * Create a logical brush with specified hatch pattern and color.
 *
 * @param hatchStyle            hatch style.
 * @param hatchForegroundColor  foreground color of the hatch brush.
 * @param brushObject           return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
#ifndef SHRINK_CODE_SIZE
MMP_RESULT
mmpM2dCreateHatchBrush(
    MMP_M2D_HATCH_STYLE hatchStyle,
    MMP_UINT8           *userDefinedStyle,
    MMP_M2D_COLOR       hatchForegroundColor,
    MMP_M2D_HANDLE      *brushObject)
{
    M2D_BRUSHOBJ *newBrushobj = NULL;
    MMP_UINT32   mask3210     = 0;
    MMP_UINT32   mask7654     = 0;

    newBrushobj = M2D_CreateBrush();
    if (newBrushobj == NULL)
        return MMP_RESULT_ERROR;

    newBrushobj->realizedBrush->foregroundColor = hatchForegroundColor;
    newBrushobj->realizedBrush->flag            = M2D_HATCH_BRUSH;

    switch (hatchStyle)
    {
    case MMP_M2D_HS_HORIZONTAL:
        mask3210 = 0xFF000000;
        mask7654 = 0xFF000000;
        break;

    case MMP_M2D_HS_VERTICAL:
        mask3210 = 0x08080808;
        mask7654 = 0x08080808;
        break;

    case MMP_M2D_HS_FDIAGONAL:
        mask3210 = 0x10204080;
        mask7654 = 0x01020408;
        break;

    case MMP_M2D_HS_BDIAGONAL:
        mask3210 = 0x08040201;
        mask7654 = 0x80402010;
        break;

    case MMP_M2D_HS_CROSS:
        mask3210 = 0xFF080808;
        mask7654 = 0x08080808;
        break;

    case MMP_M2D_HS_DIAGCROSS:
        mask3210 = 0x18244281;
        mask7654 = 0x81422418;
        break;

    case MMP_M2D_HS_CUBE:
        mask3210 = 0x0;
        mask7654 = 0x07070700;
        break;

    case MMP_M2D_HS_USER_DEFINED:
    #if defined(__FREERTOS__)
        {
            mask3210 = (userDefinedStyle[7] << 24) + (userDefinedStyle[6] << 16) + (userDefinedStyle[5] << 8) + userDefinedStyle[4];
            mask7654 = (userDefinedStyle[3] << 24) + (userDefinedStyle[2] << 16) + (userDefinedStyle[1] << 8) + userDefinedStyle[0];
        }
    #else
        {
            mask3210 = *((MMP_UINT32 *) (userDefinedStyle + 4));
            mask7654 = *((MMP_UINT32 *) userDefinedStyle);
        }
    #endif
        break;
    }

    newBrushobj->realizedBrush->mask3210 = mask3210;
    newBrushobj->realizedBrush->mask7654 = mask7654;
    *brushObject                         = (MMP_M2D_HANDLE)newBrushobj;

    return MMP_SUCCESS;
}

/**
 * Create a logical brush with specified color image pattern.
 *
 * @param imagePattern    image handle to be used to creat the logical brush.
 * @param brushObject     return the created object handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dCreatePatternBrush(
    MMP_UINT       pitch,
    const void     *imageDataPtr,
    MMP_M2D_HANDLE *brushObject)
{
    M2D_BRUSHOBJ *newBrushobj     = NULL;
    MMP_UINT8    *patBrushPtr     = NULL;
    MMP_UINT8    *tmpPatBrushPtr  = NULL;
    MMP_UINT8    *tmpImageDataPtr = (MMP_UINT8 *)imageDataPtr;
    MMP_UINT32   brushSize        = 0;
    MMP_UINT     i                = 0;

    brushSize   = M2D_02_BYTES_ALIGN(16 * 8);
    patBrushPtr = (MMP_UINT8 *)itpVmemAlloc(brushSize);
    if (patBrushPtr == NULL)
        return MMP_RESULT_ERROR;

    newBrushobj = M2D_CreateBrush();
    if (newBrushobj == NULL)
    {
        free(patBrushPtr);
        return MMP_RESULT_ERROR;
    }

    newBrushobj->realizedBrush->pattern = patBrushPtr;
    newBrushobj->realizedBrush->flag    = M2D_PAT_BRUSH;

    // generate a 8x8 pattern brush
    tmpPatBrushPtr                      = patBrushPtr = ithMapVram((MMP_UINT32)patBrushPtr, brushSize, ITH_VRAM_WRITE);
    for (i = 0; i < 8; i++)
    {
        memcpy((void *)tmpPatBrushPtr, (void *)tmpImageDataPtr, 16);
        tmpPatBrushPtr  += 16;
        tmpImageDataPtr += pitch;
    }
    ithFlushDCacheRange(patBrushPtr, brushSize);
    ithFlushMemBuffer();
    ithUnmapVram(patBrushPtr, brushSize);

    *brushObject = (MMP_M2D_HANDLE)newBrushobj;
    return MMP_SUCCESS;
}

/**
 * Get brush color of the specified surface.
 *
 * @param surface       surface handle.
 * @param fgColor       return foreground color.
 * @param bgColor       return background color.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dSetBrushColor().
 */
MMP_RESULT
mmpM2dGetBrushColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   *fgColor,
    MMP_M2D_COLOR   *bgColor)
{
    M2D_SURFACE *m2dSurf = (M2D_SURFACE *)surface;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(surface))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    *fgColor = m2dSurf->brush->realizedBrush->foregroundColor;
    *bgColor = m2dSurf->brush->realizedBrush->backgroundColor;

    return MMP_RESULT_SUCCESS;
}
#endif

#if !defined(SHRINK_CODE_SIZE) || defined(USB_LOGO_TEST)
/**
 * Set brush color to the specified position.
 *
 * @param surface       surface.
 * @param fgColor       foreground color.
 * @param bgColor       background color.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dGetBrushColor().
 */
MMP_RESULT
mmpM2dSetBrushColor(
    MMP_M2D_SURFACE surface,
    MMP_M2D_COLOR   fgColor,
    MMP_M2D_COLOR   bgColor)
{
    M2D_SURFACE *m2dSurf = (M2D_SURFACE *)surface;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(surface))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    m2dSurf->brush->realizedBrush->foregroundColor = fgColor;
    m2dSurf->brush->realizedBrush->backgroundColor = bgColor;

    return MMP_RESULT_SUCCESS;
}
#endif

/**
 * Create an empty memory space for brush.
 *
 * @param newBrushobj    create a brush object.
 *
 */
M2D_BRUSHOBJ *
M2D_CreateBrush(void)
{
    M2D_BRUSHOBJ *newBrushobj = NULL;

    newBrushobj = (M2D_BRUSHOBJ *)malloc(sizeof(M2D_BRUSHOBJ));
    if (newBrushobj == NULL)
        return NULL;

    newBrushobj->brushStructSize = sizeof(M2D_BRUSHOBJ);
    newBrushobj->realizedBrush   = (M2D_BRUSH *)malloc(sizeof(M2D_BRUSH));
    if (newBrushobj->realizedBrush == NULL)
    {
        free(newBrushobj);
        return NULL;
    }

    newBrushobj->realizedBrush->flag            = M2D_SOLID_BRUSH;
    newBrushobj->realizedBrush->foregroundColor = 0;
    newBrushobj->realizedBrush->backgroundColor = 0xFFFFFF;
    newBrushobj->realizedBrush->pattern         = NULL;

    return newBrushobj;
} // End of M2D_CreateBrush

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
    M2D_SURFACE *display)
{
    //For hatch brush
    MMP_UINT16 i, j, k, l, m;
    MMP_UINT8  temp2Mask[8];  //save the mask which is handled according Y coordinate
    MMP_UINT8  temp3Mask[8];  //save the mask which is handled according X coordinate

    //For pattern brush
    MMP_UINT8  *patScanPtr   = NULL;
    MMP_UINT8  *patScanPtr2  = NULL;
    MMP_UINT8  *tempScanPtr  = NULL;
    MMP_UINT8  *tempScanPtr2 = NULL;
    MMP_UINT8  *tempScanPtr3 = NULL;
    MMP_UINT8  *newPatPtr    = NULL;

    MMP_UINT32 tempMask3210  = display->brush->realizedBrush->mask3210;
    MMP_UINT32 tempMask7654  = display->brush->realizedBrush->mask7654;
    MMP_INT32  brushX        = display->brushOriginPos.x;
    MMP_INT32  brushY        = display->brushOriginPos.y;

    //save the origin scan line 0~8 of the brush
    MMP_UINT32 tempMask[8]   =
    {
        0, 0, 0, 0, 0, 0, 0, 0,
    };

    //brushY 0~8
    MMP_UINT8 mask[8] =
    {
        0xFF, 0x80, 0xC0, 0xE0,
        0xF0, 0xF8, 0xFC, 0xFE,
    };

    tempMask[0] = (tempMask3210 & 0x000000FF);
    tempMask[1] = (tempMask3210 & 0x0000FF00) >> 8;
    tempMask[2] = (tempMask3210 & 0x00FF0000) >> 16;
    tempMask[3] = (tempMask3210 & 0xFF000000) >> 24;
    tempMask[4] = (tempMask7654 & 0x000000FF);
    tempMask[5] = (tempMask7654 & 0x0000FF00) >> 8;
    tempMask[6] = (tempMask7654 & 0x00FF0000) >> 16;
    tempMask[7] = (tempMask7654 & 0xFF000000) >> 24;

    // revise the coordinate of brush, cannot < 0
    if (brushX < 0)
    {
        brushX = 8 - (8 + brushX);
    }
    if (brushY < 0)
    {
        brushY = 8 - (8 + brushY);
    }
    //the re-set position of brush is restricted to 0~8
    if (brushX > 8 || brushY > 8)
    {
        return MMP_FALSE;
    }

    // Hatch brush
    if (display->brush->realizedBrush->flag == M2D_HATCH_BRUSH)
    {
        if (brushY > 0 && brushY < 8)
        {
            // Y coordinate: re-deploy the column line of the brush
            for (i = 0; i < 8; i++)
            {
                temp2Mask[i] = (MMP_UINT8)(((MMP_UINT8)tempMask[i] & mask[brushY]) >> (8 - brushY)) | (MMP_UINT8)((MMP_UINT8)tempMask[i] << brushY);
            }
        }

        if (brushX > 0 && brushX < 8)
        {
            // X coordinate: re-deploy the scan line of the brush
            for (j = 0; j < 8; j++)
            {
                if (j >= (8 - brushX))
                {
                    temp3Mask[j] = temp2Mask[j - (8 - brushX)];
                }
                else
                {
                    temp3Mask[j] = temp2Mask[j + brushX];
                }
            }
        }

        tempMask3210 = (temp3Mask[3] << 24) | (temp3Mask[2] << 16) | (temp3Mask[1] << 8) | (temp3Mask[0]);
        tempMask7654 = (temp3Mask[7] << 24) | (temp3Mask[6] << 16) | (temp3Mask[5] << 8) | (temp3Mask[4]);

        display->brush->realizedBrush->mask3210 = tempMask3210;
        display->brush->realizedBrush->mask7654 = tempMask7654;
    }
    // Pattern brush
    else if (display->brush->realizedBrush->flag == M2D_PAT_BRUSH)
    {
        if (display->brush->realizedBrush->pattern != NULL)
        {
            if ((brushY > 0 && brushY < 8) || (brushX > 0 && brushX))
            {
                patScanPtr  = (MMP_UINT8 *)((MMP_UINT32)(display->brush->realizedBrush->pattern)
                                          + (MMP_UINT32)m2d_vramBasePtr);
                patScanPtr2 = patScanPtr;
                tempScanPtr = (MMP_UINT8 *)malloc(512);
                if (tempScanPtr == NULL)
                {
                    return MMP_FALSE;
                }
                tempScanPtr2 = tempScanPtr;
                tempScanPtr3 = tempScanPtr;

                //Read from VRAM and write to S.M.
                for (k = 0; k < 16; k++)
                {
                    for (l = 0; l < 2; l++)
                    {
                        memcpy((MMP_UINT32) tempScanPtr,
                               (MMP_UINT32) patScanPtr,
                               16);
                        tempScanPtr += 16;
                    }

                    if (k == 7)
                    {
                        patScanPtr = (MMP_UINT8 *)((MMP_UINT32)(display->brush->realizedBrush->pattern)
                                                 + (MMP_UINT32)m2d_vramBasePtr);
                    }
                    else
                    {
                        patScanPtr += 16;
                    }
                }

                //Generate new pattern brush, read from S.M. and write to VRAM
                newPatPtr = (MMP_UINT8 *)malloc(128);

                if (newPatPtr != NULL)
                {
                    display->brush->realizedBrush->pattern = (MMP_UINT8 *)((MMP_UINT32) newPatPtr
                                                                         - (MMP_UINT32) m2d_vramBasePtr);
                    tempScanPtr2                           = tempScanPtr2
                                                             + (brushX * 32)
                                                             + (brushY * 2);
                    for (m = 0; m < 8; m++)
                    {
                        memcpy((MMP_UINT32) newPatPtr,
                               (MMP_UINT32) tempScanPtr2,
                               16);
                        newPatPtr    += 16;
                        tempScanPtr2 += 32;
                    }

                    //release original patScanPtr
                    free(patScanPtr2);
                }

                // Release the temp brush
                free(tempScanPtr3);
            }
        }
    }

    return MMP_TRUE;
}
#endif