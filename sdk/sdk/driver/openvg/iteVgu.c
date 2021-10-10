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
#include "vgu.h"
#include "iteDefs.h"
#include "iteContext.h"
#include <math.h>
#include "iteUtility.h"
#include "iteVectors.h"

static VGUErrorCode append(VGPath path, ITEint commSize, const VGubyte *comm,
                             ITEint dataSize, const VGfloat *data)
{
  VGErrorCode err = VG_NO_ERROR;
  VGPathDatatype type = vgGetParameterf(path, VG_PATH_DATATYPE);
  VGfloat scale = vgGetParameterf(path, VG_PATH_SCALE);
  VGfloat bias = vgGetParameterf(path, VG_PATH_BIAS);
  ITE_ASSERT(dataSize <= 26);
  
  switch(type)
  {
  case VG_PATH_DATATYPE_S_8: {
      
      ITEint8 data8[26]; int i;
      for (i=0; i<dataSize; ++i)
        data8[i] = (ITEint8)ITE_FLOOR((data[i] - bias) / scale + 0.5f);
      vgAppendPathData(path, commSize, comm, data8);
      
      break;}
  case VG_PATH_DATATYPE_S_16: {
      
      ITEint16 data16[26]; int i;
      for (i=0; i<dataSize; ++i)
        data16[i] = (ITEint16)ITE_FLOOR((data[i] - bias) / scale + 0.5f);
      vgAppendPathData(path, commSize, comm, data16);
      
      break;}
  case VG_PATH_DATATYPE_S_32: {
      
      ITEint32 data32[26]; int i;
      for (i=0; i<dataSize; ++i)
        data32[i] = (ITEint32)ITE_FLOOR((data[i] - bias) / scale + 0.5f);
      vgAppendPathData(path, commSize, comm, data32);
      
      break;}
  default: {
      
      VGfloat dataF[26]; int i;
      for (i=0; i<dataSize; ++i)
        dataF[i] = (data[i] - bias) / scale;
      vgAppendPathData(path, commSize, comm, dataF);
      
      break;}
  }
  
  err = vgGetError();
  if (err == VG_PATH_CAPABILITY_ERROR)
    return VGU_PATH_CAPABILITY_ERROR;
  else if (err == VG_BAD_HANDLE_ERROR)
    return VGU_BAD_HANDLE_ERROR;
  else if (err == VG_OUT_OF_MEMORY_ERROR)
    return VGU_OUT_OF_MEMORY_ERROR;
  
  return VGU_NO_ERROR;
}

VGU_API_CALL VGUErrorCode 
vguLine(
	VGPath  path,
	VGfloat x0, 
	VGfloat y0,
	VGfloat x1, 
	VGfloat y1)
{
	VGUErrorCode err = VGU_NO_ERROR;
	const VGubyte comm[] = {VG_MOVE_TO_ABS, VG_LINE_TO_ABS};

	VGfloat data[4];
	data[0] = x0; data[1] = y0;
	data[2] = x1; data[3] = y1;

	err = append(path, 2, comm, 4, data);
	return err;
}

VGU_API_CALL VGUErrorCode 
vguPolygon(
	VGPath         path,
	const VGfloat* points, 
	VGint          count,
	VGboolean      closed)
{
	VGint i;
	VGubyte *comm = NULL;
	VGUErrorCode err = VGU_NO_ERROR;

	if (points == NULL || count <= 0)
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}
	/* TODO: check points array alignment */
	if ( CheckAlignment(points, sizeof(VGfloat)) == VG_FALSE )
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}

	comm = (VGubyte*)malloc( (count+1) * sizeof(VGubyte) );
	if (comm == NULL) 
	{
		return VGU_OUT_OF_MEMORY_ERROR;
	}

	comm[0] = VG_MOVE_TO_ABS;
	for (i=1; i<count; ++i)
	{
		comm[i] = VG_LINE_TO_ABS;
	}
	comm[count] = VG_CLOSE_PATH;

	if (closed) err = append(path, count+1, comm, count*2, points);
	else        err = append(path, count, comm, count*2, points);

	free(comm);
	return err;
}

VGU_API_CALL VGUErrorCode 
vguRect(
	VGPath  path,
	VGfloat x, 
	VGfloat y,
	VGfloat width, 
	VGfloat height)
{
	VGUErrorCode err = VGU_NO_ERROR;

	VGubyte comm[5] = 
	{
		VG_MOVE_TO_ABS, VG_HLINE_TO_REL,
		VG_VLINE_TO_REL, VG_HLINE_TO_REL,
		VG_CLOSE_PATH 
	};

	VGfloat data[5];

	if (width <= 0 || height <= 0)
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}
	
	data[0] = x;     
	data[1] = y;
	data[2] = width; 
	data[3] = height;
	data[4] = -width;

	err = append(path, 5, comm, 5, data);
	return err;
}

