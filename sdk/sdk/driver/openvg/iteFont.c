/*------------------------------------------------------------------------
 *
 * OpenVG 1.1 Reference Implementation
 * -----------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and /or associated documentation files
 * (the "Materials "), to deal in the Materials without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Materials,
 * and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Materials. 
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR
 * THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *//**
 * \file
 * \brief	Implementation of Font class.
 * \note	
 *//*-------------------------------------------------------------------*/

#include "openvg.h"
#include "iteContext.h"
#include "iteFont.h"
#include "iteutility.h"

#define _ITEM_T ITEFont*
#define _ARRAY_T ITEFontArray
#define _FUNC_T iteFontArray
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

ITEfloat iteValidInputFloat(VGfloat f);

void ITEFont_ctor(ITEFont* font)
{
	font->glyphCapacityHint = 0;
	font->referenceCount    = 0;
	ITE_INITOBJ(ITEGlyphArray, font->glyphs);
}

void ITEFont_dtor(ITEFont* font)
{
	ITE_DEINITOBJ(ITEGlyphArray, font->glyphs);
}

static VGint
ITEFont_AddReference(
	ITEFont* font)
{
	return ++font->referenceCount;
}

static VGint
ITEFont_RemoveReference(
	ITEFont* font)
{
	font->referenceCount--;
	ITE_ASSERT(font->referenceCount >= 0);
	return font->referenceCount;
}

static ITEGlyph*
iteFindGlyphInFont(
	ITEFont* pFont,
	ITEint   glyphIndex)
{
	ITEint32  i      = 0;
	ITEGlyph* pGlyph = NULL;

	for ( i = 0; i < pFont->glyphs.size; i++ )
	{
		ITEGlyph* pCurrGlyph = *iteGlyphArrayAtP_Ex(&pFont->glyphs, i);
		if ( pCurrGlyph && (pCurrGlyph->index == glyphIndex) )
		{
			pGlyph = pCurrGlyph;
			break;
		}
	}

	return pGlyph;
}

static ITEboolean
iteSetGlyphToPath(
	ITEFont*  font, 
	VGuint    glyphIndex, 
	ITEPath*  path, 
	VGboolean isHinted, 
	VGfloat   glyphOrigin[2], 
	VGfloat   escapement[2])
{
	ITEGlyph* pGlyph = NULL;
	ITEGlyph* glyph  = NULL;
	VG_GETCONTEXT(ITE_FALSE);

	pGlyph = iteFindGlyphInFont(font, glyphIndex);
	if ( pGlyph == NULL )
	{
		ITE_NEWOBJ(ITEGlyph, pGlyph);
		iteGlyphArrayPushBack(&font->glyphs, pGlyph);
	}
	else
	{
		ITEGlyph_Clear(pGlyph);
	}

	VG_RETURN_ERR_IF(!pGlyph, 
		             VG_OUT_OF_MEMORY_ERROR, ITE_FALSE);

	pGlyph->index       = glyphIndex;
	pGlyph->path        = path;
	pGlyph->image       = NULL;
	pGlyph->isHinted    = isHinted;
	pGlyph->origin.x    = glyphOrigin[0];
	pGlyph->origin.y    = glyphOrigin[1];
	pGlyph->escapment.x = escapement[0];
	pGlyph->escapment.y = escapement[1];

	if ( path != VG_INVALID_HANDLE )
	{
		ITEPath_AddReference(path);
	}

	return ITE_TRUE;	
}

