#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_MANAGER_H
#include FT_BITMAP_H
#include FT_INTERNAL_OBJECTS_H
#include FT_OUTLINE_H

#include <assert.h>
#include <malloc.h>
#include <wchar.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "itu_private.h"

#define MAX_SBIT_CACHE 512

/* this simple record is used to model a given `installed' face */
typedef struct
{
    char*           filepathname;
    char*           filename;
    uint8_t*        array;
    int             array_size;
    int             face_index;
    int             cmap_index;
    int             num_indices;
    FTC_ScalerRec   scaler;
    FT_ULong        load_flags;
    FT_Render_Mode  render_mode;
    ITUGlyphFormat  format;
    FT_Face         face;
} TFont;

struct
{
    FT_Library     library;             /* the FreeType library            */
    FTC_Manager    cache_manager;       /* the cache manager               */
    FTC_ImageCache image_cache;         /* the glyph image cache           */
    FTC_SBitCache  sbits_cache;         /* the glyph small bitmaps cache   */
    FTC_CMapCache  cmap_cache;          /* the charmap cache..             */
    TFont          fonts[ITU_FREETYPE_MAX_FONTS];    /* installed fonts */
    TFont*         current_font;        /* selected font */
    TFont*         default_font;        /* default font */
    FT_Bitmap      bitmap;              /* used as bitmap conversion buffer */
    unsigned int    style;
    int				bold_size;
} ft;

static FT_Error
Bitmap_Convert_GRAY4(FT_Library        library,
                     const FT_Bitmap  *source,
                     FT_Bitmap        *target)
{
    FT_Error   error = FT_Err_Ok;
    FT_Memory  memory;

    if ( !library )
        return FT_Err_Invalid_Library_Handle;

    memory = library->memory;

    switch (source->pixel_mode)
    {
    case FT_PIXEL_MODE_GRAY:
    case FT_PIXEL_MODE_GRAY2:
        {
            FT_Long  old_size;

            old_size = target->rows * target->pitch;
            if ( old_size < 0 )
                old_size = -old_size;

            target->pixel_mode = FT_PIXEL_MODE_GRAY4;
            target->rows       = source->rows;
            target->width      = source->width;

            target->pitch = ITH_ALIGN_UP(source->width, 2) / 2;

            if ( target->rows * target->pitch > (unsigned int)old_size &&
                FT_QREALLOC( target->buffer,
                old_size, target->rows * target->pitch ) )
                return error;
        }
        break;

    default:
        error = FT_Err_Invalid_Argument;
    }

    switch (source->pixel_mode)
    {
    case FT_PIXEL_MODE_GRAY:
        {
            FT_Byte*  s       = source->buffer;
            FT_Byte*  t       = target->buffer;
            FT_Int    i;

            target->num_grays = 16;

            for ( i = source->rows; i > 0; i-- )
            {
                FT_Byte*  ss = s;
                FT_Byte*  tt = t;
                FT_Int    j;

                /* get the full bytes */
                for ( j = source->width >> 1; j > 0; j-- )
                {
                    tt[0] = (FT_Byte)( ( ss[0] & 0xF0 ) | ss[1] >> 4 );

                    ss += 2;
                    tt += 1;
                }

                j = source->width & 1;
                if ( j > 0 )
                {
                    tt[0] = (FT_Byte)( ss[0] & 0xF0 );
                }

                s += source->pitch;
                t += target->pitch;
            }
        }
        break;

    case FT_PIXEL_MODE_GRAY2:
        {
            FT_Byte*  s = source->buffer;
            FT_Byte*  t = target->buffer;
            FT_Int    i;

            target->num_grays = 16;

            for ( i = source->rows; i > 0; i-- )
            {
                FT_Byte*  ss = s;
                FT_Byte*  tt = t;
                FT_Int    j;

                /* get the full bytes */
                for ( j = source->width >> 2; j > 0; j-- )
                {
                    FT_Int  val = ss[0];

                    tt[0] = (FT_Byte)( (( val & 0xC0 ) >> 2) | (( val & 0x30 ) >> 4) );
                    tt[1] = (FT_Byte)( (( val & 0x0C ) << 2) | ( val & 0x03 ) );

                    ss += 1;
                    tt += 2;
                }

                j = source->width & 3;
                if ( j > 0 )
                {
                    FT_Int  val = ss[0];

                    switch (j)
                    {
                    case 3:
                        tt[0] = (FT_Byte)( (( val & 0xC0 ) >> 2) | (( val & 0x30 ) >> 4) );
                        tt[1] = (FT_Byte)( ( val & 0x0C ) << 2 );
                        break;

                    case 2:
                        tt[0] = (FT_Byte)( (( val & 0xC0 ) >> 2) | (( val & 0x30 ) >> 4) );
                        break;

                    case 1:
                        tt[0] = (FT_Byte)( ( val & 0xC0 ) >> 2 );
                        break;
                    }
                }

                s += source->pitch;
                t += target->pitch;
            }
        }
        break;

    default:
        ;
    }

    return error;
}

