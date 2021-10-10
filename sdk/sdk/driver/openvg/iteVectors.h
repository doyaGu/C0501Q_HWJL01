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

#ifndef __ITEVECTORS_H
#define __ITEVECTORS_H

#include "iteDefs.h"

/* Vector structures
 *--------------------------------------------------------------*/
typedef struct
{
  ITEfloat x,y;
} ITEVector2;

typedef struct _ITEVector2Coord
{
	ITEint64 x;
	ITEint64 y;
}ITEVector2Coord;

void ITEVector2_ctor(ITEVector2 *v);
void ITEVector2_dtor(ITEVector2 *v);

typedef struct
{
  ITEfloat x,y,z;
} ITEVector3;

void ITEVector3_ctor(ITEVector3 *v);
void ITEVector3_dtor(ITEVector3 *v);

typedef struct
{
  ITEfloat x,y,z,w;
} ITEVector4;

void ITEVector4_ctor(ITEVector4 *v);
void ITEVector4_dtor(ITEVector4 *v);

typedef struct
{
	ITEint x, y, w, h;
}ITERectangle;

typedef struct _ITERectCoord
{
	ITEint64 left;
	ITEint64 top;
	ITEint64 right;
	ITEint64 down;
}ITERectCoord;

void ITERectangle_ctor(ITERectangle *r);
void ITERectangle_dtor(ITERectangle *r);
void ITERectangleSet(ITERectangle *r, ITEfloat x, ITEfloat y, ITEfloat w, ITEfloat h);

typedef struct
{
  ITEfloat m[3][3];
} ITEMatrix3x3;

void ITEMatrix3x3_ctor(ITEMatrix3x3 *m);
void ITEMatrix3x3_dtor(ITEMatrix3x3 *m);

/*------------------------------------------------------------
 * Vector Arrays
 *------------------------------------------------------------*/

#define _ITEM_T ITEVector2
#define _ARRAY_T ITEVector2Array
#define _FUNC_T iteVector2Array
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

/*-----------------------------------------------------
 * Macros for matrix operations
 *-----------------------------------------------------*/

#define SETMAT(mat, m00, m01, m02, m10, m11, m12, m20, m21, m22) { \
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

#define IDMAT(mat) SETMAT(mat, 1,0,0, 0,1,0, 0,0,1)

#define TRANSLATEMATL(mat, tx, ty) { \
ITEMatrix3x3 trans,temp; \
  SETMAT(trans, 1,0,tx, 0,1,ty, 0,0,1); \
  MULMATMAT(trans, mat, temp); \
  SETMATMAT(mat, temp); }

#define TRANSLATEMATR(mat, tx, ty) { \
ITEMatrix3x3 trans,temp; \
  SETMAT(trans, 1,0,tx, 0,1,ty, 0,0,1); \
  MULMATMAT(mat, trans, temp); \
  SETMATMAT(mat, temp); }

#define SCALEMATL(mat, sx, sy) { \
ITEMatrix3x3 scale, temp; \
  SETMAT(scale, sx,0,0, 0,sy,0, 0,0,1); \
  MULMATMAT(scale, mat, temp); \
  SETMATMAT(mat, temp); }

#define SCALEMATR(mat, sx, sy) { \
ITEMatrix3x3 scale, temp; \
  SETMAT(scale, sx,0,0, 0,sy,0, 0,0,1); \
  MULMATMAT(mat, scale, temp); \
  SETMATMAT(mat, temp); }

#define ITEEARMATL(mat, shx, shy) {\
ITEMatrix3x3 shear, temp;\
  SETMAT(shear, 1,shx,0, shy,1,0, 0,0,1); \
  MULMATMAT(shear, mat, temp); \
  SETMATMAT(mat, temp); }

#define ITEEARMATR(mat, shx, shy) {\
ITEMatrix3x3 shear, temp;\
  SETMAT(shear, 1,shx,0, shy,1,0, 0,0,1); \
  MULMATMAT(mat, shear, temp); \
  SETMATMAT(mat, temp); }

#define ROTATEMATL(mat, a) { \
ITEfloat cosa=ITE_COS(a), sina=ITE_SIN(a); \
  ITEMatrix3x3 rot, temp; \
  SETMAT(rot, cosa,-sina,0, sina,cosa,0, 0,0,1); \
  MULMATMAT(rot, mat, temp); \
  SETMATMAT(mat, temp); }

#define ROTATEMATR(mat, a) { \
ITEfloat cosa=ITE_COS(a), sina=ITE_SIN(a); \
  ITEMatrix3x3 rot, temp; \
  SETMAT(rot, cosa,-sina,0, sina,cosa,0, 0,0,1); \
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
ITEVector2 temp; TRANSFORM2TO(v, mat, temp); v = temp; }

#define TRANSFORM2DIRTO(v, mat, vout) { \
vout.x = v.x*mat.m[0][0] + v.y*mat.m[0][1]; \
  vout.y = v.x*mat.m[1][0] + v.y*mat.m[1][1]; }

#define TRANSFORM2DIR(v, mat) { \
ITEVector2 temp; TRANSFORM2DIRTO(v, mat, temp); v = temp; }


/*--------------------------------------------------------
 * Additional functions
 *--------------------------------------------------------- */

ITEint iteInvertMatrix(ITEMatrix3x3 *m, ITEMatrix3x3 *mout);

ITEfloat iteVectorOrientation(ITEVector2 *v);

int iteLineLineXsection(ITEVector2 *o1, ITEVector2 *v1,
                       ITEVector2 *o2, ITEVector2 *v2,
                       ITEVector2 *xsection);

#endif/* __ITEVECTORS_H */
