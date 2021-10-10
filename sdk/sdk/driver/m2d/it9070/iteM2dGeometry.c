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
#include "m2d/m2d_engine.h"
#include "m2d/iteM2dUtility.h"
#include "m2d/iteM2dPath.h"
#include "m2d/iteM2dVectors.h"

static int CmdIndex;

static int iteM2dAddVertex(
    ITEM2DPath   *p,
    ITEM2DVertex *v,
    ITEM2Dint    *contourStart)
{
    /* Assert contour was open */
    ITEM2D_ASSERT((*contourStart) >= 0);

    /* Check vertex limit */
    if (p->vertices.size >= ITEM2D_MAX_VERTICES)
        return 0;

    /* Add vertex to subdivision */
    iteM2dVertexArrayPushBackP(&p->vertices, v);

    /* Increment contour size. Its stored in
       the flags of first contour vertex */
    p->vertices.items[*contourStart].flags++;

    return 1;
}

static void iteM2dSubrecurseQuad(
    ITEM2DPath *p,
    ITEM2DQuad *quad,
    ITEM2Dint  *contourStart)
{
    ITEM2DVertex  v;
    ITEM2DVector2 mid, dif, c1, c2, c3;
    ITEM2DQuad    quads[ITEM2D_MAX_RECURSE_DEPTH];
    ITEM2DQuad    *q, *qleft, *qright;
    ITEM2Dint     qindex = 0;
    quads[0] = *quad;

    while (qindex >= 0)
    {
        q = &quads[qindex];

        /* Calculate distance of control point from its
           counterpart on the line between end points */
        SET2V(mid, q->p1); ADD2V(mid, q->p3); DIV2(mid, 2);
        SET2V(dif, q->p2); SUB2V(dif, mid); ABS2(dif);

        /* Cancel if the curve is flat enough */
        if (dif.x + dif.y <= 1.0f || qindex == ITEM2D_MAX_RECURSE_DEPTH - 1)
        {
            /* Add subdivision point */
            v.point = q->p3; v.flags = 0;
            if (qindex == 0)
                return;              /* Skip last point */
            if (!iteM2dAddVertex(p, &v, contourStart))
                return;
            --qindex;
        }
        else
        {
            /* Left recursion goes on top of stack! */
            qright = q; qleft = &quads[++qindex];

            /* Subdivide into 2 sub-curves */
            SET2V(c1, q->p1); ADD2V(c1, q->p2); DIV2(c1, 2);
            SET2V(c3, q->p2); ADD2V(c3, q->p3); DIV2(c3, 2);
            SET2V(c2, c1); ADD2V(c2, c3); DIV2(c2, 2);

            /* Add left recursion onto stack */
            qleft->p1  = q->p1;
            qleft->p2  = c1;
            qleft->p3  = c2;

            /* Add right recursion onto stack */
            qright->p1 = c2;
            qright->p2 = c3;
            qright->p3 = q->p3;
        }
    }
}

