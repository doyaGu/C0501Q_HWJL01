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
#include "itePaint.h"
#include <stdio.h>
#include "iteHardware.h"

#define _ITEM_T ITEFloatStop
#define _ARRAY_T ITEFloatStopArray
#define _FUNC_T iteFloatStopArray
#define _COMPARE_T(s1,s2) 0
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

#define _ITEM_T ITEStop
#define _ARRAY_T ITEStopArray
#define _FUNC_T iteStopArray
#define _COMPARE_T(s1,s2) 0
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

#define _ITEM_T ITEPaint*
#define _ARRAY_T ITEPaintArray
#define _FUNC_T itePaintArray
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

ITEint iteIsValidPaintMode(VGPaintMode paintMode)
{
	return (paintMode == VG_STROKE_PATH ||
		    paintMode == VG_FILL_PATH);
}

ITEint iteIsValidPaintModeBit(VGbitfield paintMode)
{
	return ((paintMode & VG_STROKE_PATH) ||
		    (paintMode & VG_FILL_PATH));
}

void ITEPaint_ctor(ITEPaint *p)
{
	int i;

  	p->objType = ITE_VG_OBJ_PAINT;
	p->type = VG_PAINT_TYPE_COLOR;
	CSET(p->color_input, 0,0,0,255);
	CSET(p->color, 0,0,0,255);
	ITE_INITOBJ(ITEFloatStopArray, p->instops_input);
	ITE_INITOBJ(ITEStopArray, p->instops);
	ITE_INITOBJ(ITEStopArray, p->stops);
	p->premultiplied = VG_TRUE;
	p->spreadMode = VG_COLOR_RAMP_SPREAD_PAD;
	p->tilingMode = VG_TILE_FILL;
	for (i=0; i<4; ++i)
	{
		p->linearGradient_input[i] = 0.0f;
		p->linearGradient[i]       = 0.0f;
	}
	for (i=0; i<5; ++i)
	{
		p->radialGradient_input[i] = 0.0f;
		p->radialGradient[i]       = 0.0f;
	}
	p->pattern = VG_INVALID_HANDLE;

}

void ITEPaint_dtor(ITEPaint *p)
{
	ITE_DEINITOBJ(ITEFloatStopArray, p->instops_input);
	ITE_DEINITOBJ(ITEStopArray, p->instops);
	ITE_DEINITOBJ(ITEStopArray, p->stops); 
}

void iteValidateInputStops(ITEPaint *p)
{
	ITEStop *instop, stop;
	ITEfloat lastOffset=0.0f;
	int i;
	//ITEHardware *h;

	VG_GETCONTEXT(VG_NO_RETVAL);
	//h = context->hardware;
  
	iteStopArrayClear(&p->stops);
	iteStopArrayReserve(&p->stops, p->instops.size);
  
	/* Assure input stops are properly defined */
	for (i=0; i<p->instops.size; ++i) {
    
		/* Copy stop color */
		instop = &p->instops.items[i];
		stop.color = instop->color;
    
		/* Offset must be in [0,1] */
		if (instop->offset < 0.0f || instop->offset > 1.0f)
			continue;
    
		/* Discard whole sequence if not in ascending order */
		if (instop->offset < lastOffset)
		{
			iteStopArrayClear(&p->stops); 
			break;
		}
    
		/* Add stop at offset 0 with same color if first not at 0 */
		if ( p->stops.size == 0 && instop->offset != 0.0f ) 
		{
			stop.offset = 0.0f;
			iteStopArrayPushBackP(&p->stops, &stop);
		}
    
		/* Add current stop to array */
		stop.offset = instop->offset;
		iteStopArrayPushBackP(&p->stops, &stop);
    
		/* Save last offset */
		lastOffset = instop->offset;
	}
  
	/* Add stop at offset 1 with same color if last not at 1 */
	if (p->stops.size > 0 && lastOffset != 1.0f)
	{
		stop.offset = 1.0f;
		iteStopArrayPushBackP(&p->stops, &stop);
	}
  
	/* Add 2 default stops if no valid found */
	if (p->stops.size == 0) 
	{
		/* First opaque black */
		stop.offset = 0.0f;
		CSET(stop.color, 0,0,0,255);
		iteStopArrayPushBackP(&p->stops, &stop);
		/* Last opaque white */
		stop.offset = 1.0f;
		CSET(stop.color, 255,255,255,255);
		iteStopArrayPushBackP(&p->stops, &stop);
	}

	// Awin@20100307, below code was C model, it should be disabled
	/*
	if( h->gradientData )
	{
		ITEint i,j;
		for(i=0;i<p->stops.size-1;i++)
		{
			ITEint start = (ITEint)(p->stops.items[i].offset*(1<<h->gradientLen));
			ITEint end = (ITEint)(p->stops.items[i+1].offset*(1<<h->gradientLen));
			for(j=start;j<end;j++)
			{
				ITEfloat g;
				ITEColor sc, ec, c;
                                ITEHColor hc;

				g = (ITEfloat)(j - start) / (end - start);
				ec = p->stops.items[i+1].color;
				sc = p->stops.items[i].color; 
				CMUL(sc, (1.0f-g));
				CMUL(ec, g);
				CSETC(c, sc)
				CADDC(c, ec);
				iteStoreColor( &c, &hc, &h->gradientData[j*4], VG_sRGBA_8888, 0);  //return interpolated value
			}	
		}
	}
	*/
}

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

