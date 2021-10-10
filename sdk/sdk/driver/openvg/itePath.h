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

#ifndef __ITEPATH_H
#define __ITEPATH_H

#include "iteVectors.h"
#include "iteArrays.h"

/* Helper structures for subdivision */
typedef struct {
	ITEVector2 p1;
	ITEVector2 p2;
	ITEVector2 p3;
}ITEQuad;

typedef struct {
	ITEVector2 p1;
	ITEVector2 p2;
	ITEVector2 p3;
	ITEVector2 p4;
}ITECubic;

typedef struct {
	ITEVector2 p1;
	ITEVector2 p2;
	ITEfloat   a1;
	ITEfloat   a2;
}ITEArc;

/* ITEVertex */
typedef struct
{
	ITEVector2 point;
	ITEVector2 tangent;
	ITEfloat   length;
	ITEuint    flags;
}ITEVertex;

struct _ITEImage;

/* Vertex flags for contour definition */
#define ITE_VERTEX_FLAG_CLOSE   (1 << 0)
#define ITE_VERTEX_FLAG_SEGEND  (1 << 1)
#define ITE_SEGMENT_TYPE_COUNT  13

/* Threshold for path data copy */
#if 1
#define ITE_PATH_COPY_SIZE_THRESHOLD (8 * 1024) ///< byte
#else // temporary solve the performance issue by Kuoping
#define ITE_PATH_COPY_SIZE_THRESHOLD 0
#endif

/* Vertex array */
#define _ITEM_T ITEVertex
#define _ARRAY_T ITEVertexArray
#define _FUNC_T iteVertexArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

/* Path command array */
#define _ITEM_T ITEuint32
#define _ARRAY_T ITEPathCommandArray
#define _FUNC_T itePathCommandArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"


/* ITEPath */
typedef struct ITEPath
{
	ITE_VG_OBJ_TYPE objType;
	
	/* Properties */
	VGint format;
	ITEfloat scale;
	ITEfloat bias;
	ITEint segHint;
	ITEint dataHint;
	VGbitfield caps;
	VGPathDatatype datatype;

	/* Raw data */
	ITEuint8 *segs;
	void *data;
	ITEint segCount;
	ITEint dataCount;

	/* Flatten data */
	ITEPathCommandArray pathCommand;	///< Store flattened data from "segs" & "data".
	ITEboolean          cmdDirty;		///< If this flag enable, regenerate path command from "segs" & "data".

	/* Subdivision */
	ITEVertexArray vertices;
	ITEVector2 min, max;
	ITEVector2 surfMin, surfMax;

	/* Additional stroke geometry (dash vertices if 
	   path dashed or triangle vertices if width > 1 */
	ITEVector2Array stroke;

	ITEint referenceCount;
} ITEPath;

void ITEPath_ctor(ITEPath *p);
void ITEPath_dtor(ITEPath *p);
ITEint ITEPath_AddReference(ITEPath* path);
ITEint ITEPath_RemoveReference(ITEPath* path);


/* Processing normalization flags */
#define ITE_PROCESS_SIMPLIFY_LINES    (1 << 0)
#define ITE_PROCESS_SIMPLIFY_CURVES   (1 << 1)
#define ITE_PROCESS_CENTRALIZE_ARCS   (1 << 2)
#define ITE_PROCESS_REPAIR_ENDS       (1 << 3)

/* Segment callback function type */
typedef void (*SegmentFunc) (ITEPath *p, VGPathSegment segment,
                             VGPathCommand originalCommand,
                             ITEfloat *data, void *userData);

/* Processes raw path data into normalized segments */
void iteProcessPathData(ITEPath *p, int flags,
                       SegmentFunc callback,
                       void *userData);

void iteDrawPath(ITEPath* p, ITEuint paintModes, ITEMatrix3x3* userToSurfaceMatrix);
void iteDrawPathToMask (ITEPath* p, ITEuint paintModes, VGMaskOperation operation);
void iteDrawPathToMask2(ITEPath* p, ITEuint paintModes, VGMaskOperation operation, struct _ITEImage* dstMaskImage);

/* Pointer-to-path array */
#define _ITEM_T ITEPath*
#define _ARRAY_T ITEPathArray
#define _FUNC_T itePathArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#endif /* __ITEPATH_H */
