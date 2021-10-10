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

#define VG_API_EXPORT
#include "openvg.h"
#include "iteContext.h"
#include "iteUtility.h"

static int CmdIndex;

static int iteAddVertex(ITEPath *p, ITEVertex *v, ITEint *contourStart)
{
  /* Assert contour was open */
  ITE_ASSERT((*contourStart) >= 0);

  /* Check vertex limit */
  if (p->vertices.size >= ITE_MAX_VERTICES) return 0;

  /* Add vertex to subdivision */
  iteVertexArrayPushBackP(&p->vertices, v);

  /* Increment contour size. Its stored in
     the flags of first contour vertex */
  p->vertices.items[*contourStart].flags++;

  return 1;
}

static void iteSubrecurseQuad(ITEPath *p, ITEQuad *quad, ITEint *contourStart)
{
  ITEVertex v;
  ITEVector2 mid, dif, c1, c2, c3;
  ITEQuad quads[ITE_MAX_RECURSE_DEPTH];
  ITEQuad *q, *qleft, *qright;
  ITEint qindex=0;
  quads[0] = *quad;

  while (qindex >= 0) {

    q = &quads[qindex];

    /* Calculate distance of control point from its
     counterpart on the line between end points */
    SET2V(mid, q->p1); ADD2V(mid, q->p3); DIV2(mid, 2);
    SET2V(dif, q->p2); SUB2V(dif, mid); ABS2(dif);

    /* Cancel if the curve is flat enough */
    if (dif.x + dif.y <= 1.0f || qindex == ITE_MAX_RECURSE_DEPTH-1) {

      /* Add subdivision point */
      v.point = q->p3; v.flags = 0;
      if (qindex == 0) return; /* Skip last point */
      if (!iteAddVertex(p, &v, contourStart)) return;
      --qindex;

    }else{

      /* Left recursion goes on top of stack! */
      qright = q; qleft = &quads[++qindex];

      /* Subdivide into 2 sub-curves */
      SET2V(c1, q->p1); ADD2V(c1, q->p2); DIV2(c1, 2);
      SET2V(c3, q->p2); ADD2V(c3, q->p3); DIV2(c3, 2);
      SET2V(c2, c1); ADD2V(c2, c3); DIV2(c2, 2);

      /* Add left recursion onto stack */
      qleft->p1 = q->p1;
      qleft->p2 = c1;
      qleft->p3 = c2;

      /* Add right recursion onto stack */
      qright->p1 = c2;
      qright->p2 = c3;
      qright->p3 = q->p3;
    }
  }
}

static void iteSubrecurseCubic(ITEPath *p, ITECubic *cubic, ITEint *contourStart)
{
  ITEVertex v;
  ITEfloat dx1, dy1, dx2, dy2;
  ITEVector2 mm, c1, c2, c3, c4, c5;
  ITECubic cubics[ITE_MAX_RECURSE_DEPTH];
  ITECubic *c, *cleft, *cright;
  ITEint cindex = 0;
  cubics[0] = *cubic;

  while (cindex >= 0) {

    c = &cubics[cindex];

    /* Calculate distance of control points from their
     counterparts on the line between end points */
    dx1 = 3.0f*c->p2.x - 2.0f*c->p1.x - c->p4.x; dx1 *= dx1;
    dy1 = 3.0f*c->p2.y - 2.0f*c->p1.y - c->p4.y; dy1 *= dy1;
    dx2 = 3.0f*c->p3.x - 2.0f*c->p4.x - c->p1.x; dx2 *= dx2;
    dy2 = 3.0f*c->p3.y - 2.0f*c->p4.y - c->p1.y; dy2 *= dy2;
    if (dx1 < dx2) dx1 = dx2;
    if (dy1 < dy2) dy1 = dy2;

    /* Cancel if the curve is flat enough */
    if (dx1+dy1 <= 1.0 || cindex == ITE_MAX_RECURSE_DEPTH-1) {

      /* Add subdivision point */
      v.point = c->p4; v.flags = 0;
      if (cindex == 0) return; /* Skip last point */
      if (!iteAddVertex(p, &v, contourStart)) return;
      --cindex;

    }else{

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
      cleft->p1 = c->p1;
      cleft->p2 = c1;
      cleft->p3 = c2;
      cleft->p4 = c3;

      /* Add right recursion to stack */
      cright->p1 = c3;
      cright->p2 = c4;
      cright->p3 = c5;
      cright->p4 = c->p4;
    }
  }
}

static void iteSubrecurseArc(ITEPath *p, ITEArc *arc,
                            ITEVector2 *c,ITEVector2 *ux, ITEVector2 *uy,
                            ITEint *contourStart)
{
  ITEVertex v;
  ITEfloat am, cosa, sina, dx, dy;
  ITEVector2 uux, uuy, c1, m;
  ITEArc arcs[ITE_MAX_RECURSE_DEPTH];
  ITEArc *a, *aleft, *aright;
  ITEint aindex=0;
  arcs[0] = *arc;

  while (aindex >= 0) {

    a = &arcs[aindex];

    /* Middle angle and its cos/sin */
    am = (a->a1 + a->a2)/2;
    cosa = ITE_COS(am);
    sina = ITE_SIN(am);

    /* New point */
    SET2V(uux, (*ux)); MUL2(uux, cosa);
    SET2V(uuy, (*uy)); MUL2(uuy, sina);
    SET2V(c1, (*c)); ADD2V(c1, uux); ADD2V(c1, uuy);

    /* Check distance from linear midpoint */
    SET2V(m, a->p1); ADD2V(m, a->p2); DIV2(m, 2);
    dx = c1.x - m.x; dy = c1.y - m.y;
    if (dx < 0.0f) dx = -dx;
    if (dy < 0.0f) dy = -dy;

    /* Stop if flat enough */
    if (dx+dy <= 1.0f || aindex == ITE_MAX_RECURSE_DEPTH-1) {

      /* Add middle subdivision point */
      v.point = c1; v.flags = 0;
      if (!iteAddVertex(p, &v, contourStart)) return;
      if (aindex == 0) return; /* Skip very last point */

      /* Add end subdivision point */
      v.point = a->p2; v.flags = 0;
      if (!iteAddVertex(p, &v, contourStart)) return;
      --aindex;

    }else{

      /* Left subdivision goes on top of stack! */
      aright = a; aleft = &arcs[++aindex];

      /* Add left recursion to stack */
      aleft->p1 = a->p1;
      aleft->a1 = a->a1;
      aleft->p2 = c1;
      aleft->a2 = am;

      /* Add right recursion to stack */
      aright->p1 = c1;
      aright->a1 = am;
      aright->p2 = a->p2;
      aright->a2 = a->a2;
    }
  }
}

