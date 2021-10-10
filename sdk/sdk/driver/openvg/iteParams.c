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
#include <stdio.h>
#include "openvg.h"
#include "iteContext.h"
#include "iteUtility.h"

/*----------------------------------------------------
 * Returns true (1) if the specified parameter takes
 * a vector of values (more than 1) or false (0)
 * otherwise.
 *----------------------------------------------------*/

static int iteIsParamVector(ITEint type)
{
  return
    (type == VG_SCISSOR_RECTS ||
     type == VG_STROKE_DASH_PATTERN ||
     type == VG_TILE_FILL_COLOR ||
     type == VG_CLEAR_COLOR ||
     type == VG_PAINT_COLOR ||
     type == VG_PAINT_COLOR_RAMP_STOPS ||
     type == VG_PAINT_LINEAR_GRADIENT ||
     type == VG_PAINT_RADIAL_GRADIENT);
}

/*----------------------------------------------------
 * Returns true (1) if the given integer is a valid
 * enumeration value for the specified parameter or
 * false (0) otherwise.
 *----------------------------------------------------*/
  
static int iteIsEnumValid(ITEint type, VGint val)
{
	switch(type)
	{
	case VG_MATRIX_MODE:
		return (val == VG_MATRIX_PATH_USER_TO_SURFACE  ||
				val == VG_MATRIX_IMAGE_USER_TO_SURFACE ||
				val == VG_MATRIX_FILL_PAINT_TO_USER    ||
				val == VG_MATRIX_STROKE_PAINT_TO_USER  ||
				val == VG_MATRIX_GLYPH_USER_TO_SURFACE);

	case VG_FILL_RULE:
		return (val == VG_EVEN_ODD ||
				val == VG_NON_ZERO)  ? VG_TRUE : VG_FALSE;

	case VG_IMAGE_QUALITY:
		return (val == VG_IMAGE_QUALITY_NONANTIALIASED ||
				val == VG_IMAGE_QUALITY_FASTER ||
				val == VG_IMAGE_QUALITY_BETTER);

	case VG_RENDERING_QUALITY:
		return (val == VG_RENDERING_QUALITY_NONANTIALIASED ||
				val == VG_RENDERING_QUALITY_FASTER ||
				val == VG_RENDERING_QUALITY_BETTER);

	case VG_BLEND_MODE:
		return (val == VG_BLEND_SRC ||
				val == VG_BLEND_SRC_OVER ||
				val == VG_BLEND_DST_OVER ||
				val == VG_BLEND_SRC_IN ||
				val == VG_BLEND_DST_IN ||
				val == VG_BLEND_MULTIPLY ||
				val == VG_BLEND_SCREEN ||
				val == VG_BLEND_DARKEN ||
				val == VG_BLEND_LIGHTEN ||
				val == VG_BLEND_ADDITIVE /*||
				val == VG_BLEND_SRC_OUT_SH ||
				val == VG_BLEND_DST_OUT_SH ||
				val == VG_BLEND_SRC_ATOP_SH ||
				val == VG_BLEND_DST_ATOP_SH*/);

	case VG_IMAGE_MODE:
		return (val == VG_DRAW_IMAGE_NORMAL ||
				val == VG_DRAW_IMAGE_MULTIPLY ||
				val == VG_DRAW_IMAGE_STENCIL);

	case VG_STROKE_CAP_STYLE:
		return (val == VG_CAP_BUTT ||
				val == VG_CAP_ROUND ||
				val == VG_CAP_SQUARE);

	case VG_STROKE_JOIN_STYLE:
		return (val == VG_JOIN_MITER ||
				val == VG_JOIN_ROUND ||
				val == VG_JOIN_BEVEL);

	case VG_STROKE_DASH_PHASE_RESET:
	case VG_SCISSORING:
	case VG_MASKING:
		return (val == VG_TRUE ||
				val == VG_FALSE);

	case VG_PIXEL_LAYOUT:
		return (val == VG_PIXEL_LAYOUT_UNKNOWN ||
				val == VG_PIXEL_LAYOUT_RGB_VERTICAL ||
				val == VG_PIXEL_LAYOUT_BGR_VERTICAL ||
				val == VG_PIXEL_LAYOUT_RGB_HORIZONTAL ||
				val == VG_PIXEL_LAYOUT_BGR_HORIZONTAL);

	case VG_FILTER_FORMAT_LINEAR:
	case VG_FILTER_FORMAT_PREMULTIPLIED:
		return (val == VG_TRUE ||
				val == VG_FALSE);

	case VG_PAINT_TYPE:
		return (val == VG_PAINT_TYPE_COLOR ||
				val == VG_PAINT_TYPE_LINEAR_GRADIENT ||
				val == VG_PAINT_TYPE_RADIAL_GRADIENT ||
				val == VG_PAINT_TYPE_PATTERN);

	case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
		return (val == VG_COLOR_RAMP_SPREAD_PAD ||
				val == VG_COLOR_RAMP_SPREAD_REPEAT ||
				val == VG_COLOR_RAMP_SPREAD_REFLECT);

	case VG_PAINT_PATTERN_TILING_MODE:
		return (val == VG_TILE_FILL ||
				val == VG_TILE_PAD ||
				val == VG_TILE_REPEAT ||
				val == VG_TILE_REFLECT);

	case VG_IMAGE_FORMAT:
		return (val >= VG_sRGBX_8888 &&
				val <= VG_lABGR_8888_PRE);

	default:
		return 1;
	}
}

/*--------------------------------------------------------
 * These two functions check for invalid (erroneus) float
 * input and correct it to acceptable ranges.
 *---------------------------------------------------*/

ITEfloat getMaxFloat()
{
  ITEfloatint fi;
  fi.i = ITE_MAX_FLOAT_BITS;
  return fi.f;
}

ITEfloat iteValidInputFloat(VGfloat f)
{
	ITEfloat max = getMaxFloat();
	if (ITE_ISNAN(f))
	{
		return 0.0f; /* convert NAN to zero */
	}
	ITE_CLAMP(f, -max, max); /* clamp to valid range */
	return (ITEfloat)f;
}

static ITEint iteValidInputFloat2Int(VGfloat f)
{
	double v = (double)ITE_FLOOR(iteValidInputFloat(f));
	ITE_CLAMP(v, (double)ITE_MIN_INT, (double)ITE_MAX_INT);
	return (ITEint)v;
}

static ITEint iteValidInputFloat2Int2(VGfloat f)
{
	double v = (double)ITE_FLOOR(f);
	ITE_CLAMP(v, (double)ITE_MIN_INT, (double)ITE_MAX_INT);
	return (ITEint)v;
}

/*---------------------------------------------------
 * Interpretes the input value vector as an array of
 * integers and returns the value at given index
 *---------------------------------------------------*/

static ITEint iteParamAlwaysToInt(const void *values, ITEint floats, ITEint index)
{
	return (ITEint)((const VGint*)values)[index];
}

/*---------------------------------------------------
 * Interpretes the input value vector as an array of
 * integers and returns the value at given index
 *---------------------------------------------------*/

static ITEint iteParamToInt(const void *values, ITEint floats, ITEint index)
{
	if (floats)
	{
		return iteValidInputFloat2Int(((const VGfloat*)values)[index]);
	}
	else
	{
		return (ITEint)((const VGint*)values)[index];
	}
}