static void iteM2dSubrecurseCubic(
    ITEM2DPath  *p,
    ITEM2DCubic *cubic,
    ITEM2Dint   *contourStart)
{
    ITEM2DVertex  v;
    ITEM2Dfloat   dx1, dy1, dx2, dy2;
    ITEM2DVector2 mm, c1, c2, c3, c4, c5;
    ITEM2DCubic   cubics[ITEM2D_MAX_RECURSE_DEPTH];
    ITEM2DCubic   *c, *cleft, *cright;
    ITEM2Dint     cindex = 0;
    cubics[0] = *cubic;

    while (cindex >= 0)
    {
        c   = &cubics[cindex];

        /* Calculate distance of control points from their
           counterparts on the line between end points */
        dx1 = 3.0f * c->p2.x - 2.0f * c->p1.x - c->p4.x; dx1 *= dx1;
        dy1 = 3.0f * c->p2.y - 2.0f * c->p1.y - c->p4.y; dy1 *= dy1;
        dx2 = 3.0f * c->p3.x - 2.0f * c->p4.x - c->p1.x; dx2 *= dx2;
        dy2 = 3.0f * c->p3.y - 2.0f * c->p4.y - c->p1.y; dy2 *= dy2;
        if (dx1 < dx2)
            dx1 = dx2;
        if (dy1 < dy2)
            dy1 = dy2;

        /* Cancel if the curve is flat enough */
        if (dx1 + dy1 <= 1.0 || cindex == ITEM2D_MAX_RECURSE_DEPTH - 1)
        {
            /* Add subdivision point */
            v.point = c->p4; v.flags = 0;
            if (cindex == 0)
                return;              /* Skip last point */
            if (!iteM2dAddVertex(p, &v, contourStart))
                return;
            --cindex;
        }
        else
        {
            /* Left recursion goes on top of stack! */
            cright = c; cleft = &cubics[++cindex];

            /* Subdivide into 2 sub-curves */
            SET2V(c1, c->p1); ADD2V(c1, c->p2); DIV2(c1, 2);
            SET2V(mm, c->p2); ADD2V(mm, c->p3); DIV2(mm, 2);
            SET2V(c5, c->p3); ADD2V(c5, c->p4); DIV2(c5, 2);

            SET2V(c2, c1); ADD2V(c2, mm); DIV2(c2, 2);
            SET2V(c4, mm); ADD2V(c4, c5); DIV2(c4, 2);

            SET2V(c3, c2); ADD2V(c3, c4); DIV2(c3, 2);

            /* Add left recursion to stack */
            cleft->p1  = c->p1;
            cleft->p2  = c1;
            cleft->p3  = c2;
            cleft->p4  = c3;

            /* Add right recursion to stack */
            cright->p1 = c3;
            cright->p2 = c4;
            cright->p3 = c5;
            cright->p4 = c->p4;
        }
    }
}

static void iteM2dSubrecurseArc(
    ITEM2DPath    *p,
    ITEM2DArc     *arc,
    ITEM2DVector2 *c,
    ITEM2DVector2 *ux,
    ITEM2DVector2 *uy,
    ITEM2Dint     *contourStart)
{
    ITEM2DVertex  v;
    ITEM2Dfloat   am, cosa, sina, dx, dy;
    ITEM2DVector2 uux, uuy, c1, m;
    ITEM2DArc     arcs[ITEM2D_MAX_RECURSE_DEPTH];
    ITEM2DArc     *a, *aleft, *aright;
    ITEM2Dint     aindex = 0;
    arcs[0] = *arc;

    while (aindex >= 0)
    {
        a    = &arcs[aindex];

        /* Middle angle and its cos/sin */
        am   = (a->a1 + a->a2) / 2;
        cosa = ITEM2D_COS(am);
        sina = ITEM2D_SIN(am);

        /* New point */
        SET2V(uux, (*ux)); MUL2(uux, cosa);
        SET2V(uuy, (*uy)); MUL2(uuy, sina);
        SET2V(c1, (*c)); ADD2V(c1, uux); ADD2V(c1, uuy);

        /* Check distance from linear midpoint */
        SET2V(m, a->p1); ADD2V(m, a->p2); DIV2(m, 2);
        dx = c1.x - m.x; dy = c1.y - m.y;
        if (dx < 0.0f)
            dx = -dx;
        if (dy < 0.0f)
            dy = -dy;

        /* Stop if flat enough */
        if (dx + dy <= 1.0f || aindex == ITEM2D_MAX_RECURSE_DEPTH - 1)
        {
            /* Add middle subdivision point */
            v.point = c1; v.flags = 0;
            if (!iteM2dAddVertex(p, &v, contourStart))
                return;
            if (aindex == 0)
                return;              /* Skip very last point */

            /* Add end subdivision point */
            v.point = a->p2; v.flags = 0;
            if (!iteM2dAddVertex(p, &v, contourStart))
                return;
            --aindex;
        }
        else
        {
            /* Left subdivision goes on top of stack! */
            aright     = a; aleft = &arcs[++aindex];

            /* Add left recursion to stack */
            aleft->p1  = a->p1;
            aleft->a1  = a->a1;
            aleft->p2  = c1;
            aleft->a2  = am;

            /* Add right recursion to stack */
            aright->p1 = c1;
            aright->a1 = am;
            aright->p2 = a->p2;
            aright->a2 = a->a2;
        }
    }
}

