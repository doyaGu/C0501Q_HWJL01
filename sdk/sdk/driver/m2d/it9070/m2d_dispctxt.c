/*
 * Copyright (c) 2011 ITE Corp. All Rights Reserved.
 */
/** @file
 * Used as implementatoin template file.
 *
 * @author James Lin
 * @version 1.0
 */
#include <stdio.h>
#include <string.h>
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_dispctxt.h"
#include "m2d/m2d_engine.h"
#include "m2d/m2d_brush.h"
#include "m2d/m2d_pen.h"
#include "m2d/m2d_font.h"
#include "../../include/ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
MMP_UINT8 M2D_BPP[10] =
{
    M2D_32BPP,
    M2D_16BPP,
    M2D_16BPP,
    M2D_16BPP,
    M2D_16BPP,
    M2D_16BPP,
    M2D_16BPP,
    M2D_16BPP,
    M2D_16BPP,
    M2D_12BPP
};

typedef enum _LCD_BUFFER_INDEX
{
    LCD_BUFFER_A = 0,
    LCD_BUFFER_B,
    LCD_BUFFER_C
} LCD_BUFFER_INDEX;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================
extern MMP_UINT8 *m2d_vramBasePtr;

//=============================================================================
//                              Static Data Definition
//=============================================================================
static M2D_SURFACE  defaultDisplay = { 0 };
static M2D_BRUSHOBJ defaultBrushObj = { 0 };
static M2D_BRUSH    defaultBrush = { 0 };
static M2D_PENOBJ   defaultPenObj = { 0 };
static M2D_FONTOBJ  defaultFontObj = { 0 };

static int          bitbltId3_framecnt = 0;

#define MAX_QUEUE_CNT 5
//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
//                              Function Definition
//=============================================================================
/**
 * Create a surface in memory that is compatible with the existing
 * display.
 *
 * @param targetDisplay    the existing specified surface. If NULL, the
 *                         target surface will be the default screen display.
 * @param display          return the created surface.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpDeleteSurface().
 */
MMP_RESULT
mmpM2dCreateSurface(
    MMP_M2D_SURFACE targetDisplay,
    MMP_M2D_SURFACE *display)
{
    M2D_SURFACE *newDisplay  = NULL;
    MMP_UINT32  displaySize  = 0;
    MMP_UINT8   *baseScanPtr = NULL;

    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(targetDisplay))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    newDisplay = (M2D_SURFACE *)malloc(sizeof(M2D_SURFACE));
    if (newDisplay == NULL)
        return MMP_RESULT_ERROR;

    *newDisplay = *(M2D_SURFACE *)targetDisplay;

    displaySize = M2D_02_BYTES_ALIGN(newDisplay->pitch * newDisplay->height);
    baseScanPtr = (MMP_UINT8 *)malloc(displaySize);
    if (baseScanPtr == NULL)
    {
        M2D_DeleteSurface(newDisplay);
        return MMP_RESULT_ERROR;
    }

    newDisplay->baseScanPtr    = baseScanPtr;
    newDisplay->baseAddrOffset = (MMP_UINT32)(newDisplay->baseScanPtr - m2d_vramBasePtr);

    *display                   = (MMP_M2D_SURFACE)newDisplay;

    return MMP_RESULT_SUCCESS;
} // End of mmpCreateSurface

/**
 * Create a virtual display context in system memory from the existing image
 * data in system memory. A virtual display context can't be used as
 * destination.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageDelta     image width stride (row pitch), in bytes.
 * @param imageFormat    image format.
 * @param imageDataBits  point to the first pixel data of the image.
 * @param display        return the created display handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpDeleteSurface().
 */
