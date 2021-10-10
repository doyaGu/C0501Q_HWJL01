/*
 * Copyright (c) 2011 ITE, Awin Huang
 *
 */

#define VG_API_EXPORT
#include "openvg.h"
#include "iteContext.h"
#include "itePath.h"
#include "iteGlyph.h"

/* Pointer-to-glyph array */
#define _ITEM_T ITEGlyph*
#define _ARRAY_T ITEGlyphArray
#define _FUNC_T iteGlyphArray
#define _ARRAY_DEFINE
#include "iteArrayBase.h"

void ITEGlyph_ctor(ITEGlyph *glyph)
{
	glyph->index       = -1;
	glyph->path        = NULL;
	glyph->image       = NULL;
	glyph->isHinted    = ITE_FALSE;
	glyph->origin.x    = 0;
	glyph->origin.y    = 0;
	glyph->escapment.x = 0;
	glyph->escapment.y = 0;
}

void ITEGlyph_dtor(ITEGlyph *glyph)
{
}

void 
ITEGlyph_Clear(
	ITEGlyph* glyph)
{
	ITE_ASSERT(glyph != NULL);

	if ( glyph->path )
	{
		if ( ITEPath_RemoveReference(glyph->path) == 0 )
		{
			ITE_DELETEOBJ(ITEPath, glyph->path);
		}
	}

	if ( glyph->image )
	{
		if ( ITEImage_RemoveReference(glyph->image) == 0 )
		{
			ITE_DELETEOBJ(ITEImage, glyph->image);
		}
	}

	ITEGlyph_ctor(glyph);
}