static void iteM2dSubdivideSegment(
    ITEM2DPath       *p,
    M2DVGPathSegment segment,
    M2DVGPathCommand originalCommand,
    ITEM2Dfloat      *data,
    void             *userData)
{
    ITEM2DVertex    v;
    ITEM2Dint       *contourStart = ((ITEM2Dint **)userData)[0];
    ITEM2Dint       *surfaceSpace = ((ITEM2Dint **)userData)[1];
    ITEM2DQuad      quad; ITEM2DCubic cubic; ITEM2DArc arc;
    ITEM2DVector2   c, ux, uy;

    ITEM2DMatrix3x3 ctm;

    /* Get current transform matrix */
    M2DSETMAT(ctm,
              1.0f, 0, 0,
              0, 1.0f, 0,
              0, 0, 1.0f);

    switch (segment)
    {
    case M2DVG_MOVE_TO:
        /* Set contour start here */
        (*contourStart) = p->vertices.size;

        /* First contour vertex */
        v.point.x       = data[2];
        v.point.y       = data[3];
        v.flags         = 0;
        if (*surfaceSpace)
            TRANSFORM2(v.point, ctm);
        break;

    case M2DVG_CLOSE_PATH:
        /* Last contour vertex */
        v.point.x = data[2];
        v.point.y = data[3];
        v.flags   = ITEM2D_VERTEX_FLAG_SEGEND | ITEM2D_VERTEX_FLAG_CLOSE;
        if (*surfaceSpace)
            TRANSFORM2(v.point, ctm);
        break;

    case M2DVG_LINE_TO:
        /* Last segment vertex */
        v.point.x = data[2];
        v.point.y = data[3];
        v.flags   = ITEM2D_VERTEX_FLAG_SEGEND;
        if (*surfaceSpace)
            TRANSFORM2(v.point, ctm);
        break;

    case M2DVG_QUAD_TO:
        /* Recurse subdivision */
        SET2(quad.p1, data[0], data[1]);
        SET2(quad.p2, data[2], data[3]);
        SET2(quad.p3, data[4], data[5]);
        if (*surfaceSpace)
        {
            TRANSFORM2(quad.p1, ctm);
            TRANSFORM2(quad.p2, ctm);
            TRANSFORM2(quad.p3, ctm);
        }
        iteM2dSubrecurseQuad(p, &quad, contourStart);

        /* Last segment vertex */
        v.point.x = data[4];
        v.point.y = data[5];
        v.flags   = ITEM2D_VERTEX_FLAG_SEGEND;
        if (*surfaceSpace)
            TRANSFORM2(v.point, ctm);
        break;

    case M2DVG_CUBIC_TO:
        /* Recurse subdivision */
        SET2(cubic.p1, data[0], data[1]);
        SET2(cubic.p2, data[2], data[3]);
        SET2(cubic.p3, data[4], data[5]);
        SET2(cubic.p4, data[6], data[7]);
        if (*surfaceSpace)
        {
            TRANSFORM2(cubic.p1, ctm);
            TRANSFORM2(cubic.p2, ctm);
            TRANSFORM2(cubic.p3, ctm);
            TRANSFORM2(cubic.p4, ctm);
        }
        iteM2dSubrecurseCubic(p, &cubic, contourStart);

        /* Last segment vertex */
        v.point.x = data[6];
        v.point.y = data[7];
        v.flags   = ITEM2D_VERTEX_FLAG_SEGEND;
        if (*surfaceSpace)
            TRANSFORM2(v.point, ctm);
        break;

    default:
        ITEM2D_ASSERT(segment == M2DVG_SCWARC_TO || segment == M2DVG_SCCWARC_TO ||
                      segment == M2DVG_LCWARC_TO || segment == M2DVG_LCCWARC_TO);

        /* Recurse subdivision */
        SET2(arc.p1, data[0], data[1]);
        SET2(arc.p2, data[10], data[11]);
        arc.a1 = data[8]; arc.a2 = data[9];
        SET2(c, data[2], data[3]);
        SET2(ux, data[4], data[5]);
        SET2(uy, data[6], data[7]);
        if (*surfaceSpace)
        {
            TRANSFORM2(arc.p1, ctm);
            TRANSFORM2(arc.p2, ctm);
            TRANSFORM2(c, ctm);
            TRANSFORM2DIR(ux, ctm);
            TRANSFORM2DIR(uy, ctm);
        }
        iteM2dSubrecurseArc(p, &arc, &c, &ux, &uy, contourStart);

        /* Last segment vertex */
        v.point.x = data[10];
        v.point.y = data[11];
        v.flags   = ITEM2D_VERTEX_FLAG_SEGEND;
        if (*surfaceSpace)
        {
            TRANSFORM2(v.point, ctm);
        }
        break;
    }

    /* Add subdivision vertex */
    iteM2dAddVertex(p, &v, contourStart);
}