/* The face requester is a function provided by the client application   */
/* to the cache manager, whose role is to translate an `abstract' face   */
/* ID into a real FT_Face object.                                        */
/*                                                                       */
/* In this program, the face IDs are simply pointers to TFont objects.   */
static FT_Error face_requester(FTC_FaceID face_id,
                               FT_Library lib,
                               FT_Pointer request_data,
                               FT_Face*   aface)
{
    FT_Error error;
    TFont* font = (TFont*) face_id;

    FT_UNUSED(request_data);

    if (font->array)
    {
        error = FT_New_Memory_Face(lib,
            font->array,
            font->array_size,
            font->face_index,
            aface);
    }
    else
    {
        error = FT_New_Face(lib,
            font->filepathname,
            font->face_index,
            aface);
    }
    if (error)
    {
        LOG_ERR "FT_New_Face fail: %d\n", error LOG_END
        font->face = NULL;
    }
    else
    {
        if ((*aface)->charmaps)
            (*aface)->charmap = (*aface)->charmaps[font->cmap_index];

        font->face = *aface;
    }

    return error;
}

void ituFtExit(void)
{
    int i;

    for (i = 0; i < ITU_FREETYPE_MAX_FONTS; i++)
    {
        if (ft.fonts[i].filepathname)
            free((void*) ft.fonts[i].filepathname);
    }

    FT_Bitmap_Done(ft.library, &ft.bitmap);
    FTC_Manager_Done(ft.cache_manager);
    FT_Done_FreeType(ft.library);

    memset(&ft, 0, sizeof(ft));
}

int ituFtLoadFont(int index, char* filename, ITUGlyphFormat format)
{
    FT_Error error;
    FT_Face  face;
    TFont* font;

    assert(index >= 0);
    assert(index < ITU_FREETYPE_MAX_FONTS);
    assert(filename);

    if (index >= ITU_FREETYPE_MAX_FONTS)
    {
        LOG_ERR "out of font index: %d >= \n", index, ITU_FREETYPE_MAX_FONTS LOG_END
        return __LINE__;
    }

    font = &ft.fonts[index];

    if (font->array)
    {
        FTC_Manager_RemoveFaceID(ft.cache_manager, font->scaler.face_id);
        font->array = NULL;
    }
    else if (font->filepathname)
    {
        FTC_Manager_RemoveFaceID(ft.cache_manager, font->scaler.face_id);
        free((void*) font->filepathname);
        font->filepathname = NULL;
    }

    error = FT_New_Face(ft.library, filename, 0, &face);
    if (error)
    {
        LOG_ERR "couldn't open this file: %s\n", filename LOG_END
        return error;
    }

    font->filepathname = (char*) malloc(strlen(filename) + 1);
    if (!font->filepathname)
    {
        LOG_ERR "out of memory: %d\n", strlen(filename) + 1 LOG_END
        return __LINE__;
    }

    strcpy(font->filepathname, filename);

    font->filename = strrchr(font->filepathname, '/');
    if (font->filename)
    {
        font->filename++;
    }
    else
    {
        font->filename = font->filepathname;
    }

    font->face_index = 0;
    font->cmap_index = face->charmap ? FT_Get_Charmap_Index(face->charmap) : 0;
    font->num_indices = 0x110000L;

    FT_Done_Face(face);
    face = NULL;

    font->scaler.face_id = (FTC_FaceID) font;

    switch (format)
    {
    case ITU_GLYPH_1BPP:
        font->load_flags     = FT_LOAD_TARGET_MONO | FT_LOAD_RENDER;
        font->render_mode    = FT_RENDER_MODE_MONO;
        break;

    case ITU_GLYPH_4BPP:
    case ITU_GLYPH_8BPP:
        font->load_flags     = FT_LOAD_TARGET_NORMAL;
        font->render_mode    = FT_RENDER_MODE_NORMAL;
        break;

    default:
        LOG_ERR "unknown glyph format: %d\n", format LOG_END
        return __LINE__;
    }
    font->format = format;

    if (!ft.current_font)
        ft.current_font = font;

    if (!ft.default_font)
        ft.default_font = font;

    return FT_Err_Ok;
}

