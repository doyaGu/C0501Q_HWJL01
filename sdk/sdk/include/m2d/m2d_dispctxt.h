/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Used as private template header file.
 *
 * @author Erica Chang
 * @version 0.1
 */

#ifndef __DISPCTXT_H
#define __DISPCTXT_H

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
 * Initialize the onscreen display structure.
 *
 * @author Erica Chang
 */
MMP_BOOL
M2D_EnableLcdSurface(
    void);

void
M2D_DisableLcdSurface(
    void);

/**
 * Create a empty display context for the existing memory space.
 *
 * @param displayWidth     width, in pixels.
 * @param displayHeight    height, in pixels.
 * @param displayDelta     display width stride (row pitch), in bytes.
 * @param displayFormat    display format.
 * @param scanPtr          point to the base of the memory space.
 * @see mmpM2dDeleteSurface().
 */
M2D_API M2D_SURFACE *
M2D_CreateSurface(
    MMP_UINT32           displayWidth,
    MMP_UINT32           displayHeight,
    MMP_UINT32           displayDelta,
    MMP_M2D_IMAGE_FORMAT displayFormat,
    const void           *scanPtr);

/**
 * Delete a empty display context for the existing memory space.
 *
 * @param bltBufDisplay     the handle to the display that is being deleted.
 * @see M2D_CreateCpuBltDisplay().
 * @author Erica Chang
 */
M2D_API MMP_BOOL
M2D_DeleteSurface(
    M2D_SURFACE *display);

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __DISPCTXT_H