VG_API_CALL VGPaint vgCreatePaint(void)
{
  ITEPaint *p = NULL;
  VG_GETCONTEXT(VG_INVALID_HANDLE);
  
  /* Create new paint object */
  ITE_NEWOBJ(ITEPaint, p);
  VG_RETURN_ERR_IF(!p, VG_OUT_OF_MEMORY_ERROR,
                   VG_INVALID_HANDLE);
  
  /* Add to resource list */
  itePaintArrayPushBack(&context->paints, p);
  
  VG_RETURN((VGPaint)p);
}

VG_API_CALL void vgDestroyPaint(VGPaint paint)
{
  ITEint index;
  VG_GETCONTEXT(VG_NO_RETVAL);
  
  /* Check if handle valid */
  index = itePaintArrayFind(&context->paints, (ITEPaint*)paint);
  VG_RETURN_ERR_IF(index == -1, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
  
  /* Delete object and remove resource */
  ITE_DELETEOBJ(ITEPaint, (ITEPaint*)paint);
  itePaintArrayRemoveAt(&context->paints, index);
  
  VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgSetPaint(VGPaint paint, VGbitfield paintModes)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Check if handle valid */
	VG_RETURN_ERR_IF(!iteIsValidPaint(context, paint) &&
	                 paint != VG_INVALID_HANDLE,
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* Check for invalid mode */
	VG_RETURN_ERR_IF(!paintModes ||
		             (paintModes & ~(VG_STROKE_PATH | VG_FILL_PATH)),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Set stroke / fill */
	if (paintModes & VG_STROKE_PATH)
	{
		context->strokePaint = (ITEPaint*)paint;
	}
	if (paintModes & VG_FILL_PATH)
	{
		context->fillPaint = (ITEPaint*)paint;
	}

	if ( paint == VG_INVALID_HANDLE )
	{
		context->strokePaint = &context->defaultPaint;
		context->fillPaint = &context->defaultPaint;
	}

	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL VGPaint vgGetPaint(VGPaintMode paintMode)
{
	VG_GETCONTEXT(VG_INVALID_HANDLE);

    VG_RETURN_ERR_IF(!iteIsValidPaintMode(paintMode),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);

	if (paintMode & VG_STROKE_PATH)
		VG_RETURN((VGPaint)context->strokePaint);

	// VG_FILL_PATH
	VG_RETURN((VGPaint)context->fillPaint);
}

VG_API_CALL void vgSetColor(VGPaint paint, VGuint rgba)
{
  ITEPaint* p;

  VG_GETCONTEXT(VG_NO_RETVAL);
  
  /* Check if handle valid */
  VG_RETURN_ERR_IF(!iteIsValidPaint(context, paint),
                   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
  
  // set color
  p = (ITEPaint*)paint;
  p->color.r = (ITEuint8)((rgba>>24) & 0xff);
  p->color.g = (ITEuint8)((rgba>>16) & 0xff);  
  p->color.b = (ITEuint8)((rgba>>8) & 0xff);
  p->color.a = rgba & 0xff;

  p->premultiplied = HW_FALSE;
  
  VG_RETURN(VG_NO_RETVAL);
}


VG_API_CALL VGuint vgGetColor(VGPaint paint)
{
  ITEPaint* p;
  VGuint	ret;

  VG_GETCONTEXT(0);
  
  /* Check if handle valid */
  VG_RETURN_ERR_IF(!iteIsValidPaint(context, paint),
					VG_BAD_HANDLE_ERROR, 0);
	
  p = (ITEPaint*)paint;
  ret = (VGuint)( (p->color.r<<24) + (p->color.r<<16) + (p->color.r<<8) + p->color.a );
  return ret;
}


VG_API_CALL void vgPaintPattern(VGPaint paint, VGImage pattern)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if paint is not a valid paint handle, or is not shared with the current context
		¡V if pattern is neither a valid image handle nor equal to
		VG_INVALID_HANDLE, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidPaint(context, paint),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!iteIsValidImage(context, pattern) && (pattern != VG_INVALID_HANDLE),
		             VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if pattern is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, pattern), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL)

	/* Set pattern image */
	((ITEPaint*)paint)->pattern = (ITEImage*)pattern;

	VG_RETURN(VG_NO_RETVAL);
}