static void iteSubdivideSegment(ITEPath *p, VGPathSegment segment,
                               VGPathCommand originalCommand,
                               ITEfloat *data, void *userData)
{
  ITEVertex v;
  ITEint *contourStart = ((ITEint**)userData)[0];
  ITEint *surfaceSpace = ((ITEint**)userData)[1];
  ITEQuad quad; ITECubic cubic; ITEArc arc;
  ITEVector2 c, ux, uy;
  VG_GETCONTEXT(VG_NO_RETVAL);

  switch (segment)
  {
  case VG_MOVE_TO:

    /* Set contour start here */
    (*contourStart) = p->vertices.size;

    /* First contour vertex */
    v.point.x = data[2];
    v.point.y = data[3];
    v.flags = 0;
    if (*surfaceSpace)
      TRANSFORM2(v.point, context->pathTransform);
    break;

  case VG_CLOSE_PATH:

    /* Last contour vertex */
    v.point.x = data[2];
    v.point.y = data[3];
    v.flags = ITE_VERTEX_FLAG_SEGEND | ITE_VERTEX_FLAG_CLOSE;
    if (*surfaceSpace)
      TRANSFORM2(v.point, context->pathTransform);
    break;

  case VG_LINE_TO:

    /* Last segment vertex */
    v.point.x = data[2];
    v.point.y = data[3];
    v.flags = ITE_VERTEX_FLAG_SEGEND;
    if (*surfaceSpace)
      TRANSFORM2(v.point, context->pathTransform);
    break;

  case VG_QUAD_TO:

    /* Recurse subdivision */
    SET2(quad.p1, data[0], data[1]);
    SET2(quad.p2, data[2], data[3]);
    SET2(quad.p3, data[4], data[5]);
    if (*surfaceSpace) {
      TRANSFORM2(quad.p1, context->pathTransform);
      TRANSFORM2(quad.p2, context->pathTransform);
      TRANSFORM2(quad.p3, context->pathTransform); }
    iteSubrecurseQuad(p, &quad, contourStart);

    /* Last segment vertex */
    v.point.x = data[4];
    v.point.y = data[5];
    v.flags = ITE_VERTEX_FLAG_SEGEND;
    if (*surfaceSpace)
      TRANSFORM2(v.point, context->pathTransform);
    break;

  case VG_CUBIC_TO:

    /* Recurse subdivision */
    SET2(cubic.p1, data[0], data[1]);
    SET2(cubic.p2, data[2], data[3]);
    SET2(cubic.p3, data[4], data[5]);
    SET2(cubic.p4, data[6], data[7]);
    if (*surfaceSpace) {
      TRANSFORM2(cubic.p1, context->pathTransform);
      TRANSFORM2(cubic.p2, context->pathTransform);
      TRANSFORM2(cubic.p3, context->pathTransform);
      TRANSFORM2(cubic.p4, context->pathTransform); }
    iteSubrecurseCubic(p, &cubic, contourStart);

    /* Last segment vertex */
    v.point.x = data[6];
    v.point.y = data[7];
    v.flags = ITE_VERTEX_FLAG_SEGEND;
    if (*surfaceSpace)
      TRANSFORM2(v.point, context->pathTransform);
    break;

  default:

    ITE_ASSERT(segment==VG_SCWARC_TO || segment==VG_SCCWARC_TO ||
              segment==VG_LCWARC_TO || segment==VG_LCCWARC_TO);

    /* Recurse subdivision */
    SET2(arc.p1, data[0], data[1]);
    SET2(arc.p2, data[10], data[11]);
    arc.a1 = data[8]; arc.a2 = data[9];
    SET2(c,  data[2], data[3]);
    SET2(ux, data[4], data[5]);
    SET2(uy, data[6], data[7]);
    if (*surfaceSpace) {
      TRANSFORM2(arc.p1, context->pathTransform);
      TRANSFORM2(arc.p2, context->pathTransform);
      TRANSFORM2(c, context->pathTransform);
      TRANSFORM2DIR(ux, context->pathTransform);
      TRANSFORM2DIR(uy, context->pathTransform); }
    iteSubrecurseArc(p, &arc, &c, &ux, &uy, contourStart);

    /* Last segment vertex */
    v.point.x = data[10];
    v.point.y = data[11];
    v.flags = ITE_VERTEX_FLAG_SEGEND;
    if (*surfaceSpace) {
      TRANSFORM2(v.point, context->pathTransform); }
    break;
  }

  /* Add subdivision vertex */
  iteAddVertex(p, &v, contourStart);
}

