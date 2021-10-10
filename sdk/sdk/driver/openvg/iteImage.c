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
#include "iteHardware.h"
#include "iteImage.h"
#include "iteVectors.h"
#include "iteGeometry.h"
#include "iteUtility.h"
#include "vgmem.h"

#define _ITEM_T ITEImage*
#define _ARRAY_T ITEImageArray
#define _FUNC_T iteImageArray
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

ITEfloat iteValidInputFloat(VGfloat f);

/*-----------------------------------------------------
 * Returns 1 if the given format is valid according to
 * the OpenVG specification, else 0.
 *-----------------------------------------------------*/

ITEint
iteIsValidImageFormat(
	VGImageFormat format)
{
    ITEint aOrderBit = (1 << 6);
    ITEint rgbOrderBit = (1 << 7);
    ITEint baseFormat = format & 0x1F;
    ITEint unorderedRgba = format & (~(aOrderBit | rgbOrderBit));
    ITEint isRgba = (baseFormat == VG_sRGBX_8888     ||
                     baseFormat == VG_sRGBA_8888     ||
                     baseFormat == VG_sRGBA_8888_PRE ||
                     baseFormat == VG_sRGBA_5551     ||
                     baseFormat == VG_sRGBA_4444     ||
                     baseFormat == VG_lRGBX_8888     ||
                     baseFormat == VG_lRGBA_8888     ||
                     baseFormat == VG_lRGBA_8888_PRE);
    
    ITEint check = isRgba ? unorderedRgba : format;
    return ((check >= VG_sRGBX_8888 && check <= VG_A_4) || (check == 0x83));
}

static ITEboolean
iteIsValidTilingMode(
	VGTilingMode tilingMode)
{
	return (tilingMode == VG_TILE_FILL   ||
		    tilingMode == VG_TILE_PAD    ||
		    tilingMode == VG_TILE_REPEAT ||
		    tilingMode == VG_TILE_REFLECT);
}

ITEboolean
iteImageIsOverlap(
	ITEImage* image1,
	ITEImage* image2)
{
	if ( image1->data != image2->data )
	{
		return ITE_FALSE;
	}

	if ( image1->width  >= 0 &&
		 image1->height >= 0 &&
		 image2->width  >= 0 && 
		 image2->height >= 0 )
	{
		// Get righr x
		int rightX = ITE_MIN((image1->offsetX + image1->width), (image2->offsetX + image2->width));
		// Get left x
		int leftX = ITE_MAX(image1->offsetX, image2->offsetX);
		// Get interset width
		int width = ITE_MAX((rightX - leftX), 0);

		// Get upper y
		int upperY = ITE_MIN((image1->offsetY + image1->height), (image2->offsetY + image2->height));
		// Get lower y
		int lowerY = ITE_MAX(image1->offsetY, image2->offsetY);
		// Get interset height
		int height = ITE_MAX((upperY - lowerY), 0);

		if ( width == 0 || height == 0 )
		{
			return ITE_FALSE;
		}
		else
		{
			return ITE_TRUE;
		}
	}
	else
	{
		return ITE_FALSE;
	}
}

ITEint
ITEImage_AddReference(
	ITEImage* image)
{
	return ++image->referenceCount;
}

ITEint
ITEImage_RemoveReference(
	ITEImage* image)
{
	image->referenceCount--;
	ITE_ASSERT(image->referenceCount >= 0);
	return image->referenceCount;
}

ITEint
ITEImage_AddInUse(
	ITEImage* image)
{
	return ++image->inUseCount;
}

ITEint
ITEImage_RemoveInUse(
	ITEImage* image)
{
	image->inUseCount--;
	ITE_ASSERT(image->inUseCount >= 0);
	return image->inUseCount;
}

ITEint
ITEImage_AddDrawCount(
	ITEImage* image)
{
	return ++image->drawCount;
}

ITEint
ITEImage_RemoveDrawCount(
	ITEImage* image)
{
	image->drawCount--;
	ITE_ASSERT(image->drawCount >= 0);
	return image->drawCount;
}

ITEint
ITEImage_AddDeleteCount(
	ITEImage* image)
{
	return ++image->deleteCount;
}

ITEint
ITEImage_RemoveDeleteCount(
	ITEImage* image)
{
	image->deleteCount--;
	ITE_ASSERT(image->deleteCount >= 0);
	return image->deleteCount;
}

ITEboolean
ITEImage_IsRgbImageFormat(
	VGImageFormat imageFormat)
{
	return (imageFormat != VG_sL_8 &&
		    imageFormat != VG_lL_8 &&
		    imageFormat != VG_A_8  &&
		    imageFormat != VG_BW_1 &&
		    imageFormat != VG_A_1  &&
		    imageFormat != VG_A_4);
}

ITEboolean
ITEImage_IsRgbaImageFormat(
	VGImageFormat imageFormat)
{
	return (imageFormat != VG_sL_8 &&
		    imageFormat != VG_lL_8 &&
		    imageFormat != VG_A_8  &&
		    imageFormat != VG_BW_1 &&
		    imageFormat != VG_A_1  &&
		    imageFormat != VG_A_4  &&
		    imageFormat != VG_sRGB_565 &&
		    imageFormat != VG_sBGR_565);
}

ITEboolean
ITEImage_IsPreMultipliedFormat(
	VGImageFormat imageFormat)
{
	return ((imageFormat & 0x0F) == 2 ||
		    (imageFormat & 0x0F) == 9);
}

ITEboolean
ITEImage_IsSrgbFormat(
	VGImageFormat imageFormat)
{
	return ((imageFormat & 0x07) <= 6) ? ITE_TRUE : ITE_FALSE;
}

ITEboolean
ITEImage_IsLrgbFormat(
	VGImageFormat imageFormat)
{
	ITEuint format = imageFormat & 0x07;

	return ((format > 6) && (format < 11)) ? ITE_TRUE : ITE_FALSE;
}

ITEboolean
ITEImage_IsLess8bitFormat(
	VGImageFormat imageFormat)
{
	ITEuint format = imageFormat & 0x0F;

	return ((format >= 0x0C) && (format <= 0x0E));
}

ITEboolean
ITEImage_IsValidImageChannel(
	VGImageChannel imageChannel)
{
	return (imageChannel == VG_RED) || (imageChannel == VG_GREEN) || (imageChannel == VG_BLUE) || (imageChannel == VG_ALPHA);
}


/*-----------------------------------------------------------
 * Get format bytes
 *-----------------------------------------------------------*/
static ITEuint iteGetFormatLogBits(VGImageFormat vg)
{
	ITEuint logBits = 5;
	switch(vg & 0x1F)
	{
		case 3: /* VG_sRGB_565 */
		case 4: /* VG_sRGBA_5551 */
		case 5: /* VG_sRGBA_4444 */
			logBits = 4;
			break;
		case 6: /* VG_sL_8 */
		case 10: /* VG_lL_8 */
		case 11: /* VG_A_8 */
			logBits = 3;
			break;
		case 12: /* VG_BW_1 */
		case 13: /* VG_A_1 */
			logBits = 0;
			break;
		case 14: /* VG_A_4 */
			logBits = 2;
			break;
		case 15: /* VG_RGBA_16 */
			logBits = 6;
			break;
		default:
			logBits = 5;
			break;
	}
	return logBits;
}

static ITEuint 
iteGetFormatBytes(
	VGImageFormat format)
{
	ITEuint formatByte = 4;
	
	switch(format & 0x1F)
	{
		case 3:  /* VG_sRGB_565 */
		case 4:  /* VG_sRGBA_5551 */
		case 5:  /* VG_sRGBA_4444 */
			formatByte = 2;
			break;
			
		case 6:  /* VG_sL_8 */
		case 10: /* VG_lL_8 */
		case 11: /* VG_A_8 */
		case 12: /* VG_BW_1 */
		case 13: /* VG_A_1 */
		case 14: /* VG_A_4 */
			formatByte = 1;
			break;
			
		case 15: /* VG_RGBA_16 */
			formatByte = 8;
			break;
			
		default:
			formatByte = 4;
			break;
	}
	return formatByte;
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

static ITEboolean
iteGenImage4X(
	ITEImage* image)
{
	ITEImage* image4X      = NULL;
	ITEImage* imageQuarter = NULL;
	VG_GETCONTEXT(ITE_FALSE);

	if (   context->surface->image4X != NULL
		&& context->surface->imageQuarter != NULL 
		&& context->surface->image4XBase == image )
	{
		return ITE_TRUE;
	}

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(image->objectID);

	if ( context->surface->image4X )
	{
		ITE_DELETEOBJ(ITEImage, context->surface->image4X);
		context->surface->image4X = NULL;
	}
	if ( context->surface->imageQuarter )
	{
		ITE_DELETEOBJ(ITEImage, context->surface->imageQuarter);
		context->surface->imageQuarter = NULL;
	}

	image4X = iteCreateImage(image->vgformat, image->width << 1, image->height << 1, 0, ITE_TRUE, ITE_FALSE);
	if ( image4X == NULL )
	{
		return ITE_FALSE;
	}
	image4X->objectID = image->objectID;
	
	imageQuarter = iteCreateImage(image->vgformat, image->width >> 1, image->height >> 1, 0, ITE_TRUE, ITE_FALSE);
	if ( imageQuarter == NULL )
	{
		if ( image4X )
		{
			ITE_DELETEOBJ(ITEImage, image4X);
		}
		return ITE_FALSE;
	}
	imageQuarter->objectID = image->objectID;

	iteStretchSrcCopy(image4X, 0, 0, image4X->width, image4X->height, image, 0, 0, image->width, image->height);
	iteStretchSrcCopy(imageQuarter, 0, 0, imageQuarter->width, imageQuarter->height, image, 0, 0, image->width, image->height);

	context->surface->image4XBase  = image;
	context->surface->image4X      = image4X;
	context->surface->imageQuarter = imageQuarter;

	return ITE_TRUE;
}

#if 0
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
	/* Do nothing if out of boundary */
	if (   *pDstCopyX >= dstImage->width 
		|| *pDstCopyY >= dstImage->height
		|| *pSrcCopyX >= dstImage->width 
		|| *pSrcCopyY >= dstImage->height
		|| (*pDstCopyX + *pDstCopyWidth) < 0 
		|| (*pDstCopyY + *pDstCopyHeight) < 0
		|| (*pSrcCopyX + *pSrcCopyWidth) < 0 
		|| (*pSrcCopyY + *pSrcCopyHeight) < 0 )
	{
		return ITE_FALSE;
	}

	/* Check copy boundary */
	// 1. Check dst
	if ( *pDstCopyX < 0 )
	{
		*pDstCopyWidth = *pDstCopyWidth + *pDstCopyX;
		*pDstCopyX = 0;
	}
	if ( *pDstCopyX + *pDstCopyWidth > dstImage->width )
	{
		*pDstCopyWidth = dstImage->width - *pDstCopyX;
	}
	if ( *pDstCopyY < 0 )
	{
		*pDstCopyHeight = *pDstCopyHeight + *pDstCopyY;
		*pDstCopyY = 0;
	}
	if ( *pDstCopyY + *pDstCopyHeight > dstImage->height )
	{
		*pDstCopyHeight = dstImage->height - *pDstCopyY;
	}

	// 2. Check src
	if ( *pSrcCopyX < 0 )
	{
		*pSrcCopyWidth = *pSrcCopyWidth + *pSrcCopyX;
		*pSrcCopyX = 0;
	}
	if ( *pSrcCopyX + *pSrcCopyWidth > srcImage->width )
	{
		*pSrcCopyWidth = srcImage->width - *pSrcCopyX;
	}
	if ( *pSrcCopyY < 0 )
	{
		*pSrcCopyHeight = *pSrcCopyHeight + *pSrcCopyY;
		*pSrcCopyY = 0;
	}
	if ( *pSrcCopyY + *pSrcCopyHeight > srcImage->height )
	{
		*pSrcCopyHeight = srcImage->height - *pSrcCopyY;
	}

	return ITE_TRUE;
}
#endif

void ITEImage_ctor(ITEImage *i)
{
	i->objType        = ITE_VG_OBJ_IMAGE;
	i->objectID		  = INVALID_OBJECT_ID;
	i->data           = NULL;
	i->offsetX        = 0;
	i->offsetY        = 0;
	i->width          = 0;
	i->height         = 0;
	i->pitch          = 0;
	i->allowedQuality = 0;
	i->parent         = NULL;
	i->inUseCount     = 0;
	i->referenceCount = 0;
	i->drawCount      = 0;
	i->deleteCount    = 0;
}

void ITEImage_dtor(ITEImage *i)
{
	if ( i->parent )
	{
		ITEImage_RemoveInUse(i->parent);
		if ( ITEImage_RemoveReference(i->parent) == 0 )
		{
			ITEImage_dtor(i->parent);
		}

		ITEImage_RemoveInUse(i);
	}
	
	if (i->data != NULL)
	{
		VG_VMemFree((uint32_t)i->data);
	}
}

ITEImage* 
iteCreateImage(
	VGImageFormat format,
	VGint         width, 
	VGint         height,
	VGbitfield    allowedQuality,
	VGboolean     ownsData,
	VGboolean     forceAlign64bit)
{
	ITEImage *i = NULL;
	ITEuint logBits = iteGetFormatLogBits(format);
	VG_GETCONTEXT(VG_INVALID_HANDLE);  

	/* Create new image object */
	ITE_NEWOBJ(ITEImage, i);
	VG_RETURN_ERR_IF(!i, VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	i->width          = width;
	i->height         = height;
	i->vgformat       = format;
	i->allowedQuality = allowedQuality;

	if ( forceAlign64bit == VG_TRUE )
	{
        i->pitch = (((((width << logBits) + 7) >> 3) + 7) >> 3) <<3 ;
	}
	else
	{
		i->pitch = ((width << logBits) + 7) >> 3;
	}

	/* Allocate data memory if needed */
	if ( ownsData == VG_TRUE )
	{
		i->data = (ITEuint8*)VG_VMemAlloc(i->pitch * i->height);
		if (i->data == NULL)
		{
			ITE_DELETEOBJ(ITEImage, i);
			VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
		}
	}

	/* Add reference count */
	ITEImage_AddReference(i);

	/* Initialize data by zeroing-out */
	//memset(i->data, 1, i->pitch * i->height);

	return (ITEImage*)i;
}

void
iteDestroyImage(
	ITEImage* image)
{
	ITE_DELETEOBJ(ITEImage, (ITEImage*)image);
}

void 
iteSetImage(
	ITEboolean	useContext,
	ITEImage*	im, 
	ITEint		x, 
	ITEint		y, 
	ITEint		width, 
	ITEint		height, 
	ITEColor	color)
{
#if 1
	ITEHardwareRegister hwReg = {0};

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(im->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);

	/* Transform to linear colro or not */
	//1 Todo: Transform to linear colro or not

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = 0;

	/* Disable coverage engine */
	hwReg.REG_RCR_BASE = 0;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (ITEImage_IsPreMultipliedFormat(im->vgformat) ? ITE_VG_CMD_DST_PRE_EN : 0) |
		                 (ITEImage_IsRgbaImageFormat(im->vgformat) ? ITE_VG_CMD_DITHER_EN : 0) |
		                 (ITEImage_IsLess8bitFormat(im->vgformat) ? ITE_VG_CMD_DESTINATION_EN : 0) |
		                 (((useContext == ITE_TRUE) && (context->scissoring == ITE_TRUE)) ? ITE_VG_CMD_SCISSOR_EN : 0) |
	                     (ITE_VG_CMD_RENDERMODE_0);

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING | ITE_VG_CMD_FULLRDY_EN | ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (im->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (im->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	hwReg.REG_RMR_BASE = ITE_VG_CMD_PAINTTYPE_COLOR;
	hwReg.REG_RFR_BASE &= 0x0000FF00;
	hwReg.REG_RFR_BASE |= im->vgformat << 8;

	hwReg.REG_RCR00_BASE = (((ITEs8p4)(color.b << 4) << ITE_VG_CMDSHIFT_RAMPCOLOR0B) & ITE_VG_CMDMASK_RAMPCOLOR0B) |
		                   (color.a & ITE_VG_CMDMASK_RAMPCOLOR0A);

	hwReg.REG_RCR01_BASE = (((ITEs8p4)(color.r << 4) << ITE_VG_CMDSHIFT_RAMPCOLOR0R) & ITE_VG_CMDMASK_RAMPCOLOR0R) |
		                   ((ITEs8p4)(color.g << 4) & ITE_VG_CMDMASK_RAMPCOLOR0G);
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)y << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)x & ITE_VG_CMDMASK_DSTX);
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)width & ITE_VG_CMDMASK_DSTWIDTH);
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(im->data) & ITE_VG_CMDMASK_SRCBASE;
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)im->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (im->pitch & ITE_VG_CMDMASK_DSTPITCH);
	// Scissor/Mask Pitch Register
	hwReg.REG_SMPR_BASE = context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH;
	// Scissor Base Register
	hwReg.REG_SCBR_BASE = (ITEuint32)context->scissorImage.data;

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();
	im->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);
