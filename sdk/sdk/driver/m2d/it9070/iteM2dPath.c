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
#include <string.h>
#include "m2d/m2d_engine.h"
#include "m2d/iteM2dPath.h"
#include "m2d/iteM2dGeometry.h"
#include "m2d/iteM2dUtility.h"

/* Vertex array */
#define _ITEM_T  ITEM2DVertex
#define _ARRAY_T ITEM2DVertexArray
#define _FUNC_T  iteM2dVertexArray
#define _COMPARE_T(v1, v2) 0
#define _ARRAY_DEFINE
#include "m2d/iteM2dArrayBase.h"

#define _ITEM_T  ITEM2DPath *
#define _ARRAY_T ITEM2DPathArray
#define _FUNC_T  iteM2dPathArray
#define _ARRAY_DEFINE
#include "m2d/iteM2dArrayBase.h"

/* Path command array */
#define _ITEM_T  ITEM2Duint32
#define _ARRAY_T ITEM2DPathCommandArray
#define _FUNC_T  iteM2dPathCommandArray
#define _ARRAY_DEFINE
#include "m2d/iteM2dArrayBase.h"

static const ITEM2Dint iteM2dCoordsPerCommand[] = {
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

#define ITEM2D_PATH_MAX_COORDS           6
#define ITEM2D_PATH_MAX_COORDS_PROCESSED 12

static const ITEM2Dint iteM2dBytesPerDatatype[] = {
    1, /* VG_PATH_DATATYPE_S_8 */
    2, /* VG_PATH_DATATYPE_S_16 */
    4, /* VG_PATH_DATATYPE_S_32 */
    4  /* VG_PATH_DATATYPE_F */
};

#define ITEM2D_PATH_MAX_BYTES 4

#ifdef _WIN32
    #define _INLINE           _inline
#elif defined(__FREERTOS__)
    #define _INLINE           inline
#endif

/*-----------------------------------------------------
 * Path constructor
 *-----------------------------------------------------*/
ITEM2Dfloat iteM2dValidInputFloat(float f);

void ITEM2DPath_ctor(ITEM2DPath *p)
{
    p->objType   = ITEM2D_VG_OBJ_PATH;
    p->format    = 0;
    p->scale     = 0.0f;
    p->bias      = 0.0f;
    p->caps      = 0;
    p->datatype  = M2DVG_PATH_DATATYPE_F;

    p->segs      = NULL;
    p->data      = NULL;
    p->segCount  = 0;
    p->dataCount = 0;
    p->cmdDirty  = ITEM2D_TRUE;

    ITEM2D_INITOBJ(ITEM2DVertexArray, p->vertices);
    ITEM2D_INITOBJ(ITEM2DVector2Array, p->stroke);
    ITEM2D_INITOBJ(ITEM2DPathCommandArray, p->pathCommand);

    iteM2dPathCommandArrayReserve(&p->pathCommand, ITEM2D_PATH_CMD_INIT_SIZE);

    p->referenceCount = 0;
}

/*-----------------------------------------------------
 * Path destructor
 *-----------------------------------------------------*/
void ITEM2DPath_dtor(ITEM2DPath *p)
{
    if (p->segs)
        free(p->segs);
    if (p->data)
        free(p->data);

    ITEM2D_DEINITOBJ(ITEM2DVertexArray, p->vertices);
    ITEM2D_DEINITOBJ(ITEM2DVector2Array, p->stroke);
    ITEM2D_DEINITOBJ(ITEM2DPathCommandArray, p->pathCommand);
}

/*-----------------------------------------------------
 * Operator for refence count of path
 *-----------------------------------------------------*/
ITEM2Dint
ITEM2DPath_AddReference(
    ITEM2DPath *path)
{
    return ++path->referenceCount;
}

ITEM2Dint
ITEM2DPath_RemoveReference(
    ITEM2DPath *path)
{
    path->referenceCount--;
    ITEM2D_ASSERT(path->referenceCount >= 0);
    return path->referenceCount;
}

/*-----------------------------------------------------
 * Returns true (1) if given path data type is valid
 *-----------------------------------------------------*/
static ITEM2Dint iteM2dIsValidDatatype(M2DVGPathDatatype t)
{
    return (t == M2DVG_PATH_DATATYPE_S_8 ||
            t == M2DVG_PATH_DATATYPE_S_16 ||
            t == M2DVG_PATH_DATATYPE_S_32 ||
            t == M2DVG_PATH_DATATYPE_F);
}

static ITEM2Dint iteM2dIsValidCommand(ITEM2Dint c)
{
    return (c >= (M2DVG_CLOSE_PATH >> 1) &&
            c <= (M2DVG_LCWARC_TO >> 1));
}

static ITEM2Dint iteM2dIsArcSegment(ITEM2Dint s)
{
    return (s == M2DVG_SCWARC_TO || s == M2DVG_SCCWARC_TO ||
            s == M2DVG_LCWARC_TO || s == M2DVG_LCCWARC_TO);
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
static inline void
iteM2dPathSetCommandDirty(
    ITEM2DPath    *path,
    ITEM2Dboolean dirty)
{
    path->cmdDirty = dirty;
}

/*-----------------------------------------------------
 * Walks the given path segments and returns the total
 * number of coordinates.
 *-----------------------------------------------------*/
static ITEM2Dint iteM2dCoordCountForData(int segcount, const ITEM2Duint8 *segs)
{
    int s;
    int command;
    int count = 0;

    for (s = 0; s < segcount; ++s)
    {
        command = ((segs[s] & 0x1E) >> 1);
        if (!iteM2dIsValidCommand(command))
        {
            return -1;
        }
        count += iteM2dCoordsPerCommand[command];
    }

    return count;
}

/*-------------------------------------------------------
 * Interpretes the path data array according to the
 * path data type and returns the value at given index
 * in final interpretation (including scale and bias)
 *-------------------------------------------------------*/
static ITEM2Dfloat
iteM2dRealCoordFromData(
    M2DVGPathDatatype type,
    ITEM2Dfloat       scale,
    ITEM2Dfloat       bias,
    void              *data,
    ITEM2Dint         index)
{
    switch (type)
    {
    case M2DVG_PATH_DATATYPE_S_8:  return ( (ITEM2Dfloat) ((   ITEM2Dint8 *)data)[index] ) * scale + bias;
    case M2DVG_PATH_DATATYPE_S_16: return ( (ITEM2Dfloat) ((  ITEM2Dint16 *)data)[index] ) * scale + bias;
    case M2DVG_PATH_DATATYPE_S_32: return ( (ITEM2Dfloat) ((  ITEM2Dint32 *)data)[index] ) * scale + bias;
    default:                       return ( (ITEM2Dfloat) ((ITEM2Dfloat32 *)data)[index] ) * scale + bias;
    }
}

/*-------------------------------------------------------
 * Interpretes the path data array according to the
 * path data type and returns the value at given index
 * in final interpretation (but ignore scale and bias)
 *-------------------------------------------------------*/

static ITEM2Dfloat
iteM2dCoordFromData(
    M2DVGPathDatatype type,
    ITEM2Dfloat       scale,
    ITEM2Dfloat       bias,
    void              *data,
    ITEM2Dint         index)
{
    switch (type)
    {
    case M2DVG_PATH_DATATYPE_S_8:  return ( (ITEM2Dfloat) ((   ITEM2Dint8 *)data)[index] );
    case M2DVG_PATH_DATATYPE_S_16: return ( (ITEM2Dfloat) ((  ITEM2Dint16 *)data)[index] );
    case M2DVG_PATH_DATATYPE_S_32: return ( (ITEM2Dfloat) ((  ITEM2Dint32 *)data)[index] );
    default:                       return ( (ITEM2Dfloat) ((ITEM2Dfloat32 *)data)[index] );
    }
}

/*-------------------------------------------------------
 * Interpretes the path data array according to the
 * path data type and sets the value at given index
 * from final interpretation (including scale and bias)
 *-------------------------------------------------------*/

static void iteM2dRealCoordToData(M2DVGPathDatatype type, ITEM2Dfloat scale, ITEM2Dfloat bias,
                                  void *data,  ITEM2Dint index, ITEM2Dfloat c)
{
    c -= bias;
    c /= scale;

    switch (type)
    {
    case M2DVG_PATH_DATATYPE_S_8:  ((   ITEM2Dint8 *)data) [index] = ( ITEM2Dint8)ITEM2D_FLOOR(c + 0.5f); break;
    case M2DVG_PATH_DATATYPE_S_16: ((  ITEM2Dint16 *)data) [index] = (ITEM2Dint16)ITEM2D_FLOOR(c + 0.5f); break;
    case M2DVG_PATH_DATATYPE_S_32: ((  ITEM2Dint32 *)data) [index] = (ITEM2Dint32)ITEM2D_FLOOR(c + 0.5f); break;
    default:                       ((ITEM2Dfloat32 *)data) [index] = (ITEM2Dfloat32)c; break;
    }
}

/*-------------------------------------------------
 * Resizes storage for segment and coordinate data
 * of the specified path by given number of items
 *-------------------------------------------------*/

static int
iteM2dResizePathData(
    ITEM2DPath  *p,
    ITEM2Dint   newSegCount,
    ITEM2Dint   newDataCount,
    ITEM2Duint8 **newSegs,
    ITEM2Duint8 **newData)
{
    ITEM2Dint oldDataSize = 0;
    ITEM2Dint newDataSize = 0;

    /* Allocate memory for new segments */
    (*newSegs) = (ITEM2Duint8 *)malloc(p->segCount + newSegCount);
    if ((*newSegs) == NULL)
    {
        return 0;
    }

    /* Allocate memory for new data */
    oldDataSize = p->dataCount * iteM2dBytesPerDatatype[p->datatype];
    newDataSize = newDataCount * iteM2dBytesPerDatatype[p->datatype];
    (*newData)  = (ITEM2Duint8 *)malloc(oldDataSize + newDataSize);
    if ((*newData) == NULL)
    {
        free(*newSegs);
        return 0;
    }

    /* Copy old segments */
    if (p->segs)
    {
        memcpy(*newSegs, p->segs, p->segCount);
    }

    /* Copy old data */
    if (p->data)
    {
        memcpy(*newData, p->data, oldDataSize);
    }

    return 1;
}

/*------------------------------------------------------------
 * Converts standart endpoint arc parametrization into center
 * arc parametrization for further internal processing
 *------------------------------------------------------------*/

static ITEM2Dint iteM2dCentralizeArc(ITEM2Duint command, ITEM2Dfloat *data)
{
    ITEM2Dfloat     rx, ry, r, a;
    ITEM2DVector2   p1, p2, pp1, pp2;
    ITEM2DVector2   d, dt;
    ITEM2Dfloat     dist, halfdist, q;
    ITEM2DVector2   c1, c2, *c;
    ITEM2DVector2   pc1, pc2;
    ITEM2Dfloat     a1, ad, a2;
    ITEM2DVector2   ux, uy;
    ITEM2DMatrix3x3 user2arc;
    ITEM2DMatrix3x3 arc2user;

    rx = data[2];
    ry = data[3];
    a  = ITEM2D_DEG2RAD(data[4]);
    SET2(p1, data[0], data[1]);
    SET2(p2, data[5], data[6]);

    /* Check for invalid radius and return line */
    if (ITEM2D_NEARZERO(rx) || ITEM2D_NEARZERO(ry))
    {
        data[2] = p2.x; data[3] = p2.y;
        return 0;
    }

    /* Rotate to arc coordinates.
       Scale to correct eccentricity. */
    IDMAT(user2arc);
    ROTATEMATL(user2arc, -a);
    SCALEMATL(user2arc, 1, rx / ry);
    TRANSFORM2TO(p1, user2arc, pp1);
    TRANSFORM2TO(p2, user2arc, pp2);

    /* Distance between points and
       perpendicular vector */
    SET2V(d, pp2); SUB2V(d, pp1);
    dist     = NORM2(d);
    halfdist = dist * 0.5f;
    SET2(dt, -d.y, d.x);
    DIV2(dt, dist);

    /* Check if too close and return line */
    if (halfdist == 0.0f)
    {
        data[2] = p2.x; data[3] = p2.y;
        return 0;
    }

    /* Scale radius if too far away */
    r = rx;
    if (halfdist > rx)
        r = halfdist;

    /* Intersections of circles centered to start and
       end point. (i.e. centers of two possible arcs) */
    q    = ITEM2D_SIN(ITEM2D_ACOS(halfdist / r)) * r;
    if (ITEM2D_ISNAN(q))
        q = 0.0f;
    c1.x = pp1.x + d.x / 2 + dt.x * q;
    c1.y = pp1.y + d.y / 2 + dt.y * q;
    c2.x = pp1.x + d.x / 2 - dt.x * q;
    c2.y = pp1.y + d.y / 2 - dt.y * q;

    /* Pick the right arc center */
    switch (command & 0x1E)
    {
    case M2DVG_SCWARC_TO: 
    case M2DVG_LCCWARC_TO:
        c = &c2; 
        break;
    case M2DVG_LCWARC_TO: 
    case M2DVG_SCCWARC_TO:
        c = &c1; 
        break;
    default:
        c = &c1;
    }

    /* Find angles of p1,p2 towards the chosen center */
    SET2V(pc1, pp1); SUB2V(pc1, (*c));
    SET2V(pc2, pp2); SUB2V(pc2, (*c));
    a1 = iteM2dVectorOrientation(&pc1);

    /* Correct precision error when ry very small
       (q gets very large and points very far appart) */
    if (ITEM2D_ISNAN(a1))
        a1 = 0.0f;

    /* Small or large one? */
    switch (command & 0x1E)
    {
    case M2DVG_SCWARC_TO: 
    case M2DVG_SCCWARC_TO:
        ad = ANGLE2(pc1, pc2); 
        break;
    case M2DVG_LCWARC_TO: 
    case M2DVG_LCCWARC_TO:
        ad = 2 * PI - ANGLE2(pc1, pc2); 
        break;
    default:
        ad = 0.0f;
    }

    /* Correct precision error when solution is single
       (pc1 and pc2 are nearly collinear but not really) */
    if (ITEM2D_ISNAN(ad))
        ad = PI;

    /* CW or CCW? */
    switch (command & 0x1E)
    {
    case M2DVG_SCWARC_TO: 
    case M2DVG_LCWARC_TO:
        a2 = a1 - ad; 
        break;
    case M2DVG_SCCWARC_TO: 
    case M2DVG_LCCWARC_TO:
        a2 = a1 + ad; 
        break;
    default:
        a2 = a1;
    }

    /* Arc unit vectors */
    SET2(ux, r, 0);
    SET2(uy, 0, r);

    /* Transform back to user coordinates */
    IDMAT(arc2user);
    SCALEMATL(arc2user, 1, ry / rx);
    ROTATEMATL(arc2user, a);
    TRANSFORM2((*c), arc2user);
    TRANSFORM2(ux, arc2user);
    TRANSFORM2(uy, arc2user);

    /* Write out arc coords */
    data[2]  = c->x;  data[3] = c->y;
    data[4]  = ux.x;  data[5] = ux.y;
    data[6]  = uy.x;  data[7] = uy.y;
    data[8]  = a1;    data[9] = a2;
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

#define ITEM2D_PROCESS_SIMPLIFY_LINES  (1 << 0)
#define ITEM2D_PROCESS_SIMPLIFY_CURVES (1 << 1)
#define ITEM2D_PROCESS_REPAIR_ENDS     (1 << 3)

void
iteM2dProcessPathData(
    ITEM2DPath  *p,
    int         flags,
    SegmentFunc callback,
    void        *userData)
{
    ITEM2Dint        i = 0, s = 0, d = 0, c = 0;
    ITEM2Duint       command;
    //ITEuint segment;
    M2DVGPathSegment segment;
    ITEM2Dint        segindex;
    M2DVGPathAbsRel  absrel;
    ITEM2Dint        numcoords;
    ITEM2Dfloat      data[ITEM2D_PATH_MAX_COORDS_PROCESSED];
    ITEM2DVector2    start;    /* start of the current contour */
    ITEM2DVector2    pen;      /* current pen position */
    ITEM2DVector2    tan;      /* backward tangent for smoothing */
    ITEM2Dint        open = 0; /* contour-open flag */

    /* Reset points */
    SET2(start, 0, 0);
    SET2(pen, 0, 0);
    SET2(tan, 0, 0);

    for (s = 0; s < p->segCount; ++s, d += numcoords)
    {
        /* Extract command */
        command   = (p->segs[s]);
        absrel    = (command & 1);
        segment   = (command & 0x1E);
        segindex  = (segment >> 1);
        numcoords = iteM2dCoordsPerCommand[segindex];

        /* Repair segment start / end */
        if (flags & ITEM2D_PROCESS_REPAIR_ENDS)
        {
            /* Prevent double CLOSE_PATH */
            if (!open && segment == M2DVG_CLOSE_PATH)
            {
                continue;
            }

            /* Implicit MOVE_TO if segment starts without */
            if (!open && segment != M2DVG_MOVE_TO)
            {
                data[0] = pen.x; data[1] = pen.y;
                data[2] = pen.x; data[3] = pen.y;
                (*callback)(p, M2DVG_MOVE_TO, command, data, userData);
                open    = 1;
            }

            /* Avoid a MOVE_TO at the end of data */
            if (segment == M2DVG_MOVE_TO)
            {
                if (s == p->segCount - 1)
                {
                    break;
                }
                else
                {
                    /* Avoid a lone MOVE_TO  */
                    M2DVGPathSegment nextsegment = (p->segs[s + 1] & 0x1E);
                    if (nextsegment == M2DVG_MOVE_TO)
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
        c       = 2;

        /* Unpack coordinates from path data */
        for (i = 0; i < numcoords; ++i)
        {
            data[c++] = iteM2dRealCoordFromData(p->datatype, p->scale, p->bias, p->data, d + i);
        }

        /* Simplify complex segments */
        switch (segment)
        {
        case M2DVG_CLOSE_PATH:
            data[2] = start.x;
            data[3] = start.y;

            SET2V(pen, start);
            SET2V(tan, start);
            open = 0;

            (*callback)(p, M2DVG_CLOSE_PATH, command, data, userData);
            break;

        case M2DVG_MOVE_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[2] += pen.x;
                data[3] += pen.y;
            }

            SET2(pen, data[2], data[3]);
            SET2V(start, pen);
            SET2V(tan, pen);
            open = 1;

            (*callback)(p, M2DVG_MOVE_TO, command, data, userData);
            break;

        case M2DVG_LINE_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[2] += pen.x;
                data[3] += pen.y;
            }

            SET2(pen, data[2], data[3]);
            SET2V(tan, pen);

            (*callback)(p, M2DVG_LINE_TO, command, data, userData);
            break;

        case M2DVG_HLINE_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[2] += pen.x;
            }

            SET2(pen, data[2], pen.y);
            SET2V(tan, pen);

            if (flags & ITEM2D_PROCESS_SIMPLIFY_LINES)
            {
                data[3] = pen.y;
                (*callback)(p, M2DVG_LINE_TO, command, data, userData);
                break;
            }

            (*callback)(p, M2DVG_HLINE_TO, command, data, userData);
            break;

        case M2DVG_VLINE_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[2] += pen.y;
            }

            SET2(pen, pen.x, data[2]);
            SET2V(tan, pen);

            if (flags & ITEM2D_PROCESS_SIMPLIFY_LINES)
            {
                data[2] = pen.x;
                data[3] = pen.y;
                (*callback)(p, M2DVG_LINE_TO, command, data, userData);
                break;
            }

            (*callback)(p, M2DVG_VLINE_TO, command, data, userData);
            break;

        case M2DVG_QUAD_TO:
            if (absrel == M2DVG_RELATIVE)
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

            (*callback)(p, M2DVG_QUAD_TO, command, data, userData);
            break;

        case M2DVG_CUBIC_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[2] += pen.x; data[3] += pen.y;
                data[4] += pen.x; data[5] += pen.y;
                data[6] += pen.x; data[7] += pen.y;
            }

            SET2(tan, data[4], data[5]);
            SET2(pen, data[6], data[7]);

            (*callback)(p, M2DVG_CUBIC_TO, command, data, userData);
            break;

        case M2DVG_SQUAD_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[2] += pen.x; data[3] += pen.y;
            }

            SET2(tan, 2 * pen.x - tan.x, 2 * pen.y - tan.y);
            SET2(pen, data[2], data[3]);

            if (flags & ITEM2D_PROCESS_SIMPLIFY_CURVES)
            {
                data[2] = tan.x; data[3] = tan.y;
                data[4] = pen.x; data[5] = pen.y;
                (*callback)(p, M2DVG_QUAD_TO, command, data, userData);
                break;
            }

            (*callback)(p, M2DVG_SQUAD_TO, command, data, userData);
            break;

        case M2DVG_SCUBIC_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[2] += pen.x; data[3] += pen.y;
                data[4] += pen.x; data[5] += pen.y;
            }

            if (flags & ITEM2D_PROCESS_SIMPLIFY_CURVES)
            {
                data[7] = data[5];
                data[6] = data[4];
                data[5] = data[3];
                data[4] = data[2];

                data[2] = 2 * pen.x - tan.x;
                data[3] = 2 * pen.y - tan.y;

                SET2(tan, data[4], data[5]);
                SET2(pen, data[6], data[7]);
                (*callback)(p, M2DVG_CUBIC_TO, command, data, userData);
                break;
            }

            SET2(tan, data[2], data[3]);
            SET2(pen, data[4], data[5]);

            (*callback)(p, M2DVG_SCUBIC_TO, command, data, userData);
            break;

        case M2DVG_SCWARC_TO:
        case M2DVG_SCCWARC_TO:
        case M2DVG_LCWARC_TO:
        case M2DVG_LCCWARC_TO:
            if (absrel == M2DVG_RELATIVE)
            {
                data[5] += pen.x;
                data[6] += pen.y;
            }

            data[2] = ITEM2D_ABS(data[2]);
            data[3] = ITEM2D_ABS(data[3]);

            SET2(tan, data[5], data[6]);
            SET2V(pen, tan);

            (*callback)(p, segment, command, data, userData);
            break;
        } /* switch (command) */
    }     /* for each segment */
}

