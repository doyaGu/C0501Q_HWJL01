#define VG_API_EXPORT
#include "openvg.h"
#include "iteHardware.h"
#include "iteImage.h"
#include "iteVectors.h"
#include "iteGeometry.h"
#include "iteUtility.h"
#include "vgmem.h"
#include "iteDefs.h"
#include"itePath.h"
#include <ft2build.h>
#include <freetype/ftoutln.h>
#include FT_FREETYPE_H
#include <ft2build.h>
#include <freetype/ftoutln.h>
#include FT_FREETYPE_H

#ifndef _CONVERT_PATH_H
#define _CONVERT_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

ITEPath*
convert_outline_glyph(FT_GlyphSlot glyph);

#ifdef __cplusplus
}
#endif

#endif //_CONVERT_PATH_H
