/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file gfx_math.h
 *  GFX mathematical layer API header file.
 *
 * @author Awin Huang
 * @version 1.0
 */

#ifndef __GFX_MATH_H__
#define __GFX_MATH_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * DLL export API declaration for Win32.
 */

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#define GFX_PI  3.14159f

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct _GFX_MATRIX
{
    float m[3][3];
} GFX_MATRIX;

typedef struct _GFX_RECTANGLE
{
    float x0, y0;
    float x1, y1;
    float x2, y2;
    float x3, y3;
} GFX_RECTANGLE;

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Declaration
//=============================================================================
void
gfxMatrixIdentify(
    GFX_MATRIX* mtx);

void
gfxMatrixSet(
    GFX_MATRIX* mtx,
    float v00, float v01, float v02,
    float v10, float v11, float v12,
    float v20, float v21, float v22);

void
gfxMatrixSet2(
    GFX_MATRIX* mtx,
    GFX_MATRIX* newmtx);

void
gfxMatrixMultiply(
    GFX_MATRIX* mtx,
    GFX_MATRIX* rightMtx);

void
gfxMatrixRotate(
    GFX_MATRIX* mtx,
    float       degree);

void
gfxMatrixTranslate(
    GFX_MATRIX* mtx,
    int         offsetX,
    int         offsetY);

void
gfxMatrixScale(
    GFX_MATRIX* mtx,
    float       scaleX,
    float       scaleY);

bool
gfxMatrixInverse(
    GFX_MATRIX* originMtx,
    GFX_MATRIX* inverseMtx);

void
gfxMatrixTransform(
    GFX_MATRIX* mtx,
    int         srcX,
    int         srcY,
    float*      dstX,
    float*      dstY);

bool
gfxMatrixWarpQuadToQuad(
    GFX_RECTANGLE d,
    GFX_RECTANGLE s,
    GFX_MATRIX* mtx,
    bool bInverse);

bool
gfxMatrixWarpSquareToQuad(
    GFX_RECTANGLE d,
    GFX_MATRIX* mtx,
    bool bInverse);

bool
gfxIsAffine(
    GFX_MATRIX* mtx);

#ifdef __cplusplus
}
#endif

#endif // __GFX_MATH_H__
