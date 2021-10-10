/*
 * Copyright (c) 2004 SMedia technology Systems Corp. All Rights Reserved.
 */
/** @file
 * Provide some APIs for internal use.
 *
 * @author Mandy Wu
 * @version 0.1
 */
#include "m2d/m2d_util.h"
#include "m2d/m2d_graphics.h"
#include "m2d/m2d_engine.h"

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
#ifndef SHRINK_CODE_SIZE
/**
 * Check if a display is valid.
 *
 * @return MMP_TRUE if a display is valid, invalid of MMP_FALSE otherwise.
 */
MMP_BOOL
M2D_ValidDisplay(
    MMP_M2D_SURFACE disp)
{
    return !IS_INVALIDATE_SURFACE(disp);
}
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Get display width, for internal use.
 *
 * @return display width.
 */
MMP_UINT32
M2D_GetDisplayWidth(
    MMP_M2D_SURFACE disp)
{
    return ((M2D_SURFACE *)disp)->width;
}
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Get display height, for internal use.
 *
 * @return display height.
 */
MMP_UINT32
M2D_GetDisplayHeight(
    MMP_M2D_SURFACE disp)
{
    return ((M2D_SURFACE *)disp)->height;
}
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Get display base offset, for internal use.
 *
 * @return display base offset.
 */
MMP_UINT32
M2D_GetDisplayBaseOffset(
    MMP_M2D_SURFACE disp)
{
    return (MMP_UINT32)(((M2D_SURFACE *)disp)->baseAddrOffset);
}
#endif

#ifndef SHRINK_CODE_SIZE
/**
 * Get display base address, for internal use.
 *
 * @return display base address.
 */
MMP_UINT32
M2D_GetDisplayBaseAddress(
    MMP_M2D_SURFACE disp)
{
    return (MMP_UINT32)(((M2D_SURFACE *)disp)->baseScanPtr);
}
#endif