int ituFtLoadFontArray(int index, uint8_t* array, int size, ITUGlyphFormat format)
{
    FT_Error error;
    FT_Face  face;
    TFont* font;

    assert(index >= 0);
    assert(index < ITU_FREETYPE_MAX_FONTS);
    assert(array);
    assert(size > 0);

    if (index >= ITU_FREETYPE_MAX_FONTS)
    {
        LOG_ERR "out of font index: %d >= \n", index, ITU_FREETYPE_MAX_FONTS LOG_END
            return __LINE__;
    }

    font = &ft.fonts[index];

    if (font->array)
    {
        FTC_Manager_RemoveFaceID(ft.cache_manager, font->scaler.face_id);
        font->array = NULL;
    }
    else if (font->filepathname)
    {
        FTC_Manager_RemoveFaceID(ft.cache_manager, font->scaler.face_id);
        free((void*)font->filepathname);
        font->filepathname = NULL;
    }

    error = FT_New_Memory_Face(ft.library, array, size, 0, &face);
    if (error)
    {
        LOG_ERR "couldn't open font array: %d\n", error LOG_END
        return error;
    }

    font->array = array;
    font->array_size = size;
    font->face_index = 0;
    font->cmap_index = face->charmap ? FT_Get_Charmap_Index(face->charmap) : 0;
    font->num_indices = 0x110000L;

    FT_Done_Face(face);
    face = NULL;

    font->scaler.face_id = (FTC_FaceID)font;

    switch (format)
    {
    case ITU_GLYPH_1BPP:
        font->load_flags = FT_LOAD_TARGET_MONO | FT_LOAD_RENDER;
        font->render_mode = FT_RENDER_MODE_MONO;
        break;

    case ITU_GLYPH_4BPP:
    case ITU_GLYPH_8BPP:
        font->load_flags = FT_LOAD_TARGET_NORMAL;
        font->render_mode = FT_RENDER_MODE_NORMAL;
        break;

    default:
        LOG_ERR "unknown glyph format: %d\n", format LOG_END
            return __LINE__;
    }
    font->format = format;

    if (!ft.current_font)
        ft.current_font = font;

    if (!ft.default_font)
        ft.default_font = font;

    return FT_Err_Ok;
}

void ituFtSetCurrentFont(int index)
{
    assert(index >= 0);
    assert(index < ITU_FREETYPE_MAX_FONTS);

    if (index < 0 || index >= ITU_FREETYPE_MAX_FONTS)
    {
        LOG_ERR "out of font index: %d >= %d\n", index, ITU_FREETYPE_MAX_FONTS LOG_END
        return;
    }

    ft.current_font   = &ft.fonts[index];
}

void ituFtSetFontSize(int width, int height)
{
    if (!ft.current_font)
    {
        LOG_ERR "current font not exist\n" LOG_END
        return;
    }

    ft.current_font->scaler.width  = (FT_UInt) width;
    ft.current_font->scaler.height = (FT_UInt) height;
    ft.current_font->scaler.pixel  = 1;
}

