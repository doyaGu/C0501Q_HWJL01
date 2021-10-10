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

#ifndef __ITEM2DPATH_H
#define __ITEM2DPATH_H

#include "m2d/iteM2dVectors.h"
#include "m2d/iteM2dArrays.h"
#include "m2d/m2d_engine.h"

/* Helper structures for subdivision */
typedef struct {
  ITEM2DVector2 p1;
  ITEM2DVector2 p2;
  ITEM2DVector2 p3;
} ITEM2DQuad;

typedef struct {
  ITEM2DVector2 p1;
  ITEM2DVector2 p2;
  ITEM2DVector2 p3;
  ITEM2DVector2 p4;
} ITEM2DCubic;

typedef struct {
  ITEM2DVector2 p1;
  ITEM2DVector2 p2;
  ITEM2Dfloat a1;
  ITEM2Dfloat a2;
} ITEM2DArc;

/* ITEVertex */
typedef struct
{
  ITEM2DVector2 point;
  ITEM2DVector2 tangent;
  ITEM2Dfloat length;
  ITEM2Duint flags;
  
} ITEM2DVertex;

/* Vertex flags for contour definition */
#define ITEM2D_VERTEX_FLAG_CLOSE   (1 << 0)
#define ITEM2D_VERTEX_FLAG_SEGEND  (1 << 1)
#define ITEM2D_SEGMENT_TYPE_COUNT  13

/* Threshold for path data copy */
//#define ITEM2D_PATH_COPY_SIZE_THRESHOLD (8 * 1024) ///< byte
#define ITEM2D_PATH_COPY_SIZE_THRESHOLD 0 ///< byte

/* Vertex array */
#define _ITEM_T ITEM2DVertex
#define _ARRAY_T ITEM2DVertexArray
#define _FUNC_T iteM2dVertexArray
#define _ARRAY_DECLARE
#include "iteM2dArrayBase.h"

/* Path command array */
#define _ITEM_T ITEM2Duint32
#define _ARRAY_T ITEM2DPathCommandArray
#define _FUNC_T iteM2dPathCommandArray
#define _ARRAY_DECLARE
#include "iteM2dArrayBase.h"


/* ITEPath */
typedef struct ITEM2DPath
{
	ITEM2D_VG_OBJ_TYPE objType;
	
	/* Properties */
	ITEM2Dint format;
	ITEM2Dfloat scale;
	ITEM2Dfloat bias;
	ITEM2Dint segHint;
	ITEM2Dint dataHint;
	unsigned int caps;
	M2DVGPathDatatype datatype;

	/* Raw data */
	ITEM2Duint8 *segs;
	void *data;
	ITEM2Dint segCount;
	ITEM2Dint dataCount;

	/* Flatten data */
	ITEM2DPathCommandArray pathCommand;	///< Store flattened data from "segs" & "data".
	ITEM2Dboolean          cmdDirty;		///< If this flag enable, regenerate path command from "segs" & "data".

	/* Subdivision */
	ITEM2DVertexArray vertices;
	ITEM2DVector2 min, max;
	ITEM2DVector2 surfMin, surfMax;

	/* Additional stroke geometry (dash vertices if 
	   path dashed or triangle vertices if width > 1 */
	ITEM2DVector2Array stroke;

	ITEM2Dint referenceCount;
} ITEM2DPath;

void ITEM2DPath_ctor(ITEM2DPath *p);
void ITEM2DPath_dtor(ITEM2DPath *p);
ITEM2Dint ITEM2DPath_AddReference(ITEM2DPath* path);
ITEM2Dint ITEM2DPath_RemoveReference(ITEM2DPath* path);


/* Processing normalization flags */
#define ITEM2D_PROCESS_SIMPLIFY_LINES    (1 << 0)
#define ITEM2D_PROCESS_SIMPLIFY_CURVES   (1 << 1)
#define ITEM2D_PROCESS_CENTRALIZE_ARCS   (1 << 2)
#define ITEM2D_PROCESS_REPAIR_ENDS       (1 << 3)

/* Segment callback function type */
typedef void (*SegmentFunc) (ITEM2DPath *p, M2DVGPathSegment segment,
                             M2DVGPathCommand originalCommand,
                             ITEM2Dfloat *data, void *userData);

/* Processes raw path data into normalized segments */
void iteM2dProcessPathData(ITEM2DPath *p, int flags,
                       SegmentFunc callback,
                       void *userData);
                       
void
m2dvgAppendPathData(
	ITEM2DPath*             dstPath, 
	int                  newSegCount,
	const unsigned char* segs, 
	const void*          data);
	
ITEM2DPath* 
m2dvgCreatePath(ITEM2Dint pathFormat,
            M2DVGPathDatatype datatype,
            ITEM2Dfloat scale, ITEM2Dfloat bias,
            ITEM2Dint segmentCapacityHint,
            ITEM2Dint coordCapacityHint,
            unsigned int capabilities);
            
void 
m2dvgDestroyPath(ITEM2DPath* path);            
                                	                       

/* Pointer-to-path array */
#define _ITEM_T ITEM2DPath*
#define _ARRAY_T ITEM2DPathArray
#define _FUNC_T iteM2dPathArray
#define _ARRAY_DECLARE
#include "iteM2dArrayBase.h"

#endif /* __ITEM2DPATH_H */