static void
iteM2dGenCommand(
    ITEM2DPath       *p,
    M2DVGPathSegment segment,
    M2DVGPathCommand originalCommand,
    ITEM2Dfloat      *data,
    void             *userData)
{
    //	ITEuint16 lines = 4;
    ITEM2Duint16 lines = 8;
    //	ITEuint16 lines = 16;
    //	ITEuint16 lines = 32;
    //	ITEuint16 lines = 64;
    //	ITEuint16 lines = 128;
    //	ITEuint16 lines = 256;
    //	ITEuint16 lines = 1024;
    //	ITEuint16 lines = 2048;

    ITEM2Dint              *contourStart  = ((ITEM2Dint **)userData)[0];
    ITEM2DPathCommandArray *pPathCmdArray = ((ITEM2DPathCommandArray **)userData)[2];
    ITEM2Dfloat            rot, sinrot, cosrot, x0p, y0p, x1p, y1p;

    switch (segment)
    {
    case M2DVG_MOVE_TO:
        //h->tessellateCmd[(*contourStart)++] = 0x80000000 | lines;
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80000000 | lines);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[2] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[3] * 0x800));
        break;

    case M2DVG_CLOSE_PATH:
        //h->tessellateCmd[(*contourStart)++] = 0x80080000 | lines;
        iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80080000 | lines);
        break;

    case M2DVG_LINE_TO:
        //h->tessellateCmd[(*contourStart)++] = 0x80010000 | lines;
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80010000 | lines);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[2] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[3] * 0x800));
        break;

    case M2DVG_QUAD_TO:
        //h->tessellateCmd[(*contourStart)++] = 0x80020000 | lines;
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[4]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[5]*0x800);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80020000 | lines);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[2] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[3] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[4] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[5] * 0x800));
        break;

    case M2DVG_CUBIC_TO:
        //h->tessellateCmd[(*contourStart)++] = 0x80030000 | lines;
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[4]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[5]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[6]*0x800);
        //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[7]*0x800);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80030000 | lines);
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[2] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[3] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[4] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[5] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[6] * 0x800));
        iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[7] * 0x800));
        break;

    default:
        {
            ITEM2Dboolean simapleLineTo = ITEM2D_FALSE;

            ITEM2D_ASSERT(segment == M2DVG_SCWARC_TO || segment == M2DVG_SCCWARC_TO || segment == M2DVG_LCWARC_TO || segment == M2DVG_LCCWARC_TO);

            if (data[2] == 0 || data[3] == 0 || (data[0] == data[5] && data[1] == data[6]))
            {
                simapleLineTo = ITEM2D_TRUE;

                //h->tessellateCmd[(*contourStart)++] = 0x80010000;
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[5]*0x800);
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[6]*0x800);
                iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80010000);
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[5] * 0x800));
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[6] * 0x800));
            }
            else if (segment == M2DVG_SCCWARC_TO)
            {
                //h->tessellateCmd[(*contourStart)++] = 0x80040000 | lines;
                iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80040000 | lines);
            }
            else if (segment == M2DVG_SCWARC_TO)
            {
                //h->tessellateCmd[(*contourStart)++] = 0x80050000 | lines;
                iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80050000 | lines);
            }
            else if (segment == M2DVG_LCCWARC_TO)
            {
                //h->tessellateCmd[(*contourStart)++] = 0x80060000 | lines;
                iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80060000 | lines);
            }
            else //VG_LCWARC_TO
            {
                //h->tessellateCmd[(*contourStart)++] = 0x80070000 | lines;
                iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80070000 | lines);
            }

            if (simapleLineTo == ITEM2D_FALSE)
            {
                float dsq = 0.0f;
                float rh  = 0.0f;
                float rv  = 0.0f;

                rot    = ITEM2D_DEG2RAD(data[4]);
                sinrot = ITEM2D_SIN(rot);
                cosrot = ITEM2D_COS(rot);

                rh     = data[2];
                rv     = data[3];

                x0p    = (data[0] * cosrot + data[1] * sinrot) / rh;  // x0p = (x0*cos(rot) + y0*sin(rot))/rh;
                y0p    = (-data[0] * sinrot + data[1] * cosrot) / rv; // y0p = (-x0*sin(rot) + y0*cos(rot))/rv;
                x1p    = (data[5] * cosrot + data[6] * sinrot) / rh;  // x1p = (x1*cos(rot) + y1*sin(rot))/rh;
                y1p    = (-data[5] * sinrot + data[6] * cosrot) / rv; // y1p = (-x1*sin(rot) + y1*cos(rot))/rv;

                dsq    = (x0p - x1p) * (x0p - x1p) + (y0p - y1p) * (y0p - y1p);

                if (dsq > 4)
                {
                    float l = ITEM2D_SQRT(dsq);
                    rh *= 0.5f * l;
                    rv *= 0.5f * l;

                    x0p = (data[0] * cosrot + data[1] * sinrot) / rh;  // x0p = (x0*cos(rot) + y0*sin(rot))/rh;
                    y0p = (-data[0] * sinrot + data[1] * cosrot) / rv; // y0p = (-x0*sin(rot) + y0*cos(rot))/rv;
                    x1p = (data[5] * cosrot + data[6] * sinrot) / rh;  // x1p = (x1*cos(rot) + y1*sin(rot))/rh;
                    y1p = (-data[5] * sinrot + data[6] * cosrot) / rv; // y1p = (-x1*sin(rot) + y1*cos(rot))/rv;
                }

                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[5]*0x800);
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[6]*0x800);
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(x0p*0x800);					// x0p
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(y0p*0x800);					// y0p
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(x1p*0x800);					// x1p
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(y1p*0x800);					// y1p
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*cosrot*0x800);		// rh*cos(rot)
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(-data[3]*sinrot*0x800);		// -rv*sin(rot)
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*sinrot*0x800);		// rh*sin(rot)
                //h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*cosrot*0x800);		// rv*cos(rot)
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[5] * 0x800));
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(data[6] * 0x800));
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(x0p * 0x800));          // x0p
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(y0p * 0x800));          // y0p
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(x1p * 0x800));          // x1p
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(y1p * 0x800));          // y1p
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(rh * cosrot * 0x800));  // rh*cos(rot)
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(-rv * sinrot * 0x800)); // -rv*sin(rot)
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(rh * sinrot * 0x800));  // rh*sin(rot)
                iteM2dPathCommandArrayPushBack(pPathCmdArray, (ITEM2Ds15p16)(rv * cosrot * 0x800));  // rv*cos(rot)
            }
        }
        break;
    }
}