MMP_RESULT
mmpM2dCreateVirtualSurface(
    MMP_UINT             imageWidth,
    MMP_UINT             imageHeight,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    MMP_UINT8            *imageDataBits,
    MMP_M2D_SURFACE      *display)
{
    MMP_UINT8   *baseScanPtr = NULL;
    M2D_SURFACE *newDisplay  = NULL;

    MMP_UINT    imageDelta   = (M2D_BPP[imageFormat] * imageWidth) / 8;

    if ((imageWidth == 0)
        || (imageHeight == 0))
    {
        return MMP_RESULT_ERROR;
    }

    baseScanPtr = imageDataBits;
    if (baseScanPtr == NULL)
    {
        return MMP_RESULT_ERROR;
    }

    newDisplay = M2D_CreateSurface(imageWidth,
                                   imageHeight,
                                   imageDelta,
                                   imageFormat,
                                   (const void *)baseScanPtr);
    if (newDisplay == NULL)
    {
        return MMP_RESULT_ERROR;
    }

    *display                     = (MMP_M2D_SURFACE)newDisplay;

    newDisplay->WCE_isCreateByOS = MMP_TRUE; // To detemine it's allocated by driver

    return MMP_RESULT_SUCCESS;
} // End of mmpCreateVirtualDisplay()

//#ifndef SHRINK_CODE_SIZE
/**
 * Create a display context in memory from the existing image data in memory.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageDelta     image width stride (row pitch), in bytes.
 * @param imageFormat    image format.
 * @param imageDataBits  point to the first pixel data of the image.
 * @param display        return the created display handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpDeleteSurface().
 */
MMP_RESULT
mmpM2dCreateSurfaceFromImage(
    MMP_UINT             imageWidth,
    MMP_UINT             imageHeight,
    MMP_UINT             imageDelta,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    const void           *imageDataBits,
    MMP_M2D_SURFACE      *display)
{
    MMP_UINT32  displaySize = 0;

    M2D_SURFACE *newDisplay = NULL;

    if ((imageWidth == 0)
        || (imageHeight == 0)
        || (imageWidth > imageDelta)
        || (imageDataBits == NULL))
    {
        return MMP_RESULT_ERROR;
    }

    displaySize = M2D_02_BYTES_ALIGN(imageDelta * imageHeight);
    newDisplay  = M2D_CreateSurface(imageWidth,
                                    imageHeight,
                                    imageDelta,
                                    imageFormat,
                                    (const void *)imageDataBits);
    if (newDisplay == NULL)
    {
        return MMP_RESULT_ERROR;
    }

    *display = (MMP_M2D_SURFACE)newDisplay;

    return MMP_RESULT_SUCCESS;
} // End of mmpCreateSurfaceFromImage()

//#endif

/**
 * Create a display context in video memory with no image data.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageDelta     image width stride (row pitch), in bytes.
 * @param imageFormat    image format.
 * @param display        return the created display handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpDeleteSurface().
 */
MMP_RESULT
mmpM2dCreateNullSurface(
    MMP_UINT             imageWidth,
    MMP_UINT             imageHeight,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    MMP_M2D_SURFACE      *display)
{
    MMP_UINT32  displaySize  = 0;
    MMP_UINT8   *baseScanPtr = NULL;
    M2D_SURFACE *newDisplay  = NULL;

    MMP_UINT    imageDelta   = M2D_PITCH_ALIGN(M2D_BPP[imageFormat] * imageWidth);

    if ((imageWidth == 0)
        || (imageHeight == 0))
    {
        return MMP_RESULT_ERROR;
    }

    displaySize = M2D_02_BYTES_ALIGN(imageDelta * imageHeight);
    baseScanPtr = (MMP_UINT8 *)malloc(displaySize);
    if (baseScanPtr == NULL)
    {
        return MMP_RESULT_ERROR;
    }

    newDisplay = M2D_CreateSurface(imageWidth,
                                   imageHeight,
                                   imageDelta,
                                   imageFormat,
                                   (const void *)baseScanPtr);
    if (newDisplay == NULL)
    {
        free(baseScanPtr);
        return MMP_RESULT_ERROR;
    }

    *display                     = (MMP_M2D_SURFACE)newDisplay;
    newDisplay->WCE_isCreateByOS = MMP_TRUE; // To detemine it's allocated by driver

    return MMP_RESULT_SUCCESS;
} // End of mmpCreateSurfaceFromImage()

/**
 * Create a null surface through assigned scan buffer address.
 *
 * @param imageWidth     image width, in pixels.
 * @param imageHeight    image height, in pixels.
 * @param imageFormat    image format.
 * @param display        return the created display handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpDeleteSurface().
 */