static void
iteGenCommand(
	ITEPath*      p,
	VGPathSegment segment,
	VGPathCommand originalCommand,
	ITEfloat*     data,
	void*         userData)
{
//	ITEuint16 lines = 4;
	ITEuint16 lines = 8;
//	ITEuint16 lines = 16;
//	ITEuint16 lines = 32;
//	ITEuint16 lines = 64;
//	ITEuint16 lines = 128;
//	ITEuint16 lines = 256;
//	ITEuint16 lines = 1024;
//	ITEuint16 lines = 2048;

	ITEint*              contourStart  = ((ITEint**)userData)[0];
	ITEPathCommandArray* pPathCmdArray = ((ITEPathCommandArray**)userData)[2];
	ITEHardwareRegister* h             = NULL;
	ITEfloat rot, sinrot, cosrot, x0p, y0p, x1p, y1p;
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = &context->hardware;

	switch (segment)
	{
	case VG_MOVE_TO:
		//h->tessellateCmd[(*contourStart)++] = 0x80000000 | lines;
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80000000 | lines));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[2]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[3]*0x800)));
		break;

	case VG_CLOSE_PATH:
		//h->tessellateCmd[(*contourStart)++] = 0x80080000 | lines;
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80080000 | lines));
		break;

	case VG_LINE_TO:
		//h->tessellateCmd[(*contourStart)++] = 0x80010000 | lines;
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80010000 | lines));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[2]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[3]*0x800)));
		break;

	case VG_QUAD_TO:
		//h->tessellateCmd[(*contourStart)++] = 0x80020000 | lines;
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[4]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[5]*0x800);
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80020000 | lines));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[2]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[3]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[4]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[5]*0x800)));
		break;

	case VG_CUBIC_TO:
		//h->tessellateCmd[(*contourStart)++] = 0x80030000 | lines;
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[2]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[3]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[4]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[5]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[6]*0x800);
		//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[7]*0x800);
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80030000 | lines));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[2]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[3]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[4]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[5]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[6]*0x800)));
		itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[7]*0x800)));
		break;

	default:
		{
			ITEboolean simapleLineTo = ITE_FALSE;

			ITE_ASSERT(segment==VG_SCWARC_TO || segment==VG_SCCWARC_TO || segment==VG_LCWARC_TO || segment==VG_LCCWARC_TO);

			if (data[2]==0 || data[3]==0 || (data[0]==data[5] && data[1]==data[6]) )
			{
				simapleLineTo = ITE_TRUE;

				//h->tessellateCmd[(*contourStart)++] = 0x80010000;
				//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[5]*0x800);
				//h->tessellateCmd[(*contourStart)++] = (ITEs15p16)(data[6]*0x800);
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80010000));
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[5]*0x800)));
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[6]*0x800)));
			}
			else if(segment==VG_SCCWARC_TO)
			{
				//h->tessellateCmd[(*contourStart)++] = 0x80040000 | lines;
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80040000 | lines));
			}
			else if(segment==VG_SCWARC_TO)
			{
				//h->tessellateCmd[(*contourStart)++] = 0x80050000 | lines;
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80050000 | lines));
			}
			else if(segment==VG_LCCWARC_TO)
			{
				//h->tessellateCmd[(*contourStart)++] = 0x80060000 | lines;
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80060000 | lines));
			}
			else //VG_LCWARC_TO
			{
				//h->tessellateCmd[(*contourStart)++] = 0x80070000 | lines;
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80070000 | lines));
			}

			if ( simapleLineTo == ITE_FALSE )
			{
				float dsq = 0.0f;
				float rh  = 0.0f;
				float rv  = 0.0f;

				rot = ITE_DEG2RAD(data[4]);
				sinrot = ITE_SIN(rot);
				cosrot = ITE_COS(rot);

				rh = data[2];
				rv = data[3];

				x0p = (data[0]*cosrot + data[1]*sinrot)/rh; 	// x0p = (x0*cos(rot) + y0*sin(rot))/rh;
				y0p = (-data[0]*sinrot + data[1]*cosrot)/rv; 	// y0p = (-x0*sin(rot) + y0*cos(rot))/rv;
				x1p = (data[5]*cosrot + data[6]*sinrot)/rh; 	// x1p = (x1*cos(rot) + y1*sin(rot))/rh;
				y1p = (-data[5]*sinrot + data[6]*cosrot)/rv; 	// y1p = (-x1*sin(rot) + y1*cos(rot))/rv;

				dsq = (x0p-x1p)*(x0p-x1p) + (y0p-y1p)*(y0p-y1p);

				if (dsq > 4)
				{
					float l = ITE_SQRT(dsq);
					rh *= 0.5f * l;
					rv *= 0.5f * l;

					x0p = (data[0]*cosrot + data[1]*sinrot)/rh; 	// x0p = (x0*cos(rot) + y0*sin(rot))/rh;
					y0p = (-data[0]*sinrot + data[1]*cosrot)/rv; 	// y0p = (-x0*sin(rot) + y0*cos(rot))/rv;
					x1p = (data[5]*cosrot + data[6]*sinrot)/rh; 	// x1p = (x1*cos(rot) + y1*sin(rot))/rh;
					y1p = (-data[5]*sinrot + data[6]*cosrot)/rv; 	// y1p = (-x1*sin(rot) + y1*cos(rot))/rv;
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
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[5]*0x800)));
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(data[6]*0x800)));
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(x0p*0x800)));				// x0p
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(y0p*0x800)));				// y0p
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(x1p*0x800)));				// x1p
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(y1p*0x800)));				// y1p
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(rh*cosrot*0x800)));		// rh*cos(rot)
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(-rv*sinrot*0x800)));	// -rv*sin(rot)
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(rh*sinrot*0x800)));		// rh*sin(rot)
				itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32((ITEs15p16)(rv*cosrot*0x800)));		// rv*cos(rot)
			}
		}
		break;
	}
}