int ituFtDrawText(ITUSurface* surf, int x, int y, const char* text)
{
    FT_Error error = FT_Err_Ok;
    int i, len, x_advance;
    wchar_t buf[512];
    assert(text);

    if (!ft.current_font)
    {
        LOG_ERR "current font not exist\n" LOG_END
        goto end;
    }

    len = mbstowcs(buf, text, 512);
    x_advance = 0;

    if ((ft.style & ITU_FT_STYLE_BOLD) && ft.bold_size > 0)
    {
        for (i = 0; i < len; i++)
        {
            int     charcode;
            FT_GlyphSlot glyf;
            FT_Glyph glyph;
            FT_Face face = ft.current_font->face;
            ITUGlyphFormat format;
            FT_BitmapGlyph bitmapGlyph;
			FT_UInt gindex;
			TFont*  tfont = ft.current_font;

            charcode = buf[i];
			gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
			if (gindex == 0 && tfont != ft.default_font)
			{
				ft.default_font->scaler.width = tfont->scaler.width;
				ft.default_font->scaler.height = tfont->scaler.height;
				ft.default_font->scaler.pixel = 1;
				tfont = ft.default_font;
				gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
			}

			error = FTC_Manager_LookupFace(ft.cache_manager, tfont->scaler.face_id, &face);
			if (error)
			{
				LOG_ERR "FTC_Manager_LookupFace fail: %d\n", error LOG_END
					continue;
			}

            error = FT_Load_Char(face, charcode, ft.current_font->load_flags);
            if (error && face != ft.default_font->face)
            {
                face = ft.default_font->face;
                error = FT_Load_Char(face, charcode, ft.default_font->load_flags);
            }
            if (error)
                continue;

			if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
				FT_Outline_Embolden(&face->glyph->outline, ft.bold_size * ((face->glyph->outline.flags >= 8) ? (face->glyph->outline.flags * 16):(64)));
            else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP)
                FT_Bitmap_Embolden(ft.library, &face->glyph->bitmap, ft.bold_size * 64, ft.bold_size * 64);

            glyf = face->glyph;
            error = FT_Get_Glyph(glyf, &glyph);
            if (error)
                continue;

			FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
            bitmapGlyph = (FT_BitmapGlyph)glyph;

            switch (bitmapGlyph->bitmap.pixel_mode)
            {
            case FT_PIXEL_MODE_MONO:
                format = ITU_GLYPH_1BPP;
                break;

            case FT_PIXEL_MODE_GRAY4:
                format = ITU_GLYPH_4BPP;
                break;

            case FT_PIXEL_MODE_GRAY:
                format = ITU_GLYPH_8BPP;
                break;
            }
            //ituDrawGlyph(surf, x + x_advance, y + face->size->metrics.y_ppem - bitmapGlyph->top, format, bitmapGlyph->bitmap.buffer, bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);
			if (bitmapGlyph->bitmap.buffer)
			{
				FTC_SBit sbit;
				FT_Glyph glyph_local_mapping;

				error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
					&tfont->scaler,
					tfont->load_flags,
					gindex,
					&sbit,
					NULL);
				error = FTC_ImageCache_LookupScaler(ft.image_cache,
					&tfont->scaler,
					tfont->load_flags,
					gindex,
					&glyph_local_mapping,
					NULL);
				if (error)
				{
					LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
						continue;
				}
				else
				{
					ituDrawGlyph(surf, x + x_advance + sbit->left, y + tfont->scaler.height - sbit->top, format, bitmapGlyph->bitmap.buffer, bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);
				}
			}
			else
			{
				LOG_ERR "FreeCache drawtext get no valid glyph map from bold outline!\n" LOG_END
			}
			x_advance += glyf->advance.x >> 6;
			FT_Done_Glyph(glyph);
        }
    }
    else
    {
        for (i = 0; i < len; i++)
        {
            FT_UInt gindex;
            int     charcode;
            TFont*  tfont = ft.current_font;
            FTC_SBit sbit;
            FT_Face face;

            charcode = buf[i];
            gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
            if (gindex == 0 && tfont != ft.default_font)
            {
                ft.default_font->scaler.width = tfont->scaler.width;
                ft.default_font->scaler.height = tfont->scaler.height;
                ft.default_font->scaler.pixel = 1;
                tfont = ft.default_font;
                gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
            }

            error = FTC_Manager_LookupFace(ft.cache_manager, tfont->scaler.face_id, &face);
            if (error)
            {
                LOG_ERR "FTC_Manager_LookupFace fail: %d\n", error LOG_END
                    continue;
            }

            /* use the SBits cache to store small glyph bitmaps; this is a lot */
            /* more memory-efficient                                           */

            if (tfont->scaler.width < MAX_SBIT_CACHE && tfont->scaler.height < MAX_SBIT_CACHE)
            {
                error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                    &tfont->scaler,
                    tfont->load_flags,
                    gindex,
                    &sbit,
                    NULL);
                if (error)
                {
                    LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                        continue;
                }

                if (sbit->buffer)
                {
                    if ((tfont->format == ITU_GLYPH_4BPP) && (sbit->format == FT_PIXEL_MODE_GRAY || sbit->format == FT_PIXEL_MODE_GRAY2))
                    {
                        FT_Bitmap  source;
                    	int yy;

                        source.rows = sbit->height;
                        source.width = sbit->width;
                        source.pitch = sbit->pitch;
                        source.buffer = sbit->buffer;
                        source.pixel_mode = sbit->format;
                        Bitmap_Convert_GRAY4(ft.library, &source, &ft.bitmap);

	                    yy = y + tfont->scaler.height - sbit->top;
	                    if (yy < 0)
	                        yy = 0;
	                    ituDrawGlyph(surf, x + x_advance + sbit->left, yy, ITU_GLYPH_4BPP, ft.bitmap.buffer, ft.bitmap.width, ft.bitmap.rows);
                    }
                    else
                    {
                        ITUGlyphFormat format;
                    	int yy;

                        switch (sbit->format)
                        {
                        case FT_PIXEL_MODE_MONO:
                            format = ITU_GLYPH_1BPP;
                            break;

                        case FT_PIXEL_MODE_GRAY4:
                            format = ITU_GLYPH_4BPP;
                            break;

                        case FT_PIXEL_MODE_GRAY:
                            format = ITU_GLYPH_8BPP;
                            break;
                        }
	                    yy = y + tfont->scaler.height - sbit->top;
	                    if (yy < 0)
	                        yy = 0;
	                    ituDrawGlyph(surf, x + x_advance + sbit->left, yy, format, sbit->buffer, sbit->width, sbit->height);
                    }
                }

                x_advance += sbit->xadvance;
            }
            else
            {
                /* otherwise, use an image cache to store glyph outlines, and render */
                /* them on demand. we can thus support very large sizes easily..     */
                FT_Glyph glyph = NULL;

                error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                    &tfont->scaler,
                    tfont->load_flags,
                    gindex,
                    &sbit,
                    NULL);
                if (error)
                {
                    LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                        continue;
                }

                error = FTC_ImageCache_LookupScaler(ft.image_cache,
                    &tfont->scaler,
                    tfont->load_flags,
                    gindex,
                    &glyph,
                    NULL);
                if (error)
                {
                    LOG_ERR "FTC_ImageCache_LookupScaler fail: %d\n", error LOG_END
                        continue;
                }

                if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    //error = FT_Glyph_To_Bitmap(&glyph, tfont->render_mode, NULL, 0);
                    error = FT_Outline_Embolden(&face->glyph->outline, 2 * 64);
                    if (error)
                    {
                        LOG_ERR "FT_Glyph_To_Bitmap fail: %d\n", error LOG_END
                            FT_Done_Glyph(glyph);
                        continue;
                    }
                }
                else if (glyph->format == FT_GLYPH_FORMAT_BITMAP)
                {
                    FT_BitmapGlyph  bitmap = (FT_BitmapGlyph)glyph;
                    FT_Bitmap*      source = &bitmap->bitmap;

                    if (source->width > 0)
                    {
                        if ((tfont->format == ITU_GLYPH_4BPP) && (sbit->format == FT_PIXEL_MODE_GRAY || sbit->format == FT_PIXEL_MODE_GRAY2))
                        {
                        	int yy;

                            Bitmap_Convert_GRAY4(ft.library, source, &ft.bitmap);

	                        yy = y + tfont->scaler.height - sbit->top;
	                        if (yy < 0)
	                            yy = 0;
	                        ituDrawGlyph(surf, x + x_advance + sbit->left, yy, ITU_GLYPH_4BPP, ft.bitmap.buffer, ft.bitmap.width, ft.bitmap.rows);
                        }
                        else
                        {
                            ITUGlyphFormat format;
                        	int yy;

                            switch (sbit->format)
                            {
                            case FT_PIXEL_MODE_MONO:
                                format = ITU_GLYPH_1BPP;
                                break;

                            case FT_PIXEL_MODE_GRAY4:
                                format = ITU_GLYPH_4BPP;
                                break;

                            case FT_PIXEL_MODE_GRAY:
                            default:
                                format = ITU_GLYPH_8BPP;
                                break;
                            }
	                        yy = y + tfont->scaler.height - sbit->top;
	                        if (yy < 0)
	                            yy = 0;
	                        ituDrawGlyph(surf, x + x_advance + sbit->left, yy, format, sbit->buffer, sbit->width, sbit->height);
                        }
                    }
                }
                else
                {
                    LOG_ERR "invalid glyph format returned: %d\n", glyph->format LOG_END

                    if (glyph)
                        FT_Done_Glyph(glyph);

                    goto end;
                }
                x_advance += (glyph->advance.x + 0x8000) >> 16;

                if (glyph)
                    FT_Done_Glyph(glyph);
            }
        }
    }