/*-------------------------------------------
 * Adds a rectangle to the path's stroke.
 *-------------------------------------------*/

static void
iteM2dPushStrokeQuad(
    ITEM2DPath    *p,
    ITEM2DVector2 *p1,
    ITEM2DVector2 *p2,
    ITEM2DVector2 *p3,
    ITEM2DVector2 *p4,
    ITEM2Duint    *cmdData)
{
    iteM2dVector2ArrayPushBackP(&p->stroke, p1);
    iteM2dVector2ArrayPushBackP(&p->stroke, p2);
    iteM2dVector2ArrayPushBackP(&p->stroke, p3);
    iteM2dVector2ArrayPushBackP(&p->stroke, p3);
    iteM2dVector2ArrayPushBackP(&p->stroke, p4);
    iteM2dVector2ArrayPushBackP(&p->stroke, p1);

    cmdData[CmdIndex++] = ((ITEM2Duint)(p1->x * 8) & 0xffff) | ((ITEM2Duint)(p1->y * 8) << 16);
    MIN2V(p->min, (*p1));
    MAX2V(p->max, (*p1));

    cmdData[CmdIndex++] = ((ITEM2Duint)(p2->x * 8) & 0xffff) | ((ITEM2Duint)(p2->y * 8) << 16);
    MIN2V(p->min, (*p2));
    MAX2V(p->max, (*p2));

    cmdData[CmdIndex++] = ((ITEM2Duint)(p3->x * 8) & 0xffff) | ((ITEM2Duint)(p3->y * 8) << 16);
    MIN2V(p->min, (*p3));
    MAX2V(p->max, (*p3));

    cmdData[CmdIndex++] = ((ITEM2Duint)(p3->x * 8) & 0xffff) | ((ITEM2Duint)(p3->y * 8) << 16);
    MIN2V(p->min, (*p3));
    MAX2V(p->max, (*p3));

    cmdData[CmdIndex++] = ((ITEM2Duint)(p4->x * 8) & 0xffff) | ((ITEM2Duint)(p4->y * 8) << 16);
    MIN2V(p->min, (*p4));
    MAX2V(p->max, (*p4));

    cmdData[CmdIndex++] = ((ITEM2Duint)(p1->x * 8) & 0xffff) | ((ITEM2Duint)(p1->y * 8) << 16);
    MIN2V(p->min, (*p1));
    MAX2V(p->max, (*p1));
}

