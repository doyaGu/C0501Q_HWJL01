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
#include "itePath.h"
#include "iteVectors.h"
#include "iteGeometry.h"
#include "iteUtility.h"
#include "vgmem.h"

/* Vertex array */
#define _ITEM_T ITEVertex
#define _ARRAY_T ITEVertexArray
#define _FUNC_T iteVertexArray
#define _COMPARE_T(v1,v2) 0
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

#define _ITEM_T ITEPath*
#define _ARRAY_T ITEPathArray
#define _FUNC_T itePathArray
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

/* Path command array */
#define _ITEM_T ITEuint32
#define _ARRAY_T ITEPathCommandArray
#define _FUNC_T itePathCommandArray
#define _ARRAY_DEFINE
#include "iteArrayBase.h"


static const ITEint iteCoordsPerCommand[] = {
  0, /* VG_CLOSE_PATH */
  2, /* VG_MOVE_TO */
  2, /* VG_LINE_TO */
  1, /* VG_HLINE_TO */
  1, /* VG_VLINE_TO */
  4, /* VG_QUAD_TO */
  6, /* VG_CUBIC_TO */
  2, /* VG_SQUAD_TO */
  4, /* VG_SCUBIC_TO */
  5, /* VG_SCCWARC_TO */
  5, /* VG_SCWARC_TO */
  5, /* VG_LCCWARC_TO */
  5  /* VG_LCWARC_TO */
};

#define ITE_PATH_MAX_COORDS 6
#define ITE_PATH_MAX_COORDS_PROCESSED 12

static const ITEint iteBytesPerDatatype[] = {
  1, /* VG_PATH_DATATYPE_S_8 */
  2, /* VG_PATH_DATATYPE_S_16 */
  4, /* VG_PATH_DATATYPE_S_32 */
  4  /* VG_PATH_DATATYPE_F */
};

#define ITE_PATH_MAX_BYTES  4

/*-----------------------------------------------------
 * Path constructor
 *-----------------------------------------------------*/

ITEfloat iteValidInputFloat(VGfloat f);

void ITEPath_ctor(ITEPath *p)
{
	p->objType = ITE_VG_OBJ_PATH;
	p->format = 0;
	p->scale = 0.0f;
	p->bias = 0.0f;
	p->caps = 0;
	p->datatype = VG_PATH_DATATYPE_F;

	p->segs = NULL;
	p->data = NULL;
	p->segCount = 0;
	p->dataCount = 0;
	p->cmdDirty = ITE_TRUE;

	ITE_INITOBJ(ITEVertexArray, p->vertices);
	ITE_INITOBJ(ITEVector2Array, p->stroke);
	ITE_INITOBJ(ITEPathCommandArray, p->pathCommand);

	itePathCommandArrayReserve(&p->pathCommand, ITE_PATH_CMD_INIT_SIZE);

	p->referenceCount = 0;
}

/*-----------------------------------------------------
 * Path destructor
 *-----------------------------------------------------*/

void ITEPath_dtor(ITEPath *p)
{
  if (p->segs) free(p->segs);
  if (p->data) free(p->data);
  
  ITE_DEINITOBJ(ITEVertexArray, p->vertices);
  ITE_DEINITOBJ(ITEVector2Array, p->stroke);
  ITE_DEINITOBJ(ITEPathCommandArray, p->pathCommand);
}

/*-----------------------------------------------------
 * Operator for refence count of path
 *-----------------------------------------------------*/
ITEint 
ITEPath_AddReference(
	ITEPath* path)
{
	return ++path->referenceCount;
}

ITEint 
ITEPath_RemoveReference(
	ITEPath* path)
{
	path->referenceCount--;
	ITE_ASSERT(path->referenceCount >= 0);
	return path->referenceCount;
}

/*-----------------------------------------------------
 * Returns true (1) if given path data type is valid
 *-----------------------------------------------------*/

static ITEint iteIsValidDatatype(VGPathDatatype t)
{
  return (t == VG_PATH_DATATYPE_S_8 ||
          t == VG_PATH_DATATYPE_S_16 ||
          t == VG_PATH_DATATYPE_S_32 ||
          t == VG_PATH_DATATYPE_F);
}

static ITEint iteIsValidCommand(ITEint c)
{
  return (c >= (VG_CLOSE_PATH >> 1) &&
          c <= (VG_LCWARC_TO >> 1));
}

static ITEint iteIsArcSegment(ITEint s)
{
  return (s == VG_SCWARC_TO || s == VG_SCCWARC_TO ||
          s == VG_LCWARC_TO || s == VG_LCCWARC_TO);
}

/**
 * @brief itePathSetCommandDirty()
 *
 * If dirty is ITE_TRUE, pathCommand will regenerate from segCount & segCount
 *
 * @param path Path will be set.
 * @param dirty Flag to indicate dirty or not
 * @return void
 */
 static VGINLINE void
 itePathSetCommandDirty(
 	ITEPath*   path,
 	ITEboolean dirty)
{
	path->cmdDirty = dirty;
}

/*-----------------------------------------------------
 * Walks the given path segments and returns the total
 * number of coordinates.
 *-----------------------------------------------------*/

static ITEint iteCoordCountForData(VGint segcount, const ITEuint8 *segs)
{
	int s;
	int command;
	int count = 0;

	for (s=0; s<segcount; ++s) 
	{
		command = ((segs[s] & 0x1E) >> 1);
		if (!iteIsValidCommand(command))
		{
			return -1;
		}
		count += iteCoordsPerCommand[command];
	}

	return count;
}

/*-------------------------------------------------------
 * Interpretes the path data array according to the
 * path data type and returns the value at given index
 * in final interpretation (including scale and bias)
 *-------------------------------------------------------*/

static ITEfloat 
iteRealCoordFromData(
	VGPathDatatype type, 
	ITEfloat       scale, 
	ITEfloat       bias,
	void*          data, 
	ITEint         index)
{
	switch (type) 
	{
	case VG_PATH_DATATYPE_S_8:  return ( (ITEfloat) ((   ITEint8*)data)[index] ) * scale + bias;
	case VG_PATH_DATATYPE_S_16: return ( (ITEfloat) ((  ITEint16*)data)[index] ) * scale + bias;
	case VG_PATH_DATATYPE_S_32: return ( (ITEfloat) ((  ITEint32*)data)[index] ) * scale + bias;
	default:                    return ( (ITEfloat) ((ITEfloat32*)data)[index] ) * scale + bias;
	}
}

/*-------------------------------------------------------
 * Interpretes the path data array according to the
 * path data type and returns the value at given index
 * in final interpretation (but ignore scale and bias)
 *-------------------------------------------------------*/

static ITEfloat 
iteCoordFromData(
	VGPathDatatype type, 
	ITEfloat       scale, 
	ITEfloat       bias,
	void*          data, 
	ITEint         index)
{
	switch (type) 
	{
	case VG_PATH_DATATYPE_S_8:  return ( (ITEfloat) ((   ITEint8*)data)[index] );
	case VG_PATH_DATATYPE_S_16: return ( (ITEfloat) ((  ITEint16*)data)[index] );
	case VG_PATH_DATATYPE_S_32: return ( (ITEfloat) ((  ITEint32*)data)[index] );
	default:                    return ( (ITEfloat) ((ITEfloat32*)data)[index] );
	}
}

/*-------------------------------------------------------
 * Interpretes the path data array according to the
 * path data type and sets the value at given index
 * from final interpretation (including scale and bias)
 *-------------------------------------------------------*/

static void iteRealCoordToData(VGPathDatatype type, ITEfloat scale, ITEfloat bias,
                              void *data,  ITEint index, ITEfloat c)
{
	c -= bias;
	c /= scale;

	switch (type) 
	{
		case VG_PATH_DATATYPE_S_8:  ((   ITEint8*)data) [index] = ( ITEint8)ITE_FLOOR(c + 0.5f); break;
		case VG_PATH_DATATYPE_S_16: ((  ITEint16*)data) [index] = (ITEint16)ITE_FLOOR(c + 0.5f); break;
		case VG_PATH_DATATYPE_S_32: ((  ITEint32*)data) [index] = (ITEint32)ITE_FLOOR(c + 0.5f); break;
		default:                    ((ITEfloat32*)data) [index] = (ITEfloat32)c; break;
	}
}

/*-------------------------------------------------
 * Resizes storage for segment and coordinate data
 * of the specified path by given number of items
 *-------------------------------------------------*/

static int 
iteResizePathData(
	ITEPath*   p, 
	ITEint     newSegCount, 
	ITEint     newDataCount,
    ITEuint8** newSegs, 
    ITEuint8** newData)
{
	ITEint oldDataSize = 0;
	ITEint newDataSize = 0;

	/* Allocate memory for new segments */
	(*newSegs) = (ITEuint8*)malloc(p->segCount + newSegCount);
	if ((*newSegs) == NULL) 
	{
		return 0;
	}

	/* Allocate memory for new data */
	oldDataSize = p->dataCount * iteBytesPerDatatype[p->datatype];
	newDataSize = newDataCount * iteBytesPerDatatype[p->datatype];
	(*newData) = (ITEuint8*)malloc(oldDataSize + newDataSize);
	if ((*newData) == NULL)
	{
		free(*newSegs);
		return 0;
	}

	/* Copy old segments */
    if ( p->segs )
    {
	    memcpy(*newSegs, p->segs, p->segCount);
    }

	/* Copy old data */
    if ( p->data )
    {
    	memcpy(*newData, p->data, oldDataSize);
    }

	return 1;
}

/*------------------------------------------------------------
 * Converts standart endpoint arc parametrization into center
 * arc parametrization for further internal processing
 *------------------------------------------------------------*/

static ITEint iteCentralizeArc(ITEuint command, ITEfloat *data)
{
  ITEfloat rx,ry,r,a;
  ITEVector2 p1, p2, pp1,pp2;
  ITEVector2 d, dt;
  ITEfloat dist, halfdist, q;
  ITEVector2 c1, c2, *c;
  ITEVector2 pc1, pc2;
  ITEfloat a1, ad, a2;
  ITEVector2 ux, uy;
  ITEMatrix3x3 user2arc;
  ITEMatrix3x3 arc2user;
  
  rx = data[2];
  ry = data[3];
  a = ITE_DEG2RAD(data[4]);
  SET2(p1, data[0], data[1]);
  SET2(p2, data[5], data[6]);
  
  /* Check for invalid radius and return line */
  if (ITE_NEARZERO(rx) || ITE_NEARZERO(ry)) {
    data[2] = p2.x; data[3] = p2.y;
    return 0;
  }
  
  /* Rotate to arc coordinates.
     Scale to correct eccentricity. */
  IDMAT(user2arc);
  ROTATEMATL(user2arc, -a);
  SCALEMATL(user2arc, 1, rx/ry);
  TRANSFORM2TO(p1, user2arc, pp1);
  TRANSFORM2TO(p2, user2arc, pp2);
  
  /* Distance between points and
     perpendicular vector */
  SET2V(d, pp2); SUB2V(d, pp1);
  dist = NORM2(d);
  halfdist = dist * 0.5f;
  SET2(dt, -d.y, d.x);
  DIV2(dt, dist);
  
  /* Check if too close and return line */
  if (halfdist == 0.0f) {
    data[2] = p2.x; data[3] = p2.y;
    return 0;
  }
  
  /* Scale radius if too far away */
  r = rx;
  if (halfdist > rx)
    r = halfdist;
  
  /* Intersections of circles centered to start and
     end point. (i.e. centers of two possible arcs) */
  q = ITE_SIN(ITE_ACOS(halfdist/r)) * r;
  if (ITE_ISNAN(q)) q = 0.0f;
  c1.x = pp1.x + d.x/2 + dt.x * q;
  c1.y = pp1.y + d.y/2 + dt.y * q;
  c2.x = pp1.x + d.x/2 - dt.x * q;
  c2.y = pp1.y + d.y/2 - dt.y * q;
  
  /* Pick the right arc center */
  switch (command & 0x1E) {
  case VG_SCWARC_TO: case VG_LCCWARC_TO:
    c = &c2; break;
  case VG_LCWARC_TO: case VG_SCCWARC_TO:
    c = &c1; break;
  default:
    c = &c1;
  }
  
  /* Find angles of p1,p2 towards the chosen center */
  SET2V(pc1, pp1); SUB2V(pc1, (*c));
  SET2V(pc2, pp2); SUB2V(pc2, (*c));
  a1 = iteVectorOrientation(&pc1);
  
  /* Correct precision error when ry very small
     (q gets very large and points very far appart) */
  if (ITE_ISNAN(a1)) a1 = 0.0f;
  
  /* Small or large one? */
  switch (command & 0x1E) {
  case VG_SCWARC_TO: case VG_SCCWARC_TO:
    ad = ANGLE2(pc1,pc2); break;
  case VG_LCWARC_TO: case VG_LCCWARC_TO:
    ad = 2*PI - ANGLE2(pc1,pc2); break;
  default:
    ad = 0.0f;
  }
  
  /* Correct precision error when solution is single
     (pc1 and pc2 are nearly collinear but not really) */
  if (ITE_ISNAN(ad)) ad = PI;
  
  /* CW or CCW? */
  switch (command & 0x1E) {
  case VG_SCWARC_TO: case VG_LCWARC_TO:
    a2 = a1 - ad; break;
  case VG_SCCWARC_TO: case VG_LCCWARC_TO:
    a2 = a1 + ad; break;
  default:
    a2 = a1;
  }
  
  /* Arc unit vectors */
  SET2(ux, r, 0);
  SET2(uy, 0, r);
  
  /* Transform back to user coordinates */
  IDMAT(arc2user);
  SCALEMATL(arc2user, 1, ry/rx);
  ROTATEMATL(arc2user, a);
  TRANSFORM2((*c), arc2user);
  TRANSFORM2(ux, arc2user);
  TRANSFORM2(uy, arc2user);
  
  /* Write out arc coords */
  data[2]  = c->x;  data[3]  = c->y;
  data[4]  = ux.x;  data[5]  = ux.y;
  data[6]  = uy.x;  data[7]  = uy.y;
  data[8]  = a1;    data[9]  = a2;
  data[10] = p2.x;  data[11] = p2.y;
  return 1;
}

/*-------------------------------------------------------
 * Walks the raw (standard) path data and simplifies the
 * complex and implicit segments according to given
 * simplificatin request flags. Instead of storing the
 * processed data into another array, the given callback
 * function is called, so we don't need to walk the
 * raw data twice just to find the neccessary memory
 * size for processed data.
 *-------------------------------------------------------*/

#define ITE_PROCESS_SIMPLIFY_LINES    (1 << 0)
#define ITE_PROCESS_SIMPLIFY_CURVES   (1 << 1)
#define ITE_PROCESS_REPAIR_ENDS       (1 << 3)

void
iteProcessPathData(
	ITEPath*    p,
	int         flags,
	SegmentFunc callback,
	void*       userData)
{
	ITEint i=0, s=0, d=0, c=0;
	ITEuint command;
	//ITEuint segment;
	VGPathSegment segment;
	ITEint segindex;
	VGPathAbsRel absrel;
	ITEint numcoords;
	ITEfloat data[ITE_PATH_MAX_COORDS_PROCESSED];
	ITEVector2 start; /* start of the current contour */
	ITEVector2 pen; /* current pen position */
	ITEVector2 tan; /* backward tangent for smoothing */
	ITEint open = 0; /* contour-open flag */

	/* Reset points */
	SET2(start, 0,0);
	SET2(pen, 0,0);
	SET2(tan, 0,0);

	for (s=0; s<p->segCount; ++s, d+=numcoords) 
	{
		/* Extract command */
		command = (p->segs[s]);
		absrel = (command & 1);
		segment = (command & 0x1E);
		segindex = (segment >> 1);
		numcoords = iteCoordsPerCommand[segindex];

		/* Repair segment start / end */
		if (flags & ITE_PROCESS_REPAIR_ENDS)
		{
			/* Prevent double CLOSE_PATH */
			if (!open && segment == VG_CLOSE_PATH)
			{
				continue;
			}

			/* Implicit MOVE_TO if segment starts without */
			if (!open && segment != VG_MOVE_TO) 
			{
				data[0] = pen.x; data[1] = pen.y;
				data[2] = pen.x; data[3] = pen.y;
				(*callback)(p,VG_MOVE_TO,command,data,userData);
				open = 1;
			}

			/* Avoid a MOVE_TO at the end of data */
			if (segment == VG_MOVE_TO) 
			{
				if (s == p->segCount-1)
				{
					break;
				}
				else
				{
					/* Avoid a lone MOVE_TO  */
					VGPathSegment nextsegment = (p->segs[s+1] & 0x1E);
					if (nextsegment == VG_MOVE_TO)
					{
						open = 0;
						continue;
					}
				}
			}
		}

		/* Place pen into first two coords */
		data[0] = pen.x;
		data[1] = pen.y;
		c = 2;

		/* Unpack coordinates from path data */
		for (i=0; i<numcoords; ++i)
		{
			data[c++] = iteRealCoordFromData(p->datatype, p->scale, p->bias, p->data, d+i);
		}

		/* Simplify complex segments */
		switch (segment)
		{
		case VG_CLOSE_PATH:		  
			data[2] = start.x;
			data[3] = start.y;

			SET2V(pen, start);
			SET2V(tan, start);
			open = 0;

			(*callback)(p,VG_CLOSE_PATH,command,data,userData);
			break;
			
		case VG_MOVE_TO:
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.x;
				data[3] += pen.y;
			}

			SET2(pen, data[2], data[3]);
			SET2V(start, pen);
			SET2V(tan, pen);
			open = 1;

			(*callback)(p,VG_MOVE_TO,command,data,userData);
			break;
		  
		case VG_LINE_TO:		  
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.x;
				data[3] += pen.y;
			}

			SET2(pen, data[2], data[3]);
			SET2V(tan, pen);

			(*callback)(p,VG_LINE_TO,command,data,userData);
			break;
		  
		case VG_HLINE_TO:
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.x;
			}

			SET2(pen, data[2], pen.y);
			SET2V(tan, pen);

			if (flags & ITE_PROCESS_SIMPLIFY_LINES)
			{
				data[3] = pen.y;
				(*callback)(p,VG_LINE_TO,command,data,userData);
				break;
			}

			(*callback)(p,VG_HLINE_TO,command,data,userData);
			break;
			
		case VG_VLINE_TO:
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.y;
			}

			SET2(pen, pen.x, data[2]);
			SET2V(tan, pen);

			if (flags & ITE_PROCESS_SIMPLIFY_LINES)
			{
				data[2] = pen.x; 
				data[3] = pen.y;
				(*callback)(p,VG_LINE_TO,command,data,userData);
				break;
			}

			(*callback)(p,VG_VLINE_TO,command,data,userData);
			break;
			
		case VG_QUAD_TO:
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.x; data[3] += pen.y;
				data[4] += pen.x; data[5] += pen.y;
			}

			SET2(tan, data[2], data[3]);
			SET2(pen, data[4], data[5]);

			// Set tan & pen
			data[6] = tan.x;
			data[7] = tan.y;
			data[8] = pen.x;
			data[9] = pen.y;

			(*callback)(p,VG_QUAD_TO,command,data,userData);
			break;
			
		case VG_CUBIC_TO:
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.x; data[3] += pen.y;
				data[4] += pen.x; data[5] += pen.y;
				data[6] += pen.x; data[7] += pen.y;
			}

			SET2(tan, data[4], data[5]);
			SET2(pen, data[6], data[7]);

			(*callback)(p,VG_CUBIC_TO,command,data,userData);
			break;
			
		case VG_SQUAD_TO:		  
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.x; data[3] += pen.y;
			}

			SET2(tan, 2*pen.x - tan.x, 2*pen.y - tan.y);
			SET2(pen, data[2], data[3]);

			if (flags & ITE_PROCESS_SIMPLIFY_CURVES)
			{
				data[2] = tan.x; data[3] = tan.y;
				data[4] = pen.x; data[5] = pen.y;
				(*callback)(p,VG_QUAD_TO,command,data,userData);
				break;
			}

			(*callback)(p,VG_SQUAD_TO,command,data,userData);
			break;

		case VG_SCUBIC_TO:
			if (absrel == VG_RELATIVE)
			{
				data[2] += pen.x; data[3] += pen.y;
				data[4] += pen.x; data[5] += pen.y;
			}

			if (flags & ITE_PROCESS_SIMPLIFY_CURVES)
			{
				data[7] = data[5];  
				data[6] = data[4];        
				data[5] = data[3];
				data[4] = data[2]; 

				data[2] = 2*pen.x - tan.x;
				data[3] = 2*pen.y - tan.y;

				SET2(tan, data[4], data[5]);
				SET2(pen, data[6], data[7]);
				(*callback)(p,VG_CUBIC_TO,command,data,userData);
				break;
			}

			SET2(tan, data[2], data[3]);
			SET2(pen, data[4], data[5]);

			(*callback)(p,VG_SCUBIC_TO,command,data,userData);
			break;
			
		case VG_SCWARC_TO: 
		case VG_SCCWARC_TO:
		case VG_LCWARC_TO:
		case VG_LCCWARC_TO:
			if (absrel == VG_RELATIVE)
			{
				data[5] += pen.x;
				data[6] += pen.y;
			}

			data[2] = ITE_ABS(data[2]);
			data[3] = ITE_ABS(data[3]);

			SET2(tan, data[5], data[6]);
			SET2V(pen, tan);

			(*callback)(p,segment,command,data,userData);
			break;		  
		} /* switch (command) */
	} /* for each segment */
}

/*-------------------------------------------------------
 * Walks raw path data and counts the resulting number
 * of segments and coordinates if the simplifications
 * specified in the given processing flags were applied.
 *-------------------------------------------------------*/

static void 
iteProcessedDataCount(
	ITEPath* p, 
	ITEint   flags,
	ITEint*  segCount, 
	ITEint*  dataCount)
{
	ITEint s, segment, segindex;
	
	*segCount = 0; 
	*dataCount = 0;

	for (s=0; s<p->segCount; ++s) 
	{
		segment = (p->segs[s] & 0x1E);
		segindex = (segment >> 1);

		switch (segment) 
		{
		case VG_HLINE_TO:
		case VG_VLINE_TO:
			if (flags & ITE_PROCESS_SIMPLIFY_LINES)
			{
				*dataCount += iteCoordsPerCommand[VG_LINE_TO >> 1];
			}
			else
			{
				*dataCount += iteCoordsPerCommand[segindex];
			}
			break;

		case VG_SQUAD_TO:
			if (flags & ITE_PROCESS_SIMPLIFY_CURVES)
			{
				*dataCount += iteCoordsPerCommand[VG_QUAD_TO >> 1];
			}
			else
			{
				*dataCount += iteCoordsPerCommand[segindex];
			}
			break;

		case VG_SCUBIC_TO:
			if (flags & ITE_PROCESS_SIMPLIFY_CURVES)
			{
				*dataCount += iteCoordsPerCommand[VG_CUBIC_TO >> 1];
			}
			else
			{
				*dataCount += iteCoordsPerCommand[segindex];
			}
			break;

		case VG_SCWARC_TO: 
		case VG_SCCWARC_TO:
		case VG_LCWARC_TO: 
		case VG_LCCWARC_TO:
			if (flags & ITE_PROCESS_CENTRALIZE_ARCS)
			{
				*dataCount += 10;
			}
			else
			{
				*dataCount += iteCoordsPerCommand[segindex];
			}
			break;

		default:
			*dataCount += iteCoordsPerCommand[segindex];
		}

		*segCount += 1;
	}
}

/*-------------------------------------------------------
 * Walks raw path data and counts the resulting number
 * of segments and coordinates if the simplifications
 * specified in the given processing flags were applied.
 *-------------------------------------------------------*/

static void 
iteProcessedDataCountForInterpolation(
	ITEPath* p, 
	ITEint   flags,
	ITEint*  segCount, 
	ITEint*  dataCount)
{
	ITEint s, segment, segindex;
	
	*segCount = 0; 
	*dataCount = 0;

	for (s=0; s<p->segCount; ++s) 
	{
		segment = (p->segs[s] & 0x1E);
		segindex = (segment >> 1);

		switch (segment) 
		{
		case VG_HLINE_TO:
		case VG_VLINE_TO:
			if (flags & ITE_PROCESS_SIMPLIFY_LINES)
			{
				*dataCount += iteCoordsPerCommand[VG_LINE_TO >> 1];
			}
			else
			{
				*dataCount += iteCoordsPerCommand[segindex];
			}
			break;

		case VG_SCWARC_TO: 
		case VG_SCCWARC_TO:
		case VG_LCWARC_TO: 
		case VG_LCCWARC_TO:
			if (flags & ITE_PROCESS_CENTRALIZE_ARCS)
			{
				*dataCount += 10;
			}
			else
			{
				*dataCount += iteCoordsPerCommand[segindex];
			}
			break;

		case VG_QUAD_TO:
		case VG_CUBIC_TO:
		case VG_SQUAD_TO:
		case VG_SCUBIC_TO:
			*dataCount += 6;

		default:
			*dataCount += iteCoordsPerCommand[segindex];
		}

		*segCount += 1;
	}
}

static void iteTransformSegment(ITEPath *p, VGPathSegment segment,
                               VGPathCommand originalCommand,
                               ITEfloat *data, void *userData)
{
  int i, numPoints; ITEVector2 point;
  ITEuint8* newSegs   = (ITEuint8*) ((void**)userData)[0];
  ITEint*   segCount  = (ITEint*)   ((void**)userData)[1];
  ITEuint8* newData   = (ITEuint8*) ((void**)userData)[2];
  ITEint*   dataCount = (ITEint*)   ((void**)userData)[3];
  ITEPath*  dst       = (ITEPath*)  ((void**)userData)[4];
  ITEMatrix3x3 *ctm;
  
  /* Get current transform matrix */
  VG_GETCONTEXT(VG_NO_RETVAL);
  ctm = &context->pathTransform;
  
  switch (segment) {
  case VG_CLOSE_PATH:
      
    /* No cordinates for this segment */
    
    break;
  case VG_MOVE_TO:
  case VG_LINE_TO:
  case VG_QUAD_TO:
  case VG_CUBIC_TO:
  case VG_SQUAD_TO:
  case VG_SCUBIC_TO:
    
    /* Walk over control points */
    numPoints = iteCoordsPerCommand[segment >> 1] / 2;
    for (i=0; i<numPoints; ++i) {
      
      /* Transform point by user to surface matrix */
      SET2(point, data[2 + i*2], data[2 + i*2 + 1]);
      TRANSFORM2(point, (*ctm));
      
      /* Write coordinates back to path data */
      iteRealCoordToData(dst->datatype, dst->scale, dst->bias,
                        newData, (*dataCount)++, point.x);
      
      iteRealCoordToData(dst->datatype, dst->scale, dst->bias,
                        newData, (*dataCount)++, point.y);
    }
    
    break;
  default:{
      
      ITEMatrix3x3 u, u2;
      ITEfloat a, cosa, sina, rx, ry;
      ITEfloat A,B,C,AC,K,A2,C2;
      ITEint invertible;
      ITEVector2 p;
      ITEfloat out[5];
      ITEint i;
      
      ITE_ASSERT(segment==VG_SCWARC_TO || segment==VG_SCCWARC_TO ||
                segment==VG_LCWARC_TO || segment==VG_LCCWARC_TO);
      
      rx = data[2]; ry = data[3];
      a = ITE_DEG2RAD(data[4]);
      cosa = ITE_COS(a); sina = ITE_SIN(a);
      
      /* Center parametrization of the elliptical arc. */
      SETMAT(u, rx * cosa, -ry * sina, 0,
             rx * sina,  ry * cosa,    0,
             0,          0,            1);
      
      /* Add current transformation */
      MULMATMAT((*ctm), u, u2);
      
      /* Inverse (transforms ellipse back to unit circle) */
      invertible = iteInvertMatrix(&u2, &u);
      if (!invertible) {
        
        /* Zero-out radii and rotation */
        out[0] = out[1] = out[2] = 0.0f;
        
      }else{
      
        /* Extract implicit ellipse equation */
        A =     u.m[0][0]*u.m[0][0] + u.m[1][0]*u.m[1][0];
        B = 2*( u.m[0][0]*u.m[0][1] + u.m[1][0]*u.m[1][1] );
        C =     u.m[0][1]*u.m[0][1] + u.m[1][1]*u.m[1][1];
        AC = A-C;
        
        /* Find rotation and new radii */
        if (B == 0.0f) {
          
          /* 0 or 90 degree */
          out[2] = 0.0f;
          A2 = A;
          C2 = C;
          
        }else if (AC == 0.0f) {
          
          /* 45 degree */
          out[2] = PI/4;
          out[2] = ITE_RAD2DEG(out[2]);
          A2 = A + B * 0.5f;
          C2 = A - B * 0.5f;
          
        }else{
          
          /* Any other angle */
          out[2] = 0.5f * ITE_ATAN(B / AC);
          out[2] = ITE_RAD2DEG(out[2]);
          
          K = 1 + (B*B) / (AC*AC);
          if (K <= 0.0f) K = 0.0f;
          else K = ITE_SQRT(K);
          
          A2 = 0.5f * (A + C + K*AC);
          C2 = 0.5f * (A + C - K*AC);
        }
        
        /* New radii */
        out[0] = (A2 <= 0.0f ? 0.0f : 1 / ITE_SQRT(A2));
        out[1] = (C2 <= 0.0f ? 0.0f : 1 / ITE_SQRT(C2));
        
        /* Check for a mirror component in the transform matrix */
        if (ctm->m[0][0] * ctm->m[1][1] - ctm->m[1][0] * ctm->m[0][1] < 0.0f) {
          switch (segment) {
          case VG_SCWARC_TO: segment = VG_SCCWARC_TO; break;
          case VG_LCWARC_TO: segment = VG_LCCWARC_TO; break;
          case VG_SCCWARC_TO: segment = VG_SCWARC_TO; break;
          case VG_LCCWARC_TO: segment = VG_LCWARC_TO; break;
          default: break; }
        }
        
      }/* if (invertible) */
      
      /* Transform target point */
      SET2(p, data[5], data[6]);
      TRANSFORM2(p, context->pathTransform);
      out[3] = p.x; out[4] = p.y;
      
      /* Write coordinates back to path data */
      for (i=0; i<5; ++i)
        iteRealCoordToData(dst->datatype, dst->scale, dst->bias,
                          newData, (*dataCount)++, out[i]);
      
      break;}
  }
  
  /* Write segment to new dst path data */
  newSegs[(*segCount)++] = segment | VG_ABSOLUTE;
}