end:
    return error;
}

void ituFtResetCache(void)
{
    FTC_Manager_Reset(ft.cache_manager);
}

void ituFtGetTextDimension(const char* text, int* width, int* height)
{
    FT_Error error = FT_Err_Ok;
    int i, len, x_advance;
    wchar_t buf[512];
    unsigned int max_height;
    assert(text);

    if (!ft.current_font)
    {
        LOG_ERR "current font not exist\n" LOG_END
        return;
    }

    len = mbstowcs(buf, text, 512);
    x_advance = 0;
    max_height = 0;

    for (i = 0; i < len; i++)
    {
        FT_UInt gindex;
        int     charcode;
        TFont*  tfont = ft.current_font;
        FTC_SBit sbit;
        FT_Face face;
        unsigned int h;

        charcode = buf[i];
        gindex   = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
        if (gindex == 0 && tfont != ft.default_font)
        {
            ft.default_font->scaler.width  = tfont->scaler.width;
            ft.default_font->scaler.height = tfont->scaler.height;
            ft.default_font->scaler.pixel  = 1;
            tfont   = ft.default_font;
            gindex  = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
        }

        error = FTC_Manager_LookupFace(ft.cache_manager, tfont->scaler.face_id, &face);
        if (error)
        {
            LOG_ERR "FTC_Manager_LookupFace fail: %d\n", error LOG_END
            continue;
        }

        /* use the SBits cache to store small glyph bitmaps; this is a lot */
        /* more memory-efficient                                           */

        if (tfont->scaler.width < MAX_SBIT_CACHE && tfont->scaler.height < MAX_SBIT_CACHE)
        {
            error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                                              &tfont->scaler,
                                              tfont->load_flags,
                                              gindex,
                                              &sbit,
                                              NULL);
            if (error)
            {
                LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                continue;
            }

            h = tfont->scaler.height;
            if (max_height < h)
                max_height = h;

            x_advance += sbit->xadvance;
        }
        else
        {
            /* otherwise, use an image cache to store glyph outlines, and render */
            /* them on demand. we can thus support very large sizes easily..     */
            FT_Glyph glyph;

            error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                                              &tfont->scaler,
                                              tfont->load_flags,
                                              gindex,
                                              &sbit,
                                              NULL);
            if (error)
            {
                LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                continue;
            }

            error = FTC_ImageCache_LookupScaler(ft.image_cache,
                                               &tfont->scaler,
                                               tfont->load_flags,
                                               gindex,
                                               &glyph,
                                               NULL);
            if (error)
            {
                LOG_ERR "FTC_ImageCache_LookupScaler fail: %d\n", error LOG_END
                continue;
            }

            if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
            {
                error = FT_Glyph_To_Bitmap(&glyph, tfont->render_mode, NULL, 0);
                if (error)
                {
                    LOG_ERR "FT_Glyph_To_Bitmap fail: %d\n", error LOG_END
                    continue;
                }
            }

            h = tfont->scaler.height;
            if (max_height < h)
                max_height = h;

            x_advance += (glyph->advance.x + 0x8000) >> 16;

            if (glyph)
                FT_Done_Glyph(glyph);
        }
    }
    if (width)
        *width = x_advance;

    if (height)
        *height = max_height;
}