/*-------------------------------------------
 * Adds a triangle to the path's stroke.
 *-------------------------------------------*/
static void
iteM2dPushStrokeTri(
    ITEM2DPath    *p,
    ITEM2DVector2 *p1,
    ITEM2DVector2 *p2,
    ITEM2DVector2 *p3,
    ITEM2Duint    *cmdData)
{
    iteM2dVector2ArrayPushBackP(&p->stroke, p1);
    iteM2dVector2ArrayPushBackP(&p->stroke, p2);
    iteM2dVector2ArrayPushBackP(&p->stroke, p3);

    cmdData[CmdIndex++] = ((ITEM2Duint)(p1->x * 8) & 0xffff) | ((ITEM2Duint)(p1->y * 8) << 16);
    MIN2V(p->min, (*p1));
    MAX2V(p->max, (*p1));

    cmdData[CmdIndex++] = ((ITEM2Duint)(p2->x * 8) & 0xffff) | ((ITEM2Duint)(p2->y * 8) << 16);
    MIN2V(p->min, (*p2));
    MAX2V(p->max, (*p2));

    cmdData[CmdIndex++] = ((ITEM2Duint)(p3->x * 8) & 0xffff) | ((ITEM2Duint)(p3->y * 8) << 16);
    MIN2V(p->min, (*p3));
    MAX2V(p->max, (*p3));
}

/*-----------------------------------------------------------
 * Adds a miter join to the path's stroke at the given
 * turn point [c], with the end of the previous segment
 * outset [o1] and the beginning of the next segment
 * outset [o2], transiting from tangent [d1] to [d2].
 *-----------------------------------------------------------*/
static void iteM2dStrokeJoinMiter(
    ITEM2DPath    *p,
    ITEM2DVector2 *c,
    ITEM2DVector2 *o1,
    ITEM2DVector2 *d1,
    ITEM2DVector2 *o2,
    ITEM2DVector2 *d2,
    ITEM2Duint    *cmdData)
{
    /* Init miter top to first point in case lines are colinear */
    ITEM2DVector2 x; SET2V(x, (*o1));

    /* Find intersection of two outer turn edges
       (lines defined by origin and direction) */
    iteM2dLineLineXsection(o1, d1, o2, d2, &x);

    /* Add a "diamond" quad with top on intersected point
       and bottom on center of turn (on the line) */
    iteM2dPushStrokeQuad(p, &x, o1, c, o2, cmdData);
}

/*-----------------------------------------------------------
 * Adds a round join to the path's stroke at the given
 * turn point [c], with the end of the previous segment
 * outset [pstart] and the beginning of the next segment
 * outset [pend], transiting from perpendicular vector
 * [tstart] to [tend].
 *-----------------------------------------------------------*/
static void iteM2dStrokeJoinRound(
    ITEM2DPath    *p,
    ITEM2DVector2 *c,
    ITEM2DVector2 *pstart,
    ITEM2DVector2 *tstart,
    ITEM2DVector2 *pend,
    ITEM2DVector2 *tend,
    ITEM2Duint    *cmdData)
{
    ITEM2DVector2 p1, p2;
    ITEM2Dfloat   a, ang, cosa, sina;

    /* Find angle between lines */
    ang = ANGLE2((*tstart), (*tend));

    /* Begin with start point */
    SET2V(p1, (*pstart));
    for (a = 0.0f; a < ang; a += PI / 12)
    {
        /* Rotate perpendicular vector around and
           find next offset point from center */
        cosa = ITEM2D_COS(-a);
        sina = ITEM2D_SIN(-a);
        SET2(p2, tstart->x * cosa - tstart->y * sina,
             tstart->x * sina + tstart->y * cosa);
        ADD2V(p2, (*c));

        /* Add triangle, save previous */
        iteM2dPushStrokeTri(p, &p1, &p2, c, cmdData);
        SET2V(p1, p2);
    }

    /* Add last triangle */
    iteM2dPushStrokeTri(p, &p1, pend, c, cmdData);
}