MMP_RESULT
mmpM2dCreateNullSurfaceByAddr(
    MMP_UINT             imageWidth,
    MMP_UINT             imageHeight,
    MMP_M2D_IMAGE_FORMAT imageFormat,
    MMP_UINT8            *pBuffer,
    MMP_INT32            bufferSize,
    MMP_M2D_SURFACE      *display)
{
    MMP_INT32   displaySize = 0;
    M2D_SURFACE *newDisplay = NULL;

    MMP_UINT    imageDelta  = M2D_PITCH_ALIGN(M2D_BPP[imageFormat] * imageWidth);

    if ((imageWidth == 0)
        || (imageHeight == 0))
    {
        return MMP_RESULT_ERROR;
    }

    displaySize = M2D_02_BYTES_ALIGN(imageDelta * imageHeight);
    if (displaySize > bufferSize)
        return MMP_RESULT_ERROR;

    newDisplay = M2D_CreateSurface(imageWidth,
                                   imageHeight,
                                   imageDelta,
                                   imageFormat,
                                   (const void *)pBuffer);
    if (newDisplay == NULL)
    {
        return MMP_RESULT_ERROR;
    }

    *display = (MMP_M2D_SURFACE)newDisplay;
    return MMP_RESULT_SUCCESS;
} // End of mmpCreateSurfaceFromImage()

/**
 * Delete a logical display context, and frees all system resources accociated
 * with the display context.
 *
 * @param object    the display handle that is being deleted.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpCreateSurface(), mmpCreateSurfaceFromImage(),mmpCreateVirtualSurface().
 */
MMP_RESULT
mmpM2dDeleteSurface(
    MMP_M2D_SURFACE display)
{
    MMP_UINT8  *deletedScanPtr = NULL;
    MMP_UINT32 tempId          = 0;
    MMP_UINT32 bitbltId2       = 0;
    MMP_UINT32 currObjID       = 0;
    MMP_UINT32 objId0          = 0;
    MMP_UINT32 objId1          = 0;
    MMP_UINT32 timeout         = 0;

    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    deletedScanPtr = ((M2D_SURFACE *)display)->baseScanPtr;

    //LCD A cannot be deleted
    if (deletedScanPtr == (MMP_UINT8 *)ithLcdGetBaseAddrA())
    {
        return MMP_RESULT_ERROR;
    }

    timeout   = 0;
    // Make sure the HW operation (use the surface) has finished.
    bitbltId2 = ((M2D_SURFACE *)display)->bitbltId2;
    do
    {
        currObjID = ithReadRegA(ITEM2D_VG_AHB_REG_BASE + ITEM2D_VG_REG_BID2_BASE);
        if (bitbltId2 > currObjID)
        {
            if ((bitbltId2 - currObjID) > INVALID_OBJECT_ID / 2)
                break;
        }

        timeout++;
        if ((timeout % 1000) == 0)
            usleep(1000);

        if (timeout == 300000)
            printf("wait id timeout!\n");
    } while (currObjID < bitbltId2 && timeout < 30000);

    if (deletedScanPtr != NULL)
    {
        //LCD B & LCD C can be deleted only their structure
        if (deletedScanPtr != (MMP_UINT8 *)ithLcdGetBaseAddrB()
            && deletedScanPtr != (MMP_UINT8 *)ithLcdGetBaseAddrC())
        {
            // Release the memory space of the display
            if (((M2D_SURFACE *)display)->WCE_isCreateByOS)  // It's allocated by driver
                free(deletedScanPtr);
        }
    }

    // Delete display structure
    if (M2D_DeleteSurface((M2D_SURFACE *)display) == MMP_FALSE)
    {
        return MMP_RESULT_ERROR;
    }

    return MMP_RESULT_SUCCESS;
} // End of mmpDeleteSurface()