VGU_API_CALL VGUErrorCode 
vguRoundRect(
	VGPath  path,
	VGfloat x, 
	VGfloat y,
	VGfloat width, 
	VGfloat height,
	VGfloat arcWidth, 
	VGfloat arcHeight)
{
	VGUErrorCode err = VGU_NO_ERROR;

	VGubyte comm[10] = 
	{
		VG_MOVE_TO_ABS,
		VG_HLINE_TO_REL, VG_SCCWARC_TO_REL,
		VG_VLINE_TO_REL, VG_SCCWARC_TO_REL,
		VG_HLINE_TO_REL, VG_SCCWARC_TO_REL,
		VG_VLINE_TO_REL, VG_SCCWARC_TO_REL,
		VG_CLOSE_PATH 
	};

	VGfloat data[26];
	VGfloat rx, ry;

	if (width <= 0 || height <= 0)
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}

	ITE_CLAMP(arcWidth, 0.0f, width);
	ITE_CLAMP(arcHeight, 0.0f, height);
	rx = arcWidth/2;
	ry = arcHeight/2;

	data[0]  =  x + rx; data[1] = y;

	data[2]  =  width - arcWidth;
	data[3]  =  rx; data[4]  =  ry; data[5]  = 0;
	data[6]  =  rx; data[7]  =  ry;

	data[8]  =  height - arcHeight;
	data[9]  =  rx; data[10] =  ry; data[11] = 0;
	data[12] = -rx; data[13] =  ry;

	data[14] = -(width - arcWidth);
	data[15] =  rx; data[16] =  ry; data[17] = 0;
	data[18] = -rx; data[19] = -ry;

	data[20] = -(height - arcHeight);
	data[21] =  rx; data[22] =  ry; data[23] = 0;
	data[24] =  rx; data[25] = -ry;

	err = append(path, 10, comm, 26, data);
	return err;
}

VGU_API_CALL VGUErrorCode 
vguEllipse(
	VGPath  path,
	VGfloat cx, 
	VGfloat cy,
	VGfloat width, 
	VGfloat height)
{
	VGUErrorCode err = VGU_NO_ERROR;

	const VGubyte comm[] = 
	{
		VG_MOVE_TO_ABS, 
		VG_SCCWARC_TO_REL,
		VG_SCCWARC_TO_REL, 
		VG_CLOSE_PATH
	};

	VGfloat data[12];

	if (width <= 0 || height <= 0)
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}

	data[0] = cx + width/2; data[1] = cy;

	data[2] = width/2; data[3] = height/2; data[4] = 0;
	data[5] = -width; data[6] = 0;

	data[7] = width/2; data[8] = height/2; data[9] = 0;
	data[10] = width; data[11] = 0;

	err = append(path, 4, comm, 12, data);
	return err;
}

#include <stdio.h>

