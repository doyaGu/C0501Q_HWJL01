#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_INTERNAL_OBJECTS_H
#include FT_OUTLINE_H
#include FT_BITMAP_H

#include <assert.h>
#include <malloc.h>
#include <wchar.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "itu_private.h"

struct
{
    FT_Library     library;             /* the FreeType library            */
    FT_Face        fonts[ITU_FREETYPE_MAX_FONTS];    /* installed fonts */
    FT_Face        current_font;        /* selected font */
    FT_Face        default_font;        /* default font */
    ITUGlyphFormat glyph_formats[ITU_FREETYPE_MAX_FONTS];
    ITUGlyphFormat current_glyph_format;
    ITUGlyphFormat default_glyph_format;
    unsigned int    style;
    int				bold_size;
} ft;

void ituFtExit(void)
{
    int i;

    for (i = 0; i < ITU_FREETYPE_MAX_FONTS; i++)
    {
        if (ft.fonts[i])
            FT_Done_Face(ft.fonts[i]);
    }

    FT_Done_FreeType(ft.library);

    memset(&ft, 0, sizeof(ft));
}

int ituFtLoadFont(int index, char* filename, ITUGlyphFormat format)
{
    FT_Error error;

    assert(index >= 0);
    assert(index < ITU_FREETYPE_MAX_FONTS);
    assert(filename);

    if (index >= ITU_FREETYPE_MAX_FONTS)
    {
        LOG_ERR "out of font index: %d >= \n", index, ITU_FREETYPE_MAX_FONTS LOG_END
        return __LINE__;
    }

    error = FT_New_Face(ft.library, filename, 0, &ft.fonts[index]);
    if (error)
    {
        LOG_ERR "couldn't open this file: %s\n", filename LOG_END
        return error;
    }

    ft.glyph_formats[index] = format;

    if (!ft.current_font)
    {
        ft.current_font = ft.fonts[index];
        ft.current_glyph_format = format;
    }

    if (!ft.default_font)
    {
        ft.default_font = ft.fonts[index];
        ft.default_glyph_format = format;
    }

    return FT_Err_Ok;
}

int ituFtLoadFontArray(int index, uint8_t* array, int size, ITUGlyphFormat format)
{
    FT_Error error;

    assert(index >= 0);
    assert(index < ITU_FREETYPE_MAX_FONTS);
    assert(array);
    assert(size > 0);

    if (index >= ITU_FREETYPE_MAX_FONTS)
    {
        LOG_ERR "out of font index: %d >= \n", index, ITU_FREETYPE_MAX_FONTS LOG_END
            return __LINE__;
    }

    error = FT_New_Memory_Face(ft.library, array, size, 0, &ft.fonts[index]);
    if (error)
    {
        LOG_ERR "couldn't open font array: %d\n", error LOG_END
            return error;
    }

    ft.glyph_formats[index] = format;

    if (!ft.current_font)
    {
        ft.current_font = ft.fonts[index];
        ft.current_glyph_format = format;
    }

    if (!ft.default_font)
    {
        ft.default_font = ft.fonts[index];
        ft.default_glyph_format = format;
    }

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

    ft.current_font   = ft.fonts[index];
    ft.current_glyph_format = ft.glyph_formats[index];
}

void ituFtSetFontSize(int width, int height)
{
    if (!ft.current_font)
    {
        LOG_ERR "current font not exist\n" LOG_END
        return;
    }

    FT_Set_Pixel_Sizes(ft.current_font, width, height);
}

int ituFtDrawText(ITUSurface* surf, int x, int y, const char* text)
{
    FT_Error error = FT_Err_Ok;
    FT_ULong load_flags;
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

    switch (ft.current_glyph_format)
    {
    case ITU_GLYPH_1BPP:
        load_flags = FT_LOAD_TARGET_MONO | FT_LOAD_RENDER;
        break;

    case ITU_GLYPH_4BPP:
    case ITU_GLYPH_8BPP:
        load_flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_RENDER;
        break;

    default:
        LOG_ERR "unknown glyph format: %d\n", ft.current_glyph_format LOG_END
        goto end;
    }

    for (i = 0; i < len; i++)
    {
        int     charcode;
        FT_GlyphSlot glyf;
        FT_Glyph glyph;
        FT_Face face = ft.current_font;
        ITUGlyphFormat format;
        int yy;

        charcode = buf[i];

        error = FT_Load_Char(face, charcode, load_flags);
        if (error && face != ft.default_font)
        {
            face   = ft.default_font;
            error = FT_Load_Char(face, charcode, load_flags);
        }
        if (error)
            continue;

        if (ft.style & ITU_FT_STYLE_BOLD)
        {
            if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                FT_Outline_Embolden(&face->glyph->outline, ft.bold_size * 64);
            else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP)
                FT_Bitmap_Embolden(ft.library, &face->glyph->bitmap, ft.bold_size * 64, ft.bold_size * 64);
        }

        glyf = face->glyph;
        error = FT_Get_Glyph(glyf, &glyph);
        if (error)
            continue;

        switch (glyf->bitmap.pixel_mode)
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
        yy = y + face->size->metrics.y_ppem - glyf->bitmap_top;
        if (yy < 0)
            yy = 0;
		ituDrawGlyph(surf, x + x_advance + glyf->bitmap_left, yy, format, glyf->bitmap.buffer, glyf->bitmap.width, glyf->bitmap.rows);

        x_advance += glyf->advance.x >> 6;
        FT_Done_Glyph(glyph);
    }

