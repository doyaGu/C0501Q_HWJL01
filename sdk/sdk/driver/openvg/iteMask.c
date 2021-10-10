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
#include "iteDefs.h"
#include "iteContext.h"
#include "iteImage.h"
#include "itePath.h"

static int 
iteIsValidMaskOperation(
	VGMaskOperation operation)
{
  return
	(operation == VG_CLEAR_MASK ||
     operation == VG_FILL_MASK ||
     operation == VG_SET_MASK ||
     operation == VG_SET_MASK ||
     operation == VG_UNION_MASK ||
     operation == VG_INTERSECT_MASK ||
     operation == VG_SUBTRACT_MASK);
}

static ITEboolean
iteCheckCopyBoundary(
	ITEImage* dstImage,
	VGint*    pDstCopyX,
	VGint*    pDstCopyY,
	VGint*    pDstCopyWidth,
	VGint*    pDstCopyHeight,
	ITEImage* srcImage,
	VGint*    pSrcCopyX,
	VGint*    pSrcCopyY,
	VGint*    pSrcCopyWidth,
	VGint*    pSrcCopyHeight)
{
	ITERectCoord    dst         = { *pDstCopyX, *pDstCopyY, *pDstCopyX + (ITEint64)*pDstCopyWidth, *pDstCopyY + (ITEint64)*pDstCopyHeight };
	ITERectCoord    src         = { *pSrcCopyX, *pSrcCopyY, *pSrcCopyX + (ITEint64)*pSrcCopyWidth, *pSrcCopyY + (ITEint64)*pSrcCopyHeight };
	ITEVector2Coord offset      = { dst.left - src.left, dst.top - src.top};
	ITERectCoord    dstValid    = {0};
	ITERectCoord    srcValid    = {0};
	ITEVector2Coord offsetValid = {0};
	ITEint          finalWidth  = 0;
	ITEint          finalHeight = 0;

	/* Do nothing if out of boundary */
	if (   dst.left > dstImage->width 
		|| dst.top > dstImage->height
		|| src.left > srcImage->width 
		|| src.top > srcImage->height
		|| dst.right < 0 
		|| dst.down < 0
		|| src.right < 0 
		|| src.down < 0 )
	{
		return ITE_FALSE;
	}

	srcValid.left  = ITE_MAX(0, src.left);
	srcValid.top   = ITE_MAX(0, src.top);
	srcValid.right = ITE_MIN(srcImage->width, src.right);
	srcValid.down  = ITE_MIN(srcImage->height, src.down);

	dstValid.left  = srcValid.left + offset.x;
	dstValid.top   = srcValid.top + offset.y;
	dstValid.right = srcValid.right + offset.x;
	dstValid.down  = srcValid.down + offset.y;

	if ( dstValid.left < 0 )
	{
		ITEint dx = 0 - (ITEint)dstValid.left;
		dstValid.left = 0;
		srcValid.left += dx;
	}
	if ( dstValid.top < 0 )
	{
		ITEint dy = 0 - (ITEint)dstValid.top;
		dstValid.top = 0;
		srcValid.top += dy;
	}
	if ( dstValid.right > dstImage->width )
	{
		ITEint dx = dstImage->width - (ITEint)dstValid.right;
		dstValid.right += dx;
		srcValid.right += dx;
	}
	if (dstValid.down > dstImage->height )
	{
		ITEint dy = dstImage->height - (ITEint)dstValid.down;
		dstValid.down += dy;
		srcValid.down += dy;
	}

	/* Do nothing if out of boundary */
	if (   dstValid.left > dstImage->width 
		|| dstValid.top > dstImage->height
		|| srcValid.left > srcImage->width 
		|| srcValid.top > srcImage->height
		|| dstValid.right < 0 
		|| dstValid.down < 0
		|| srcValid.right < 0 
		|| srcValid.down < 0 )
	{
		return ITE_FALSE;
	}

	finalWidth = (ITEint)ITE_MIN(dstValid.right - dstValid.left, srcValid.right - srcValid.left);
	finalHeight = (ITEint)ITE_MIN(dstValid.down - dstValid.top, srcValid.down - srcValid.top);

	srcValid.right = srcValid.left + finalWidth;
	srcValid.down  = srcValid.top  + finalHeight;
	dstValid.right = dstValid.left + finalWidth;
	dstValid.down  = dstValid.top  + finalHeight;

	*pSrcCopyX = (ITEint)srcValid.left;
	*pSrcCopyY = (ITEint)srcValid.top;
	*pSrcCopyWidth = (ITEint)(srcValid.right - srcValid.left);
	*pSrcCopyHeight = (ITEint)(srcValid.down - srcValid.top);
	
	*pDstCopyX = (ITEint)dstValid.left;
	*pDstCopyY = (ITEint)dstValid.top;
	*pDstCopyWidth = (ITEint)(srcValid.right - srcValid.left);
	*pDstCopyHeight = (ITEint)(srcValid.down - srcValid.top);

	return ITE_TRUE;
}