static void 
iteInterpolateSegment(
	ITEPath*      p, 
	VGPathSegment segment,
	VGPathCommand originalCommand,
	ITEfloat*     data, 
	void*         userData)
{
	ITEuint8* procSegs      = (ITEuint8*) ((void**)userData)[0];
	ITEint*   procSegCount  = (ITEint*)   ((void**)userData)[1];
	ITEfloat* procData      = (ITEfloat*) ((void**)userData)[2];
	ITEint*   procDataCount = (ITEint*)   ((void**)userData)[3];
	ITEint segindex, i;

	/* Write the processed segment back */
	if ( segment == VG_QUAD_TO )
	{
		procSegs[(*procSegCount)++] = VG_CUBIC_TO | VG_ABSOLUTE;
	}
	else
	{
		procSegs[(*procSegCount)++] = segment | VG_ABSOLUTE;
	}

	/* Write the processed data back (exclude first 2 pen coords!) */
	if ( segment == VG_QUAD_TO )
	{
		ITEVector2 c0 = {data[2], data[3]};		
		ITEVector2 c1 = {data[4], data[5]};
		ITEVector2 d0 = {0};
		ITEVector2 d1 = {0};
		ITEVector2 tan = {data[6], data[7]};
		ITEVector2 pen = {data[8], data[9]};

		segindex = (VG_CUBIC_TO >> 1);

		d0.x = (1.0f/3.0f) * (tan.x + 2.0f * c0.x);
		d0.y = (1.0f/3.0f) * (tan.y + 2.0f * c0.y);
		d1.x = (1.0f/3.0f) * (c1.x + 2.0f * c0.x);
		d1.y = (1.0f/3.0f) * (c1.y + 2.0f * c0.y);

		procData[(*procDataCount)++] = d0.x;
		procData[(*procDataCount)++] = d0.y;
		procData[(*procDataCount)++] = d1.x;
		procData[(*procDataCount)++] = d1.y;
		procData[(*procDataCount)++] = c1.x;
		procData[(*procDataCount)++] = c1.y;
	}
	else
	{
		segindex = (segment >> 1);
		for ( i = 2; i < (2 + iteCoordsPerCommand[segindex]); ++i)
		{
			procData[(*procDataCount)++] = data[i];
		}
	}
}

static void
iteGenDashParamter(
	float          dashPhase,			/* IN */
	ITEFloatArray* contextDashArray,	/* IN */
	ITEuint32      hwDashArraySize,		/* IN */
	ITEs15p16* 	   hwDashArray,			/* OUT */
	ITEuint32*     hwDashArrayUsedSize)	/* OUT */
{
	if ( contextDashArray->size != 0 )
	{
		int i=0, /*j=0,*/ phstart=0, phfirst=1, OtherSize=0;
		
		*hwDashArrayUsedSize = 0;
		
		for (i=0; i<contextDashArray->size; i++)
		{
			if (dashPhase <= contextDashArray->items[i] || phstart)
			{
				if (phfirst)
				{
					if (i & 1)
					{
						hwDashArray[*hwDashArrayUsedSize] = (ITEs15p16)((contextDashArray->items[i]-dashPhase)*(1<<POINTPREC));	// s12.11
					}
					else 
					{
						hwDashArray[*hwDashArrayUsedSize] = (ITEs15p16)((contextDashArray->items[i]-dashPhase)*(1<<POINTPREC) + (1<<(POINTPREC+12)));	// s12.11
					}
				}
				else
				{
					if (i & 1)
					{
						hwDashArray[*hwDashArrayUsedSize] = (ITEs15p16)(contextDashArray->items[i]*(1<<POINTPREC) );	// s12.11
					}
					else
					{
						hwDashArray[*hwDashArrayUsedSize] = (ITEs15p16)(contextDashArray->items[i]*(1<<POINTPREC) + (1<<(POINTPREC+12)));	// s12.11
					}
				}
				phstart = 1;
				phfirst = 0;
				(*hwDashArrayUsedSize)++;

				if ( *hwDashArrayUsedSize >= hwDashArraySize )
				{
					return;
				}
			}
			else
			{
				dashPhase -= contextDashArray->items[i];
			}
		}

		OtherSize = contextDashArray->size - (*hwDashArrayUsedSize) + 1;

		for (i=0; i<OtherSize; i++)
		{
			if (i & 1)
			{
				hwDashArray[*hwDashArrayUsedSize] = (ITEs15p16)(contextDashArray->items[i]*(1<<POINTPREC));	// s12.11
			}
			else
			{
				hwDashArray[*hwDashArrayUsedSize] = (ITEs15p16)(contextDashArray->items[i]*(1<<POINTPREC) + (1<<(POINTPREC+12)));	// s12.11
			}

			(*hwDashArrayUsedSize)++;

			if ( *hwDashArrayUsedSize >= hwDashArraySize )
			{
				return;
			}
		}
	}
	else
	{
		VG_Memset(hwDashArray, 0x00, sizeof(float) * hwDashArraySize);
	}
}