/*-------------------------------------------
 * Adds a rectangle to the path's stroke.
 *-------------------------------------------*/

static void
itePushStrokeQuad(
	ITEPath*    p,
	ITEVector2* p1,
	ITEVector2* p2,
	ITEVector2* p3,
	ITEVector2* p4,
	ITEuint*    cmdData)
{
	iteVector2ArrayPushBackP(&p->stroke, p1);
	iteVector2ArrayPushBackP(&p->stroke, p2);
	iteVector2ArrayPushBackP(&p->stroke, p3);
	iteVector2ArrayPushBackP(&p->stroke, p3);
	iteVector2ArrayPushBackP(&p->stroke, p4);
	iteVector2ArrayPushBackP(&p->stroke, p1);

	cmdData[CmdIndex++] = ((ITEuint)(p1->x*8)&0xffff) | ((ITEuint)(p1->y*8)<<16);
	MIN2V(p->min, (*p1));
	MAX2V(p->max, (*p1));

	cmdData[CmdIndex++] = ((ITEuint)(p2->x*8)&0xffff) | ((ITEuint)(p2->y*8)<<16);
	MIN2V(p->min, (*p2));
	MAX2V(p->max, (*p2));

	cmdData[CmdIndex++] = ((ITEuint)(p3->x*8)&0xffff) | ((ITEuint)(p3->y*8)<<16);
	MIN2V(p->min, (*p3));
	MAX2V(p->max, (*p3));

	cmdData[CmdIndex++] = ((ITEuint)(p3->x*8)&0xffff) | ((ITEuint)(p3->y*8)<<16);
	MIN2V(p->min, (*p3));
	MAX2V(p->max, (*p3));

	cmdData[CmdIndex++] = ((ITEuint)(p4->x*8)&0xffff) | ((ITEuint)(p4->y*8)<<16);
	MIN2V(p->min, (*p4));
	MAX2V(p->max, (*p4));

	cmdData[CmdIndex++] = ((ITEuint)(p1->x*8)&0xffff) | ((ITEuint)(p1->y*8)<<16);
	MIN2V(p->min, (*p1));
	MAX2V(p->max, (*p1));

}

/*-------------------------------------------
 * Adds a triangle to the path's stroke.
 *-------------------------------------------*/

static void
itePushStrokeTri(
	ITEPath*    p,
	ITEVector2* p1,
	ITEVector2* p2,
	ITEVector2* p3,
	ITEuint*    cmdData)
{
	iteVector2ArrayPushBackP(&p->stroke, p1);
	iteVector2ArrayPushBackP(&p->stroke, p2);
	iteVector2ArrayPushBackP(&p->stroke, p3);

	cmdData[CmdIndex++] = ((ITEuint)(p1->x*8)&0xffff) | ((ITEuint)(p1->y*8)<<16);
	MIN2V(p->min, (*p1));
	MAX2V(p->max, (*p1));

	cmdData[CmdIndex++] = ((ITEuint)(p2->x*8)&0xffff) | ((ITEuint)(p2->y*8)<<16);
	MIN2V(p->min, (*p2));
	MAX2V(p->max, (*p2));

	cmdData[CmdIndex++] = ((ITEuint)(p3->x*8)&0xffff) | ((ITEuint)(p3->y*8)<<16);
	MIN2V(p->min, (*p3));
	MAX2V(p->max, (*p3));

}

/*-----------------------------------------------------------
 * Adds a miter join to the path's stroke at the given
 * turn point [c], with the end of the previous segment
 * outset [o1] and the beginning of the next segment
 * outset [o2], transiting from tangent [d1] to [d2].
 *-----------------------------------------------------------*/

static void iteStrokeJoinMiter(ITEPath *p, ITEVector2 *c,
                              ITEVector2 *o1, ITEVector2 *d1,
                              ITEVector2 *o2, ITEVector2 *d2, ITEuint* cmdData)
{
  /* Init miter top to first point in case lines are colinear */
  ITEVector2 x; SET2V(x,(*o1));

  /* Find intersection of two outer turn edges
     (lines defined by origin and direction) */
  iteLineLineXsection(o1, d1, o2, d2, &x);

  /* Add a "diamond" quad with top on intersected point
     and bottom on center of turn (on the line) */
  itePushStrokeQuad(p, &x, o1, c, o2, cmdData);
}

/*-----------------------------------------------------------
 * Adds a round join to the path's stroke at the given
 * turn point [c], with the end of the previous segment
 * outset [pstart] and the beginning of the next segment
 * outset [pend], transiting from perpendicular vector
 * [tstart] to [tend].
 *-----------------------------------------------------------*/

static void iteStrokeJoinRound(ITEPath *p, ITEVector2 *c,
                              ITEVector2 *pstart, ITEVector2 *tstart,
                              ITEVector2 *pend, ITEVector2 *tend, ITEuint* cmdData)
{
  ITEVector2 p1, p2;
  ITEfloat a, ang, cosa, sina;

  /* Find angle between lines */
   ang = ANGLE2((*tstart),(*tend));

  /* Begin with start point */
  SET2V(p1,(*pstart));
  for (a=0.0f; a<ang; a+=PI/12) {

    /* Rotate perpendicular vector around and
       find next offset point from center */
    cosa = ITE_COS(-a);
    sina = ITE_SIN(-a);
    SET2(p2, tstart->x*cosa - tstart->y*sina,
         tstart->x*sina + tstart->y*cosa);
    ADD2V(p2, (*c));

    /* Add triangle, save previous */
    itePushStrokeTri(p, &p1, &p2, c, cmdData);
    SET2V(p1, p2);
  }

  /* Add last triangle */
  itePushStrokeTri(p, &p1, pend, c, cmdData);
}