/*
void 
ITEMask_ctor(
	ITEMask *mask)
{
	memset(mask, 0x00, sizeof(mask));
}

void 
ITEMask_dtor(
	ITEMask *mask)
{
	free(mask);
}
*/

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

VG_API_CALL VGMaskLayer vgCreateMaskLayer(VGint width, VGint height)
{
	ITEImage* mask;
	ITEColor  color;

	VG_GETCONTEXT(VG_INVALID_HANDLE);

	if (   !context->surface
		|| !context->surface->maskImage )
	{
		VG_RETURN(VG_INVALID_HANDLE);
	}

	VG_RETURN_ERR_IF(width <= 0 || height <= 0, 
		VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);
	
	VG_RETURN_ERR_IF(width        > ITE_MAX_IMAGE_WIDTH  || 
		             height       > ITE_MAX_IMAGE_HEIGHT || 
		             width*height > ITE_MAX_IMAGE_BYTES  ||
		             width*height > ITE_MAX_IMAGE_PIXELS, 
		VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);
	
    ITE_NEWOBJ(ITEImage, mask);
	VG_RETURN_ERR_IF(!mask, VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);

	mask->objType  = ITE_VG_OBJ_MASKLAYER;
	mask->width    = width;
	mask->height   = height;
	mask->vgformat = VG_A_8;
	mask->pitch    = width;
	mask->data     = (ITEuint8*)VG_VMemAlloc(width * height);
	if ( !mask->data )
	{
		ITE_DELETEOBJ(ITEImage, mask);
		VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	}

	/* Add to resource list */
	if ( !iteImageArrayPushBack(&context->maskLayers, mask) )
	{
		VG_VMemFree((uint32_t)mask->data);
		ITE_DELETEOBJ(ITEImage, mask);
		VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	}

	CSET(color,255,255,255,255);
	iteSetImage(ITE_FALSE, mask, 0, 0, width, height, color);
	
	/*
	RI_GET_CONTEXT(VG_INVALID_HANDLE);
	RI_IF_ERROR(width <= 0 || height <= 0, VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);
	RI_IF_ERROR(width > RI_MAX_IMAGE_WIDTH || height > RI_MAX_IMAGE_HEIGHT || width*height > RI_MAX_IMAGE_PIXELS ||
				width*height > RI_MAX_IMAGE_BYTES, VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);
    Drawable* curr = context->getCurrentDrawable();
    if(!curr || !curr->getMaskBuffer())
        RI_RETURN(VG_INVALID_HANDLE);   //no current drawing surface

	Surface* layer = NULL;
	try
	{
		layer = RI_NEW(Surface, (Color::formatToDescriptor(VG_A_8), width, height, curr->getNumSamples()));	//throws bad_alloc
		RI_ASSERT(layer);
		context->m_maskLayerManager->addResource(layer, context);	//throws bad_alloc
        layer->clear(Color(1,1,1,1,Color::sRGBA), 0, 0, width, height);
		RI_RETURN((VGMaskLayer)layer);
	}
	catch(std::bad_alloc)
	{
		RI_DELETE(layer);
		context->setError(VG_OUT_OF_MEMORY_ERROR);
		RI_RETURN(VG_INVALID_HANDLE);
	}
	*/
	
	VG_RETURN( (VGMaskLayer)mask );
}

