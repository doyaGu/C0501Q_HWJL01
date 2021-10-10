/*
 * Copyright (c) 2011 ITE, Awin Huang
 *
 */
 
#ifndef __ITEGLYPH_H
#define __ITEGLYPH_H

#include "iteDefs.h"

/* ITEGlyph */
typedef struct ITEGlyph
{
	ITEint     index; // Index in font->glyphs
	ITEPath*   path;
	ITEImage*  image;
	ITEboolean isHinted;
	ITEVector2 origin;
	ITEVector2 escapment;
}ITEGlyph;

void ITEGlyph_ctor(ITEGlyph *glyph);
void ITEGlyph_dtor(ITEGlyph *glyph);
void ITEGlyph_Clear(ITEGlyph* glyph);


/* Pointer-to-glyph array */
#define _ITEM_T ITEGlyph*
#define _ARRAY_T ITEGlyphArray
#define _FUNC_T iteGlyphArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#endif /* __ITEGLYPH_H */