#else
	ITEHardware		*h;
	
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

/*	if( h->enScissor )
	{
		h->coverageX = 0;
		h->coverageY = 0;
		h->coverageWidth = context->scissorImage.width;
		h->coverageHeight = context->scissorImage.height;
		h->coverageData = (ITEint16*)context->scissorImage.data;
	} 
*/

	h->surfacePitch = (ITEint16)im->pitch;
	h->surfaceWidth = (ITEint16)im->width;
	h->surfaceHeight = (ITEint16)im->height;
	h->surfaceData = im->data;
	h->surfaceFormat = im->vgformat;
	h->dstX = (ITEs12p3)x<<3;
	h->dstY = (ITEs12p3)y<<3;
	h->dstWidth = (ITEint16)width;
	h->dstHeight = (ITEint16)height;
	h->paintColor = color;
	h->paintType = HW_PAINT_TYPE_COLOR;
	h->enCoverage = HW_FALSE;  
	h->enMask = HW_FALSE;  
	h->enTexture = HW_FALSE; 
	h->enBlend = HW_FALSE;
	h->enColorTransform = HW_FALSE;
	h->enSrcMultiply = HW_FALSE;
	h->enSrcUnMultiply = HW_FALSE;
	h->enDstMultiply = HW_FALSE;

	iteHardwareRender();

	context->updateFlag.surfaceFlag = VG_TRUE;
	context->updateFlag.destinationFlag = VG_TRUE;
	context->updateFlag.paintBaseFlag = VG_TRUE;
	context->updateFlag.enableFlag = VG_TRUE;
#endif
}

void 
iteLookUpSourceCopy(
	ITEboolean useContext,
	ITEImage*  dst, 
	ITEint     dstX, 
	ITEint     dstY, 
	ITEint     dstWidth, 
	ITEint     dstHeight, 
	ITEImage*  src, 
	ITEint     srcX, 
	ITEint     srcY,
	ITEint     lookupMode)
{
	ITEHardwareRegister hwReg = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	HWMatrix3x3         transMat             = {0};
	ITEboolean          enScissor            = ITE_FALSE;
	ITEint              colorFormat = 0xa;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(dst->objectID);
	iteHardwareWaitObjID(src->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 
		0x10000, 0,       (ITEs15p16)((srcX - dstX) << 16), 
		0,       0x10000, (ITEs15p16)((srcY - dstY) << 16), 
		0,       0,       0x10000);

	if ( useContext && context && context->scissoring )
	{
		enScissor = ITE_TRUE;
		if( context->scissorImage.data == NULL )
		{
			context->scissorImage.width = context->surface->colorImage->width;
			context->scissorImage.height = context->surface->colorImage->height;
			context->scissorImage.pitch = GetAlignment(context->surface->colorImage->width, 64) / 8;
			context->scissorImage.vgformat = VG_A_1;
			context->scissorImage.data = (ITEuint8*)VG_VMemAlloc(context->scissorImage.pitch * context->scissorImage.height);
		}
	}

	if ( ITEImage_IsPreMultipliedFormat(src->vgformat) )
	{
		srcPreMultiFormat = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(dst->vgformat) )
	{
		dstPreMultiFormat = ITE_TRUE;
	}

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreMultiForDst = ITE_FALSE;
	}
	else if ( srcPreMultiFormat == ITE_TRUE && dstPreMultiFormat == ITE_FALSE )
	{
		EnPreMultiForTexture = ITE_FALSE;
		EnUnpreMultiForDst   = ITE_TRUE;
	}
	else
	{
		// srcPreMultiFormat == ITE_FALSE && dstPreMultiFormat == ITE_TRUE
		EnPreMultiForTexture = ITE_TRUE;
		EnUnpreMultiForDst   = ITE_FALSE;
	}

	/* Transform to linear colro or not */
	//1 Todo: Transform to linear colro or not

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = 0;

	/* Disable coverage engine */
	hwReg.REG_RCR_BASE = 0;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
	                     (EnUnpreMultiForDst ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     ITE_VG_CMD_DESTINATION_EN |
						 ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
	                     ((enScissor == ITE_TRUE) ? ITE_VG_CMD_SCISSOR_EN : 0) | 
	                     ITE_VG_CMD_RENDERMODE_1 |
	                     ITE_VG_CMD_LOOKUP_EN;


    hwReg.REG_RMR_BASE |= lookupMode;

	if(lookupMode == ITE_VG_CMD_LOOKUPMODE_A)
       colorFormat = 0xb;
	else
	   colorFormat = 0xa;

	/* Render format register */
	hwReg.REG_RFR_BASE &= 0x00FFFFFF;
	hwReg.REG_RFR_BASE |= (dst->vgformat << 8) | colorFormat;
	hwReg.REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | ITE_VG_CMD_DSTEXTEND_EN | ITE_VG_CMD_MASKEXTEND_EN;
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)dstY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)dstX & ITE_VG_CMDMASK_DSTX);
	
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)dstHeight << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)dstWidth & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(dst->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)src->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (dst->pitch & ITE_VG_CMDMASK_DSTPITCH);
	
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)src->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)src->offsetX & ITE_VG_CMDMASK_DSTX);
	
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)src->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)src->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(src->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Scissor/Mask Pitch Register
	hwReg.REG_SMPR_BASE = (enScissor == ITE_TRUE) ? ((context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) & ITE_VG_CMDMASK_SCISPITCH) : 0;
	
	// Scissor Base Register
	hwReg.REG_SCBR_BASE = (enScissor == ITE_TRUE) ? (((ITEuint32)context->scissorImage.data<< ITE_VG_CMDSHIFT_SCISBASE) & ITE_VG_CMDMASK_SCISBASE) : 0;
	
	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();
	src->objectID = hwReg.REG_BID2_BASE;
	dst->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);
}

void 
iteSourceCopy(
	ITEboolean useContext,
	ITEImage*  dst, 
	ITEint     dstX, 
	ITEint     dstY, 
	ITEint     dstWidth, 
	ITEint     dstHeight, 
	ITEImage*  src, 
	ITEint     srcX, 
	ITEint     srcY)
{
#if 1
	ITEHardwareRegister hwReg = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	HWMatrix3x3         transMat             = {0};
	ITEboolean          enScissor            = ITE_FALSE;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(dst->objectID);
	iteHardwareWaitObjID(src->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 
		0x10000, 0,       (ITEs15p16)((srcX - dstX) << 16), 
		0,       0x10000, (ITEs15p16)((srcY - dstY) << 16), 
		0,       0,       0x10000);

	if ( useContext && context && context->scissoring )
	{
		enScissor = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(src->vgformat) )
	{
		srcPreMultiFormat = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(dst->vgformat) )
	{
		dstPreMultiFormat = ITE_TRUE;
	}

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreMultiForDst = ITE_FALSE;
	}
	else if ( srcPreMultiFormat == ITE_TRUE && dstPreMultiFormat == ITE_FALSE )
	{
		EnPreMultiForTexture = ITE_FALSE;
		EnUnpreMultiForDst   = ITE_TRUE;
	}
	else
	{
		// srcPreMultiFormat == ITE_FALSE && dstPreMultiFormat == ITE_TRUE
		EnPreMultiForTexture = ITE_TRUE;
		EnUnpreMultiForDst   = ITE_FALSE;
	}

	/* Transform to linear colro or not */
	//1 Todo: Transform to linear colro or not

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING | ITE_VG_CMD_FULLRDY_EN | ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (dst->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (dst->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
	                     (EnUnpreMultiForDst ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     (ITEImage_IsLess8bitFormat(dst->vgformat) ? ITE_VG_CMD_DESTINATION_EN : 0) |
						 ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
	                     ((enScissor == ITE_TRUE) ? ITE_VG_CMD_SCISSOR_EN : 0) | 
	                     ITE_VG_CMD_RENDERMODE_1;

	hwReg.REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN;

	/* Render format register */
	hwReg.REG_RFR_BASE &= 0x00FFFFFF;
	hwReg.REG_RFR_BASE |= (dst->vgformat << 8) | src->vgformat;
	hwReg.REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | ITE_VG_CMD_DSTEXTEND_EN | ITE_VG_CMD_MASKEXTEND_EN;
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)dstY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)dstX & ITE_VG_CMDMASK_DSTX);
	
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)dstHeight << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)dstWidth & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(dst->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)src->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (dst->pitch & ITE_VG_CMDMASK_DSTPITCH);
	
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)src->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)src->offsetX & ITE_VG_CMDMASK_DSTX);
	
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)src->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)src->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(src->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Scissor/Mask Pitch Register
	hwReg.REG_SMPR_BASE = (enScissor == ITE_TRUE) ? ((context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) & ITE_VG_CMDMASK_SCISPITCH) : 0;
	
	// Scissor Base Register
	hwReg.REG_SCBR_BASE = (enScissor == ITE_TRUE) ? ((ITEuint32)context->scissorImage.data) : 0;
	
	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();
	src->objectID = hwReg.REG_BID2_BASE;
	dst->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);
#else
	ITEHardware		*h;
	ITEColor c;
	
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	
	CSET(c, 0, 0, 0, 0);
	SETMAT(h->imageTransform, 0x10000, 0, (ITEs15p16)(srcX<<16)-(ITEs15p16)(dstX<<16), 0, 0x10000, (ITEs15p16)(srcY<<16)-(ITEs15p16)(dstY<<16), 0, 0, 0x10000);

	/*
	if( h->enScissor )
	{
		h->coverageX = 0;
		h->coverageY = 0;
		h->coverageWidth = context->scissorImage.width;
		h->coverageHeight = context->scissorImage.height;
		h->coverageData = (ITEint16*)context->scissorImage.data;
	}
	*/

	h->enCoverage = HW_FALSE;
	h->enBlend = HW_FALSE;
	h->enMask = HW_FALSE;
	h->enTexture = HW_TRUE;
	h->enColorTransform = HW_FALSE;
	h->enSrcMultiply = HW_FALSE;
	h->enSrcUnMultiply = HW_FALSE;
	h->enDstMultiply = HW_FALSE;

	h->surfaceData = dst->data;
	h->surfaceFormat = dst->vgformat;
	h->surfacePitch = (ITEint16)dst->pitch;
	h->surfaceWidth = (ITEint16)dst->width;
	h->surfaceHeight = (ITEint16)dst->height;

	h->imageQuality = HW_IMAGE_QUALITY_NONANTIALIASED;
	h->textureData = src->data;
	h->textureFormat = src->vgformat;
	h->texturePitch = (ITEint16)src->pitch;
	h->textureWidth = (ITEint16)src->width;
	h->textureHeight = (ITEint16)src->height;
	
	h->dstWidth = (ITEint16)dstWidth;
	h->dstHeight = (ITEint16)dstHeight;
	h->dstX = (ITEint16)dstX<<3;	//s12.3
	h->dstY = (ITEint16)dstY<<3;


	h->paintColor = c;
	h->paintType = HW_PAINT_TYPE_COLOR;
	h->tilingMode = HW_TILE_FILL;
	h->paintMode = HW_FILL_PATH;
	h->blendMode = HW_BLEND_SRC;
	h->maskMode = HW_INTERSECT_RENDERMASK;
	IDMAT(h->fillTransform);

	iteHardwareRender();

	context->updateFlag.paintBaseFlag = HW_TRUE;
	context->updateFlag.destinationFlag = HW_TRUE;
	context->updateFlag.textureFlag = HW_TRUE;
	context->updateFlag.surfaceFlag = HW_TRUE;
	context->updateFlag.enableFlag = HW_TRUE;
	context->updateFlag.imageMatrixFlag = HW_TRUE;
	context->updateFlag.fillMatrixFlag = HW_TRUE;
	context->updateFlag.modeFlag = HW_TRUE;
#endif
}

ITEboolean 
iteMergeCopy(
	ITEImage*  output,
	ITEint     outputX, 
	ITEint     outputY, 
	ITEint     outputWidth, 
	ITEint     outputHeight, 
	ITEImage*  dstImage, 
	ITEint     dstX, 
	ITEint     dstY, 
	ITEint     dstWidth, 
	ITEint     dstHeight, 
	ITEImage*  srcPattern, 
	ITEint     srcX, 
	ITEint     srcY,
	ITEint     srcWidth, 
	ITEint     srcHeight)
{
	ITEHardwareRegister hwReg = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreBeforeCT      = ITE_FALSE; // Reg 0x0AC, Bit 30
	HWMatrix3x3         transMat             = {0};
	ITEboolean          enScissor            = ITE_FALSE;
	ITEboolean          hwEnGamma            = ITE_FALSE;
	ITEuint32           hwGammaMode          = ITE_VG_CMD_GAMMAMODE_INVERSE;

	//VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(dstImage->objectID);
	iteHardwareWaitObjID(srcPattern->objectID);
	iteHardwareWaitObjID(output->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 
		   0x10000, 0,       (ITEs15p16)((srcX - dstX) << 16), 
		   0,       0x10000, (ITEs15p16)((srcY - dstY) << 16), 
		   0,       0,       0x10000);

	if ( ITEImage_IsPreMultipliedFormat(srcPattern->vgformat) )
	{
		srcPreMultiFormat = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(dstImage->vgformat) )
	{
		dstPreMultiFormat = ITE_TRUE;
	}

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreBeforeCT = ITE_FALSE;
	}
	else if ( srcPreMultiFormat == ITE_TRUE && dstPreMultiFormat == ITE_FALSE )
	{
		EnPreMultiForTexture = ITE_FALSE;
		EnUnpreBeforeCT      = ITE_TRUE;
	}
	else
	{
		// srcPreMultiFormat == ITE_FALSE && dstPreMultiFormat == ITE_TRUE
		EnPreMultiForTexture = ITE_TRUE;
		EnUnpreBeforeCT      = ITE_FALSE;
	}

	/* Transform to linear colro or not */
	if (   ITEImage_IsSrgbFormat(dstImage->vgformat)
		&& ITEImage_IsLrgbFormat(srcPattern->vgformat) )
	{
		hwEnGamma = ITE_TRUE;
		/* lRGB --> sRGB */
		hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
	}
	else if (   ITEImage_IsLrgbFormat(dstImage->vgformat)
		     && ITEImage_IsSrgbFormat(srcPattern->vgformat) )
	{
		hwEnGamma = ITE_TRUE;
		/* sRGB --> lRGB */
		hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
	}
	else
	{
		hwEnGamma = ITE_FALSE;
	}

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING | ITE_VG_CMD_FULLRDY_EN | ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (dstImage->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (dstImage->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreBeforeCT ? ITE_VG_CMD_SRC_NONPRE_EN : 0) |
	                     (EnPreMultiForTexture ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     //(hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
	                     ITE_VG_CMD_DESTINATION_EN |
						 ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
						 ITE_VG_CMD_BLDMERGE_EN |
	                     ((enScissor == ITE_TRUE) ? ITE_VG_CMD_SCISSOR_EN : 0) | 
	                     ITE_VG_CMD_RENDERMODE_2;

	hwReg.REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
		                 hwGammaMode |
						 ITE_VG_CMD_PAINTTYPE_PATTERN |
						 ITE_VG_CMD_IMAGEMODE_NORMAL;

	/* Render format register */
	hwReg.REG_RFR_BASE &= 0x00FFFFFF;
	hwReg.REG_RFR_BASE |= (output->vgformat << 8) | srcPattern->vgformat;
	hwReg.REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | ITE_VG_CMD_DSTEXTEND_EN | ITE_VG_CMD_MASKEXTEND_EN;
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)dstY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)dstX & ITE_VG_CMDMASK_DSTX);
	
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)dstHeight << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)dstWidth & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(output->data) & ITE_VG_CMDMASK_SRCBASE;
	
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)srcPattern->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (output->pitch & ITE_VG_CMDMASK_DSTPITCH);
	
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)srcPattern->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)srcPattern->offsetX & ITE_VG_CMDMASK_DSTX);
	
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)srcPattern->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)srcPattern->width & ITE_VG_CMDMASK_DSTWIDTH);
	
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(srcPattern->data) & ITE_VG_CMDMASK_SRCBASE;

	// At RENDERMODE=2 b10
	hwReg.REG_SPR12_BASE = ITE_VG_SRC1EXTEND_EN |
	                       (dstImage->vgformat << ITE_VG_SRC1FORMAT_SHIFT) |
						   dstImage->pitch;
	hwReg.REG_SBR1_BASE = (ITEuint32)dstImage->data;
	hwReg.REG_SBR2_BASE = (dstImage->offsetY << ITE_VG_CMDSHIFT_SRCY1) |
		                  dstImage->offsetX;
	hwReg.REG_TQSR_BASE = (dstImage->height << ITE_VG_CMDSHIFT_SRC1HEIGHT) |
		                  dstImage->width;
	
	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transMat.m[0][0]) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transMat.m[0][1]) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transMat.m[0][2]) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transMat.m[1][0]) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transMat.m[1][1]) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transMat.m[1][2]) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transMat.m[2][0]) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transMat.m[2][1]) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transMat.m[2][2]) & ITE_VG_CMDMASK_USRINV22;

	// Paint Inverse Transform
	hwReg.REG_PITR00_BASE = (1 << 16) & ITE_VG_CMDMASK_PATINV00;
	hwReg.REG_PITR01_BASE = 0;
	hwReg.REG_PITR02_BASE = 0;
	hwReg.REG_PITR10_BASE = 0;
	hwReg.REG_PITR11_BASE = (1 << 16) & ITE_VG_CMDMASK_PATINV11;
	hwReg.REG_PITR12_BASE = 0;

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();

	/* Set object ID to image */
	output->objectID     = hwReg.REG_BID2_BASE;
	dstImage->objectID   = hwReg.REG_BID2_BASE;
	srcPattern->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);

	return ITE_TRUE;
}