MMP_RESULT
mmpM2dDeleteVirtualSurface(
    MMP_M2D_SURFACE display)
{
    MMP_UINT8  *deletedScanPtr = NULL;
    MMP_UINT32 tempId          = 0;
    MMP_UINT32 bitbltId2       = 0;
    MMP_UINT32 currObjID       = 0;
    MMP_UINT32 objId0          = 0;
    MMP_UINT32 objId1          = 0;
    MMP_UINT32 timeout         = 0;

    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    deletedScanPtr = ((M2D_SURFACE *)display)->baseScanPtr;
    //LCD A cannot be deleted
    if (deletedScanPtr == (MMP_UINT8 *)ithLcdGetBaseAddrA())
    {
        return MMP_RESULT_ERROR;
    }

    timeout   = 0;
    // Make sure the HW operation (use the surface) has finished.
    bitbltId2 = ((M2D_SURFACE *)display)->bitbltId2;
    do
    {
        currObjID = ithReadRegA(ITEM2D_VG_AHB_REG_BASE + ITEM2D_VG_REG_BID2_BASE);
        if (bitbltId2 > currObjID)
        {
            if ((bitbltId2 - currObjID) > INVALID_OBJECT_ID / 2)
                break;
        }

        timeout++;
        if ((timeout % 1000) == 0)
            usleep(1000);

        if (timeout == 300000)
            printf("wait id timeout!\n");
    } while (currObjID < bitbltId2 && timeout < 300000);

    if (deletedScanPtr != NULL)
    {
        //LCD B & LCD C can be deleted only their structure
        if (deletedScanPtr != (MMP_UINT8 *)ithLcdGetBaseAddrB()
            && deletedScanPtr != (MMP_UINT8 *)ithLcdGetBaseAddrC())
        {
            // Release the memory space of the display
            //if (((M2D_SURFACE*)display)->WCE_isCreateByOS)  // It's allocated by driver
            //memoryStatus = MEM_Release(deletedScanPtr);
        }
    }

    // Delete display structure
    if (M2D_DeleteSurface((M2D_SURFACE *)display) == MMP_FALSE)
    {
        return MMP_RESULT_ERROR;
    }

    return MMP_RESULT_SUCCESS;
} // End of mmpDeleteSurface()

/**
 * Get the display context of the LCD screen.
 *
 * @param   return the screen display handle.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @author Erica Chang
 */
MMP_RESULT
mmpM2dGetScreenSurface(
    MMP_M2D_SURFACE *display)
{
    if (defaultDisplay.dispalyStructSize == 0)
    {
        if (M2D_EnableLcdSurface() == MMP_FALSE)
        {
            return MMP_RESULT_ERROR;
        }
    }

    *display = (MMP_M2D_SURFACE)&defaultDisplay;
    return MMP_RESULT_SUCCESS;
} // End of mmpM2dGetScreenSurface()

/**
 * Create the display context of the LCD_A, LCD_B, or LCD_C.
 *
 * @param   display      return the screen display handle.
 * @param   displayType  fill LCDB or LCDC.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @author Mandy Wu
 */
MMP_RESULT
mmpM2dCreateScreenSurface(
    MMP_M2D_SURFACE  *display,
    MMP_M2D_LCD_TYPE LCDType)
{
    MMP_UINT8   *lcdScanPtr = NULL;
    M2D_SURFACE *lcdDisplay;

    MMP_UINT16  lcdWidth    = (MMP_UINT16) CFG_LCD_WIDTH;
    MMP_UINT16  lcdHeight   = (MMP_UINT16) CFG_LCD_HEIGHT;
    MMP_UINT16  lcdPitch    = (MMP_UINT16) CFG_LCD_PITCH;

    if (LCDType == MMP_M2D_LCDA)
    {
        return MMP_RESULT_ERROR;
    }

    if (LCDType == MMP_M2D_LCDB)
    {
        lcdScanPtr = (MMP_UINT8 *)ithLcdGetBaseAddrB();
    }
    else if (LCDType == MMP_M2D_LCDC)
    {
        lcdScanPtr = (MMP_UINT8 *)ithLcdGetBaseAddrC();
    }

    if (m2d_trueColor == MMP_TRUE)
    {
        lcdDisplay = M2D_CreateSurface(lcdWidth,
                                       lcdHeight,
                                       lcdPitch,
                                       MMP_M2D_IMAGE_FORMAT_ARGB8888,
                                       (const void *)lcdScanPtr);
    }
    else
    {
        lcdDisplay = M2D_CreateSurface(lcdWidth,
                                       lcdHeight,
                                       lcdPitch,
                                       MMP_M2D_IMAGE_FORMAT_RGB565,
                                       (const void *)lcdScanPtr);
    }

    if (lcdDisplay == NULL)
    {
        return MMP_RESULT_ERROR;
    }

    *display = (MMP_M2D_SURFACE)lcdDisplay;

    return MMP_RESULT_SUCCESS;
} // End of mmpM2dCreateScreenSurface