#if 1
void 
iteDrawPath(
	ITEPath*      p, 
	ITEuint       paintModes,
	ITEMatrix3x3* userToSurfaceMatrix)
{
	ITEMatrix3x3 pathMatrix          = {0};
    ITEuint8*    tessellateCmdBuffer = NULL;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Precondition */
	if ( p->segCount == 0 )
	{
		/* Nothing need to draw */
		return;
	}

	pathMatrix = *userToSurfaceMatrix;

	/* Apply scale & bias */
    // Todo: Does need it?

	if ( p->cmdDirty == ITE_TRUE )
	{
		itePathCommandArrayClear(&p->pathCommand);
		iteFlattenPath(p, 1, &p->pathCommand);
		p->cmdDirty = ITE_FALSE;
	}
	if (paintModes & VG_FILL_PATH) 
	{
		ITEHardwareRegister* hw                    = &context->hardware;
		ITEPaint*            paint                 = NULL;
		ITEint               minterLength          = 100*16;
		ITEboolean           enPerspective         = ITE_FALSE;
		ITEuint32            hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;
		ITEuint32            hwImageQuality        = ITE_VG_CMD_IMAGEQ_NONAA;
		ITEuint32            hwFillRule            = ITE_VG_CMD_FILLRULE_ODDEVEN;
		ITEImage*            hwCoverageImage       = NULL;
		ITEImage*            hwValidImage		   = NULL;
		ITEuint32            hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
		ITEboolean           hwEnPreMulDstImage    = ITE_FALSE; // 0x0B0[31]
		ITEboolean           hwEnUnpreColorTrans   = ITE_FALSE; // 0x0B0[30]
		ITEboolean           hwEnPreMulBlending    = ITE_FALSE; // 0x0B0[29]
		ITEboolean           hwEnPreMulTexImage    = ITE_FALSE; // 0x0B0[28]
		ITEboolean           hwEnGamma             = ITE_FALSE;
		ITEuint32            hwGammaMode           = ITE_VG_CMD_GAMMAMODE_INVERSE;
		ITEboolean           hwWaitObjID           = ITE_FALSE;

		paint = (context->fillPaint) ? context->fillPaint : &context->defaultPaint;

		if (   pathMatrix.m[2][0] 
			|| pathMatrix.m[2][1] 
			|| pathMatrix.m[2][2] != 1.0f )
		{
			enPerspective = ITE_TRUE;
		}

		/* Get render quality parameter */
		switch (context->renderingQuality)
		{
		default:
		case VG_RENDERING_QUALITY_NONANTIALIASED:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;  
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_FASTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_FASTER;
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_BETTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_BETTER; 
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_TWOBYTES;
			break;
		}

		/* Get image quality parameter */
		switch (context->imageQuality)
		{
		default:
		case VG_IMAGE_QUALITY_NONANTIALIASED: hwImageQuality = ITE_VG_CMD_IMAGEQ_NONAA;  break;
		case VG_IMAGE_QUALITY_FASTER:         hwImageQuality = ITE_VG_CMD_IMAGEQ_FASTER; break;
		case VG_IMAGE_QUALITY_BETTER:         hwImageQuality = ITE_VG_CMD_IMAGEQ_BETTER; break;
		}

		/* Get fill rule parameter */
		switch (context->fillRule)
		{
		default:
		case VG_EVEN_ODD: hwFillRule = ITE_VG_CMD_FILLRULE_ODDEVEN; break;
		case VG_NON_ZERO: hwFillRule = ITE_VG_CMD_FILLRULE_NONZERO; break;
		}

		/* Get coverage image parameter */
		if ( context->surface->coverageIndex )
		{
			hwCoverageImage = context->surface->coverageImageB;
			hwValidImage = context->surface->validImageB;
			context->surface->coverageIndex = 0;
		}
		else
		{
			hwCoverageImage = context->surface->coverageImageA;
			hwValidImage = context->surface->validImageA;
			context->surface->coverageIndex = 1;
		}

		/* Get pre/unpre parameter */
		// 0x0B0[28]
		if ( paint && paint->pattern )
		{
			if ( ITEImage_IsPreMultipliedFormat(paint->pattern->vgformat) )
			{
				hwEnPreMulTexImage = ITE_FALSE;
			}
			else
			{
				hwEnPreMulTexImage = ITE_TRUE;
			}
		}
		else
		{
			hwEnPreMulTexImage = ITE_FALSE;
		}
		// 0x0B0[30]
		hwEnUnpreColorTrans = ITE_TRUE;
		// 0x0B0[29]
		if (   context->blendMode != VG_BLEND_SRC
			|| hwRenderQuality != ITE_VG_CMD_RENDERQ_NONAA )
		{
			hwEnPreMulBlending = ITE_TRUE;
		}
		else
		{
			hwEnPreMulBlending = ITE_FALSE;
		}
		// 0x0B0[31]
		if ( ITEImage_IsPreMultipliedFormat(context->surface->colorImage->vgformat) )
		{
			hwEnPreMulDstImage = ITE_FALSE;
		}
		else
		{
			hwEnPreMulDstImage = ITE_TRUE;
		}

		/* Get Gamma/Degamma parameter */
		if (   paint->type == VG_PAINT_TYPE_PATTERN 
			&& paint->pattern )
		{
			if (   ITEImage_IsSrgbFormat(paint->pattern->vgformat) 
				!= ITEImage_IsSrgbFormat(context->surface->colorImage->vgformat) )
			{
				hwEnGamma = ITE_FALSE;
				if ( ITEImage_IsSrgbFormat(paint->pattern->vgformat) )
				{
					/* sRGB --> lRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
					hwEnGamma = ITE_TRUE;
				}
				else if ( ITEImage_IsLrgbFormat(paint->pattern->vgformat) )
				{
					/* lRGB --> sRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
					hwEnGamma = ITE_TRUE;
				}
			}
			else
			{
				hwEnGamma = ITE_FALSE;
			}
		}
		else
		{
			hwEnGamma = ITE_FALSE;
		}

		/* Set hardware parameter */
		hw->REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN |
		                   ITE_VG_CMD_SHIFTROUNDING |
		                   ITE_VG_CMD_JOINROUND |
			               ITE_VG_CMD_READMEM |
			               ITE_VG_CMD_TELWORK;
		//hw->REG_PLR_BASE = hw->cmdLength * sizeof(ITEuint32);
		hw->REG_PLR_BASE = p->pathCommand.size * sizeof(ITEuint32);

		//allocate vram buffer 
		if ( (p->pathCommand.size * sizeof(ITEuint32)) <= ITE_PATH_COPY_SIZE_THRESHOLD )
		{
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, allocSize, iteHardwareGenObjectID());
			mappedSysRam = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
		}
		else
		{
#ifdef _WIN32
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
			mappedSysRam = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
#else
			tessellateCmdBuffer = (ITEuint8*)p->pathCommand.items;
#endif
			hwWaitObjID = ITE_TRUE;
		}
		hw->REG_PBR_BASE = ((ITEuint32)tessellateCmdBuffer) << ITE_VG_CMDSHIFT_PATHBASE;
		hw->REG_BID2_BASE = iteHardwareGetCurrentObjectID();

		hw->REG_UTR00_BASE = (ITEs15p16)(pathMatrix.m[0][0] * 0x10000 + 1.0f);
		hw->REG_UTR01_BASE = (ITEs15p16)(pathMatrix.m[0][1] * 0x10000 + 1.0f);
		hw->REG_UTR02_BASE = (ITEs15p16)(pathMatrix.m[0][2] * 0x10000 + 1.0f);
		hw->REG_UTR10_BASE = (ITEs15p16)(pathMatrix.m[1][0] * 0x10000 + 1.0f);
		hw->REG_UTR11_BASE = (ITEs15p16)(pathMatrix.m[1][1] * 0x10000 + 1.0f);
		hw->REG_UTR12_BASE = (ITEs15p16)(pathMatrix.m[1][2] * 0x10000 + 1.0f);
		hw->REG_UTR20_BASE = (ITEs15p16)(pathMatrix.m[2][0] * 0x10000 + 1.0f);
		hw->REG_UTR21_BASE = (ITEs15p16)(pathMatrix.m[2][1] * 0x10000 + 1.0f);
		hw->REG_UTR22_BASE = (ITEs15p16)(pathMatrix.m[2][2] * 0x10000 + 1.0f);
		hw->REG_CCR_BASE   = ITE_VG_CMD_TIMERDY_EN |
          	                 ITE_VG_CMD_FULLRDY_EN |
          	                 ITE_VG_CMD_CLIPPING |
			                 hwRenderQuality |
			                 hwCoverageFormatBytes;
		hw->REG_CPBR_BASE = ((ITEuint32)hwCoverageImage->data) >> ITE_VG_CMDSHIFT_PLANBASE;
		hw->REG_CVPPR_BASE = (hwValidImage->pitch << ITE_VG_CMDSHIFT_VALIDPITCH) |
			                 (hwCoverageImage->pitch<< ITE_VG_CMDSHIFT_PLANPITCH);
		hw->REG_VPBR_BASE = ((ITEuint32)hwValidImage->data) << ITE_VG_CMDSHIFT_VALIDBASE;
		hw->REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
		hw->REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;
		hw->REG_RCR_BASE = (hwEnPreMulDstImage ? ITE_VG_CMD_DST_PRE_EN : 0) |
			               (hwEnUnpreColorTrans ? ITE_VG_CMD_SRC_NONPRE_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_SRC_PRE_EN : 0) |
			               (hwEnPreMulTexImage ? ITE_VG_CMD_TEX_PRE_EN : 0) |
			               ITE_VG_CMD_DITHER_EN | // always enable dither 
			               ITE_VG_CMD_BLEND_EN |
			               (hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
			               (context->enColorTransform ? ITE_VG_CMD_COLORCLIP_EN : 0) |
			               (context->enColorTransform ? ITE_VG_CMD_COLORXFM_EN : 0) |
						   ((context->masking == VG_TRUE) ? ITE_VG_CMD_MASK_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_DESTINATION_EN : 0) |
			               ITE_VG_CMD_TEXCACHE_EN |
			               ITE_VG_CMD_TEXTURE_EN |
			               (context->scissoring ? ITE_VG_CMD_SCISSOR_EN : 0) |
			               ITE_VG_CMD_COVERAGE_EN |
			               hwImageQuality |
			               hwFillRule |
			               ITE_VG_CMD_RENDERMODE_0 |
			               ITE_VG_CMD_RDPLN_VLD_EN;
		hw->REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
			               ITE_VG_CMD_MASKMODE_INTERSECT |
			               hwGammaMode |
			               ((context->blendMode & 0x0F) << ITE_VG_CMDSHIFT_BLENDMODE) |
		                   ((paint != NULL) ? ((paint->tilingMode & 0x03) << 6) : ITE_VG_CMD_TILEMODE_FILL) |
			               ((paint != NULL) ? ((paint->spreadMode & 0x03) << 4) : ITE_VG_CMD_RAMPMODE_PAD) |
			               ((paint != NULL) ? ((paint->type & 0x03) << 2) : ITE_VG_CMD_PAINTTYPE_COLOR) |
			                context->imageMode & 0x03;
		hw->REG_RFR_BASE = ITE_VG_CMD_MASKEXTEND_EN |
		                   ITE_VG_CMD_DSTEXTEND_EN |
		                   ITE_VG_CMD_SRCEXTEND_EN |
			               (context->surface->maskImage->vgformat << 16) |
			               (context->surface->colorImage->vgformat << 8) |
			               ((paint && paint->pattern) ? paint->pattern->vgformat : 0);

		/* context->colorTransform = { Sr, Sg, Sb, Sa, Br, Bg, Bb, Ba } */
#if 0
		/* Color Transform, [31:16]=Ba, [15:0]=Sa */
		hw->REG_CTR0_BASE = (((ITEint16)(context->colorTransform[7] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM01) |
		                    (ITEint16)(context->colorTransform[3] * 0x100);
		/* Color Transform, [31:16]=Br, [15:0]=Sr */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[4] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM11) |
		                    (ITEint16)(context->colorTransform[0] * 0x100);
		/* Color Transform, [31:16]=Bg, [15:0]=Sg */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[5] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM21) |
		                    (ITEint16)(context->colorTransform[1] * 0x100);
		/* Color Transform, [31:16]=Bb, [15:0]=Sb */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[6] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM31) |
		                    (ITEint16)(context->colorTransform[2] * 0x100);
#else
		hw->REG_CTBR0_BASE = (ITEint32)(context->colorTransform[7] * 0x100);
		hw->REG_CTBR1_BASE = (ITEint32)(context->colorTransform[4] * 0x100);
		hw->REG_CTBR2_BASE = (ITEint32)(context->colorTransform[5] * 0x100);
	    hw->REG_CTBR3_BASE = (ITEint32)(context->colorTransform[6] * 0x100);

		hw->REG_CTSR0_BASE = (((ITEuint32)(context->colorTransform[1] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[0] * 0x100);


		hw->REG_CTSR1_BASE = (((ITEint32)(context->colorTransform[3] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[2] * 0x100);
#endif

		hw->REG_DCR_BASE = 0;
		hw->REG_DHWR_BASE = (context->surface->colorImage->height << ITE_VG_CMDSHIFT_DSTHEIGHT) |
			                context->surface->colorImage->width;
		hw->REG_DBR_BASE = (ITEuint32)(context->surface->colorImage->data);
		hw->REG_SDPR_BASE = ((paint && paint->pattern) ? paint->pattern->pitch << ITE_VG_CMDSHIFT_SRCPITCH0 : 0) |
			                context->surface->colorImage->pitch;
		hw->REG_SCR_BASE = 0;
		hw->REG_SHWR_BASE = (paint && paint->pattern)
			                ? (paint->pattern->height << ITE_VG_CMDSHIFT_SRCHEIGHT | paint->pattern->width)
			                : 0;
		hw->REG_SBR_BASE = (paint && paint->pattern) ? (ITEuint32)paint->pattern->data : 0;
		hw->REG_MBR_BASE = (ITEuint32)context->surface->maskImage->data;
		hw->REG_SMPR_BASE = ((context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) : 0) |
			                context->surface->maskImage->pitch;
		hw->REG_SCBR_BASE = (context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.data >> ITE_VG_CMDSHIFT_SCISBASE) : 0;
		{
			ITEMatrix3x3 pathInvMatrix;
			
			iteInvertMatrix(&pathMatrix, &pathInvMatrix);
			hw->REG_UITR00_BASE = (ITEs15p16)(pathInvMatrix.m[0][0] * 0x10000 + 1.0f);
			hw->REG_UITR01_BASE = (ITEs15p16)(pathInvMatrix.m[0][1] * 0x10000 + 1.0f);
			hw->REG_UITR02_BASE = (ITEs15p16)(pathInvMatrix.m[0][2] * 0x10000 + 1.0f);
			hw->REG_UITR10_BASE = (ITEs15p16)(pathInvMatrix.m[1][0] * 0x10000 + 1.0f);
			hw->REG_UITR11_BASE = (ITEs15p16)(pathInvMatrix.m[1][1] * 0x10000 + 1.0f);
			hw->REG_UITR12_BASE = (ITEs15p16)(pathInvMatrix.m[1][2] * 0x10000 + 1.0f);
			hw->REG_UITR20_BASE = (ITEs15p16)(pathInvMatrix.m[2][0] * 0x10000 + 1.0f);
			hw->REG_UITR21_BASE = (ITEs15p16)(pathInvMatrix.m[2][1] * 0x10000 + 1.0f);
			hw->REG_UITR22_BASE = (ITEs15p16)(pathInvMatrix.m[2][2] * 0x10000 + 1.0f);
		}

		{
			ITEMatrix3x3 paintInvMatrix;
			
			iteInvertMatrix(&context->fillTransform, &paintInvMatrix);
			hw->REG_PITR00_BASE = (ITEs15p16)(paintInvMatrix.m[0][0] * 0x10000 + 1.0f);
			hw->REG_PITR01_BASE = (ITEs15p16)(paintInvMatrix.m[0][1] * 0x10000 + 1.0f);
			hw->REG_PITR02_BASE = (ITEs15p16)(paintInvMatrix.m[0][2] * 0x10000 + 1.0f);
			hw->REG_PITR10_BASE = (ITEs15p16)(paintInvMatrix.m[1][0] * 0x10000 + 1.0f);
			hw->REG_PITR11_BASE = (ITEs15p16)(paintInvMatrix.m[1][1] * 0x10000 + 1.0f);
			hw->REG_PITR12_BASE = (ITEs15p16)(paintInvMatrix.m[1][2] * 0x10000 + 1.0f);
		}

		if ( paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT )
		{
			ITEfloat R;
			ITEVector2 u,v,w;

			/*
				A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
				B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
				C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			*/
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f )
			{
				R = 1.0f;
			}
			R = 1.0f/R;

			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000);
			hw->REG_GPRB_BASE = (ITEs15p16)(R * w.y * 0x10000);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (w.x * u.x + w.y * u.y) * 0x10000);	
		}
		else if ( paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;
			ITEint64 gradientValue = 0;

			/*
				R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
				A = (fx-cx) * R
				B = (fy-cy) * R
				C = - (fx(fx-cx) + fy(fy-cy)) * R
				D = (r^2 + (fy-cy)^2) * R^2;
				E = (r^2 + (fx-cx)^2) * R^2;
				F = 2*(fx-cx)(fy-cy) * R^2
				G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
				H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
				I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			*/
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;

			/* s7.12 */
			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000 + 0.5f);
			hw->REG_GPRB_BASE = (ITEs15p16)(R*w.y*0x10000 + 0.5f);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (v.x*w.x + v.y*w.y) * 0x10000 + 0.5f);

			/* s13.24 */
			// D
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.y*w.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRD0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRD1_BASE = (ITEuint32)gradientValue;
			// E
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRE0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRE1_BASE = (ITEuint32)gradientValue;
			// F
			gradientValue = (ITEint64)((double)(2*R*R*w.x*w.y) * 0x1000000 + 0.5f);
			hw->REG_GPRF0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRF1_BASE = (ITEuint32)gradientValue;
			// G
			gradientValue = (ITEint64)((double)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRG0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRG1_BASE = (ITEuint32)gradientValue;
			// H
			gradientValue = (ITEint64)((double)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRH0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRH1_BASE = (ITEuint32)gradientValue;
			// I
			gradientValue = (ITEint64)((double)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRI0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRI1_BASE = (ITEuint32)gradientValue;
		}
		else
		{
			ITEuint16 preR, preG, preB;
				
			preR = (ITEuint16)paint->color.r * paint->color.a;
			preG = (ITEuint16)paint->color.g * paint->color.a;
			preB = (ITEuint16)paint->color.b * paint->color.a;

			preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
			preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
			preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;

			hw->REG_RCR00_BASE = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) |
				                 (paint->color.a & ITE_VG_CMDMASK_RAMPCOLOR0A);
			hw->REG_RCR01_BASE = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0R) |
				                 (preG & ITE_VG_CMDMASK_RAMPCOLOR0G);
		    iteHardwareFire(hw);
		}

		/* Fill gradient parameter */
		{
			ITEfloat   lastOffset       = 0.0f;
			//ITEint32   stopNumber       = 0;
			ITEint32   itemIndex        = 0;
			ITEint32   startIndex       = 0;
			ITEboolean rampEnd     = ITE_FALSE;
			ITEboolean even             = ITE_TRUE;
			ITEuint32* currRampStopReg  = &hw->REG_RSR01_BASE;
			ITEuint32* currRampColorReg = &hw->REG_RCR00_BASE;
			ITEuint32* currDividerReg   = &hw->REG_RDR01_BASE;

			/* Disable all ramp stop registers */
			for ( itemIndex = 0; itemIndex < 8; itemIndex++ )
			{
				*currRampStopReg = 0;
				currRampStopReg++;
			}

			/* Restore */
			currRampStopReg  = &hw->REG_RSR01_BASE;
			currRampColorReg = &hw->REG_RCR00_BASE;
			currDividerReg   = &hw->REG_RDR01_BASE;

			/* Fill first 8 ramp stop value */
			for ( startIndex = 0, even = ITE_TRUE, itemIndex = 0; 
				  itemIndex < paint->stops.size; 
				  itemIndex++, even = !even )
			{
				ITEStop*  pStop     = &paint->stops.items[itemIndex];
				ITEColor  stopColor = pStop->color;
				ITEuint16 preR, preG, preB;

				if ( rampEnd == ITE_TRUE )
				{
					rampEnd = ITE_FALSE;
					startIndex = itemIndex;
				}

				/* Offset */
				if ( even )
				{
					*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0VLD;

					if (   (startIndex == itemIndex)
						&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					{
						*currRampStopReg |= ITE_VG_CMD_RAMPSTOP0EQ;
					}
				}
				else
				{
					*currRampStopReg |= (((ITEuint32)(pStop->offset * (1 << 12))) << ITE_VG_CMDSHIFT_RAMPSTOP1) | ITE_VG_CMD_RAMPSTOP1VLD;
					currRampStopReg++;
				}

				/* Color */
				if ( paint->premultiplied == VG_FALSE )
				{
					/* Transform color to pre-multiplied */
					/*
					stopColor.r = (ITEuint8)((float)stopColor.r * (float)stopColor.a/255.0f);
					stopColor.g = (ITEuint8)((float)stopColor.g * (float)stopColor.a/255.0f);
					stopColor.b = (ITEuint8)((float)stopColor.b * (float)stopColor.a/255.0f);
					*/

					preR = (ITEuint16)stopColor.r * stopColor.a;
					preG = (ITEuint16)stopColor.g * stopColor.a;
					preB = (ITEuint16)stopColor.b * stopColor.a;

					preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
					preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
					preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
				}
				else
				{
					preR = ((ITEuint16)stopColor.r) << 4;
					preG = ((ITEuint16)stopColor.g) << 4;
					preB = ((ITEuint16)stopColor.b) << 4;
				}
				*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
				currRampColorReg++;
				*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
				currRampColorReg++;

				/* Divider */
				if ( itemIndex > startIndex )
				{
					ITEs15p16 delta = (ITEs15p16)((pStop->offset - lastOffset) * (1 << 12));
					
					if ( delta )
					{
						*currDividerReg = ((ITEs15p16)(1 << 24)) / delta;
					}
					else
					{
						*currDividerReg = 0;
					}
					currDividerReg++;
				}

				lastOffset = pStop->offset;

				/* Engine fire */
				if (   ( itemIndex > startIndex )
					&& ( itemIndex % 7) == 0 )
				{
					ITEint32 i = 0;

					iteHardwareFire(hw);

					/* Disable all ramp stop registers */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
					for ( i = 0; i < 8; i++ )
					{
						*currRampStopReg = 0;
						currRampStopReg++;
					}

					/* Step back */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
					//lastOffset       = 0.0f;
					itemIndex--;
					rampEnd = ITE_TRUE;
				}
			}

			//if (   (paint->stops.size > 8)
			//	|| (paint->stops.size % 8) )
			if ( paint->stops.size % 8 )
			{
				/* Handle remaining stops, then fire engine */
				iteHardwareFire(hw);
			}
		}

#if 0
		{
			ITEfloat   lastOffset       = 0.0f;
			ITEint32   stopNumber       = 0;
			ITEint32   itemIndex        = 0;
			ITEboolean headItem         = ITE_FALSE;
			ITEuint32* currRampStopReg  = &hw->REG_RSR01_BASE;
			ITEuint32* currRampColorReg = &hw->REG_RCR00_BASE;
			ITEuint32* currDividerReg   = &hw->REG_RDR01_BASE;

			/* Disable all ramp stop registers */
			for ( itemIndex = 0; itemIndex < 8; itemIndex++ )
			{
				*currRampStopReg = 0;
				currRampStopReg++;
			}

			/* Restore */
			currRampStopReg  = &hw->REG_RSR01_BASE;
			currRampColorReg = &hw->REG_RCR00_BASE;
			currDividerReg   = &hw->REG_RDR01_BASE;

			/* Fill first 8 ramp stop value */
			for ( headItem = ITE_TRUE, itemIndex = 0, stopNumber = 0; itemIndex < paint->stops.size; stopNumber++ )
			{
				ITEStop*  pStop     = &paint->stops.items[itemIndex];
				ITEColor  stopColor = pStop->color;
				ITEuint16 preR, preG, preB;

				if (   (headItem == ITE_TRUE)
					&& (itemIndex > 0) )
				{
					pStop = &paint->stops.items[itemIndex - 1];
				}

				/* Offset */
				if ( stopNumber & 0x01 )
				{
					*currRampStopReg |= (((ITEuint32)(pStop->offset * (1 << 12))) << ITE_VG_CMDSHIFT_RAMPSTOP1) | ITE_VG_CMD_RAMPSTOP1VLD;
					currRampStopReg++;
				}
				else
				{
					*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0VLD;

					/*  Enable/Disable gradient round */
					//if (   (itemIndex < 8)
					//	&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					if (   (headItem == ITE_TRUE)
						&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					{
						*currRampStopReg |= ITE_VG_CMD_RAMPSTOP0EQ;
					}
				}

				/* Color */
				if ( paint->premultiplied == VG_FALSE )
				{
					/* Transform color to pre-multiplied */
					/*
					stopColor.r = (ITEuint8)((float)stopColor.r * (float)stopColor.a/255.0f);
					stopColor.g = (ITEuint8)((float)stopColor.g * (float)stopColor.a/255.0f);
					stopColor.b = (ITEuint8)((float)stopColor.b * (float)stopColor.a/255.0f);
					*/

					preR = (ITEuint16)stopColor.r * stopColor.a;
					preG = (ITEuint16)stopColor.g * stopColor.a;
					preB = (ITEuint16)stopColor.b * stopColor.a;

					preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
					preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
					preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
				}
				else
				{
					preR = ((ITEuint16)stopColor.r) << 4;
					preG = ((ITEuint16)stopColor.g) << 4;
					preB = ((ITEuint16)stopColor.b) << 4;
				}
				*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
				currRampColorReg++;
				*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
				currRampColorReg++;

				/* Divider */
				if ( itemIndex > 0 )
				{
					ITEs15p16 delta = (ITEs15p16)((pStop->offset - lastOffset) * (1 << 12));
					
					if ( delta )
					{
						*currDividerReg = ((ITEs15p16)(1 << 24)) / delta;
					}
					else
					{
						*currDividerReg = 0;
					}
					currDividerReg++;
				}

				lastOffset = pStop->offset;

				/* Engine fire */
				//if (   (itemIndex > 0) 
				//	&& (itemIndex % 7 == 0) )
				if ( headItem == ITE_TRUE
					&& itemIndex > 0 )
				{
					iteHardwareFire(hw);

					/* Step back */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
					headItem         = ITE_FALSE;
					itemIndex--;
				}
				//else
				{
					itemIndex++;
				}

				if ( (itemIndex % 7 == 0) )
				{
					headItem = ITE_TRUE;
				}
			}

			if (   (paint->stops.size > 8)
				|| (paint->stops.size % 8) )
			{
				/* Handle remaining stops, then fire engine */
				iteHardwareFire(hw);
			}
		}
#endif

		if ( hwWaitObjID == ITE_TRUE )
		{
			iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());
		}
	}

	if ((paintModes & VG_STROKE_PATH) && context->strokeLineWidth > 0.0f) 
	{
		ITEHardwareRegister* hw                    = &context->hardware;
		ITEPaint*            paint                 = NULL;
		ITEint               minterLength          = 100*16;
		ITEboolean           enPerspective         = ITE_FALSE;
		ITEuint32            hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;
		ITEuint32            hwImageQuality        = ITE_VG_CMD_IMAGEQ_NONAA;
		ITEuint32            hwFillRule            = ITE_VG_CMD_FILLRULE_ODDEVEN;
		ITEImage*            hwCoverageImage       = NULL;
		ITEImage*            hwValidImage		   = NULL;
		ITEuint32            hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
		ITEboolean           hwEnPreMulDstImage    = ITE_FALSE; // 0x0B0[31]
		ITEboolean           hwEnUnpreColorTrans   = ITE_FALSE; // 0x0B0[30]
		ITEboolean           hwEnPreMulBlending    = ITE_FALSE; // 0x0B0[29]
		ITEboolean           hwEnPreMulTexImage    = ITE_FALSE; // 0x0B0[28]
		ITEboolean           hwEnGamma             = ITE_FALSE;
		ITEuint32            hwGammaMode           = ITE_VG_CMD_GAMMAMODE_INVERSE;
		ITEboolean           hwWaitObjID           = ITE_FALSE;

		paint = (context->strokePaint) ? context->strokePaint : &context->defaultPaint;

		if (   pathMatrix.m[2][0] 
			|| pathMatrix.m[2][1] 
			|| pathMatrix.m[2][2] != 1.0f )
		{
			enPerspective = ITE_TRUE;
		}

		/* Get render quality parameter */
		switch (context->renderingQuality)
		{
		default:
		case VG_RENDERING_QUALITY_NONANTIALIASED:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;  
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_FASTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_FASTER;
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_BETTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_BETTER; 
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_TWOBYTES;
			break;
		}

		/* Get image quality parameter */
		switch (context->imageQuality)
		{
		default:
		case VG_IMAGE_QUALITY_NONANTIALIASED: hwImageQuality = ITE_VG_CMD_IMAGEQ_NONAA;  break;
		case VG_IMAGE_QUALITY_FASTER:         hwImageQuality = ITE_VG_CMD_IMAGEQ_FASTER; break;
		case VG_IMAGE_QUALITY_BETTER:         hwImageQuality = ITE_VG_CMD_IMAGEQ_BETTER; break;
		}

		/* Get fill rule parameter */
		// Fill rule was signored in stroked render
		hwFillRule = ITE_VG_CMD_FILLRULE_NONZERO;

		/* Get coverage image parameter */
		if ( context->surface->coverageIndex )
		{
			hwCoverageImage = context->surface->coverageImageB;
			hwValidImage = context->surface->validImageB;
			context->surface->coverageIndex = 0;
		}
		else
		{
			hwCoverageImage = context->surface->coverageImageA;
			hwValidImage = context->surface->validImageA;
			context->surface->coverageIndex = 1;
		}

		/* Get pre/unpre parameter */
		// 0x0B0[28]
		if ( paint && paint->pattern )
		{
			if ( ITEImage_IsPreMultipliedFormat(paint->pattern->vgformat) )
			{
				hwEnPreMulTexImage = ITE_FALSE;
			}
			else
			{
				hwEnPreMulTexImage = ITE_TRUE;
			}
		}
		else
		{
			hwEnPreMulTexImage = ITE_FALSE;
		}
		// 0x0B0[30]
		hwEnUnpreColorTrans = ITE_TRUE;
		// 0x0B0[29]
		if (   context->blendMode != VG_BLEND_SRC
			|| hwRenderQuality != ITE_VG_CMD_RENDERQ_NONAA )
		{
			hwEnPreMulBlending = ITE_TRUE;
		}
		else
		{
			hwEnPreMulBlending = ITE_FALSE;
		}
		// 0x0B0[31]
		if ( ITEImage_IsPreMultipliedFormat(context->surface->colorImage->vgformat) )
		{
			hwEnPreMulDstImage = ITE_FALSE;
		}
		else
		{
			hwEnPreMulDstImage = ITE_TRUE;
		}

		/* Get Gamma/Degamma parameter */
		if (   paint->type == VG_PAINT_TYPE_PATTERN 
			&& paint->pattern )
		{
			if ( paint->pattern->vgformat != context->surface->colorImage->vgformat )
			{
				hwEnGamma = ITE_TRUE;

				if ( ITEImage_IsSrgbFormat(paint->pattern->vgformat) )
				{
					/* sRGB --> lRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
				}
				if ( ITEImage_IsLrgbFormat(paint->pattern->vgformat) )
				{
					/* lRGB --> sRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
				}
			}
			else
			{
				hwEnGamma = ITE_FALSE;
			}
		}
		else
		{
			hwEnGamma = ITE_FALSE;
		}

		/* Set hardware parameter */
		hw->REG_TCR_BASE = ((context->strokeJoinStyle & 0xF) << ITE_VG_CMDSHIFT_JOINTYPE) |
		                   ((context->strokeCapStyle & 0xF) << ITE_VG_CMDSHIFT_CAPTYPE) |
			               ITE_VG_CMD_TRANSFORM_EN |
			               ((context->strokeDashPhaseReset == ITE_TRUE) ? ITE_VG_CMD_DASHPHASERESET : 0) |
			               ITE_VG_CMD_STROKEPATH |
			               ITE_VG_CMD_SHIFTROUNDING |
		                   ITE_VG_CMD_JOINROUND |
			               ITE_VG_CMD_READMEM |
			               ITE_VG_CMD_TELWORK;
		hw->REG_LWR_BASE = ((ITEuint32)(context->strokeLineWidth * (1 << 12)) & ITE_VG_CMDMASK_LINEWIDTH);
		{
			ITEs15p16  hwDashArray[ITE_MAX_DASH_COUNT] = {0};
			ITEuint32  hwDashArrayUsedSize             = 0;
			ITEuint32  dashIndex                       = 0;
			ITEuint32* dprBase                         = &hw->REG_DPR00_BASE;

			iteGenDashParamter(
				context->strokeDashPhase, 
				&context->strokeDashPattern, 
				ITE_MAX_DASH_COUNT, 
				hwDashArray, 
				&hwDashArrayUsedSize);

			hw->REG_SRNR_BASE = (((ITEuint32)(context->strokeMiterLimit * context->strokeLineWidth * (1 << 8) + 0.5f) << ITE_VG_CMDSHIFT_MITERLIMIT) & ITE_VG_CMDMASK_MITERLIMIT) |
				                ITE_MAX_STROKE_DIVIDE_NUMBER;
			hw->REG_TCR_BASE |= (hwDashArrayUsedSize << ITE_VG_CMDSHIFT_DASHCOUNT) & ITE_VG_CMDMASK_DASHCOUNT;

		    for ( dashIndex = 0; (dashIndex <= hwDashArrayUsedSize) && (dashIndex < ITE_MAX_DASH_COUNT); dashIndex++ )
			{
				*dprBase = hwDashArray[dashIndex];
				dprBase++;
			}
		}
		//hw->REG_PLR_BASE = hw->cmdLength * sizeof(ITEuint32);
		hw->REG_PLR_BASE = p->pathCommand.size * sizeof(ITEuint32);

		//allocate vram buffer 
		if ( (p->pathCommand.size * sizeof(ITEuint32)) <= ITE_PATH_COPY_SIZE_THRESHOLD )
		{
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, allocSize, iteHardwareGenObjectID());
			mappedSysRam = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
			
		}
		else
		{
#ifdef _WIN32
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
			mappedSysRam = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
#else
			tessellateCmdBuffer = (ITEuint8*)p->pathCommand.items;
#endif
			hwWaitObjID = ITE_TRUE;
		}
		hw->REG_PBR_BASE = ((ITEuint32)tessellateCmdBuffer) << ITE_VG_CMDSHIFT_PATHBASE;
		hw->REG_BID2_BASE = iteHardwareGetCurrentObjectID();

		hw->REG_UTR00_BASE = (ITEs15p16)(pathMatrix.m[0][0] * 0x10000);
		hw->REG_UTR01_BASE = (ITEs15p16)(pathMatrix.m[0][1] * 0x10000);
		hw->REG_UTR02_BASE = (ITEs15p16)(pathMatrix.m[0][2] * 0x10000);
		hw->REG_UTR10_BASE = (ITEs15p16)(pathMatrix.m[1][0] * 0x10000);
		hw->REG_UTR11_BASE = (ITEs15p16)(pathMatrix.m[1][1] * 0x10000);
		hw->REG_UTR12_BASE = (ITEs15p16)(pathMatrix.m[1][2] * 0x10000);
		hw->REG_UTR20_BASE = (ITEs15p16)(pathMatrix.m[2][0] * 0x10000);
		hw->REG_UTR21_BASE = (ITEs15p16)(pathMatrix.m[2][1] * 0x10000);
		hw->REG_UTR22_BASE = (ITEs15p16)(pathMatrix.m[2][2] * 0x10000);
		hw->REG_CCR_BASE   = ITE_VG_CMD_TIMERDY_EN |
          	                 ITE_VG_CMD_FULLRDY_EN |
          	                 ITE_VG_CMD_CLIPPING |
			                 hwRenderQuality |
			                 hwCoverageFormatBytes;
		hw->REG_CPBR_BASE = ((ITEuint32)hwCoverageImage->data) >> ITE_VG_CMDSHIFT_PLANBASE;
		hw->REG_CVPPR_BASE = (hwValidImage->pitch << ITE_VG_CMDSHIFT_VALIDPITCH) |
			                 (hwCoverageImage->pitch<< ITE_VG_CMDSHIFT_PLANPITCH);
		hw->REG_VPBR_BASE = ((ITEuint32)hwValidImage->data) << ITE_VG_CMDSHIFT_VALIDBASE;
		hw->REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
		hw->REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;
		hw->REG_RCR_BASE = (hwEnPreMulDstImage ? ITE_VG_CMD_DST_PRE_EN : 0) |
			               (hwEnUnpreColorTrans ? ITE_VG_CMD_SRC_NONPRE_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_SRC_PRE_EN : 0) |
			               (hwEnPreMulTexImage ? ITE_VG_CMD_TEX_PRE_EN : 0) |
			               ITE_VG_CMD_DITHER_EN | // always enable dither 
			               ITE_VG_CMD_BLEND_EN |
			               (hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
			               (context->enColorTransform ? ITE_VG_CMD_COLORCLIP_EN : 0) |
			               (context->enColorTransform ? ITE_VG_CMD_COLORXFM_EN : 0) |
						   ((context->masking == VG_TRUE) ? ITE_VG_CMD_MASK_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_DESTINATION_EN : 0) |
			               ITE_VG_CMD_TEXCACHE_EN |
			               ITE_VG_CMD_TEXTURE_EN |
			               (context->scissoring ? ITE_VG_CMD_SCISSOR_EN : 0) |
			               ITE_VG_CMD_COVERAGE_EN |
			               hwImageQuality |
			               hwFillRule |
			               ITE_VG_CMD_RENDERMODE_0 |
			               ITE_VG_CMD_RDPLN_VLD_EN;
		hw->REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
			               ITE_VG_CMD_MASKMODE_INTERSECT |
			               hwGammaMode |
			               ((context->blendMode & 0x0F) << ITE_VG_CMDSHIFT_BLENDMODE) |
		                   ((paint != NULL) ? ((paint->tilingMode & 0x03) << 6) : ITE_VG_CMD_TILEMODE_FILL) |
			               ((paint != NULL) ? ((paint->spreadMode & 0x03) << 4) : ITE_VG_CMD_RAMPMODE_PAD) |
			               ((paint != NULL) ? ((paint->type & 0x03) << 2) : ITE_VG_CMD_PAINTTYPE_COLOR) |
			                context->imageMode & 0x03;
		hw->REG_RFR_BASE = ITE_VG_CMD_MASKEXTEND_EN |
		                   ITE_VG_CMD_DSTEXTEND_EN |
		                   ITE_VG_CMD_SRCEXTEND_EN |
		                   (context->surface->maskImage->vgformat << 16) |
			               (context->surface->colorImage->vgformat << 8) |
			               ((paint && paint->pattern) ? paint->pattern->vgformat : 0);

		/* context->colorTransform = { Sr, Sg, Sb, Sa, Br, Bg, Bb, Ba } */
#if 0
		/* Color Transform, [31:16]=Ba, [15:0]=Sa */
		hw->REG_CTR0_BASE = (((ITEint16)(context->colorTransform[7] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM01) |
		                    (ITEint16)(context->colorTransform[3] * 0x100);
		/* Color Transform, [31:16]=Br, [15:0]=Sr */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[4] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM11) |
		                    (ITEint16)(context->colorTransform[0] * 0x100);
		/* Color Transform, [31:16]=Bg, [15:0]=Sg */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[5] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM21) |
		                    (ITEint16)(context->colorTransform[1] * 0x100);
		/* Color Transform, [31:16]=Bb, [15:0]=Sb */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[6] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM31) |
		                    (ITEint16)(context->colorTransform[2] * 0x100);
#else
	hw->REG_CTBR0_BASE = (ITEint32)(context->colorTransform[7] * 0x100);
	hw->REG_CTBR1_BASE = (ITEint32)(context->colorTransform[4] * 0x100);
	hw->REG_CTBR2_BASE = (ITEint32)(context->colorTransform[5] * 0x100);
    hw->REG_CTBR3_BASE = (ITEint32)(context->colorTransform[6] * 0x100);

	hw->REG_CTSR0_BASE = (((ITEuint32)(context->colorTransform[1] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[0] * 0x100);


	hw->REG_CTSR1_BASE = (((ITEint32)(context->colorTransform[3] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
	                     (ITEint16)(context->colorTransform[2] * 0x100);
#endif
		hw->REG_DCR_BASE = 0;
		hw->REG_DHWR_BASE = (context->surface->colorImage->height << ITE_VG_CMDSHIFT_DSTHEIGHT) |
			                context->surface->colorImage->width;
		hw->REG_DBR_BASE = (ITEuint32)(context->surface->colorImage->data);
		hw->REG_SDPR_BASE = ((paint && paint->pattern) ? paint->pattern->pitch << ITE_VG_CMDSHIFT_SRCPITCH0 : 0) |
			                context->surface->colorImage->pitch;
		hw->REG_SCR_BASE = 0;
		hw->REG_SHWR_BASE = (paint && paint->pattern)
			                ? (paint->pattern->height << ITE_VG_CMDSHIFT_SRCHEIGHT | paint->pattern->width)
			                : 0;
		hw->REG_SBR_BASE = (paint && paint->pattern) ? (ITEuint32)paint->pattern->data : 0;
		hw->REG_MBR_BASE = (ITEuint32)context->surface->maskImage->data;
		hw->REG_SMPR_BASE = ((context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) : 0) |
			                context->surface->maskImage->pitch;
		hw->REG_SCBR_BASE = (context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.data >> ITE_VG_CMDSHIFT_SCISBASE) : 0;
		{
			ITEMatrix3x3 pathInvMatrix;
			
			iteInvertMatrix(&pathMatrix, &pathInvMatrix);
			hw->REG_UITR00_BASE = (ITEs15p16)(pathInvMatrix.m[0][0] * 0x10000);
			hw->REG_UITR01_BASE = (ITEs15p16)(pathInvMatrix.m[0][1] * 0x10000);
			hw->REG_UITR02_BASE = (ITEs15p16)(pathInvMatrix.m[0][2] * 0x10000);
			hw->REG_UITR10_BASE = (ITEs15p16)(pathInvMatrix.m[1][0] * 0x10000);
			hw->REG_UITR11_BASE = (ITEs15p16)(pathInvMatrix.m[1][1] * 0x10000);
			hw->REG_UITR12_BASE = (ITEs15p16)(pathInvMatrix.m[1][2] * 0x10000);
			hw->REG_UITR20_BASE = (ITEs15p16)(pathInvMatrix.m[2][0] * 0x10000);
			hw->REG_UITR21_BASE = (ITEs15p16)(pathInvMatrix.m[2][1] * 0x10000);
			hw->REG_UITR22_BASE = (ITEs15p16)(pathInvMatrix.m[2][2] * 0x10000);
		}

		{
			ITEMatrix3x3 paintInvMatrix;
			
			iteInvertMatrix(&context->fillTransform, &paintInvMatrix);
			hw->REG_PITR00_BASE = (ITEs15p16)(paintInvMatrix.m[0][0] * 0x10000);
			hw->REG_PITR01_BASE = (ITEs15p16)(paintInvMatrix.m[0][1] * 0x10000);
			hw->REG_PITR02_BASE = (ITEs15p16)(paintInvMatrix.m[0][2] * 0x10000);
			hw->REG_PITR10_BASE = (ITEs15p16)(paintInvMatrix.m[1][0] * 0x10000);
			hw->REG_PITR11_BASE = (ITEs15p16)(paintInvMatrix.m[1][1] * 0x10000);
			hw->REG_PITR12_BASE = (ITEs15p16)(paintInvMatrix.m[1][2] * 0x10000);
		}

		if ( paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT )
		{
			ITEfloat R;
			ITEVector2 u,v,w;

			/*
				A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
				B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
				C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			*/
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f )
			{
				R = 1.0f;
			}
			R = 1.0f/R;

			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000);
			hw->REG_GPRB_BASE = (ITEs15p16)(R * w.y * 0x10000);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (w.x * u.x + w.y * u.y) * 0x10000);	
		}
		else if ( paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;
			ITEint64 gradientValue = 0;

			/*
				R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
				A = (fx-cx) * R
				B = (fy-cy) * R
				C = - (fx(fx-cx) + fy(fy-cy)) * R
				D = (r^2 + (fy-cy)^2) * R^2;
				E = (r^2 + (fx-cx)^2) * R^2;
				F = 2*(fx-cx)(fy-cy) * R^2
				G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
				H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
				I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			*/
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;

			/* s7.12 */
			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000 + 0.5f);
			hw->REG_GPRB_BASE = (ITEs15p16)(R*w.y*0x10000 + 0.5f);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (v.x*w.x + v.y*w.y) * 0x10000 + 0.5f);

			/* s13.24 */
			// D
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.y*w.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRD0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRD1_BASE = (ITEuint32)gradientValue;
			// E
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRE0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRE1_BASE = (ITEuint32)gradientValue;
			// F
			gradientValue = (ITEint64)((double)(2*R*R*w.x*w.y) * 0x1000000 + 0.5f);
			hw->REG_GPRF0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRF1_BASE = (ITEuint32)gradientValue;
			// G
			gradientValue = (ITEint64)((double)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRG0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRG1_BASE = (ITEuint32)gradientValue;
			// H
			gradientValue = (ITEint64)((double)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRH0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRH1_BASE = (ITEuint32)gradientValue;
			// I
			gradientValue = (ITEint64)((double)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRI0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRI1_BASE = (ITEuint32)gradientValue;
		}
		else
		{
			hw->REG_RCR00_BASE = ((paint->color.b * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0B) |
				                 (paint->color.a & ITE_VG_CMDMASK_RAMPCOLOR0A);
			hw->REG_RCR01_BASE = ((paint->color.r * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0R) |
				                 ((paint->color.g * 0x10) &ITE_VG_CMDMASK_RAMPCOLOR0G);
		    iteHardwareFire(hw);
		}

		/* Fill gradient parameter */
		{
			ITEfloat   lastOffset       = 0.0f;
			ITEint32   stopNumber        = 0;
			ITEint32   itemIndex        = 0;
			ITEuint32* currRampStopReg  = &hw->REG_RSR01_BASE;
			ITEuint32* currRampColorReg = &hw->REG_RCR00_BASE;
			ITEuint32* currDividerReg   = &hw->REG_RDR01_BASE;

			/* Disable all ramp stop registers */
			for ( itemIndex = 0; itemIndex < 8; itemIndex++ )
			{
				*currRampStopReg = 0;
				currRampStopReg++;
			}

			/* Restore */
			currRampStopReg  = &hw->REG_RSR01_BASE;
			currRampColorReg = &hw->REG_RCR00_BASE;
			currDividerReg   = &hw->REG_RDR01_BASE;

			/* Fill first 8 ramp stop value */
			for ( itemIndex = 0, stopNumber = 0; itemIndex < paint->stops.size; stopNumber++ )
			{
				ITEStop*  pStop     = &paint->stops.items[itemIndex];
				ITEColor  stopColor = pStop->color;
				ITEuint16 preR, preG, preB;

				/* Offset */
				if ( stopNumber & 0x01 )
				{
					*currRampStopReg |= (((ITEuint32)(pStop->offset * (1 << 12))) << ITE_VG_CMDSHIFT_RAMPSTOP1) | ITE_VG_CMD_RAMPSTOP1VLD;
					currRampStopReg++;
				}
				else
				{
					*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0VLD;

					/*  Enable/Disable gradient round */
					if (   (itemIndex < 8)
						&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					{
						*currRampStopReg |= ITE_VG_CMD_RAMPSTOP0EQ;
					}
				}

				/* Color */
				if ( paint->premultiplied == VG_FALSE )
				{
					/* Transform color to pre-multiplied */
					/*
					stopColor.r = (ITEuint8)((float)stopColor.r * (float)stopColor.a/255.0f);
					stopColor.g = (ITEuint8)((float)stopColor.g * (float)stopColor.a/255.0f);
					stopColor.b = (ITEuint8)((float)stopColor.b * (float)stopColor.a/255.0f);
					*/

					preR = (ITEuint16)stopColor.r * stopColor.a;
					preG = (ITEuint16)stopColor.g * stopColor.a;
					preB = (ITEuint16)stopColor.b * stopColor.a;

					preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
					preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
					preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
				}
				else
				{
					preR = ((ITEuint16)stopColor.r) << 4;
					preG = ((ITEuint16)stopColor.g) << 4;
					preB = ((ITEuint16)stopColor.b) << 4;
				}
				*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
				currRampColorReg++;
				*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
				currRampColorReg++;

				/* Divider */
				if ( itemIndex > 0 )
				{
					ITEs15p16 delta = (ITEs15p16)((pStop->offset - lastOffset) * (1 << 12));

					if ( delta )
					{
						*currDividerReg = ((ITEs15p16)(1 << 24)) / delta;
					}
					else
					{
						*currDividerReg = 0;
					}
					currDividerReg++;
				}

				lastOffset = pStop->offset;

				/* Engine fire */
				if (   (itemIndex > 0) 
					&& (itemIndex % 7 == 0) )
				{
					iteHardwareFire(hw);

					/* Step back */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
				}
				else
				{
					itemIndex++;
				}
			}

			if (   (paint->stops.size > 8)
				|| (paint->stops.size % 8) )
			{
				/* Handle remaining stops, then fire engine */
				iteHardwareFire(hw);
			}
		}

		if ( hwWaitObjID == ITE_TRUE )
		{
			iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());
		}
	}

	context->surface->colorImage->objectID = iteHardwareGetCurrentObjectID();

	// clear flag
	//VG_Memset(&context->updateFlag, 0, sizeof(ITEUpdateFlag));
}

#else
void iteDrawPath(ITEPath* p, ITEuint paintModes)
{
	ITEImage covImage, valImage;
	ITEPaint* paint;
	ITEMatrix3x3 m, invm;
	ITEHardware *h;
	ITEColor c;
	HWVector2 covmax, covmin;

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	/*
	// handle scissoring
	if( context->scissoring )
	{
		if( context->scissorImage.data== NULL )
		{
			context->scissorImage.width = context->surface->colorImage->width;
			context->scissorImage.height = context->surface->colorImage->height;
			context->scissorImage.pitch = (context->surface->colorImage->width + 7)>>3;
			context->scissorImage.vgformat = VG_A_1;
			context->scissorImage.data = (ITEuint8*)malloc(context->scissorImage.width*context->scissorImage.pitch);
		}
	}
	else
	{
		if( context->scissorImage.data )
		{
			free(context->scissorImage.data);
			context->scissorImage.data = NULL;
		}
	}

	if( context->updateFlag.scissorRectFlag && context->scissoring)
	{
		ITERectangle* rect;
		ITEint i;
		CSET(c, 0x80, 0, 0, 0);
		iteSetImage(&context->scissorImage, 0, 0, context->scissorImage.width, context->scissorImage.height, c);
		CSET(c, 0, 0, 0, 0);
		for(i=0;i<context->scissor.size;i++)
		{
			rect = &context->scissor.items[i];
		    iteSetImage(&context->scissorImage, (ITEint)rect->x, (ITEint)rect->y, (ITEint)rect->w, (ITEint)rect->h, c);
		}
	}
	*/

	iteFlattenPath(p, 1);
	if (paintModes & VG_FILL_PATH) 
	{
		paint = (context->fillPaint) ? context->fillPaint : &context->defaultPaint;
		//iteGenFillPath(context, p, h->cmdData);
		h->paintMode = HW_FILL_PATH;
		SETMAT(h->pathTransform, (ITEs15p16)(context->pathTransform.m[0][0]*0x10000), (ITEs15p16)(context->pathTransform.m[0][1]*0x10000), (ITEs15p16)(context->pathTransform.m[0][2]*0x10000),
								 (ITEs15p16)(context->pathTransform.m[1][0]*0x10000), (ITEs15p16)(context->pathTransform.m[1][1]*0x10000), (ITEs15p16)(context->pathTransform.m[1][2]*0x10000),
								 (ITEs15p16)(context->pathTransform.m[2][0]*0x10000), (ITEs15p16)(context->pathTransform.m[2][1]*0x10000), (ITEs15p16)(context->pathTransform.m[2][2]*0x10000));
		iteTessllationEngine();
		CSET(c, 0, 0, 0, 0);

		// centrial aligment
		SET2V(covmax,h->max);
		SET2V(covmin,h->min);
		ADD2(covmax,4,4);
		ADD2(covmin,4,4);
		
		covImage.width = (ITEint)((covmax.x>>3) - (covmin.x>>3) + 1);
		covImage.height = (ITEint)((covmax.y>>3) - (covmin.y>>3) + 1);
		covImage.pitch = covImage.width*sizeof(ITEint16);
		covImage.vgformat = VG_sRGB_565;
		covImage.data = (ITEint8*)malloc(covImage.height*covImage.pitch);

		//valImage.width = (ITEint)( ((covmax.x>>3)>>2) - ((covmin.x>>3)>>2) + 1);	// 1 valid bit for 4 coverage pixels
		valImage.width = (ITEint)( (((covmax.x>>6)+1)<<3) - ((covmin.x>>6)<<3) );	// 1 valid bit for 1 coverage pixels, byte aligment
		valImage.height = (ITEint)covImage.height;
		valImage.pitch = ( valImage.width*sizeof(ITEuint8) ) >>3;
		valImage.vgformat = VG_A_1;
		valImage.data = (ITEint8*)malloc(valImage.height*valImage.pitch);

		iteSetImage(&valImage, 0, 0, valImage.width, valImage.height, c);

		context->updateFlag.paintBaseFlag = VG_TRUE;
		context->updateFlag.coverageFlag = VG_TRUE;
		context->updateFlag.destinationFlag = VG_TRUE;
		context->updateFlag.enableFlag = VG_TRUE;
		context->updateFlag.modeFlag = VG_TRUE;
		context->updateFlag.fillMatrixFlag = VG_TRUE;

		if(context->updateFlag.enableFlag)
		{
			h->enMask = context->masking;
			h->enScissor = context->scissoring;
			h->enColorTransform = context->enColorTransform;
			h->enBlend = HW_TRUE;
			h->enTexture = HW_FALSE;
			h->enCoverage = HW_TRUE;
			h->enSrcMultiply = HW_TRUE;
			h->enSrcUnMultiply = HW_TRUE;
			h->enDstMultiply = HW_TRUE;
		}

		if(context->updateFlag.modeFlag)
		{
			h->fillRule = context->fillRule&0x1;
			h->imageQuality = (context->imageQuality>3) ? HW_IMAGE_QUALITY_BETTER : (context->imageQuality - 1);
			h->renderingQuality = context->renderingQuality&0x3;
			h->blendMode = context->blendMode&0xf;
			// h->maskMode = HW_INTERSECT_RENDERMASK;	// For vgRendToMask
			h->imageMode = context->imageMode&0x3;
			h->tilingMode = context->tilingMode&0x3;
		}

		if(context->updateFlag.surfaceFlag)
		{
			h->surfacePitch = (ITEint16)context->surface->colorImage->pitch;
			h->surfaceWidth = context->surface->colorImage->width;
			h->surfaceHeight = context->surface->colorImage->height;
			h->surfaceData = context->surface->colorImage->data;
			h->surfaceFormat = context->surface->colorImage->vgformat;
		}

		if(context->updateFlag.maskFlag)
		{
			h->maskPitch = context->surface->maskImage->pitch;
			h->maskWidth = context->surface->maskImage->width;
			h->maskHeight = context->surface->maskImage->height;
			h->maskData = context->surface->maskImage->data;
			h->maskFormat = context->surface->maskImage->vgformat;
		}

		if( context->updateFlag.paintBaseFlag )
		{
			h->paintColor = paint->color;
			h->paintType = paint->type&0x3;
			h->tilingMode = paint->tilingMode&0x3;
			h->paintMode = HW_FILL_PATH;
		}

		if( context->updateFlag.fillMatrixFlag )
		{
			MULMATMAT(context->pathTransform, context->fillTransform, m);
			iteInvertMatrix(&m, &invm);
			SETMAT(h->fillTransform, (ITEs15p16)(invm.m[0][0]*0x10000), (ITEs15p16)(invm.m[0][1]*0x10000), (ITEs15p16)(invm.m[0][2]*0x10000),
									 (ITEs15p16)(invm.m[1][0]*0x10000), (ITEs15p16)(invm.m[1][1]*0x10000), (ITEs15p16)(invm.m[1][2]*0x10000),
									 (ITEs15p16)(invm.m[2][0]*0x10000), (ITEs15p16)(invm.m[2][1]*0x10000), (ITEs15p16)(invm.m[2][2]*0x10000));
		}

		if( context->updateFlag.linearGradientFlag )
		{
			ITEfloat R;
			ITEVector2 u,v,w;
			//A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
			//B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
			//C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f ) R = 1.0f;			
			R = 1.0f/R;
			h->linearGradientA = (ITEs15p16)(R*w.x*0x10000);
			h->linearGradientB = (ITEs15p16)(R*w.y*0x10000);
			h->linearGradientC = (ITEs15p16)(-1*R*(w.x*u.x + w.y*u.y)*0x10000);	

			h->patternData = h->gradientData;
			h->patternFormat = HW_sRGBA_8888;
			h->patternPitch = 1<<(h->gradientLen+2);
			h->patternWidth = 1<<h->gradientLen;
			h->patternHeight = 1;
			h->spreadMode = paint->spreadMode&0x3;
		}

		if( context->updateFlag.radialGradientFlag )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;

			//R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
			//A = (fx-cx) * R
			//B = (fy-cy) * R
			//C = - (fx(fx-cx) + fy(fy-cy)) * R
			//D = (r^2 + (fy-cy)^2) * R^2;
			//E = (r^2 + (fx-cx)^2) * R^2;
			//F = 2*(fx-cx)(fy-cy) * R^2
			//G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
			//H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
			//I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;
			h->radialGradientA = (ITEs15p16)(R*w.x*0x10000);
			h->radialGradientB = (ITEs15p16)(R*w.y*0x10000);
			h->radialGradientC = (ITEs15p16)(-1*R*(v.x*w.x+v.y*w.y)*0x10000);
			h->radialGradientD = (ITEs15p16)(R*R*(r*r-w.y*w.y)*0x10000);
			h->radialGradientE = (ITEs15p16)(R*R*(r*r-w.x*w.x)*0x10000);
			h->radialGradientF = (ITEs15p16)(2*R*R*w.x*w.y*0x10000);
			h->radialGradientG = (ITEs15p16)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)*0x10000);
			h->radialGradientH = (ITEs15p16)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)*0x10000);
			h->radialGradientI = (ITEs15p16)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)*0x10000);

			h->patternData = h->gradientData;
			h->patternFormat = HW_sRGBA_8888;
			h->patternPitch = 1<<(h->gradientLen+2);
			h->patternWidth = 1<<h->gradientLen;
			h->patternHeight = 1;
			h->spreadMode = paint->spreadMode&0x3;
		}

		if( context->updateFlag.paintPatternFlag )
		{
			h->patternData = paint->pattern->data;
			h->patternFormat = paint->pattern->vgformat;
			h->patternPitch = paint->pattern->pitch;
			h->patternWidth = paint->pattern->width;
			h->patternHeight = paint->pattern->height;
			h->tileFillColor = context->tileFillColor;
		}

		if( context->updateFlag.coverageFlag )
		{
			h->coverageX = covmin.x;
			h->coverageY = covmin.y;
			h->coverageWidth = covImage.width;
			h->coverageHeight = covImage.height;
			h->coverageData = (ITEint16*)covImage.data;
			h->coveragevalidpitch = valImage.pitch;
			h->coverageValid = (ITEuint8*)valImage.data;
		}

		if( context->updateFlag.destinationFlag )
		{
			h->dstX = covmin.x;
			h->dstY = covmin.y;
			h->dstWidth = covImage.width;
			h->dstHeight = covImage.height;
		}

		if( context->updateFlag.colorTransformFlag )
		{
			ITEint i,j;
			for(i=0;i<4;i++)
				for(j=0;j<4;j++)
					h->colorTransform[i][j] = (i==j) ? (ITEint16)(context->colorTransform[i]*0x100) : 0;
			for(i=0;i<4;i++)
				h->colorBias[i] = (ITEint16)(context->colorTransform[i+4]*0x100);
		}

		iteHardwareRender();

		if( covImage.data )
		{
			free(covImage.data);
			covImage.data = NULL;
			
			free(valImage.data);
			valImage.data = NULL;
		}
	}
  
	if ((paintModes & VG_STROKE_PATH) && context->strokeLineWidth > 0.0f) 
	{  
		int i, j=0, phstart=0, phfirst=1, OtherSize;
		float dashPhase;
		paint = (context->strokePaint ? context->strokePaint : &context->defaultPaint);
		//iteGenStrokePath(context, p, h->cmdData);
		h->paintMode = HW_STROKE_PATH;
		h->lineWidth = (ITEs15p16)( context->strokeLineWidth*(1<<POINTPREC) );			// s12.11
		h->strokeCapStyle = context->strokeCapStyle & 0xF;
		h->strokeJoinStyle = context->strokeJoinStyle & 0xF;
		h->strokeMiterLimit = (ITEs12p3)(context->strokeMiterLimit*8 );					// s12.3
		h->strokeRoundLines = 32;
		h->dashMaxCount = context->strokeDashPattern.size;

		/// HW loop first 0 ~ h->dashSize, then 1 ~ h->dashSize
		if (context->strokeDashPattern.size!=0) {
			dashPhase = context->strokeDashPhase;
			h->dashPhaseReset = context->strokeDashPhaseReset;

			for (i=0; i<context->strokeDashPattern.size; i++) {
				if (dashPhase <= context->strokeDashPattern.items[i] || phstart) {
					if (phfirst) {
						if (i & 1)
							h->dashPattern[j] = (ITEs15p16)( (context->strokeDashPattern.items[i]-dashPhase)*(1<<POINTPREC) );	// s12.11
						else 
							h->dashPattern[j] = (ITEs15p16)( (context->strokeDashPattern.items[i]-dashPhase)*(1<<POINTPREC) +
															 (1<<(POINTPREC+12)) );												// s12.11
					} else {
						if (i & 1)
							h->dashPattern[j] = (ITEs15p16)( context->strokeDashPattern.items[i]*(1<<POINTPREC) );				// s12.11
						else
							h->dashPattern[j] = (ITEs15p16)( context->strokeDashPattern.items[i]*(1<<POINTPREC) +
														     (1<<(POINTPREC+12)) );												// s12.11
					}
					phstart = 1;
					phfirst = 0;
					j++;
				} else {
					dashPhase -= context->strokeDashPattern.items[i];
				}
			}

			OtherSize = context->strokeDashPattern.size-j+1;

			for (i=0; i<OtherSize; i++) {
				if (i & 1)
					h->dashPattern[j] = (ITEs15p16)( context->strokeDashPattern.items[i]*(1<<POINTPREC) );	// s12.11
				else
					h->dashPattern[j] = (ITEs15p16)( context->strokeDashPattern.items[i]*(1<<POINTPREC) +
													 (1<<(POINTPREC+12)) );									// s12.11

				j++;
			}
		}
		
		SETMAT(h->pathTransform, (ITEs15p16)(context->pathTransform.m[0][0]*0x10000), (ITEs15p16)(context->pathTransform.m[0][1]*0x10000), (ITEs15p16)(context->pathTransform.m[0][2]*0x10000),
								 (ITEs15p16)(context->pathTransform.m[1][0]*0x10000), (ITEs15p16)(context->pathTransform.m[1][1]*0x10000), (ITEs15p16)(context->pathTransform.m[1][2]*0x10000),
								 (ITEs15p16)(context->pathTransform.m[2][0]*0x10000), (ITEs15p16)(context->pathTransform.m[2][1]*0x10000), (ITEs15p16)(context->pathTransform.m[2][2]*0x10000));
		iteTessllationEngine();
		CSET(c, 0, 0, 0, 0);

		// centrial aligment
		SET2V(covmax,h->max);		
		SET2V(covmin,h->min);
		ADD2(covmax,4,4);
		ADD2(covmin,4,4);
		
		covImage.width = (ITEint)((covmax.x>>3) - (covmin.x>>3) + 1);
		covImage.height = (ITEint)((covmax.y>>3) - (covmin.y>>3) + 1);
		covImage.pitch = covImage.width*sizeof(ITEint16);
		covImage.vgformat = VG_sRGB_565;
		covImage.data = (ITEint8*)malloc(covImage.height*covImage.pitch);

		//valImage.width = (ITEint)( ((covmax.x>>3)>>2) - ((covmin.x>>3)>>2) + 1);	// 1 valid bit for 4 coverage pixels
		valImage.width = (ITEint)( (((covmax.x>>6)+1)<<3) - ((covmin.x>>6)<<3) );	// 1 valid bit for 1 coverage pixels, byte aligment
		valImage.height = (ITEint)covImage.height;
		valImage.pitch = ( valImage.width*sizeof(ITEuint8) ) >>3;
		valImage.vgformat = VG_A_1;
		valImage.data = (ITEint8*)malloc(valImage.height*valImage.pitch);

		iteSetImage(&valImage, 0, 0, valImage.width, valImage.height, c);

		context->updateFlag.paintBaseFlag = VG_TRUE;
		context->updateFlag.coverageFlag = VG_TRUE;
		context->updateFlag.destinationFlag = VG_TRUE;
		context->updateFlag.enableFlag = VG_TRUE;
		context->updateFlag.modeFlag = VG_TRUE;
		context->updateFlag.strokeMatrixFlag = VG_TRUE;

		if(context->updateFlag.enableFlag)
		{
			h->enMask = context->masking;
			h->enScissor = context->scissoring;
			h->enColorTransform = context->enColorTransform;
			h->enBlend = HW_TRUE;
			h->enTexture = HW_FALSE;
			h->enCoverage = HW_TRUE;
			h->enSrcMultiply = HW_TRUE;
			h->enSrcUnMultiply = HW_TRUE;
			h->enDstMultiply = HW_TRUE;
		}

		if(context->updateFlag.modeFlag)
		{
			h->fillRule = context->fillRule&0x1;
			h->imageQuality = (context->imageQuality>3) ? HW_IMAGE_QUALITY_BETTER : (context->imageQuality - 1);
			h->renderingQuality = context->renderingQuality&0x3;
			h->blendMode = context->blendMode&0xf;
			// h->maskMode = HW_INTERSECT_RENDERMASK;	// For vgRendToMask
			h->imageMode = context->imageMode&0x3;
			h->tilingMode = context->tilingMode&0x3;
		}

		if(context->updateFlag.surfaceFlag)
		{
			h->surfacePitch = (ITEint16)context->surface->colorImage->pitch;
			h->surfaceWidth = context->surface->colorImage->width;
			h->surfaceHeight = context->surface->colorImage->height;
			h->surfaceData = context->surface->colorImage->data;
			h->surfaceFormat = context->surface->colorImage->vgformat;
		}

		if(context->updateFlag.maskFlag)
		{
			h->maskPitch = context->surface->maskImage->pitch;
			h->maskWidth = context->surface->maskImage->width;
			h->maskHeight = context->surface->maskImage->height;
			h->maskData = context->surface->maskImage->data;
			h->maskFormat = context->surface->maskImage->vgformat;
		}

		if( context->updateFlag.paintBaseFlag )
		{		
			h->paintColor = paint->color;
			h->paintType = paint->type&0x3;
			h->tilingMode = paint->tilingMode&0x3;
			h->paintMode = HW_STROKE_PATH;
			h->fillRule = HW_NON_ZERO;
		}
		if( context->updateFlag.strokeMatrixFlag )
		{
			MULMATMAT(context->pathTransform, context->strokeTransform, m);
			iteInvertMatrix(&m, &invm);
			SETMAT(h->strokeTransform, (ITEs15p16)(invm.m[0][0]*0x10000), (ITEs15p16)(invm.m[0][1]*0x10000), (ITEs15p16)(invm.m[0][2]*0x10000),
									 (ITEs15p16)(invm.m[1][0]*0x10000), (ITEs15p16)(invm.m[1][1]*0x10000), (ITEs15p16)(invm.m[1][2]*0x10000),
									 (ITEs15p16)(invm.m[2][0]*0x10000), (ITEs15p16)(invm.m[2][1]*0x10000), (ITEs15p16)(invm.m[2][2]*0x10000));
		}

		if( context->updateFlag.linearGradientFlag )
		{
			ITEfloat R;
			ITEVector2 u,v,w;
			//A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
			//B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
			//C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f ) R = 1.0f;			
			R = 1.0f/R;
			h->linearGradientA = (ITEs15p16)(R*w.x*0x10000);
			h->linearGradientB = (ITEs15p16)(R*w.y*0x10000);
			h->linearGradientC = (ITEs15p16)(-1*R*(w.x*u.x + w.y*u.y)*0x10000);	
			h->patternData = h->gradientData;
			h->patternFormat = HW_sRGBA_8888;
			h->patternPitch = 1<<(h->gradientLen+2);
			h->patternWidth = 1<<h->gradientLen;
			h->patternHeight = 1;
			h->spreadMode = paint->spreadMode&0x3;
		}

		if( context->updateFlag.radialGradientFlag )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;

			//R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
			//A = (fx-cx) * R
			//B = (fy-cy) * R
			//C = - (fx(fx-cx) + fy(fy-cy)) * R
			//D = (r^2 + (fy-cy)^2) * R^2;
			//E = (r^2 + (fx-cx)^2) * R^2;
			//F = 2*(fx-cx)(fy-cy) * R^2
			//G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
			//H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
			//I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;
			h->radialGradientA = (ITEs15p16)(R*w.x*0x10000);
			h->radialGradientB = (ITEs15p16)(R*w.y*0x10000);
			h->radialGradientC = (ITEs15p16)(-1*R*(v.x*w.x+v.y*w.y)*0x10000);
			h->radialGradientD = (ITEs15p16)(R*R*(r*r-w.y*w.y)*0x10000);
			h->radialGradientE = (ITEs15p16)(R*R*(r*r-w.x*w.x)*0x10000);
			h->radialGradientF = (ITEs15p16)(2*R*R*w.x*w.y*0x10000);
			h->radialGradientG = (ITEs15p16)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)*0x10000);
			h->radialGradientH = (ITEs15p16)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)*0x10000);
			h->radialGradientI = (ITEs15p16)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)*0x10000);

			h->patternData = h->gradientData;
			h->patternFormat = HW_sRGBA_8888;
			h->patternPitch = 1<<(h->gradientLen+2);
			h->patternWidth = 1<<h->gradientLen;
			h->patternHeight = 1;
			h->spreadMode = paint->spreadMode&0x3;
		}

		if( context->updateFlag.paintPatternFlag )
		{
			h->patternData = paint->pattern->data;
			h->patternFormat = paint->pattern->vgformat;
			h->patternPitch = paint->pattern->pitch;
			h->patternWidth = paint->pattern->width;
			h->patternHeight = paint->pattern->height;
			h->tileFillColor = context->tileFillColor;
		}

		//iteVector2ArrayClear(&p->stroke);
		if( context->updateFlag.coverageFlag )
		{
			h->coverageX = covmin.x;
			h->coverageY = covmin.y;
			h->coverageWidth = covImage.width;
			h->coverageHeight = covImage.height;
			h->coverageData = (ITEint16*)covImage.data;
			h->coveragevalidpitch = valImage.pitch;
			h->coverageValid = (ITEuint8*)valImage.data;
		}

		if( context->updateFlag.destinationFlag )
		{
			h->dstX = covmin.x;
			h->dstY = covmin.y;
			h->dstWidth = covImage.width;
			h->dstHeight = covImage.height;
		}

		if( context->updateFlag.colorTransformFlag )
		{
			ITEint i,j;
			for(i=0;i<4;i++)
				for(j=0;j<4;j++)
					h->colorTransform[i][j] = (i==j) ? (ITEint16)(context->colorTransform[i]*0x100) : 0;
			for(i=0;i<4;i++)
				h->colorBias[i] = (ITEint16)(context->colorTransform[i+4]*0x100);
		}

		iteHardwareRender();

		if( covImage.data )
		{
			free(covImage.data);
			covImage.data = NULL;
		}
	}

	// clear flag
	memset(&context->updateFlag, 0, sizeof(ITEUpdateFlag));

}
#endif