void iteAlphaBlend(
	ITEImage*   dst, 
	ITEint      dstX, 
	ITEint      dstY, 
	ITEint      dstWidth, 
	ITEint      dstHeight, 
	ITEImage*   src, 
	ITEint      srcX, 
	ITEint      srcY, 
	HWBlendMode blendMode)
{
#if 1
	ITEHardwareRegister hwReg = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	HWMatrix3x3         transMat;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(dst->objectID);
	iteHardwareWaitObjID(src->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 
		0x10000, 0,       (ITEs15p16)((srcX - dstX) << 16), 
		0,       0x10000, (ITEs15p16)((srcY - dstY) << 16), 
		0,       0,       0x10000);

	if ( ITEImage_IsPreMultipliedFormat(src->vgformat) )
	{
		srcPreMultiFormat = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(dst->vgformat) )
	{
		dstPreMultiFormat = ITE_TRUE;
	}

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreMultiForDst = ITE_FALSE;
	}
	else if ( srcPreMultiFormat == ITE_TRUE && dstPreMultiFormat == ITE_FALSE )
	{
		EnPreMultiForTexture = ITE_FALSE;
		EnUnpreMultiForDst   = ITE_TRUE;
	}
	else
	{
		// srcPreMultiFormat == ITE_FALSE && dstPreMultiFormat == ITE_TRUE
		EnPreMultiForTexture = ITE_TRUE;
		EnUnpreMultiForDst   = ITE_FALSE;
	}

	/* Transform to linear colro or not */
	//1 Todo: Transform to linear colro or not

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING | ITE_VG_CMD_FULLRDY_EN | ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (dst->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (dst->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
	                     (EnUnpreMultiForDst ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     ITE_VG_CMD_BLEND_EN |				// Enable blend
	                     ITE_VG_CMD_DESTINATION_EN |		// Enable destination read
						 ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
	                     ITE_VG_CMD_RENDERMODE_1;

	/* Render format register */
	hwReg.REG_RFR_BASE &= 0x00FFFFFF;
	hwReg.REG_RFR_BASE |= (dst->vgformat << 8) | src->vgformat;
	hwReg.REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | ITE_VG_CMD_DSTEXTEND_EN | ITE_VG_CMD_MASKEXTEND_EN;
	
	// Render Mode Register
	hwReg.REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
		                 ITE_VG_CMD_MASKMODE_INTERSECT |
		                 ((blendMode & 0x1F) << 8);
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)dstY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)dstX & ITE_VG_CMDMASK_DSTX);
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)dstHeight << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                   ((ITEint16)dstWidth & ITE_VG_CMDMASK_DSTWIDTH);
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(dst->data) & ITE_VG_CMDMASK_SRCBASE;
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)src->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                   (dst->pitch & ITE_VG_CMDMASK_DSTPITCH);
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)src->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)src->offsetX & ITE_VG_CMDMASK_DSTX);
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)src->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)src->width & ITE_VG_CMDMASK_DSTWIDTH);
	// Source Base Register
	hwReg.REG_SBR_BASE |= (ITEuint32)(src->data)& ITE_VG_CMDMASK_SRCBASE;
	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();
	dst->objectID = hwReg.REG_BID2_BASE;
	src->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);
#else
	ITEHardware		*h;
	ITEColor c;
	
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	
	CSET(c, 0, 0, 0, 0);
	SETMAT(h->imageTransform, 0x10000, 0, (ITEs15p16)(srcX<<16)-(ITEs15p16)(dstX<<16), 0, 0x10000, (ITEs15p16)(srcY<<16)-(ITEs15p16)(dstY<<16), 0, 0, 0x10000);

	h->enScissor = HW_FALSE;
	h->enCoverage = HW_FALSE;
	h->enBlend = HW_TRUE;
	h->enMask = HW_FALSE;
	h->enTexture = HW_TRUE;
	h->enColorTransform = HW_FALSE;
	h->enSrcMultiply = HW_TRUE;
	h->enSrcUnMultiply = HW_TRUE;
	h->enDstMultiply = HW_TRUE;

	h->surfaceData = dst->data;
	h->surfaceFormat = dst->vgformat;
	h->surfacePitch = (ITEint16)dst->pitch;
	h->surfaceWidth = (ITEint16)dst->width;
	h->surfaceHeight = (ITEint16)dst->height;

	h->imageQuality = HW_IMAGE_QUALITY_NONANTIALIASED;
	h->textureData = src->data;
	h->textureFormat = src->vgformat;
	h->texturePitch = (ITEint16)src->pitch;
	h->textureWidth = (ITEint16)src->width;
	h->textureHeight = (ITEint16)src->height;
	
	h->dstWidth = (ITEint16)dstWidth;
	h->dstHeight = (ITEint16)dstHeight;
	h->dstX = (ITEint16)dstX<<3;
	h->dstY = (ITEint16)dstY<<3;

	h->paintColor = c;
	h->paintType = HW_PAINT_TYPE_COLOR;
	h->tilingMode = HW_TILE_FILL;
	h->paintMode = HW_FILL_PATH;
	h->blendMode = blendMode;
	h->maskMode = HW_INTERSECT_RENDERMASK;
	IDMAT(h->fillTransform);

	iteHardwareRender();

	context->updateFlag.paintBaseFlag = VG_TRUE;
	context->updateFlag.destinationFlag = VG_TRUE;
	context->updateFlag.textureFlag = VG_TRUE;
	context->updateFlag.surfaceFlag = VG_TRUE;
	context->updateFlag.enableFlag = VG_TRUE;
	context->updateFlag.imageMatrixFlag = VG_TRUE;
	context->updateFlag.fillMatrixFlag = VG_TRUE;
	context->updateFlag.modeFlag = VG_TRUE;
#endif
}

void iteStretchSrcCopy(
	ITEImage* dst, 
	ITEint    dstX, 
	ITEint    dstY, 
	ITEint    dstWidth, 
	ITEint    dstHeight, 
	ITEImage* src, 
	ITEint    srcX, 
	ITEint    srcY, 
	ITEint    srcWidth, 
	ITEint    srcHeight)
{
#if 1
	ITEHardwareRegister hwReg = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	HWMatrix3x3         transMat             = {0};
	VGfloat             floatinverseWidth    = (VGfloat)dstWidth/srcWidth;
	VGfloat             floatinverseHeight   = (VGfloat)dstHeight/srcHeight;
	VGuint              inverseWidth         = _mathFloatToS12_15((VGuint*)&floatinverseWidth);
	VGuint              inverseHeight        = _mathFloatToS12_15((VGuint*)&floatinverseHeight);

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(dst->objectID);
	iteHardwareWaitObjID(src->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 
		   inverseWidth, 0,             (ITEs15p16)((srcX - dstX) << 16), 
		   0,            inverseHeight, (ITEs15p16)((srcY - dstY) << 16), 
		   0,            0,             0x10000);

	if ( ITEImage_IsPreMultipliedFormat(src->vgformat) )
	{
		srcPreMultiFormat = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(dst->vgformat) )
	{
		dstPreMultiFormat = ITE_TRUE;
	}

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreMultiForDst = ITE_FALSE;
	}
	else if ( srcPreMultiFormat == ITE_TRUE && dstPreMultiFormat == ITE_FALSE )
	{
		EnPreMultiForTexture = ITE_FALSE;
		EnUnpreMultiForDst   = ITE_TRUE;
	}
	else
	{
		// srcPreMultiFormat == ITE_FALSE && dstPreMultiFormat == ITE_TRUE
		EnPreMultiForTexture = ITE_TRUE;
		EnUnpreMultiForDst   = ITE_FALSE;
	}

	/* Transform to linear colro or not */
	//1 Todo: Transform to linear colro or not

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = 0;

	/* Set clipping range */
	hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING | ITE_VG_CMD_FULLRDY_EN | ITE_VG_CMD_TIMERDY_EN;
	hwReg.REG_PXCR_BASE = (dst->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (dst->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
	                     (EnPreMultiForTexture ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     (ITEImage_IsLess8bitFormat(dst->vgformat) ? ITE_VG_CMD_DESTINATION_EN : 0) |
	                     ITE_VG_CMD_RENDERMODE_0;

	/* Render format register */
	hwReg.REG_RFR_BASE &= 0x00FFFFFF;
	hwReg.REG_RFR_BASE |= (dst->vgformat << 8) | src->vgformat;
	hwReg.REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | ITE_VG_CMD_DSTEXTEND_EN | ITE_VG_CMD_MASKEXTEND_EN;
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)dstY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)dstX & ITE_VG_CMDMASK_DSTX);
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)dstHeight << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)dstWidth & ITE_VG_CMDMASK_DSTWIDTH);
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(dst->data)& ITE_VG_CMDMASK_SRCBASE;
	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)src->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (dst->pitch & ITE_VG_CMDMASK_DSTPITCH);
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)src->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)src->offsetX & ITE_VG_CMDMASK_DSTX);
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)src->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)src->width& ITE_VG_CMDMASK_DSTWIDTH);
	// Source Base Register
	hwReg.REG_SBR_BASE |= (ITEuint32)(src->data)& ITE_VG_CMDMASK_SRCBASE;
	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();
	dst->objectID = hwReg.REG_BID2_BASE;
	src->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);
#else
	ITEHardware		*h;
	ITEColor c;
	
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
	
	CSET(c, 0, 0, 0, 0);
	SETMAT(h->imageTransform, 0x10000, 0, (ITEs15p16)(srcX<<16)-(ITEs15p16)(dstX<<16), 0, 0x10000, (ITEs15p16)(srcY<<16)-(ITEs15p16)(dstY<<16), 0, 0, 0x10000);

	h->enScissor = HW_FALSE;
	h->enCoverage = HW_FALSE;
	h->enBlend = HW_FALSE;
	h->enMask = HW_FALSE;
	h->enTexture = HW_TRUE;
	h->enColorTransform = HW_FALSE;
	h->enSrcMultiply = HW_FALSE;
	h->enSrcUnMultiply = HW_FALSE;
	h->enDstMultiply = HW_FALSE;

	h->surfaceData = dst->data;
	h->surfaceFormat = dst->vgformat;
	h->surfacePitch = (ITEint16)dst->pitch;
	h->surfaceWidth = (ITEint16)dst->width;
	h->surfaceHeight = (ITEint16)dst->height;

	h->imageQuality = HW_IMAGE_QUALITY_NONANTIALIASED;
	h->textureData = src->data;
	h->textureFormat = src->vgformat;
	h->texturePitch = (ITEint16)src->pitch;
	h->textureWidth = (ITEint16)src->width;
	h->textureHeight = (ITEint16)src->height;
	
	h->dstWidth = (ITEint16)dstWidth;
	h->dstHeight = (ITEint16)dstHeight;
	h->dstX = (ITEint16)dstX<<3;
	h->dstY = (ITEint16)dstY<<3;

	h->paintColor = c;
	h->paintType = HW_PAINT_TYPE_COLOR;
	h->tilingMode = HW_TILE_FILL;
	h->paintMode = HW_FILL_PATH;
	h->blendMode = HW_BLEND_SRC;
	h->maskMode = HW_INTERSECT_RENDERMASK;
	IDMAT(h->fillTransform);

	iteHardwareRender();

	context->updateFlag.paintBaseFlag = VG_TRUE;
	context->updateFlag.destinationFlag = VG_TRUE;
	context->updateFlag.textureFlag = VG_TRUE;
	context->updateFlag.surfaceFlag = VG_TRUE;
	context->updateFlag.enableFlag = VG_TRUE;
	context->updateFlag.imageMatrixFlag = VG_TRUE;
	context->updateFlag.fillMatrixFlag = VG_TRUE;
	context->updateFlag.modeFlag = VG_TRUE;
#endif	
}