/*---------------------------------------------------
 * Interpretes the input value vector as an array of
 * integers and returns the value at given index
 *---------------------------------------------------*/
 
static ITEint iteParamToInt2(const void *values, ITEint floats, ITEint index)
{
	if (floats)
	{
		return iteValidInputFloat2Int2(((const VGfloat*)values)[index]);
	}
	else
	{
		return (ITEint)((const VGint*)values)[index];
	}
}

/*---------------------------------------------------
 * Interpretes the input value vector as an array of
 * floats and returns the value at given index
 *---------------------------------------------------*/

static VGfloat iteParamToValidFloat(const void *values, ITEint floats, ITEint index)
{
	if (floats)
	{
		return iteValidInputFloat(((const VGfloat*)values)[index]);
	}
	else
	{
		return (ITEfloat)((const VGint*)values)[index];
	}
}

/*---------------------------------------------------
 * Interpretes the input value vector as an array of
 * floats and returns the value at given index
 *---------------------------------------------------*/

static VGfloat iteParamToFloat(const void *values, ITEint floats, ITEint index)
{
	if (floats)
	{
		return ((const VGfloat*)values)[index];
	}
	else
	{
		return (ITEfloat)((const VGint*)values)[index];
	}
}

/*---------------------------------------------------
 * Interpretes the output value vector as an array of
 * integers and sets the value at given index
 *---------------------------------------------------*/

static void iteIntToParam(ITEint i, ITEint count, void *output, ITEint floats, ITEint index)
{
	if (index >= count)
	{
		return;
	}
	if (floats)
	{
		((VGfloat*)output)[index] = (VGfloat)i;
	}
	else
	{
		((VGint*)output)[index] = (VGint)i;
	}
}

/*---------------------------------------------------
 * Interpretes the output value vector as an array of
 * integers and sets the value at given index
 *---------------------------------------------------*/

static void iteIntToParamAddressCast(ITEint i, ITEint count, void *output, ITEint floats, ITEint index)
{
	if (index >= count)
	{
		return;
	}
	if (floats)
	{
		((VGfloat*)output)[index] = *(VGfloat*)&i;
	}
	else
	{
		((VGint*)output)[index] = (VGint)i;
	}
}

/*----------------------------------------------------
 * Interpretes the output value vector as an array of
 * floats and sets the value at given index
 *----------------------------------------------------*/

static void iteValidFloatToParam(ITEfloat f, ITEint count, void *output, ITEint floats, ITEint index)
{
	if (index >= count)
	{
		return;
	}
	if (floats)
	{
		((VGfloat*)output)[index] = (VGfloat)f;
	}
	else
	{
		((VGint*)output)[index] = (VGint)iteValidInputFloat2Int(f);
	}
}

/*----------------------------------------------------
 * Interpretes the output value vector as an array of
 * floats and sets the value at given index
 *----------------------------------------------------*/

static void iteFloatToParam(ITEfloat f, ITEint count, void *output, ITEint floats, ITEint index)
{
	if (index >= count)
	{
		return;
	}
	if (floats)
	{
		((VGfloat*)output)[index] = (VGfloat)f;
	}
	else
	{
		((VGint*)output)[index] = (VGint)iteValidInputFloat2Int2(f);
	}
}

/*---------------------------------------------------------
 * Sets a parameter by interpreting the input value vector
 * according to the parameter type and input type.
 *---------------------------------------------------------*/

