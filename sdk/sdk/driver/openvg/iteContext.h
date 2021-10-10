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

#ifndef __ITECONTEXT_H
#define __ITECONTEXT_H

#include "iteDefs.h"
#include "iteVectors.h"
#include "iteArrays.h"
#include "itePath.h"
#include "itePaint.h"
#include "iteImage.h"
#include "iteFont.h"
#include "iteHardware.h"

/* Array for Path command array */
#define _ITEM_T ITEPathCommandArray*
#define _ARRAY_T ITEPathCommandPool
#define _FUNC_T itePathCommandPool
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#ifdef _WIN32
/* Array for Path command array */
#define _ITEM_T ITEuint8*
#define _ARRAY_T ITEPathCommandPoolHW
#define _FUNC_T itePathCommandPoolHW
#define _ARRAY_DECLARE
#include "iteArrayBase.h"
#endif

/*------------------------------------------------
 * VGContext object
 *------------------------------------------------*/
typedef struct ITEUpdateFlag
{
  VGboolean			modeFlag;
  VGboolean			enableFlag;
  VGboolean			colorFlag;
  VGboolean			fillMatrixFlag;
  VGboolean			strokeMatrixFlag;
  VGboolean			imageMatrixFlag;
  VGboolean			paintBaseFlag;
  VGboolean			paintPatternFlag;
  VGboolean			linearGradientFlag;
  VGboolean			radialGradientFlag;
  VGboolean			textureFlag;
  VGboolean			maskFlag;
  VGboolean         scissorRectFlag;
  VGboolean			coverageFlag;
  VGboolean         surfaceFlag;
  VGboolean			destinationFlag;
  VGboolean         colorTransformFlag;
} ITEUpdateFlag;

typedef struct _ITESurface
{
    void*		OSWindowContext;
	int			renderBuffer;		//EGL_BACK_BUFFER or EGL_SINGLE_BUFFER
	ITEImage*	colorImage;
	ITEImage*	maskImage;
	ITEImage*	validImageA;	
	ITEImage*	validImageB;
	ITEImage*	coverageImageA;
	ITEImage*	coverageImageB;
	ITEuint8    coverageIndex;

	/* For engine temporary use 
	   2-pass and 3-pass render */
	ITEImage*   ImagePatternMergeBase;
	ITEImage*	ImagePatternMerge; ///< The mergerd image from image + pattern

	/* For VG_IMAGE_QUALITY_BETTER */
	ITEImage*	image4XBase;
	ITEImage*	image4X;
	ITEImage*	imageQuarter;
} ITESurface;

typedef enum
{
  ITE_RESOURCE_INVALID   = 0,
  ITE_RESOURCE_PATH      = 1,
  ITE_RESOURCE_PAINT     = 2,
  ITE_RESOURCE_IMAGE     = 3,
  ITE_RESOURCE_FONT      = 4,
} ITEResourceType;