/*-------------------------------------------------------
 * Walks raw path data and counts the resulting number
 * of segments and coordinates if the simplifications
 * specified in the given processing flags were applied.
 *-------------------------------------------------------*/
static void
iteM2dProcessedDataCount(
    ITEM2DPath *p,
    ITEM2Dint  flags,
    ITEM2Dint  *segCount,
    ITEM2Dint  *dataCount)
{
    ITEM2Dint s, segment, segindex;

    *segCount  = 0;
    *dataCount = 0;

    for (s = 0; s < p->segCount; ++s)
    {
        segment  = (p->segs[s] & 0x1E);
        segindex = (segment >> 1);

        switch (segment)
        {
        case M2DVG_HLINE_TO:
        case M2DVG_VLINE_TO:
            if (flags & ITEM2D_PROCESS_SIMPLIFY_LINES)
            {
                *dataCount += iteM2dCoordsPerCommand[M2DVG_LINE_TO >> 1];
            }
            else
            {
                *dataCount += iteM2dCoordsPerCommand[segindex];
            }
            break;

        case M2DVG_SQUAD_TO:
            if (flags & ITEM2D_PROCESS_SIMPLIFY_CURVES)
            {
                *dataCount += iteM2dCoordsPerCommand[M2DVG_QUAD_TO >> 1];
            }
            else
            {
                *dataCount += iteM2dCoordsPerCommand[segindex];
            }
            break;

        case M2DVG_SCUBIC_TO:
            if (flags & ITEM2D_PROCESS_SIMPLIFY_CURVES)
            {
                *dataCount += iteM2dCoordsPerCommand[M2DVG_CUBIC_TO >> 1];
            }
            else
            {
                *dataCount += iteM2dCoordsPerCommand[segindex];
            }
            break;

        case M2DVG_SCWARC_TO:
        case M2DVG_SCCWARC_TO:
        case M2DVG_LCWARC_TO:
        case M2DVG_LCCWARC_TO:
            if (flags & ITEM2D_PROCESS_CENTRALIZE_ARCS)
            {
                *dataCount += 10;
            }
            else
            {
                *dataCount += iteM2dCoordsPerCommand[segindex];
            }
            break;

        default:
            *dataCount += iteM2dCoordsPerCommand[segindex];
        }

        *segCount += 1;
    }
}

