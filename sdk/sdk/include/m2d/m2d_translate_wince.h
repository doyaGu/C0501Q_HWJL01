/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 *  Header file to translate WinCE to MMP Graphic 2D API.
 *
 * @author Erica Chang
 * @version 0.01
 */
#ifndef __TRANSLATE_WINCE_H
#define __TRANSLATE_WINCE_H

#include "ite/mmp_types.h"

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

//=============================
//  Display Context Funcitons
//=============================
/**
 * Get the display context corresponding to the specified device context.
 *
 * @param handleDC      device context handle.
 * @param display       return the display handle corresponding to the
 *                      specified device context handle.
 * @author Erica Chang
 */
M2D_API MMP_BOOL
M2D_GetDisplayFromHdc(
    HDC             handleDC,
    MMP_M2D_SURFACE *display);

/**
 * Create the corresponding display for the specified HDC.
 *
 * @param handleDC      device context handle.
 * @param display       return the display handle corresponding to the
 *                      specified device context handle.
 * @author Erica Chang
 */
MMP_BOOL
M2D_CreateDisplayForHdc(
    HDC             handleDC,
    MMP_M2D_SURFACE *display);

/**
 * Delete corresponding display for the specified HDC.
 *
 * @param handleDC      device context handle.
 * @param display       the display handle corresponding to the
 *                      specified device context handle.
 * @author Erica Chang
 */
MMP_BOOL
M2D_DeleteDisplayForHdc(
    HDC             handleDC,
    MMP_M2D_SURFACE display);

#ifdef __cplusplus
}
#endif

#endif /* __TRANSLATE_WINCE_H */