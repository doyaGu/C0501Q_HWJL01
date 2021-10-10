/*
 * Copyright (c) 2013 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Enable and disable the 2D driver.
 *
 * @author James Lin
 * @version 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "m2d/m2d_dispctxt.h"
#include "ite/ite_m2d.h"
#include "m2d/m2d_engine.h"
#include "ite/mmp_types.h"

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
MMP_UINT8     *m2d_vramBasePtr = NULL;
MMP_INT       inited           = 0;
MMP_BOOL      m2d_trueColor    = MMP_FALSE;

ITEM2DSurface *vgSurface;

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
//                              Function Definition
//=============================================================================

/**
 * Initial 2D engine.
 *
 * @author Erica Chang
 */
static ITEM2Duint iteM2dGetFormatLogBits(M2DVGImageFormat vg)
{
    ITEM2Duint logBits = 5;
    switch (vg & 0x1F)
    {
    case 3:     /* VG_sRGB_565 */
    case 4:     /* VG_sRGBA_5551 */
    case 5:     /* VG_sRGBA_4444 */
        logBits = 4;
        break;
    case 6:     /* VG_sL_8 */
    case 10:    /* VG_lL_8 */
    case 11:    /* VG_A_8 */
        logBits = 3;
        break;
    case 12:     /* VG_BW_1 */
    case 13:     /* VG_A_1 */
        logBits = 0;
        break;
    case 14:     /* VG_A_4 */
        logBits = 2;
        break;
    case 15:     /* VG_RGBA_16 */
        logBits = 6;
        break;
    default:
        logBits = 5;
        break;
    }
    return logBits;
}

ITEM2DImage *
iteM2dCreateImage(
    M2DVGImageFormat format,
    ITEM2Dint        width,
    ITEM2Dint        height,
    ITEM2Duint       allowedQuality,
    ITEM2Dboolean    ownsData,
    ITEM2Dboolean    forceAlign64bit)
{
    ITEM2DImage *i      = NULL;
    ITEM2Duint  logBits = iteM2dGetFormatLogBits(format);

    /* Create new image object */
    i                 = (ITEM2DImage *)malloc(sizeof(ITEM2DImage));
    i->width          = width;
    i->height         = height;
    i->vgformat       = format;
    i->allowedQuality = allowedQuality;
    i->dataOwner      = ownsData;
    if (forceAlign64bit == M2DVG_TRUE)
    {
        i->pitch = (((((width << logBits) + 7) >> 3) + 7) >> 3) << 3;
    }
    else
    {
        i->pitch = ((width << logBits) + 7) >> 3;
    }
    //i->pitch          = (forceAlign64bit == M2DVG_TRUE) ? ITH_ALIGN_UP(width, 8) : width;

    /* Allocate data memory if needed */
    if (ownsData == M2DVG_TRUE)
    {
        ITEM2Dint        new_width;
        ITEM2Dint        new_height;
        ITEM2Dint        new_pitch;
        
        if (width > height)
        {
    	    new_width = width;
    	    new_height = width;
        } else {
    	    new_width = height;
    	    new_height = height;
        }
    	
        if (forceAlign64bit == M2DVG_TRUE)
        {
            new_pitch = (((((new_width << logBits) + 7) >> 3) + 7) >> 3) << 3;
        }
        else
        {
            new_pitch = ((new_width << logBits) + 7) >> 3;
        }
            	    
        i->data = (ITEM2Duint8 *)memalign(8, new_pitch * new_height);
        
        if (i->data == NULL)
        {
            free(i);
        }
    }

    return (ITEM2DImage *)i;
}

void
iteM2dDestroyImage(
    ITEM2DImage *pImage)
{
    if (pImage->dataOwner == M2DVG_TRUE)
    {
        free(pImage->data);
        pImage->data = NULL;
    }

    free(pImage);
}