#ifndef SHRINK_CODE_SIZE
/**
 * Read a rectangle region from a display to the buffer you assigned.
 *
 * @param   display     assign a display handle.
 * @param   destX       The x coordinate of the rectangle you want to read.
 * @param   destY       The y coordinate of the rectangle you want to read.
 * @param   rectWidth   The width of the rectangle you want to read.
 * @param   rectHeight  The height of the rectangle you want to read.
 * @param   bufPtr      The point of the buffer.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @author Mandy Wu
 */
MMP_RESULT
mmpM2dReadSurfaceRegion(
    MMP_M2D_SURFACE display,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        rectWidth,
    MMP_UINT        rectHeight,
    MMP_UINT8       *bufPtr)
{
    MMP_UINT32 i           = 0;
    MMP_UINT32 rectPitch   = 0;
    MMP_UINT8  *tempBufPtr = NULL;
    MMP_UINT8  *displayPtr = NULL;
    MMP_UINT32 bitbltId2   = 0;
    MMP_UINT32 currObjID   = 0;
    MMP_UINT32 objId0      = 0;
    MMP_UINT32 objId1      = 0;

    rectPitch  = rectWidth * 2;
    tempBufPtr = bufPtr;
    displayPtr = ((M2D_SURFACE *)display)->baseScanPtr
                 + (((M2D_SURFACE *)display)->pitch) * destY
                 + destX * 2;

    //WaitEngineIdle(CMDQ_VALUE_2D_IDLE, CMDQ_MASK_ENGINE_STATUS_2D_IDLE);
    //wait vg idle
    // Make sure the HW operation (use the surface) has finished.
    bitbltId2 = ((M2D_SURFACE *)display)->bitbltId2;
    do
    {
        currObjID = ithReadRegA(ITEM2D_VG_AHB_REG_BASE + ITEM2D_VG_REG_BID2_BASE);
    } while (currObjID < bitbltId2);

    for (i = 0; i < rectHeight; i++)
    {
        memcpy(tempBufPtr, displayPtr, rectPitch);
        tempBufPtr += rectPitch;
        displayPtr += ((M2D_SURFACE *)display)->pitch;
    }

    return MMP_RESULT_SUCCESS;
} // End of mmpM2dReadSurfaceRegion()
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Write a block of pixels to the display.
 *
 * @param   display     assign a display handle.
 * @param   destX       The x coordinate of the rectangle you want to write.
 * @param   destY       The y coordinate of the rectangle you want to read.
 * @param   rectWidth   The width of the rectangle you want to read.
 * @param   rectHeight  The height of the rectangle you want to read.
 * @param   bufPtr      The point of the buffer.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 * @see mmpM2dReadSurfaceRegion().
 * @author Mandy Wu
 */
MMP_RESULT
mmpM2dDrawSurfaceRegion(
    MMP_M2D_SURFACE display,
    MMP_INT         destX,
    MMP_INT         destY,
    MMP_UINT        rectWidth,
    MMP_UINT        rectHeight,
    MMP_UINT8       *bufPtr)
{
    MMP_UINT32 i           = 0;
    MMP_UINT32 rectPitch   = 0;
    MMP_UINT8  *tempBufPtr = NULL;
    MMP_UINT8  *displayPtr = NULL;
    MMP_UINT32 bitbltId2;
    MMP_UINT32 currObjID   = 0;
    MMP_UINT32 objId0      = 0;
    MMP_UINT32 objId1      = 0;

    rectPitch  = rectWidth * 2;
    tempBufPtr = bufPtr;
    displayPtr = ((M2D_SURFACE *)display)->baseScanPtr
                 + (((M2D_SURFACE *)display)->pitch) * destY
                 + destX * 2;

    //WaitEngineIdle(CMDQ_VALUE_2D_IDLE, CMDQ_MASK_ENGINE_STATUS_2D_IDLE);
    //wait vg idle

    // Make sure the HW operation (use the surface) has finished.
    bitbltId2 = ((M2D_SURFACE *)display)->bitbltId2;
    do
    {
        currObjID = ithReadRegA(ITEM2D_VG_AHB_REG_BASE + ITEM2D_VG_REG_BID2_BASE);
    } while (currObjID < bitbltId2);

    for (i = 0; i < rectHeight; i++)
    {
        memcpy(displayPtr, tempBufPtr, rectPitch);

        tempBufPtr += rectPitch;
        displayPtr += ((M2D_SURFACE *)display)->pitch;
    }
    return MMP_RESULT_SUCCESS;
} // End of mmpDrawDisplayRegion()
#endif

