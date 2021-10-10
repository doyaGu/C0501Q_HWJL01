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

#ifndef __ITEPAINT_H
#define __ITEPAINT_H

#include "iteDefs.h"
#include "iteArrays.h"
#include "iteImage.h"

typedef struct
{
  ITEfloat		offset;
  ITEFloatColor	color; 
} ITEFloatStop;

typedef struct
{
  ITEfloat offset;
  ITEColor color; 
} ITEStop;

#define _ITEM_T ITEFloatStop
#define _ARRAY_T ITEFloatStopArray
#define _FUNC_T iteFloatStopArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#define _ITEM_T ITEStop
#define _ARRAY_T ITEStopArray
#define _FUNC_T iteStopArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

typedef struct
{
	ITE_VG_OBJ_TYPE			objType;
	VGPaintType				type;
	ITEFloatColor			color_input;
	ITEColor				color;
	ITEFloatStopArray		instops_input;
	ITEStopArray			instops;
	ITEStopArray			stops;
	VGboolean				premultiplied;
	VGColorRampSpreadMode	spreadMode;
	VGTilingMode			tilingMode;
	ITEfloat				linearGradient_input[4];
	ITEfloat				linearGradient[4];
	ITEfloat				radialGradient_input[5];
	ITEfloat				radialGradient[5];
	ITEImage*				pattern;
} ITEPaint;

#define ITE_GRADIENT_TEX_SIZE 1024

void ITEPaint_ctor(ITEPaint *p);
void ITEPaint_dtor(ITEPaint *p);
ITEint iteIsValidPaintMode(VGPaintMode paintMode);
ITEint iteIsValidPaintModeBit(VGbitfield paintMode);

#define _ITEM_T ITEPaint*
#define _ARRAY_T ITEPaintArray
#define _FUNC_T itePaintArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

void iteValidateInputStops(ITEPaint *p);

#endif /* __ITEPAINT_H */