static void iteStrokeCapRound(ITEPath *p, ITEVector2 *c, ITEVector2 *t, ITEint start, ITEuint* cmdData)
{
  ITEint a;
  ITEfloat ang, cosa, sina;
  ITEVector2 p1, p2;
  ITEint steps = 12;
  ITEVector2 tt;

  /* Revert perpendicular vector if start cap */
  SET2V(tt, (*t));
  if (start) MUL2(tt, -1);

  /* Find start point */
  SET2V(p1, (*c));
  ADD2V(p1, tt);

  for (a = 1; a<=steps; ++a) {

    /* Rotate perpendicular vector around and
       find next offset point from center */
    ang = (ITEfloat)a * PI / steps;
    cosa = ITE_COS(-ang);
    sina = ITE_SIN(-ang);
    SET2(p2, tt.x*cosa - tt.y*sina,
         tt.x*sina + tt.y*cosa);
    ADD2V(p2, (*c));

    /* Add triangle, save previous */
    itePushStrokeTri(p, &p1, &p2, c, cmdData);
    SET2V(p1, p2);
  }
}

static void iteStrokeCapSquare(ITEPath *p, ITEVector2 *c, ITEVector2 *t, ITEint start, ITEuint* cmdData)
{
  ITEVector2 tt, p1, p2, p3, p4;

  /* Revert perpendicular vector if start cap */
  SET2V(tt, (*t));
  if (start) MUL2(tt, -1);

  /* Find four corners of the quad */
  SET2V(p1, (*c));
  ADD2V(p1, tt);

  SET2V(p2, p1);
  ADD2(p2, tt.y, -tt.x);

  SET2V(p3, p2);
  ADD2(p3, -2*tt.x, -2*tt.y);

  SET2V(p4, p3);
  ADD2(p4, -tt.y, tt.x);

  itePushStrokeQuad(p, &p1, &p2, &p3, &p4, cmdData);
}

/*-----------------------------------------------------------
 * Finds the tight bounding box of a path defined by its
 * control points in path's own coordinate system.
 *-----------------------------------------------------------*/

static void iteFindBoundbox(ITEPath *p)
{
	int i;

	if (p->stroke.size == 0) {
		SET2(p->min, 0,0);
		SET2(p->max, 0,0);
		return;
	}
	p->min.x = p->max.x = p->stroke.items[0].x;
	p->min.y = p->max.y = p->stroke.items[0].y;

	for (i=0; i<p->stroke.size; ++i) {
		MIN2V(p->min, p->stroke.items[i]);
		MAX2V(p->max, p->stroke.items[i]);
	}
}

/*--------------------------------------------------
 * Processes path data by simplfying it and sending
 * each segment to subdivision callback function
 *--------------------------------------------------*/

void
iteFlattenPath(
	ITEPath*             p,
	ITEint               surfaceSpace,
	ITEPathCommandArray* pPathCmdArray)
{
	ITEint               contourStart  = 0;
	ITEint               surfSpace     = surfaceSpace;
	ITEint*              userData[3]   = {0};
	ITEHardwareRegister* h             = NULL;
	ITEint               processFlags  = ITE_PROCESS_SIMPLIFY_LINES | ITE_PROCESS_SIMPLIFY_CURVES | ITE_PROCESS_CENTRALIZE_ARCS | ITE_PROCESS_REPAIR_ENDS;

	VG_GETCONTEXT(VG_NO_RETVAL);

	h = &context->hardware;

	userData[0] = &contourStart;
	userData[1] = &surfaceSpace;
	userData[2] = (ITEint*)(ITEuint32)pPathCmdArray;

	//iteVertexArrayClear(&p->vertices);
	//iteProcessPathData(p, processFlags, iteSubdivideSegment, userData);
	iteProcessPathData(p, processFlags, iteGenCommand, userData);
	//h->tessellateCmd[contourStart] = 0x80090000;  // list end
	itePathCommandArrayPushBack(pPathCmdArray, ivgByteSwap32(0x80090000));	// list end
    //h->cmdLength = contourStart + 1;
}

/*-----------------------------------------------------------
 * Generates stroke of a path according to VGContext state.
 * Produces quads for every linear subdivision segment or
 * dash "on" segment, handles line caps and joins.
 *-----------------------------------------------------------*/