static void iteSet(VGContext *context, VGParamType type, ITEint count,
                  const void* values, ITEint floats)
{
	ITEfloat fvalue = 0.0f;
	ITEint ivalue = 0;
	VGboolean bvalue = VG_FALSE;
	int i = 0;

	/* Check for negative count */
	ITE_RETURN_ERR_IF(count<0, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);

	/* Check for empty vector */
	ITE_RETURN_ERR_IF(!values && count!=0, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);

	/* Pre-convert first value for non-vector params */
	if (count == 1)
	{
		//fvalue = iteParamToFloat(values, floats, 0);
		fvalue = iteParamToFloat(values, floats, 0);
		ivalue = iteParamToInt(values, floats, 0);
		bvalue = (ivalue ? VG_TRUE : VG_FALSE);
	}

	switch (type)
	{
	case VG_MATRIX_MODE:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->matrixMode = (VGMatrixMode)ivalue;
		context->updateFlag.modeFlag = VG_TRUE;
		break;

	case VG_FILL_RULE:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->fillRule = (VGFillRule)ivalue;
		context->updateFlag.modeFlag = VG_TRUE;
		break;

	case VG_IMAGE_QUALITY:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->imageQuality = (VGImageQuality)ivalue;
		context->updateFlag.modeFlag = VG_TRUE;
		break;

	case VG_RENDERING_QUALITY:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->renderingQuality = (VGRenderingQuality)ivalue;
		context->updateFlag.modeFlag = VG_TRUE;
		break;

	case VG_BLEND_MODE:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->blendMode = (VGBlendMode)ivalue;
		context->updateFlag.modeFlag = VG_TRUE;
		break;

	case VG_IMAGE_MODE:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->imageMode = (VGImageMode)ivalue;
		context->updateFlag.modeFlag = VG_TRUE;
		break;

	case VG_STROKE_CAP_STYLE:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->strokeCapStyle = (VGCapStyle)ivalue;
		break;

	case VG_STROKE_JOIN_STYLE:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->strokeJoinStyle = (VGJoinStyle)ivalue;
		break;

	case VG_PIXEL_LAYOUT:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		ITE_RETURN_ERR_IF(!iteIsEnumValid(type,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->pixelLayout = (VGPixelLayout)ivalue;
		break;

	case VG_FILTER_CHANNEL_MASK:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->filterChannelMask = (VGbitfield)ivalue;
		break;

	case VG_FILTER_FORMAT_LINEAR:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->filterFormatLinear = bvalue;
		break;

	case VG_FILTER_FORMAT_PREMULTIPLIED:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->filterFormatPremultiplied = bvalue;
		break;

	case VG_STROKE_DASH_PHASE_RESET:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->strokeDashPhaseReset = bvalue;
		break;

	case VG_MASKING:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->masking = bvalue;
		context->updateFlag.enableFlag = VG_TRUE;
		break;

	case VG_SCISSORING:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->scissoring = bvalue;
		context->updateFlag.enableFlag = VG_TRUE;

		if ( context->scissoring )
		{
			if( context->scissorImage.data == NULL )
			{
				context->scissorImage.width    = context->surface->colorImage->width;
				context->scissorImage.height   = context->surface->colorImage->height;
                context->scissorImage.pitch    = GetAlignment(context->surface->colorImage->width, 64) / 8;
				context->scissorImage.vgformat = VG_A_1;
				context->scissorImage.data     = (ITEuint8*)VG_VMemAlloc(context->scissorImage.width*context->scissorImage.pitch);
				context->scissorImage.objectID = INVALID_OBJECT_ID;
			}
			
			{
				ITERectangle* rect;
				ITEColor      scissorColor;
				ITEint        rectIndex;

				/* Clear scissor image */
				CSET(scissorColor, 0xFF, 0xFF, 0xFF, 0x00);
				iteSetImage(ITE_FALSE, &context->scissorImage, 0, 0, context->scissorImage.width, context->scissorImage.height, scissorColor);

				/* Set scissor rectangle */
				CSET(scissorColor, 0, 0, 0, 0xFF);
				for(rectIndex = 0; rectIndex < context->scissor.size; rectIndex++)
				{
					rect = &context->scissor.items[rectIndex];

					/* A rectangle with width <= 0 or height <= 0 is ignored. */
					if (   (rect->w > 0)
						&& (rect->h > 0)
						&& ((rect->x > 0) && ((rect->x + rect->w) < context->scissorImage.width))
						&& ((rect->y > 0) && ((rect->y + rect->h) < context->scissorImage.height)) )
					{
				    	iteSetImage(ITE_FALSE, &context->scissorImage, rect->x, rect->y, rect->w, rect->h, scissorColor);
					}
				}
			}
		}
		else
		{
			if( context->scissorImage.data )
			{
				VG_VMemFree((uint32_t)context->scissorImage.data);
				context->scissorImage.data = NULL;
			}
		}
		break;

	case VG_STROKE_LINE_WIDTH:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->strokeLineWidth = fvalue;
		break;

	case VG_STROKE_MITER_LIMIT:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->strokeMiterLimit_input = fvalue;
		context->strokeMiterLimit = ITE_MAX(fvalue, 1.0f);
		break;

	case VG_STROKE_DASH_PHASE:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->strokeDashPhase = fvalue;
		break;

	case VG_STROKE_DASH_PATTERN:
		/* TODO: limit by the VG_MAX_DASH_COUNT value */
		iteFloatArrayClear(&context->strokeDashPattern);
		for (i=0; i<count; ++i)
		{
			iteFloatArrayPushBack(&context->strokeDashPattern, iteParamToFloat(values, floats, i));
		}
		break;
	case VG_TILE_FILL_COLOR:
		ITE_RETURN_ERR_IF(count!=4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		CSET(context->tileFillColor_input,
		     iteParamToFloat(values, floats, 0),
		     iteParamToFloat(values, floats, 1),
		     iteParamToFloat(values, floats, 2),
		     iteParamToFloat(values, floats, 3));
		CSET(context->tileFillColor,
		     iteParamToValidFloat(values, floats, 0),
		     iteParamToValidFloat(values, floats, 1),
		     iteParamToValidFloat(values, floats, 2),
		     iteParamToValidFloat(values, floats, 3));
		break;
		
	case VG_CLEAR_COLOR:
		ITE_RETURN_ERR_IF(count!=4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		CSET(context->clearColor_input,
		     iteParamToFloat(values, floats, 0),
		     iteParamToFloat(values, floats, 1),
		     iteParamToFloat(values, floats, 2),
		     iteParamToFloat(values, floats, 3));
		{
			ITEfloat clampR = context->clearColor_input.r;
			ITEfloat clampG = context->clearColor_input.g;
			ITEfloat clampB = context->clearColor_input.b;
			ITEfloat clampA = context->clearColor_input.a;

			ITE_CLAMP(clampR, 0.0f, 1.0f);
			ITE_CLAMP(clampG, 0.0f, 1.0f);
			ITE_CLAMP(clampB, 0.0f, 1.0f);
			ITE_CLAMP(clampA, 0.0f, 1.0f);
			CSET(context->clearColor,
				clampR * 255,
				clampG * 255,
				clampB * 255,
				clampA * 255);
		}
		break;
		
	case VG_SCISSOR_RECTS:
		ITE_RETURN_ERR_IF(count % 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteRectArrayClear(&context->scissor);
		//for (i=0; i<count && i<ITE_MAX_SCISSOR_RECTS; i+=4)
		for (i=0; i< ITE_MIN(count, ITE_MAX_SCISSOR_RECTS * 4); i+=4)
		{
			ITERectangle r;
			/*
			r.x = iteParamAlwaysToInt(values, floats, i+0);
			r.y = iteParamAlwaysToInt(values, floats, i+1);
			r.w = iteParamAlwaysToInt(values, floats, i+2);
			r.h = iteParamAlwaysToInt(values, floats, i+3);
			*/
			r.x = iteParamToInt2(values, floats, i+0);
			r.y = iteParamToInt2(values, floats, i+1);
			r.w = iteParamToInt2(values, floats, i+2);
			r.h = iteParamToInt2(values, floats, i+3);
			iteRectArrayPushBackP(&context->scissor, &r);
		}

		/* Generate scissor image */
		if ( context->scissoring /*&& context->scissor.size*/ )
		{
			ITEColor scissorColor;
			ITEint   rectIndex;

			/* Clear scissor image */
			CSET(scissorColor, 0xFF, 0xFF, 0xFF, 0x00);
			iteSetImage(ITE_FALSE, &context->scissorImage, 0, 0, context->scissorImage.width, context->scissorImage.height, scissorColor);

			/* Set scissor rectangle */
			CSET(scissorColor, 0, 0, 0, 0xFF);
			for(rectIndex = 0; rectIndex < context->scissor.size; rectIndex++)
			{
				ITERectangle* rect = &context->scissor.items[rectIndex];

				/* A rectangle with width <= 0 or height <= 0 is ignored. */
                if ( (rect->w > 0) && (rect->h > 0) )
				{
					ITERectangle normalRect = *rect;

					if ( normalRect.x < 0 )
					{
						normalRect.w = normalRect.w + normalRect.x;
						normalRect.x = 0;
					}
					else if ( normalRect.x >= context->scissorImage.width )
					{
						normalRect.w = 0;
						normalRect.x = context->scissorImage.width - 1;
					}
					if ( normalRect.y < 0 )
					{
						normalRect.h = normalRect.h + normalRect.y;
						normalRect.y = 0;
					}
					else if ( normalRect.y >= context->scissorImage.height )
					{
						normalRect.h = 0;
						normalRect.y = context->scissorImage.height - 1;
					}

					if ( (normalRect.w > 0) && (normalRect.h > 0) )
					{
			    		iteSetImage(ITE_FALSE, &context->scissorImage, normalRect.x, normalRect.y, normalRect.w, normalRect.h, scissorColor);
					}
				}
			}
		}
		context->updateFlag.scissorRectFlag = VG_TRUE;
		break;
		
	case VG_COLOR_TRANSFORM:
		ITE_RETURN_ERR_IF(count!=1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		context->enColorTransform = bvalue;
		context->updateFlag.enableFlag = VG_TRUE;
		break;
		
	case VG_COLOR_TRANSFORM_VALUES:
		ITE_RETURN_ERR_IF(count!=8, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		for (i=0; i<count; i++)
		{
			context->colorTransform[i] = iteParamToFloat(values, floats, i);  
		}
		context->updateFlag.colorTransformFlag = VG_TRUE;
		break;

	case VG_GLYPH_ORIGIN:
		ITE_RETURN_ERR_IF(count != 2, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		for (i=0; i<count; i++)
		{
			context->glyphOrigin_input[i] = iteParamToFloat(values, floats, i);  
			context->glyphOrigin[i]       = iteParamToValidFloat(values, floats, i);  
		}
		break;

	case VG_MAX_SCISSOR_RECTS:
	case VG_MAX_DASH_COUNT:
	case VG_MAX_KERNEL_SIZE:
	case VG_MAX_SEPARABLE_KERNEL_SIZE:
	case VG_MAX_COLOR_RAMP_STOPS:
	case VG_MAX_IMAGE_WIDTH:
	case VG_MAX_IMAGE_HEIGHT:
	case VG_MAX_IMAGE_PIXELS:
	case VG_MAX_IMAGE_BYTES:
	case VG_MAX_FLOAT:
	case VG_MAX_GAUSSIAN_STD_DEVIATION:
	case VG_SCREEN_LAYOUT:
		/* Read-only */
		break;

	default:
		/* Invalid VGParamType */
		ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
	}

	ITE_RETURN(ITE_NO_RETVAL);
}

/*---------------------------------------------------------
 * Outputs a parameter by interpreting the output value
 * vector according to the parameter type and input type.
 *---------------------------------------------------------*/

static void iteGet(VGContext *context, VGParamType type, ITEint count, void *values, ITEint floats)
{
	int i;

	/* Check for invalid array / count */
	ITE_RETURN_ERR_IF(!values || count<=0, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);

	switch (type)
	{
	case VG_MATRIX_MODE:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->matrixMode, count, values, floats, 0);
		break;

	case VG_FILL_RULE:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->fillRule, count, values, floats, 0);
		break;

	case VG_IMAGE_QUALITY:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->imageQuality, count, values, floats, 0);
		break;

	case VG_RENDERING_QUALITY:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->renderingQuality, count, values, floats, 0);
		break;

	case VG_BLEND_MODE:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->blendMode, count, values, floats, 0);
		break;

	case VG_IMAGE_MODE:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->imageMode, count, values, floats, 0);
		break;

	case VG_STROKE_CAP_STYLE:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->strokeCapStyle, count, values, floats, 0);
		break;

	case VG_STROKE_JOIN_STYLE:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->strokeJoinStyle, count, values, floats, 0);
		break;

	case VG_PIXEL_LAYOUT:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->pixelLayout, count, values, floats, 0);
		break;

	case VG_FILTER_CHANNEL_MASK:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->filterChannelMask, count, values, floats, 0);
		break;

	case VG_FILTER_FORMAT_LINEAR:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->filterFormatLinear, count, values, floats, 0);
		break;

	case VG_FILTER_FORMAT_PREMULTIPLIED:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->filterFormatPremultiplied, count, values, floats, 0);
		break;

	case VG_STROKE_DASH_PHASE_RESET:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->strokeDashPhaseReset, count, values, floats, 0);
		break;

	case VG_MASKING:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->masking, count, values, floats, 0);
		break;

	case VG_SCISSORING:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam((ITEint)context->scissoring, count, values, floats, 0);
		break;

	case VG_STROKE_LINE_WIDTH:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteValidFloatToParam(context->strokeLineWidth, count, values, floats, 0);
		break;

	case VG_STROKE_MITER_LIMIT:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteValidFloatToParam(context->strokeMiterLimit_input, count, values, floats, 0);
		break;

	case VG_STROKE_DASH_PHASE:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteValidFloatToParam(context->strokeDashPhase, count, values, floats, 0);
		break;

	case VG_STROKE_DASH_PATTERN:
		ITE_RETURN_ERR_IF(count > context->strokeDashPattern.size, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		for (i=0; i<context->strokeDashPattern.size; ++i)
		{
			iteValidFloatToParam(context->strokeDashPattern.items[i], count, values, floats, i);
		}
		break;
	
	case VG_TILE_FILL_COLOR:
		ITE_RETURN_ERR_IF(count > 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteFloatToParam(context->tileFillColor_input.r, count, values, floats, 0);
		iteFloatToParam(context->tileFillColor_input.g, count, values, floats, 1);
		iteFloatToParam(context->tileFillColor_input.b, count, values, floats, 2);
		iteFloatToParam(context->tileFillColor_input.a, count, values, floats, 3);
		break;
	
	case VG_CLEAR_COLOR:
		ITE_RETURN_ERR_IF(count > 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL)
		iteFloatToParam(context->clearColor_input.r, count, values, floats, 0);
		iteFloatToParam(context->clearColor_input.g, count, values, floats, 1);
		iteFloatToParam(context->clearColor_input.b, count, values, floats, 2);
		iteFloatToParam(context->clearColor_input.a, count, values, floats, 3);
		break;
	
	case VG_SCISSOR_RECTS:
		ITE_RETURN_ERR_IF(count > context->scissor.size * 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		for (i=0; i<context->scissor.size; ++i) 
		{
			iteIntToParam((ITEint)context->scissor.items[i].x, count, values, floats, i*4+0);
			iteIntToParam((ITEint)context->scissor.items[i].y, count, values, floats, i*4+1);
			iteIntToParam((ITEint)context->scissor.items[i].w, count, values, floats, i*4+2);
			iteIntToParam((ITEint)context->scissor.items[i].h, count, values, floats, i*4+3);
		}
		break;

	case VG_MAX_SCISSOR_RECTS:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_SCISSOR_RECTS, count, values, floats, 0);
		break;

	case VG_MAX_DASH_COUNT:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_DASH_COUNT, count, values, floats, 0);
		break;

	case VG_MAX_COLOR_RAMP_STOPS:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_COLOR_RAMP_STOPS, count, values, floats, 0);
		break;

	case VG_MAX_IMAGE_WIDTH:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_IMAGE_WIDTH, count, values, floats, 0);
		break;

	case VG_MAX_IMAGE_HEIGHT:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_IMAGE_WIDTH, count, values, floats, 0);
		break;

	case VG_MAX_IMAGE_PIXELS:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		// Awin@20110118
		iteIntToParam(ITE_MAX_IMAGE_PIXELS, count, values, floats, 0);
		break;

	case VG_MAX_IMAGE_BYTES:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_IMAGE_BYTES, count, values, floats, 0);
		break;

	case VG_MAX_FLOAT:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteValidFloatToParam(getMaxFloat(), count, values, floats, 0);
		break;

	case VG_MAX_KERNEL_SIZE: 
		// Awin@20110114 /* TODO: depends on convolution implementation */
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_KERNEL_SIZE, count, values, floats, 0);
		break;

	case VG_MAX_SEPARABLE_KERNEL_SIZE: 
		// Awin@20110114 /* TODO: depends on convolution implementation */
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(ITE_MAX_SEPARABLE_KERNEL_SIZE, count, values, floats, 0);
		break;

	case VG_MAX_GAUSSIAN_STD_DEVIATION: 
		// Awin@20110114 /* TODO: depends on gaussian blur implementation */
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteValidFloatToParam(ITE_MAX_GAUSSIAN_STD_DEVIATION, count, values, floats, 0);
		break;

	case VG_COLOR_TRANSFORM:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(context->enColorTransform, count, values, floats, 0);
		break;

	case VG_COLOR_TRANSFORM_VALUES:
		ITE_RETURN_ERR_IF(count != 8, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		for ( i = 0; i < count; i++)
		{
			iteValidFloatToParam(context->colorTransform[i], count, values, floats, i);
		}
		break;

	case VG_GLYPH_ORIGIN:
		ITE_RETURN_ERR_IF(count != 2, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		for ( i = 0; i < count; i++)
		{
			iteFloatToParam(context->glyphOrigin_input[i], count, values, floats, i);
		}
		break;

	case VG_SCREEN_LAYOUT:
		ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		iteIntToParam(VG_PIXEL_LAYOUT_UNKNOWN, count, values, floats, 0);
		break;

	default:
		/* Invalid VGParamType */
		ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
	}

	ITE_RETURN(ITE_NO_RETVAL);
}