//start modify

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

/*-------------------------------------------------------
 * Allocates a path resource in the current context and
 * sets its capabilities.
 *-------------------------------------------------------*/

ITEM2DPath *m2dvgCreatePath(ITEM2Dint pathFormat,
                            M2DVGPathDatatype datatype,
                            ITEM2Dfloat scale, ITEM2Dfloat bias,
                            ITEM2Dint segmentCapacityHint,
                            ITEM2Dint coordCapacityHint,
                            unsigned int capabilities)
{
    ITEM2DPath *p = NULL;

    /* Allocate new resource */
    ITEM2D_NEWOBJ(ITEM2DPath, p);

    /* Set parameters */
    p->format   = pathFormat;
    p->scale    = scale;
    p->bias     = bias;
    p->segHint  = segmentCapacityHint;
    p->dataHint = coordCapacityHint;
    p->datatype = datatype;
    p->caps     = capabilities & M2DVG_PATH_CAPABILITY_ALL;
    ITEM2DPath_AddReference(p);

    return p;
}

/*---------------------------------------------------------
 * Disposes specified path resource in the current context
 *---------------------------------------------------------*/

void m2dvgDestroyPath(ITEM2DPath *path)
{
    if (ITEM2DPath_RemoveReference(path) == 0)
    {
        ITEM2D_DELETEOBJ(ITEM2DPath, (ITEM2DPath *)path);
    }
}