ITEM2DSurface *
iteM2dContextAllocSurface(
    M2DVGImageFormat surfaceFormat,
    int              windowWidth,
    int              windowHeight,
    ITEM2Dboolean    allocMask)
{
    ITEM2DSurface *pSurface     = NULL;
    ITEM2Duint    imageQualilty = M2DVG_IMAGE_QUALITY_NONANTIALIASED | M2DVG_IMAGE_QUALITY_FASTER | M2DVG_IMAGE_QUALITY_BETTER;

    pSurface = (ITEM2DSurface *)malloc(sizeof(ITEM2DSurface));
    if (pSurface)
    {
        ITEM2DColor color = {0};

        memset(pSurface, 0x00, sizeof(ITEM2DSurface));

        /* Allocate/Set mask image */
        if (allocMask)
        {
            pSurface->maskImage = (ITEM2DImage *)iteM2dCreateImage(M2DVG_A_8, windowWidth, windowHeight, imageQualilty, M2DVG_TRUE, M2DVG_FALSE);
            if (pSurface->maskImage == NULL)
            {
                printf("Allocate/Set mask image failed\n");
            }

            CSET(color, 255, 255, 255, 255);
            //iteSetImage(ITE_FALSE, pSurface->maskImage, 0, 0, windowWidth, windowHeight, color);
            //m2dSetImage
        }

        /* Create coverage image for hardware */
        pSurface->coverageImageA = iteM2dCreateImage(M2DVG_A_8, windowWidth, windowHeight, 0, M2DVG_TRUE, M2DVG_TRUE);
        pSurface->coverageImageB = iteM2dCreateImage(M2DVG_A_8, windowWidth, windowHeight, 0, M2DVG_TRUE, M2DVG_TRUE);
        if (pSurface->coverageImageA == NULL || pSurface->coverageImageB == NULL)
        {
            printf("Allocate/Set coverage image failed\n");
        }

        pSurface->coverageIndex = 0;

        /* Create valid image for hardware */
        pSurface->validImageA   = iteM2dCreateImage(M2DVG_A_1, windowWidth, windowHeight, 0, M2DVG_TRUE, M2DVG_TRUE);
        pSurface->validImageB   = iteM2dCreateImage(M2DVG_A_1, windowWidth, windowHeight, 0, M2DVG_TRUE, M2DVG_TRUE);
        if (pSurface->validImageA == NULL || pSurface->validImageB == NULL)
        {
            printf("Allocate/Set valid image failed\n");
        }
    }

    return pSurface;
}

void
iteM2dContextFreeSurface(
    ITEM2DSurface *pSurface)
{
    if (pSurface->maskImage)
    {
        iteM2dDestroyImage(pSurface->maskImage);
        pSurface->maskImage = NULL;
    }

    if (pSurface->coverageImageA)
    {
        iteM2dDestroyImage(pSurface->coverageImageA);
        pSurface->coverageImageA = NULL;
    }

    if (pSurface->coverageImageB)
    {
        iteM2dDestroyImage(pSurface->coverageImageB);
        pSurface->coverageImageA = NULL;
    }

    if (pSurface->validImageA)
    {
        iteM2dDestroyImage(pSurface->validImageA);
        pSurface->validImageA = NULL;
    }

    if (pSurface->validImageB)
    {
        iteM2dDestroyImage(pSurface->validImageB);
        pSurface->validImageB = NULL;
    }

    free(pSurface);
}

MMP_RESULT
mmpM2dInitialize(
    void)
{
    MMP_UINT16 lcdWidth  = CFG_LCD_WIDTH;
    MMP_UINT16 lcdPitch  = CFG_LCD_HEIGHT;
    MMP_UINT16 lcdHeight = CFG_LCD_PITCH;

    if (inited < 0)
    {
        inited = 0;
    }

    inited += 1;
    if (inited > 1)
    {
        return MMP_RESULT_SUCCESS;
    }

    printf("%s[%d]\n", __FUNCTION__, __LINE__);
    m2d_trueColor = MMP_FALSE;

    if (M2D_EnableLcdSurface() == MMP_FALSE)
    {
        return MMP_RESULT_ERROR;
    }

    vgSurface = iteM2dContextAllocSurface(
        M2DVG_sRGB_565,
        lcdWidth,
        lcdHeight,
        MMP_TRUE);

    //inited = MMP_TRUE;

    return MMP_RESULT_SUCCESS;
} // mmpM2dInitialize()

/**
 * Terminate 2D engine.
 *
 * @author James Lin
 */
MMP_RESULT
mmpM2dTerminate(
    void)
{
    inited -= 1;

    if (!inited)
    {
        printf("%s[%d]\n", __FUNCTION__, __LINE__);

        M2D_DisableLcdSurface();

        // Free surface
        iteM2dContextFreeSurface(vgSurface);
        vgSurface = NULL;
    } // End of if(!inited)

    return MMP_RESULT_SUCCESS;
} // mmpM2dTerminate()

MMP_RESULT
mmpM2dSurfaceInitialize(
    void)
{
    MMP_UINT16 lcdWidth  = 0;
    MMP_UINT16 lcdPitch  = 0;
    MMP_UINT16 lcdHeight = 0;
    lcdWidth  = (MMP_UINT16) CFG_LCD_WIDTH;
    lcdPitch  = (MMP_UINT16) CFG_LCD_PITCH;
    lcdHeight = (MMP_UINT16) CFG_LCD_HEIGHT;    

    if (M2D_EnableLcdSurface() == MMP_FALSE)
    {
        return MMP_RESULT_ERROR;
    }

    vgSurface = iteM2dContextAllocSurface(
        M2DVG_sRGB_565,      
        lcdWidth,
        lcdHeight,
        MMP_FALSE);

    return MMP_RESULT_SUCCESS;
}