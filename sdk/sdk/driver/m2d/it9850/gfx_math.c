/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file gfx_math.c
 *  GFX mathematical layer API function file.
 *
 * @author Awin Huang
 * @version 1.0
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gfx.h"
#include "gfx_math.h"
#include "msg.h"

//=============================================================================
//                              Compile Option
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
/* True if address a has acceptable alignment */
#define is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define floatsEqual(x,y)    (fabs(x - y) <= 0.00001f * MIN(fabs(x), fabs(y))) 
#define floatIsZero(x)      (floatsEqual((x) + 1, 1)) 

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
gfxMatrixIdentify(
    GFX_MATRIX* mtx)
{
    GFX_FUNC_ENTRY;
    mtx->m[0][0] = 1;   mtx->m[0][1] = 0;   mtx->m[0][2] = 0;
    mtx->m[1][0] = 0;   mtx->m[1][1] = 1;   mtx->m[1][2] = 0;
    mtx->m[2][0] = 0;   mtx->m[2][1] = 0;   mtx->m[2][2] = 1;
    GFX_FUNC_LEAVE;
}

void
gfxMatrixSet(
    GFX_MATRIX* mtx,
    float v00, float v01, float v02,
    float v10, float v11, float v12,
    float v20, float v21, float v22)
{
    GFX_FUNC_ENTRY;
    mtx->m[0][0] = v00;   mtx->m[0][1] = v01;   mtx->m[0][2] = v02;
    mtx->m[1][0] = v10;   mtx->m[1][1] = v11;   mtx->m[1][2] = v12;
    mtx->m[2][0] = v20;   mtx->m[2][1] = v21;   mtx->m[2][2] = v22;
    GFX_FUNC_LEAVE;
}

void
gfxMatrixSet2(
    GFX_MATRIX* mtx,
    GFX_MATRIX* newmtx)
{
    GFX_FUNC_ENTRY;
    memcpy(mtx, newmtx, sizeof(GFX_MATRIX));
    GFX_FUNC_LEAVE;
}

void
gfxMatrixMultiply(
    GFX_MATRIX* mtx,
    GFX_MATRIX* rightMtx)
{
    GFX_MATRIX  tempMtx;
    int         i, j;

    GFX_FUNC_ENTRY;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            tempMtx.m[i][j] =
                mtx->m[i][0] * rightMtx->m[0][j] +
                mtx->m[i][1] * rightMtx->m[1][j] +
                mtx->m[i][2] * rightMtx->m[2][j];
        }
    }

    gfxMatrixSet2(mtx, &tempMtx);
    GFX_FUNC_LEAVE;
}

void
gfxMatrixRotate(
    GFX_MATRIX* mtx,
    float       degree)
{
    float       rad = (degree * GFX_PI / 180.0f);
    float       cosA = cos(rad);
    float       sinA = sin(rad);
    GFX_MATRIX  rotateMtx;

    GFX_FUNC_ENTRY;
    gfxMatrixSet(&rotateMtx,
        cosA, -sinA, 0,
        sinA, cosA,  0,
        0,    0,     1);
    gfxMatrixMultiply(mtx, &rotateMtx);
    GFX_FUNC_LEAVE;
}

void
gfxMatrixTranslate(
    GFX_MATRIX* mtx,
    int         offsetX,
    int         offsetY)
{
    GFX_MATRIX  translateMtx;

    GFX_FUNC_ENTRY;
    gfxMatrixSet(&translateMtx,
        1, 0, offsetX,
        0, 1, offsetY,
        0, 0, 1);
    gfxMatrixMultiply(mtx, &translateMtx);
    GFX_FUNC_LEAVE;
}

void
gfxMatrixScale(
    GFX_MATRIX* mtx,
    float       scaleX,
    float       scaleY)
{
    GFX_MATRIX  scaleMtx;

    GFX_FUNC_ENTRY;
    gfxMatrixSet(&scaleMtx,
        scaleX, 0,      0,
        0,      scaleY, 0,
        0,      0,      1);
    gfxMatrixMultiply(mtx, &scaleMtx);
    GFX_FUNC_LEAVE;
}