static void iteM2dStrokeCapRound(
    ITEM2DPath    *p,
    ITEM2DVector2 *c,
    ITEM2DVector2 *t,
    ITEM2Dint     start,
    ITEM2Duint    *cmdData)
{
    ITEM2Dint     a;
    ITEM2Dfloat   ang, cosa, sina;
    ITEM2DVector2 p1, p2;
    ITEM2Dint     steps = 12;
    ITEM2DVector2 tt;

    /* Revert perpendicular vector if start cap */
    SET2V(tt, (*t));
    if (start)
        MUL2(tt, -1);

    /* Find start point */
    SET2V(p1, (*c));
    ADD2V(p1, tt);

    for (a = 1; a <= steps; ++a)
    {
        /* Rotate perpendicular vector around and
           find next offset point from center */
        ang  = (ITEM2Dfloat)a * PI / steps;
        cosa = ITEM2D_COS(-ang);
        sina = ITEM2D_SIN(-ang);
        SET2(p2, tt.x * cosa - tt.y * sina,
             tt.x * sina + tt.y * cosa);
        ADD2V(p2, (*c));

        /* Add triangle, save previous */
        iteM2dPushStrokeTri(p, &p1, &p2, c, cmdData);
        SET2V(p1, p2);
    }
}

static void iteM2dStrokeCapSquare(
    ITEM2DPath    *p,
    ITEM2DVector2 *c,
    ITEM2DVector2 *t,
    ITEM2Dint     start,
    ITEM2Duint    *cmdData)
{
    ITEM2DVector2 tt, p1, p2, p3, p4;

    /* Revert perpendicular vector if start cap */
    SET2V(tt, (*t));
    if (start)
        MUL2(tt, -1);

    /* Find four corners of the quad */
    SET2V(p1, (*c));
    ADD2V(p1, tt);

    SET2V(p2, p1);
    ADD2(p2, tt.y, -tt.x);

    SET2V(p3, p2);
    ADD2(p3, -2 * tt.x, -2 * tt.y);

    SET2V(p4, p3);
    ADD2(p4, -tt.y, tt.x);

    iteM2dPushStrokeQuad(p, &p1, &p2, &p3, &p4, cmdData);
}

/*-----------------------------------------------------------
 * Finds the tight bounding box of a path defined by its
 * control points in path's own coordinate system.
 *-----------------------------------------------------------*/
static void iteM2dFindBoundbox(ITEM2DPath *p)
{
    int i;

    if (p->stroke.size == 0)
    {
        SET2(p->min, 0, 0);
        SET2(p->max, 0, 0);
        return;
    }
    p->min.x = p->max.x = p->stroke.items[0].x;
    p->min.y = p->max.y = p->stroke.items[0].y;

    for (i = 0; i < p->stroke.size; ++i)
    {
        MIN2V(p->min, p->stroke.items[i]);
        MAX2V(p->max, p->stroke.items[i]);
    }
}

/*--------------------------------------------------
 * Processes path data by simplfying it and sending
 * each segment to subdivision callback function
 *--------------------------------------------------*/
void
iteM2dFlattenPath(
    ITEM2DPath             *p,
    ITEM2Dint              surfaceSpace,
    ITEM2DPathCommandArray *pPathCmdArray)
{
    ITEM2Dint contourStart = 0;
    ITEM2Dint surfSpace    = surfaceSpace;
    ITEM2Dint *userData[3] = { 0 };
    ITEM2Dint processFlags = ITEM2D_PROCESS_SIMPLIFY_LINES | ITEM2D_PROCESS_SIMPLIFY_CURVES | ITEM2D_PROCESS_CENTRALIZE_ARCS | ITEM2D_PROCESS_REPAIR_ENDS;

    userData[0] = &contourStart;
    userData[1] = &surfaceSpace;
    userData[2] = (ITEM2Dint *)(ITEM2Duint32)pPathCmdArray;

    iteM2dProcessPathData(p, processFlags, iteM2dGenCommand, userData);
    iteM2dPathCommandArrayPushBack(pPathCmdArray, 0x80090000);  // list end
}