end:
    return error;
}

void ituFtResetCache(void)
{
    // DO NOTHING
}

void ituFtGetTextDimension(const char* text, int* width, int* height)
{
    FT_Error error = FT_Err_Ok;
    FT_ULong load_flags;
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

    switch (ft.current_glyph_format)
    {
    case ITU_GLYPH_1BPP:
        load_flags = FT_LOAD_TARGET_MONO | FT_LOAD_RENDER;
        break;

    case ITU_GLYPH_4BPP:
    case ITU_GLYPH_8BPP:
        load_flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_RENDER;
        break;

    default:
        LOG_ERR "unknown glyph format: %d\n", ft.current_glyph_format LOG_END
        return;
    }

    max_height = 0;
    for (i = 0; i < len; i++)
    {
        int     charcode;
        FT_GlyphSlot glyf;
        FT_Glyph glyph;
        unsigned int h;
        FT_Face face = ft.current_font;

        charcode = buf[i];
        error = FT_Load_Char(face, charcode, load_flags);
        if (error && face != ft.default_font)
        {
            face   = ft.default_font;
            error = FT_Load_Char(face, charcode, load_flags);
        }
        if (error)
            continue;

        glyf = face->glyph;
        error = FT_Get_Glyph(glyf, &glyph);
        if (error)
            continue;

        h = face->size->metrics.height >> 6;
        if (max_height < h)
            max_height = h;

        x_advance += glyf->advance.x >> 6;
        FT_Done_Glyph(glyph);
    }

    if (width)
        *width = x_advance;

    if (height)
        *height = max_height;
}

int ituFtGetCharWidth(const char* text, int* width)
{
    FT_Error error = FT_Err_Ok;
    FT_ULong load_flags;
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

    switch (ft.current_glyph_format)
    {
    case ITU_GLYPH_1BPP:
        load_flags = FT_LOAD_TARGET_MONO | FT_LOAD_RENDER;
        break;

    case ITU_GLYPH_4BPP:
    case ITU_GLYPH_8BPP:
        load_flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_RENDER;
        break;

    default:
        LOG_ERR "unknown glyph format: %d\n", ft.current_glyph_format LOG_END
        goto end;
    }

    if (len > 0)
    {
        int charcode;
        FT_GlyphSlot glyf;
        FT_Glyph glyph;
        FT_Face face = ft.current_font;

        charcode = buf;
        error = FT_Load_Char(face, charcode, load_flags);
        if (error && face != ft.default_font)
        {
            face   = ft.default_font;
            error = FT_Load_Char(face, charcode, load_flags);
        }
        if (error)
        {
            LOG_ERR "load char fail: %d\n", error LOG_END
            goto end;
        }

        glyf = face->glyph;
        error = FT_Get_Glyph(glyf, &glyph);
        if (error)
        {
            LOG_ERR "get glyph fail: %d\n", error LOG_END
            goto end;
        }

        *width = glyf->advance.x >> 6;
        FT_Done_Glyph(glyph);
    }

end:
    return len;
}

int ituFtDrawChar(ITUSurface* surf, int x, int y, const char* text)
{
    FT_Error error = FT_Err_Ok;
    FT_ULong load_flags;
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

    switch (ft.current_glyph_format)
    {
    case ITU_GLYPH_1BPP:
        load_flags = FT_LOAD_TARGET_MONO | FT_LOAD_RENDER;
        break;

    case ITU_GLYPH_4BPP:
    case ITU_GLYPH_8BPP:
        load_flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_RENDER;
        break;

    default:
        LOG_ERR "unknown glyph format: %d\n", ft.current_glyph_format LOG_END
        goto end;
    }

    if (len > 0)
    {
        int charcode;
        FT_GlyphSlot glyf;
        FT_Glyph glyph;
        FT_Face face = ft.current_font;
        ITUGlyphFormat format;
        int yy;

        charcode = buf;
        error = FT_Load_Char(face, charcode, load_flags);
        if (error && face != ft.default_font)
        {
            face   = ft.default_font;
            error = FT_Load_Char(face, charcode, load_flags);
        }
        if (error)
        {
            LOG_ERR "load char fail: %d\n", error LOG_END
            goto end;
        }

        if (ft.style & ITU_FT_STYLE_BOLD)
        {
            if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                FT_Outline_Embolden(&face->glyph->outline, ft.bold_size * 64);
            else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP)
                FT_Bitmap_Embolden(ft.library, &face->glyph->bitmap, ft.bold_size * 64, ft.bold_size * 64);
        }

        glyf = face->glyph;
        error = FT_Get_Glyph(glyf, &glyph);
        if (error)
        {
            LOG_ERR "get glyph fail: %d\n", error LOG_END
            goto end;
        }

        switch (glyf->bitmap.pixel_mode)
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
        yy = y + face->size->metrics.y_ppem - glyf->bitmap_top;
        if (yy < 0)
            yy = 0;
        ituDrawGlyph(surf, x + glyf->bitmap_left, yy, format, glyf->bitmap.buffer, glyf->bitmap.width, glyf->bitmap.rows);

        FT_Done_Glyph(glyph);
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
        int charcode;
        FT_UInt  gindex;
        FT_Face face = ft.current_font;

        charcode = buf;
        gindex = FT_Get_Char_Index(face, charcode);
        if (gindex == 0 && face != ft.default_font)
        {
            face = ft.default_font;
            gindex = FT_Get_Char_Index(face, charcode);
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