typedef struct _VGContext
{  
	/* GetString info */
	char vendor[256];
	char renderer[256];
	char version[256];
	char extensions[256];

	/* Mode settings */
	VGMatrixMode        matrixMode;
	VGFillRule          fillRule;
	VGImageQuality      imageQuality;
	VGRenderingQuality  renderingQuality;
	VGBlendMode         blendMode;
	VGImageMode         imageMode;
	VGTilingMode		tilingMode;
	//VGPaintMode			paintMode;

	/* Scissor rectangles */
	ITERectArray		scissor;
	VGboolean			scissoring;
	ITEImage			scissorImage;
	VGboolean			masking;

	/* Stroke parameters */
	ITEfloat           strokeLineWidth;
	VGCapStyle         strokeCapStyle;
	VGJoinStyle        strokeJoinStyle;
	ITEfloat           strokeMiterLimit_input;	// for vgSet()/vgGet()
	ITEfloat           strokeMiterLimit;
	ITEFloatArray      strokeDashPattern;
	ITEfloat           strokeDashPhase;
	VGboolean          strokeDashPhaseReset;

	/* Edge fill color for vgConvolve and pattern paint */
	ITEFloatColor		tileFillColor_input;
	ITEFloatColor		tileFillColor;

	/* Color for vgClear */
	ITEFloatColor		clearColor_input;
	ITEColor			clearColor;

	/* Color components layout inside pixel */
	VGPixelLayout		pixelLayout;

	/* Source format for image filters */
	VGboolean			filterFormatLinear;
	VGboolean			filterFormatPremultiplied;
	VGbitfield			filterChannelMask;

	/* Matrices */
	ITEMatrix3x3		pathTransform;
	ITEMatrix3x3		imageTransform;
	ITEMatrix3x3		fillTransform;
	ITEMatrix3x3		strokeTransform;
	ITEMatrix3x3        glyphTransform;

	/* Paints */
	ITEPaint*			fillPaint;
	ITEPaint*			strokePaint;
	ITEPaint			defaultPaint;

	/* ColorTransform */
	VGboolean			enColorTransform;
	ITEfloat			colorTransform[8];

	VGErrorCode			error;

	/* Resources */
	ITEPathArray		paths;
	ITEPaintArray		paints;
	ITEImageArray		images;
	ITEFontArray        fonts;
	ITEImageArray		maskLayers;

	/* Surface */
	ITESurface*			surface;

	/* Update flag */
	ITEUpdateFlag		updateFlag;  

	/* Print number */
	char 				printnum;

	/* Glyph */
	VGfloat				glyphOrigin_input[2];
	VGfloat				glyphOrigin[2];
	VGfloat				inputGlyphOrigin[2];

	/* Hardware */
	ITEHardwareRegister hardware;
} VGContext;

void iteSetError(VGContext *c, VGErrorCode e);
ITEint iteIsCurrentRenderTarget(VGContext *c, VGImage image);
ITEint iteIsValidPath(VGContext *c, VGHandle h);
ITEint iteIsValidPaint(VGContext *c, VGHandle h);
ITEint iteIsValidImage(VGContext *c, VGHandle h);
ITEint iteIsValidFont(VGContext *c, VGHandle h);
ITEint iteIsValidMaskLayer(VGContext *c, VGHandle h);
ITEResourceType iteGetResourceType(VGContext *c, VGHandle h);
VGContext* iteGetContext();
VGboolean iteCreateContext();
void iteDestroyContext();
ITESurface* iteContextAllocSurface(void* OSWindowContext, int renderBuffer, VGImageFormat surfaceFormat, int windowWidth, int windowHeight, ITEboolean allocMask);

/*----------------------------------------------------
 * TODO: Add mutex locking/unlocking to these macros
 * to assure sequentiallity in multithreading.
 *----------------------------------------------------*/

#define VG_NO_RETVAL

#define VG_GETCONTEXT(RETVAL) \
  VGContext *context = iteGetContext(); \
  if (!context) return RETVAL;
  
#define VG_RETURN(RETVAL) \
  { return RETVAL; }

#define VG_RETURN_ERR(ERRORCODE, RETVAL) \
  { iteSetError(context,ERRORCODE); return RETVAL; }

#define VG_RETURN_ERR_IF(COND, ERRORCODE, RETVAL) \
  { if (COND) {iteSetError(context,ERRORCODE); return RETVAL;} }

/*-----------------------------------------------------------
 * Same macros but no mutex handling - used by sub-functions
 *-----------------------------------------------------------*/

#define ITE_NO_RETVAL

#define ITE_GETCONTEXT(RETVAL) \
  VGContext *context = iteGetContext(); \
  if (!context) return RETVAL;
  
#define ITE_RETURN(RETVAL) \
  { return RETVAL; }

#define ITE_RETURN_ERR(ERRORCODE, RETVAL) \
  { iteSetError(context,ERRORCODE); return RETVAL; }

#define ITE_RETURN_ERR_IF(COND, ERRORCODE, RETVAL) \
  { if (COND) {iteSetError(context,ERRORCODE); return RETVAL;} }


#endif /* __ITECONTEXT_H */