/*-----------------------------------------------------------
 * Sets a resource parameter by interpreting the input value
 * vector according to the parameter type and input type.
 *-----------------------------------------------------------*/

static void iteSetParameter(VGContext *context, VGHandle object,
                           ITEResourceType rtype, VGint ptype,
                           ITEint count, const void *values, ITEint floats)
{
	ITEfloat fvalue = 0.0f;
	ITEint ivalue = 0;
	VGboolean bvalue = VG_FALSE;
	int i;

	/* Check for negative count */
	ITE_RETURN_ERR_IF(count<0, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);

	/* Check for empty vector */
	ITE_RETURN_ERR_IF(!values && count!=0, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);

	/* Pre-convert first value for non-vector params */
	if (count == 1)
	{
		fvalue = iteParamToValidFloat(values, floats, 0);
		ivalue = iteParamToInt(values, floats, 0);
		bvalue = (ivalue ? VG_TRUE : VG_FALSE);
	}

	switch (rtype)
	{
	case ITE_RESOURCE_PATH: 
		/* Path parameters */
		switch (ptype)
		{
		case VG_PATH_FORMAT:
		case VG_PATH_DATATYPE:
		case VG_PATH_SCALE:
		case VG_PATH_BIAS:
		case VG_PATH_NUM_SEGMENTS:
		case VG_PATH_NUM_COORDS:
			/* Read-only */ 
			break;

		default:
			/* Invalid VGParamType */
			ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);	  
		}
		break;
		
	case ITE_RESOURCE_PAINT: 
		/* Paint parameters */
		switch (ptype) 
		{ 
		context->updateFlag.paintBaseFlag = VG_TRUE;  
		case VG_PAINT_TYPE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			ITE_RETURN_ERR_IF(!iteIsEnumValid(ptype,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			((ITEPaint*)object)->type = (VGPaintType)ivalue;
			if(ivalue == VG_PAINT_TYPE_LINEAR_GRADIENT)
			{
				context->updateFlag.linearGradientFlag = VG_TRUE;  
			}
			else if( ivalue == VG_PAINT_TYPE_RADIAL_GRADIENT )
			{
				context->updateFlag.radialGradientFlag = VG_TRUE;  
			}
			else if( ivalue == VG_PAINT_TYPE_PATTERN)
			{
				context->updateFlag.paintPatternFlag = VG_TRUE;  
			}
			break;
	  
		case VG_PAINT_COLOR:
			ITE_RETURN_ERR_IF(count != 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			((ITEPaint*)object)->color_input.r = (ITEfloat)iteParamToFloat(values, floats, 0);
			((ITEPaint*)object)->color_input.g = (ITEfloat)iteParamToFloat(values, floats, 1);
			((ITEPaint*)object)->color_input.b = (ITEfloat)iteParamToFloat(values, floats, 2);
			((ITEPaint*)object)->color_input.a = (ITEfloat)iteParamToFloat(values, floats, 3);
			((ITEPaint*)object)->color.r = (ITEuint8)(iteParamToValidFloat(values, floats, 0)*255);
			((ITEPaint*)object)->color.g = (ITEuint8)(iteParamToValidFloat(values, floats, 1)*255);
			((ITEPaint*)object)->color.b = (ITEuint8)(iteParamToValidFloat(values, floats, 2)*255);
			((ITEPaint*)object)->color.a = (ITEuint8)(iteParamToValidFloat(values, floats, 3)*255);
			break;
	  
		case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			ITE_RETURN_ERR_IF(!iteIsEnumValid(ptype,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			context->updateFlag.linearGradientFlag = VG_TRUE;  
			context->updateFlag.radialGradientFlag = VG_TRUE;  
			((ITEPaint*)object)->spreadMode = (VGColorRampSpreadMode)ivalue;
			break;
	  
		case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			//ITE_RETURN_ERR_IF(!iteIsEnumValid(ptype,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			((ITEPaint*)object)->premultiplied = (VGboolean)ivalue;
			break;
	  
		case VG_PAINT_COLOR_RAMP_STOPS:
			{
				int				max; 
				ITEPaint*		paint; 
				ITEStop			stop;
				ITEFloatStop	floatStop;
				
				ITE_RETURN_ERR_IF(count % 5, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
				max  = ITE_MIN(count, ITE_MAX_COLOR_RAMP_STOPS * 5);
				paint = (ITEPaint*)object;
				iteFloatStopArrayClear(&paint->instops_input); 
				iteStopArrayClear(&paint->instops); 
				for (i=0; i<max; i+=5)
				{
					floatStop.offset = iteParamToFloat(values, floats, i+0);
					CSET(floatStop.color,
						iteParamToFloat(values, floats, i+1),
						iteParamToFloat(values, floats, i+2),
						iteParamToFloat(values, floats, i+3),
						iteParamToFloat(values, floats, i+4));
					iteFloatStopArrayPushBackP(&paint->instops_input, &floatStop);
					
					stop.offset = iteParamToFloat(values, floats, i+0);
					CSET(stop.color,
						(ITEuint8)(iteParamToFloat(values, floats, i+1) * 255),
						(ITEuint8)(iteParamToFloat(values, floats, i+2) * 255),
						(ITEuint8)(iteParamToFloat(values, floats, i+3) * 255),
						(ITEuint8)(iteParamToFloat(values, floats, i+4) * 255));
					iteStopArrayPushBackP(&paint->instops, &stop);
				}
				iteValidateInputStops(paint);
				context->updateFlag.linearGradientFlag = VG_TRUE;
				context->updateFlag.radialGradientFlag = VG_TRUE;
				break;
			}
	  
		case VG_PAINT_LINEAR_GRADIENT:
			ITE_RETURN_ERR_IF(count != 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			for (i=0; i<4; ++i)
			{
				((ITEPaint*)object)->linearGradient_input[i] = iteParamToFloat(values, floats, i);
				((ITEPaint*)object)->linearGradient[i]       = iteParamToValidFloat(values, floats, i);
			}
			context->updateFlag.linearGradientFlag = VG_TRUE;  
			break;
	  
		case VG_PAINT_RADIAL_GRADIENT:
			ITE_RETURN_ERR_IF(count != 5, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			context->updateFlag.radialGradientFlag = VG_TRUE;  
			for (i=0; i<5; ++i)
			{
				((ITEPaint*)object)->radialGradient_input[i] = iteParamToFloat(values, floats, i);
				((ITEPaint*)object)->radialGradient[i]       = iteParamToValidFloat(values, floats, i);
			}
			break;
	  
		case VG_PAINT_PATTERN_TILING_MODE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			ITE_RETURN_ERR_IF(!iteIsEnumValid(ptype,ivalue), VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			context->updateFlag.paintPatternFlag = VG_TRUE;  
			((ITEPaint*)object)->tilingMode = (VGTilingMode)ivalue;
			break;
	  
		default:
			/* Invalid VGParamType */
			ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		} 
		break;
		
	case ITE_RESOURCE_IMAGE: 
		/* Image parameters */
		switch (ptype)
		{	  
		case VG_IMAGE_FORMAT:
		case VG_IMAGE_WIDTH:
		case VG_IMAGE_HEIGHT:
		  /* Read-only */
		  break;
		  
		default:
		  /* Invalid VGParamType */
		  ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL); 
		} 
		break;

	default:
		/* Invalid resource handle */
		ITE_ASSERT(rtype!=ITE_RESOURCE_INVALID);
		break;
	}

	ITE_RETURN(ITE_NO_RETVAL);
}

/*---------------------------------------------------------------
 * Outputs a resource parameter by interpreting the output value
 * vector according to the parameter type and input type.
 *---------------------------------------------------------------*/

static void 
iteGetParameter(
	VGContext*      context, 
	VGHandle        object,
	ITEResourceType rtype, 
	VGint           ptype,
	ITEint          count, 
	void*           values, 
	ITEint          floats)
{
	int i;

	/* Check for invalid array / count */
	ITE_RETURN_ERR_IF(!values || count<=0, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);

	switch (rtype)
	{
	case ITE_RESOURCE_PATH: 
		/* Path parameters */
		switch (ptype) 
		{
		case VG_PATH_FORMAT:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPath*)object)->format, count, values, floats, 0);
			break;

		case VG_PATH_DATATYPE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPath*)object)->datatype, count, values, floats, 0);
			break;

		case VG_PATH_SCALE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteValidFloatToParam(((ITEPath*)object)->scale, count, values, floats, 0);
			break;

		case VG_PATH_BIAS:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteValidFloatToParam(((ITEPath*)object)->bias, count, values, floats, 0);
			break;

		case VG_PATH_NUM_SEGMENTS:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPath*)object)->segCount, count, values, floats, 0);
			break;

		case VG_PATH_NUM_COORDS:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPath*)object)->dataCount, count, values, floats, 0);
			break;

		default:
			/* Invalid VGParamType */
			ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		} 
		break;
		
	case ITE_RESOURCE_PAINT: 
		/* Paint parameters */
		switch (ptype)
		{ 
		case VG_PAINT_TYPE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPaint*)object)->type, count, values, floats, 0);
			break;

		case VG_PAINT_COLOR:
			ITE_RETURN_ERR_IF(count > 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteFloatToParam((((ITEPaint*)object)->color_input.r), count, values, floats, 0);
			iteFloatToParam((((ITEPaint*)object)->color_input.g), count, values, floats, 1);
			iteFloatToParam((((ITEPaint*)object)->color_input.b), count, values, floats, 2);
			iteFloatToParam((((ITEPaint*)object)->color_input.a), count, values, floats, 3);
			break;

		case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPaint*)object)->spreadMode, count, values, floats, 0);
			break;

		case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPaint*)object)->premultiplied, count, values, floats, 0);
			break;

		case VG_PAINT_COLOR_RAMP_STOPS:
			{
				int				i; 
				ITEPaint*		paint = (ITEPaint*)object; 
				ITEFloatStop*	stop;
				
				ITE_RETURN_ERR_IF(count > (paint->instops_input.size * 5),
					              VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);

				for (i=0; i<paint->instops_input.size; ++i) 
				{
					stop = &paint->instops_input.items[i];
					iteFloatToParam(stop->offset, count, values, floats, i*5+0);
					iteFloatToParam(stop->color.r, count, values, floats, i*5+1);
					iteFloatToParam(stop->color.g, count, values, floats, i*5+2);
					iteFloatToParam(stop->color.b, count, values, floats, i*5+3);
					iteFloatToParam(stop->color.a, count, values, floats, i*5+4);
				}
				break;
			}

		case VG_PAINT_LINEAR_GRADIENT:
			ITE_RETURN_ERR_IF(count > 4, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			for (i=0; i<4; ++i)
			{
				iteFloatToParam(((ITEPaint*)object)->linearGradient_input[i], count, values, floats, i);
			}
			break;

		case VG_PAINT_RADIAL_GRADIENT:
			ITE_RETURN_ERR_IF(count > 5, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			for (i=0; i<5; ++i)
			{
				iteFloatToParam(((ITEPaint*)object)->radialGradient_input[i], count, values, floats, i);
			}
			break;

		case VG_PAINT_PATTERN_TILING_MODE:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEPaint*)object)->tilingMode, count, values, floats, 0);
			break;

		default:
			/* Invalid VGParamType */
			ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		} 
		break;
	case ITE_RESOURCE_IMAGE: 
		/* Image parameters */
		switch (ptype) 
		{ 
		/* TODO: output image parameters when image implemented */
		case VG_IMAGE_FORMAT:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEImage*)object)->vgformat, count, values, floats, 0);
			break;

		case VG_IMAGE_WIDTH:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEImage*)object)->width, count, values, floats, 0);
			break;

		case VG_IMAGE_HEIGHT:
			ITE_RETURN_ERR_IF(count != 1, VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
			iteIntToParam(((ITEImage*)object)->height, count, values, floats, 0);
			break;

		default:
			/* Invalid VGParamType */
			ITE_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, ITE_NO_RETVAL);
		} 
		break;

	case ITE_RESOURCE_FONT:
		/* Font parameters */
		switch (ptype) 
		{
		case VG_FONT_NUM_GLYPHS:
			iteIntToParam(((ITEFont*)object)->glyphCapacityHint, count, values, floats, 0);
			break;
		}
		break;

	default:
		/* Invalid resource handle */
		ITE_ASSERT(rtype!=ITE_RESOURCE_INVALID);
		break;
	}

	ITE_RETURN(ITE_NO_RETVAL);
}

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