VGU_API_CALL VGUErrorCode 
vguArc(
	VGPath     path,
	VGfloat    x, 
	VGfloat    y,
	VGfloat    width, 
	VGfloat    height,
	VGfloat    startAngle, 
	VGfloat    angleExtent,
	VGUArcType arcType)
{
	VGUErrorCode err = VGU_NO_ERROR;

	VGubyte commStart[1] = {VG_MOVE_TO_ABS};
	VGfloat dataStart[2];

	VGubyte commArcCCW[1] = {VG_SCCWARC_TO_ABS};
	VGubyte commArcCW[1]  = {VG_SCWARC_TO_ABS};
	VGfloat dataArc[5];

	VGubyte commEndPie[2] = {VG_LINE_TO_ABS, VG_CLOSE_PATH};
	VGfloat dataEndPie[2];

	VGubyte commEndChord[1] = {VG_CLOSE_PATH};
	VGfloat dataEndChord[1] = {0.0f};

	VGfloat alast, a = 0.0f;
	VGfloat rx = width/2, ry = height/2;

	if (width <= 0 || height <= 0)
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}

	if (    arcType != VGU_ARC_OPEN 
		 && arcType != VGU_ARC_CHORD 
		 && arcType != VGU_ARC_PIE)
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}

	startAngle = ITE_DEG2RAD(startAngle);
	angleExtent = ITE_DEG2RAD(angleExtent);
	alast = startAngle + angleExtent;

	dataStart[0] = x + ITE_COS(startAngle) * rx;
	dataStart[1] = y + ITE_SIN(startAngle) * ry;
	err = append(path, 1, commStart, 2, dataStart);
	if (err != VGU_NO_ERROR) 
	{
		return err;
	}

	dataArc[0] = rx;
	dataArc[1] = ry;
	dataArc[2] = 0.0f;

	if (angleExtent > 0)
	{
		a = startAngle + PI;
		while (a < alast)
		{
			dataArc[3] = x + ITE_COS(a) * rx;
			dataArc[4] = y + ITE_SIN(a) * ry;
			err = append(path, 1, commArcCCW, 5, dataArc);
			if (err != VGU_NO_ERROR) 
			{
				return err;
			}
			a += PI; 
		}

		dataArc[3] = x + ITE_COS(alast) * rx;
		dataArc[4] = y + ITE_SIN(alast) * ry;
		err = append(path, 1, commArcCCW, 5, dataArc);
		if (err != VGU_NO_ERROR) 
		{
			return err;
		}
	}
	else
	{
		a = startAngle - PI;
		while (a > alast)
		{
			dataArc[3] = x + ITE_COS(a) * rx;
			dataArc[4] = y + ITE_SIN(a) * ry;
			err = append(path, 1, commArcCW, 5, dataArc);
			if (err != VGU_NO_ERROR) 
			{
				return err;
			}
			a -= PI; 
		}

		dataArc[3] = x + ITE_COS(alast) * rx;
		dataArc[4] = y + ITE_SIN(alast) * ry;
		err = append(path, 1, commArcCW, 5, dataArc);
		if (err != VGU_NO_ERROR) 
		{
			return err;
		}
	}


	if (arcType == VGU_ARC_PIE) 
	{
		dataEndPie[0] = x; dataEndPie[1] = y;
		err = append(path, 2, commEndPie, 2, dataEndPie);
	}
	else if (arcType == VGU_ARC_CHORD) 
	{
		err = append(path, 1, commEndChord, 0, dataEndChord);
	}

	return err;
}

VGU_API_CALL VGUErrorCode 
vguComputeWarpQuadToSquare(
	VGfloat  sx0, 
	VGfloat  sy0,
	VGfloat  sx1, 
	VGfloat  sy1,
	VGfloat  sx2, 
	VGfloat  sy2,
	VGfloat  sx3, 
	VGfloat  sy3,
	VGfloat* matrix)
{
	VGUErrorCode ret;
	VGfloat      mat[9];
	ITEMatrix3x3 tempMatrix;
	ITEMatrix3x3 invMatrix;
	
	if(!matrix || !CheckAlignment(matrix, sizeof(VGfloat)))
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}
	
	ret = vguComputeWarpSquareToQuad(sx0, sy0, sx1, sy1, sx2, sy2, sx3, sy3, mat);
	if(ret == VGU_BAD_WARP_ERROR)
	{
		return VGU_BAD_WARP_ERROR;
	}

	SETMAT(tempMatrix, 
		mat[0], mat[3], mat[6], 
		mat[1], mat[4], mat[7], 
		mat[2], mat[5], mat[8]);
	
	iteInvertMatrix(&tempMatrix, &invMatrix);
			
	matrix[0] = tempMatrix.m[0][0];
	matrix[1] = tempMatrix.m[1][0];
	matrix[2] = tempMatrix.m[2][0];
	matrix[3] = tempMatrix.m[0][1];
	matrix[4] = tempMatrix.m[1][1];
	matrix[5] = tempMatrix.m[2][1];
	matrix[6] = tempMatrix.m[0][2];
	matrix[7] = tempMatrix.m[1][2];
	matrix[8] = tempMatrix.m[2][2];
	
	return VGU_NO_ERROR;
}