bool
gfxMatrixInverse(
    GFX_MATRIX* originMtx,
    GFX_MATRIX* inverseMtx)
{
    bool result;
    bool affine;
    float D0;
    float D1;
    float D2;
    float D;

    result = true;

    GFX_FUNC_ENTRY;

    affine = gfxIsAffine(originMtx);

    /* Calculate determinant */


    D0 = originMtx->m[1][1] * originMtx->m[2][2] - originMtx->m[2][1] * originMtx->m[1][2];
    D1 = originMtx->m[2][0] * originMtx->m[1][2] - originMtx->m[1][0] * originMtx->m[2][2];
    D2 = originMtx->m[1][0] * originMtx->m[2][1] - originMtx->m[2][0] * originMtx->m[1][1];
    D  = originMtx->m[0][0] * D0 + originMtx->m[0][1] * D1 + originMtx->m[0][2] * D2;

    /* Check if singular */
    if( D == 0.0f )
    {
        return false;
    }
    D = 1.0f / D;

    /* Calculate inverse */
    inverseMtx->m[0][0] = D * D0;
    inverseMtx->m[1][0] = D * D1;
    inverseMtx->m[2][0] = D * D2;
    inverseMtx->m[0][1] = D * (originMtx->m[2][1] * originMtx->m[0][2] - originMtx->m[0][1] * originMtx->m[2][2]);
    inverseMtx->m[1][1] = D * (originMtx->m[0][0] * originMtx->m[2][2] - originMtx->m[2][0] * originMtx->m[0][2]);
    inverseMtx->m[2][1] = D * (originMtx->m[2][0] * originMtx->m[0][1] - originMtx->m[0][0] * originMtx->m[2][1]);
    inverseMtx->m[0][2] = D * (originMtx->m[0][1] * originMtx->m[1][2] - originMtx->m[1][1] * originMtx->m[0][2]);
    inverseMtx->m[1][2] = D * (originMtx->m[1][0] * originMtx->m[0][2] - originMtx->m[0][0] * originMtx->m[1][2]);
    inverseMtx->m[2][2] = D * (originMtx->m[0][0] * originMtx->m[1][1] - originMtx->m[1][0] * originMtx->m[0][1]);

    //affine matrix stays affine
    if(affine)
    {
        inverseMtx->m[0][2] = 0;
        inverseMtx->m[1][2] = 0;
        inverseMtx->m[2][2] = 1;
    }

    GFX_FUNC_LEAVE;
    return result;
}

void
gfxMatrixTransform(
    GFX_MATRIX* mtx,
    int         srcX,
    int         srcY,
    float*      dstX,
    float*      dstY)
{
    GFX_FUNC_ENTRY;
    *dstX = (srcX*mtx->m[0][0] + srcY*mtx->m[0][1] + mtx->m[0][2]) /
            (srcX*mtx->m[2][0] + srcY*mtx->m[2][1] + mtx->m[2][2]) ;
    *dstY = (srcX*mtx->m[1][0] + srcY*mtx->m[1][1] + mtx->m[1][2]) /
            (srcX*mtx->m[2][0] + srcY*mtx->m[2][1] + mtx->m[2][2]) ;
    GFX_FUNC_LEAVE;
}