int ituFtGetCharWidth(const char* text, int* width)
{
    FT_Error error = FT_Err_Ok;
    int len;
    wchar_t buf;
    assert(text);
    assert(width);

    if (!ft.current_font)
    {
        LOG_ERR "current font not exist\n" LOG_END
        goto end;
    }

    len = strlen(text);
    len = mbtowc(&buf, text, len);

    if (len > 0)
    {
        FT_UInt gindex;
        int     charcode;
        TFont*  tfont = ft.current_font;
        FTC_SBit sbit;
        FT_Face face;

        charcode = buf;
        gindex   = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
        if (gindex == 0 && tfont != ft.default_font)
        {
            ft.default_font->scaler.width  = tfont->scaler.width;
            ft.default_font->scaler.height = tfont->scaler.height;
            ft.default_font->scaler.pixel  = 1;
            tfont   = ft.default_font;
            gindex  = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
        }

        error = FTC_Manager_LookupFace(ft.cache_manager, tfont->scaler.face_id, &face);
        if (error)
        {
            LOG_ERR "FTC_Manager_LookupFace fail: %d\n", error LOG_END
            goto end;
        }

        /* use the SBits cache to store small glyph bitmaps; this is a lot */
        /* more memory-efficient                                           */

        if (tfont->scaler.width < MAX_SBIT_CACHE && tfont->scaler.height < MAX_SBIT_CACHE)
        {
            error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                                              &tfont->scaler,
                                              tfont->load_flags,
                                              gindex,
                                              &sbit,
                                              NULL);
            if (error)
            {
                LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                goto end;
            }

            *width = sbit->xadvance;
        }
        else
        {
            /* otherwise, use an image cache to store glyph outlines, and render */
            /* them on demand. we can thus support very large sizes easily..     */
            FT_Glyph glyph;

            error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                                              &tfont->scaler,
                                              tfont->load_flags,
                                              gindex,
                                              &sbit,
                                              NULL);
            if (error)
            {
                LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                goto end;
            }

            error = FTC_ImageCache_LookupScaler(ft.image_cache,
                                               &tfont->scaler,
                                               tfont->load_flags,
                                               gindex,
                                               &glyph,
                                               NULL);
            if (error)
            {
                LOG_ERR "FTC_ImageCache_LookupScaler fail: %d\n", error LOG_END
                goto end;
            }

            if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
            {
                error = FT_Glyph_To_Bitmap(&glyph, tfont->render_mode, NULL, 0);
                if (error)
                {
                    LOG_ERR "FT_Glyph_To_Bitmap fail: %d\n", error LOG_END
                    goto end;
                }
            }

            *width += (glyph->advance.x + 0x8000) >> 16;

            if (glyph)
                FT_Done_Glyph(glyph);
        }
    }

end:
    return len;
}