/**
 * Initialize the onscreen display structure.
 *
 * @author Erica Chang
 */
MMP_BOOL
M2D_EnableLcdSurface(
    void)
{
    M2D_BRUSHOBJ *brushObj = NULL;
    M2D_PENOBJ   *penObj   = NULL;
    M2D_FONTOBJ  *fontObj  = NULL;

    // init default brush
    brushObj = M2D_CreateBrush();
    memcpy(&defaultBrushObj, brushObj, sizeof(M2D_BRUSHOBJ));
    memcpy(&defaultBrush, brushObj->realizedBrush, sizeof(M2D_BRUSH));

    defaultBrushObj.realizedBrush = &defaultBrush;

    if (mmpM2dDeleteObject(MMP_M2D_BRUSHOBJ, (MMP_M2D_HANDLE) brushObj) != MMP_RESULT_SUCCESS)
        return MMP_FALSE;

    // init default pen
    penObj = M2D_CreatePen();
    memcpy(&defaultPenObj, penObj, sizeof(M2D_PENOBJ));

    if (mmpM2dDeleteObject(MMP_M2D_PENOBJ, (MMP_M2D_HANDLE) penObj) != MMP_RESULT_SUCCESS)
        return MMP_FALSE;

    return MMP_TRUE;
} // M2D_EnableLcdDisplay()

void
M2D_DisableLcdSurface(
    void)
{
    // Reset defaultDisplay
    memset(&defaultDisplay, 0, sizeof(M2D_SURFACE));
}

/**
 * Create a empty display context for the existing memory space.
 *
 * @param displayWidth     width, in pixels.
 * @param displayHeight    height, in pixels.
 * @param displayDelta     display width stride (row pitch), in bytes.
 * @param displayFormat    display format.
 * @param scanPtr          point to the base of the memory space.
 * @see M2D_DeleteSurface().
 */
M2D_SURFACE *
M2D_CreateSurface(
    MMP_UINT32           displayWidth,
    MMP_UINT32           displayHeight,
    MMP_UINT32           displayDelta,
    MMP_M2D_IMAGE_FORMAT displayFormat,
    const void           *scanPtr)
{
    M2D_SURFACE *display = NULL;

    if (/*(defaultBrushObj.brushStructSize == 0)
         ||*/
        (displayWidth == 0)
        || (displayHeight == 0)
        || (displayWidth > displayDelta)
        || (scanPtr == NULL))
    {
        return NULL;
    }

    display = (M2D_SURFACE *)malloc(sizeof(M2D_SURFACE));
    if (display == NULL)
        return NULL;

    display->baseScanPtr             = (MMP_UINT8 *)scanPtr;
    display->baseAddrOffset          = 0;
    display->dispalyStructSize       = sizeof(M2D_SURFACE);
    display->WCE_isCreateByOS        = MMP_FALSE;
    display->width                   = displayWidth;
    display->height                  = displayHeight;
    display->bitsPerPixel            = M2D_BPP[displayFormat];
    display->pitch                   = displayDelta;
    display->imageFormat             = displayFormat;
    display->backgroundMode          = MMP_M2D_BACKGROUND_MODE_OPAQUE;
    display->backgroundColor         = 0xFFFFFF;
    display->currPos.x               = 0;
    display->currPos.y               = 0;
    display->brushOriginPos.x        = 0;
    display->brushOriginPos.y        = 0;
    display->fontStyle               = MMP_M2D_FS_NORMAL;
    display->rop2                    = MMP_M2D_ROP2_COPYPEN;
    display->rotateOp                = MMP_M2D_ROTATE_OP_NOT;
    display->rotateRefPixelPos.x     = 0;
    display->rotateRefPixelPos.y     = 0;
    display->pen                     = &defaultPenObj;
    display->brush                   = &defaultBrushObj;
    display->font                    = &defaultFontObj;
    display->isResetLineStyle        = 0;

    display->clipSet.clipRegion      = NULL;
    display->clipSet.clipRegionCount = 0;
    display->bitbltId2               = 0;

    return display;
} // End of M2D_CreateSurface()