/*-----------------------------------------------------
 * Removes capabilities defined in the given bitfield
 * from the specified path.
 *-----------------------------------------------------*/

ITEM2Dfloat getMaxFloat()
{
    ITEM2Dfloatint fi;
    fi.i = ITEM2D_MAX_FLOAT_BITS;
    return fi.f;
}

ITEM2Dfloat iteM2dValidInputFloat(ITEM2Dfloat f)
{
    ITEM2Dfloat max = getMaxFloat();
    if (ITEM2D_ISNAN(f))
    {
        return 0.0f;            /* convert NAN to zero */
    }
    ITEM2D_CLAMP(f, -max, max); /* clamp to valid range */
    return (ITEM2Dfloat)f;
}

/*-----------------------------------------------------
 * Appends data to destination path resource
 *-----------------------------------------------------*/
void
m2dvgAppendPathData(
    ITEM2DPath          *dstPath,
    int                 newSegCount,
    const unsigned char *segs,
    const void          *data)
{
    int         i            = 0;
    ITEM2DPath  *dst         = NULL;
    ITEM2Dint   newDataCount = 0;
    ITEM2Dint   oldDataSize  = 0;
    ITEM2Dint   newDataSize  = 0;
    ITEM2Duint8 *newSegs     = NULL;
    ITEM2Duint8 *newData     = NULL;

    /* VG_PATH_CAPABILITY_ERROR
        �V if VG_PATH_CAPABILITY_APPEND_TO is not enabled for dstPath */
    dst          = (ITEM2DPath *)dstPath;

    /* Count number of coordinatess in appended data */
    newDataCount = iteM2dCoordCountForData(newSegCount, segs);
    newDataSize  = newDataCount * iteM2dBytesPerDatatype[dst->datatype];
    oldDataSize  = dst->dataCount * iteM2dBytesPerDatatype[dst->datatype];

    /* Resize path storage */
    iteM2dResizePathData(dst, newSegCount, newDataCount, &newSegs, &newData);

    /* Copy new segments */
    memcpy(newSegs + dst->segCount, segs, newSegCount);

    /* Copy new coordinates */
    if (dst->datatype == M2DVG_PATH_DATATYPE_F)
    {
        for (i = 0; i < newDataCount; ++i)
        {
            ((ITEM2Dfloat32 *)newData) [dst->dataCount + i] = iteM2dValidInputFloat( ((ITEM2Dfloat *)data) [i]);
        }
    }
    else
    {
        if (newDataSize)
        {
            memcpy(newData + oldDataSize, data, newDataSize);
        }
    }

    /* Free old arrays */
    if (dst->segs)
    {
        free(dst->segs);
    }
    if (dst->data)
    {
        free(dst->data);
    }

    /* Adjust new properties */
    dst->segs       = newSegs;
    dst->data       = newData;
    dst->segCount  += newSegCount;
    dst->dataCount += newDataCount;

    /* Set dirty flag to true, so we can get the newest data in iteDrawPath() */
    iteM2dPathSetCommandDirty(dst, ITEM2D_TRUE);
}