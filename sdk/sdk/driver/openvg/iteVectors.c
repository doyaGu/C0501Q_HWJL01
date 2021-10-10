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

#include "iteVectors.h"

#define _ITEM_T ITEVector2
#define _ARRAY_T ITEVector2Array
#define _FUNC_T iteVector2Array
#define _COMPARE_T(v1,v2) EQ2V(v1,v2)
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

void ITEVector2_ctor(ITEVector2 *v) {
  v->x=0.0f; v->y=0.0f;
}

void ITEVector2_dtor(ITEVector2 *v) {
}

void ITEVector3_ctor(ITEVector3 *v) {
  v->x=0.0f; v->y=0.0f; v->z=0.0f;
}

void ITEVector3_dtor(ITEVector3 *v) {
}

void ITEVector4_ctor(ITEVector4 *v) {
  v->x=0.0f; v->y=0.0f; v->z=0.0f; v->w=0.0f;
}

void ITEVector4_dtor(ITEVector4 *v) {
}

void ITERectangle_ctor(ITERectangle *r) 
{
	r->x = 0; 
	r->y = 0; 
	r->w = 0; 
	r->h = 0;
}

void ITERectangle_dtor(ITERectangle *r) 
{
}

void 
iteRectangleSet(
	ITERectangle*	r, 
	ITEfloat		x,
	ITEfloat		y, 
	ITEfloat		w, 
	ITEfloat		h)
{
	r->x = *(ITEint*)&x;
	r->y = *(ITEint*)&y;
	r->w = *(ITEint*)&w;
	r->h = *(ITEint*)&h;
}

void ITEMatrix3x3_ctor(ITEMatrix3x3 *mt)
{
  IDMAT((*mt));
}

void ITEMatrix3x3_dtor(ITEMatrix3x3 *mt)
{
}

int iteInvertMatrix(ITEMatrix3x3 *m, ITEMatrix3x3 *mout)
{
  /* Calculate determinant */
  ITEfloat D0 = m->m[1][1]*m->m[2][2] - m->m[2][1]*m->m[1][2];
  ITEfloat D1 = m->m[2][0]*m->m[1][2] - m->m[1][0]*m->m[2][2];
  ITEfloat D2 = m->m[1][0]*m->m[2][1] - m->m[2][0]*m->m[1][1]; 
  ITEfloat D = m->m[0][0]*D0 + m->m[0][1]*D1 + m->m[0][2]*D2;
  
  /* Check if singular */
  if( D == 0.0f ) return 0;
  D = 1.0f / D;
  
  /* Calculate inverse */
  mout->m[0][0] = D * D0;
  mout->m[1][0] = D * D1;
  mout->m[2][0] = D * D2;
  mout->m[0][1] = D * (m->m[2][1]*m->m[0][2] - m->m[0][1]*m->m[2][2]);
  mout->m[1][1] = D * (m->m[0][0]*m->m[2][2] - m->m[2][0]*m->m[0][2]);
  mout->m[2][1] = D * (m->m[2][0]*m->m[0][1] - m->m[0][0]*m->m[2][1]);
  mout->m[0][2] = D * (m->m[0][1]*m->m[1][2] - m->m[1][1]*m->m[0][2]);
  mout->m[1][2] = D * (m->m[1][0]*m->m[0][2] - m->m[0][0]*m->m[1][2]);
  mout->m[2][2] = D * (m->m[0][0]*m->m[1][1] - m->m[1][0]*m->m[0][1]);
  
  return 1;
}

ITEfloat iteVectorOrientation(ITEVector2 *v) {
  ITEfloat norm = (ITEfloat)NORM2((*v));
  ITEfloat cosa = v->x/norm;
  ITEfloat sina = v->y/norm;
  return (ITEfloat)(sina>=0 ? ITE_ACOS(cosa) : 2*PI-ITE_ACOS(cosa));
}

int iteLineLineXsection(ITEVector2 *o1, ITEVector2 *v1,
                       ITEVector2 *o2, ITEVector2 *v2,
                       ITEVector2 *xsection)
{
  ITEfloat rightU = o2->x - o1->x;
  ITEfloat rightD = o2->y - o1->y;
  
  ITEfloat D  = v1->x  * (-v2->y) - v1->y   * (-v2->x);
  ITEfloat DX = rightU * (-v2->y) - rightD * (-v2->x);
/*ITEfloat DY = v1.x   * rightD  - v1.y   * rightU;*/
  
  ITEfloat t1 = DX / D;
  
  if (D == 0.0f)
    return 0;
  
  xsection->x = o1->x + t1*v1->x;
  xsection->y = o1->y + t1*v1->y;
  return 1;
}