/**
 * Delete a empty display context for the existing memory space.
 *
 * @param bltBufDisplay     the handle to the display that is being deleted.
 * @see M2D_CreateSurface
 * @author Erica Chang
 */
MMP_BOOL
M2D_DeleteSurface(
    M2D_SURFACE *display)
{
    // Verify the display handle is valid or not.
    if (IS_INVALIDATE_SURFACE(display))
    {
        return MMP_FALSE;
    }

    // Delete clipping region if needed
    if (display->clipSet.clipRegion != NULL)
    {
        //printf("free clipRegion\n");
        free(display->clipSet.clipRegion);
    }

    free(display);

    return MMP_TRUE;
} // End of M2D_DeleteSurface()

/**
 * Select a logical object into the specified display context.
 *
 * @param display           display context handle.
 * @param object            selected object handle.
 * @param prevObject        return the previously selected object of the
 *                          specified type.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dSelectObject(
    MMP_M2D_OBJECT_TYPE objectType,
    MMP_M2D_SURFACE     display,
    MMP_M2D_HANDLE      object,
    MMP_M2D_HANDLE      *prevObject)
{
    //if display is a valid display
    if (IS_INVALIDATE_SURFACE(display))
    {
        printf("MMP_RESULT_ERROR_INVALID_DISP\n");
        return MMP_RESULT_ERROR_INVALID_DISP;
    }

    /***** Object Type *****/
    if (objectType == MMP_M2D_PENOBJ)
    {
        //Check if the brush object is a volid brush
        if (IS_INVALIDATE_PENOBJ(object))
        {
            return MMP_RESULT_ERROR_INVALID_OBJECT;
        }

        //assign the pen object to the display
        if (((M2D_SURFACE *)display)->pen != NULL)
        {
            *prevObject = (MMP_M2D_HANDLE)(((M2D_SURFACE *)display)->pen);
        }
        ((M2D_SURFACE *)display)->pen = (M2D_PENOBJ *)object;
    }
    else if (objectType == MMP_M2D_BRUSHOBJ)
    {
        //if the brush object is a volid brush
        if (IS_INVALIDATE_BRUSHOBJ(object))
        {
            printf("MMP_RESULT_ERROR_INVALID_OBJECT\n");

            return MMP_RESULT_ERROR_INVALID_OBJECT;
        }

        //assign the brush object to the display
        if (((M2D_SURFACE *)display)->brush != NULL)
        {
            *prevObject = (MMP_M2D_HANDLE)(((M2D_SURFACE *)display)->brush);
        }
        ((M2D_SURFACE *)display)->brush                                 = (M2D_BRUSHOBJ *)object;

        //set the bkcolor of the display's brush is the same as the bkcolor of the display
        ((M2D_SURFACE *)display)->brush->realizedBrush->backgroundColor = ((M2D_SURFACE *)display)->backgroundColor;
    }
    else if (objectType == MMP_M2D_FONTOBJ)
    {
        //if the font object is volid
        if ((object == 0)
            || ((M2D_FONTOBJ *)object)->fontStructSize != sizeof(M2D_FONTOBJ))
        {
            return MMP_RESULT_ERROR_INVALID_OBJECT;
        }

        //assign the font object to the display
        if (((M2D_SURFACE *)display)->font != NULL)
        {
            *prevObject = (MMP_M2D_HANDLE)(((M2D_SURFACE *)display)->font);
        }
        ((M2D_SURFACE *)display)->font = (M2D_FONTOBJ *)object;
    } // End of if (objectType)

    return MMP_RESULT_SUCCESS;
}

/**
 * Delete a logical object, and frees all system resources accociated with the
 * object.
 *
 * @param object        the object that is being deleted.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR
 *         otherwise.
 */
