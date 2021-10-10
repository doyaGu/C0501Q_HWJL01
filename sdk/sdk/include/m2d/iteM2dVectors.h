/*
 * Copyright (c) 2007 Ivan Leben
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file COPYING;
 * if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __ITEM2DVECTORS_H
#define __ITEM2DVECTORS_H

#include "iteM2dDefs.h"

/* Vector structures
 *--------------------------------------------------------------*/
typedef struct
{
  ITEM2Dfloat x,y;
} ITEM2DVector2;

typedef struct _ITEM2DVector2Coord
{
	ITEM2Dint64 x;
	ITEM2Dint64 y;
}ITEM2DVector2Coord;

void ITEM2DVector2_ctor(ITEM2DVector2 *v);
void ITEM2DVector2_dtor(ITEM2DVector2 *v);

typedef struct
{
  ITEM2Dfloat x,y,z;
} ITEM2DVector3;

void ITEM2DVector3_ctor(ITEM2DVector3 *v);
void ITEM2DVector3_dtor(ITEM2DVector3 *v);

typedef struct
{
  ITEM2Dfloat x,y,z,w;
} ITEM2DVector4;

void ITEM2DVector4_ctor(ITEM2DVector4 *v);
void ITEM2DVector4_dtor(ITEM2DVector4 *v);

typedef struct
{
	ITEM2Dint x, y, w, h;
}ITEM2DRectangle;

typedef struct _ITEM2DRectCoord
{
	ITEM2Dint64 left;
	ITEM2Dint64 top;
	ITEM2Dint64 right;
	ITEM2Dint64 down;
}ITEM2DRectCoord;

void ITEM2DRectangle_ctor(ITEM2DRectangle *r);
void ITEM2DRectangle_dtor(ITEM2DRectangle *r);
void ITEM2DRectangleSet(ITEM2DRectangle *r, ITEM2Dfloat x, ITEM2Dfloat y, ITEM2Dfloat w, ITEM2Dfloat h);

typedef struct
{
  ITEM2Dfloat m[3][3];
} ITEM2DMatrix3x3;

void ITEM2DMatrix3x3_ctor(ITEM2DMatrix3x3 *m);
void ITEM2DMatrix3x3_dtor(ITEM2DMatrix3x3 *m);

/*------------------------------------------------------------
 * Vector Arrays
 *------------------------------------------------------------*/

#define _ITEM_T ITEM2DVector2
#define _ARRAY_T ITEM2DVector2Array
#define _FUNC_T iteM2dVector2Array
#define _ARRAY_DECLARE
#include "iteM2dArrayBase.h"

/*-----------------------------------------------------
 * Macros for matrix operations
 *-----------------------------------------------------*/

#define M2DSETMAT(mat, m00, m01, m02, m10, m11, m12, m20, m21, m22) { \
mat.m[0][0] = m00; mat.m[0][1] = m01; mat.m[0][2] = m02; \
  mat.m[1][0] = m10; mat.m[1][1] = m11; mat.m[1][2] = m12; \
  mat.m[2][0] = m20; mat.m[2][1] = m21; mat.m[2][2] = m22; }

#define SETMATMAT(m1, m2) { \
int i,j; \
  for(i=0;i<3;i++) \
  for(j=0;j<3;j++) \
    m1.m[i][j] = m2.m[i][j]; }

#define MULMATS(mat, s) { \
int i,j; \
  for(i=0;i<3;i++) \
  for(j=0;j<3;j++) \
    mat.m[i][j] *= s; }

#define DIVMATS(mat, s) { \
int i,j; \
  for(i=0;i<3;i++) \
  for(j=0;j<3;j++) \
    mat.m[i][j] /= s; }

#define MULMATMAT(m1, m2, mout) { \
int i,j; \
  for(i=0;i<3;i++) \
  for(j=0;j<3;j++) \
    mout.m[i][j] = \
      m1.m[i][0] * m2.m[0][j] + \
      m1.m[i][1] * m2.m[1][j] + \
      m1.m[i][2] * m2.m[2][j]; }

#define IDMAT(mat) M2DSETMAT(mat, 1,0,0, 0,1,0, 0,0,1)

#define TRANSLATEMATL(mat, tx, ty) { \
ITEM2DMatrix3x3 trans,temp; \
  M2DSETMAT(trans, 1,0,tx, 0,1,ty, 0,0,1); \
  MULMATMAT(trans, mat, temp); \
  SETMATMAT(mat, temp); }