bool
gfxMatrixWarpQuadToSquare(
    GFX_RECTANGLE s,
    GFX_MATRIX* mtx,
    bool bInverse)
{
    bool result;
    bool ret;
    GFX_MATRIX Invmat;
    GFX_FUNC_ENTRY;

    result = true;


    if(!mtx) {
        result = false;
        goto exit;
    }

    ret = gfxMatrixWarpSquareToQuad(s, mtx, bInverse);
    if(ret == false) {
        result = false;
        goto exit;
    }

    if(!gfxMatrixInverse(mtx, &Invmat)) {
        result = false;
        goto exit;
    }

    mtx->m[0][0] = Invmat.m[0][0];
    mtx->m[1][0] = Invmat.m[1][0];
    mtx->m[2][0] = Invmat.m[2][0];
    mtx->m[0][1] = Invmat.m[0][1];
    mtx->m[1][1] = Invmat.m[1][1];
    mtx->m[2][1] = Invmat.m[2][1];
    mtx->m[0][2] = Invmat.m[0][2];
    mtx->m[1][2] = Invmat.m[1][2];
    mtx->m[2][2] = Invmat.m[2][2];

exit:
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxMatrixWarpSquareToQuad(
    GFX_RECTANGLE d,
    GFX_MATRIX* mtx,
    bool bInverse)
{
    bool result;
    float diffx1 ,diffy1 ,diffx2 = d.x2 - d.x3 ,diffy2;
    float det;
    float sumx ,sumy;
    float oodet ,g ,h;
    GFX_FUNC_ENTRY;

    result = true;

    if(!mtx) {
        result = false;
        goto exit;
    }

    //from Heckbert:Fundamentals of Texture Mapping and Image Warping
    //Note that his mapping of vertices is different from OpenVG's
    //(0,0) => (dx0,dy0)
    //(1,0) => (dx1,dy1)
    //(0,1) => (dx2,dy2)
    //(1,1) => (dx3,dy3)

    diffx1 = d.x1 - d.x3;
    diffy1 = d.y1 - d.y3;
    diffx2 = d.x2 - d.x3;
    diffy2 = d.y2 - d.y3;

    det = diffx1*diffy2 - diffx2*diffy1;
    if(det == 0.0f) {
        result = false;
        goto exit;
    }

    sumx = d.x0 - d.x1 + d.x3 - d.x2;
    sumy = d.y0 - d.y1 + d.y3 - d.y2;

    //printf("sumx:%f,sumy:%f\n",sumx,sumy);
#if 1
    if(sumx == 0.0f && sumy == 0.0f)
    {	//affine mapping
        mtx->m[0][0] = d.x1 - d.x0;  mtx->m[0][1] = d.y1 - d.y0;  mtx->m[0][2] = 0.0f;
        mtx->m[1][0] = d.x3 - d.x1;  mtx->m[1][1] = d.y3 - d.y1;  mtx->m[1][2] = 0.0f;
        mtx->m[2][0] = d.x0;         mtx->m[2][1] = d.y0;         mtx->m[2][2] = 1.0f;
        
        if(bInverse)
            mtx->m[2][0] = d.x1;

        result = true;
        goto exit;
    }

    oodet = 1.0f / det;
    g = (sumx*diffy2 - diffx2*sumy) * oodet;
    h = (diffx1*sumy - sumx*diffy1) * oodet;
    //printf("g:%f h:%f oodet:%f det:%f\n",g,h,oodet,det);
    mtx->m[0][0] = d.x1-d.x0+g*d.x1;  mtx->m[0][1] = d.y1-d.y0+g*d.y1;  mtx->m[0][2] = g;
    mtx->m[1][0] = d.x2-d.x0+h*d.x2;  mtx->m[1][1] = d.y2-d.y0+h*d.y2;  mtx->m[1][2] = h;
    mtx->m[2][0] = d.x0;              mtx->m[2][1] = d.y0;              mtx->m[2][2] = 1.0f;

    if(bInverse)
    {
        mtx->m[0][0] = d.x0-d.x1+g*d.x0;
        mtx->m[1][0] = d.x3-d.x1+h*d.x3;
        mtx->m[2][0] = d.x1;
    }
#else
    if(sumx == 0.0f && sumy == 0.0f)
    {   //affine mapping
        mtx->m[0][0] = d.x1 - d.x0;
        mtx->m[0][1] = d.y1 - d.y0;
        mtx->m[0][2] = d.x0;
        mtx->m[1][0] = d.x3 - d.x1;
        mtx->m[1][1] = d.y3 - d.y1;
        mtx->m[1][2] = d.y0;
        mtx->m[2][0] = 0.0f;
        mtx->m[2][1] = 0.0f;
        mtx->m[2][2] = 1.0f;
        result = true;
        goto exit;
    }

    oodet = 1.0f / det;
    g = (sumx*diffy2 - diffx2*sumy) * oodet;
    h = (diffx1*sumy - sumx*diffy1) * oodet;

    mtx->m[0][0] = d.x1-d.x0+g*d.x1;
    mtx->m[0][1] = d.y1-d.y0+g*d.y1;
    mtx->m[0][2] = d.x0;
    mtx->m[1][0] = d.x2-d.x0+h*d.x2;
    mtx->m[1][1] = d.y2-d.y0+h*d.y2;
    mtx->m[1][2] = d.y0;
    mtx->m[2][0] = g;
    mtx->m[2][1] = h;
    mtx->m[2][2] = 1.0f;
#endif
exit:
    GFX_FUNC_LEAVE;

    return result;
}

bool
gfxMatrixWarpQuadToQuad(
    GFX_RECTANGLE d,
    GFX_RECTANGLE s,
    GFX_MATRIX* mtx,
    bool bInverse)
{
    bool result;
    GFX_MATRIX mtx1;
    GFX_MATRIX mtx2;
    GFX_FUNC_ENTRY;

    result = true;

    result = gfxMatrixWarpQuadToSquare(s, &mtx1, bInverse);
    if (result == false)
        goto exit;

    result = gfxMatrixWarpSquareToQuad(d, &mtx2, bInverse);
    if (result == false)
        goto exit;

    gfxMatrixSet2(mtx, &mtx2);

    //printf("++Multiply 1++\n");
    //printf("%f %f %f\n",mtx1.m[0][0],mtx1.m[0][1],mtx1.m[0][2]);
    //printf("%f %f %f\n",mtx1.m[1][0],mtx1.m[1][1],mtx1.m[1][2]);
    //printf("%f %f %f\n",mtx1.m[2][0],mtx1.m[2][1],mtx1.m[2][2]);
    //printf("++Multiply 2++\n");
    //printf("%f %f %f\n",mtx->m[0][0],mtx->m[0][1],mtx->m[0][2]);
    //printf("%f %f %f\n",mtx->m[1][0],mtx->m[1][1],mtx->m[1][2]);
    //printf("%f %f %f\n",mtx->m[2][0],mtx->m[2][1],mtx->m[2][2]);

    gfxMatrixMultiply(mtx, &mtx1);

    //printf("++Multiply++\n");
    //printf("%f %f %f\n",mtx->m[0][0],mtx->m[0][1],mtx->m[0][2]);
    //printf("%f %f %f\n",mtx->m[1][0],mtx->m[1][1],mtx->m[1][2]);
    //printf("%f %f %f\n",mtx->m[2][0],mtx->m[2][1],mtx->m[2][2]);
    
exit:
    GFX_FUNC_LEAVE;

    return true;
}

bool
gfxIsAffine(
    GFX_MATRIX* mtx)
{
    return floatIsZero(mtx->m[0][2]) && floatIsZero(mtx->m[1][2])
        && floatsEqual(mtx->m[2][2], 1);
}



//=============================================================================
//                              Private Function Definition
//=============================================================================