void iteGenStrokePath(VGContext* c, ITEPath *p, ITEuint *cmdData)
{
  /* Line width and vertex count */
  ITEfloat w = c->strokeLineWidth / 2;
  ITEfloat mlimit = c->strokeMiterLimit;
  ITEint vertsize = p->vertices.size;

  /* Contour state */
  ITEint contourStart = 0;
  ITEint contourLength = 0;
  ITEint start = 0;
  ITEint end = 0;
  ITEint loop = 0;
  ITEint close = 0;
  ITEint segend = 0;

  /* Current vertices */
  ITEint i1=0, i2=0;
  ITEVertex *v1, *v2;
  ITEVector2 *p1, *p2;
  ITEVector2 d, t, dprev, tprev;
  ITEfloat norm, cross, mlength;

  /* Stroke edge points */
  ITEVector2 l1, r1, l2, r2, lprev, rprev;

  /* Dash state */
  ITEint dashIndex = 0;
  ITEfloat dashLength = 0.0f, strokeLength = 0.0f;
  ITEint dashSize = c->strokeDashPattern.size;
  ITEfloat *dashPattern = c->strokeDashPattern.items;
  ITEint dashOn = 1;

  /* Dash edge points */
  ITEVector2 dash1, dash2;
  ITEVector2 dashL1, dashR1;
  ITEVector2 dashL2, dashR2;
  ITEfloat nextDashLength, dashOffset;

  /* Discard odd dash segment */
  dashSize -= dashSize % 2;

  /* Init previous so compiler doesn't warn
     for uninitialized usage */
  SET2(tprev, 0,0); SET2(dprev, 0,0);
  SET2(lprev, 0,0); SET2(rprev, 0,0);

  CmdIndex = 0;
  cmdData[CmdIndex++] = 1;  // means triangle list
  cmdData[CmdIndex++] = 1;  // default one triangle list
  cmdData[CmdIndex++] = 0;  //total vertex number, initially 0;

  p->min.x = p->max.x = p->vertices.items[0].point.x;
  p->min.y = p->max.y = p->vertices.items[0].point.y;

  /* Walk over subdivision vertices */
  for (i1=0; i1<vertsize; ++i1) {

    if (loop) {
      /* Start new contour if exists */
      if (contourStart < vertsize)
        i1 = contourStart;
      else break;
    }

    start = end = loop = close = segend = 0;
    i2 = i1 + 1;

    if (i1 == contourStart) {
      /* Contour has started. Get length */
      contourLength = p->vertices.items[i1].flags;
      start = 1;
    }

    if (contourLength <= 1) {
      /* Discard empty contours. */
      contourStart = i1 + 1;
      loop = 1;
      continue;
    }

    v1 = &p->vertices.items[i1];
    v2 = &p->vertices.items[i2];

    if (i2 == contourStart + contourLength-1) {
      /* Contour has ended. Check close */
      close = v2->flags & ITE_VERTEX_FLAG_CLOSE;
      end = 1;
    }

    if (i1 == contourStart + contourLength-1) {
      /* Loop back to first edge. Check close */
      close = v1->flags & ITE_VERTEX_FLAG_CLOSE;
      i2 = contourStart+1;
      contourStart = i1 + 1;
      i1 = i2 - 1;
      loop = 1;
    }

    if (!start && !loop) {
      /* We are inside a contour. Check segment end. */
      segend = (v1->flags & ITE_VERTEX_FLAG_SEGEND);
    }

    if (dashSize > 0 && start &&
        (contourStart == 0 || c->strokeDashPhaseReset)) {

      /* Reset pattern phase at contour start */
      dashLength = -c->strokeDashPhase;
      strokeLength = 0.0f;
      dashIndex = 0;
      dashOn = 1;

      if (dashLength < 0.0f) {
        /* Consume dash segments forward to reach stroke start */
        while (dashLength + dashPattern[dashIndex] <= 0.0f) {
          dashLength += dashPattern[dashIndex];
          dashIndex = (dashIndex + 1) % dashSize;
          dashOn = !dashOn; }

      }else if (dashLength > 0.0f) {
        /* Consume dash segments backward to return to stroke start */
        dashIndex = dashSize;
        while (dashLength > 0.0f) {
          dashIndex = dashIndex ? (dashIndex-1) : (dashSize-1);
          dashLength -= dashPattern[dashIndex];
          dashOn = !dashOn; }
      }
    }

    /* Subdiv segment vertices and points */
    v1 = &p->vertices.items[i1];
    v2 = &p->vertices.items[i2];
    p1 = &v1->point;
    p2 = &v2->point;

    /* Direction vector */
    SET2(d, p2->x-p1->x, p2->y-p1->y);
    norm = NORM2(d);
    if (norm == 0.0f) d = dprev;
    else DIV2(d, norm);

    /* Perpendicular vector */
    SET2(t, -d.y, d.x);
    MUL2(t, w);
    cross = CROSS2(t,tprev);

    /* Left and right edge points */
    SET2V(l1, (*p1)); ADD2V(l1, t);
    SET2V(r1, (*p1)); SUB2V(r1, t);
    SET2V(l2, (*p2)); ADD2V(l2, t);
    SET2V(r2, (*p2)); SUB2V(r2, t);

    /* Check if join needed */
    if ((segend || (loop && close)) && dashOn) {

      switch (c->strokeJoinStyle) {
      case VG_JOIN_ROUND:

        /* Add a round join to stroke */
        if (cross >= 0.0f)
          iteStrokeJoinRound(p, p1, &lprev, &tprev, &l1, &t, cmdData);
        else{
          ITEVector2 _t, _tprev;
          SET2(_t, -t.x, -t.y);
          SET2(_tprev, -tprev.x, -tprev.y);
          iteStrokeJoinRound(p, p1,  &r1, &_t, &rprev, &_tprev, cmdData);
        }

        break;
      case VG_JOIN_MITER:

        /* Add a miter join to stroke */
        mlength = 1/ITE_COS((ANGLE2(t, tprev))/2);
        if (mlength <= mlimit) {
          if (cross > 0.0f)
            iteStrokeJoinMiter(p, p1, &lprev, &dprev, &l1, &d, cmdData);
          else if (cross < 0.0f)
            iteStrokeJoinMiter(p, p1, &rprev, &dprev, &r1, &d, cmdData);
          break;
        }/* Else fall down to bevel */

      case VG_JOIN_BEVEL:

        /* Add a bevel join to stroke */
        if (cross > 0.0f)
          itePushStrokeTri(p, &l1, &lprev, p1, cmdData);
        else if (cross < 0.0f)
          itePushStrokeTri(p, &r1, &rprev, p1, cmdData);

        break;
      }
    }else if (!start && !loop && dashOn) {

      /* Fill gap with previous (= bevel join) */
      if (cross > 0.0f)
        itePushStrokeTri(p, &l1, &lprev, p1, cmdData);
      else if (cross < 0.0f)
        itePushStrokeTri(p, &r1, &rprev, p1, cmdData);
    }


    /* Apply cap to start of a non-closed contour or
       if we are dashing and dash segment is on */
    if ((dashSize == 0 && loop && !close) ||
        (dashSize > 0 && start && dashOn)) {
      switch (c->strokeCapStyle) {
      case VG_CAP_ROUND:
        iteStrokeCapRound(p, p1, &t, 1, cmdData); break;
      case VG_CAP_SQUARE:
        iteStrokeCapSquare(p, p1, &t, 1, cmdData); break;
      default: break;
      }
    }

    if (loop)
      continue;

    /* Handle dashing */
    if (dashSize > 0) {

      /* Start with beginning of subdiv segment */
      SET2V(dash1, (*p1)); SET2V(dashL1, l1); SET2V(dashR1, r1);

      do {
        /* Interpolate point on the current subdiv segment */
        nextDashLength = dashLength + dashPattern[dashIndex];
        dashOffset = (nextDashLength - strokeLength) / norm;
        if (dashOffset > 1.0f) dashOffset = 1;
        SET2V(dash2, (*p2)); SUB2V(dash2, (*p1));
        MUL2(dash2, dashOffset); ADD2V(dash2, (*p1));

        /* Left and right edge points */
        SET2V(dashL2, dash2); ADD2V(dashL2, t);
        SET2V(dashR2, dash2); SUB2V(dashR2, t);

        /* Add quad for this dash segment */
        if (dashOn) itePushStrokeQuad(p, &dashL2, &dashL1, &dashR1, &dashR2, cmdData);

        /* Move to next dash segment if inside this subdiv segment */
        if (nextDashLength <= strokeLength + norm) {
          dashIndex = (dashIndex + 1) % dashSize;
          dashLength = nextDashLength;
          dashOn = !dashOn;
          SET2V(dash1, dash2);
          SET2V(dashL1, dashL2);
          SET2V(dashR1, dashR2);

          /* Apply cap to dash segment */
          switch (c->strokeCapStyle) {
          case VG_CAP_ROUND:
            iteStrokeCapRound(p, &dash1, &t, dashOn, cmdData); break;
          case VG_CAP_SQUARE:
            iteStrokeCapSquare(p, &dash1, &t, dashOn, cmdData); break;
          default: break;
          }
        }

        /* Consume dash segments until subdiv end met */
      } while (nextDashLength < strokeLength + norm);

    }else{

      /* Add quad for this line segment */
      itePushStrokeQuad(p, &r2, &r1, &l1, &l2, cmdData);
    }


    /* Apply cap to end of a non-closed contour or
       if we are dashing and dash segment is on */
    if ((dashSize == 0 && end && !close) ||
        (dashSize > 0 && end && dashOn)) {
      switch (c->strokeCapStyle) {
      case VG_CAP_ROUND:
        iteStrokeCapRound(p, p2, &t, 0, cmdData); break;
      case VG_CAP_SQUARE:
        iteStrokeCapSquare(p, p2, &t, 0, cmdData); break;
      default: break;
      }
    }

    /* Save previous edge */
    strokeLength += norm;
    SET2V(lprev, l2);
    SET2V(rprev, r2);
    dprev = d;
    tprev = t;
  }

  cmdData[2] = (CmdIndex-3);  //total vertex number;

}

