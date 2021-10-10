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
#include "m2d/iteM2dVectors.h"

#define _ITEM_T  ITEM2DVector2
#define _ARRAY_T ITEM2DVector2Array
#define _FUNC_T  iteM2dVector2Array
#define _COMPARE_T(v1, v2) EQ2V(v1, v2)
#define _ARRAY_DEFINE
#include "m2d/iteM2dArrayBase.h"

void ITEM2DVector2_ctor(ITEM2DVector2 *v)
{
    v->x = 0.0f; v->y = 0.0f;
}

void ITEM2DVector2_dtor(ITEM2DVector2 *v)
{
}

void ITEM2DVector3_ctor(ITEM2DVector3 *v)
{
    v->x = 0.0f; v->y = 0.0f; v->z = 0.0f;
}

void ITEM2DVector3_dtor(ITEM2DVector3 *v)
{
}

void ITEM2DVector4_ctor(ITEM2DVector4 *v)
{
    v->x = 0.0f; v->y = 0.0f; v->z = 0.0f; v->w = 0.0f;
}

void ITEM2DVector4_dtor(ITEM2DVector4 *v)
{
}

void ITEM2DRectangle_ctor(ITEM2DRectangle *r)
{
    r->x = 0;
    r->y = 0;
    r->w = 0;
    r->h = 0;
}

void ITEM2DRectangle_dtor(ITEM2DRectangle *r)
{
}

void
iteM2dRectangleSet(
    ITEM2DRectangle *r,
    ITEM2Dfloat     x,
    ITEM2Dfloat     y,
    ITEM2Dfloat     w,
    ITEM2Dfloat     h)
{
    r->x = *(ITEM2Dint *)&x;
    r->y = *(ITEM2Dint *)&y;
    r->w = *(ITEM2Dint *)&w;
    r->h = *(ITEM2Dint *)&h;
}

void ITEM2DMatrix3x3_ctor(ITEM2DMatrix3x3 *mt)
{
    IDMAT((*mt));
}

void ITEM2DMatrix3x3_dtor(ITEM2DMatrix3x3 *mt)
{
}

ITEM2Dint iteM2dInvertMatrix(ITEM2DMatrix3x3 *m, ITEM2DMatrix3x3 *mout)
{
    /* Calculate determinant */
    ITEM2Dfloat D0 = m->m[1][1] * m->m[2][2] - m->m[2][1] * m->m[1][2];
    ITEM2Dfloat D1 = m->m[2][0] * m->m[1][2] - m->m[1][0] * m->m[2][2];
    ITEM2Dfloat D2 = m->m[1][0] * m->m[2][1] - m->m[2][0] * m->m[1][1];
    ITEM2Dfloat D  = m->m[0][0] * D0 + m->m[0][1] * D1 + m->m[0][2] * D2;

    /* Check if singular */
    if (D == 0.0f)
        return 0;
    D             = 1.0f / D;

    /* Calculate inverse */
    mout->m[0][0] = D * D0;
    mout->m[1][0] = D * D1;
    mout->m[2][0] = D * D2;
    mout->m[0][1] = D * (m->m[2][1] * m->m[0][2] - m->m[0][1] * m->m[2][2]);
    mout->m[1][1] = D * (m->m[0][0] * m->m[2][2] - m->m[2][0] * m->m[0][2]);
    mout->m[2][1] = D * (m->m[2][0] * m->m[0][1] - m->m[0][0] * m->m[2][1]);
    mout->m[0][2] = D * (m->m[0][1] * m->m[1][2] - m->m[1][1] * m->m[0][2]);
    mout->m[1][2] = D * (m->m[1][0] * m->m[0][2] - m->m[0][0] * m->m[1][2]);
    mout->m[2][2] = D * (m->m[0][0] * m->m[1][1] - m->m[1][0] * m->m[0][1]);

    return 1;
}

ITEM2Dfloat iteM2dVectorOrientation(ITEM2DVector2 *v)
{
    ITEM2Dfloat norm = (ITEM2Dfloat)NORM2((*v));
    ITEM2Dfloat cosa = v->x / norm;
    ITEM2Dfloat sina = v->y / norm;
    return (ITEM2Dfloat)(sina >= 0 ? ITEM2D_ACOS(cosa) : 2 * PI - ITEM2D_ACOS(cosa));
}

int iteM2dLineLineXsection(
    ITEM2DVector2 *o1,
    ITEM2DVector2 *v1,
    ITEM2DVector2 *o2,
    ITEM2DVector2 *v2,
    ITEM2DVector2 *xsection)
{
    ITEM2Dfloat rightU = o2->x - o1->x;
    ITEM2Dfloat rightD = o2->y - o1->y;

    ITEM2Dfloat D      = v1->x * (-v2->y) - v1->y * (-v2->x);
    ITEM2Dfloat DX     = rightU * (-v2->y) - rightD * (-v2->x);
/*ITEfloat DY = v1.x   * rightD  - v1.y   * rightU;*/

    ITEM2Dfloat t1     = DX / D;

    if (D == 0.0f)
        return 0;

    xsection->x = o1->x + t1 * v1->x;
    xsection->y = o1->y + t1 * v1->y;
    return 1;
}