int ituFtDrawChar(ITUSurface* surf, int x, int y, const char* text)
{
    FT_Error error = FT_Err_Ok;
    int len;
    wchar_t buf;
    assert(text);

    if (!ft.current_font)
    {
        LOG_ERR "current font not exist\n" LOG_END
        goto end;
    }

    len = strlen(text);
    len = mbtowc(&buf, text, len);

    if (len > 0)
    {
        if ((ft.style & ITU_FT_STYLE_BOLD) && ft.bold_size > 0)
        {
            int     charcode;
            FT_GlyphSlot glyf;
            FT_Glyph glyph;
            FT_Face face = ft.current_font->face;
            ITUGlyphFormat format;
            FT_BitmapGlyph bitmapGlyph;
			FT_UInt gindex;
			TFont*  tfont = ft.current_font;

			charcode = buf;
			gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
			if (gindex == 0 && tfont != ft.default_font)
			{
				ft.default_font->scaler.width = tfont->scaler.width;
				ft.default_font->scaler.height = tfont->scaler.height;
				ft.default_font->scaler.pixel = 1;
				tfont = ft.default_font;
				gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
			}

			error = FTC_Manager_LookupFace(ft.cache_manager, tfont->scaler.face_id, &face);
			if (error)
			{
				LOG_ERR "FTC_Manager_LookupFace fail: %d\n", error LOG_END
			}


            error = FT_Load_Char(face, charcode, ft.current_font->load_flags);
            if (error && face != ft.default_font->face)
            {
                face = ft.default_font->face;
                error = FT_Load_Char(face, charcode, ft.default_font->load_flags);
            }
            if (error)
            {
                LOG_ERR "FT_Load_Char fail: %d\n", error LOG_END
                goto end;
            }

            if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                FT_Outline_Embolden(&face->glyph->outline, ft.bold_size * ((face->glyph->outline.flags >= 8) ? (face->glyph->outline.flags * 16) : (64)));
            else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP)
                FT_Bitmap_Embolden(ft.library, &face->glyph->bitmap, ft.bold_size * 64, ft.bold_size * 64);

            glyf = face->glyph;
            error = FT_Get_Glyph(glyf, &glyph);
            if (error)
            {
                LOG_ERR "FT_Get_Glyph fail: %d\n", error LOG_END
                goto end;
            }

            FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
            bitmapGlyph = (FT_BitmapGlyph)glyph;

            switch (bitmapGlyph->bitmap.pixel_mode)
            {
            case FT_PIXEL_MODE_MONO:
                format = ITU_GLYPH_1BPP;
                break;

            case FT_PIXEL_MODE_GRAY4:
                format = ITU_GLYPH_4BPP;
                break;

            case FT_PIXEL_MODE_GRAY:
                format = ITU_GLYPH_8BPP;
                break;
            }
            //ituDrawGlyph(surf, x, y + face->size->metrics.y_ppem - bitmapGlyph->top, format, bitmapGlyph->bitmap.buffer, bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);
			if (bitmapGlyph->bitmap.buffer)
			{
				FTC_SBit sbit;
				FT_Glyph glyph_local_mapping;

				error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
					&tfont->scaler,
					tfont->load_flags,
					gindex,
					&sbit,
					NULL);
				error = FTC_ImageCache_LookupScaler(ft.image_cache,
					&tfont->scaler,
					tfont->load_flags,
					gindex,
					&glyph_local_mapping,
					NULL);
				if (error)
				{
					LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
				}
				else
				{
					int yy = y + tfont->scaler.height - sbit->top;
					if (yy < 0)
						yy = 0;
					ituDrawGlyph(surf, x + sbit->left, yy, format, bitmapGlyph->bitmap.buffer, bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);
				}
			}
			else
			{
				LOG_ERR "FreeCache drawchar get no valid glyph map from bold outline!\n" LOG_END
			}
            FT_Done_Glyph(glyph);
        }
        else
        {
            FT_UInt gindex;
            int     charcode;
            TFont*  tfont = ft.current_font;
            FTC_SBit sbit;
            FT_Face face;

            charcode = buf;
            gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
            if (gindex == 0 && tfont != ft.default_font)
            {
                ft.default_font->scaler.width = tfont->scaler.width;
                ft.default_font->scaler.height = tfont->scaler.height;
                ft.default_font->scaler.pixel = 1;
                tfont = ft.default_font;
                gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
            }

            error = FTC_Manager_LookupFace(ft.cache_manager, tfont->scaler.face_id, &face);
            if (error)
            {
                LOG_ERR "FTC_Manager_LookupFace fail: %d\n", error LOG_END
                goto end;
            }

            /* use the SBits cache to store small glyph bitmaps; this is a lot */
            /* more memory-efficient                                           */

            if (tfont->scaler.width < MAX_SBIT_CACHE && tfont->scaler.height < MAX_SBIT_CACHE)
            {
                error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                    &tfont->scaler,
                    tfont->load_flags,
                    gindex,
                    &sbit,
                    NULL);
                if (error)
                {
                    LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                        goto end;
                }

                if (sbit->buffer)
                {
                    if ((tfont->format == ITU_GLYPH_4BPP) && (sbit->format == FT_PIXEL_MODE_GRAY || sbit->format == FT_PIXEL_MODE_GRAY2))
                    {
                        FT_Bitmap  source;
                    	int yy;

                        source.rows = sbit->height;
                        source.width = sbit->width;
                        source.pitch = sbit->pitch;
                        source.buffer = sbit->buffer;
                        source.pixel_mode = sbit->format;
                        Bitmap_Convert_GRAY4(ft.library, &source, &ft.bitmap);
	                    yy = y + tfont->scaler.height - sbit->top;
	                    if (yy < 0)
	                        yy = 0;
	                    ituDrawGlyph(surf, x + sbit->left, yy, ITU_GLYPH_4BPP, ft.bitmap.buffer, ft.bitmap.width, ft.bitmap.rows);
                    }
                    else
                    {
                        ITUGlyphFormat format;
                    	int yy;
                        switch (sbit->format)
                        {
                        case FT_PIXEL_MODE_MONO:
                            format = ITU_GLYPH_1BPP;
                            break;

                        case FT_PIXEL_MODE_GRAY4:
                            format = ITU_GLYPH_4BPP;
                            break;

                        case FT_PIXEL_MODE_GRAY:
                            format = ITU_GLYPH_8BPP;
                            break;
                        }
	                    yy = y + tfont->scaler.height - sbit->top;
	                    if (yy < 0)
	                        yy = 0;
	                    ituDrawGlyph(surf, x + sbit->left, yy, format, sbit->buffer, sbit->width, sbit->height);
                    }
                }
            }
            else
            {
                /* otherwise, use an image cache to store glyph outlines, and render */
                /* them on demand. we can thus support very large sizes easily..     */
                FT_Glyph glyph = NULL;

                error = FTC_SBitCache_LookupScaler(ft.sbits_cache,
                    &tfont->scaler,
                    tfont->load_flags,
                    gindex,
                    &sbit,
                    NULL);
                if (error)
                {
                    LOG_ERR "FTC_SBitCache_LookupScaler fail: %d\n", error LOG_END
                        goto end;
                }

                error = FTC_ImageCache_LookupScaler(ft.image_cache,
                    &tfont->scaler,
                    tfont->load_flags,
                    gindex,
                    &glyph,
                    NULL);
                if (error)
                {
                    LOG_ERR "FTC_ImageCache_LookupScaler fail: %d\n", error LOG_END
                        goto end;
                }

                if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    error = FT_Glyph_To_Bitmap(&glyph, tfont->render_mode, NULL, 0);
                    if (error)
                    {
                        LOG_ERR "FT_Glyph_To_Bitmap fail: %d\n", error LOG_END
                            FT_Done_Glyph(glyph);
                        goto end;
                    }
                }

                if (glyph->format == FT_GLYPH_FORMAT_BITMAP)
                {
                    FT_BitmapGlyph  bitmap = (FT_BitmapGlyph)glyph;
                    FT_Bitmap*      source = &bitmap->bitmap;

                    if (source->width > 0)
                    {
                        if ((tfont->format == ITU_GLYPH_4BPP) && (sbit->format == FT_PIXEL_MODE_GRAY || sbit->format == FT_PIXEL_MODE_GRAY2))
                        {
                        	int yy;

                            Bitmap_Convert_GRAY4(ft.library, source, &ft.bitmap);

	                        yy = y + tfont->scaler.height - sbit->top;
	                        if (yy < 0)
	                            yy = 0;
	                        ituDrawGlyph(surf, x + sbit->left, yy, ITU_GLYPH_4BPP, ft.bitmap.buffer, ft.bitmap.width, ft.bitmap.rows);
                        }
                        else
                        {
                            ITUGlyphFormat format;
                        	int yy;
                            switch (sbit->format)
                            {
                            case FT_PIXEL_MODE_MONO:
                                format = ITU_GLYPH_1BPP;
                                break;

                            case FT_PIXEL_MODE_GRAY4:
                                format = ITU_GLYPH_4BPP;
                                break;

                            case FT_PIXEL_MODE_GRAY:
                            default:
                                format = ITU_GLYPH_8BPP;
                                break;
                            }
	                        yy = y + tfont->scaler.height - source->rows;
	                        if (yy < 0)
	                            yy = 0;
	                        ituDrawGlyph(surf, x + bitmap->left, yy, format, source->buffer, source->width, source->rows);
                        }
                    }
                }
                else
                {
                    LOG_ERR "invalid glyph format returned: %d\n", glyph->format LOG_END

                    if (glyph)
                        FT_Done_Glyph(glyph);

                    goto end;
                }

                if (glyph)
                    FT_Done_Glyph(glyph);
            }
        }
    }