/*-----------------------------------------------------------
 * Finds the tight bounding box of a path defined by its
 * control points in path's own coordinate system.
 *-----------------------------------------------------------*/

void iteGenFillPath(VGContext* c, ITEPath *p, ITEuint* cmdData)
{
	ITEuint i;
	ITEint start = 0;
	ITEint count = 0;
	ITEint index = 0;

	if (p->vertices.size == 0) {
		SET2(p->min, 0,0);
		SET2(p->max, 0,0);
		return;
	}

	cmdData[index++] = 0;				// means triangle fan
	cmdData[index++] = 0;				// total triangle fan num, initially 0

	p->min.x = p->max.x = p->vertices.items[0].point.x;
	p->min.y = p->max.y = p->vertices.items[0].point.y;

	while (start < p->vertices.size)
	{
		cmdData[index++] = p->vertices.items[start].flags; //vertex number in one triangle fan
		count ++;
		for(i=0;i<p->vertices.items[start].flags;i++)
		{
			cmdData[index++] = ((ITEuint)(p->vertices.items[start+i].point.x*8)&0xffff) |
							((ITEuint)(p->vertices.items[start+i].point.y*8)<<16);
			MIN2V(p->min, p->vertices.items[start+i].point);
			MAX2V(p->max, p->vertices.items[start+i].point);
		}
		start += p->vertices.items[start].flags;
	}
	cmdData[1] = count; // total triangle fan number

}

void iteGenRectanglePath(ITEVector2 min, ITEVector2 max, ITEuint* cmdData)
{
	cmdData[0] = 0;				// means triangle fan
	cmdData[1] = 1;				// total triangle fan num, initially 0
	cmdData[2] = 4;
	cmdData[3] = ((ITEuint)(min.x*8)&0xffff) | ((ITEuint)(min.y*8)<<16);
	cmdData[4] = ((ITEuint)(max.x*8)&0xffff) | ((ITEuint)(min.y*8)<<16);
	cmdData[5] = ((ITEuint)(max.x*8)&0xffff) | ((ITEuint)(max.y*8)<<16);
	cmdData[6] = ((ITEuint)(min.x*8)&0xffff) | ((ITEuint)(max.y*8)<<16);
}

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