VGU_API_CALL VGUErrorCode 
vguComputeWarpSquareToQuad(
	VGfloat  dx0, 
	VGfloat  dy0,
	VGfloat  dx1, 
	VGfloat  dy1,
	VGfloat  dx2, 
	VGfloat  dy2,
	VGfloat  dx3, 
	VGfloat  dy3,
	VGfloat* matrix)
{
	VGfloat diffx1;
	VGfloat diffy1;
	VGfloat diffx2;
	VGfloat diffy2;
	VGfloat det;
	VGfloat sumx;
	VGfloat sumy;
	VGfloat oodet;
	VGfloat g;
	VGfloat h;
	
	if(!matrix || !CheckAlignment(matrix, sizeof(VGfloat)))
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}

	//from Heckbert:Fundamentals of Texture Mapping and Image Warping
	//Note that his mapping of vertices is different from OpenVG's
	//(0,0) => (dx0,dy0)
	//(1,0) => (dx1,dy1)
	//(0,1) => (dx2,dy2)
	//(1,1) => (dx3,dy3)

	diffx1 = dx1 - dx3;
	diffy1 = dy1 - dy3;
	diffx2 = dx2 - dx3;
	diffy2 = dy2 - dy3;

	det = diffx1*diffy2 - diffx2*diffy1;
	if(det == 0.0f)
	{
		return VGU_BAD_WARP_ERROR;
	}

	sumx = dx0 - dx1 + dx3 - dx2;
	sumy = dy0 - dy1 + dy3 - dy2;

	if(sumx == 0.0f && sumy == 0.0f)
	{
		//affine mapping
		matrix[0] = dx1 - dx0;
		matrix[1] = dy1 - dy0;
		matrix[2] = 0.0f;
		matrix[3] = dx3 - dx1;
		matrix[4] = dy3 - dy1;
		matrix[5] = 0.0f;
		matrix[6] = dx0;
		matrix[7] = dy0;
		matrix[8] = 1.0f;
		return VGU_NO_ERROR;
	}

	oodet = 1.0f / det;
	g = (sumx*diffy2 - diffx2*sumy) * oodet;
	h = (diffx1*sumy - sumx*diffy1) * oodet;

	matrix[0] = dx1-dx0+g*dx1;
	matrix[1] = dy1-dy0+g*dy1;
	matrix[2] = g;
	matrix[3] = dx2-dx0+h*dx2;
	matrix[4] = dy2-dy0+h*dy2;
	matrix[5] = h;
	matrix[6] = dx0;
	matrix[7] = dy0;
	matrix[8] = 1.0f;
	
	return VGU_NO_ERROR;
}

VGU_API_CALL VGUErrorCode 
vguComputeWarpQuadToQuad(
	VGfloat  dx0, 
	VGfloat  dy0,
	VGfloat  dx1, 
	VGfloat  dy1,
	VGfloat  dx2, 
	VGfloat  dy2,
	VGfloat  dx3, 
	VGfloat  dy3,
	VGfloat  sx0, 
	VGfloat  sy0,
	VGfloat  sx1, 
	VGfloat  sy1,
	VGfloat  sx2, 
	VGfloat  sy2,
	VGfloat  sx3, 
	VGfloat  sy3,
	VGfloat* matrix)
{
	VGfloat qtos[9];
	VGfloat stoq[9];
	VGUErrorCode ret1;
	VGUErrorCode ret2;
	ITEMatrix3x3 m1;
	ITEMatrix3x3 m2;
	ITEMatrix3x3 mout;
	
	if(!matrix || !CheckAlignment(matrix, sizeof(VGfloat)))
	{
		return VGU_ILLEGAL_ARGUMENT_ERROR;
	}

	ret1 = vguComputeWarpQuadToSquare(sx0, sy0, sx1, sy1, sx2, sy2, sx3, sy3, qtos);
	if(ret1 == VGU_BAD_WARP_ERROR)
	{
		return VGU_BAD_WARP_ERROR;
	}

	ret2 = vguComputeWarpSquareToQuad(dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3, stoq);
	if(ret2 == VGU_BAD_WARP_ERROR)
	{
		return VGU_BAD_WARP_ERROR;
	}

	SETMAT(m1, 
		qtos[0], qtos[3], qtos[6], 
		qtos[1], qtos[4], qtos[7], 
		qtos[2], qtos[5], qtos[8]);
	
	SETMAT(m2, 
		stoq[0], stoq[3], stoq[6], 
		stoq[1], stoq[4], stoq[7], 
		stoq[2], stoq[5], stoq[8]);

	MULMATMAT(m2, m1, mout);

	matrix[0] = mout.m[0][0];
	matrix[1] = mout.m[1][0];
	matrix[2] = mout.m[2][0];
	matrix[3] = mout.m[0][1];
	matrix[4] = mout.m[1][1];
	matrix[5] = mout.m[2][1];
	matrix[6] = mout.m[0][2];
	matrix[7] = mout.m[1][2];
	matrix[8] = mout.m[2][2];
	
	return VGU_NO_ERROR;
}