VG_API_CALL void vgDestroyMaskLayer(VGMaskLayer maskLayer)
{
	ITEint index;
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Check if valid resource */
	VG_RETURN_ERR_IF(!maskLayer, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	index = iteImageArrayFind(&context->maskLayers, (ITEImage*)maskLayer);
	VG_RETURN_ERR_IF(index == -1, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* Delete object and remove resource */
	ITE_DELETEOBJ(ITEImage, (ITEImage*)maskLayer);
	iteImageArrayRemoveAt(&context->maskLayers, index);
	
	/*
	RI_GET_CONTEXT(RI_NO_RETVAL);
	RI_IF_ERROR(!context->isValidMaskLayer(maskLayer), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid handle

	context->m_maskLayerManager->removeResource((Surface*)maskLayer);
	RI_RETURN(RI_NO_RETVAL);
	*/
	
	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void 
vgMask(
	VGHandle        mask, 
	VGMaskOperation operation,
	VGint           x, 
	VGint           y, 
	VGint           width, 
	VGint           height)
{
	ITEColor c;
	ITEImage *dstMask, *srcMask;
	ITEImage srcMask2;
	//ITEHardware *h;
	
	VG_GETCONTEXT(VG_NO_RETVAL);
	
	//h = context->hardware;
	dstMask = context->surface->maskImage;
	srcMask = (ITEImage*)mask;

	if ( (srcMask != NULL) &&
		 (srcMask->objType != ITE_VG_OBJ_MASK) &&
		 (srcMask->objType != ITE_VG_OBJ_MASKLAYER) &&
		 (srcMask->objType != ITE_VG_OBJ_IMAGE) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}

	// Todo: VG_BAD_HANDLE_ERROR: Check if operation is not VG_CLEAR_MASK or VG_FILL_MASK, and not 
	//       a valid mask layer or image handle.
	VG_RETURN_ERR_IF(operation != VG_CLEAR_MASK &&
	                 operation != VG_FILL_MASK && 
	                 srcMask == NULL, 
		             VG_BAD_HANDLE_ERROR, VG_NO_RETVAL); // mask not a valid image handle

	// ToDo: VG_IMAGE_IN_USE_ERROR: If mask is a VGImage that is currently a rendering target.
	VG_RETURN_ERR_IF(srcMask == context->surface->colorImage, VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	// ToDo: VG_ILLEGAL_ARGUMENT_ERROR: 
	//       1. If operation is not a valid value from the VGMaskOperation.
	//       2. If width or height is less than or equal to 0.
	//       3. If mask is VGMaskLayer and is not compatible with the current surface mask.
	VG_RETURN_ERR_IF(!iteIsValidMaskOperation(operation), VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL); // 1
	VG_RETURN_ERR_IF(width <= 0 || height <= 0, VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL); // 2
	//VG_RETURN_ERR_IF(srcMask && (srcMask->vgformat != dstMask->vgformat), VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL); // 3

	if( !dstMask)
		return;

	//h->enScissor = HW_FALSE;
	switch(operation)
	{
	case VG_CLEAR_MASK:
		CSET(c,0,0,0,0);
		iteSetImage(ITE_FALSE, dstMask, x, y, width, height, c);
		break;
		
	case VG_FILL_MASK:
		CSET(c,255,255,255,255);
		iteSetImage(ITE_FALSE, dstMask, x, y, width, height, c);
		break;

	case VG_SET_MASK:
		if ( srcMask )
        {
			VGint DstCopyX = x;
			VGint DstCopyY = y;
			VGint DstCopyWidth = width; 
			VGint DstCopyHeight = height;
			VGint SrcCopyX = 0; 
			VGint SrcCopyY = 0; 
			VGint SrcCopyWidth = width;
			VGint SrcCopyHeight = height;

			if ( iteCheckCopyBoundary(
				dstMask,
				&DstCopyX, &DstCopyY, &DstCopyWidth, &DstCopyHeight,
				srcMask,
				&SrcCopyX, &SrcCopyY, &SrcCopyWidth, &SrcCopyHeight) )
			{
				iteSourceCopy(ITE_FALSE, dstMask, DstCopyX, DstCopyY, DstCopyWidth, DstCopyHeight, srcMask, SrcCopyX, SrcCopyY);
			}
        }
		break;
		
	case VG_UNION_MASK:
		if ( srcMask )
        {
			VGint DstCopyX = x;
			VGint DstCopyY = y;
			VGint DstCopyWidth = width; 
			VGint DstCopyHeight = height;
			VGint SrcCopyX = 0; 
			VGint SrcCopyY = 0; 
			VGint SrcCopyWidth = width;
			VGint SrcCopyHeight = height;

			if ( iteCheckCopyBoundary(
				dstMask,
				&DstCopyX, &DstCopyY, &DstCopyWidth, &DstCopyHeight,
				srcMask,
				&SrcCopyX, &SrcCopyY, &SrcCopyWidth, &SrcCopyHeight) )
			{
				iteAlphaBlend(dstMask, DstCopyX, DstCopyY, DstCopyWidth, DstCopyHeight, srcMask, SrcCopyX, SrcCopyY, HW_UNION_MASK);
			}
        }
		break;
		
	case VG_SUBTRACT_MASK:
		if ( srcMask )
        {
			VGint DstCopyX = x;
			VGint DstCopyY = y;
			VGint DstCopyWidth = width; 
			VGint DstCopyHeight = height;
			VGint SrcCopyX = 0; 
			VGint SrcCopyY = 0; 
			VGint SrcCopyWidth = width;
			VGint SrcCopyHeight = height;

			if ( iteCheckCopyBoundary(
				dstMask,
				&DstCopyX, &DstCopyY, &DstCopyWidth, &DstCopyHeight,
				srcMask,
				&SrcCopyX, &SrcCopyY, &SrcCopyWidth, &SrcCopyHeight) )
			{
				iteAlphaBlend(dstMask, DstCopyX, DstCopyY, DstCopyWidth, DstCopyHeight, srcMask, SrcCopyX, SrcCopyY, HW_SUBTRACT_MASK);
			}
        }
		break;
		
	case VG_INTERSECT_MASK:
		if ( srcMask )
        {
			VGint DstCopyX = x;
			VGint DstCopyY = y;
			VGint DstCopyWidth = width; 
			VGint DstCopyHeight = height;
			VGint SrcCopyX = 0; 
			VGint SrcCopyY = 0; 
			VGint SrcCopyWidth = width;
			VGint SrcCopyHeight = height;

			if ( iteCheckCopyBoundary(
				dstMask,
				&DstCopyX, &DstCopyY, &DstCopyWidth, &DstCopyHeight,
				srcMask,
				&SrcCopyX, &SrcCopyY, &SrcCopyWidth, &SrcCopyHeight) )
			{
				iteAlphaBlend(dstMask, DstCopyX, DstCopyY, DstCopyWidth, DstCopyHeight, srcMask, SrcCopyX, SrcCopyY, HW_INTERSECT_MASK);
			}
        }
		break;
	}
	
	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgClear(VGint x, VGint y, VGint width, VGint height)
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	VG_RETURN_ERR_IF(width <= 0 || height <= 0,
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	iteSetImage(ITE_TRUE, context->surface->colorImage, x, y, width, height, context->clearColor);
	
	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void 
vgRenderToMask(
	VGPath          path, 
	VGbitfield      paintModes, 
	VGMaskOperation operation)
{
	ITEPath*  p         = NULL;
	ITEPaint* paint     = NULL;
	ITEImage* maskImage = NULL;
	ITEColor  maskColor = {0};

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if path is not a valid path handle */
	VG_RETURN_ERR_IF(!iteIsValidPath(context, path),
				     VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if paintModes is not a valid bitwise OR of values from the VGPaintMode enumeration
		¡V if operation is not a valid value from the VGMaskOperation enumeration */
	VG_RETURN_ERR_IF((paintModes == 0) ||
	                 paintModes & ~(VG_STROKE_PATH | VG_FILL_PATH),
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!iteIsValidMaskOperation(operation),
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	maskImage = context->surface->maskImage;
	p = (ITEPath*)path;
	paint = (context->fillPaint) ? context->fillPaint : &context->defaultPaint;

	// mask operation
	switch(operation)
	{
	case VG_CLEAR_MASK:
		CSET(maskColor, 0, 0, 0, 0);
		iteSetImage(ITE_FALSE, maskImage, 0, 0, maskImage->width, maskImage->height, maskColor);
		break;

	case VG_FILL_MASK:
		CSET(maskColor, 0x00, 0x00, 0x00, 0xFF);
		iteSetImage(ITE_FALSE, maskImage, 0, 0, maskImage->width, maskImage->height, maskColor);
		/*
		iteSourceCopy(
			ITE_FALSE,
			context->surface->maskImage, 
			0, 
			0, 
			context->surface->colorImage->width, 
			context->surface->colorImage->height, 
			paint->pattern, 
			0, 
			0);
		*/
		break;

	case VG_SET_MASK:
		/*
		context->masking = VG_FALSE;
		iteDrawPath(p, paintModes);
		*/
		if ( paintModes & VG_FILL_PATH )
		{
			CSET(maskColor, 0, 0, 0, 0);
			iteSetImage(ITE_FALSE, maskImage, 0, 0, maskImage->width, maskImage->height, maskColor);
			iteDrawPathToMask(p, VG_FILL_PATH, operation);
		}
		if ( paintModes & VG_STROKE_PATH )
		{
			CSET(maskColor, 0, 0, 0, 0);
			iteSetImage(ITE_FALSE, maskImage, 0, 0, maskImage->width, maskImage->height, maskColor);
			iteDrawPathToMask(p, VG_STROKE_PATH, operation);
		}
		break;

	case VG_UNION_MASK:
		{
			ITEImage* localImage = NULL;

			localImage = iteCreateImage(
				context->surface->maskImage->vgformat,
				context->surface->maskImage->width,
				context->surface->maskImage->height,
				0,
				VG_TRUE,
				VG_TRUE);
			if ( localImage )
			{
				if ( paintModes & VG_FILL_PATH )
				{
					CSET(maskColor, 0, 0, 0, 0);
					iteSetImage(ITE_FALSE, localImage, 0, 0, localImage->width, localImage->height, maskColor);
					iteDrawPathToMask2(p, VG_FILL_PATH, VG_SET_MASK, localImage);
					iteAlphaBlend(maskImage, 0, 0, localImage->width, localImage->height, localImage, 0, 0, HW_UNION_MASK);
				}

				if ( paintModes & VG_STROKE_PATH )
				{
					CSET(maskColor, 0, 0, 0, 0);
					iteSetImage(ITE_FALSE, localImage, 0, 0, localImage->width, localImage->height, maskColor);
					iteDrawPathToMask2(p, VG_STROKE_PATH, VG_SET_MASK, localImage);
					iteAlphaBlend(maskImage, 0, 0, localImage->width, localImage->height, localImage, 0, 0, HW_UNION_MASK);
				}
				iteHardwareWaitObjID(localImage->objectID);
				iteDestroyImage(localImage);
			}
		}
		break;

	case VG_INTERSECT_MASK:
		/*
		context->hardware->maskMode = HW_INTERSECT_RENDERMASK;
		iteDrawPath(p, paintModes);
		*/
		{
			ITEImage* localImage = NULL;

			localImage = iteCreateImage(
				context->surface->maskImage->vgformat,
				context->surface->maskImage->width,
				context->surface->maskImage->height,
				0,
				VG_TRUE,
				VG_TRUE);
			if ( localImage )
			{
				if ( paintModes & VG_FILL_PATH )
				{
					CSET(maskColor, 0, 0, 0, 0);
					iteSetImage(ITE_FALSE, localImage, 0, 0, localImage->width, localImage->height, maskColor);
					iteDrawPathToMask2(p, VG_FILL_PATH, VG_SET_MASK, localImage);
					iteAlphaBlend(maskImage, 0, 0, localImage->width, localImage->height, localImage, 0, 0, HW_INTERSECT_MASK);
				}

				if ( paintModes & VG_STROKE_PATH )
				{
					CSET(maskColor, 0, 0, 0, 0);
					iteSetImage(ITE_FALSE, localImage, 0, 0, localImage->width, localImage->height, maskColor);
					iteDrawPathToMask2(p, VG_STROKE_PATH, VG_SET_MASK, localImage);
					iteAlphaBlend(maskImage, 0, 0, localImage->width, localImage->height, localImage, 0, 0, HW_INTERSECT_MASK);
				}
				iteHardwareWaitObjID(localImage->objectID);
				iteDestroyImage(localImage);
			}
		}
		break;

	case VG_SUBTRACT_MASK:
		/*
		context->hardware->maskMode = HW_SUBTRACT_RENDERMASK;
		iteDrawPath(p, paintModes);
		*/
		//iteDrawPathToMask(p, paintModes, operation);
		{
			ITEImage* localImage = NULL;

			localImage = iteCreateImage(
				context->surface->maskImage->vgformat,
				context->surface->maskImage->width,
				context->surface->maskImage->height,
				0,
				VG_TRUE,
				VG_TRUE);
			if ( localImage )
			{
				if ( paintModes & VG_FILL_PATH )
				{
					CSET(maskColor, 0, 0, 0, 0);
					iteSetImage(ITE_FALSE, localImage, 0, 0, localImage->width, localImage->height, maskColor);
					iteDrawPathToMask2(p, VG_FILL_PATH, VG_SET_MASK, localImage);
					iteAlphaBlend(maskImage, 0, 0, localImage->width, localImage->height, localImage, 0, 0, HW_SUBTRACT_MASK);
				}

				if ( paintModes & VG_STROKE_PATH )
				{
					CSET(maskColor, 0, 0, 0, 0);
					iteSetImage(ITE_FALSE, localImage, 0, 0, localImage->width, localImage->height, maskColor);
					iteDrawPathToMask2(p, VG_STROKE_PATH, VG_SET_MASK, localImage);
					iteAlphaBlend(maskImage, 0, 0, localImage->width, localImage->height, localImage, 0, 0, HW_SUBTRACT_MASK);
				}

				iteHardwareWaitObjID(localImage->objectID);
				iteDestroyImage(localImage);
			}
		}
		break;
	}

	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgFillMaskLayer(VGMaskLayer maskLayer, VGint x, VGint y, VGint width, VGint height, VGfloat value)
{
	ITEImage* mask = (ITEImage*)maskLayer;
	ITEColor  color;
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	VG_RETURN_ERR_IF(!mask || !iteIsValidMaskLayer(context, maskLayer),
		             VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(value < 0.0f || value > 1.0f, 
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(width <= 0 || 
		             height <= 0 || 
		             x < 0 || 
		             y < 0 || 
		             x > mask->width - width || 
		             y > mask->height - height, 
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	CSET(color, (ITEuint8)(255.0f * value), (ITEuint8)(255.0f * value), (ITEuint8)(255.0f * value), (ITEuint8)(255.0f * value));
	iteSetImage(ITE_FALSE, mask, x, y, width, height, color);
	
	/*
	RI_IF_ERROR(!context->isValidMaskLayer(maskLayer), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid handle
    RI_IF_ERROR(value < 0.0f || value > 1.0f, VG_ILLEGAL_ARGUMENT_ERROR, RI_NO_RETVAL);
    Surface* layer = (Surface*)maskLayer;
    RI_IF_ERROR(width <= 0 || height <= 0 || x < 0 || y < 0 || x > layer->getWidth()-width || y > layer->getHeight()-height, VG_ILLEGAL_ARGUMENT_ERROR, RI_NO_RETVAL);
    layer->clear(Color(1,1,1,value,Color::sRGBA), x, y, width, height);
	*/

	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void 
vgCopyMask(
	VGMaskLayer maskLayer, 
	VGint       dx, 
	VGint       dy, 
	VGint       sx, 
	VGint       sy, 
	VGint       width, 
	VGint       height)
{
	ITEImage* srcMask;
	ITEImage* dstMask;
	VGint     DstCopyX;
	VGint     DstCopyY;
	VGint     DstCopyWidth; 
	VGint     DstCopyHeight;
	VGint     SrcCopyX; 
	VGint     SrcCopyY; 
	VGint     SrcCopyWidth;
	VGint     SrcCopyHeight;
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	srcMask = context->surface->maskImage;
	dstMask = (ITEImage*)maskLayer;
	DstCopyX = dx;
	DstCopyY = dy;
	DstCopyWidth = width; 
	DstCopyHeight = height;
	SrcCopyX = sx; 
	SrcCopyY = sy; 
	SrcCopyWidth = width;
	SrcCopyHeight = height;

	VG_RETURN_ERR_IF(!dstMask || !iteIsValidMaskLayer(context, maskLayer),
		             VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(width <= 0 || height <= 0,
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(dstMask->vgformat != srcMask->vgformat,
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	if ( iteCheckCopyBoundary(
		dstMask,
		&DstCopyX, &DstCopyY, &DstCopyWidth, &DstCopyHeight,
		srcMask,
		&SrcCopyX, &SrcCopyY, &SrcCopyWidth, &SrcCopyHeight) )
	{
		iteSourceCopy(ITE_FALSE, dstMask, DstCopyX, DstCopyY, DstCopyWidth, DstCopyHeight, srcMask, SrcCopyX, SrcCopyY);
		//iteSourceCopy(ITE_FALSE, dstMask, dx, dy, width, height, srcMask, sx, sy);
	}

	/*
	RI_IF_ERROR(!context->isValidMaskLayer(maskLayer), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid handle
    Drawable* drawable = context->getCurrentDrawable();
    if(!drawable || !drawable->getMaskBuffer())
    {
        RI_RETURN(RI_NO_RETVAL);	//no EGL surface is current at the moment or context has no mask buffer
    }
    Surface* layer = (Surface*)maskLayer;
    RI_IF_ERROR(width <= 0 || height <= 0 || drawable->getNumSamples() != layer->getNumSamples(), VG_ILLEGAL_ARGUMENT_ERROR, RI_NO_RETVAL);
    try
    {   //copy drawing surface mask to mask layer
        layer->blit(drawable->getMaskBuffer(), sx, sy, dx, dy, width, height);	//throws bad_alloc
    }
	catch(std::bad_alloc)
	{
		context->setError(VG_OUT_OF_MEMORY_ERROR);
	}
	*/

	VG_RETURN(VG_NO_RETVAL);
}
