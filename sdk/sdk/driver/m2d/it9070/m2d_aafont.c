/*
 * Copyright (c) 2004 SMedia technology Corp. All Rights Reserved.
 */
/** @file
 * Font functions.
 *
 * @author Mandy Wu
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
mmpM2dDrawAABmpTtx2(
    MMP_M2D_SURFACE destSurface,
    void            *fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight)
{
    MMP_RESULT  result       = MMP_RESULT_SUCCESS;
    M2D_SURFACE *destDisplay = (M2D_SURFACE *)destSurface;
    MMP_UINT8   *srcBase;
    MMP_UINT32  fontW        = ITH_ALIGN_UP(fontWidth, 2);
    MMP_UINT32  fontH        = fontHeight;
    MMP_UINT32  srcPitch     = (fontW) / 2;
    MMP_UINT32  srcWidth     = srcPitch / 2;
    MMP_UINT32  data         = 0;
    MMP_UINT16  tempId       = 0;
//++ test
    MMP_UINT32  i;
    MMP_UINT32  dataSize     = 0;
//-- test

    srcBase = (MMP_UINT8 *)fontPtr;

//++ test
#if 0
    printf("===================================================\n");
    printf("startX = %d\n startY = %d\n fontWidth = %d\n fontHeight = %d\n",
           startX,
           startY,
           fontWidth,
           fontHeight);

    dataSize = ((srcPitch * fontH + 1) >> 1) << 1;
    printf("srcPitch = %d fontH = %d dataSize = %d\n",
           srcPitch, fontH, dataSize);
    for (i = 0; i < dataSize; ++i)
        printf("0x%x, ", srcBase[i]);
    printf("\n===================================================\n");

#endif
/*
   //swap data
    {
       unsigned char c;
       for(i = 0; i<dataSize; i++)
       {
            c = ((srcBase[i] & 0xf0) >> 4) | ((srcBase[i]  & 0x0f) << 4);
            srcBase[i] = c;
       }

    }*/

//-- test
//hw register
    m2dDrawAABmpTextTtx2(destDisplay,
                         startX,
                         startY,
                         srcBase,
                         fontW,
                         fontH,
                         srcPitch);

    return (result);
}

MMP_RESULT
mmpM2dDrawAABmpTtx3(
    MMP_M2D_SURFACE destSurface,
    void            *fontPtr,
    MMP_UINT        startX,
    MMP_UINT        startY,
    MMP_UINT        fontWidth,
    MMP_UINT        fontHeight)
{
    MMP_RESULT  result       = MMP_RESULT_SUCCESS;
    M2D_SURFACE *destDisplay = (M2D_SURFACE *)destSurface;
    MMP_UINT8   *srcBase;
    MMP_UINT32  fontW        = fontWidth;//((fontWidth+1)>>1)<<1;
    MMP_UINT32  fontH        = fontHeight;
    MMP_UINT32  srcPitch     = (fontW);
    MMP_UINT32  srcWidth     = srcPitch / 2;
    MMP_UINT32  data         = 0;
    MMP_UINT16  tempId       = 0;
//++ test
    MMP_UINT32  i;
    MMP_UINT32  dataSize     = 0;
//-- test

    srcBase = (MMP_UINT8 *)fontPtr;

//++ test
#if 0
    printf("===================================================\n");
    printf("startX = %d\n startY = %d\n fontWidth = %d\n fontHeight = %d\n",
           startX,
           startY,
           fontWidth,
           fontHeight);

    dataSize = ((srcPitch * fontH + 1) >> 1) << 1;
    printf("srcPitch = %d fontH = %d dataSize = %d\n",
           srcPitch, fontH, dataSize);
    for (i = 0; i < dataSize; ++i)
        printf("0x%x, ", srcBase[i]);
    printf("\n===================================================\n");

#endif
/*
   //swap data
    {
       unsigned char c;
       for(i = 0; i<dataSize; i++)
       {
            c = ((srcBase[i] & 0xf0) >> 4) | ((srcBase[i]  & 0x0f) << 4);
            srcBase[i] = c;
       }

    }*/

//-- test
//hw register
    m2dDrawAABmpTextTtx3(destDisplay,
                         startX,
                         startY,
                         srcBase,
                         fontW,
                         fontH,
                         srcPitch);

    return (result);
}