MMP_RESULT
mmpM2dDeleteObject(
    MMP_M2D_OBJECT_TYPE objectType,
    MMP_M2D_HANDLE      object)
{
    MMP_UINT8 *deletedScanPtr = NULL;
    MMP_UINT  index           = 0;

    if (objectType == MMP_M2D_PENOBJ)
    {
        //check if the object is a valid object
        if (IS_INVALIDATE_PENOBJ(object))
        {
            return MMP_RESULT_ERROR_INVALID_OBJECT;
        }
        if ((MMP_UINT8 *)object != (MMP_UINT8 *)&defaultPenObj)
            free((M2D_PENOBJ *)object);
    }
    else if (objectType == MMP_M2D_BRUSHOBJ)
    {
        //check if the object is a valid object
        if (IS_INVALIDATE_BRUSHOBJ(object))
        {
            return MMP_RESULT_ERROR_INVALID_OBJECT;
        }

        /*if (((M2D_BRUSHOBJ*)object)->realizedBrush->flag == M2D_PAT_BRUSH)
           {
            deletedScanPtr = (MMP_UINT8*)((MMP_UINT32)((M2D_BRUSHOBJ*)object)->realizedBrush->pattern
         + (MMP_UINT32)m2d_vramBasePtr);
           }*/

        if ((MMP_UINT8 *)object != (MMP_UINT8 *)&defaultBrushObj)
        {
            free(((M2D_BRUSHOBJ *)object)->realizedBrush);
            free((M2D_BRUSHOBJ *)object);
        }
        //release the pattern brush in VRAM
        /*if (deletedScanPtr != NULL)
           {
            free(deletedScanPtr);
           }*/
    }
    else if (objectType == MMP_M2D_FONTOBJ)
    {
        if (object != 0)
        {
            //check if the object is a valid object
            if (((M2D_FONTOBJ *)object)->fontStructSize != sizeof(M2D_FONTOBJ))
            {
                return MMP_RESULT_ERROR_INVALID_OBJECT;
            }

            for (index = 0; index < (((M2D_FONTOBJ *)object)->fontIndex); index++)
            {
                free(((M2D_FONTOBJ *)object)->PartsofFont[index]);
            }

            free((M2D_FONTOBJ *)object);
        } // End of if(object != 0)
    }

    return MMP_RESULT_SUCCESS;
}

MMP_UINT32
mmpM2dGetSurfaceBaseAddress(
    MMP_SURFACE surface)
{
    MMP_UINT32 addr;
    addr = (MMP_UINT32)((M2D_SURFACE *)surface)->baseScanPtr;
    return addr;
}

void
mmpM2dSetSurfaceBaseAddress(
    MMP_SURFACE surface,
    MMP_UINT32  addr)
{
    ((M2D_SURFACE *)surface)->baseScanPtr = addr;
}

void
mmpM2dSetSurfaceBaseWidth(
    MMP_SURFACE surface,
    MMP_UINT32  width)
{
    ((M2D_SURFACE *)surface)->width = width;
}

void
mmpM2dSetSurfaceBaseHeight(
    MMP_SURFACE surface,
    MMP_UINT32  height)
{
    ((M2D_SURFACE *)surface)->height = height;
}

void
mmpM2dSetSurfaceBasePitch(
    MMP_SURFACE surface,
    MMP_UINT32  pitch)
{
    ((M2D_SURFACE *)surface)->pitch = pitch;
}

/**
 * Set the visible LCD type.(LCD A, LCD B, or LCD C)
 *
 * @param lcdType   fill the LCD to be displayed.
 *
 */
void
mmpM2dSetVisibleLCD(
    MMP_M2D_LCD_TYPE lcdType)
{
    ithLcdEnableHwFlip();

    if (lcdType == MMP_M2D_LCDA)
    {
        ithCmdQFlip(LCD_BUFFER_A);
    }
    else if (lcdType == MMP_M2D_LCDB)
    {
        ithCmdQFlip(LCD_BUFFER_B);
    }
    else if (lcdType == MMP_M2D_LCDC)
    {
        ithCmdQFlip(LCD_BUFFER_C);
    }
} // mmpM2dSetVisibleLCD

void
mmpM2dWaitIdle()
{
    ITEM2Duint32 frameId = iteM2dHardwareGenFrameID();

    iteM2dHardwareNullFire(frameId);
    iteM2dHardwareWaitFrameID(frameId);
}