/*-----------------------------------------------------
 * The vgRenderToMask function modifies the current surface mask by applying the
 * given operation to the set of coverage values associated with the rendering of the
 * given path. If paintModes contains VG_FILL_PATH, the path is filled; if it
 * contains VG_STROKE_PATH, the path is stroked. If both are present, the mask
 * operation is performed in two passes, first on the filled path geometry, then on the
 * stroked path geometry.
 *
 *   when drawing a path with vgDrawPath using the given set of paint
 * modes and all current OpenVG state settings that affect path rendering (scissor
 * rectangles, rendering quality, fill rule, stroke parameters, etc.). Paint settings (e.g., paint
 * matrices) are ignored.
 *-----------------------------------------------------*/
void 
iteDrawPathToMask(
	ITEPath*        p, 
	ITEuint         paintModes,
	VGMaskOperation operation)
{
    ITEuint8* tessellateCmdBuffer = NULL;

	VG_GETCONTEXT(VG_NO_RETVAL);
	
	if ( p->cmdDirty == ITE_TRUE )
	{
		itePathCommandArrayClear(&p->pathCommand);
		iteFlattenPath(p, 1, &p->pathCommand);
		p->cmdDirty = ITE_FALSE;
	}
	if (paintModes & VG_FILL_PATH) 
	{
		ITEHardwareRegister* hw                    = &context->hardware;
		ITEPaint*            paint                 = NULL;
		ITEint               minterLength          = 100*16;
		ITEboolean           enPerspective         = ITE_FALSE;
		ITEuint32            hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;
		ITEuint32            hwImageQuality        = ITE_VG_CMD_IMAGEQ_NONAA;
		ITEuint32            hwFillRule            = ITE_VG_CMD_FILLRULE_NONZERO;
		ITEImage*            hwCoverageImage       = NULL;
		ITEImage*            hwValidImage		   = NULL;
		ITEuint32            hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
		ITEboolean           hwEnPreMulDstImage    = ITE_FALSE; // 0x0B0[31]
		ITEboolean           hwEnUnpreColorTrans   = ITE_FALSE; // 0x0B0[30]
		ITEboolean           hwEnPreMulBlending    = ITE_FALSE; // 0x0B0[29]
		ITEboolean           hwEnPreMulTexImage    = ITE_FALSE; // 0x0B0[28]
		ITEboolean           hwEnGamma             = ITE_FALSE;
		ITEuint32            hwGammaMode           = ITE_VG_CMD_GAMMAMODE_INVERSE;
		ITEboolean           hwEnMask              = ITE_FALSE;
		ITEuint32            hwMaskMode            = ITE_VG_CMD_MASKMODE_UNION;
		ITEboolean           hwWaitObjID           = ITE_FALSE;

		paint = (context->fillPaint) ? context->fillPaint : &context->defaultPaint;

		if (   context->pathTransform.m[2][0] 
			|| context->pathTransform.m[2][1] 
			|| context->pathTransform.m[2][2] != 1.0f )
		{
			enPerspective = ITE_TRUE;
		}

		/* Get render quality parameter */
		// Awin: Render quality was ignored in mask render ???
		switch (context->renderingQuality)
		{
		default:
		case VG_RENDERING_QUALITY_NONANTIALIASED:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;  
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_FASTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_FASTER;
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_BETTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_BETTER; 
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_TWOBYTES;
			break;
		}

		/* Get image quality parameter */
		// Image quality was signored in mask render
		hwImageQuality = ITE_VG_CMD_IMAGEQ_FASTER;

		/* Get fill rule parameter */
		// Fill rule was signored in mask render
		hwFillRule = ITE_VG_CMD_FILLRULE_NONZERO;

		/* Get coverage image parameter */
		if ( context->surface->coverageIndex )
		{
			hwCoverageImage = context->surface->coverageImageB;
			hwValidImage = context->surface->validImageB;
			context->surface->coverageIndex = 0;
		}
		else
		{
			hwCoverageImage = context->surface->coverageImageA;
			hwValidImage = context->surface->validImageA;
			context->surface->coverageIndex = 1;
		}

		/* Get pre/unpre parameter */
		// 0x0B0[28]
		if ( paint && paint->pattern )
		{
			if ( ITEImage_IsPreMultipliedFormat(paint->pattern->vgformat) )
			{
				hwEnPreMulTexImage = ITE_FALSE;
			}
			else
			{
				hwEnPreMulTexImage = ITE_TRUE;
			}
		}
		else
		{
			hwEnPreMulTexImage = ITE_FALSE;
		}
		// 0x0B0[30]
		hwEnUnpreColorTrans = ITE_TRUE;
		// 0x0B0[29]
		if (   context->blendMode != VG_BLEND_SRC
			|| hwRenderQuality != ITE_VG_CMD_RENDERQ_NONAA )
		{
			hwEnPreMulBlending = ITE_TRUE;
		}
		else
		{
			hwEnPreMulBlending = ITE_FALSE;
		}
		// 0x0B0[31]
		if ( ITEImage_IsPreMultipliedFormat(context->surface->colorImage->vgformat) )
		{
			hwEnPreMulDstImage = ITE_FALSE;
		}
		else
		{
			hwEnPreMulDstImage = ITE_TRUE;
		}

		/* Get Gamma/Degamma parameter */
		if (   paint->type == VG_PAINT_TYPE_PATTERN 
			&& paint->pattern )
		{
			if ( paint->pattern->vgformat != context->surface->colorImage->vgformat )
			{
				hwEnGamma = ITE_TRUE;

				if ( ITEImage_IsSrgbFormat(paint->pattern->vgformat) )
				{
					/* sRGB --> lRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
				}
				if ( ITEImage_IsLrgbFormat(paint->pattern->vgformat) )
				{
					/* lRGB --> sRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
				}
			}
			else
			{
				hwEnGamma = ITE_FALSE;
			}
		}
		else
		{
			hwEnGamma = ITE_FALSE;
		}

		/* Get mask operation */
		switch(operation)
		{
		default:
		case VG_SET_MASK:       hwEnMask = ITE_FALSE; hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		case VG_UNION_MASK:     hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_UNION;     break;
		case VG_INTERSECT_MASK: hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_INTERSECT; break;
		case VG_SUBTRACT_MASK:  hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		}
		hwEnMask = ITE_FALSE;

		/* Set hardware parameter */
		hw->REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN |
			               ITE_VG_CMD_READMEM |
			               ITE_VG_CMD_TELWORK;
		//hw->REG_PLR_BASE = hw->cmdLength * sizeof(ITEuint32);
		hw->REG_PLR_BASE = p->pathCommand.size * sizeof(ITEuint32);

		//allocate vram buffer 
		if ( (p->pathCommand.size * sizeof(ITEuint32)) <= ITE_PATH_COPY_SIZE_THRESHOLD )
		{
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
			mappedSysRam = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
		}
		else
		{
#ifdef _WIN32
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
			mappedSysRam = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
#else
			tessellateCmdBuffer = (ITEuint8*)p->pathCommand.items;
#endif
			hwWaitObjID = ITE_TRUE;
		}
		hw->REG_PBR_BASE = ((ITEuint32)tessellateCmdBuffer) << ITE_VG_CMDSHIFT_PATHBASE;
		hw->REG_BID2_BASE = iteHardwareGetCurrentObjectID();

		hw->REG_UTR00_BASE = (ITEs15p16)(context->pathTransform.m[0][0] * 0x10000);
		hw->REG_UTR01_BASE = (ITEs15p16)(context->pathTransform.m[0][1] * 0x10000);
		hw->REG_UTR02_BASE = (ITEs15p16)(context->pathTransform.m[0][2] * 0x10000);
		hw->REG_UTR10_BASE = (ITEs15p16)(context->pathTransform.m[1][0] * 0x10000);
		hw->REG_UTR11_BASE = (ITEs15p16)(context->pathTransform.m[1][1] * 0x10000);
		hw->REG_UTR12_BASE = (ITEs15p16)(context->pathTransform.m[1][2] * 0x10000);
		hw->REG_UTR20_BASE = (ITEs15p16)(context->pathTransform.m[2][0] * 0x10000);
		hw->REG_UTR21_BASE = (ITEs15p16)(context->pathTransform.m[2][1] * 0x10000);
		hw->REG_UTR22_BASE = (ITEs15p16)(context->pathTransform.m[2][2] * 0x10000);
		hw->REG_CCR_BASE   = ITE_VG_CMD_TIMERDY_EN |
          	                 ITE_VG_CMD_FULLRDY_EN |
          	                 ITE_VG_CMD_CLIPPING |
			                 hwRenderQuality |
			                 hwCoverageFormatBytes;
		hw->REG_CPBR_BASE = ((ITEuint32)hwCoverageImage->data) >> ITE_VG_CMDSHIFT_PLANBASE;
		hw->REG_CVPPR_BASE = (hwValidImage->pitch << ITE_VG_CMDSHIFT_VALIDPITCH) |
			                 (hwCoverageImage->pitch<< ITE_VG_CMDSHIFT_PLANPITCH);
		hw->REG_VPBR_BASE = ((ITEuint32)hwValidImage->data) << ITE_VG_CMDSHIFT_VALIDBASE;
		hw->REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
		hw->REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;
		hw->REG_RCR_BASE = (hwEnPreMulDstImage ? ITE_VG_CMD_DST_PRE_EN : 0) |
			               (hwEnUnpreColorTrans ? ITE_VG_CMD_SRC_NONPRE_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_SRC_PRE_EN : 0) |
			               (hwEnPreMulTexImage ? ITE_VG_CMD_TEX_PRE_EN : 0) |
			               ITE_VG_CMD_DITHER_EN | // always enable dither 
		                   (hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
			               //(context->enColorTransform ? ITE_VG_CMD_COLORXFM_EN : 0) |
			               ((hwEnMask == ITE_TRUE) ? ITE_VG_CMD_MASK_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_DESTINATION_EN : 0) |
			               ITE_VG_CMD_TEXCACHE_EN |
			               ITE_VG_CMD_TEXTURE_EN |
						   (context->scissoring ? ITE_VG_CMD_SCISSOR_EN : 0) |
			               ITE_VG_CMD_COVERAGE_EN |
			               hwImageQuality |
			               hwFillRule |
			               ITE_VG_CMD_RENDERMODE_0 |
			               ITE_VG_CMD_RDPLN_VLD_EN;
		hw->REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
			               hwMaskMode |
		                   hwGammaMode |
			               ((context->blendMode & 0x0F) << ITE_VG_CMDSHIFT_BLENDMODE) |
		                   ((paint != NULL) ? ((paint->tilingMode & 0x03) << 6) : ITE_VG_CMD_TILEMODE_FILL) |
			               ((paint != NULL) ? ((paint->spreadMode & 0x03) << 4) : ITE_VG_CMD_RAMPMODE_PAD) |
			               ((paint != NULL) ? ((paint->type & 0x03) << 2) : ITE_VG_CMD_PAINTTYPE_COLOR) |
			                context->imageMode & 0x03;
		hw->REG_RFR_BASE = ITE_VG_CMD_MASKEXTEND_EN |
		                   ITE_VG_CMD_DSTEXTEND_EN |
		                   ITE_VG_CMD_SRCEXTEND_EN |
		                   (context->surface->maskImage->vgformat << 16) |
			               (context->surface->maskImage->vgformat << 8) |
			               ((paint && paint->pattern) ? paint->pattern->vgformat : 0);

		/* context->colorTransform = { Sr, Sg, Sb, Sa, Br, Bg, Bb, Ba } */
#if 0
		/* Color Transform, [31:16]=Ba, [15:0]=Sa */
		hw->REG_CTR0_BASE = (((ITEint16)(context->colorTransform[7] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM01) |
		                    (ITEint16)(context->colorTransform[3] * 0x100);
		/* Color Transform, [31:16]=Br, [15:0]=Sr */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[4] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM11) |
		                    (ITEint16)(context->colorTransform[0] * 0x100);
		/* Color Transform, [31:16]=Bg, [15:0]=Sg */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[5] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM21) |
		                    (ITEint16)(context->colorTransform[1] * 0x100);
		/* Color Transform, [31:16]=Bb, [15:0]=Sb */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[6] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM31) |
		                    (ITEint16)(context->colorTransform[2] * 0x100);
#else
	hw->REG_CTBR0_BASE = (ITEint32)(context->colorTransform[7] * 0x100);
	hw->REG_CTBR1_BASE = (ITEint32)(context->colorTransform[4] * 0x100);
	hw->REG_CTBR2_BASE = (ITEint32)(context->colorTransform[5] * 0x100);
    hw->REG_CTBR3_BASE = (ITEint32)(context->colorTransform[6] * 0x100);

	hw->REG_CTSR0_BASE = (((ITEuint32)(context->colorTransform[1] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[0] * 0x100);


	hw->REG_CTSR1_BASE = (((ITEint32)(context->colorTransform[3] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
	                     (ITEint16)(context->colorTransform[2] * 0x100);
#endif
		hw->REG_DCR_BASE = 0;
		hw->REG_DHWR_BASE = (context->surface->colorImage->height << ITE_VG_CMDSHIFT_DSTHEIGHT) |
			                context->surface->colorImage->width;
		//hw->REG_DBR_BASE = (ITEuint32)(context->surface->colorImage->data);
		hw->REG_DBR_BASE = (ITEuint32)context->surface->maskImage->data;
		hw->REG_SDPR_BASE = ((paint && paint->pattern) ? paint->pattern->pitch << ITE_VG_CMDSHIFT_SRCPITCH0 : 0) |
			                context->surface->maskImage->pitch;
		hw->REG_SCR_BASE = 0;
		hw->REG_SHWR_BASE = (paint && paint->pattern)
			                ? (paint->pattern->height << ITE_VG_CMDSHIFT_SRCHEIGHT | paint->pattern->width)
			                : 0;
		hw->REG_SBR_BASE = (paint && paint->pattern) ? (ITEuint32)paint->pattern->data : 0;
		hw->REG_MBR_BASE = (ITEuint32)context->surface->maskImage->data;
		hw->REG_SMPR_BASE = ((context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) : 0) |
			                context->surface->maskImage->pitch;
		hw->REG_SCBR_BASE = (context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.data >> ITE_VG_CMDSHIFT_SCISBASE) : 0;
		{
			ITEMatrix3x3 pathInvMatrix;
			
			iteInvertMatrix(&context->pathTransform, &pathInvMatrix);
			hw->REG_UITR00_BASE = (ITEs15p16)(pathInvMatrix.m[0][0] * 0x10000);
			hw->REG_UITR01_BASE = (ITEs15p16)(pathInvMatrix.m[0][1] * 0x10000);
			hw->REG_UITR02_BASE = (ITEs15p16)(pathInvMatrix.m[0][2] * 0x10000);
			hw->REG_UITR10_BASE = (ITEs15p16)(pathInvMatrix.m[1][0] * 0x10000);
			hw->REG_UITR11_BASE = (ITEs15p16)(pathInvMatrix.m[1][1] * 0x10000);
			hw->REG_UITR12_BASE = (ITEs15p16)(pathInvMatrix.m[1][2] * 0x10000);
			hw->REG_UITR20_BASE = (ITEs15p16)(pathInvMatrix.m[2][0] * 0x10000);
			hw->REG_UITR21_BASE = (ITEs15p16)(pathInvMatrix.m[2][1] * 0x10000);
			hw->REG_UITR22_BASE = (ITEs15p16)(pathInvMatrix.m[2][2] * 0x10000);
		}

		{
			ITEMatrix3x3 paintInvMatrix;

			IDMAT(paintInvMatrix);
			hw->REG_PITR00_BASE = (ITEs15p16)(paintInvMatrix.m[0][0] * 0x10000);
			hw->REG_PITR01_BASE = (ITEs15p16)(paintInvMatrix.m[0][1] * 0x10000);
			hw->REG_PITR02_BASE = (ITEs15p16)(paintInvMatrix.m[0][2] * 0x10000);
			hw->REG_PITR10_BASE = (ITEs15p16)(paintInvMatrix.m[1][0] * 0x10000);
			hw->REG_PITR11_BASE = (ITEs15p16)(paintInvMatrix.m[1][1] * 0x10000);
			hw->REG_PITR12_BASE = (ITEs15p16)(paintInvMatrix.m[1][2] * 0x10000);
		}

		if ( paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT )
		{
			ITEfloat R;
			ITEVector2 u,v,w;

			/*
				A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
				B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
				C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			*/
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f )
			{
				R = 1.0f;
			}
			R = 1.0f/R;

			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000);
			hw->REG_GPRB_BASE = (ITEs15p16)(R * w.y * 0x10000);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (w.x * u.x + w.y * u.y) * 0x10000);	
		}
		else if ( paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;
			ITEint64 gradientValue = 0;

			/*
				R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
				A = (fx-cx) * R
				B = (fy-cy) * R
				C = - (fx(fx-cx) + fy(fy-cy)) * R
				D = (r^2 + (fy-cy)^2) * R^2;
				E = (r^2 + (fx-cx)^2) * R^2;
				F = 2*(fx-cx)(fy-cy) * R^2
				G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
				H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
				I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			*/
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;

			/* s7.12 */
			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000 + 0.5f);
			hw->REG_GPRB_BASE = (ITEs15p16)(R*w.y*0x10000 + 0.5f);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (v.x*w.x + v.y*w.y) * 0x10000 + 0.5f);

			/* s13.24 */
			// D
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.y*w.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRD0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRD1_BASE = (ITEuint32)gradientValue;
			// E
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRE0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRE1_BASE = (ITEuint32)gradientValue;
			// F
			gradientValue = (ITEint64)((double)(2*R*R*w.x*w.y) * 0x1000000 + 0.5f);
			hw->REG_GPRF0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRF1_BASE = (ITEuint32)gradientValue;
			// G
			gradientValue = (ITEint64)((double)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRG0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRG1_BASE = (ITEuint32)gradientValue;
			// H
			gradientValue = (ITEint64)((double)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRH0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRH1_BASE = (ITEuint32)gradientValue;
			// I
			gradientValue = (ITEint64)((double)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRI0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRI1_BASE = (ITEuint32)gradientValue;
		}
		else
		{
			hw->REG_RCR00_BASE = ((paint->color.b * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0B) |
				                 (paint->color.a &ITE_VG_CMDMASK_RAMPCOLOR0A);
			hw->REG_RCR01_BASE = ((paint->color.r * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0R) |
				                 ((paint->color.g * 0x10) &ITE_VG_CMDMASK_RAMPCOLOR0G);
		    iteHardwareFire(hw);
		}

		/* Fill gradient parameter */
		{
			ITEfloat   lastOffset       = 0.0f;
			ITEint32   stopNumber        = 0;
			ITEint32   itemIndex        = 0;
			ITEuint32* currRampStopReg  = &hw->REG_RSR01_BASE;
			ITEuint32* currRampColorReg = &hw->REG_RCR00_BASE;
			ITEuint32* currDividerReg   = &hw->REG_RDR01_BASE;

			/* Disable all ramp stop registers */
			for ( itemIndex = 0; itemIndex < 8; itemIndex++ )
			{
				*currRampStopReg = 0;
				currRampStopReg++;
			}

			/* Restore */
			currRampStopReg  = &hw->REG_RSR01_BASE;
			currRampColorReg = &hw->REG_RCR00_BASE;
			currDividerReg   = &hw->REG_RDR01_BASE;

			/* Fill first 8 ramp stop value */
			for ( itemIndex = 0, stopNumber = 0; itemIndex < paint->stops.size; stopNumber++ )
			{
				ITEStop*  pStop     = &paint->stops.items[itemIndex];
				ITEColor  stopColor = pStop->color;
				ITEuint16 preR, preG, preB;

				/* Offset */
				if ( stopNumber & 0x01 )
				{
					*currRampStopReg |= (((ITEuint32)(pStop->offset * (1 << 12))) << ITE_VG_CMDSHIFT_RAMPSTOP1) | ITE_VG_CMD_RAMPSTOP1VLD;
					currRampStopReg++;
				}
				else
				{
					*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0VLD;

					/*  Enable/Disable gradient round */
					if (   (itemIndex < 8)
						&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					{
						*currRampStopReg |= ITE_VG_CMD_RAMPSTOP0EQ;
					}
				}

				/* Color */
				if ( paint->premultiplied == VG_FALSE )
				{
					/* Transform color to pre-multiplied */
					/*
					stopColor.r = (ITEuint8)((float)stopColor.r * (float)stopColor.a/255.0f);
					stopColor.g = (ITEuint8)((float)stopColor.g * (float)stopColor.a/255.0f);
					stopColor.b = (ITEuint8)((float)stopColor.b * (float)stopColor.a/255.0f);
					*/

					preR = (ITEuint16)stopColor.r * stopColor.a;
					preG = (ITEuint16)stopColor.g * stopColor.a;
					preB = (ITEuint16)stopColor.b * stopColor.a;

					preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
					preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
					preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
				}
				else
				{
					preR = ((ITEuint16)stopColor.r) << 4;
					preG = ((ITEuint16)stopColor.g) << 4;
					preB = ((ITEuint16)stopColor.b) << 4;
				}
				*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
				currRampColorReg++;
				*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
				currRampColorReg++;

				/* Divider */
				if ( itemIndex > 0 )
				{
					*currDividerReg = ((ITEs15p16)(1 << 24)) / ((ITEs15p16)((pStop->offset - lastOffset) * (1 << 12)));
					currDividerReg++;
				}

				lastOffset = pStop->offset;

				/* Engine fire */
				if (   (itemIndex > 0) 
					&& (itemIndex % 7 == 0) )
				{
					iteHardwareFire(hw);

					/* Step back */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
				}
				else
				{
					itemIndex++;
				}
			}

			if (   (paint->stops.size > 8)
				|| (paint->stops.size % 8) )
			{
				/* Handle remaining stops, then fire engine */
				iteHardwareFire(hw);
			}
		}

		if ( hwWaitObjID == ITE_TRUE )
		{
			iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());
		}
	}

	if ((paintModes & VG_STROKE_PATH) && context->strokeLineWidth > 0.0f) 
	{
		ITEHardwareRegister* hw                    = &context->hardware;
		ITEPaint*            paint                 = NULL;
		ITEint               minterLength          = 100*16;
		ITEboolean           enPerspective         = ITE_FALSE;
		ITEuint32            hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;
		ITEuint32            hwImageQuality        = ITE_VG_CMD_IMAGEQ_NONAA;
		ITEuint32            hwFillRule            = ITE_VG_CMD_FILLRULE_NONZERO;
		ITEImage*            hwCoverageImage       = NULL;
		ITEImage*            hwValidImage		   = NULL;
		ITEuint32            hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
		ITEboolean           hwEnPreMulDstImage    = ITE_FALSE; // 0x0B0[31]
		ITEboolean           hwEnUnpreColorTrans   = ITE_FALSE; // 0x0B0[30]
		ITEboolean           hwEnPreMulBlending    = ITE_FALSE; // 0x0B0[29]
		ITEboolean           hwEnPreMulTexImage    = ITE_FALSE; // 0x0B0[28]
		ITEboolean           hwEnGamma             = ITE_FALSE;
		ITEuint32            hwGammaMode           = ITE_VG_CMD_GAMMAMODE_INVERSE;
		ITEboolean           hwEnMask              = ITE_FALSE;
		ITEuint32            hwMaskMode            = ITE_VG_CMD_MASKMODE_UNION;
		ITEboolean           hwWaitObjID           = ITE_FALSE;

		paint = (context->strokePaint) ? context->strokePaint : &context->defaultPaint;

		if (   context->pathTransform.m[2][0] 
			|| context->pathTransform.m[2][1] 
			|| context->pathTransform.m[2][2] != 1.0f )
		{
			enPerspective = ITE_TRUE;
		}

		/* Get render quality parameter */
		// Awin: Image quality was ignored in mask render ???
		switch (context->renderingQuality)
		{
		default:
		case VG_RENDERING_QUALITY_NONANTIALIASED:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;  
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_FASTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_FASTER;
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_BETTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_BETTER; 
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_TWOBYTES;
			break;
		}

		/* Get fill rule parameter */
		// Fill rule was signored in mask render
		hwFillRule = ITE_VG_CMD_FILLRULE_NONZERO;

		/* Get coverage image parameter */
		if ( context->surface->coverageIndex )
		{
			hwCoverageImage = context->surface->coverageImageB;
			hwValidImage = context->surface->validImageB;
			context->surface->coverageIndex = 0;
		}
		else
		{
			hwCoverageImage = context->surface->coverageImageA;
			hwValidImage = context->surface->validImageA;
			context->surface->coverageIndex = 1;
		}

		/* Get pre/unpre parameter */
		// 0x0B0[28]
		if ( paint && paint->pattern )
		{
			if ( ITEImage_IsPreMultipliedFormat(paint->pattern->vgformat) )
			{
				hwEnPreMulTexImage = ITE_FALSE;
			}
			else
			{
				hwEnPreMulTexImage = ITE_TRUE;
			}
		}
		else
		{
			hwEnPreMulTexImage = ITE_FALSE;
		}
		// 0x0B0[30]
		hwEnUnpreColorTrans = ITE_TRUE;
		// 0x0B0[29]
		if (   context->blendMode != VG_BLEND_SRC
			|| hwRenderQuality != ITE_VG_CMD_RENDERQ_NONAA )
		{
			hwEnPreMulBlending = ITE_TRUE;
		}
		else
		{
			hwEnPreMulBlending = ITE_FALSE;
		}
		// 0x0B0[31]
		if ( ITEImage_IsPreMultipliedFormat(context->surface->colorImage->vgformat) )
		{
			hwEnPreMulDstImage = ITE_FALSE;
		}
		else
		{
			hwEnPreMulDstImage = ITE_TRUE;
		}

		/* Get Gamma/Degamma parameter */
		if (   paint->type == VG_PAINT_TYPE_PATTERN 
			&& paint->pattern )
		{
			if ( paint->pattern->vgformat != context->surface->colorImage->vgformat )
			{
				hwEnGamma = ITE_TRUE;

				if ( ITEImage_IsSrgbFormat(paint->pattern->vgformat) )
				{
					/* sRGB --> lRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
				}
				if ( ITEImage_IsLrgbFormat(paint->pattern->vgformat) )
				{
					/* lRGB --> sRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
				}
			}
			else
			{
				hwEnGamma = ITE_FALSE;
			}
		}
		else
		{
			hwEnGamma = ITE_FALSE;
		}

		/* Get mask operation */
		switch(operation)
		{
		default:
		case VG_SET_MASK:       hwEnMask = ITE_FALSE; hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		case VG_UNION_MASK:     hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_UNION;     break;
		case VG_INTERSECT_MASK: hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_INTERSECT; break;
		case VG_SUBTRACT_MASK:  hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		}
		hwEnMask = ITE_FALSE;

		/* Set hardware parameter */
		hw->REG_TCR_BASE = ((context->strokeJoinStyle & 0xF) << ITE_VG_CMDSHIFT_JOINTYPE) |
		                   ((context->strokeCapStyle & 0xF) << ITE_VG_CMDSHIFT_CAPTYPE) |
			               ITE_VG_CMD_TRANSFORM_EN |
			               ((context->strokeDashPhaseReset == ITE_TRUE) ? ITE_VG_CMD_DASHPHASERESET : 0) |
			               ITE_VG_CMD_STROKEPATH |
			               ITE_VG_CMD_READMEM |
			               ITE_VG_CMD_TELWORK;
		hw->REG_LWR_BASE = ((ITEuint32)(context->strokeLineWidth * (1 << 12)) & ITE_VG_CMDMASK_LINEWIDTH);
		{
			ITEs15p16  hwDashArray[ITE_MAX_DASH_COUNT] = {0};
			ITEuint32  hwDashArrayUsedSize             = 0;
			ITEuint32  dashIndex                       = 0;
			ITEuint32* dprBase                         = &hw->REG_DPR00_BASE;

			iteGenDashParamter(
				context->strokeDashPhase, 
				&context->strokeDashPattern, 
				ITE_MAX_DASH_COUNT, 
				hwDashArray, 
				&hwDashArrayUsedSize);

			hw->REG_SRNR_BASE = (((ITEuint32)(context->strokeMiterLimit * context->strokeLineWidth * (1 << 8) + 0.5f) << ITE_VG_CMDSHIFT_MITERLIMIT) & ITE_VG_CMDMASK_MITERLIMIT) |
				                ITE_MAX_STROKE_DIVIDE_NUMBER;
			hw->REG_TCR_BASE |= (hwDashArrayUsedSize << ITE_VG_CMDSHIFT_DASHCOUNT) & ITE_VG_CMDMASK_DASHCOUNT;

		    for ( dashIndex = 0; (dashIndex <= hwDashArrayUsedSize) && (dashIndex < ITE_MAX_DASH_COUNT); dashIndex++ )
			{
				*dprBase = hwDashArray[dashIndex];
				dprBase++;
			}
		}
		//hw->REG_PLR_BASE = hw->cmdLength * sizeof(ITEuint32);
		hw->REG_PLR_BASE = p->pathCommand.size * sizeof(ITEuint32);

		//allocate vram buffer 
		if ( (p->pathCommand.size * sizeof(ITEuint32)) <= ITE_PATH_COPY_SIZE_THRESHOLD )
		{
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
			mappedSysRam = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
		}
		else
		{
#ifdef _WIN32
			ITEuint8* mappedSysRam = NULL;
			ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
			
			tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
			mappedSysRam = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
			VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
			ithFlushDCacheRange(mappedSysRam, allocSize);
			ithUnmapVram(mappedSysRam, allocSize);
#else
			tessellateCmdBuffer = (ITEuint8*)p->pathCommand.items;
#endif
			hwWaitObjID = ITE_TRUE;
		}
		hw->REG_PBR_BASE = ((ITEuint32)tessellateCmdBuffer) << ITE_VG_CMDSHIFT_PATHBASE;
		hw->REG_BID2_BASE = iteHardwareGetCurrentObjectID();

		hw->REG_UTR00_BASE = (ITEs15p16)(context->pathTransform.m[0][0] * 0x10000);
		hw->REG_UTR01_BASE = (ITEs15p16)(context->pathTransform.m[0][1] * 0x10000);
		hw->REG_UTR02_BASE = (ITEs15p16)(context->pathTransform.m[0][2] * 0x10000);
		hw->REG_UTR10_BASE = (ITEs15p16)(context->pathTransform.m[1][0] * 0x10000);
		hw->REG_UTR11_BASE = (ITEs15p16)(context->pathTransform.m[1][1] * 0x10000);
		hw->REG_UTR12_BASE = (ITEs15p16)(context->pathTransform.m[1][2] * 0x10000);
		hw->REG_UTR20_BASE = (ITEs15p16)(context->pathTransform.m[2][0] * 0x10000);
		hw->REG_UTR21_BASE = (ITEs15p16)(context->pathTransform.m[2][1] * 0x10000);
		hw->REG_UTR22_BASE = (ITEs15p16)(context->pathTransform.m[2][2] * 0x10000);
		hw->REG_CCR_BASE   = ITE_VG_CMD_TIMERDY_EN |
          	                 ITE_VG_CMD_FULLRDY_EN |
          	                 ITE_VG_CMD_CLIPPING |
			                 hwRenderQuality |
			                 hwCoverageFormatBytes;
		hw->REG_CPBR_BASE = ((ITEuint32)hwCoverageImage->data) >> ITE_VG_CMDSHIFT_PLANBASE;
		hw->REG_CVPPR_BASE = (hwValidImage->pitch << ITE_VG_CMDSHIFT_VALIDPITCH) |
			                 (hwCoverageImage->pitch<< ITE_VG_CMDSHIFT_PLANPITCH);
		hw->REG_VPBR_BASE = ((ITEuint32)hwValidImage->data) << ITE_VG_CMDSHIFT_VALIDBASE;
		hw->REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
		hw->REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;
		hw->REG_RCR_BASE = (hwEnPreMulDstImage ? ITE_VG_CMD_DST_PRE_EN : 0) |
			               (hwEnUnpreColorTrans ? ITE_VG_CMD_SRC_NONPRE_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_SRC_PRE_EN : 0) |
			               (hwEnPreMulTexImage ? ITE_VG_CMD_TEX_PRE_EN : 0) |
			               ITE_VG_CMD_DITHER_EN | // always enable dither 
		                   (hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
			               //(context->enColorTransform ? ITE_VG_CMD_COLORXFM_EN : 0) |
			               ((hwEnMask == ITE_TRUE) ? ITE_VG_CMD_MASK_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_DESTINATION_EN : 0) |
			               ITE_VG_CMD_TEXCACHE_EN |
			               ITE_VG_CMD_TEXTURE_EN |
						   (context->scissoring ? ITE_VG_CMD_SCISSOR_EN : 0) |
			               ITE_VG_CMD_COVERAGE_EN |
			               hwImageQuality |
			               hwFillRule |
			               ITE_VG_CMD_RENDERMODE_0 |
			               ITE_VG_CMD_RDPLN_VLD_EN;
		hw->REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
			               hwMaskMode |
		                   hwGammaMode |
			               ((context->blendMode & 0x0F) << ITE_VG_CMDSHIFT_BLENDMODE) |
		                   ((paint != NULL) ? ((paint->tilingMode & 0x03) << 6) : ITE_VG_CMD_TILEMODE_FILL) |
			               ((paint != NULL) ? ((paint->spreadMode & 0x03) << 4) : ITE_VG_CMD_RAMPMODE_PAD) |
			               ((paint != NULL) ? ((paint->type & 0x03) << 2) : ITE_VG_CMD_PAINTTYPE_COLOR) |
			                context->imageMode & 0x03;
		hw->REG_RFR_BASE = ITE_VG_CMD_MASKEXTEND_EN |
		                   ITE_VG_CMD_DSTEXTEND_EN |
		                   ITE_VG_CMD_SRCEXTEND_EN |
		                   (context->surface->maskImage->vgformat << 16) |
			               (context->surface->maskImage->vgformat << 8) |
			               ((paint && paint->pattern) ? paint->pattern->vgformat : 0);

		/* context->colorTransform = { Sr, Sg, Sb, Sa, Br, Bg, Bb, Ba } */
#if 0
		/* Color Transform, [31:16]=Ba, [15:0]=Sa */
		hw->REG_CTR0_BASE = (((ITEint16)(context->colorTransform[7] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM01) |
		                    (ITEint16)(context->colorTransform[3] * 0x100);
		/* Color Transform, [31:16]=Br, [15:0]=Sr */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[4] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM11) |
		                    (ITEint16)(context->colorTransform[0] * 0x100);
		/* Color Transform, [31:16]=Bg, [15:0]=Sg */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[5] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM21) |
		                    (ITEint16)(context->colorTransform[1] * 0x100);
		/* Color Transform, [31:16]=Bb, [15:0]=Sb */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[6] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM31) |
		                    (ITEint16)(context->colorTransform[2] * 0x100);
#else
		hw->REG_CTBR0_BASE = (ITEint32)(context->colorTransform[7] * 0x100);
		hw->REG_CTBR1_BASE = (ITEint32)(context->colorTransform[4] * 0x100);
		hw->REG_CTBR2_BASE = (ITEint32)(context->colorTransform[5] * 0x100);
	    hw->REG_CTBR3_BASE = (ITEint32)(context->colorTransform[6] * 0x100);

		hw->REG_CTSR0_BASE = (((ITEuint32)(context->colorTransform[1] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
			                     (ITEint16)(context->colorTransform[0] * 0x100);


		hw->REG_CTSR1_BASE = (((ITEint32)(context->colorTransform[3] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[2] * 0x100);
#endif
		hw->REG_DCR_BASE = 0;
		hw->REG_DHWR_BASE = (context->surface->colorImage->height << ITE_VG_CMDSHIFT_DSTHEIGHT) |
			                context->surface->colorImage->width;
		//hw->REG_DBR_BASE = (ITEuint32)(context->surface->colorImage->data);
		hw->REG_DBR_BASE = (ITEuint32)context->surface->maskImage->data;
		hw->REG_SDPR_BASE = ((paint && paint->pattern) ? paint->pattern->pitch << ITE_VG_CMDSHIFT_SRCPITCH0 : 0) |
			                context->surface->maskImage->pitch;
		hw->REG_SCR_BASE = 0;
		hw->REG_SHWR_BASE = (paint && paint->pattern)
			                ? (paint->pattern->height << ITE_VG_CMDSHIFT_SRCHEIGHT | paint->pattern->width)
			                : 0;
		hw->REG_SBR_BASE = (paint && paint->pattern) ? (ITEuint32)paint->pattern->data : 0;
		hw->REG_MBR_BASE = (ITEuint32)context->surface->maskImage->data;
		hw->REG_SMPR_BASE = ((context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) : 0) |
			                context->surface->maskImage->pitch;
		hw->REG_SCBR_BASE = (context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.data >> ITE_VG_CMDSHIFT_SCISBASE) : 0;
		{
			ITEMatrix3x3 pathInvMatrix;
			
			iteInvertMatrix(&context->pathTransform, &pathInvMatrix);
			hw->REG_UITR00_BASE = (ITEs15p16)(pathInvMatrix.m[0][0] * 0x10000);
			hw->REG_UITR01_BASE = (ITEs15p16)(pathInvMatrix.m[0][1] * 0x10000);
			hw->REG_UITR02_BASE = (ITEs15p16)(pathInvMatrix.m[0][2] * 0x10000);
			hw->REG_UITR10_BASE = (ITEs15p16)(pathInvMatrix.m[1][0] * 0x10000);
			hw->REG_UITR11_BASE = (ITEs15p16)(pathInvMatrix.m[1][1] * 0x10000);
			hw->REG_UITR12_BASE = (ITEs15p16)(pathInvMatrix.m[1][2] * 0x10000);
			hw->REG_UITR20_BASE = (ITEs15p16)(pathInvMatrix.m[2][0] * 0x10000);
			hw->REG_UITR21_BASE = (ITEs15p16)(pathInvMatrix.m[2][1] * 0x10000);
			hw->REG_UITR22_BASE = (ITEs15p16)(pathInvMatrix.m[2][2] * 0x10000);
		}

		{
			ITEMatrix3x3 paintInvMatrix;

			IDMAT(paintInvMatrix);
			hw->REG_PITR00_BASE = (ITEs15p16)(paintInvMatrix.m[0][0] * 0x10000);
			hw->REG_PITR01_BASE = (ITEs15p16)(paintInvMatrix.m[0][1] * 0x10000);
			hw->REG_PITR02_BASE = (ITEs15p16)(paintInvMatrix.m[0][2] * 0x10000);
			hw->REG_PITR10_BASE = (ITEs15p16)(paintInvMatrix.m[1][0] * 0x10000);
			hw->REG_PITR11_BASE = (ITEs15p16)(paintInvMatrix.m[1][1] * 0x10000);
			hw->REG_PITR12_BASE = (ITEs15p16)(paintInvMatrix.m[1][2] * 0x10000);
		}

		if ( paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT )
		{
			ITEfloat R;
			ITEVector2 u,v,w;

			/*
				A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
				B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
				C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			*/
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f )
			{
				R = 1.0f;
			}
			R = 1.0f/R;

			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000);
			hw->REG_GPRB_BASE = (ITEs15p16)(R * w.y * 0x10000);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (w.x * u.x + w.y * u.y) * 0x10000);	
		}
		else if ( paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;
			ITEint64 gradientValue = 0;

			/*
				R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
				A = (fx-cx) * R
				B = (fy-cy) * R
				C = - (fx(fx-cx) + fy(fy-cy)) * R
				D = (r^2 + (fy-cy)^2) * R^2;
				E = (r^2 + (fx-cx)^2) * R^2;
				F = 2*(fx-cx)(fy-cy) * R^2
				G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
				H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
				I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			*/
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;

			/* s7.12 */
			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000 + 0.5f);
			hw->REG_GPRB_BASE = (ITEs15p16)(R*w.y*0x10000 + 0.5f);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (v.x*w.x + v.y*w.y) * 0x10000 + 0.5f);

			/* s13.24 */
			// D
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.y*w.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRD0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRD1_BASE = (ITEuint32)gradientValue;
			// E
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRE0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRE1_BASE = (ITEuint32)gradientValue;
			// F
			gradientValue = (ITEint64)((double)(2*R*R*w.x*w.y) * 0x1000000 + 0.5f);
			hw->REG_GPRF0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRF1_BASE = (ITEuint32)gradientValue;
			// G
			gradientValue = (ITEint64)((double)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRG0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRG1_BASE = (ITEuint32)gradientValue;
			// H
			gradientValue = (ITEint64)((double)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRH0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRH1_BASE = (ITEuint32)gradientValue;
			// I
			gradientValue = (ITEint64)((double)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRI0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRI1_BASE = (ITEuint32)gradientValue;
		}
		else
		{
			hw->REG_RCR00_BASE = ((paint->color.b * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0B) |
				                 (paint->color.a &ITE_VG_CMDMASK_RAMPCOLOR0A);
			hw->REG_RCR01_BASE = ((paint->color.r * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0R) |
				                 ((paint->color.g * 0x10) &ITE_VG_CMDMASK_RAMPCOLOR0G);
		    iteHardwareFire(hw);
		}

		/* Fill gradient parameter */
		{
			ITEfloat   lastOffset       = 0.0f;
			ITEint32   stopNumber        = 0;
			ITEint32   itemIndex        = 0;
			ITEuint32* currRampStopReg  = &hw->REG_RSR01_BASE;
			ITEuint32* currRampColorReg = &hw->REG_RCR00_BASE;
			ITEuint32* currDividerReg   = &hw->REG_RDR01_BASE;

			/* Disable all ramp stop registers */
			for ( itemIndex = 0; itemIndex < 8; itemIndex++ )
			{
				*currRampStopReg = 0;
				currRampStopReg++;
			}

			/* Restore */
			currRampStopReg  = &hw->REG_RSR01_BASE;
			currRampColorReg = &hw->REG_RCR00_BASE;
			currDividerReg   = &hw->REG_RDR01_BASE;

			/* Fill first 8 ramp stop value */
			for ( itemIndex = 0, stopNumber = 0; itemIndex < paint->stops.size; stopNumber++ )
			{
				ITEStop*  pStop     = &paint->stops.items[itemIndex];
				ITEColor  stopColor = pStop->color;
				ITEuint16 preR, preG, preB;

				/* Offset */
				if ( stopNumber & 0x01 )
				{
					*currRampStopReg |= (((ITEuint32)(pStop->offset * (1 << 12))) << ITE_VG_CMDSHIFT_RAMPSTOP1) | ITE_VG_CMD_RAMPSTOP1VLD;
					currRampStopReg++;
				}
				else
				{
					*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0VLD;

					/*  Enable/Disable gradient round */
					if (   (itemIndex < 8)
						&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					{
						*currRampStopReg |= ITE_VG_CMD_RAMPSTOP0EQ;
					}
				}

				/* Color */
				if ( paint->premultiplied == VG_FALSE )
				{
					/* Transform color to pre-multiplied */
					/*
					stopColor.r = (ITEuint8)((float)stopColor.r * (float)stopColor.a/255.0f);
					stopColor.g = (ITEuint8)((float)stopColor.g * (float)stopColor.a/255.0f);
					stopColor.b = (ITEuint8)((float)stopColor.b * (float)stopColor.a/255.0f);
					*/

					preR = (ITEuint16)stopColor.r * stopColor.a;
					preG = (ITEuint16)stopColor.g * stopColor.a;
					preB = (ITEuint16)stopColor.b * stopColor.a;

					preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
					preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
					preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
				}
				else
				{
					preR = ((ITEuint16)stopColor.r) << 4;
					preG = ((ITEuint16)stopColor.g) << 4;
					preB = ((ITEuint16)stopColor.b) << 4;
				}
				*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
				currRampColorReg++;
				*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
				currRampColorReg++;

				/* Divider */
				if ( itemIndex > 0 )
				{
					*currDividerReg = ((ITEs15p16)(1 << 24)) / ((ITEs15p16)((pStop->offset - lastOffset) * (1 << 12)));
					currDividerReg++;
				}

				lastOffset = pStop->offset;

				/* Engine fire */
				if (   (itemIndex > 0) 
					&& (itemIndex % 7 == 0) )
				{
					iteHardwareFire(hw);

					/* Step back */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
				}
				else
				{
					itemIndex++;
				}
			}

			if (   (paint->stops.size > 8)
				|| (paint->stops.size % 8) )
			{
				/* Handle remaining stops, then fire engine */
				iteHardwareFire(hw);
			}
		}

		if ( hwWaitObjID == ITE_TRUE )
		{
			iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());
		}
	}

	// clear flag
	VG_Memset(&context->updateFlag, 0, sizeof(ITEUpdateFlag));
}

/*-----------------------------------------------------
 * The vgRenderToMask function modifies the current surface mask by applying the
 * given operation to the set of coverage values associated with the rendering of the
 * given path. If paintModes contains VG_FILL_PATH, the path is filled; if it
 * contains VG_STROKE_PATH, the path is stroked. If both are present, the mask
 * operation is performed in two passes, first on the filled path geometry, then on the
 * stroked path geometry.
 *
 *   when drawing a path with vgDrawPath using the given set of paint
 * modes and all current OpenVG state settings that affect path rendering (scissor
 * rectangles, rendering quality, fill rule, stroke parameters, etc.). Paint settings (e.g., paint
 * matrices) are ignored.
 *-----------------------------------------------------*/
void 
iteDrawPathToMask2(
	ITEPath*        p, 
	ITEuint         paintModes,
	VGMaskOperation operation,
	ITEImage*       dstMaskImage)
{
    ITEuint8* tessellateCmdBuffer = NULL;

	VG_GETCONTEXT(VG_NO_RETVAL);
	
	if ( p->cmdDirty == ITE_TRUE )
	{
		itePathCommandArrayClear(&p->pathCommand);
		iteFlattenPath(p, 1, &p->pathCommand);
		p->cmdDirty = ITE_FALSE;
	}
	if (paintModes & VG_FILL_PATH) 
	{
		ITEHardwareRegister* hw                    = &context->hardware;
		ITEPaint*            paint                 = NULL;
		ITEint               minterLength          = 100*16;
		ITEboolean           enPerspective         = ITE_FALSE;
		ITEuint32            hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;
		ITEuint32            hwImageQuality        = ITE_VG_CMD_IMAGEQ_NONAA;
		ITEuint32            hwFillRule            = ITE_VG_CMD_FILLRULE_NONZERO;
		ITEImage*            hwCoverageImage       = NULL;
		ITEImage*            hwValidImage		   = NULL;
		ITEuint32            hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
		ITEboolean           hwEnPreMulDstImage    = ITE_FALSE; // 0x0B0[31]
		ITEboolean           hwEnUnpreColorTrans   = ITE_FALSE; // 0x0B0[30]
		ITEboolean           hwEnPreMulBlending    = ITE_FALSE; // 0x0B0[29]
		ITEboolean           hwEnPreMulTexImage    = ITE_FALSE; // 0x0B0[28]
		ITEboolean           hwEnGamma             = ITE_FALSE;
		ITEuint32            hwGammaMode           = ITE_VG_CMD_GAMMAMODE_INVERSE;
		ITEboolean           hwEnMask              = ITE_FALSE;
		ITEuint32            hwMaskMode            = ITE_VG_CMD_MASKMODE_UNION;
		ITEboolean           hwWaitObjID           = ITE_FALSE;

		paint = (context->fillPaint) ? context->fillPaint : &context->defaultPaint;

		if (   context->pathTransform.m[2][0] 
			|| context->pathTransform.m[2][1] 
			|| context->pathTransform.m[2][2] != 1.0f )
		{
			enPerspective = ITE_TRUE;
		}

		/* Get render quality parameter */
		// Awin: Render quality was ignored in mask render ???
		switch (context->renderingQuality)
		{
		default:
		case VG_RENDERING_QUALITY_NONANTIALIASED:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;  
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_FASTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_FASTER;
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_BETTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_BETTER; 
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_TWOBYTES;
			break;
		}

		/* Get image quality parameter */
		// Image quality was signored in mask render
		hwImageQuality = ITE_VG_CMD_IMAGEQ_FASTER;

		/* Get fill rule parameter */
		// Fill rule was signored in mask render
		hwFillRule = ITE_VG_CMD_FILLRULE_NONZERO;

		/* Get coverage image parameter */
		if ( context->surface->coverageIndex )
		{
			hwCoverageImage = context->surface->coverageImageB;
			hwValidImage = context->surface->validImageB;
			context->surface->coverageIndex = 0;
		}
		else
		{
			hwCoverageImage = context->surface->coverageImageA;
			hwValidImage = context->surface->validImageA;
			context->surface->coverageIndex = 1;
		}

		/* Get pre/unpre parameter */
		// 0x0B0[28]
		if ( paint && paint->pattern )
		{
			if ( ITEImage_IsPreMultipliedFormat(paint->pattern->vgformat) )
			{
				hwEnPreMulTexImage = ITE_FALSE;
			}
			else
			{
				hwEnPreMulTexImage = ITE_TRUE;
			}
		}
		else
		{
			hwEnPreMulTexImage = ITE_FALSE;
		}
		// 0x0B0[30]
		hwEnUnpreColorTrans = ITE_TRUE;
		// 0x0B0[29]
		if (   context->blendMode != VG_BLEND_SRC
			|| hwRenderQuality != ITE_VG_CMD_RENDERQ_NONAA )
		{
			hwEnPreMulBlending = ITE_TRUE;
		}
		else
		{
			hwEnPreMulBlending = ITE_FALSE;
		}
		// 0x0B0[31]
		if ( ITEImage_IsPreMultipliedFormat(context->surface->colorImage->vgformat) )
		{
			hwEnPreMulDstImage = ITE_FALSE;
		}
		else
		{
			hwEnPreMulDstImage = ITE_TRUE;
		}

		/* Get Gamma/Degamma parameter */
		if (   paint->type == VG_PAINT_TYPE_PATTERN 
			&& paint->pattern )
		{
			if ( paint->pattern->vgformat != context->surface->colorImage->vgformat )
			{
				hwEnGamma = ITE_TRUE;

				if ( ITEImage_IsSrgbFormat(paint->pattern->vgformat) )
				{
					/* sRGB --> lRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
				}
				if ( ITEImage_IsLrgbFormat(paint->pattern->vgformat) )
				{
					/* lRGB --> sRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
				}
			}
			else
			{
				hwEnGamma = ITE_FALSE;
			}
		}
		else
		{
			hwEnGamma = ITE_FALSE;
		}

		/* Get mask operation */
		switch(operation)
		{
		default:
		case VG_SET_MASK:       hwEnMask = ITE_FALSE; hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		case VG_UNION_MASK:     hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_UNION;     break;
		case VG_INTERSECT_MASK: hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_INTERSECT; break;
		case VG_SUBTRACT_MASK:  hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		}
		hwEnMask = ITE_FALSE;

		/* Set hardware parameter */
		hw->REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN |
			               ITE_VG_CMD_READMEM |
			               ITE_VG_CMD_TELWORK;
		//hw->REG_PLR_BASE = hw->cmdLength * sizeof(ITEuint32);
		hw->REG_PLR_BASE = p->pathCommand.size * sizeof(ITEuint32);

		//allocate vram buffer 
        if ( (p->pathCommand.size * sizeof(ITEuint32)) <= ITE_PATH_COPY_SIZE_THRESHOLD )
        {
        	ITEuint8* mappedSysRam = NULL;
        	ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
        	
        	tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, allocSize, iteHardwareGenObjectID());
        	mappedSysRam = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        	VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
        	ithFlushDCacheRange(mappedSysRam, allocSize);
        	ithUnmapVram(mappedSysRam, allocSize);
        }
        else
        {
#ifdef _WIN32
        	ITEuint8* mappedSysRam = NULL;
        	ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
        	
        	tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
        	mappedSysRam = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        	VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
        	ithFlushDCacheRange(mappedSysRam, allocSize);
        	ithUnmapVram(mappedSysRam, allocSize);
#else
        	tessellateCmdBuffer = (ITEuint8*)p->pathCommand.items;
#endif
        	hwWaitObjID = ITE_TRUE;
        }
		hw->REG_PBR_BASE = ((ITEuint32)tessellateCmdBuffer) << ITE_VG_CMDSHIFT_PATHBASE;
		hw->REG_BID2_BASE = iteHardwareGetCurrentObjectID();

		hw->REG_UTR00_BASE = (ITEs15p16)(context->pathTransform.m[0][0] * 0x10000);
		hw->REG_UTR01_BASE = (ITEs15p16)(context->pathTransform.m[0][1] * 0x10000);
		hw->REG_UTR02_BASE = (ITEs15p16)(context->pathTransform.m[0][2] * 0x10000);
		hw->REG_UTR10_BASE = (ITEs15p16)(context->pathTransform.m[1][0] * 0x10000);
		hw->REG_UTR11_BASE = (ITEs15p16)(context->pathTransform.m[1][1] * 0x10000);
		hw->REG_UTR12_BASE = (ITEs15p16)(context->pathTransform.m[1][2] * 0x10000);
		hw->REG_UTR20_BASE = (ITEs15p16)(context->pathTransform.m[2][0] * 0x10000);
		hw->REG_UTR21_BASE = (ITEs15p16)(context->pathTransform.m[2][1] * 0x10000);
		hw->REG_UTR22_BASE = (ITEs15p16)(context->pathTransform.m[2][2] * 0x10000);
		hw->REG_CCR_BASE   = ITE_VG_CMD_TIMERDY_EN |
          	                 ITE_VG_CMD_FULLRDY_EN |
          	                 ITE_VG_CMD_CLIPPING |
			                 hwRenderQuality |
			                 hwCoverageFormatBytes;
		hw->REG_CPBR_BASE = ((ITEuint32)hwCoverageImage->data) >> ITE_VG_CMDSHIFT_PLANBASE;
		hw->REG_CVPPR_BASE = (hwValidImage->pitch << ITE_VG_CMDSHIFT_VALIDPITCH) |
			                 (hwCoverageImage->pitch<< ITE_VG_CMDSHIFT_PLANPITCH);
		hw->REG_VPBR_BASE = ((ITEuint32)hwValidImage->data) << ITE_VG_CMDSHIFT_VALIDBASE;
		hw->REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
		hw->REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;
		hw->REG_RCR_BASE = (hwEnPreMulDstImage ? ITE_VG_CMD_DST_PRE_EN : 0) |
			               (hwEnUnpreColorTrans ? ITE_VG_CMD_SRC_NONPRE_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_SRC_PRE_EN : 0) |
			               (hwEnPreMulTexImage ? ITE_VG_CMD_TEX_PRE_EN : 0) |
			               ITE_VG_CMD_DITHER_EN | // always enable dither 
		                   (hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
		                   (context->enColorTransform ? ITE_VG_CMD_COLORCLIP_EN : 0) |
			               (context->enColorTransform ? ITE_VG_CMD_COLORXFM_EN : 0) |
			               ((hwEnMask == ITE_TRUE) ? ITE_VG_CMD_MASK_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_DESTINATION_EN : 0) |
			               ITE_VG_CMD_TEXCACHE_EN |
			               ITE_VG_CMD_TEXTURE_EN |
						   (context->scissoring ? ITE_VG_CMD_SCISSOR_EN : 0) |
			               ITE_VG_CMD_COVERAGE_EN |
			               hwImageQuality |
			               hwFillRule |
			               ITE_VG_CMD_RENDERMODE_0 |
			               ITE_VG_CMD_RDPLN_VLD_EN;
		hw->REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
			               hwMaskMode |
		                   hwGammaMode |
			               ((context->blendMode & 0x0F) << ITE_VG_CMDSHIFT_BLENDMODE) |
		                   ((paint != NULL) ? ((paint->tilingMode & 0x03) << 6) : ITE_VG_CMD_TILEMODE_FILL) |
			               ((paint != NULL) ? ((paint->spreadMode & 0x03) << 4) : ITE_VG_CMD_RAMPMODE_PAD) |
			               ((paint != NULL) ? ((paint->type & 0x03) << 2) : ITE_VG_CMD_PAINTTYPE_COLOR) |
			                context->imageMode & 0x03;
		hw->REG_RFR_BASE = ITE_VG_CMD_MASKEXTEND_EN |
		                   ITE_VG_CMD_DSTEXTEND_EN |
		                   ITE_VG_CMD_SRCEXTEND_EN |
		                   (dstMaskImage->vgformat << 16) |
			               (dstMaskImage->vgformat << 8) |
			               ((paint && paint->pattern) ? paint->pattern->vgformat : 0);

		/* context->colorTransform = { Sr, Sg, Sb, Sa, Br, Bg, Bb, Ba } */
#if 0
		/* Color Transform, [31:16]=Ba, [15:0]=Sa */
		hw->REG_CTR0_BASE = (((ITEint16)(context->colorTransform[7] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM01) |
		                    (ITEint16)(context->colorTransform[3] * 0x100);
		/* Color Transform, [31:16]=Br, [15:0]=Sr */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[4] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM11) |
		                    (ITEint16)(context->colorTransform[0] * 0x100);
		/* Color Transform, [31:16]=Bg, [15:0]=Sg */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[5] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM21) |
		                    (ITEint16)(context->colorTransform[1] * 0x100);
		/* Color Transform, [31:16]=Bb, [15:0]=Sb */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[6] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM31) |
		                    (ITEint16)(context->colorTransform[2] * 0x100);
#else
		hw->REG_CTBR0_BASE = (ITEint32)(context->colorTransform[7] * 0x100);
		hw->REG_CTBR1_BASE = (ITEint32)(context->colorTransform[4] * 0x100);
		hw->REG_CTBR2_BASE = (ITEint32)(context->colorTransform[5] * 0x100);
	    hw->REG_CTBR3_BASE = (ITEint32)(context->colorTransform[6] * 0x100);

		hw->REG_CTSR0_BASE = (((ITEuint32)(context->colorTransform[1] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
			                 (ITEint16)(context->colorTransform[0] * 0x100);


		hw->REG_CTSR1_BASE = (((ITEint32)(context->colorTransform[3] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[2] * 0x100);
#endif
		hw->REG_DCR_BASE = 0;
		hw->REG_DHWR_BASE = (context->surface->colorImage->height << ITE_VG_CMDSHIFT_DSTHEIGHT) |
			                context->surface->colorImage->width;
		//hw->REG_DBR_BASE = (ITEuint32)(context->surface->colorImage->data);
		hw->REG_DBR_BASE = (ITEuint32)dstMaskImage->data;
		hw->REG_SDPR_BASE = ((paint && paint->pattern) ? paint->pattern->pitch << ITE_VG_CMDSHIFT_SRCPITCH0 : 0) |
			                dstMaskImage->pitch;
		hw->REG_SCR_BASE = 0;
		hw->REG_SHWR_BASE = (paint && paint->pattern)
			                ? (paint->pattern->height << ITE_VG_CMDSHIFT_SRCHEIGHT | paint->pattern->width)
			                : 0;
		hw->REG_SBR_BASE = (paint && paint->pattern) ? (ITEuint32)paint->pattern->data : 0;
		hw->REG_MBR_BASE = (ITEuint32)dstMaskImage->data;
		hw->REG_SMPR_BASE = ((context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) : 0) |
			                dstMaskImage->pitch;
		hw->REG_SCBR_BASE = (context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.data >> ITE_VG_CMDSHIFT_SCISBASE) : 0;
		{
			ITEMatrix3x3 pathInvMatrix;
			
			iteInvertMatrix(&context->pathTransform, &pathInvMatrix);
			hw->REG_UITR00_BASE = (ITEs15p16)(pathInvMatrix.m[0][0] * 0x10000);
			hw->REG_UITR01_BASE = (ITEs15p16)(pathInvMatrix.m[0][1] * 0x10000);
			hw->REG_UITR02_BASE = (ITEs15p16)(pathInvMatrix.m[0][2] * 0x10000);
			hw->REG_UITR10_BASE = (ITEs15p16)(pathInvMatrix.m[1][0] * 0x10000);
			hw->REG_UITR11_BASE = (ITEs15p16)(pathInvMatrix.m[1][1] * 0x10000);
			hw->REG_UITR12_BASE = (ITEs15p16)(pathInvMatrix.m[1][2] * 0x10000);
			hw->REG_UITR20_BASE = (ITEs15p16)(pathInvMatrix.m[2][0] * 0x10000);
			hw->REG_UITR21_BASE = (ITEs15p16)(pathInvMatrix.m[2][1] * 0x10000);
			hw->REG_UITR22_BASE = (ITEs15p16)(pathInvMatrix.m[2][2] * 0x10000);
		}

		{
			ITEMatrix3x3 paintInvMatrix;

			IDMAT(paintInvMatrix);
			hw->REG_PITR00_BASE = (ITEs15p16)(paintInvMatrix.m[0][0] * 0x10000);
			hw->REG_PITR01_BASE = (ITEs15p16)(paintInvMatrix.m[0][1] * 0x10000);
			hw->REG_PITR02_BASE = (ITEs15p16)(paintInvMatrix.m[0][2] * 0x10000);
			hw->REG_PITR10_BASE = (ITEs15p16)(paintInvMatrix.m[1][0] * 0x10000);
			hw->REG_PITR11_BASE = (ITEs15p16)(paintInvMatrix.m[1][1] * 0x10000);
			hw->REG_PITR12_BASE = (ITEs15p16)(paintInvMatrix.m[1][2] * 0x10000);
		}

		if ( paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT )
		{
			ITEfloat R;
			ITEVector2 u,v,w;

			/*
				A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
				B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
				C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			*/
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f )
			{
				R = 1.0f;
			}
			R = 1.0f/R;

			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000);
			hw->REG_GPRB_BASE = (ITEs15p16)(R * w.y * 0x10000);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (w.x * u.x + w.y * u.y) * 0x10000);	
		}
		else if ( paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;
			ITEint64 gradientValue = 0;

			/*
				R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
				A = (fx-cx) * R
				B = (fy-cy) * R
				C = - (fx(fx-cx) + fy(fy-cy)) * R
				D = (r^2 + (fy-cy)^2) * R^2;
				E = (r^2 + (fx-cx)^2) * R^2;
				F = 2*(fx-cx)(fy-cy) * R^2
				G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
				H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
				I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			*/
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;

			/* s7.12 */
			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000 + 0.5f);
			hw->REG_GPRB_BASE = (ITEs15p16)(R*w.y*0x10000 + 0.5f);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (v.x*w.x + v.y*w.y) * 0x10000 + 0.5f);

			/* s13.24 */
			// D
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.y*w.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRD0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRD1_BASE = (ITEuint32)gradientValue;
			// E
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRE0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRE1_BASE = (ITEuint32)gradientValue;
			// F
			gradientValue = (ITEint64)((double)(2*R*R*w.x*w.y) * 0x1000000 + 0.5f);
			hw->REG_GPRF0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRF1_BASE = (ITEuint32)gradientValue;
			// G
			gradientValue = (ITEint64)((double)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRG0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRG1_BASE = (ITEuint32)gradientValue;
			// H
			gradientValue = (ITEint64)((double)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRH0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRH1_BASE = (ITEuint32)gradientValue;
			// I
			gradientValue = (ITEint64)((double)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRI0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRI1_BASE = (ITEuint32)gradientValue;
		}
		else
		{
			hw->REG_RCR00_BASE = ((paint->color.b * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0B) |
				                 (paint->color.a &ITE_VG_CMDMASK_RAMPCOLOR0A);
			hw->REG_RCR01_BASE = ((paint->color.r * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0R) |
				                 ((paint->color.g * 0x10) &ITE_VG_CMDMASK_RAMPCOLOR0G);
		    iteHardwareFire(hw);
		}

		/* Fill gradient parameter */
		{
			ITEfloat   lastOffset       = 0.0f;
			ITEint32   stopNumber        = 0;
			ITEint32   itemIndex        = 0;
			ITEuint32* currRampStopReg  = &hw->REG_RSR01_BASE;
			ITEuint32* currRampColorReg = &hw->REG_RCR00_BASE;
			ITEuint32* currDividerReg   = &hw->REG_RDR01_BASE;

			/* Disable all ramp stop registers */
			for ( itemIndex = 0; itemIndex < 8; itemIndex++ )
			{
				*currRampStopReg = 0;
				currRampStopReg++;
			}

			/* Restore */
			currRampStopReg  = &hw->REG_RSR01_BASE;
			currRampColorReg = &hw->REG_RCR00_BASE;
			currDividerReg   = &hw->REG_RDR01_BASE;

			/* Fill first 8 ramp stop value */
			for ( itemIndex = 0, stopNumber = 0; itemIndex < paint->stops.size; stopNumber++ )
			{
				ITEStop*  pStop     = &paint->stops.items[itemIndex];
				ITEColor  stopColor = pStop->color;
				ITEuint16 preR, preG, preB;

				/* Offset */
				if ( stopNumber & 0x01 )
				{
					*currRampStopReg |= (((ITEuint32)(pStop->offset * (1 << 12))) << ITE_VG_CMDSHIFT_RAMPSTOP1) | ITE_VG_CMD_RAMPSTOP1VLD;
					currRampStopReg++;
				}
				else
				{
					*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0VLD;

					/*  Enable/Disable gradient round */
					if (   (itemIndex < 8)
						&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					{
						*currRampStopReg |= ITE_VG_CMD_RAMPSTOP0EQ;
					}
				}

				/* Color */
				if ( paint->premultiplied == VG_FALSE )
				{
					/* Transform color to pre-multiplied */
					/*
					stopColor.r = (ITEuint8)((float)stopColor.r * (float)stopColor.a/255.0f);
					stopColor.g = (ITEuint8)((float)stopColor.g * (float)stopColor.a/255.0f);
					stopColor.b = (ITEuint8)((float)stopColor.b * (float)stopColor.a/255.0f);
					*/

					preR = (ITEuint16)stopColor.r * stopColor.a;
					preG = (ITEuint16)stopColor.g * stopColor.a;
					preB = (ITEuint16)stopColor.b * stopColor.a;

					preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
					preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
					preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
				}
				else
				{
					preR = ((ITEuint16)stopColor.r) << 4;
					preG = ((ITEuint16)stopColor.g) << 4;
					preB = ((ITEuint16)stopColor.b) << 4;
				}
				*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
				currRampColorReg++;
				*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
				currRampColorReg++;

				/* Divider */
				if ( itemIndex > 0 )
				{
					*currDividerReg = ((ITEs15p16)(1 << 24)) / ((ITEs15p16)((pStop->offset - lastOffset) * (1 << 12)));
					currDividerReg++;
				}

				lastOffset = pStop->offset;

				/* Engine fire */
				if (   (itemIndex > 0) 
					&& (itemIndex % 7 == 0) )
				{
					iteHardwareFire(hw);

					/* Step back */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
				}
				else
				{
					itemIndex++;
				}
			}

			if (   (paint->stops.size > 8)
				|| (paint->stops.size % 8) )
			{
				/* Handle remaining stops, then fire engine */
				iteHardwareFire(hw);
			}
		}

		if ( hwWaitObjID == ITE_TRUE )
		{
			iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());
		}
	}

	if ((paintModes & VG_STROKE_PATH) && context->strokeLineWidth > 0.0f) 
	{
		ITEHardwareRegister* hw                    = &context->hardware;
		ITEPaint*            paint                 = NULL;
		ITEint               minterLength          = 100*16;
		ITEboolean           enPerspective         = ITE_FALSE;
		ITEuint32            hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;
		ITEuint32            hwImageQuality        = ITE_VG_CMD_IMAGEQ_NONAA;
		ITEuint32            hwFillRule            = ITE_VG_CMD_FILLRULE_NONZERO;
		ITEImage*            hwCoverageImage       = NULL;
		ITEImage*            hwValidImage		   = NULL;
		ITEuint32            hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
		ITEboolean           hwEnPreMulDstImage    = ITE_FALSE; // 0x0B0[31]
		ITEboolean           hwEnUnpreColorTrans   = ITE_FALSE; // 0x0B0[30]
		ITEboolean           hwEnPreMulBlending    = ITE_FALSE; // 0x0B0[29]
		ITEboolean           hwEnPreMulTexImage    = ITE_FALSE; // 0x0B0[28]
		ITEboolean           hwEnGamma             = ITE_FALSE;
		ITEuint32            hwGammaMode           = ITE_VG_CMD_GAMMAMODE_INVERSE;
		ITEboolean           hwEnMask              = ITE_FALSE;
		ITEuint32            hwMaskMode            = ITE_VG_CMD_MASKMODE_UNION;
		ITEboolean           hwWaitObjID           = ITE_FALSE;

		paint = (context->strokePaint) ? context->strokePaint : &context->defaultPaint;

		if (   context->pathTransform.m[2][0] 
			|| context->pathTransform.m[2][1] 
			|| context->pathTransform.m[2][2] != 1.0f )
		{
			enPerspective = ITE_TRUE;
		}

		/* Get render quality parameter */
		// Awin: Image quality was ignored in mask render ???
		switch (context->renderingQuality)
		{
		default:
		case VG_RENDERING_QUALITY_NONANTIALIASED:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_NONAA;  
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_FASTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_FASTER;
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_ONEBYTE;
			break;
			
		case VG_RENDERING_QUALITY_BETTER:
			hwRenderQuality       = ITE_VG_CMD_RENDERQ_BETTER; 
			hwCoverageFormatBytes = ITE_VG_CMD_PLANFORMAT_TWOBYTES;
			break;
		}

		/* Get fill rule parameter */
		// Fill rule was signored in mask render
		hwFillRule = ITE_VG_CMD_FILLRULE_NONZERO;

		/* Get coverage image parameter */
		if ( context->surface->coverageIndex )
		{
			hwCoverageImage = context->surface->coverageImageB;
			hwValidImage = context->surface->validImageB;
			context->surface->coverageIndex = 0;
		}
		else
		{
			hwCoverageImage = context->surface->coverageImageA;
			hwValidImage = context->surface->validImageA;
			context->surface->coverageIndex = 1;
		}

		/* Get pre/unpre parameter */
		// 0x0B0[28]
		if ( paint && paint->pattern )
		{
			if ( ITEImage_IsPreMultipliedFormat(paint->pattern->vgformat) )
			{
				hwEnPreMulTexImage = ITE_FALSE;
			}
			else
			{
				hwEnPreMulTexImage = ITE_TRUE;
			}
		}
		else
		{
			hwEnPreMulTexImage = ITE_FALSE;
		}
		// 0x0B0[30]
		hwEnUnpreColorTrans = ITE_TRUE;
		// 0x0B0[29]
		if (   context->blendMode != VG_BLEND_SRC
			|| hwRenderQuality != ITE_VG_CMD_RENDERQ_NONAA )
		{
			hwEnPreMulBlending = ITE_TRUE;
		}
		else
		{
			hwEnPreMulBlending = ITE_FALSE;
		}
		// 0x0B0[31]
		if ( ITEImage_IsPreMultipliedFormat(context->surface->colorImage->vgformat) )
		{
			hwEnPreMulDstImage = ITE_FALSE;
		}
		else
		{
			hwEnPreMulDstImage = ITE_TRUE;
		}

		/* Get Gamma/Degamma parameter */
		if (   paint->type == VG_PAINT_TYPE_PATTERN 
			&& paint->pattern )
		{
			if ( paint->pattern->vgformat != context->surface->colorImage->vgformat )
			{
				hwEnGamma = ITE_TRUE;

				if ( ITEImage_IsSrgbFormat(paint->pattern->vgformat) )
				{
					/* sRGB --> lRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
				}
				if ( ITEImage_IsLrgbFormat(paint->pattern->vgformat) )
				{
					/* lRGB --> sRGB */
					hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
				}
			}
			else
			{
				hwEnGamma = ITE_FALSE;
			}
		}
		else
		{
			hwEnGamma = ITE_FALSE;
		}

		/* Get mask operation */
		switch(operation)
		{
		default:
		case VG_SET_MASK:       hwEnMask = ITE_FALSE; hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		case VG_UNION_MASK:     hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_UNION;     break;
		case VG_INTERSECT_MASK: hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_INTERSECT; break;
		case VG_SUBTRACT_MASK:  hwEnMask = ITE_TRUE;  hwMaskMode = ITE_VG_CMD_MASKMODE_SUBTRACT;  break;
		}
		hwEnMask = ITE_FALSE;

		/* Set hardware parameter */
		hw->REG_TCR_BASE = ((context->strokeJoinStyle & 0xF) << ITE_VG_CMDSHIFT_JOINTYPE) |
		                   ((context->strokeCapStyle & 0xF) << ITE_VG_CMDSHIFT_CAPTYPE) |
			               ITE_VG_CMD_TRANSFORM_EN |
			               ((context->strokeDashPhaseReset == ITE_TRUE) ? ITE_VG_CMD_DASHPHASERESET : 0) |
			               ITE_VG_CMD_STROKEPATH |
			               ITE_VG_CMD_READMEM |
			               ITE_VG_CMD_TELWORK;
		hw->REG_LWR_BASE = ((ITEuint32)(context->strokeLineWidth * (1 << 12)) & ITE_VG_CMDMASK_LINEWIDTH);
		{
			ITEs15p16  hwDashArray[ITE_MAX_DASH_COUNT] = {0};
			ITEuint32  hwDashArrayUsedSize             = 0;
			ITEuint32  dashIndex                       = 0;
			ITEuint32* dprBase                         = &hw->REG_DPR00_BASE;

			iteGenDashParamter(
				context->strokeDashPhase, 
				&context->strokeDashPattern, 
				ITE_MAX_DASH_COUNT, 
				hwDashArray, 
				&hwDashArrayUsedSize);

			hw->REG_SRNR_BASE = (((ITEuint32)(context->strokeMiterLimit * context->strokeLineWidth * (1 << 8) + 0.5f) << ITE_VG_CMDSHIFT_MITERLIMIT) & ITE_VG_CMDMASK_MITERLIMIT) |
				                ITE_MAX_STROKE_DIVIDE_NUMBER;
			hw->REG_TCR_BASE |= (hwDashArrayUsedSize << ITE_VG_CMDSHIFT_DASHCOUNT) & ITE_VG_CMDMASK_DASHCOUNT;

		    for ( dashIndex = 0; (dashIndex <= hwDashArrayUsedSize) && (dashIndex < ITE_MAX_DASH_COUNT); dashIndex++ )
			{
				*dprBase = hwDashArray[dashIndex];
				dprBase++;
			}
		}
		//hw->REG_PLR_BASE = hw->cmdLength * sizeof(ITEuint32);
		hw->REG_PLR_BASE = p->pathCommand.size * sizeof(ITEuint32);

		//allocate vram buffer 
        if ( (p->pathCommand.size * sizeof(ITEuint32)) <= ITE_PATH_COPY_SIZE_THRESHOLD )
        {
        	ITEuint8* mappedSysRam = NULL;
        	ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
        	
        	tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, allocSize, iteHardwareGenObjectID());
        	mappedSysRam = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        	VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
        	ithFlushDCacheRange(mappedSysRam, allocSize);
        	ithUnmapVram(mappedSysRam, allocSize);
        }
        else
        {
#ifdef _WIN32
        	ITEuint8* mappedSysRam = NULL;
        	ITEuint32 allocSize    = p->pathCommand.size * sizeof(ITEuint32);
        	
        	tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, p->pathCommand.size * sizeof(ITEuint32), iteHardwareGenObjectID());
        	mappedSysRam = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
        	VG_Memcpy(mappedSysRam, p->pathCommand.items, allocSize);
        	ithFlushDCacheRange(mappedSysRam, allocSize);
        	ithUnmapVram(mappedSysRam, allocSize);
#else
        	tessellateCmdBuffer = (ITEuint8*)p->pathCommand.items;
#endif
        	hwWaitObjID = ITE_TRUE;
        }
		hw->REG_PBR_BASE = ((ITEuint32)tessellateCmdBuffer) << ITE_VG_CMDSHIFT_PATHBASE;
		hw->REG_BID2_BASE = iteHardwareGetCurrentObjectID();

		hw->REG_UTR00_BASE = (ITEs15p16)(context->pathTransform.m[0][0] * 0x10000);
		hw->REG_UTR01_BASE = (ITEs15p16)(context->pathTransform.m[0][1] * 0x10000);
		hw->REG_UTR02_BASE = (ITEs15p16)(context->pathTransform.m[0][2] * 0x10000);
		hw->REG_UTR10_BASE = (ITEs15p16)(context->pathTransform.m[1][0] * 0x10000);
		hw->REG_UTR11_BASE = (ITEs15p16)(context->pathTransform.m[1][1] * 0x10000);
		hw->REG_UTR12_BASE = (ITEs15p16)(context->pathTransform.m[1][2] * 0x10000);
		hw->REG_UTR20_BASE = (ITEs15p16)(context->pathTransform.m[2][0] * 0x10000);
		hw->REG_UTR21_BASE = (ITEs15p16)(context->pathTransform.m[2][1] * 0x10000);
		hw->REG_UTR22_BASE = (ITEs15p16)(context->pathTransform.m[2][2] * 0x10000);
		hw->REG_CCR_BASE   = ITE_VG_CMD_TIMERDY_EN |
          	                 ITE_VG_CMD_FULLRDY_EN | 
          	                 ITE_VG_CMD_CLIPPING |
			                 hwRenderQuality |
			                 hwCoverageFormatBytes;
		hw->REG_CPBR_BASE = ((ITEuint32)hwCoverageImage->data) >> ITE_VG_CMDSHIFT_PLANBASE;
		hw->REG_CVPPR_BASE = (hwValidImage->pitch << ITE_VG_CMDSHIFT_VALIDPITCH) |
			                 (hwCoverageImage->pitch<< ITE_VG_CMDSHIFT_PLANPITCH);
		hw->REG_VPBR_BASE = ((ITEuint32)hwValidImage->data) << ITE_VG_CMDSHIFT_VALIDBASE;
		hw->REG_PXCR_BASE = (hwCoverageImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
		hw->REG_PYCR_BASE = (hwCoverageImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;
		hw->REG_RCR_BASE = (hwEnPreMulDstImage ? ITE_VG_CMD_DST_PRE_EN : 0) |
			               (hwEnUnpreColorTrans ? ITE_VG_CMD_SRC_NONPRE_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_SRC_PRE_EN : 0) |
			               (hwEnPreMulTexImage ? ITE_VG_CMD_TEX_PRE_EN : 0) |
			               ITE_VG_CMD_DITHER_EN | // always enable dither 
		                   (hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
		                   (context->enColorTransform ? ITE_VG_CMD_COLORCLIP_EN : 0) |
			               (context->enColorTransform ? ITE_VG_CMD_COLORXFM_EN : 0) |
			               ((hwEnMask == ITE_TRUE) ? ITE_VG_CMD_MASK_EN : 0) |
			               (hwEnPreMulBlending ? ITE_VG_CMD_DESTINATION_EN : 0) |
			               ITE_VG_CMD_TEXCACHE_EN |
			               ITE_VG_CMD_TEXTURE_EN |
						   (context->scissoring ? ITE_VG_CMD_SCISSOR_EN : 0) |
			               ITE_VG_CMD_COVERAGE_EN |
			               hwImageQuality |
			               hwFillRule |
			               ITE_VG_CMD_RENDERMODE_0 |
			               ITE_VG_CMD_RDPLN_VLD_EN;
		hw->REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
			               hwMaskMode |
		                   hwGammaMode |
			               ((context->blendMode & 0x0F) << ITE_VG_CMDSHIFT_BLENDMODE) |
		                   ((paint != NULL) ? ((paint->tilingMode & 0x03) << 6) : ITE_VG_CMD_TILEMODE_FILL) |
			               ((paint != NULL) ? ((paint->spreadMode & 0x03) << 4) : ITE_VG_CMD_RAMPMODE_PAD) |
			               ((paint != NULL) ? ((paint->type & 0x03) << 2) : ITE_VG_CMD_PAINTTYPE_COLOR) |
			                context->imageMode & 0x03;
		hw->REG_RFR_BASE = ITE_VG_CMD_MASKEXTEND_EN |
		                   ITE_VG_CMD_DSTEXTEND_EN |
		                   ITE_VG_CMD_SRCEXTEND_EN |
		                   (dstMaskImage->vgformat << 16) |
			               (dstMaskImage->vgformat << 8) |
			               ((paint && paint->pattern) ? paint->pattern->vgformat : 0);

		/* context->colorTransform = { Sr, Sg, Sb, Sa, Br, Bg, Bb, Ba } */
#if 0
		/* Color Transform, [31:16]=Ba, [15:0]=Sa */
		hw->REG_CTR0_BASE = (((ITEint16)(context->colorTransform[7] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM01) |
		                    (ITEint16)(context->colorTransform[3] * 0x100);
		/* Color Transform, [31:16]=Br, [15:0]=Sr */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[4] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM11) |
		                    (ITEint16)(context->colorTransform[0] * 0x100);
		/* Color Transform, [31:16]=Bg, [15:0]=Sg */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[5] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM21) |
		                    (ITEint16)(context->colorTransform[1] * 0x100);
		/* Color Transform, [31:16]=Bb, [15:0]=Sb */
		hw->REG_CTR1_BASE = (((ITEint16)(context->colorTransform[6] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM31) |
		                    (ITEint16)(context->colorTransform[2] * 0x100);
#else
		hw->REG_CTBR0_BASE = (ITEint32)(context->colorTransform[7] * 0x100);
		hw->REG_CTBR1_BASE = (ITEint32)(context->colorTransform[4] * 0x100);
		hw->REG_CTBR2_BASE = (ITEint32)(context->colorTransform[5] * 0x100);
	    hw->REG_CTBR3_BASE = (ITEint32)(context->colorTransform[6] * 0x100);

		hw->REG_CTSR0_BASE = (((ITEuint32)(context->colorTransform[1] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[0] * 0x100);


		hw->REG_CTSR1_BASE = (((ITEint32)(context->colorTransform[3] * 0x100) << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10) |
		                     (ITEint16)(context->colorTransform[2] * 0x100);
#endif
		hw->REG_DCR_BASE = 0;
		hw->REG_DHWR_BASE = (context->surface->colorImage->height << ITE_VG_CMDSHIFT_DSTHEIGHT) |
			                context->surface->colorImage->width;
		//hw->REG_DBR_BASE = (ITEuint32)(context->surface->colorImage->data);
		hw->REG_DBR_BASE = (ITEuint32)dstMaskImage->data;
		hw->REG_SDPR_BASE = ((paint && paint->pattern) ? paint->pattern->pitch << ITE_VG_CMDSHIFT_SRCPITCH0 : 0) |
			                dstMaskImage->pitch;
		hw->REG_SCR_BASE = 0;
		hw->REG_SHWR_BASE = (paint && paint->pattern)
			                ? (paint->pattern->height << ITE_VG_CMDSHIFT_SRCHEIGHT | paint->pattern->width)
			                : 0;
		hw->REG_SBR_BASE = (paint && paint->pattern) ? (ITEuint32)paint->pattern->data : 0;
		hw->REG_MBR_BASE = (ITEuint32)dstMaskImage->data;
		hw->REG_SMPR_BASE = ((context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) : 0) |
			                dstMaskImage->pitch;
		hw->REG_SCBR_BASE = (context->scissoring && context->scissorImage.data) ? ((ITEuint32)context->scissorImage.data >> ITE_VG_CMDSHIFT_SCISBASE) : 0;
		{
			ITEMatrix3x3 pathInvMatrix;
			
			iteInvertMatrix(&context->pathTransform, &pathInvMatrix);
			hw->REG_UITR00_BASE = (ITEs15p16)(pathInvMatrix.m[0][0] * 0x10000);
			hw->REG_UITR01_BASE = (ITEs15p16)(pathInvMatrix.m[0][1] * 0x10000);
			hw->REG_UITR02_BASE = (ITEs15p16)(pathInvMatrix.m[0][2] * 0x10000);
			hw->REG_UITR10_BASE = (ITEs15p16)(pathInvMatrix.m[1][0] * 0x10000);
			hw->REG_UITR11_BASE = (ITEs15p16)(pathInvMatrix.m[1][1] * 0x10000);
			hw->REG_UITR12_BASE = (ITEs15p16)(pathInvMatrix.m[1][2] * 0x10000);
			hw->REG_UITR20_BASE = (ITEs15p16)(pathInvMatrix.m[2][0] * 0x10000);
			hw->REG_UITR21_BASE = (ITEs15p16)(pathInvMatrix.m[2][1] * 0x10000);
			hw->REG_UITR22_BASE = (ITEs15p16)(pathInvMatrix.m[2][2] * 0x10000);
		}

		{
			ITEMatrix3x3 paintInvMatrix;

			IDMAT(paintInvMatrix);
			hw->REG_PITR00_BASE = (ITEs15p16)(paintInvMatrix.m[0][0] * 0x10000);
			hw->REG_PITR01_BASE = (ITEs15p16)(paintInvMatrix.m[0][1] * 0x10000);
			hw->REG_PITR02_BASE = (ITEs15p16)(paintInvMatrix.m[0][2] * 0x10000);
			hw->REG_PITR10_BASE = (ITEs15p16)(paintInvMatrix.m[1][0] * 0x10000);
			hw->REG_PITR11_BASE = (ITEs15p16)(paintInvMatrix.m[1][1] * 0x10000);
			hw->REG_PITR12_BASE = (ITEs15p16)(paintInvMatrix.m[1][2] * 0x10000);
		}

		if ( paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT )
		{
			ITEfloat R;
			ITEVector2 u,v,w;

			/*
				A = (x1-x0)/((x1-x0)^2 + (y1-y0)^2)
				B = (y1-y0)/(x1-x0)^2 + (y1-y0)^2)
				C = - (x0*(x1-x0) + y0*(y1-y0))/((x1-x0)^2 + (y1-y0)^2)
			*/
			SET2(u, paint->linearGradient[0], paint->linearGradient[1]);
			SET2(v, paint->linearGradient[2], paint->linearGradient[3]);
			SET2(w, v.x-u.x, v.y-u.y);
			R = DOT2(w, w);
			if( R <= 0.0f )
			{
				R = 1.0f;
			}
			R = 1.0f/R;

			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000);
			hw->REG_GPRB_BASE = (ITEs15p16)(R * w.y * 0x10000);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (w.x * u.x + w.y * u.y) * 0x10000);	
		}
		else if ( paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT )
		{
			ITEfloat r, R;
			ITEVector2 u,v,w;
			ITEint64 gradientValue = 0;

			/*
				R = 1.0f / (r^2 - ((fx-cx)^2 + (fy-cy)^2))
				A = (fx-cx) * R
				B = (fy-cy) * R
				C = - (fx(fx-cx) + fy(fy-cy)) * R
				D = (r^2 + (fy-cy)^2) * R^2;
				E = (r^2 + (fx-cx)^2) * R^2;
				F = 2*(fx-cx)(fy-cy) * R^2
				G = 2*( fy(fx-cx)(fy-cy) - fy(fx-cx)(fy-cy) - r^2*fx) * R^2
				H = 2*( fx(fx-cx)(fy-cy) - fx(fx-cx)(fy-cy) - r^2*fy) * R^2
				I = ( r^2*(fx^2+fy^2) + 2*fx*fy*(fx-cx)(fy-cy) - fx^2*(fy-cy)^2 - fy^2*(fx-cx)^2) * R^2
			*/
			SET2(u, paint->radialGradient[0], paint->radialGradient[1]);
			SET2(v, paint->radialGradient[2], paint->radialGradient[3]);
			r = paint->radialGradient[4];
			SET2(w, v.x-u.x, v.y-u.y);
			R = r*r - DOT2(w,w);
			if(R==0) R = 1.0f;
			R = 1.0f/R;

			/* s7.12 */
			hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000 + 0.5f);
			hw->REG_GPRB_BASE = (ITEs15p16)(R*w.y*0x10000 + 0.5f);
			hw->REG_GPRC_BASE = (ITEs15p16)(-1 * R * (v.x*w.x + v.y*w.y) * 0x10000 + 0.5f);

			/* s13.24 */
			// D
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.y*w.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRD0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRD1_BASE = (ITEuint32)gradientValue;
			// E
			gradientValue = (ITEint64)((double)(R*R*(r*r-w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRE0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRE1_BASE = (ITEuint32)gradientValue;
			// F
			gradientValue = (ITEint64)((double)(2*R*R*w.x*w.y) * 0x1000000 + 0.5f);
			hw->REG_GPRF0_BASE = (ITEint32)(gradientValue >> 24);
			hw->REG_GPRF1_BASE = (ITEuint32)gradientValue;
			// G
			gradientValue = (ITEint64)((double)(2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRG0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRG1_BASE = (ITEuint32)gradientValue;
			// H
			gradientValue = (ITEint64)((double)(2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y)) * 0x1000000 + 0.5f);
			hw->REG_GPRH0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRH1_BASE = (ITEuint32)gradientValue;
			// I
			gradientValue = (ITEint64)((double)(R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x)) * 0x1000000 + 0.5f);
			hw->REG_GPRI0_BASE = (ITEuint32)(gradientValue >> 24);
			hw->REG_GPRI1_BASE = (ITEuint32)gradientValue;
		}
		else
		{
			hw->REG_RCR00_BASE = ((paint->color.b * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0B) |
				                 (paint->color.a &ITE_VG_CMDMASK_RAMPCOLOR0A);
			hw->REG_RCR01_BASE = ((paint->color.r * 0x10) << ITE_VG_CMDSHIFT_RAMPCOLOR0R) |
				                 ((paint->color.g * 0x10) &ITE_VG_CMDMASK_RAMPCOLOR0G);
		    iteHardwareFire(hw);
		}

		/* Fill gradient parameter */
		{
			ITEfloat   lastOffset       = 0.0f;
			ITEint32   stopNumber        = 0;
			ITEint32   itemIndex        = 0;
			ITEuint32* currRampStopReg  = &hw->REG_RSR01_BASE;
			ITEuint32* currRampColorReg = &hw->REG_RCR00_BASE;
			ITEuint32* currDividerReg   = &hw->REG_RDR01_BASE;

			/* Disable all ramp stop registers */
			for ( itemIndex = 0; itemIndex < 8; itemIndex++ )
			{
				*currRampStopReg = 0;
				currRampStopReg++;
			}

			/* Restore */
			currRampStopReg  = &hw->REG_RSR01_BASE;
			currRampColorReg = &hw->REG_RCR00_BASE;
			currDividerReg   = &hw->REG_RDR01_BASE;

			/* Fill first 8 ramp stop value */
			for ( itemIndex = 0, stopNumber = 0; itemIndex < paint->stops.size; stopNumber++ )
			{
				ITEStop*  pStop     = &paint->stops.items[itemIndex];
				ITEColor  stopColor = pStop->color;
				ITEuint16 preR, preG, preB;

				/* Offset */
				if ( stopNumber & 0x01 )
				{
					*currRampStopReg |= (((ITEuint32)(pStop->offset * (1 << 12))) << ITE_VG_CMDSHIFT_RAMPSTOP1) | ITE_VG_CMD_RAMPSTOP1VLD;
					currRampStopReg++;
				}
				else
				{
					*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0VLD;

					/*  Enable/Disable gradient round */
					if (   (itemIndex < 8)
						&& (currRampStopReg == &hw->REG_RSR01_BASE) )
					{
						*currRampStopReg |= ITE_VG_CMD_RAMPSTOP0EQ;
					}
				}

				/* Color */
				if ( paint->premultiplied == VG_FALSE )
				{
					/* Transform color to pre-multiplied */
					/*
					stopColor.r = (ITEuint8)((float)stopColor.r * (float)stopColor.a/255.0f);
					stopColor.g = (ITEuint8)((float)stopColor.g * (float)stopColor.a/255.0f);
					stopColor.b = (ITEuint8)((float)stopColor.b * (float)stopColor.a/255.0f);
					*/

					preR = (ITEuint16)stopColor.r * stopColor.a;
					preG = (ITEuint16)stopColor.g * stopColor.a;
					preB = (ITEuint16)stopColor.b * stopColor.a;

					preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
					preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
					preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
				}
				else
				{
					preR = ((ITEuint16)stopColor.r) << 4;
					preG = ((ITEuint16)stopColor.g) << 4;
					preB = ((ITEuint16)stopColor.b) << 4;
				}
				*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
				currRampColorReg++;
				*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
				currRampColorReg++;

				/* Divider */
				if ( itemIndex > 0 )
				{
					*currDividerReg = ((ITEs15p16)(1 << 24)) / ((ITEs15p16)((pStop->offset - lastOffset) * (1 << 12)));
					currDividerReg++;
				}

				lastOffset = pStop->offset;

				/* Engine fire */
				if (   (itemIndex > 0) 
					&& (itemIndex % 7 == 0) )
				{
					iteHardwareFire(hw);

					/* Step back */
					currRampStopReg  = &hw->REG_RSR01_BASE;
					currRampColorReg = &hw->REG_RCR00_BASE;
					currDividerReg   = &hw->REG_RDR01_BASE;
				}
				else
				{
					itemIndex++;
				}
			}

			if (   (paint->stops.size > 8)
				|| (paint->stops.size % 8) )
			{
				/* Handle remaining stops, then fire engine */
				iteHardwareFire(hw);
			}
		}

		if ( hwWaitObjID == ITE_TRUE )
		{
			iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());
		}
	}

	// clear flag
	VG_Memset(&context->updateFlag, 0, sizeof(ITEUpdateFlag));
}

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

/*-------------------------------------------------------
 * Allocates a path resource in the current context and
 * sets its capabilities.
 *-------------------------------------------------------*/

VG_API_CALL VGPath vgCreatePath(VGint pathFormat,
                                VGPathDatatype datatype,
                                VGfloat scale, VGfloat bias,
                                VGint segmentCapacityHint,
                                VGint coordCapacityHint,
                                VGbitfield capabilities)
{
	ITEPath *p = NULL;
	VG_GETCONTEXT(VG_INVALID_HANDLE);

	/* VG_UNSUPPORTED_PATH_FORMAT_ERROR
    	¡V if pathFormat is not a supported format */
	VG_RETURN_ERR_IF(pathFormat != VG_PATH_FORMAT_STANDARD,
	                 VG_UNSUPPORTED_PATH_FORMAT_ERROR,
	                 VG_INVALID_HANDLE);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if datatype is not a valid value from the VGPathDatatype enumeration
		¡V if scale is equal to 0 */
	VG_RETURN_ERR_IF(!iteIsValidDatatype(datatype) || scale == 0.0f,
	               VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);

	/* Allocate new resource */
	ITE_NEWOBJ(ITEPath, p);
	VG_RETURN_ERR_IF(!p, VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	itePathArrayPushBack(&context->paths, p);

	/* Set parameters */
	p->format = pathFormat;
	p->scale = scale;
	p->bias = bias;
	p->segHint = segmentCapacityHint;
	p->dataHint = coordCapacityHint;
	p->datatype = datatype;
	p->caps = capabilities & VG_PATH_CAPABILITY_ALL;
	ITEPath_AddReference(p);

	VG_RETURN((VGPath)p);
}

/*-----------------------------------------------------
 * Clears the specified path of all data and sets new
 * capabilities to it.
 *-----------------------------------------------------*/

VG_API_CALL void vgClearPath(VGPath path, VGbitfield capabilities)
{
	ITEPath *p = NULL;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if path is not a valid path handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, path),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* Clear raw data */
	p = (ITEPath*)path;
	if ( p->segs )
	{
		free(p->segs);
	}
	if ( p->data )
	{
		free(p->data);
	}
	p->segs = NULL;
	p->data = NULL;
	p->segCount = 0;
	p->dataCount = 0;

	/* Downsize arrays to save memory */
	iteVertexArrayRealloc(&p->vertices, 1);
	iteVector2ArrayRealloc(&p->stroke, 1);

	/* Re-set capabilities */
	p->caps = capabilities & VG_PATH_CAPABILITY_ALL;

	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Disposes specified path resource in the current context
 *---------------------------------------------------------*/

VG_API_CALL void vgDestroyPath(VGPath path)
{
	int      index   = -1;
	ITEPath* itePath = (ITEPath*)path;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if path is not a valid path handle, or is not shared with the current context */
	index = itePathArrayFind(&context->paths, (ITEPath*)path);
	VG_RETURN_ERR_IF(index == -1, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* Delete object and remove resource */
	itePathArrayRemoveAt(&context->paths, index);

	if ( ITEPath_RemoveReference(itePath) == 0 )
	{
		ITE_DELETEOBJ(ITEPath, (ITEPath*)path);
	}

	VG_RETURN(VG_NO_RETVAL);
}

/*-----------------------------------------------------
 * Removes capabilities defined in the given bitfield
 * from the specified path.
 *-----------------------------------------------------*/

VG_API_CALL void vgRemovePathCapabilities(VGPath path, VGbitfield capabilities)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if path is not a valid path handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, path),
	               VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	capabilities &= VG_PATH_CAPABILITY_ALL;
	((ITEPath*)path)->caps &= ~capabilities;

	VG_RETURN(VG_NO_RETVAL);
}

/*-----------------------------------------------------
 * Returns capabilities of a path resource
 *-----------------------------------------------------*/

VG_API_CALL VGbitfield vgGetPathCapabilities(VGPath path)
{
	VG_GETCONTEXT(0x0);

	/* VG_BAD_HANDLE_ERROR
		¡V if path is not a valid path handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, path),
	                VG_BAD_HANDLE_ERROR, 0x0);

	VG_RETURN( ((ITEPath*)path)->caps );
}

/*-------------------------------------------------------------
 * Appends path data from source to destination path resource
 *-------------------------------------------------------------*/

VG_API_CALL void 
vgAppendPath(
	VGPath dstPath, 
	VGPath srcPath)
{
	int       i       = 0;
	ITEPath*  src     = NULL;
	ITEPath*  dst     = NULL;
	ITEuint8* newSegs = NULL;
	ITEuint8* newData = NULL;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if either dstPath or srcPath is not a valid path handle, 
		   or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, srcPath) ||
	                 !iteIsValidPath(context, dstPath),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	src = (ITEPath*)srcPath;
	dst = (ITEPath*)dstPath;
	/* VG_PATH_CAPABILITY_ERROR
		¡V if VG_PATH_CAPABILITY_APPEND_FROM is not enabled for srcPath
		¡V if VG_PATH_CAPABILITY_APPEND_TO is not enabled for dstPath */
	VG_RETURN_ERR_IF(!(src->caps & VG_PATH_CAPABILITY_APPEND_FROM) ||
	                 !(dst->caps & VG_PATH_CAPABILITY_APPEND_TO),
	                 VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);

	/* Resize path storage */
	iteResizePathData(dst, src->segCount, src->dataCount, &newSegs, &newData);
	VG_RETURN_ERR_IF(!newData, VG_OUT_OF_MEMORY_ERROR, VG_NO_RETVAL);

	/* Copy new segments */
	memcpy(newSegs+dst->segCount, src->segs, src->segCount);

	/* Copy new coordinates */
	for (i=0; i<src->dataCount; ++i)
	{
		ITEfloat coord = iteRealCoordFromData(src->datatype, src->scale, src->bias, src->data, i);
		
		iteRealCoordToData(dst->datatype, dst->scale, dst->bias, newData, dst->dataCount+i, coord);
	}

	/* Free old arrays */
	if ( dst->segs )
	{
		free(dst->segs);
	}
	if ( dst->data )
	{
		free(dst->data);
	}

	/* Adjust new properties */
	dst->segs = newSegs;
	dst->data = newData;
	dst->segCount += src->segCount;
	dst->dataCount += src->dataCount;

	/* Set dirty flag to true, so we can get the newest data in iteDrawPath() */
	itePathSetCommandDirty(dst, ITE_TRUE);

	VG_RETURN(VG_NO_RETVAL);
}

/*-----------------------------------------------------
 * Appends data to destination path resource
 *-----------------------------------------------------*/
VG_API_CALL void
vgAppendPathData(
	VGPath         dstPath, 
	VGint          newSegCount,
	const VGubyte* segs, 
	const void*    data)
{
	int       i            = 0;
	ITEPath*  dst          = NULL;
	ITEint    newDataCount = 0;
	ITEint    oldDataSize  = 0;
	ITEint    newDataSize  = 0;
	ITEuint8* newSegs      = NULL;
	ITEuint8* newData      = NULL;
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if dstPath is not a valid path handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, dstPath),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_PATH_CAPABILITY_ERROR
		¡V if VG_PATH_CAPABILITY_APPEND_TO is not enabled for dstPath */
	dst = (ITEPath*)dstPath;
	VG_RETURN_ERR_IF(!(dst->caps & VG_PATH_CAPABILITY_APPEND_TO),
    				 VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if pathSegments or pathData is NULL
		¡V if pathData is not properly aligned
		¡V if numSegments is less than or equal to 0
		¡V if pathSegments contains an illegal command */
	VG_RETURN_ERR_IF(!segs || !data || newSegCount <= 0,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!CheckAlignment(data, iteBytesPerDatatype[dst->datatype]),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Count number of coordinatess in appended data */
	newDataCount = iteCoordCountForData(newSegCount, segs);
	newDataSize = newDataCount * iteBytesPerDatatype[dst->datatype];
	oldDataSize = dst->dataCount * iteBytesPerDatatype[dst->datatype];
	VG_RETURN_ERR_IF(newDataCount == -1, VG_ILLEGAL_ARGUMENT_ERROR,
	                 VG_NO_RETVAL);

	/* Resize path storage */
	iteResizePathData(dst, newSegCount, newDataCount, &newSegs, &newData);
	VG_RETURN_ERR_IF(!newData, VG_OUT_OF_MEMORY_ERROR, VG_NO_RETVAL);

	/* Copy new segments */
	memcpy(newSegs+dst->segCount, segs, newSegCount);

	/* Copy new coordinates */
	if (dst->datatype == VG_PATH_DATATYPE_F)
	{
		for (i=0; i<newDataCount; ++i)
		{
			((ITEfloat32*)newData) [dst->dataCount+i] = iteValidInputFloat( ((VGfloat*)data) [i] );
		}
	}
	else
	{
		if ( newDataSize )
		{
			memcpy(newData+oldDataSize, data, newDataSize);
		}
	}

	/* Free old arrays */
    if ( dst->segs )
    {
	    free(dst->segs);
    }
    if ( dst->data )
    {
	    free(dst->data);
    }

	/* Adjust new properties */
	dst->segs = newSegs;
	dst->data = newData;
	dst->segCount += newSegCount;
	dst->dataCount += newDataCount;

	/* Set dirty flag to true, so we can get the newest data in iteDrawPath() */
	itePathSetCommandDirty(dst, ITE_TRUE);

	VG_RETURN(VG_NO_RETVAL);
}

/*--------------------------------------------------------
 * Modifies the coordinates of the existing path segments
 *--------------------------------------------------------*/

VG_API_CALL void 
vgModifyPathCoords(
	VGPath      dstPath, 
	VGint       startIndex,
	VGint       numSegments,  
	const void* data)
{
	int i;
	ITEPath *p;
	ITEint newDataCount;
	ITEint newDataSize;
	ITEint dataStartCount;
	ITEint dataStartSize;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if dstPath is not a valid path handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, dstPath),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_PATH_CAPABILITY_ERROR
		¡V if VG_PATH_CAPABILITY_MODIFY is not enabled for dstPath */
	p = (ITEPath*)dstPath;
	VG_RETURN_ERR_IF(!(p->caps & VG_PATH_CAPABILITY_MODIFY),
	                 VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if pathData is NULL
		¡V if pathData is not properly aligned
		¡V if startIndex is less than 0
		¡V if numSegments is less than or equal to 0
		¡V if startIndex + numSegments is greater than the number of segments in the path */
	VG_RETURN_ERR_IF(!data || startIndex < 0 || numSegments  <= 0 ||
	                 (startIndex + numSegments > p->segCount),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Check data array alignment */
	VG_RETURN_ERR_IF(!CheckAlignment(data, iteBytesPerDatatype[p->datatype]),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Find start of the coordinates to be changed */
	dataStartCount = iteCoordCountForData(startIndex, p->segs);
	dataStartSize = dataStartCount * iteBytesPerDatatype[p->datatype];

	/* Find byte size of coordinates to be changed */
	newDataCount = iteCoordCountForData(numSegments, &p->segs[startIndex]);
	newDataSize = newDataCount * iteBytesPerDatatype[p->datatype];

	/* Copy new coordinates */
	if (p->datatype == VG_PATH_DATATYPE_F)
	{
		for (i=0; i<newDataCount; ++i)
		{
			((ITEfloat32*)p->data) [dataStartCount+i] = iteValidInputFloat( ((VGfloat*)data) [i] );
		}
	}
	else
	{
		memcpy(((ITEuint8*)p->data) + dataStartSize, data, newDataSize);
	}

	/* Set dirty flag to true, so we can get the newest data in iteDrawPath() */
	itePathSetCommandDirty(p, ITE_TRUE);

	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void 
vgTransformPath(
	VGPath dstPath, 
	VGPath srcPath)
{
	ITEint    newSegCount  = 0;
	ITEint    newDataCount = 0;
	ITEPath*  src          = NULL; 
	ITEPath*  dst          = NULL;
	ITEuint8* newSegs      = NULL;
	ITEuint8* newData      = NULL;
	ITEint    segCount     = 0;
	ITEint    dataCount    = 0;
	void*     userData[5]  = { NULL };
	ITEint    processFlags = ITE_PROCESS_SIMPLIFY_LINES;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if either dstPath or srcPath is not a valid path handle, or is not 
		   shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, dstPath) ||
	                 !iteIsValidPath(context, srcPath),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_PATH_CAPABILITY_ERROR
		¡V if VG_PATH_CAPABILITY_TRANSFORM_FROM is not enabled for srcPath
		¡V if VG_PATH_CAPABILITY_TRANSFORM_TO is not enabled for dstPath */
	src = (ITEPath*)srcPath;
	dst = (ITEPath*)dstPath;
	VG_RETURN_ERR_IF(!(src->caps & VG_PATH_CAPABILITY_TRANSFORM_FROM) ||
	                 !(dst->caps & VG_PATH_CAPABILITY_TRANSFORM_TO),
	                 VG_PATH_CAPABILITY_ERROR, VG_NO_RETVAL);

	/* Resize path storage */
	iteProcessedDataCount(src, processFlags, &newSegCount, &newDataCount);
	iteResizePathData(dst, newSegCount, newDataCount, &newSegs, &newData);
	VG_RETURN_ERR_IF(!newData, VG_OUT_OF_MEMORY_ERROR, VG_NO_RETVAL);

	/* Transform src path into new data */
	segCount = dst->segCount;
	dataCount = dst->dataCount;
	userData[0] = newSegs;
	userData[1] = &segCount;
	userData[2] = newData;
	userData[3] = &dataCount;
	userData[4] = dst;
	iteProcessPathData(src, processFlags, iteTransformSegment, userData);

	/* Free old arrays */
	if ( dst->segs )
	{
		free(dst->segs);
	}
	if ( dst->data )
	{
		free(dst->data);
	}

	/* Adjust new properties */
	dst->segs = newSegs;
	dst->data = newData;
	dst->segCount = segCount;
	dst->dataCount = dataCount;

	/* Set dirty flag to true, so we can get the newest data in iteDrawPath() */
	itePathSetCommandDirty(dst, ITE_TRUE);

	VG_RETURN_ERR(VG_NO_ERROR, VG_NO_RETVAL);
}


VG_API_CALL VGboolean 
vgInterpolatePath(
	VGPath  dstPath, 
	VGPath  startPath,
	VGPath  endPath, 
	VGfloat amount)
{
	ITEPath *dst, *start, *end;
	ITEuint8 *procSegs1, *procSegs2;
	ITEfloat *procData1, *procData2;
	ITEint procSegCount1=0, procSegCount2=0;
	ITEint procDataCount1=0, procDataCount2=0;
	ITEuint8 *newSegs, *newData;
	void *userData[4];
	ITEint segment1, segment2;
	ITEint segindex, s,d,i;
	ITEint processFlags = ITE_PROCESS_SIMPLIFY_LINES | ITE_PROCESS_SIMPLIFY_CURVES;

	VG_GETCONTEXT(VG_FALSE);

	/* VG_BAD_HANDLE_ERROR
		¡V if any of dstPath, startPath, or endPath is not a valid path handle, 
		   or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, dstPath) ||
	                 !iteIsValidPath(context, startPath) ||
	                 !iteIsValidPath(context, endPath),
	                 VG_BAD_HANDLE_ERROR, VG_FALSE);

	/* VG_PATH_CAPABILITY_ERROR
		¡V if VG_PATH_CAPABILITY_PATH_INTERPOLATE_TO is not enabled for dstPath
		¡V if VG_PATH_CAPABILITY_PATH_INTERPOLATE_FROM is not enabled for startPath or endPath */
	dst   = (ITEPath*)dstPath; 
	start = (ITEPath*)startPath; 
	end   = (ITEPath*)endPath;
	VG_RETURN_ERR_IF(!(start->caps & VG_PATH_CAPABILITY_INTERPOLATE_FROM) ||
	                 !(end->caps & VG_PATH_CAPABILITY_INTERPOLATE_FROM) ||
	                 !(dst->caps & VG_PATH_CAPABILITY_INTERPOLATE_TO),
	                 VG_PATH_CAPABILITY_ERROR, VG_FALSE);

	/* Segment count must be equal */
	VG_RETURN_ERR_IF(start->segCount != end->segCount,
	                 VG_NO_ERROR, VG_FALSE);

	/* Allocate storage for processed path data */
	//iteProcessedDataCount(start, processFlags, &procSegCount1, &procDataCount1);
	//iteProcessedDataCount(end, processFlags, &procSegCount2, &procDataCount2);
	iteProcessedDataCountForInterpolation(start, processFlags, &procSegCount1, &procDataCount1);
	iteProcessedDataCountForInterpolation(end, processFlags, &procSegCount2, &procDataCount2);
	
	procSegs1 = (ITEuint8*)malloc(procSegCount1 * sizeof(ITEuint8));
	procSegs2 = (ITEuint8*)malloc(procSegCount2 * sizeof(ITEuint8));
	procData1 = (ITEfloat*)malloc(procDataCount1 * sizeof(ITEfloat));
	procData2 = (ITEfloat*)malloc(procDataCount2 * sizeof(ITEfloat));
	if (!procSegs1 || !procSegs2 || !procData1 || !procData2)
	{
		if ( procSegs1 ) free(procSegs1); 
		if ( procSegs2 ) free(procSegs2); 
		if ( procData1 ) free(procData1); 
		if ( procData2 ) free(procData2);
		VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_FALSE);
	}

	/* Process data of start path */
	procSegCount1  = 0; 
	procDataCount1 = 0;
	userData[0] = procSegs1; userData[1] = &procSegCount1;
	userData[2] = procData1; userData[3] = &procDataCount1;
	iteProcessPathData(start, processFlags, iteInterpolateSegment, userData);

	/* Process data of end path */
	procSegCount2  = 0; 
	procDataCount2 = 0;
	userData[0] = procSegs2; userData[1] = &procSegCount2;
	userData[2] = procData2; userData[3] = &procDataCount2;
	iteProcessPathData(end, processFlags, iteInterpolateSegment, userData);
	if (   (procSegCount1 != procSegCount2) 
		|| (procDataCount1 != procDataCount2) )
	{
		if ( procSegs1 ) free(procSegs1); 
		if ( procSegs2 ) free(procSegs2); 
		if ( procData1 ) free(procData1); 
		if ( procData2 ) free(procData2);
		VG_RETURN_ERR(VG_NO_ERROR, VG_FALSE);
	}

	/* Resize dst path storage to include interpolated data */
	iteResizePathData(dst, procSegCount1, procDataCount1, &newSegs, &newData);
	if (!newData)
	{
		free(procSegs1); 
		free(procData1);
		free(procSegs2); 
		free(procData2);
		VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_FALSE);
	}

	/* Interpolate data between paths */
	for (s=0, d=0; s<procSegCount1; ++s)
	{
		segment1 = (procSegs1[s] & 0x1E);
		segment2 = (procSegs2[s] & 0x1E);

		/* Pick the right arc type */
		if (   iteIsArcSegment(segment1) 
			&& iteIsArcSegment(segment2)) 
		{  
			if (amount < 0.5f)
			{
				segment2 = segment1;
			}
			else
			{
				segment1 = segment2;
			}
		}

		/* Segment types must match */
		if (segment1 != segment2)
		{
			free(procSegs1); 
			free(procData1);
			free(procSegs2); 
			free(procData2);
			free(newSegs); 
			free(newData);
			VG_RETURN_ERR(VG_NO_ERROR, VG_FALSE);
		}

		/* Interpolate values */
		segindex = (segment1 >> 1);
		newSegs[s + dst->segCount] = segment1 | VG_ABSOLUTE;
		for (i=0; i<iteCoordsPerCommand[segindex]; ++i, ++d)
		{
			ITEfloat diff  = procData2[d] - procData1[d];
			//ITEfloat value = procData1[d] + amount * diff;
			ITEfloat value = procData1[d] * (1.0f - amount) + amount * procData2[d];
			iteRealCoordToData(dst->datatype, dst->scale, dst->bias, newData, dst->dataCount + d, value);
		}
	}

	/* Free processed data */
	free(procSegs1); free(procData1);
	free(procSegs2); free(procData2);

	/* Assign interpolated data */
	dst->segs = newSegs;
	dst->data = newData;
	dst->segCount += procSegCount1;
	dst->dataCount += procDataCount1;

	/* Set dirty flag to true, so we can get the newest data in iteDrawPath() */
	itePathSetCommandDirty(dst, ITE_TRUE);

	VG_RETURN_ERR(VG_NO_ERROR, VG_TRUE);
}

/*-----------------------------------------------------------
 * Tessellates / strokes the path and draws it according to
 * VGContext state.
 *-----------------------------------------------------------*/
VG_API_CALL void vgDrawPath(VGPath path, VGbitfield paintModes)
{
	ITEPath* p;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if path is not a valid path handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, path),
                   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

  	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if paintModes is not a valid bitwise OR of values from the VGPaintMode enumeration */
	VG_RETURN_ERR_IF(!paintModes ||
		             (paintModes & (~(VG_STROKE_PATH | VG_FILL_PATH))),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	p = (ITEPath*)path;

	iteDrawPath(p, paintModes, &context->pathTransform);

	VG_RETURN(VG_NO_RETVAL);
}

