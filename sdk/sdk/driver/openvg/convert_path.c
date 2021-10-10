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

static int
path_append(ITEPath* path, unsigned char segment, const FT_Vector **vectors)
{
	float coords[6];
	int i, num_vectors;

	switch (segment) 
	{
	case VG_MOVE_TO:
	case VG_LINE_TO:
		num_vectors = 1;
		break;
	case VG_QUAD_TO:
		num_vectors = 2;
		break;
	case VG_CUBIC_TO:
		num_vectors = 3;
		break;
	default:
		return -1;
		break;
	}

	for (i = 0; i < num_vectors; i++)
	{
		coords[2 * i + 0] = (float) vectors[i]->x / 64.0f;
		coords[2 * i + 1] = (float) vectors[i]->y / 64.0f;
	}

	vgAppendPathData(path, 1, &segment, (const void *) coords);

	return 0;
}

static int
decompose_move_to(const FT_Vector *to, void *user)
{
   ITEPath* path = (ITEPath*) (long) user;

   return path_append(path, VG_MOVE_TO, &to);
}

static int
decompose_line_to(const FT_Vector *to, void *user)
{
   ITEPath* path = (ITEPath*) (long) user;

   return path_append(path, VG_LINE_TO, &to);
}

static int
decompose_conic_to(const FT_Vector *control, const FT_Vector *to, void *user)
{
   ITEPath* path = (ITEPath*) (long) user;
   const FT_Vector *vectors[2] = { control, to };

   return path_append(path, VG_QUAD_TO, vectors);
}

static int
decompose_cubic_to(const FT_Vector *control1, const FT_Vector *control2,
                   const FT_Vector *to, void *user)
{
   ITEPath* path = (ITEPath*) (long) user;
   const FT_Vector *vectors[3] = { control1, control2, to };

   return path_append(path, VG_CUBIC_TO, vectors);
}

static ITEPath*
convert_outline_glyph(FT_GlyphSlot glyph)
{
	FT_Outline_Funcs funcs = 
	{
		decompose_move_to,
		decompose_line_to,
		decompose_conic_to,
		decompose_cubic_to,
		0, 0
	};
	ITEPath* path;

	path = vgCreatePath(
		VG_PATH_FORMAT_STANDARD,
		VG_PATH_DATATYPE_F, 
		1.0f, 
		0.0f, 
		0, 
		glyph->outline.n_points,
		VG_PATH_CAPABILITY_ALL);

	if ( FT_Outline_Decompose(&glyph->outline, &funcs, (void *) (long) path) )
	{
		vgDestroyPath(path);
		path = 0;
	}

	return (ITEPath*) path;
}
