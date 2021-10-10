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

#ifndef __ITEIMAGE_H
#define __ITEIMAGE_H

//#include "iteDefs.h"
#include "iteVectors.h"
#include "iteHardware.h"
//#include "iteContext.h"

#define HW_DEF_RGBA_FFFF	15

typedef struct _ITEImage
{
	ITE_VG_OBJ_TYPE     objType;
	ITEuint32           objectID;
	ITEuint8*			data;
	ITEint				width;
	ITEint				height;
	ITEint				pitch;
	VGImageFormat		vgformat;
	VGbitfield			allowedQuality;
	ITEint				offsetX;
	ITEint				offsetY;

	struct _ITEImage*	parent;
	ITEint				inUseCount;
	ITEint				referenceCount;
	ITEint              drawCount;
	ITEint              deleteCount;
} ITEImage;

// Image member function
void ITEImage_ctor(ITEImage *i);
void ITEImage_dtor(ITEImage *i);
ITEint ITEImage_AddReference(ITEImage* image);
ITEint ITEImage_RemoveReference(ITEImage* image);
ITEint ITEImage_AddInUse(ITEImage* image);
ITEint ITEImage_RemoveInUse(ITEImage* image);
ITEint ITEImage_AddDrawCount(ITEImage* image);
ITEint ITEImage_RemoveDrawCount(ITEImage* image);
ITEint ITEImage_AddDeleteCount(ITEImage* image);
ITEint ITEImage_RemoveDeleteCount(ITEImage* image);

// Image API
ITEint iteIsValidImageFormat(VGImageFormat format);
ITEboolean ITEImage_IsRgbImageFormat(VGImageFormat imageFormat);
ITEboolean ITEImage_IsRgbaImageFormat(VGImageFormat imageFormat);
ITEboolean ITEImage_IsPreMultipliedFormat(VGImageFormat imageFormat);
ITEboolean ITEImage_IsSrgbFormat(VGImageFormat imageFormat);
ITEboolean ITEImage_IsLrgbFormat(VGImageFormat imageFormat);

#define _ITEM_T ITEImage*
#define _ARRAY_T ITEImageArray
#define _FUNC_T iteImageArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

ITEImage* 
iteCreateImage(
	VGImageFormat format,
	VGint         width, 
	VGint         height,
	VGbitfield    allowedQuality,
	VGboolean     ownsData,
	VGboolean     forceAlign64bit);

void
iteDestroyImage(
	ITEImage* image);

void 
iteSetImage(
	ITEboolean	useContext,
	ITEImage*	im, 
	ITEint		x, 
	ITEint		y, 
	ITEint		width, 
	ITEint		height, 
	ITEColor	color);

void
iteSourceCopy(
	ITEboolean useContext,
	ITEImage* dst, 
	ITEint dstX, 
	ITEint dstY, 
	ITEint dstWidth, 
	ITEint dstHeight, 
	ITEImage* src, 
	ITEint srcX, 
	ITEint srcY);

void iteAlphaBlend(ITEImage *dst, ITEint dstX, ITEint dstY, ITEint dstWidth, ITEint dstHeight, 
				   ITEImage *src, ITEint srcX, ITEint srcY, HWBlendMode blendMode);

void iteStretchSrcCopy(ITEImage *dst, ITEint dstX, ITEint dstY, ITEint dstWidth, ITEint dstHeight, 
				   ITEImage *src, ITEint srcX, ITEint srcY, ITEint srcWidth, ITEint srcHeight);
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
	ITEboolean		colorclip);

void iteDrawImage(ITEImage* pImage, ITEMatrix3x3* userToSurfaceMatrix);

#endif /* __ITEIMAGE_H */