end:
    return error;
}

int ituFtInit(void)
{
    FT_Error error;

	memset(&ft, 0, sizeof(ft));

    error = FT_Init_FreeType(&ft.library);
    if (error)
    {
        LOG_ERR "could not initialize FreeType\n" LOG_END
            goto end;
    }

    error = FTC_Manager_New(ft.library, 0, 0, CFG_ITU_FT_CACHE_SIZE,
        face_requester, 0, &ft.cache_manager);
    if (error)
    {
        LOG_ERR "could not initialize cache manager\n" LOG_END
            goto end;
    }

    error = FTC_SBitCache_New(ft.cache_manager, &ft.sbits_cache);
    if (error)
    {
        LOG_ERR "could not initialize small bitmaps cache\n" LOG_END
            goto end;
    }

    error = FTC_ImageCache_New(ft.cache_manager, &ft.image_cache);
    if (error)
    {
        LOG_ERR "could not initialize glyph image cache\n" LOG_END
            goto end;
    }

    error = FTC_CMapCache_New(ft.cache_manager, &ft.cmap_cache);
    if (error)
    {
        LOG_ERR "could not initialize charmap cache\n" LOG_END
            goto end;
    }

    FT_Bitmap_New(&ft.bitmap);

end:
    if (error)
        ituFtExit();

    return error;
}

void ituFtSetFontStyle(unsigned int style)
{
    ft.style = style;
}

void ituFtSetFontStyleValue(unsigned int style, int value)
{
    switch (style)
    {
    case ITU_FT_STYLE_BOLD:
        ft.bold_size = value;
        break;
    }
}

bool ituFtIsCharValid(const char* text)
{
    FT_Error error = FT_Err_Ok;
    int len;
    wchar_t buf;
    bool result = false;
    assert(text);

    if (!ft.current_font)
    {
        LOG_ERR "current font not exist\n" LOG_END
        goto end;
    }

    len = strlen(text);
    len = mbtowc(&buf, text, len);

    if (len > 0)
    {
        FT_UInt gindex;
        int     charcode;
        TFont*  tfont = ft.current_font;

        charcode = buf;
        gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
        if (gindex == 0 && tfont != ft.default_font)
        {
            ft.default_font->scaler.width = tfont->scaler.width;
            ft.default_font->scaler.height = tfont->scaler.height;
            ft.default_font->scaler.pixel = 1;
            tfont = ft.default_font;
            gindex = FTC_CMapCache_Lookup(ft.cmap_cache, tfont->scaler.face_id, tfont->cmap_index, charcode);
        }

        if (gindex)
        {
            result = true;
            goto end;
        }
    }
end:
    return result;
}