void 
iteImageCombine(
	ITEImage*		dst, 
	ITEint			dstX, 
	ITEint			dstY, 
	ITEint			dstWidth, 
	ITEint			dstHeight, 
	ITEImage*		src, 
	ITEint			srcX, 
	ITEint			srcY, 
	ITEs7p8			scale, 
	ITEs15p16		bias, 
	VGBlendMode		blendMode, 
	VGTilingMode	tilingMode, 
	ITEboolean		colorclip)
{
	ITEHardwareRegister hwReg                = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	ITEboolean          EnPreMultiForBlend   = ITE_FALSE; // Reg 0x0AC, Bit 29
	ITEboolean          EnUnpreMultiForCT    = ITE_FALSE; // Reg 0x0AC, Bit 30
	HWMatrix3x3         transMat             = {0};
	ITEboolean          enBlend              = ITE_FALSE;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(dst->objectID);
	iteHardwareWaitObjID(src->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);
	
	SETMAT(transMat, 0x10000, 0, (ITEs15p16)(srcX<<16)-(ITEs15p16)(dstX<<16), 0, 0x10000, (ITEs15p16)(srcY<<16)-(ITEs15p16)(dstY<<16), 0, 0, 0x10000);

	if( blendMode == VG_BLEND_SRC)
	{
		enBlend = HW_FALSE;
	}
	else
	{
		enBlend = HW_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(src->vgformat) )
	{
		srcPreMultiFormat = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(dst->vgformat) )
	{
		dstPreMultiFormat = ITE_TRUE;
	}

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreMultiForDst = ITE_FALSE;
	}
	else if ( srcPreMultiFormat == ITE_TRUE && dstPreMultiFormat == ITE_FALSE )
	{
        EnPreMultiForTexture = ITE_FALSE;
		EnUnpreMultiForDst = ITE_TRUE;
	}
	else
	{
		// srcPreMultiFormat == ITE_FALSE && dstPreMultiFormat == ITE_TRUE
		EnPreMultiForTexture = ITE_TRUE;
		EnUnpreMultiForDst = ITE_FALSE;		
	}

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN;

	/* Set clipping range */
	//hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING;
	hwReg.REG_PXCR_BASE = (dst->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (dst->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
	                     (EnPreMultiForTexture ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     ITE_VG_CMD_DESTINATION_EN |
    	                 (enBlend ? ITE_VG_CMD_BLEND_EN : 0)|
    	                 ITE_VG_CMD_COLORXFM_EN |
    	                 (colorclip ? ITE_VG_CMD_COLORCLIP_EN : 0)|
	                     ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
	                     ITE_VG_CMD_RENDERMODE_0;
	
     // Render Mode Register
	hwReg.REG_RMR_BASE = (0x4 << ITE_VG_CMDSHIFT_COLLSBSHIFT) |
	                     (0x4 << ITE_VG_CMDSHIFT_TEXLSBSHIFT) |
	                     (((ITEuint32)blendMode & 0x1F) << ITE_VG_CMDSHIFT_BLENDMODE) |
	                     (((ITEuint32)tilingMode & 0x03) << ITE_VG_CMDSHIFT_TILEMODE) |
	                     ITE_VG_CMD_PAINTTYPE_PATTERN |
	                     ITE_VG_CMD_MASKMODE_INTERSECT; 

	hwReg.REG_RFR_BASE &= ~0xFFFF;
	hwReg.REG_RFR_BASE |= (dst->vgformat << 8) | src->vgformat;
	hwReg.REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | ITE_VG_CMD_DSTEXTEND_EN | ITE_VG_CMD_MASKEXTEND_EN;

	// Color Transform Register
/*	hwReg.REG_CTR0_BASE = (((ITEuint32)scale << ITE_VG_CMDSHIFT_COLXFM01) & ITE_VG_CMDMASK_COLXFM01) |
	                      (bias & ITE_VG_CMDMASK_COLXFM00);
	hwReg.REG_CTR1_BASE = (((ITEuint32)scale << ITE_VG_CMDSHIFT_COLXFM11) & ITE_VG_CMDMASK_COLXFM11) |
	                      (bias & ITE_VG_CMDMASK_COLXFM10);
	hwReg.REG_CTR2_BASE = (((ITEuint32)scale << ITE_VG_CMDSHIFT_COLXFM21) & ITE_VG_CMDMASK_COLXFM21) |
	                      (bias & ITE_VG_CMDMASK_COLXFM20);

    if (blendMode == VG_BLEND_SRC) 
	{
		hwReg.REG_CTR3_BASE = (((ITEuint32)0x100 << ITE_VG_CMDSHIFT_COLXFM31) & ITE_VG_CMDMASK_COLXFM31) |
		                      (0 & ITE_VG_CMDMASK_COLXFM30);
    }
	else
	{
		hwReg.REG_CTR3_BASE = (((ITEuint32)0 << ITE_VG_CMDSHIFT_COLXFM31) & ITE_VG_CMDMASK_COLXFM31) |
                      (0 & ITE_VG_CMDMASK_COLXFM30);
	}*/

	hwReg.REG_CTBR0_BASE = bias;
	hwReg.REG_CTBR1_BASE = bias;
	hwReg.REG_CTBR2_BASE = bias;

	hwReg.REG_CTSR0_BASE = (((ITEuint32)scale << ITE_VG_CMDSHIFT_SCOLXFM00) & ITE_VG_CMDMASK_SCOLXFM00) |
	                       (((ITEuint32)scale << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10);

    if (blendMode == VG_BLEND_SRC && colorclip) 
	{

		hwReg.REG_CTSR1_BASE = (((ITEuint32)scale << ITE_VG_CMDSHIFT_SCOLXFM20) & ITE_VG_CMDMASK_SCOLXFM20) |
		                       (((ITEuint32)0 << ITE_VG_CMDSHIFT_SCOLXFM30) & ITE_VG_CMDMASK_SCOLXFM30);
		hwReg.REG_CTBR3_BASE = 0xff00;
    }
	else
	{
		hwReg.REG_CTSR1_BASE = (((ITEuint32)scale << ITE_VG_CMDSHIFT_SCOLXFM20) & ITE_VG_CMDMASK_SCOLXFM20) |
		                       (((ITEuint32)0 << ITE_VG_CMDSHIFT_SCOLXFM30) & ITE_VG_CMDMASK_SCOLXFM30);
	    hwReg.REG_CTBR3_BASE = 0;
	}
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)dstY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)dstX & ITE_VG_CMDMASK_DSTX);
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)dstHeight << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)dstWidth & ITE_VG_CMDMASK_DSTWIDTH);
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(dst->data)& ITE_VG_CMDMASK_SRCBASE;

	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)src->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (dst->pitch & ITE_VG_CMDMASK_DSTPITCH);
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)src->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)src->offsetX & ITE_VG_CMDMASK_DSTX);
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)src->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)src->width& ITE_VG_CMDMASK_DSTWIDTH);
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(src->data)& ITE_VG_CMDMASK_SRCBASE;

	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	hwReg.REG_PITR00_BASE = (ITEs15p16)(1 * 0x10000);
	hwReg.REG_PITR01_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR02_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR10_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR11_BASE = (ITEs15p16)(1 * 0x10000);
	hwReg.REG_PITR12_BASE = (ITEs15p16)(0 * 0x10000);

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();
	dst->objectID = hwReg.REG_BID2_BASE;
	src->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);
	
}