#define TRANSLATEMATR(mat, tx, ty) { \
ITEM2DMatrix3x3 trans,temp; \
  M2DSETMAT(trans, 1,0,tx, 0,1,ty, 0,0,1); \
  MULMATMAT(mat, trans, temp); \
  SETMATMAT(mat, temp); }

#define SCALEMATL(mat, sx, sy) { \
ITEM2DMatrix3x3 scale, temp; \
  M2DSETMAT(scale, sx,0,0, 0,sy,0, 0,0,1); \
  MULMATMAT(scale, mat, temp); \
  SETMATMAT(mat, temp); }

#define SCALEMATR(mat, sx, sy) { \
ITEM2DMatrix3x3 scale, temp; \
  M2DSETMAT(scale, sx,0,0, 0,sy,0, 0,0,1); \
  MULMATMAT(mat, scale, temp); \
  SETMATMAT(mat, temp); }

#define ITEEARMATL(mat, shx, shy) {\
ITEM2DMatrix3x3 shear, temp;\
  M2DSETMAT(shear, 1,shx,0, shy,1,0, 0,0,1); \
  MULMATMAT(shear, mat, temp); \
  SETMATMAT(mat, temp); }

#define ITEEARMATR(mat, shx, shy) {\
ITEM2DMatrix3x3 shear, temp;\
  M2DSETMAT(shear, 1,shx,0, shy,1,0, 0,0,1); \
  MULMATMAT(mat, shear, temp); \
  SETMATMAT(mat, temp); }

#define ROTATEMATL(mat, a) { \
ITEM2Dfloat cosa=ITEM2D_COS(a), sina=ITEM2D_SIN(a); \
  ITEM2DMatrix3x3 rot, temp; \
  M2DSETMAT(rot, cosa,-sina,0, sina,cosa,0, 0,0,1); \
  MULMATMAT(rot, mat, temp); \
  SETMATMAT(mat, temp); }

#define ROTATEMATR(mat, a) { \
ITEM2Dfloat cosa=ITEM2D_COS(a), sina=ITEM2D_SIN(a); \
 ITEM2DMatrix3x3 rot, temp; \
  M2DSETMAT(rot, cosa,-sina,0, sina,cosa,0, 0,0,1); \
  MULMATMAT(mat, rot, temp); \
  SETMATMAT(mat, temp); }

#define TRANSFORM2TO(v, mat, vout) { \
vout.x = v.x*mat.m[0][0] + v.y*mat.m[0][1] + 1*mat.m[0][2]; \
  vout.y = v.x*mat.m[1][0] + v.y*mat.m[1][1] + 1*mat.m[1][2]; }

#define TRANSFORM3TO(v, mat, vout) { \
vout.x = v.x*mat.m[0][0] + v.y*mat.m[0][1] + v.z*mat.m[0][2]; \
  vout.y = v.x*mat.m[1][0] + v.y*mat.m[1][1] + v.z*mat.m[1][2]; \
  vout.z = v.x*mat.m[2][0] + v.y*mat.m[2][1] + v.z*mat.m[2][2]; }

#define TRANSFORM2(v, mat) { \
ITEM2DVector2 temp; TRANSFORM2TO(v, mat, temp); v = temp; }

#define TRANSFORM2DIRTO(v, mat, vout) { \
vout.x = v.x*mat.m[0][0] + v.y*mat.m[0][1]; \
  vout.y = v.x*mat.m[1][0] + v.y*mat.m[1][1]; }

#define TRANSFORM2DIR(v, mat) { \
ITEM2DVector2 temp; TRANSFORM2DIRTO(v, mat, temp); v = temp; }


/*--------------------------------------------------------
 * Additional functions
 *--------------------------------------------------------- */

ITEM2Dint iteM2dInvertMatrix(ITEM2DMatrix3x3 *m, ITEM2DMatrix3x3 *mout);

ITEM2Dfloat iteM2dVectorOrientation(ITEM2DVector2 *v);

int iteM2dLineLineXsection(ITEM2DVector2 *o1, ITEM2DVector2 *v1,
                       ITEM2DVector2 *o2, ITEM2DVector2 *v2,
                       ITEM2DVector2 *xsection);

#endif/* __ITEM2DVECTORS_H */