/*--------------------------------------------------
 * Sets a parameter of a single integer value
 *--------------------------------------------------*/

VG_API_CALL void vgSetf (VGParamType type, VGfloat value)
{
  VG_GETCONTEXT(VG_NO_RETVAL);
  
  /* Check if target vector */
  VG_RETURN_ERR_IF(iteIsParamVector(type),
                   VG_ILLEGAL_ARGUMENT_ERROR,
                   VG_NO_RETVAL);
  
  /* Error code will be set by iteSet */
  iteSet(context, type, 1, &value, 1);
  VG_RETURN(VG_NO_RETVAL);
}

/*--------------------------------------------------
 * Sets a parameter of a single float value
 *--------------------------------------------------*/

VG_API_CALL void vgSeti (VGParamType type, VGint value)
{
  VG_GETCONTEXT(VG_NO_RETVAL);
  
  /* Check if target vector */
  VG_RETURN_ERR_IF(iteIsParamVector(type),
                   VG_ILLEGAL_ARGUMENT_ERROR,
                   VG_NO_RETVAL);
  
  /* Error code will be set by iteSet */
  iteSet(context, type, 1, &value, 0);
  VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------
 * Sets a parameter which takes a vector of float values
 *-------------------------------------------------------*/

VG_API_CALL void 
vgSetfv(
	VGParamType    type, 
	VGint          count,
	const VGfloat* values)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	VG_RETURN_ERR_IF((values == NULL) && (count > 0), VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!CheckAlignment(values, sizeof(VGfloat)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code will be set by iteSet */
	iteSet(context, type, count, values, 1);
	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Sets a parameter which takes a vector of integer values
 *---------------------------------------------------------*/

VG_API_CALL void 
vgSetiv(
	VGParamType  type, 
	VGint        count,
	const VGint* values)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	VG_RETURN_ERR_IF((values == NULL) && (count > 0), VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	// Awin@20110114 /* TODO: check input array alignment */
	VG_RETURN_ERR_IF(!CheckAlignment(values, sizeof(VGint)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code wil be set by iteSet */
	iteSet(context, type, count, values, 0);
	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Returns a parameter of a single float value
 *---------------------------------------------------------*/

VG_API_CALL VGfloat vgGetf(VGParamType type)
{
  VGfloat retval = 0.0f;
  VG_GETCONTEXT(retval);
  
  /* Check if target vector */
  VG_RETURN_ERR_IF(iteIsParamVector(type),
                   VG_ILLEGAL_ARGUMENT_ERROR,
                   retval);
  
  /* Error code will be set by iteGet */
  iteGet(context, type, 1, &retval, 1);
  VG_RETURN(retval);
}

/*---------------------------------------------------------
 * Returns a parameter of a single integer value
 *---------------------------------------------------------*/

VG_API_CALL VGint vgGeti(VGParamType type)
{
  VGint retval = 0;
  VG_GETCONTEXT(retval);
  
  /* Check if target vector */
  VG_RETURN_ERR_IF(iteIsParamVector(type),
                   VG_ILLEGAL_ARGUMENT_ERROR,
                   retval);
  
  /* Error code will be set by iteGet */
  iteGet(context, type, 1, &retval, 0);
  VG_RETURN(retval);
}

/*---------------------------------------------------------
 * Outputs a parameter of a float vector value
 *---------------------------------------------------------*/

VG_API_CALL void vgGetfv(VGParamType type, VGint count, VGfloat * values)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR 
		¡V if values is NULL in vgGetfv or vgGetiv 
		¡V if values is not properly aligned in vgGetfv or vgGetiv 
		¡V if count is less than or equal to 0 in vgGetfv or vgGetiv */
	VG_RETURN_ERR_IF(count <= 0 || !values || !CheckAlignment(values, sizeof(VGint)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code will be set by iteGet */
	iteGet(context, type, count, values, 1);
	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Outputs a parameter of an integer vector value
 *---------------------------------------------------------*/

VG_API_CALL void vgGetiv(VGParamType type, VGint count, VGint * values)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR 
		¡V if values is NULL in vgGetfv or vgGetiv 
		¡V if values is not properly aligned in vgGetfv or vgGetiv 
		¡V if count is less than or equal to 0 in vgGetfv or vgGetiv */
	VG_RETURN_ERR_IF(count <= 0 || !values || !CheckAlignment(values, sizeof(VGint)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code will be set by iteGet */
	iteGet(context, type, count, values, 0);
	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Returns the size of the output array required to
 * receive the whole vector of parameter values
 *---------------------------------------------------------*/

VG_API_CALL VGint vgGetVectorSize(VGParamType type)
{
	int retval = 0;
	VG_GETCONTEXT(retval);

	switch(type)
	{
	case VG_MATRIX_MODE:
	case VG_FILL_RULE:
	case VG_IMAGE_QUALITY:
	case VG_RENDERING_QUALITY:
	case VG_BLEND_MODE:
	case VG_IMAGE_MODE:
	case VG_STROKE_CAP_STYLE:
	case VG_STROKE_JOIN_STYLE:
	case VG_PIXEL_LAYOUT:
	case VG_FILTER_CHANNEL_MASK:
	case VG_FILTER_FORMAT_LINEAR:
	case VG_FILTER_FORMAT_PREMULTIPLIED:
	case VG_STROKE_DASH_PHASE_RESET:
	case VG_MASKING:
	case VG_SCISSORING:
	case VG_STROKE_LINE_WIDTH:
	case VG_STROKE_MITER_LIMIT:
	case VG_STROKE_DASH_PHASE:
	case VG_MAX_SCISSOR_RECTS:
	case VG_MAX_DASH_COUNT:
	case VG_MAX_KERNEL_SIZE:
	case VG_MAX_SEPARABLE_KERNEL_SIZE:
	case VG_MAX_COLOR_RAMP_STOPS:
	case VG_MAX_IMAGE_WIDTH:
	case VG_MAX_IMAGE_HEIGHT:
	case VG_MAX_IMAGE_PIXELS:
	case VG_MAX_IMAGE_BYTES:
	case VG_MAX_FLOAT:
	case VG_MAX_GAUSSIAN_STD_DEVIATION:
	case VG_SCREEN_LAYOUT:
	case VG_COLOR_TRANSFORM:
		retval = 1;
		break;

	case VG_GLYPH_ORIGIN:
		retval = 2;
		break;

	case VG_TILE_FILL_COLOR:
	case VG_CLEAR_COLOR:
		retval = 4;
		break;

	case VG_COLOR_TRANSFORM_VALUES:
		retval = 8;
		break;

	case VG_STROKE_DASH_PATTERN:
		retval = context->strokeDashPattern.size;
		break;

	case VG_SCISSOR_RECTS:
		retval = context->scissor.size * 4;
		break;

	default:
		/* Invalid VGParamType */
		VG_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, retval);
	}

	VG_RETURN(retval);
}

/*------------------------------------------------------------
 * Sets a resource parameter which takes a single float value
 *------------------------------------------------------------*/

VG_API_CALL void vgSetParameterf(VGHandle object, VGint paramType, VGfloat value)
{
  ITEResourceType resType;
  VG_GETCONTEXT(VG_NO_RETVAL);
  
  /* Validate object */
  resType = iteGetResourceType(context, object);
  VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
                   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
  
  /* Check if param vector */
  VG_RETURN_ERR_IF(iteIsParamVector(paramType),
                   VG_ILLEGAL_ARGUMENT_ERROR,
                   VG_NO_RETVAL);
  
  /* Error code will be set by iteSetParam() */
  iteSetParameter(context, object, resType, paramType, 1, &value, 1);
  VG_RETURN(VG_NO_RETVAL);
}

/*--------------------------------------------------------------
 * Sets a resource parameter which takes a single integer value
 *--------------------------------------------------------------*/

VG_API_CALL void vgSetParameteri(VGHandle object, VGint paramType, VGint value)
{
  ITEResourceType resType;
  VG_GETCONTEXT(VG_NO_RETVAL);
  
  /* Validate object */
  resType = iteGetResourceType(context, object);
  VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
                   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
  
  /* Check if param vector */
  VG_RETURN_ERR_IF(iteIsParamVector(paramType),
                   VG_ILLEGAL_ARGUMENT_ERROR,
                   VG_NO_RETVAL);
  
  /* Error code will be set by iteSetParam() */
  iteSetParameter(context, object, resType, paramType, 1, &value, 0);
  VG_RETURN(VG_NO_RETVAL);
}

/*----------------------------------------------------------------
 * Sets a resource parameter which takes a vector of float values
 *----------------------------------------------------------------*/

VG_API_CALL void 
vgSetParameterfv(
	VGHandle       object, 
	VGint          paramType,
	VGint          count, 
	const VGfloat* values)
{
	ITEResourceType resType;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Validate object */
	resType = iteGetResourceType(context, object);
	VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
				     VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!CheckAlignment(values, sizeof(VGfloat)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code will be set by iteSetParam() */
	iteSetParameter(context, object, resType, paramType, count, values, 1);
	VG_RETURN(VG_NO_RETVAL);
}

/*------------------------------------------------------------------
 * Sets a resource parameter which takes a vector of integer values
 *------------------------------------------------------------------*/

VG_API_CALL void 
vgSetParameteriv(
	VGHandle     object, 
	VGint        paramType,
	VGint        count, 
	const VGint* values)
{
	ITEResourceType resType;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Validate object */
	resType = iteGetResourceType(context, object);
	VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
				   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	// Awin@20110114 /* TODO: Check for input array alignment */
	/*
	if ( (((VGuint)values) + 3 & ~3) != (VGuint)values )
	{
		VG_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	}
	*/
	VG_RETURN_ERR_IF(!CheckAlignment(values, sizeof(VGint)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code will be set by iteSetParam() */
	iteSetParameter(context, object, resType, paramType, count, values, 0);
	VG_RETURN(VG_NO_RETVAL);
}

/*----------------------------------------------------------
 * Returns a resource parameter of a single float value
 *----------------------------------------------------------*/

VG_API_CALL VGfloat vgGetParameterf(VGHandle object, VGint paramType)
{
  VGfloat retval = 0.0f;
  ITEResourceType resType;
  VG_GETCONTEXT(retval);
  
  /* Validate object */
  resType = iteGetResourceType(context, object);
  VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
                   VG_BAD_HANDLE_ERROR, retval);
  
  /* Check if param vector */
  VG_RETURN_ERR_IF(iteIsParamVector(paramType),
                   VG_ILLEGAL_ARGUMENT_ERROR, retval);
  
  /* Error code will be set by iteGetParameter() */
  iteGetParameter(context, object, resType, paramType, 1, &retval, 1);
  VG_RETURN(retval);
}

/*----------------------------------------------------------
 * Returns a resource parameter of a single integer value
 *----------------------------------------------------------*/

VG_API_CALL VGint vgGetParameteri(VGHandle object, VGint paramType)
{
  VGint retval = 0;
  ITEResourceType resType;
  VG_GETCONTEXT(retval);
  
  /* Validate object */
  resType = iteGetResourceType(context, object);
  VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
                   VG_BAD_HANDLE_ERROR, retval);
  
  /* Check if param vector */
  VG_RETURN_ERR_IF(iteIsParamVector(paramType),
                   VG_ILLEGAL_ARGUMENT_ERROR, retval);
  
  /* Error code will be set by iteGetParameter() */
  iteGetParameter(context, object, resType, paramType, 1, &retval, 0);
  VG_RETURN(retval);
}

/*----------------------------------------------------------
 * Outputs a resource parameter of a float vector value
 *----------------------------------------------------------*/

VG_API_CALL void 
vgGetParameterfv(
	VGHandle	object, 
	VGint		paramType,
	VGint		count, 
	VGfloat*	values)
{
	ITEResourceType resType;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Validate object */
	resType = iteGetResourceType(context, object);
	VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
				   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	// Awin@20110114 /* TODO: check input array alignment */
	/*
	if ( (((VGuint)values) + 3 & ~3) != (VGuint)values )
	{
		VG_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	}
	*/
	VG_RETURN_ERR_IF(!CheckAlignment(values, sizeof(VGfloat)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code will be set by iteGetParameter() */
	iteGetParameter(context, object, resType, paramType, count, values, 1);
	VG_RETURN(VG_NO_RETVAL);
}

/*----------------------------------------------------------
 * Outputs a resource parameter of an integer vector value
 *----------------------------------------------------------*/

VG_API_CALL void 
vgGetParameteriv(
	VGHandle	object, 
	VGint		paramType,
	VGint		count, 
	VGint*		values)
{
	ITEResourceType resType;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Validate object */
	resType = iteGetResourceType(context, object);
	VG_RETURN_ERR_IF(resType == ITE_RESOURCE_INVALID,
				     VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!CheckAlignment(values, sizeof(VGint)),
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Error code will be set by iteGetParameter() */
	iteGetParameter(context, object, resType, paramType, count, values, 0);
	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Returns the size of the output array required to
 * receive the whole vector of resource parameter values
 *---------------------------------------------------------*/
  
VG_API_CALL VGint vgGetParameterVectorSize(VGHandle object, VGint ptype)
{
	int retval = 0;
	ITEResourceType rtype;
	VG_GETCONTEXT(retval);

	/* Validate object */
	rtype = iteGetResourceType(context, object);
	VG_RETURN_ERR_IF(rtype == ITE_RESOURCE_INVALID, 
	                 VG_BAD_HANDLE_ERROR, retval);

	switch (rtype)
	{
	case ITE_RESOURCE_PATH:
		/* Path parameters */  
		switch (ptype) 
		{
		case VG_PATH_FORMAT:
		case VG_PATH_DATATYPE:
		case VG_PATH_SCALE:
		case VG_PATH_BIAS:
		case VG_PATH_NUM_SEGMENTS:
		case VG_PATH_NUM_COORDS:
			retval = 1; 
			break;

		default:
			/* Invalid VGParamType */
			VG_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, retval);
		} 
		break;
		
	case ITE_RESOURCE_PAINT:
		/* Paint parameters */
		switch (ptype) 
		{
		case VG_PAINT_TYPE:						retval = 1; break;
		case VG_PAINT_COLOR:					retval = 4; break;
		case VG_PAINT_COLOR_RAMP_SPREAD_MODE:	retval = 1; break;
		case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:	retval = 1; break;
		case VG_PAINT_LINEAR_GRADIENT:			retval = 4; break;
		case VG_PAINT_RADIAL_GRADIENT:			retval = 5; break;
		case VG_PAINT_PATTERN_TILING_MODE:		retval = 1; break;
		
		case VG_PAINT_COLOR_RAMP_STOPS:
			retval = ((ITEPaint*)object)->instops.size*5;
			break;

		default:
			/* Invalid VGParamType */
			VG_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, retval);
		} 
		break;
		
	case ITE_RESOURCE_IMAGE:
		/* Image parameters */
		switch (ptype) 
		{ 
		case VG_IMAGE_FORMAT:
		case VG_IMAGE_WIDTH:
		case VG_IMAGE_HEIGHT:
			retval = 1;
			break;

		default:
			/* Invalid VGParamType */
			VG_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, retval);
		} 
		break;

	case ITE_RESOURCE_FONT:
		switch (ptype)
		{
		case VG_FONT_NUM_GLYPHS:
			retval = 1;
			break;

		default:
			/* Invalid VGParamType */
			VG_RETURN_ERR(VG_ILLEGAL_ARGUMENT_ERROR, retval);
		}
		break;
		
	default:
		/* Invalid resource handle */
		ITE_ASSERT(rtype!=ITE_RESOURCE_INVALID);
		break;
	}

	VG_RETURN(retval);
}

VG_API_CALL const VGubyte * 
vgGetString(
	VGStringID name)
{
	VG_GETCONTEXT(NULL);

	switch(name) 
	{
	case VG_VENDOR:		VG_RETURN((const VGubyte*)context->vendor);
	case VG_RENDERER:	VG_RETURN((const VGubyte*)context->renderer);
	case VG_VERSION:	VG_RETURN((const VGubyte*)context->version);
	case VG_EXTENSIONS:	VG_RETURN((const VGubyte*)context->extensions);
	default:			VG_RETURN(NULL);
	}
}