/*-----------------------------------------------------------
 * Outputs a tight bounding box of a path defined by its
 * control points in path's own coordinate system.
 *-----------------------------------------------------------*/

VG_API_CALL void
vgPathBounds(
	VGPath   path,
	VGfloat* minX,
	VGfloat* minY,
	VGfloat* width,
	VGfloat* height)
{
	ITEPath *p = NULL;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if path is not a valid path handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, path),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if minX, minY, width, or height is NULL
		¡V if minX, minY, width, or height is not properly aligned */
	VG_RETURN_ERR_IF(minX == NULL || minY == NULL ||
	                 width == NULL || height == NULL,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	VG_RETURN_ERR_IF(!CheckAlignment(minX, sizeof(VGfloat)) ||
	                 !CheckAlignment(minY, sizeof(VGfloat)) ||
				     !CheckAlignment(width, sizeof(VGfloat)) ||
				     !CheckAlignment(height, sizeof(VGfloat)) ,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* VG_PATH_CAPABILITY_ERROR
		¡V if VG_PATH_CAPABILITY_PATH_BOUNDS is not enabled for path */
	p = (ITEPath*)path;
	VG_RETURN_ERR_IF(!(p->caps & VG_PATH_CAPABILITY_PATH_BOUNDS),
	                 VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);

	/* Update path geometry */
	//1 ToDo: Below code seems not needed !!!???
	//iteFlattenPath(p, 0);
	iteFindBoundbox(p);

	/* Output bounds */
	*minX = p->min.x;
	*minY = p->min.y;
	*width = p->max.x - p->min.x;
	*height = p->max.y - p->min.y;

	VG_RETURN(VG_NO_RETVAL);
}

/*------------------------------------------------------------
 * Outputs a bounding box of a path defined by its control
 * points that is guaranteed to enclose the path geometry
 * after applying the current path-user-to-surface transform
 *------------------------------------------------------------*/

VG_API_CALL void
vgPathTransformedBounds(
	VGPath   path,
	VGfloat* minX,
	VGfloat* minY,
	VGfloat* width,
	VGfloat* height)
{
	ITEPath *p = NULL;
	VG_GETCONTEXT(VG_NO_RETVAL);

	VG_RETURN_ERR_IF(!iteIsValidPath(context, path),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	VG_RETURN_ERR_IF(minX == NULL ||
		             minY == NULL ||
	                 width == NULL ||
					 height == NULL,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* TODO: check output pointer alignment */
	VG_RETURN_ERR_IF(!CheckAlignment(minX, sizeof(VGfloat)) ||
	                 !CheckAlignment(minY, sizeof(VGfloat)) ||
	                 !CheckAlignment(width, sizeof(VGfloat)) ||
	                 !CheckAlignment(height, sizeof(VGfloat)),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	p = (ITEPath*)path;
	VG_RETURN_ERR_IF(!(p->caps & VG_PATH_CAPABILITY_PATH_TRANSFORMED_BOUNDS),
	                 VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);

	/* Update path geometry */
	//1 ToDo: Below code seems not needed !!!???
	//iteFlattenPath(p, 1);
	iteFindBoundbox(p);

	/* Output bounds */
	*minX = p->min.x;
	*minY = p->min.y;
	*width = p->max.x - p->min.x;
	*height = p->max.y - p->min.y;

	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL VGfloat
vgPathLength(
	VGPath path,
	VGint  startSegment,
	VGint  numSegments)
{
	ITEPath *p = (ITEPath*)path;
	VG_GETCONTEXT(-1.0f);

	VG_RETURN_ERR_IF(!p || !iteIsValidPath(context, path),
                     VG_BAD_HANDLE_ERROR, -1.0f);
	VG_RETURN_ERR_IF(!(p->caps & VG_PATH_CAPABILITY_PATH_LENGTH),
                     VG_PATH_CAPABILITY_ERROR, -1.0f);
	VG_RETURN_ERR_IF(startSegment < 0 ||
		             startSegment >= p->segCount ||
					 numSegments <= 0 ||
					 (startSegment + numSegments - 1) >= p->segCount,
                     VG_ILLEGAL_ARGUMENT_ERROR, -1.0f);

	// ToDo: Implment vgPathLength() here.

	return 0.0f;
}

VG_API_CALL void
vgPointAlongPath(
	VGPath   path,
	VGint    startSegment,
	VGint    numSegments,
	VGfloat  distance,
	VGfloat* x,
	VGfloat* y,
	VGfloat* tangentX,
	VGfloat* tangentY)
{
	ITEPath *p = (ITEPath*)path;
	VG_GETCONTEXT(VG_NO_RETVAL);

	VG_RETURN_ERR_IF(!p || !iteIsValidPath(context, path),
                     VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(x && y && !(p->caps & VG_PATH_CAPABILITY_POINT_ALONG_PATH),
                     VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(tangentX && tangentY && !(p->caps & VG_PATH_CAPABILITY_TANGENT_ALONG_PATH),
                     VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(startSegment < 0 ||
		             startSegment >= p->segCount ||
					 numSegments <= 0 ||
					 (startSegment + numSegments - 1) >= p->segCount,
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!CheckAlignment(x, sizeof(VGfloat)) ||
		             !CheckAlignment(y, sizeof(VGfloat)) ||
		             !CheckAlignment(tangentX, sizeof(VGfloat)) ||
		             !CheckAlignment(tangentY, sizeof(VGfloat)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

    // ToDo: Implment vgPointAlongPath() here.

}