void 
iteImageColorMatrix(
	ITEImage*		dst, 
	ITEint			dstX, 
	ITEint			dstY, 
	ITEint			dstWidth, 
	ITEint			dstHeight, 
	ITEImage*		src, 
	ITEint			srcX, 
	ITEint			srcY, 
	ITEint*		    matrix1, 
	ITEint*		    matrix2, 
	VGBlendMode		blendMode, 
	ITEboolean		colorclip,
	ITEint			lookupMode)
{
	ITEHardwareRegister hwReg                = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	ITEboolean          EnPreMultiForBlend   = ITE_FALSE; // Reg 0x0AC, Bit 29
	ITEboolean          EnUnpreMultiForCT    = ITE_FALSE; // Reg 0x0AC, Bit 30
	HWMatrix3x3         transMat             = {0};
	ITEboolean          enBlend              = ITE_FALSE;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(dst->objectID);
	iteHardwareWaitObjID(src->objectID);

	ITE_INITOBJ(ITEHardware, hwReg);
	
	SETMAT(transMat, 0x10000, 0, (ITEs15p16)(srcX<<16)-(ITEs15p16)(dstX<<16), 0, 0x10000, (ITEs15p16)(srcY<<16)-(ITEs15p16)(dstY<<16), 0, 0, 0x10000);

	if( blendMode == VG_BLEND_SRC)
	{
		enBlend = HW_FALSE;
	}
	else
	{
		enBlend = HW_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(src->vgformat) )
	{
		srcPreMultiFormat = ITE_TRUE;
	}

	if ( ITEImage_IsPreMultipliedFormat(dst->vgformat) )
	{
		dstPreMultiFormat = ITE_TRUE;
	}

	if ( srcPreMultiFormat == dstPreMultiFormat )
	{
		EnPreMultiForTexture = EnUnpreMultiForDst = ITE_FALSE;
	}
	else if ( srcPreMultiFormat == ITE_TRUE && dstPreMultiFormat == ITE_FALSE )
	{
        EnPreMultiForTexture = ITE_FALSE;
		EnUnpreMultiForDst = ITE_TRUE;
	}
	else
	{
		// srcPreMultiFormat == ITE_FALSE && dstPreMultiFormat == ITE_TRUE
		EnPreMultiForTexture = ITE_TRUE;
		EnUnpreMultiForDst = ITE_FALSE;		
	}

	/* Disable tesslellation */
	hwReg.REG_TCR_BASE = ITE_VG_CMD_TRANSFORM_EN;

	/* Set clipping range */
	//hwReg.REG_CCR_BASE = ITE_VG_CMD_CLIPPING;
	hwReg.REG_PXCR_BASE = (dst->width - 1) << ITE_VG_CMDSHIFT_PXCR_CLIPXEND;
	hwReg.REG_PYCR_BASE = (dst->height - 1) << ITE_VG_CMDSHIFT_PYCR_CLIPXEND;

	/* Setup render engine */
	// Render Control Register
	hwReg.REG_RCR_BASE = (EnUnpreMultiForDst ? ITE_VG_CMD_DST_PRE_EN : 0) |
	                     (EnPreMultiForTexture ? ITE_VG_CMD_TEX_PRE_EN : 0) |
	                     ITE_VG_CMD_DESTINATION_EN |
    	                 (enBlend ? ITE_VG_CMD_BLEND_EN : 0)|
    	                 ITE_VG_CMD_COLORXFM_EN |
    	                 (colorclip ? ITE_VG_CMD_COLORCLIP_EN : 0)|
	                     ITE_VG_CMD_TEXCACHE_EN |
						 ITE_VG_CMD_TEXTURE_EN |
	                     ITE_VG_CMD_RENDERMODE_0 |
						 ITE_VG_CMD_COLCOMBIN_EN;
	
     // Render Mode Register
	hwReg.REG_RMR_BASE = (0x4 << ITE_VG_CMDSHIFT_COLLSBSHIFT) |
	                     (0x4 << ITE_VG_CMDSHIFT_TEXLSBSHIFT) |
	                     (((ITEuint32)blendMode & 0x1F) << ITE_VG_CMDSHIFT_BLENDMODE) |
	                     ITE_VG_CMD_PAINTTYPE_PATTERN |
	                     ITE_VG_CMD_MASKMODE_INTERSECT |
						 lookupMode; 

	hwReg.REG_RFR_BASE &= ~0xFFFF;
	hwReg.REG_RFR_BASE |= (dst->vgformat << 8) | src->vgformat;
	hwReg.REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | ITE_VG_CMD_DSTEXTEND_EN | ITE_VG_CMD_MASKEXTEND_EN;

	// Color Transform Register
	if(matrix2)
	{
		hwReg.REG_CTBR0_BASE = matrix2[0];
		hwReg.REG_CTBR1_BASE = matrix2[1];
		hwReg.REG_CTBR2_BASE = matrix2[2];
		hwReg.REG_CTBR3_BASE = matrix2[3];
	}
	else
	{
		hwReg.REG_CTBR0_BASE = 0;
		hwReg.REG_CTBR1_BASE = 0;
		hwReg.REG_CTBR2_BASE = 0;
		hwReg.REG_CTBR3_BASE = 0;
	}
	
	hwReg.REG_CTSR0_BASE = (((ITEuint32)matrix1[0] << ITE_VG_CMDSHIFT_SCOLXFM00) & ITE_VG_CMDMASK_SCOLXFM00) |
	                       (((ITEuint32)matrix1[1] << ITE_VG_CMDSHIFT_SCOLXFM10) & ITE_VG_CMDMASK_SCOLXFM10);


	hwReg.REG_CTSR1_BASE = (((ITEuint32)matrix1[2] << ITE_VG_CMDSHIFT_SCOLXFM20) & ITE_VG_CMDMASK_SCOLXFM20) |
	                       (((ITEuint32)matrix1[3] << ITE_VG_CMDSHIFT_SCOLXFM30) & ITE_VG_CMDMASK_SCOLXFM30);	
	
	// Destination Coordinate Register
	hwReg.REG_DCR_BASE = (((ITEs12p3)dstY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)dstX & ITE_VG_CMDMASK_DSTX);
	// Destination Height/Width Register
	hwReg.REG_DHWR_BASE = (((ITEint16)dstHeight << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)dstWidth & ITE_VG_CMDMASK_DSTWIDTH);
	// Destination Base Register
	hwReg.REG_DBR_BASE = (ITEuint32)(dst->data)& ITE_VG_CMDMASK_SRCBASE;

	// Src/Dst Pitch Register
	hwReg.REG_SDPR_BASE = (((ITEint16)src->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) & ITE_VG_CMDMASK_SRCPITCH0) |
		                  (dst->pitch & ITE_VG_CMDMASK_DSTPITCH);
	// Source Coordinate Register
	hwReg.REG_SCR_BASE = (((ITEs12p3)src->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		                 ((ITEs12p3)src->offsetX & ITE_VG_CMDMASK_DSTX);
	// Source Height/Width Register
	hwReg.REG_SHWR_BASE = (((ITEint16)src->height << ITE_VG_CMDSHIFT_DSTHEIGHT) & ITE_VG_CMDMASK_DSTHEIGHT) |
		                  ((ITEint16)src->width& ITE_VG_CMDMASK_DSTWIDTH);
	// Source Base Register
	hwReg.REG_SBR_BASE = (ITEuint32)(src->data)& ITE_VG_CMDMASK_SRCBASE;

	// User Inverse Transform
	hwReg.REG_UITR00_BASE = (transMat.m[0][0] << ITE_VG_CMDSHIFT_USRINV00) & ITE_VG_CMDMASK_USRINV00;
	hwReg.REG_UITR01_BASE = (transMat.m[0][1] << ITE_VG_CMDSHIFT_USRINV01) & ITE_VG_CMDMASK_USRINV01;
	hwReg.REG_UITR02_BASE = (transMat.m[0][2] << ITE_VG_CMDSHIFT_USRINV02) & ITE_VG_CMDMASK_USRINV02;
	hwReg.REG_UITR10_BASE = (transMat.m[1][0] << ITE_VG_CMDSHIFT_USRINV10) & ITE_VG_CMDMASK_USRINV10;
	hwReg.REG_UITR11_BASE = (transMat.m[1][1] << ITE_VG_CMDSHIFT_USRINV11) & ITE_VG_CMDMASK_USRINV11;
	hwReg.REG_UITR12_BASE = (transMat.m[1][2] << ITE_VG_CMDSHIFT_USRINV12) & ITE_VG_CMDMASK_USRINV12;
	hwReg.REG_UITR20_BASE = (transMat.m[2][0] << ITE_VG_CMDSHIFT_USRINV20) & ITE_VG_CMDMASK_USRINV20;
	hwReg.REG_UITR21_BASE = (transMat.m[2][1] << ITE_VG_CMDSHIFT_USRINV21) & ITE_VG_CMDMASK_USRINV21;
	hwReg.REG_UITR22_BASE = (transMat.m[2][2] << ITE_VG_CMDSHIFT_USRINV22) & ITE_VG_CMDMASK_USRINV22;

	hwReg.REG_PITR00_BASE = (ITEs15p16)(1 * 0x10000);
	hwReg.REG_PITR01_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR02_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR10_BASE = (ITEs15p16)(0 * 0x10000);
	hwReg.REG_PITR11_BASE = (ITEs15p16)(1 * 0x10000);
	hwReg.REG_PITR12_BASE = (ITEs15p16)(0 * 0x10000);

	/* Set image object ID */
	hwReg.REG_BID2_BASE = iteHardwareGenObjectID();
	dst->objectID = hwReg.REG_BID2_BASE;
	src->objectID = hwReg.REG_BID2_BASE;
	
	/* Set HW to cmdq */
	iteHardwareFire(&hwReg);
	
}


static ITEboolean
iteMergeImagePattern(
	ITEImage* pattern,
	ITEImage* image)
{
	ITEImage* merge = NULL;
	VG_GETCONTEXT(ITE_FALSE);
	
	if (   context->surface->ImagePatternMerge != NULL
		&& context->surface->ImagePatternMergeBase == image )
	{
		return ITE_TRUE;
	}

	if ( context->surface->ImagePatternMerge )
	{
		/* Prevent to modify not processed data */
		iteHardwareWaitObjID(context->surface->ImagePatternMerge->objectID);
		ITE_DELETEOBJ(ITEImage, context->surface->ImagePatternMerge);
		context->surface->ImagePatternMerge = NULL;
	}

	merge = iteCreateImage(HW_DEF_RGBA_FFFF, image->width, image->height, 0, ITE_TRUE, ITE_FALSE);
	if ( merge == NULL )
	{
		return ITE_FALSE;
	}

	context->surface->ImagePatternMergeBase = image;
	context->surface->ImagePatternMerge     = merge;

	iteMergeCopy(
		merge, 0, 0, merge->width, merge->height, 
		image, 0, 0, image->width, image->height, 
		pattern, 0, 0, pattern->width, pattern->height);

	return ITE_TRUE;
}

void
iteDrawImage(
	ITEImage*     pImage,
	ITEMatrix3x3* userToSurfaceMatrix)
{
	ITEfloat             coords[5]             = {0};
	ITEuint8             segments[]            = { VG_MOVE_TO_ABS, VG_HLINE_TO_REL, VG_VLINE_TO_REL, VG_HLINE_TO_REL, VG_CLOSE_PATH };
	ITEPath	             p                     = {0};
	ITEHardwareRegister* hw                    = NULL;
	ITEPaint*            paint                 = NULL;
	ITEMatrix3x3         imageMatrix           = {0};
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
	//ITEuint32            hwRenderMode          = ITE_VG_CMD_RENDERMODE_1;
	ITEuint32            hwEnTexPatIn          = 0;
	ITEuint32            hwPaintType           = ITE_VG_CMD_PAINTTYPE_COLOR;
	ITEboolean           hwEnGamma             = ITE_FALSE;
	ITEuint32            hwGammaMode           = ITE_VG_CMD_GAMMAMODE_INVERSE;
    ITEuint8*            tessellateCmdBuffer   = NULL;
	ITEboolean           hwWaitObjID           = ITE_FALSE;
	ITEImage*            pSrcImage             = pImage;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Prevent to modify not processed data */
	iteHardwareWaitObjID(pImage->objectID);

	hw = &context->hardware;
	imageMatrix = *userToSurfaceMatrix;

	coords[0] = 0.0f;
	coords[1] = 0.0f;
	coords[2] = (ITEfloat)(pSrcImage->width);
	coords[3] = (ITEfloat)(pSrcImage->height);
	coords[4] = (ITEfloat)(-pSrcImage->width);

	p.format = VG_PATH_FORMAT_STANDARD;
	p.scale = 1.0f;
	p.bias = 0.0f;
	p.segHint = 0;
	p.dataHint = 0;
	p.datatype = VG_PATH_DATATYPE_F;
	p.caps = VG_PATH_CAPABILITY_ALL;
	p.segCount = 5;
	p.segs = segments;
	p.dataCount = 5;
	p.data = coords;

	iteFlattenPath(&p, 1, &p.pathCommand);
	
	paint = (context->fillPaint) ? context->fillPaint : &context->defaultPaint;

	if (   imageMatrix.m[2][0] 
		|| imageMatrix.m[2][1] 
		|| imageMatrix.m[2][2] != 1.0f )
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
	case VG_IMAGE_QUALITY_NONANTIALIASED: 
		hwImageQuality = ITE_VG_CMD_IMAGEQ_NONAA;
		break;
		
	case VG_IMAGE_QUALITY_FASTER:
		hwImageQuality = ITE_VG_CMD_IMAGEQ_FASTER;
		break;
		
	case VG_IMAGE_QUALITY_BETTER:
		{
			ITEboolean genResult = ITE_FALSE;
			
			hwImageQuality = ITE_VG_CMD_IMAGEQ_BETTER;
			genResult = iteGenImage4X(pSrcImage);
			ITE_ASSERT(genResult);
		}
		break;
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

	/* Get render mode */
	hwPaintType = (paint->type & 0x03) << 2;
	if (   paint->type == VG_PAINT_TYPE_PATTERN
		&& paint->pattern == NULL )
	{
		hwPaintType = ITE_VG_CMD_PAINTTYPE_COLOR;
	}

	if (   (hwPaintType == ITE_VG_CMD_PAINTTYPE_PATTERN)
		&& (context->imageMode == VG_DRAW_IMAGE_MULTIPLY || context->imageMode == VG_DRAW_IMAGE_STENCIL) )
	{
		ITEboolean genResult = ITE_FALSE;
		
		genResult = iteMergeImagePattern(paint->pattern, pSrcImage);
		ITE_ASSERT(genResult);
		pSrcImage = context->surface->ImagePatternMerge;
		hwEnTexPatIn = ITE_VG_CMD_TEXPATIN_EN;
	}

	/* Get pre/unpre parameter */
	// 0x0B0[28]
	if ( ITEImage_IsPreMultipliedFormat(pImage->vgformat) )
	{
		hwEnPreMulTexImage = ITE_FALSE;
	}
	else
	{
		hwEnPreMulTexImage = ITE_TRUE;
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
	if (   ITEImage_IsSrgbFormat(pImage->vgformat) 
		&& ITEImage_IsLrgbFormat(context->surface->colorImage->vgformat) )
	{
		/* sRGB --> lRGB */
		hwGammaMode = ITE_VG_CMD_GAMMAMODE_INVERSE;
		hwEnGamma = ITE_TRUE;
	}
	else if (   ITEImage_IsLrgbFormat(pImage->vgformat) 
		     && ITEImage_IsSrgbFormat(context->surface->colorImage->vgformat) )
	{
		/* lRGB --> sRGB */
		hwGammaMode = ITE_VG_CMD_GAMMAMODE_GAMMA;
		hwEnGamma = ITE_TRUE;
	}
	else
	{
		hwEnGamma = ITE_FALSE;
	}

	/* Set hardware parameter */
	hw->REG_TCR_BASE = (enPerspective ? ITE_VG_CMD_PERSPECTIVE_EN : 0) |
		               ITE_VG_CMD_TRANSFORM_EN |
		               ITE_VG_CMD_READMEM |
		               ITE_VG_CMD_TELWORK;
    //hw->REG_PLR_BASE = hw->cmdLength * sizeof(ITEuint32);
	hw->REG_PLR_BASE = p.pathCommand.size * sizeof(ITEuint32);

	//allocate vram buffer
	if ( (p.pathCommand.size * sizeof(ITEuint32)) <= ITE_PATH_COPY_SIZE_THRESHOLD )
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = p.pathCommand.size * sizeof(ITEuint32);
		
		tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, allocSize, iteHardwareGenObjectID());
		mappedSysRam = ithMapVram((uint32_t)tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, p.pathCommand.items, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
	else
	{
#ifdef _WIN32
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = p.pathCommand.size * sizeof(ITEuint32);

		tessellateCmdBuffer = (ITEuint8*)vgMemalign(4, allocSize, iteHardwareGenObjectID());
		mappedSysRam = ithMapVram(tessellateCmdBuffer, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, p.pathCommand.items, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
#else
		tessellateCmdBuffer = (ITEuint8*)p.pathCommand.items;
#endif
		hwWaitObjID = ITE_TRUE;
	}

	/* Set object ID to image */
	pImage->objectID = iteHardwareGetCurrentObjectID();
	
	hw->REG_PBR_BASE = ((ITEuint32)tessellateCmdBuffer) << ITE_VG_CMDSHIFT_PATHBASE;
	hw->REG_BID2_BASE = iteHardwareGetCurrentObjectID();

	hw->REG_UTR00_BASE = (ITEs15p16)(imageMatrix.m[0][0] * 0x10000);
	hw->REG_UTR01_BASE = (ITEs15p16)(imageMatrix.m[0][1] * 0x10000);
	hw->REG_UTR02_BASE = (ITEs15p16)(imageMatrix.m[0][2] * 0x10000);
	hw->REG_UTR10_BASE = (ITEs15p16)(imageMatrix.m[1][0] * 0x10000);
	hw->REG_UTR11_BASE = (ITEs15p16)(imageMatrix.m[1][1] * 0x10000);
	hw->REG_UTR12_BASE = (ITEs15p16)(imageMatrix.m[1][2] * 0x10000);
	hw->REG_UTR20_BASE = (ITEs15p16)(imageMatrix.m[2][0] * 0x10000);
	hw->REG_UTR21_BASE = (ITEs15p16)(imageMatrix.m[2][1] * 0x10000);
	hw->REG_UTR22_BASE = (ITEs15p16)(imageMatrix.m[2][2] * 0x10000);
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
					   (hwEnPreMulDstImage ? ITE_VG_CMD_WR_NONPRE_EN : 0) |
		               ITE_VG_CMD_DITHER_EN | // always enable dither 
		               ((context->blendMode != VG_BLEND_SRC) ? ITE_VG_CMD_BLEND_EN : 0) |
		               (hwEnGamma ? ITE_VG_CMD_GAMMA_EN : 0) |
		               (context->enColorTransform ? ITE_VG_CMD_COLORXFM_EN : 0) |
                       (context->masking ? ITE_VG_CMD_MASK_EN : 0) |
	                   (hwEnPreMulBlending ? ITE_VG_CMD_DESTINATION_EN : 0) |
		               ITE_VG_CMD_TEXCACHE_EN |
		               ITE_VG_CMD_TEXTURE_EN |
		               hwEnTexPatIn |
		               (context->scissoring ? ITE_VG_CMD_SCISSOR_EN : 0) |
		               ITE_VG_CMD_COVERAGE_EN |
		               hwImageQuality |
		               hwFillRule |
		               ITE_VG_CMD_RENDERMODE_1 |
		               ITE_VG_CMD_RDPLN_VLD_EN;
	hw->REG_RMR_BASE = ITE_VG_CMD_AUTOSCAN_EN |
		               ITE_VG_CMD_MASKMODE_INTERSECT |
		               hwGammaMode |
		               ((context->blendMode & 0x1F) << 8) |
		               ((paint) ? (paint->tilingMode & 0x03) << 6 : ITE_VG_CMD_TILEMODE_FILL) |
		               ((paint) ? (paint->spreadMode & 0x03) << 4 : ITE_VG_CMD_RAMPMODE_PAD) |
		               (hwPaintType << 2) |
		               (context->imageMode & 0x03);
	hw->REG_RFR_BASE &= ~0xFFFFFF;
	hw->REG_RFR_BASE |= ITE_VG_CMD_SRCEXTEND_EN | 
		                ITE_VG_CMD_DSTEXTEND_EN | 
						ITE_VG_CMD_MASKEXTEND_EN |
		                (context->surface->maskImage->vgformat << 16) |
		                (context->surface->colorImage->vgformat << 8) |
		                pSrcImage->vgformat;

	/* context->colorTransform = { Sr, Sg, Sb, Sa, Br, Bg, Bb, Ba } */
#if 0
	/* Color Transform, [31:16]=Ba, [15:0]=Sa */	
	hw->REG_CTR0_BASE = ((ITEint16)(context->colorTransform[7] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM01 |
	                    (ITEint16)(context->colorTransform[3] * 0x100);
	/* Color Transform, [31:16]=Br, [15:0]=Sr */
	hw->REG_CTR1_BASE = ((ITEint16)(context->colorTransform[4] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM11 |
	                    (ITEint16)(context->colorTransform[0] * 0x100);
	/* Color Transform, [31:16]=Bg, [15:0]=Sg */
	hw->REG_CTR1_BASE = ((ITEint16)(context->colorTransform[5] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM21 |
	                    (ITEint16)(context->colorTransform[1] * 0x100);
	/* Color Transform, [31:16]=Bb, [15:0]=Sb */
	hw->REG_CTR1_BASE = ((ITEint16)(context->colorTransform[6] * 0x100)) << ITE_VG_CMDSHIFT_COLXFM31 |
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
	hw->REG_SDPR_BASE = (pSrcImage->pitch << ITE_VG_CMDSHIFT_SRCPITCH0) |
		                context->surface->colorImage->pitch;
	
	hw->REG_SCR_BASE = (((ITEs12p3)pSrcImage->offsetY << ITE_VG_CMDSHIFT_DSTY) & ITE_VG_CMDMASK_DSTY) |
		               ((ITEs12p3)pSrcImage->offsetX & ITE_VG_CMDMASK_DSTX);
	
	hw->REG_SHWR_BASE = (pSrcImage->height << ITE_VG_CMDSHIFT_SRCHEIGHT) | pSrcImage->width;
	hw->REG_SBR_BASE = (ITEuint32)(pSrcImage->data);
	hw->REG_MBR_BASE = (ITEuint32)(context->surface->maskImage->data);
	hw->REG_SMPR_BASE = ((context->scissoring && context->scissorImage.data) ? (context->scissorImage.pitch << ITE_VG_CMDSHIFT_SCISPITCH) : 0) |
		                context->surface->maskImage->pitch;
	hw->REG_SCBR_BASE = (context->scissoring && context->scissorImage.data) ? ((ITEuint32)(context->scissorImage.data) >> ITE_VG_CMDSHIFT_SCISBASE) : 0;
	{
		ITEMatrix3x3 imageInvMatrix;
		
		iteInvertMatrix(&imageMatrix, &imageInvMatrix);
		hw->REG_UITR00_BASE = (ITEs15p16)(imageInvMatrix.m[0][0] * 0x10000);
		hw->REG_UITR01_BASE = (ITEs15p16)(imageInvMatrix.m[0][1] * 0x10000);
		hw->REG_UITR02_BASE = (ITEs15p16)(imageInvMatrix.m[0][2] * 0x10000);
		hw->REG_UITR10_BASE = (ITEs15p16)(imageInvMatrix.m[1][0] * 0x10000);
		hw->REG_UITR11_BASE = (ITEs15p16)(imageInvMatrix.m[1][1] * 0x10000);
		hw->REG_UITR12_BASE = (ITEs15p16)(imageInvMatrix.m[1][2] * 0x10000);
		hw->REG_UITR20_BASE = (ITEs15p16)(imageInvMatrix.m[2][0] * 0x10000);
		hw->REG_UITR21_BASE = (ITEs15p16)(imageInvMatrix.m[2][1] * 0x10000);
		hw->REG_UITR22_BASE = (ITEs15p16)(imageInvMatrix.m[2][2] * 0x10000);
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

	/* Fill gradient parameter */
	if( paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT)
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
	else if( paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT )
	{
		ITEfloat r, R;
		ITEVector2 u,v,w;
		ITEfloat gradientValue = 0;

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
		hw->REG_GPRA_BASE = (ITEs15p16)(R * w.x * 0x10000);
		hw->REG_GPRB_BASE = (ITEs15p16)(R*w.y*0x10000);
		hw->REG_GPRC_BASE = (ITEs15p16)(-1*R*(v.x*w.x+v.y*w.y)*0x10000);

		/* s13.24 */
		// D
		gradientValue = R*R*(r*r-w.y*w.y);
		hw->REG_GPRD0_BASE = (ITEint32)gradientValue;
		hw->REG_GPRD1_BASE = GetSingleFloatMantissa(gradientValue);
		// E
		gradientValue = R*R*(r*r-w.x*w.x);
		hw->REG_GPRE0_BASE = (ITEint32)(gradientValue);
		hw->REG_GPRE1_BASE = GetSingleFloatMantissa(gradientValue);
		// F
		gradientValue = 2*R*R*w.x*w.y;
		hw->REG_GPRF0_BASE = (ITEint32)gradientValue;
		hw->REG_GPRF1_BASE = GetSingleFloatMantissa(gradientValue);
		// G
		gradientValue = 2*R*R*(w.y*w.y*v.x-v.y*w.x*w.y-r*r*v.x);
		hw->REG_GPRG0_BASE = (ITEint32)gradientValue;
		hw->REG_GPRG1_BASE = GetSingleFloatMantissa(gradientValue);
		// H
		gradientValue = 2*R*R*(w.x*w.x*v.y-v.x*w.x*w.y-r*r*v.y);
		hw->REG_GPRH0_BASE = (ITEint32)gradientValue;
		hw->REG_GPRH1_BASE = GetSingleFloatMantissa(gradientValue);
		// I
		gradientValue = R*R*(r*r*(v.x*v.x+v.y*v.y) + 2*v.x*v.y*w.x*w.y - v.x*v.x*w.y*w.y - v.y*v.y*w.x*w.x);
		hw->REG_GPRI0_BASE = (ITEint32)gradientValue;
		hw->REG_GPRI1_BASE = GetSingleFloatMantissa(gradientValue);
	}
	else
	{
		ITEuint16 preR, preG, preB;

		/* Transform color to pre-multiplied */
		preR = (ITEuint16)paint->color.r * paint->color.a;
		preG = (ITEuint16)paint->color.g * paint->color.a;
		preB = (ITEuint16)paint->color.b * paint->color.a;

		preR = (ITEuint16)( (preR>>8) + preR + (1<<3) )>>4;
		preG = (ITEuint16)( (preG>>8) + preG + (1<<3) )>>4;
		preB = (ITEuint16)( (preB>>8) + preB + (1<<3) )>>4;
		hw->REG_RCR00_BASE = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) |
			                 (paint->color.a &ITE_VG_CMDMASK_RAMPCOLOR0A);
		hw->REG_RCR01_BASE = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0R) |
			                 (preG & ITE_VG_CMDMASK_RAMPCOLOR0G);
	}

	/* Fill gradient parameter */
	//if(   (hwRenderMode == ITE_VG_CMD_RENDERMODE_1)
	//   && (paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT || paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT) )
	if (   (paint->type == VG_PAINT_TYPE_LINEAR_GRADIENT)
		|| (paint->type == VG_PAINT_TYPE_RADIAL_GRADIENT) )
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
				*currRampStopReg = ((ITEuint32)(pStop->offset * (1 << 12))) | ITE_VG_CMD_RAMPSTOP0EQ;

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
			*currRampColorReg = (preB << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | stopColor.a;
			currRampColorReg++;
			*currRampColorReg = (preR << ITE_VG_CMDSHIFT_RAMPCOLOR0B) | preG;
			currRampColorReg++;

			/* Divider */
			if ( itemIndex > 0 )
			{
				ITEs15p16 delta = (ITEs15p16)(pStop->offset - lastOffset);

				if ( delta )
				{
					*currDividerReg = ((ITEs15p16)(1 << 24)) / delta;
				}
				else
				{
					*currDividerReg = 0;
				}
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
	else
	{
		/* Set HW to cmdq */
		iteHardwareFire(hw);
	}

	if ( hwWaitObjID == ITE_TRUE )
	{
		iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());
	}
}

/*--------------------------------------------------
   OpenVG API
 *--------------------------------------------------*/

/*----------------------------------------------------------
 * Creates a new image object and returns the handle to it
 *----------------------------------------------------------*/

VG_API_CALL VGImage 
vgCreateImage(
	VGImageFormat format,
	VGint         width, 
	VGint         height,
	VGbitfield    allowedQuality)
{
	ITEImage *i = NULL;
	VG_GETCONTEXT(VG_INVALID_HANDLE);

	/* Reject invalid formats */
	VG_RETURN_ERR_IF(!iteIsValidImageFormat(format),
	                 VG_UNSUPPORTED_IMAGE_FORMAT_ERROR,
	                 VG_INVALID_HANDLE);

	/* Reject invalid sizes */
	VG_RETURN_ERR_IF(width  <= 0 || width > ITE_MAX_IMAGE_WIDTH ||
	                 height <= 0 || height > ITE_MAX_IMAGE_HEIGHT,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);

	/* Reject invalid quality bits */
	VG_RETURN_ERR_IF(!allowedQuality,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);
	VG_RETURN_ERR_IF(allowedQuality &
	                 ~(VG_IMAGE_QUALITY_NONANTIALIASED |
	                 VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);

	/* Check if byte size exceeds ITE_MAX_IMAGE_BYTES */
	i = iteCreateImage(format, width, height, allowedQuality, VG_TRUE, VG_FALSE);
	VG_RETURN_ERR_IF(!i, VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);

	/* Add to resource list */
	if ( !iteImageArrayPushBack(&context->images, i) )
	{
		// Add ITEImage to image array fail, free image resource
		ITE_DELETEOBJ(ITEImage, i);
		VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	}

	VG_RETURN((VGImage)i);
}

VG_API_CALL void vgDestroyImage(VGImage image)
{
	ITEint index;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* Check if valid resource */
	VG_RETURN_ERR_IF(!image, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	index = iteImageArrayFind(&context->images, (ITEImage*)image);
	VG_RETURN_ERR_IF(index == -1, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* Delete object and remove resource */
	iteImageArrayRemoveAt(&context->images, index);

	/* Delete object if noone reference to it*/
	if ( ITEImage_RemoveReference((ITEImage*)image) == 0 )
	{
		/* Prevent to modify not processed data */
		iteHardwareWaitObjID(((ITEImage*)image)->objectID);
		ITE_DELETEOBJ(ITEImage, (ITEImage*)image);
	}

	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Copies a rectangle area of pixels of size (width,height)
 * from given data buffer to image surface at destination
 * coordinates (x,y)
 *---------------------------------------------------------*/

VG_API_CALL void 
vgImageSubData(
	VGImage       image,
	const void*   data, 
	VGint         dataStride,
	VGImageFormat dataFormat,
	VGint         x, 
	VGint         y, 
	VGint         width, 
	VGint         height)
{
	ITEImage*  dstImage      = NULL;
	ITEImage   srcImage      = {0};
	ITEuint32  formatBytes   = iteGetFormatBytes(dataFormat);
	VGint      srcCopyX      = 0;
	VGint      srcCopyY      = 0;
	VGint      srcCopyWidth  = width;
	VGint      srcCopyHeight = height;
	VGint      dstCopyX      = x;
	VGint      dstCopyY      = y;
	VGint      dstCopyWidth  = width;
	VGint      dstCopyHeight = height;
	ITEboolean checkBoundRes = ITE_TRUE;
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidImage(context, image),
				     VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_UNSUPPORTED_IMAGE_FORMAT_ERROR
		¡V if dataFormat is not a valid value from the VGImageFormat enumeration */
	VG_RETURN_ERR_IF(!iteIsValidImageFormat(dataFormat),
				     VG_UNSUPPORTED_IMAGE_FORMAT_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0
		¡V if data is NULL
		¡V if data is not properly aligned */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0 || !data,
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!data || !CheckAlignment(data, iteGetFormatBytes(dataFormat)),
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Set srcImage */
	srcImage.width    = width;
	srcImage.height   = height;
	srcImage.pitch    = dataStride;
	srcImage.vgformat = dataFormat;
	
	/* Set dstImage */
	dstImage = (ITEImage*)image;

	checkBoundRes = iteCheckCopyBoundary(
		dstImage, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		&srcImage, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}

#ifdef _WIN32
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = width * height * formatBytes;
		
		srcImage.data = (ITEuint8*)VG_VMemAlloc(allocSize);
		mappedSysRam = ithMapVram(srcImage.data, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, data, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
#else
	srcImage.data = (ITEuint8*)data;
#endif

	iteSourceCopy(ITE_FALSE, dstImage, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, &srcImage, srcCopyX, srcCopyY);
				
#ifdef _WIN32
	if(srcImage.data)
	{
		VG_VMemFree(srcImage.data);
	}
#endif

	/* Wait to complete */
	iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());

	ITEImage_AddDrawCount(dstImage);

	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Copies a rectangle area of pixels of size (width,height)
 * from image surface at source coordinates (x,y) to given
 * data buffer
 *---------------------------------------------------------*/

VG_API_CALL void vgGetImageSubData(
	VGImage       image,
	void*         data, 
	VGint         dataStride,
	VGImageFormat dataFormat,
	VGint         x, 
	VGint         y,
	VGint         width, 
	VGint         height)
{
	ITEImage* srcImage       = NULL;
	ITEImage  dstImage       = {0};
	ITEuint32 formatBytes    = iteGetFormatBytes(dataFormat);
	VGint     srcCopyX       = x;
	VGint     srcCopyY       = y;
	VGint     srcCopyWidth   = width;
	VGint     srcCopyHeight  = height;
	VGint     dstCopyX       = 0;
	VGint     dstCopyY       = 0;
	VGint     dstCopyWidth   = width;
	VGint     dstCopyHeight  = height;
	ITEboolean checkBoundRes = ITE_TRUE;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidImage(context, image),
				     VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_UNSUPPORTED_IMAGE_FORMAT_ERROR
		¡V if dataFormat is not a valid value from the VGImageFormat enumeration */
	VG_RETURN_ERR_IF(!iteIsValidImageFormat(dataFormat),
				     VG_UNSUPPORTED_IMAGE_FORMAT_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0
		¡V if data is NULL
		¡V if data is not properly aligned */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0 || !data,
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!data || !CheckAlignment(data, iteGetFormatBytes(dataFormat)),
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Set srcImage */
	srcImage = (ITEImage*)image;

	/* Set dstImage */
	dstImage.pitch    = dataStride;
	dstImage.vgformat = dataFormat;
	dstImage.width    = width;
	dstImage.height   = height;

	checkBoundRes = iteCheckCopyBoundary(
		&dstImage, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		srcImage, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}

#ifdef _WIN32    
	dstImage.data = (ITEuint8*)VG_VMemAlloc(width * height * formatBytes);
#else
	dstImage.data = data;
#endif
	
	iteSourceCopy(ITE_FALSE, &dstImage, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, srcImage, srcCopyX, srcCopyY);

#ifdef _WIN32
	if(dstImage.data)
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 mapLength    = width * height * formatBytes;

		mappedSysRam = ithMapVram(dstImage.data, mapLength, ITH_VRAM_READ);
		VG_Memcpy(data, mappedSysRam, mapLength);
		ithUnmapVram(mappedSysRam, mapLength);
		VG_VMemFree(dstImage.data);
	}	  
#endif

	/* Wait to complete */
	iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());

	ITEImage_AddDrawCount(srcImage);

	VG_RETURN(VG_NO_RETVAL);
}

/*----------------------------------------------------------
 * Copies a rectangle area of pixels of size (width,height)
 * from src image surface at source coordinates (sx,sy) to
 * dst image surface at destination cordinates (dx,dy)
 *---------------------------------------------------------*/

VG_API_CALL void vgCopyImage(
	VGImage   dst, 
	VGint     dx, 
	VGint     dy,
	VGImage   src, 
	VGint     sx, 
	VGint     sy,
	VGint     width, 
	VGint     height,
	VGboolean dither)
{
	ITEImage*  srcImage      = NULL;
	ITEImage*  dstImage      = NULL;
	VGint      srcCopyX      = sx;
	VGint      srcCopyY      = sy;
	VGint      srcCopyWidth  = width;
	VGint      srcCopyHeight = height;
	VGint      dstCopyX      = dx;
	VGint      dstCopyY      = dy;
	VGint      dstCopyWidth  = width;
	VGint      dstCopyHeight = height;
	ITEboolean checkBoundRes = ITE_TRUE;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if either dst or src is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidImage(context, src) ||
	               !iteIsValidImage(context, dst),
	               VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if either dst or src is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst) || iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0,
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	srcImage = (ITEImage*)src;
	dstImage = (ITEImage*)dst;

	checkBoundRes = iteCheckCopyBoundary(
		dstImage, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		srcImage, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}

	iteSourceCopy(ITE_FALSE, dstImage, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, srcImage, srcCopyX, srcCopyY);

	ITEImage_AddDrawCount(srcImage);
	ITEImage_AddDrawCount(dstImage);

	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Copies a rectangle area of pixels of size (width,height)
 * from src image surface at source coordinates (sx,sy) to
 * window surface at destination coordinates (dx,dy)
 *---------------------------------------------------------*/

VG_API_CALL void vgSetPixels(
	VGint   dx, 
	VGint   dy,
	VGImage src, 
	VGint   sx, 
	VGint   sy,
	VGint   width, 
	VGint   height)
{
	ITEImage*  srcImage      = NULL;
	ITEImage*  dstImage      = NULL;
	VGint      srcCopyX      = sx;
	VGint      srcCopyY      = sy;
	VGint      srcCopyWidth  = width;
	VGint      srcCopyHeight = height;
	VGint      dstCopyX      = dx;
	VGint      dstCopyY      = dy;
	VGint      dstCopyWidth  = width;
	VGint      dstCopyHeight = height;
	ITEboolean checkBoundRes = ITE_TRUE;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if src is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidImage(context, src),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	
	/* VG_IMAGE_IN_USE_ERROR
		¡V if src is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	dstImage = context->surface->colorImage;
	srcImage = (ITEImage*)src;

	checkBoundRes = iteCheckCopyBoundary(
		dstImage, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		srcImage, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}

	iteSourceCopy(ITE_TRUE, dstImage, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, srcImage, srcCopyX, srcCopyY);

	ITEImage_AddDrawCount(srcImage);

	VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------------
 * Copies a rectangle area of pixels of size (width,height)
 * from given data buffer at source coordinates (sx,sy) to
 * window surface at destination coordinates (dx,dy)
 *---------------------------------------------------------*/

VG_API_CALL void 
vgWritePixels(
	const void*   data, 
	VGint         dataStride,
	VGImageFormat dataFormat,
	VGint         dx, 
	VGint         dy,
	VGint         width, 
	VGint         height)
{
	ITEImage   srcImage      = {0};
	ITEImage*  dstImage      = NULL;
	ITEuint32  formatBytes   = iteGetFormatBytes(dataFormat);
	VGint      srcCopyX      = 0;
	VGint      srcCopyY      = 0;
	VGint      srcCopyWidth  = width;
	VGint      srcCopyHeight = height;
	VGint      dstCopyX      = dx;
	VGint      dstCopyY      = dy;
	VGint      dstCopyWidth  = width;
	VGint      dstCopyHeight = height;
	ITEboolean checkBoundRes = ITE_TRUE;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_UNSUPPORTED_IMAGE_FORMAT_ERROR
		¡V if dataFormat is not a valid value from the VGImageFormat enumeration */
	VG_RETURN_ERR_IF(!iteIsValidImageFormat(dataFormat),
				     VG_UNSUPPORTED_IMAGE_FORMAT_ERROR,
				     VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0
		¡V if data is NULL
		¡V if data is not properly aligned */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0 || !data,
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!CheckAlignment(data, iteGetFormatBytes(dataFormat)),
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Set srcImage */
	srcImage.width    = width;
	srcImage.height   = height;
	srcImage.pitch    = dataStride;
	srcImage.vgformat = dataFormat;

	/* Set dstImage */
	dstImage = (ITEImage*)context->surface->colorImage;

	checkBoundRes = iteCheckCopyBoundary(
		dstImage, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		&srcImage, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}

#ifdef _WIN32    
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize	   = width * height * formatBytes;
		
		srcImage.data = (ITEuint8*)VG_VMemAlloc(width * height * formatBytes);
		mappedSysRam = ithMapVram(srcImage.data, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, data, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
#else
	srcImage.data = (ITEuint8*)data;
#endif

	iteSourceCopy(ITE_TRUE, dstImage, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, &srcImage, srcCopyX, srcCopyY);

#ifdef _WIN32
   if(srcImage.data)
   {
		VG_VMemFree(srcImage.data);
   }
#endif

	/* Wait to complete */
	iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());

	VG_RETURN(VG_NO_RETVAL); 
}

/*-----------------------------------------------------------
 * Copies a rectangle area of pixels of size (width, height)
 * from window surface at source coordinates (sx, sy) to
 * image surface at destination coordinates (dx, dy)
 *-----------------------------------------------------------*/

VG_API_CALL void vgGetPixels(
	VGImage dst, 
	VGint   dx, 
	VGint   dy,
	VGint   sx, 
	VGint   sy,
	VGint   width, 
	VGint   height)
{
	ITEImage*  srcImage      = NULL;
	ITEImage*  dstImage      = NULL;
	VGint      srcCopyX      = sx;
	VGint      srcCopyY      = sy;
	VGint      srcCopyWidth  = width;
	VGint      srcCopyHeight = height;
	VGint      dstCopyX      = dx;
	VGint      dstCopyY      = dy;
	VGint      dstCopyWidth  = width;
	VGint      dstCopyHeight = height;
	ITEboolean checkBoundRes = ITE_TRUE;
	
	VG_GETCONTEXT(VG_NO_RETVAL);

	dstImage = (ITEImage*)dst;
	srcImage = (ITEImage*)context->surface->colorImage;

	/* VG_BAD_HANDLE_ERROR
		¡V if dst is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidImage(context, dst),
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if dst is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0,
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	checkBoundRes = iteCheckCopyBoundary(
		dstImage, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		srcImage, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}
	
	iteSourceCopy(ITE_FALSE, dstImage, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, srcImage, srcCopyX, srcCopyY);

	ITEImage_AddDrawCount(dstImage);

	VG_RETURN(VG_NO_RETVAL);
}

/*-----------------------------------------------------------
 * Copies a rectangle area of pixels of size (width, height)
 * from window surface at source coordinates (sx, sy) to
 * to given output data buffer.
 *-----------------------------------------------------------*/

VG_API_CALL void 
vgReadPixels(
	void*         data, 
	VGint         dataStride,
	VGImageFormat dataFormat,
	VGint         sx, 
	VGint         sy,
	VGint         width, 
	VGint         height)
{
	ITEImage*  srcImage      = NULL;
	ITEImage   dstImage      = {0};
	ITEuint    formatBytes   = iteGetFormatBytes(dataFormat);
	VGint      srcCopyX      = sx;
	VGint      srcCopyY      = sy;
	VGint      srcCopyWidth  = width;
	VGint      srcCopyHeight = height;
	VGint      dstCopyX      = 0;
	VGint      dstCopyY      = 0;
	VGint      dstCopyWidth  = width;
	VGint      dstCopyHeight = height;
	ITEboolean checkBoundRes = ITE_TRUE;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_UNSUPPORTED_IMAGE_FORMAT_ERROR
		¡V if dataFormat is not a valid value from the VGImageFormat enumeration */
	VG_RETURN_ERR_IF(!iteIsValidImageFormat(dataFormat),
				     VG_UNSUPPORTED_IMAGE_FORMAT_ERROR,
				     VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0
		¡V if data is NULL
		¡V if data is not properly aligned */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0 || !data,
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!CheckAlignment(data, formatBytes),
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* Set srcImage */
	srcImage = context->surface->colorImage;

	/* Set dstImage */
	dstImage.width    = width;
	dstImage.height   = height;
	dstImage.pitch    = dataStride;
	dstImage.vgformat = dataFormat;

	checkBoundRes = iteCheckCopyBoundary(
		&dstImage, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		srcImage, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}

#ifdef _WIN32
	dstImage.data = (ITEuint8*)VG_VMemAlloc(width * height * formatBytes);
#else
	dstImage.data = (ITEuint8*)data;
#endif	

	iteSourceCopy(ITE_FALSE, &dstImage, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, srcImage, srcCopyX, srcCopyY);

#ifdef _WIN32
   if(dstImage.data)
   {
   		ITEuint8* mappedSysRam = NULL;
		ITEuint32 mapLength    = width * height * formatBytes;

		mappedSysRam = ithMapVram(dstImage.data, mapLength, ITH_VRAM_READ);
		VG_Memcpy(data, mappedSysRam, mapLength);
		ithUnmapVram(mappedSysRam, mapLength);
		
		VG_VMemFree(dstImage.data);
   }	  
#endif

	/* Wait to complete */
	iteHardwareWaitObjID(iteHardwareGetCurrentObjectID());

	VG_RETURN(VG_NO_RETVAL);
}

/*----------------------------------------------------------
 * Copies a rectangle area of pixels of size (width,height)
 * from window surface at source coordinates (sx,sy) to
 * windows surface at destination cordinates (dx,dy)
 *---------------------------------------------------------*/

VG_API_CALL void vgCopyPixels(
	VGint dx, 
	VGint dy,
	VGint sx, 
	VGint sy,
	VGint width, 
	VGint height)
{
    ITEImage*  im            = NULL;
	VGint      srcCopyX      = sx;
	VGint      srcCopyY      = sy;
	VGint      srcCopyWidth  = width;
	VGint      srcCopyHeight = height;
	VGint      dstCopyX      = dx;
	VGint      dstCopyY      = dy;
	VGint      dstCopyWidth  = width;
	VGint      dstCopyHeight = height;
	ITEboolean checkBoundRes = ITE_TRUE;
    VG_GETCONTEXT(VG_NO_RETVAL);
    
    /* VG_ILLEGAL_ARGUMENT_ERROR
    	¡V if width or height is less than or equal to 0 */
    VG_RETURN_ERR_IF(width <= 0 || height <= 0,
                   VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	im = (ITEImage*)context->surface->colorImage;

	checkBoundRes = iteCheckCopyBoundary(
		im, 
		&dstCopyX, 
		&dstCopyY, 
		&dstCopyWidth, 
		&dstCopyHeight, 
		im, 
		&srcCopyX, 
		&srcCopyY, 
		&srcCopyWidth, 
		&srcCopyHeight);
	if ( checkBoundRes == ITE_FALSE )
	{
		/* Do nothing if out of boundary */
		VG_RETURN(VG_NO_RETVAL);
	}
    
    iteSourceCopy(ITE_TRUE, im, dstCopyX, dstCopyY, dstCopyWidth, dstCopyHeight, im, srcCopyX, srcCopyY);
    
    VG_RETURN(VG_NO_RETVAL);
}

/*---------------------------------------------------
 * Clear given rectangle area in the image data with
 * color set via vgSetfv(VG_CLEAR_COLOR, ...)
 *---------------------------------------------------*/

VG_API_CALL void 
vgClearImage(
	VGImage image,
	VGint   x, 
	VGint   y, 
	VGint   width, 
	VGint   height)
{
	ITEImage* im          = NULL;
	VGint     clearWidth  = width;
	VGint     clearHeight = height;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current contex */
	VG_RETURN_ERR_IF((ITEImage*)image == NULL, 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image),
                     VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if width or height is less than or equal to 0 */
	VG_RETURN_ERR_IF(width <= 0 || height <= 0, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	im = (ITEImage*)image;

	/* Nothing to do if target rectangle out of bounds */
	if (   (x >= im->width)
		|| (y >= im->height) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}
	if (   ((x + im->width) < 0)
		|| ((y + im->height) < 0) )
	{
		VG_RETURN(VG_NO_RETVAL);
	}

	/* Check copy boundary */
	if ( (x + width) > im->width )
	{
		clearWidth = im->width - x;
	}
	if ( (y + height) > im->height )
	{
		clearHeight = im->height - y;
	}

	iteSetImage(ITE_FALSE, im, x, y, clearWidth, clearHeight, context->clearColor);

	ITEImage_AddDrawCount(im);

	VG_RETURN(VG_NO_RETVAL);
}

VG_API_CALL void vgDrawImage(VGImage image)
{
#if 1
	ITEImage* pImage = NULL;

	VG_GETCONTEXT(VG_NO_RETVAL);

	/*VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!iteIsValidImage(context, image),
	                   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image),
                     VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	pImage = (ITEImage*)image;

	iteDrawImage(pImage, &context->imageTransform);

	ITEImage_AddDrawCount(pImage);

	VG_RETURN(VG_NO_RETVAL);

#else
	ITEImage *im = (ITEImage *)image;
	ITEImage covImage, valImage;
	ITEColor c;
	ITEMatrix3x3 m, invm;
	ITEPaint* paint;
	ITEHardware *h;
	ITEPath	p;
	HWVector2 covmax, covmin;
	//ITEfloat coords[] = { 0.0f, 0.0f, (ITEfloat)(im->width-1), (ITEfloat)(im->height-1), (ITEfloat)(1-im->width) };
	ITEfloat coords[5];
	ITEuint8 segments[] = { VG_MOVE_TO_ABS, VG_HLINE_TO_REL, VG_VLINE_TO_REL, VG_HLINE_TO_REL, VG_CLOSE_PATH };

	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;
  
	VG_RETURN_ERR_IF(im == NULL, VG_BAD_HANDLE_ERROR, 
		VG_NO_RETVAL);

	VG_RETURN_ERR_IF(!iteIsValidImage(context, image),
                   VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	coords[0] = 0.0f;
	coords[1] = 0.0f;
	coords[2] = (ITEfloat)(im->width-1);
	coords[3] = (ITEfloat)(im->height-1);
	coords[4] = (ITEfloat)(1-im->width);

	p.format = VG_PATH_FORMAT_STANDARD;
	p.scale = 1.0f;
	p.bias = 0.0f;
	p.segHint = 0;
	p.dataHint = 0;
	p.datatype = VG_PATH_DATATYPE_F;
	p.caps = VG_PATH_CAPABILITY_ALL;
	p.segCount = 5;
	p.segs = segments;
	p.dataCount = 5;
	p.data = coords;


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
		CSET(c, 0, 0, 0, 0x80);
		iteSetImage(&context->scissorImage, 0, 0, context->scissorImage.width, context->scissorImage.height, c);
		CSET(c, 0, 0, 0, 0);
		for(i=0;i<context->scissor.size;i++)
		{
			rect = &context->scissor.items[i];
		    iteSetImage(&context->scissorImage, (ITEint)rect->x, (ITEint)rect->y, (ITEint)rect->w, (ITEint)rect->h, c);
		}
	}

	iteFlattenPath(&p, 1);

	h->paintMode = HW_FILL_PATH;

	if (context->imageTransform.m[2][0] || context->imageTransform.m[2][1] || context->imageTransform.m[2][2]!=1.0f)
		h->enPerspective = HW_TRUE;

	SETMAT(h->pathTransform, (ITEs15p16)(context->imageTransform.m[0][0]*0x10000), (ITEs15p16)(context->imageTransform.m[0][1]*0x10000), (ITEs15p16)(context->imageTransform.m[0][2]*0x10000),
							 (ITEs15p16)(context->imageTransform.m[1][0]*0x10000), (ITEs15p16)(context->imageTransform.m[1][1]*0x10000), (ITEs15p16)(context->imageTransform.m[1][2]*0x10000),
							 (ITEs15p16)(context->imageTransform.m[2][0]*0x10000), (ITEs15p16)(context->imageTransform.m[2][1]*0x10000), (ITEs15p16)(context->imageTransform.m[2][2]*0x10000));
	iteTessllationEngine();
	
	// Handle Coverage buffer
	CSET(c, 0, 0, 0, 0);
	
	// centrial aligment
	SET2V(covmax,h->max);
	ADD2(covmax,4,4);
	SET2V(covmin,h->min);
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

	context->updateFlag.imageMatrixFlag = VG_TRUE;
	context->updateFlag.coverageFlag = VG_TRUE;
	context->updateFlag.textureFlag = VG_TRUE;
	context->updateFlag.paintBaseFlag = VG_TRUE;
	context->updateFlag.fillMatrixFlag = VG_TRUE;

	if(context->updateFlag.enableFlag)
	{
		h->enMask = context->masking;
		h->enScissor = context->scissoring;
		h->enColorTransform = context->enColorTransform;
		h->enBlend = HW_TRUE;
		h->enTexture = HW_TRUE;
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
		h->maskMode = HW_INTERSECT_RENDERMASK;
		h->imageMode = context->imageMode&0x3;
		h->tilingMode = context->tilingMode&0x3;
	}

	if(context->updateFlag.surfaceFlag)
	{
		h->surfacePitch = (ITEint16)context->surface->colorImage->pitch;
		h->surfaceWidth = (ITEint16)context->surface->colorImage->width;
		h->surfaceHeight = (ITEint16)context->surface->colorImage->height;
		h->surfaceData = context->surface->colorImage->data;
		h->surfaceFormat = context->surface->colorImage->vgformat;
	}

	if(context->updateFlag.maskFlag)
	{
		h->maskPitch = (ITEint16)context->surface->maskImage->pitch;
		h->maskWidth = (ITEint16)context->surface->maskImage->width;
		h->maskHeight = (ITEint16)context->surface->maskImage->height;
		h->maskData = context->surface->maskImage->data;
		h->maskFormat = context->surface->maskImage->vgformat;
	}

	if( context->updateFlag.imageMatrixFlag )
	{
		MULMATMAT(context->pathTransform, context->imageTransform, m);
		iteInvertMatrix(&m, &invm);
		SETMAT(h->imageTransform, (ITEs15p16)(invm.m[0][0]*0x10000), (ITEs15p16)(invm.m[0][1]*0x10000), (ITEs15p16)(invm.m[0][2]*0x10000),
									 (ITEs15p16)(invm.m[1][0]*0x10000), (ITEs15p16)(invm.m[1][1]*0x10000), (ITEs15p16)(invm.m[1][2]*0x10000),
									 (ITEs15p16)(invm.m[2][0]*0x10000), (ITEs15p16)(invm.m[2][1]*0x10000), (ITEs15p16)(invm.m[2][2]*0x10000));
	}

	if( context->updateFlag.paintBaseFlag )
	{
		paint = (context->fillPaint) ? context->fillPaint : &context->defaultPaint;
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
		h->patternFormat = VG_sRGBA_8888;
		h->patternPitch = (ITEint16)(1<<(h->gradientLen+2));
		h->patternWidth = (ITEint16)(1<<h->gradientLen);
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
		h->patternFormat = VG_sRGBA_8888;
		h->patternPitch = (ITEint16)(1<<(h->gradientLen+2));
		h->patternWidth = (ITEint16)(1<<h->gradientLen);
		h->patternHeight = 1;
		h->spreadMode = paint->spreadMode&0x3;
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

	if( context->updateFlag.textureFlag )
	{
		h->texturePitch = (ITEint16)im->pitch;
		h->textureWidth = (ITEint16)im->width;
		h->textureHeight = (ITEint16)im->height;
		h->textureData = im->data;
		h->textureFormat = im->vgformat;
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

	// clear flag
	memset(&context->updateFlag, 0, sizeof(ITEUpdateFlag));

  VG_RETURN(VG_NO_RETVAL);
  #endif
}

VG_API_CALL VGImage 
vgChildImage(
	VGImage parent,
	VGint   x, 
	VGint   y, 
	VGint   width, 
	VGint   height)
{
	ITEImage* src   = (ITEImage*)parent;
	ITEImage* child = NULL;
	VG_GETCONTEXT(VG_INVALID_HANDLE);

	/* VG_BAD_HANDLE_ERROR
		¡Vif parent is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!parent || !iteIsValidImage(context, parent),
                     VG_BAD_HANDLE_ERROR, VG_INVALID_HANDLE);
	/* VG_IMAGE_IN_USE_ERROR
		¡Vif parent is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, parent),
                     VG_IMAGE_IN_USE_ERROR, VG_INVALID_HANDLE);
	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if x is less than 0 or greater than or equal to the parent width
		¡V if y is less than 0 or greater than or equal to the parent height
		¡V if width or height is less than or equal to 0
		¡V if x + width is greater than the parent width
		¡V if y + height is greater than the parent height */
	VG_RETURN_ERR_IF(x < 0 || x >= src->width ||
		             y < 0 || y >= src->height ||
		             width <= 0 || height <= 0 ||
		             (x + width) > src->width ||
		             (y + height) > src->height,
                     VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);

	child = iteCreateImage(src->vgformat, width, height, src->allowedQuality, VG_FALSE, VG_FALSE);
	VG_RETURN_ERR_IF(!child, VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	child->data    = src->data;
	child->offsetX = src->offsetX + x;
	child->offsetY = src->offsetY + y;

	/* Add to image resource list */
	if ( !iteImageArrayPushBack(&context->images, child) )
	{
		// Add ITEImage to image array fail, free image resource
		ITE_DELETEOBJ(ITEImage, child);
		VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	}

	/* Add child's InUse count & parent's InUse count */
	child->parent = src;
	ITEImage_AddInUse(child);
	ITEImage_AddInUse(src);
	ITEImage_AddReference(src);
	
	return (VGImage)child;
}

VG_API_CALL VGImage vgGetParent(VGImage image)
{
	VGImage   returnImage = image;
	ITEImage* parentImage = NULL;
	VG_GETCONTEXT(VG_INVALID_HANDLE);

	/* VG_BAD_HANDLE_ERROR
		¡V if image is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!image || !iteIsValidImage(context, image),
                     VG_BAD_HANDLE_ERROR, VG_INVALID_HANDLE);
	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image),
                     VG_IMAGE_IN_USE_ERROR, VG_INVALID_HANDLE);

	/* The vgGetParent function returns the closest valid ancestor (i.e., one that has not been
		the target of a vgDestroyImage call) of the given image. If image has no ancestors, 
		image is returned. */
	parentImage = ((ITEImage*)image)->parent;
	while ( parentImage )
	{
		if ( iteIsValidImage(context, (VGImage)parentImage) )
		{
			returnImage = (VGImage)parentImage;
			break;
		}
		parentImage = parentImage->parent;
	}
	
	return returnImage;
}

VG_API_CALL void 
vgColorMatrix(
	VGImage        dst, 
	VGImage        src,
	const VGfloat* matrix)
{
	ITEuint				channelMask          = 0;
	ITEint				i				     = 0;
	ITEuint32			colorM[20]           = {0};
	ITEHardwareRegister hwReg                = {0};
	ITEboolean          srcPreMultiFormat    = ITE_FALSE;
	ITEboolean          dstPreMultiFormat    = ITE_FALSE;
	ITEboolean          EnPreMultiForTexture = ITE_FALSE; // Reg 0x0AC, Bit 28
	ITEboolean          EnUnpreMultiForDst   = ITE_FALSE; // Reg 0x0AC, Bit 31
	ITEboolean          EnPreMultiForBlend   = ITE_FALSE; // Reg 0x0AC, Bit 29
	ITEboolean          EnUnpreMultiForCT    = ITE_FALSE; // Reg 0x0AC, Bit 30
	HWMatrix3x3         transMat             = {0};
	ITEboolean          enBlend              = ITE_FALSE;
	ITEuint             blendMode;
	ITEuint             lookupMode;  
	ITEImage *is, *id, *itmp;

	VG_GETCONTEXT(VG_NO_RETVAL);

	is = (ITEImage*)src;
	id = (ITEImage*)dst;

	/* VG_BAD_HANDLE_ERROR
		¡V if either dst or src is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!dst || !iteIsValidImage(context, dst) || 
	                 !src || !iteIsValidImage(context, src), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if either dst or src is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst) || iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if src and dst overlap
		¡V if matrix is NULL
		¡V if matrix is not properly aligned */
	VG_RETURN_ERR_IF(iteImageIsOverlap(is, id),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!matrix || !CheckAlignment(matrix, sizeof(VGfloat)),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!matrix || !CheckAlignment(matrix, sizeof(VGfloat)),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	// Ignore undefined bits
	channelMask = context->filterChannelMask & (VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA);

	// Copy matrix and check value boundary
	for ( i = 0; i < 20; i++ )
	{
		if(i <= 15)
		{
			colorM[i] = (ITEs7p8)(iteValidInputFloat(matrix[i])*0x100);
		}
		else
		{
			colorM[i] = (ITEs15p16)(iteValidInputFloat(matrix[i])*0x100*255);
		}

	}

	ITE_INITOBJ(ITEHardware, hwReg);

	SETMAT(transMat, 0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x10000);

	itmp = iteCreateImage(
		HW_RGBA_16, 
		id->width, 
		id->height, 
		0, 
		VG_FALSE,
	    VG_TRUE);
	
	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height,iteHardwareGenObjectID());


    //step1
    blendMode = VG_BLEND_SRC;
    lookupMode = ITE_VG_CMD_LOOKUPMODE_R;

	iteImageColorMatrix(itmp, 0, 0, itmp->width, itmp->height, is, 0, 0,&colorM[0] , 0, blendMode, ITE_FALSE, lookupMode);
	

    //step2
    blendMode = VG_BLEND_ADDITIVE;
    lookupMode = ITE_VG_CMD_LOOKUPMODE_G;

	iteImageColorMatrix(itmp, 0, 0, itmp->width, itmp->height, is, 0, 0,&colorM[4] , 0, blendMode, ITE_FALSE, lookupMode);


	//step3
    blendMode = VG_BLEND_ADDITIVE;
    lookupMode = ITE_VG_CMD_LOOKUPMODE_B;

	iteImageColorMatrix(itmp, 0, 0, itmp->width, itmp->height, is, 0, 0,&colorM[8] , 0, blendMode, ITE_FALSE, lookupMode);
	
	//step4
    blendMode = VG_BLEND_ADDITIVE;
    lookupMode = ITE_VG_CMD_LOOKUPMODE_A;

	iteImageColorMatrix(itmp, 0, 0, itmp->width, itmp->height, is, 0, 0,&colorM[12] , &colorM[16], blendMode, ITE_FALSE, lookupMode);

	iteImageCombine(id, 0, 0, id->width, id->height, itmp, 0, 0, (ITEs7p8)0x100, 0, VG_BLEND_SRC, VG_TILE_PAD, 1);

	ITEImage_AddDrawCount(is);
	ITEImage_AddDrawCount(id);
	
    ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);
}

VG_API_CALL void 
vgConvolve(
	VGImage        dst, 
	VGImage        src,
	VGint          kernelWidth, 
	VGint          kernelHeight,
	VGint          shiftX, 
	VGint          shiftY,
	const VGshort* kernel,
	VGfloat        scale,
	VGfloat        bias,
	VGTilingMode   tilingMode)
{
	ITEImage *is, *id, *itmp;
	ITEint i,j;
	ITEint16 k;
	VG_GETCONTEXT(VG_NO_RETVAL);
	is = (ITEImage*)src;
	id = (ITEImage*)dst;

	/* VG_BAD_HANDLE_ERROR
		 if either dst or src is not a valid image handle, or is not shared with the */
	VG_RETURN_ERR_IF(!dst || !iteIsValidImage(context, dst) ||
	                 !src || !iteIsValidImage(context, src),
                     VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if either dst or src is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst) ||
	                 iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if src and dst overlap
		¡V if kernelWidth or kernelHeight is less than or equal to 0 or greater than VG_MAX_KERNEL_SIZE
		¡V if kernel is NULL
		¡V if kernel is not properly aligned
		¡V if tilingMode is not one of the values from the VGTilingMode enumeration */
	VG_RETURN_ERR_IF(iteImageIsOverlap(is, id), 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(kernelWidth  <= 0 || 
		             kernelWidth  >  ITE_MAX_KERNEL_SIZE ||
		             kernelHeight <= 0 ||
		             kernelHeight >  ITE_MAX_KERNEL_SIZE, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!kernel || !CheckAlignment(kernel, sizeof(VGshort)), 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!iteIsValidTilingMode(tilingMode),
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	itmp = iteCreateImage(
		HW_RGBA_16, 
		id->width, 
		id->height, 
		0, 
		VG_FALSE,
	    VG_TRUE);

	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height, iteHardwareGenObjectID());
    //itmp->data = (ITEuint8*)MEM_Memalign(8,itmp->pitch * itmp->height, MEM_USER_VG);

	for(j=0;j<kernelHeight;j++)	
	//i = j = 0;
	{
		for(i=0;i<kernelWidth;i++)
		{
			int kx = kernelWidth-i-1;			
			int ky = kernelHeight-j-1;
			k = *(kernel + kx*kernelHeight+ky);
			if(i==0 && j==0)
				iteImageCombine(itmp, 0, 0, itmp->width, itmp->height, is, i-shiftX, j-shiftY, (ITEs7p8)((k*scale)*0x100), 0, VG_BLEND_SRC, tilingMode, 0);
			else if (i==kernelHeight-1 && j==kernelWidth-1)
				iteImageCombine(itmp, 0, 0, itmp->width, itmp->height, is, i-shiftX, j-shiftY, (ITEs7p8)((k*scale)*0x100), (ITEs15p16)(bias*255*0x100), VG_BLEND_ADDITIVE, tilingMode, 0);
			else
				iteImageCombine(itmp, 0, 0, itmp->width, itmp->height, is, i-shiftX, j-shiftY, (ITEs7p8)((k*scale)*0x100), 0, VG_BLEND_ADDITIVE, tilingMode, 0);
		}
	}
	iteImageCombine(id, 0, 0, id->width, id->height, itmp, 0, 0, (ITEs7p8)0x100, 0, VG_BLEND_SRC, tilingMode, 1);

	ITEImage_AddDrawCount(is);
	ITEImage_AddDrawCount(id);
	
	ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);
}

VG_API_CALL void 
vgSeparableConvolve(
	VGImage        dst, 
	VGImage        src,
	VGint          kernelWidth,
	VGint          kernelHeight,
	VGint          shiftX, 
	VGint          shiftY,
	const VGshort* kernelX,
	const VGshort* kernelY,
	VGfloat        scale,
	VGfloat        bias,
	VGTilingMode   tilingMode)
{
	ITEImage *is, *id, *itmp;
	ITEint i,j;
	ITEint16 k1;
	ITEint16 k2;
	VG_GETCONTEXT(VG_NO_RETVAL);

	is = (ITEImage*)src;
	id = (ITEImage*)dst;
	
	/* VG_BAD_HANDLE_ERROR
		¡V if either dst or src is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!dst || !iteIsValidImage(context, dst) || 
	                 !src || !iteIsValidImage(context, src), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if either dst or src is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst) || iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if src and dst overlap
		¡V if kernelWidth or kernelHeight is less than or equal to 0 or greater than VG_MAX_SEPARABLE_KERNEL_SIZE
		¡V if kernelX or kernelY is NULL
		¡V if kernelX or kernelY is not properly aligned
		¡V if tilingMode is not one of the values from the VGTilingMode enumeration */
	VG_RETURN_ERR_IF(iteImageIsOverlap(is, id),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(kernelWidth  <= 0 || 
		             kernelWidth  >  ITE_MAX_SEPARABLE_KERNEL_SIZE ||
		             kernelHeight <= 0 ||
		             kernelHeight >  ITE_MAX_SEPARABLE_KERNEL_SIZE, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!kernelX || !CheckAlignment(kernelX, sizeof(VGshort)) ||
		             !kernelY || !CheckAlignment(kernelY, sizeof(VGshort)), 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!iteIsValidTilingMode(tilingMode),
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	itmp = iteCreateImage(
		HW_RGBA_16, 
		id->width, 
		id->height, 
		0, 
		VG_FALSE,
	    VG_TRUE);

	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height,iteHardwareGenObjectID());

	for(j=0;j<kernelHeight;j++)	
	//i = j = 0;
	{
		for(i=0;i<kernelWidth;i++)
		{
			int kx = kernelWidth-i-1;			
			int ky = kernelHeight-j-1;
			k1 = *(kernelX + kx*kernelHeight);
			k2 = *(kernelY + ky*kernelWidth);
			if(i==0 && j==0)
				iteImageCombine(itmp, 0, 0, itmp->width, itmp->height, is, i-shiftX, j-shiftY, (ITEs7p8)((k1*k2*scale)*0x100), 0, VG_BLEND_SRC, tilingMode, 0);
			else if (i==kernelHeight-1 && j==kernelWidth-1)
				iteImageCombine(itmp, 0, 0, itmp->width, itmp->height, is, i-shiftX, j-shiftY, (ITEs7p8)((k1*k2*scale)*0x100), (ITEs15p16)(bias*255*0x100), VG_BLEND_ADDITIVE, tilingMode, 0);
			else
				iteImageCombine(itmp, 0, 0, itmp->width, itmp->height, is, i-shiftX, j-shiftY, (ITEs7p8)((k1*k2*scale)*0x100), 0, VG_BLEND_ADDITIVE, tilingMode, 0);
		}
	}
	iteImageCombine(id, 0, 0, id->width, id->height, itmp, 0, 0, (ITEs7p8)0x100, 0, VG_BLEND_SRC, tilingMode, 1);

	ITEImage_AddDrawCount(is);
	ITEImage_AddDrawCount(id);

	ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);
}

VG_API_CALL void 
vgGaussianBlur(
	VGImage      dst, 
	VGImage      src,
	VGfloat      stdDeviationX,
	VGfloat      stdDeviationY,
	VGTilingMode tilingMode)
{
	ITEImage* dstImage    = (ITEImage*)dst;
	ITEImage* srcImage    = (ITEImage*)src;
	ITEuint   channelMask = 0;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if either dst or src is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!dst || !iteIsValidImage(context, dst) || 
	                 !src || !iteIsValidImage(context, src), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if either dst or src is currently a rendering target*/
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst) || iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if src and dst overlap
		¡V if stdDeviationX or stdDeviationY is less than or equal to 0 or greater than VG_MAX_GAUSSIAN_STD_DEVIATION
		¡V if tilingMode is not one of the values from the VGTilingMode enumeration */
	VG_RETURN_ERR_IF(iteImageIsOverlap(srcImage, dstImage),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(stdDeviationX  <= 0 || 
		             stdDeviationX  >  ITE_MAX_GAUSSIAN_STD_DEVIATION ||
		             stdDeviationY <= 0 ||
		             stdDeviationY >  ITE_MAX_GAUSSIAN_STD_DEVIATION, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!iteIsValidTilingMode(tilingMode),
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	// Ignore undefined bits
	channelMask = context->filterChannelMask & (VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA);

	// HW implement subsequence action
}


VG_API_CALL void 
vgLookup(
	VGImage        dst, 
	VGImage        src,
	const VGubyte* redLUT,
	const VGubyte* greenLUT,
	const VGubyte* blueLUT,
	const VGubyte* alphaLUT,
	VGboolean      outputLinear,
	VGboolean      outputPremultiplied)
{
	ITEImage *is, *id, *itmp;
	VG_GETCONTEXT(VG_NO_RETVAL);
	is = (ITEImage*)src;
	id = (ITEImage*)dst;

	/* VG_BAD_HANDLE_ERROR
		¡V if either dst or src is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!dst || !iteIsValidImage(context, dst) || 
	                 !src || !iteIsValidImage(context, src), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if either dst or src is currently a rendering target*/
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst) || 
		             iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if src and dst overlap
		¡V if any pointer parameter is NULL */
	VG_RETURN_ERR_IF(iteImageIsOverlap(is, id) ||
		             !redLUT || !greenLUT || !blueLUT || !alphaLUT,
					 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	iteSourceCopy(ITE_TRUE, id, 0, 0, id->width, id->height, is, 0, 0);	

	// Red
	itmp = iteCreateImage(
		HW_sL_8, 
		256, 
		1, 
		0, 
		VG_FALSE,
	    VG_TRUE);

	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height,iteHardwareGenObjectID());

#ifdef _WIN32
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = 256;
		
		mappedSysRam = ithMapVram(itmp->data, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, redLUT, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
#else
	memcpy(itmp->data, redLUT, 256);
#endif

	iteLookUpSourceCopy(ITE_TRUE, id, 0, 0, id->width, id->height, itmp, 0, 0,ITE_VG_CMD_LOOKUPMODE_R);				  

	ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);

	// Green
	itmp = iteCreateImage(
		HW_sL_8, 
		256, 
		1, 
		0, 
		VG_FALSE,
	    VG_TRUE);

	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height,iteHardwareGenObjectID());
	
#ifdef _WIN32
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = 256;

		mappedSysRam = ithMapVram(itmp->data, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, greenLUT, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
#else
	memcpy(itmp->data, greenLUT, 256);
#endif

	/*
	for( i=0; i<256; i++ )
	{
#ifdef _WIN32
		HOST_WriteBlockMemory((ITEuint32)&itmp->data[i], (ITEuint32)&greenLUT[i], 1);
#else
		itmp->data[i] = greenLUT[i];
#endif
	}
	*/

	iteLookUpSourceCopy(ITE_TRUE, id, 0, 0, id->width, id->height, itmp, 0, 0,ITE_VG_CMD_LOOKUPMODE_G);				  

	ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);


	// Blue
	itmp = iteCreateImage(
		HW_sL_8, 
		256, 
		1, 
		0, 
		VG_FALSE,
	    VG_TRUE);

	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height,iteHardwareGenObjectID());

#ifdef _WIN32
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = 256;

		mappedSysRam = ithMapVram(itmp->data, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, blueLUT, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
#else
	memcpy(itmp->data, blueLUT, 256);
#endif
	/*
	for( i=0; i<256; i++ )
	{
#ifdef _WIN32
		HOST_WriteBlockMemory((ITEuint32)&itmp->data[i], (ITEuint32)&blueLUT[i], 1);
#else
		itmp->data[i] = blueLUT[i];
#endif
	}
	*/

	iteLookUpSourceCopy(ITE_TRUE, id, 0, 0, id->width, id->height, itmp, 0, 0,ITE_VG_CMD_LOOKUPMODE_B);				  

	ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);


	// Alpha
	itmp = iteCreateImage(
		HW_sL_8, 
		256, 
		1, 
		0, 
		VG_FALSE,
	    VG_TRUE);

	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height,iteHardwareGenObjectID());

#ifdef _WIN32
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = 256;

		mappedSysRam = ithMapVram(itmp->data, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, alphaLUT, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
#else
	memcpy(itmp->data, alphaLUT, 256);
#endif
	/*
	for( i=0; i<256; i++ )
	{
#ifdef _WIN32
		HOST_WriteBlockMemory((ITEuint32)&itmp->data[i], (ITEuint32)&alphaLUT[i], 1);
#else
		itmp->data[i] = alphaLUT[i];
#endif
	}
	*/

	iteLookUpSourceCopy(ITE_TRUE, id, 0, 0, id->width, id->height, itmp, 0, 0,ITE_VG_CMD_LOOKUPMODE_A);				  

	ITEImage_AddDrawCount(is);
	ITEImage_AddDrawCount(id);

	ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);

}

VG_API_CALL void 
vgLookupSingle(
	VGImage        dst, 
	VGImage        src,
	const VGuint*  lookupTable,
	VGImageChannel sourceChannel,
	VGboolean      outputLinear,
	VGboolean      outputPremultiplied)
{
	//1 ITEint i;
	ITEImage *is, *id, *itmp;
	ITEuint  i = 0;
	ITEuint  lookupMode;  
	VG_GETCONTEXT(VG_NO_RETVAL);

	is = (ITEImage*)src;
	id = (ITEImage*)dst;
	
	/* VG_BAD_HANDLE_ERROR
		¡V if either dst or src is not a valid image handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!dst || !iteIsValidImage(context, dst) || 
	                 !src || !iteIsValidImage(context, src), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_IMAGE_IN_USE_ERROR
		¡V if either dst or src is currently a rendering target*/
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, dst) || 
		             iteIsCurrentRenderTarget(context, src), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		 - if src is in an RGB pixel format and sourceChannel is not one of VG_RED,
		   VG_GREEN, VG_BLUE or VG_ALPHA from the VGImageChannel enumeration */
	VG_RETURN_ERR_IF(   (ITEImage_IsRgbaImageFormat(is->vgformat) == ITE_TRUE)
	                 && !ITEImage_IsValidImageChannel(sourceChannel),
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if src and dst overlap
		¡V if any pointer parameter is NULL
		¡V if lookupTable is not properly aligned */
	VG_RETURN_ERR_IF(iteImageIsOverlap(is, id) ||
	                 !lookupTable || 
	                 !CheckAlignment(lookupTable, sizeof(VGuint)),
					 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	iteSourceCopy(ITE_TRUE, id, 0, 0, id->width, id->height, is, 0, 0);				  

	if(sourceChannel & VG_RED)
	{
		lookupMode = ITE_VG_CMD_LOOKUPMODE_R;
	}
	else if(sourceChannel & VG_GREEN)
	{
		lookupMode = ITE_VG_CMD_LOOKUPMODE_G;
	}
	else if(sourceChannel & VG_BLUE)
	{
		lookupMode = ITE_VG_CMD_LOOKUPMODE_B;
	}
	else // (sourceChannel & VG_ALPHA)
	{
		lookupMode = ITE_VG_CMD_LOOKUPMODE_A;
	}

	itmp = iteCreateImage(
		HW_sL_8, 
		256, 
		1, 
		0, 
		VG_FALSE,
		VG_TRUE);

	itmp->data = (ITEuint8*)vgMemalign(8,itmp->pitch * itmp->height,iteHardwareGenObjectID());

#ifdef _WIN32
	{
		ITEuint8* mappedSysRam = NULL;
		ITEuint32 allocSize    = 256;

		mappedSysRam = ithMapVram(itmp->data, allocSize, ITH_VRAM_WRITE);
		VG_Memcpy(mappedSysRam, lookupTable, allocSize);
		ithFlushDCacheRange(mappedSysRam, allocSize);
		ithUnmapVram(mappedSysRam, allocSize);
	}
#else
	memcpy(itmp->data, lookupTable, 256);
#endif
	/*
	for( i=0; i<256; i++ )
	{
#ifdef _WIN32
		HOST_WriteBlockMemory((ITEuint32)&itmp->data[i], (ITEuint32)&lookupTable[i], 1);
#else
		itmp->data[i] = lookupTable[i];
#endif
	}
	*/

	iteLookUpSourceCopy(ITE_TRUE, id, 0, 0, id->width, id->height, itmp, 0, 0, lookupMode);	

	ITEImage_AddDrawCount(is);
	ITEImage_AddDrawCount(id);

	ITE_DELETEOBJ(ITEImage, (ITEImage*)itmp);
}