static ITEboolean
iteSetGlyphToImage(
	ITEFont*  font,
	VGuint    glyphIndex,
	ITEImage* image,
	VGfloat   glyphOrigin[2],
	VGfloat   escapment[2])
{
	ITEGlyph* pGlyph = NULL;
	VG_GETCONTEXT(ITE_FALSE);

	pGlyph = iteFindGlyphInFont(font, glyphIndex);
	if ( pGlyph == NULL )
	{
		ITE_NEWOBJ(ITEGlyph, pGlyph);
		iteGlyphArrayPushBack(&font->glyphs, pGlyph);
	}
	else
	{
		ITEGlyph_Clear(pGlyph);
	}

	VG_RETURN_ERR_IF(!pGlyph, 
		             VG_OUT_OF_MEMORY_ERROR, ITE_FALSE);

	pGlyph->index       = glyphIndex;
	pGlyph->path        = NULL;
	pGlyph->image       = image;
	pGlyph->isHinted    = ITE_FALSE;
	pGlyph->origin.x    = glyphOrigin[0];
	pGlyph->origin.y    = glyphOrigin[1];
	pGlyph->escapment.x = escapment[0];
	pGlyph->escapment.y = escapment[1];

	ITEImage_AddReference(image);

	return ITE_TRUE;	
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

VG_API_CALL VGFont vgCreateFont(VGint glyphCapacityHint)
{
	ITEFont* font = NULL;
	VG_GETCONTEXT(VG_INVALID_HANDLE);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if glyphCapacityHint is negative */
	VG_RETURN_ERR_IF(glyphCapacityHint < 0, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);

	ITE_NEWOBJ(ITEFont, font);
	VG_RETURN_ERR_IF(!font, VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	font->glyphCapacityHint = glyphCapacityHint;
	/* Add to resource list */
	if ( !iteFontArrayPushBack(&context->fonts, font) )
	{
		// Add ITEImage to image array fail, free image resource
		ITE_DELETEOBJ(ITEFont, font);
		VG_RETURN_ERR(VG_OUT_OF_MEMORY_ERROR, VG_INVALID_HANDLE);
	}
	font->referenceCount++;

	if ( font->glyphCapacityHint > 0 )
	{
		iteGlyphArrayReserve(&font->glyphs, font->glyphCapacityHint);
	}
	
	/*
	RI_IF_ERROR(glyphCapacityHint < 0, VG_ILLEGAL_ARGUMENT_ERROR, VG_INVALID_HANDLE);

	Font* font = NULL;
	try
	{
		font = RI_NEW(Font, (glyphCapacityHint));	//throws bad_alloc
		RI_ASSERT(font);
		context->m_fontManager->addResource(font, context);	//throws bad_alloc
		RI_RETURN((VGFont)font);
	}
	catch(std::bad_alloc)
	{
		RI_DELETE(font);
		context->setError(VG_OUT_OF_MEMORY_ERROR);
		RI_RETURN(VG_INVALID_HANDLE);
	}
	*/

	VG_RETURN ((VGFont)font);
}


/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

VG_API_CALL void vgDestroyFont(VGFont font)
{
	ITEint index = -1;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if font is not a valid font handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!font, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	index = iteFontArrayFind(&context->fonts, (ITEFont*)font);
	VG_RETURN_ERR_IF(index == -1, VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* Delete object and remove resource */
	iteFontArrayRemoveAt(&context->fonts, index);

	/* Delete object if noone reference to it*/
	if ( ITEFont_RemoveReference((ITEFont*)font) == 0 )
	{
		ITE_DELETEOBJ(ITEFont, (ITEFont*)font);
	}

	/*
	RI_IF_ERROR(!context->isValidFont(font), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid font handle

	context->m_fontManager->removeResource((Font*)font);
	*/

	VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

VG_API_CALL void 
vgSetGlyphToPath(
	VGFont    		font, 
	VGuint    		glyphIndex, 
	VGPath    		path, 
	VGboolean 		isHinted, 
	const VGfloat	glyphOrigin[2], 
	const VGfloat   escapement[2])
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if the pointer to glyphOrigin or escapement is NULL or is not properly aligned */
	VG_RETURN_ERR_IF(!glyphOrigin || !CheckAlignment(glyphOrigin, sizeof(VGfloat)) ||
	                 !escapement  || !CheckAlignment(escapement, sizeof(VGfloat)), 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if font is not a valid font handle, or is not shared with the current context
		¡V if path is not a valid font handle or VG_INVALID_HANDLE, or is not shared
		with the current context */
	VG_RETURN_ERR_IF(!font || !iteIsValidFont(context, font), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF((path != VG_INVALID_HANDLE) && !iteIsValidPath(context, path), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	iteSetGlyphToPath((ITEFont*)font, glyphIndex, (ITEPath*)path, isHinted, (VGfloat*)glyphOrigin, (VGfloat*)escapement);

	/*
	RI_IF_ERROR(!context->isValidFont(font), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid font handle
	RI_IF_ERROR(path != VG_INVALID_HANDLE && !context->isValidPath(path), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid path handle
    RI_IF_ERROR(!glyphOrigin || !escapement || !isAligned(glyphOrigin,sizeof(VGfloat)) || !isAligned(escapement,sizeof(VGfloat)), VG_ILLEGAL_ARGUMENT_ERROR, RI_NO_RETVAL);
	Font* f = (Font*)font;

	try
	{
        f->setGlyphToPath(glyphIndex, path, isHinted ? true : false, Vector2(inputFloat(glyphOrigin[0]), inputFloat(glyphOrigin[1])), Vector2(inputFloat(escapement[0]), inputFloat(escapement[1])));
	}
	catch(std::bad_alloc)
	{
		context->setError(VG_OUT_OF_MEMORY_ERROR);
	}
	*/

	VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

VG_API_CALL void 
vgSetGlyphToImage(
	VGFont			font, 
	VGuint			glyphIndex, 
	VGImage			image, 
	const VGfloat	glyphOrigin[2], 
	const VGfloat	escapement[2])
{
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if the pointer to glyphOrigin or escapement is NULL or is not properly aligned */
	VG_RETURN_ERR_IF(!glyphOrigin || !CheckAlignment(glyphOrigin, sizeof(VGfloat)) ||
	                 !escapement  || !CheckAlignment(escapement, sizeof(VGfloat)), 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if font is not a valid font handle, or is not shared with the current context
		¡V if image is not a valid font handle or VG_INVALID_HANDLE, or is not
		shared with the current context */
	VG_RETURN_ERR_IF(!font || !iteIsValidFont(context, font), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!image || !iteIsValidImage(context, image), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);
	
	/* VG_IMAGE_IN_USE_ERROR
		¡V if image is currently a rendering target */
	VG_RETURN_ERR_IF(iteIsCurrentRenderTarget(context, image), 
	                 VG_IMAGE_IN_USE_ERROR, VG_NO_RETVAL)

	iteSetGlyphToImage((ITEFont*)font, glyphIndex, (ITEImage*)image, (VGfloat*)glyphOrigin, (VGfloat*)escapement);

	/*
	RI_IF_ERROR(!context->isValidFont(font), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid font handle
    if(image != VG_INVALID_HANDLE)
    {
        RI_IF_ERROR(!context->isValidImage(image), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid image handle
        RI_IF_ERROR(eglvgIsInUse((Image*)image), VG_IMAGE_IN_USE_ERROR, RI_NO_RETVAL); //image in use
    }
    RI_IF_ERROR(!glyphOrigin || !escapement || !isAligned(glyphOrigin,sizeof(VGfloat)) || !isAligned(escapement,sizeof(VGfloat)), VG_ILLEGAL_ARGUMENT_ERROR, RI_NO_RETVAL);
	Font* f = (Font*)font;

	try
	{
        f->setGlyphToImage(glyphIndex, image, Vector2(inputFloat(glyphOrigin[0]), inputFloat(glyphOrigin[1])), Vector2(inputFloat(escapement[0]), inputFloat(escapement[1])));
	}
	catch(std::bad_alloc)
	{
		context->setError(VG_OUT_OF_MEMORY_ERROR);
	}
	*/

	VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

VG_API_CALL void vgClearGlyph(VGFont font, VGuint glyphIndex)
{
	ITEGlyph* pGlyph = NULL;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if font is not a valid font handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!font || !iteIsValidFont(context, font), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if glyphIndex is not defined for the font */
	pGlyph = iteFindGlyphInFont((ITEFont*)font, glyphIndex);
	VG_RETURN_ERR_IF(pGlyph == NULL, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	ITEGlyph_Clear(pGlyph);

	/*
	RI_IF_ERROR(!context->isValidFont(font), VG_BAD_HANDLE_ERROR, RI_NO_RETVAL);	//invalid font handle
	Font* f = (Font*)font;
    Font::Glyph* g = f->findGlyph(glyphIndex);
    RI_IF_ERROR(!g, VG_ILLEGAL_ARGUMENT_ERROR, RI_NO_RETVAL);   //glyphIndex not defined

	f->clearGlyph(g);
	*/

	VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

VG_API_CALL void 
vgDrawGlyph(
	VGFont     font, 
	VGuint     glyphIndex, 
	VGbitfield paintModes, 
	VGboolean  allowAutoHinting)
{
	ITEGlyph*    pGlyph = NULL;
	ITEMatrix3x3 glyphUserToSurfaceMatrix;
	ITEMatrix3x3 localTransformMatrix, tempMatrix;
	ITEVector2   deltaVector;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if font is not a valid font handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!font || !iteIsValidFont(context, font), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if glyphIndex has not been defined for a given font object
		¡V if paintModes is not a valid bitwise OR of values from the VGPaintMode enumeration, or 0 */
	pGlyph = iteFindGlyphInFont((ITEFont*)font, glyphIndex);
	VG_RETURN_ERR_IF(pGlyph == NULL, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(!iteIsValidPaintModeBit(paintModes), 
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	glyphUserToSurfaceMatrix = context->glyphTransform;
	deltaVector.x = context->glyphOrigin[0] - pGlyph->origin.x;
	deltaVector.y = context->glyphOrigin[1] - pGlyph->origin.y;
	IDMAT(localTransformMatrix);
	TRANSLATEMATL(localTransformMatrix, deltaVector.x, deltaVector.y);
	//MULMATS(glyphUserToSurfaceMatrix, localTransformMatrix);
	MULMATMAT(glyphUserToSurfaceMatrix, localTransformMatrix, tempMatrix);
	glyphUserToSurfaceMatrix = tempMatrix;
	/* force affinity */
	SETMAT(glyphUserToSurfaceMatrix, 
		glyphUserToSurfaceMatrix.m[0][0], glyphUserToSurfaceMatrix.m[0][1], glyphUserToSurfaceMatrix.m[0][2],
		glyphUserToSurfaceMatrix.m[1][0], glyphUserToSurfaceMatrix.m[1][1], glyphUserToSurfaceMatrix.m[1][2],
		0, 0, 1);

	if ( pGlyph->image != VG_INVALID_HANDLE )
	{
		iteDrawImage(pGlyph->image, &glyphUserToSurfaceMatrix);
	}
	else if ( pGlyph->path != VG_INVALID_HANDLE )
	{
		iteDrawPath(pGlyph->path, paintModes, &glyphUserToSurfaceMatrix);
	}

	context->glyphOrigin[0] += pGlyph->escapment.x;
	context->glyphOrigin[1] += pGlyph->escapment.y;
	context->inputGlyphOrigin[0] = pGlyph->escapment.x;
	context->inputGlyphOrigin[1] = pGlyph->escapment.y;

	VG_RETURN(VG_NO_RETVAL);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

VG_API_CALL void 
vgDrawGlyphs(
	VGFont			font, 
	VGint			glyphCount, 
	const VGuint*	glyphIndices, 
	const VGfloat*	adjustments_x, 
	const VGfloat*	adjustments_y, 
	VGbitfield		paintModes, 
	VGboolean		allowAutoHinting)
{
	int i = 0;
	VG_GETCONTEXT(VG_NO_RETVAL);

	/* VG_BAD_HANDLE_ERROR
		¡V if font is not a valid font handle, or is not shared with the current context */
	VG_RETURN_ERR_IF(!font || !iteIsValidFont(context, font), 
	                 VG_BAD_HANDLE_ERROR, VG_NO_RETVAL);

	/* VG_ILLEGAL_ARGUMENT_ERROR
		¡V if glyphCount is zero or a negative value
		¡V if the pointer to the glyphIndices array is NULL or is not properly aligned
		¡V if a pointer to either of the adjustments_x or adjustments_y arrays are non-NULL and are not properly aligned
		¡V if any of the glyphIndices has not been defined in a given font object
		¡V if paintModes is not a valid bitwise OR of values from the VGPaintMode enumeration, or 0 */
	VG_RETURN_ERR_IF(!glyphIndices || !CheckAlignment(glyphIndices, sizeof(VGuint)) || glyphCount <= 0, 
	                 VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);	
	VG_RETURN_ERR_IF((adjustments_x && !CheckAlignment(adjustments_x, sizeof(VGfloat))) || 
		             (adjustments_y && !CheckAlignment(adjustments_y, sizeof(VGfloat))), 
		             VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	VG_RETURN_ERR_IF(paintModes && !iteIsValidPaintModeBit(paintModes),
				     VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);

	for ( i = 0; i < glyphCount; i++ )
	{
		ITEGlyph* pGlyph = iteFindGlyphInFont((ITEFont*)font, glyphIndices[i]);
		VG_RETURN_ERR_IF(pGlyph == NULL, 
				         VG_ILLEGAL_ARGUMENT_ERROR, VG_NO_RETVAL);
	}

	/* Draw glyph */
	for ( i = 0; i < glyphCount; i++ )
	{
		ITEMatrix3x3 glyphUserToSurfaceMatrix;
		ITEMatrix3x3 localTransformMatrix, tempMatrix;
		ITEVector2   deltaVector;
		ITEGlyph*    glyph = iteFindGlyphInFont((ITEFont*)font, glyphIndices[i]);

		if ( paintModes )
		{
			glyphUserToSurfaceMatrix = context->glyphTransform;
			deltaVector.x = context->glyphOrigin[0] - glyph->origin.x;
			deltaVector.y = context->glyphOrigin[1] - glyph->origin.y;
			IDMAT(localTransformMatrix);
			TRANSLATEMATL(localTransformMatrix, deltaVector.x, deltaVector.y);
			//MULMATS(glyphUserToSurfaceMatrix, localTransformMatrix);
			MULMATMAT(glyphUserToSurfaceMatrix, localTransformMatrix, tempMatrix);
			glyphUserToSurfaceMatrix = tempMatrix;
			/* force affinity */
			SETMAT(glyphUserToSurfaceMatrix, 
				glyphUserToSurfaceMatrix.m[0][0], glyphUserToSurfaceMatrix.m[0][1], glyphUserToSurfaceMatrix.m[0][2],
				glyphUserToSurfaceMatrix.m[1][0], glyphUserToSurfaceMatrix.m[1][1], glyphUserToSurfaceMatrix.m[1][2],
				0, 0, 1);
		}

		if ( glyph->image != VG_INVALID_HANDLE )
		{
			iteDrawImage(glyph->image, &glyphUserToSurfaceMatrix);
		}
		else if ( glyph->path != VG_INVALID_HANDLE )
		{
			iteDrawPath(glyph->path, paintModes, &glyphUserToSurfaceMatrix);
		}

		context->glyphOrigin[0] += glyph->escapment.x;
		context->glyphOrigin[1] += glyph->escapment.y;
		context->inputGlyphOrigin[0] = glyph->escapment.x;
		context->inputGlyphOrigin[1] = glyph->escapment.y;

		if( adjustments_x )
		{
			context->glyphOrigin[0] += iteValidInputFloat(adjustments_x[i]);
		}
        if( adjustments_y )
        {
			context->glyphOrigin[1] += iteValidInputFloat(adjustments_x[i]);
        }
	}

	VG_RETURN(VG_NO_RETVAL);
}
