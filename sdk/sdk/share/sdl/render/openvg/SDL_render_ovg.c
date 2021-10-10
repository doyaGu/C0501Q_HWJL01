/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h" 
 
#if SDL_VIDEO_RENDER_OVG && !SDL_RENDER_DISABLED

#include "SDL_hints.h"
#include "SDL_openvg.h"
#include "../SDL_sysrender.h"

#ifndef CFG_WIN32_SIMULATOR
    //#define M2d_EXTEND
#endif

/* OpenVG 1.1 renderer implementation */

static const float inv255f = 1.0f / 255.0f;

static SDL_Renderer *OVG_CreateRenderer(SDL_Window * window, Uint32 flags);
static void OVG_WindowEvent(SDL_Renderer * renderer,
                             const SDL_WindowEvent *event);
static int OVG_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int OVG_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                              const SDL_Rect * rect, const void *pixels,
                              int pitch);
static int OVG_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                            const SDL_Rect * rect, void **pixels, int *pitch);
static void OVG_UnlockTexture(SDL_Renderer * renderer,
                               SDL_Texture * texture);
static int OVG_UpdateViewport(SDL_Renderer * renderer);
static int OVG_RenderClear(SDL_Renderer * renderer);
static int OVG_RenderDrawPoints(SDL_Renderer * renderer,
                                 const SDL_Point * points, int count);
static int OVG_RenderDrawLines(SDL_Renderer * renderer,
                                const SDL_Point * points, int count);
static int OVG_RenderFillRects(SDL_Renderer * renderer,
                                const SDL_Rect * rects, int count);
static int OVG_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                           const SDL_Rect * srcrect,
                           const SDL_Rect * dstrect);
static int OVG_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                         const SDL_Rect * srcrect, const SDL_Rect * dstrect,
                         const double angle, const SDL_Point *center, const SDL_RendererFlip flip);
static void OVG_RenderPresent(SDL_Renderer * renderer);
static void OVG_DestroyTexture(SDL_Renderer * renderer,
                                SDL_Texture * texture);
static void OVG_DestroyRenderer(SDL_Renderer * renderer);


SDL_RenderDriver OVG_RenderDriver = {
    OVG_CreateRenderer,
    {
     "openvg",
     (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
     7,
     {
      SDL_PIXELFORMAT_RGB565,
      SDL_PIXELFORMAT_RGB888,
      SDL_PIXELFORMAT_BGR888,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_PIXELFORMAT_RGBA8888,
      SDL_PIXELFORMAT_ABGR8888,
      SDL_PIXELFORMAT_BGRA8888
     },
     0,
     0}
};

typedef struct
{
    SDL_GLContext context;
    struct {
        Uint32 color;
        int blendMode;
        VGPaint paint;
    } current;

} OVG_RenderData;

typedef struct
{
    VGImage texture;
    VGint texw;
    VGint texh;
    VGImageFormat format;
    void *pixels;
    int pitch;
} OVG_TextureData;

static void
OVG_SetError(const char *prefix, VGErrorCode result)
{
    const char *error;

    switch (result) {
    case VG_NO_ERROR:
        error = "VG_NO_ERROR";
        break;
    case VG_BAD_HANDLE_ERROR:
        error = "VG_BAD_HANDLE_ERROR";
        break;
    case VG_ILLEGAL_ARGUMENT_ERROR:
        error = "VG_ILLEGAL_ARGUMENT_ERROR";
        break;
    case VG_OUT_OF_MEMORY_ERROR:
        error = "VG_OUT_OF_MEMORY_ERROR";
        break;
    case VG_PATH_CAPABILITY_ERROR:
        error = "VG_PATH_CAPABILITY_ERROR";
        break;
    case VG_UNSUPPORTED_IMAGE_FORMAT_ERROR:
        error = "VG_UNSUPPORTED_IMAGE_FORMAT_ERROR";
        break;
    case VG_UNSUPPORTED_PATH_FORMAT_ERROR:
        error = "VG_UNSUPPORTED_PATH_FORMAT_ERROR";
        break;
    case VG_IMAGE_IN_USE_ERROR:
        error = "VG_IMAGE_IN_USE_ERROR";
        break;
    case VG_NO_CONTEXT_ERROR:
        error = "VG_NO_CONTEXT_ERROR";
        break;
    default:
        error = "UNKNOWN";
        break;
    }
    SDL_SetError("%s: %s", prefix, error);
}

static SDL_GLContext SDL_CurrentContext = NULL;

static int
OVG_ActivateRenderer(SDL_Renderer * renderer)
{
    OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;

    if (SDL_CurrentContext != data->context) {
        if (SDL_GL_MakeCurrent(renderer->window, data->context) < 0) {
            return -1;
        }
        SDL_CurrentContext = data->context;

        OVG_UpdateViewport(renderer);
    }
    return 0;
}

/* This is called if we need to invalidate all of the SDL OpenVG state */
static void
OVG_ResetState(SDL_Renderer *renderer)
{
    OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;

    if (SDL_CurrentContext == data->context) {
        OVG_UpdateViewport(renderer);
    } else {
        OVG_ActivateRenderer(renderer);
    }

    data->current.color = 0;
    data->current.blendMode = -1;
	data->current.paint = vgCreatePaint();

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadIdentity();
}

SDL_Renderer *
OVG_CreateRenderer(SDL_Window * window, Uint32 flags)
{
    SDL_Renderer *renderer;
    OVG_RenderData *data;

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (OVG_RenderData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        OVG_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = OVG_WindowEvent;
    renderer->CreateTexture = OVG_CreateTexture;
    renderer->UpdateTexture = OVG_UpdateTexture;
    renderer->LockTexture = OVG_LockTexture;
    renderer->UnlockTexture = OVG_UnlockTexture;
    renderer->UpdateViewport = OVG_UpdateViewport;
    renderer->RenderClear = OVG_RenderClear;
    renderer->RenderDrawPoints = OVG_RenderDrawPoints;
    renderer->RenderDrawLines = OVG_RenderDrawLines;
    renderer->RenderFillRects = OVG_RenderFillRects;
    renderer->RenderCopy = OVG_RenderCopy;
    renderer->RenderCopyEx = OVG_RenderCopyEx;
    renderer->RenderPresent = OVG_RenderPresent;
    renderer->DestroyTexture = OVG_DestroyTexture;
    renderer->DestroyRenderer = OVG_DestroyRenderer;
    renderer->info = OVG_RenderDriver.info;
    renderer->info.flags = SDL_RENDERER_ACCELERATED;
    renderer->driverdata = data;

    data->context = SDL_GL_CreateContext(window);
    if (!data->context) {
        OVG_DestroyRenderer(renderer);
        return NULL;
    }
    if (SDL_GL_MakeCurrent(window, data->context) < 0) {
        OVG_DestroyRenderer(renderer);
        return NULL;
    }

    if (flags & SDL_RENDERER_PRESENTVSYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }
    if (SDL_GL_GetSwapInterval() > 0) {
        renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    renderer->info.max_texture_width = vgGeti(VG_MAX_IMAGE_WIDTH);
    renderer->info.max_texture_height = vgGeti(VG_MAX_IMAGE_HEIGHT);

    /* Set up parameters for rendering */
	renderer->window = window;
    OVG_ResetState(renderer);

    return renderer;
}

static void
OVG_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        /* Rebind the context to the window area and update matrices */
        SDL_CurrentContext = NULL;
    }
}

static int
OVG_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    OVG_TextureData *data;
    VGImageFormat format;
    VGErrorCode result;

    OVG_ActivateRenderer(renderer);

    switch (texture->format) {
    case SDL_PIXELFORMAT_RGB565:
        format = VG_sRGB_565;
        break;
    case SDL_PIXELFORMAT_RGB888:
        format = VG_sRGBX_8888;
        break;
    case SDL_PIXELFORMAT_BGR888:
        format = VG_sBGRX_8888;
        break;
    case SDL_PIXELFORMAT_ARGB8888:
        format = VG_sARGB_8888;
        break;
    case SDL_PIXELFORMAT_RGBA8888:
        format = VG_sRGBA_8888;
        break;
    case SDL_PIXELFORMAT_ABGR8888:
        format = VG_sABGR_8888;
        break;
    case SDL_PIXELFORMAT_BGRA8888:
        format = VG_sBGRA_8888;
        break;
    default:
        SDL_SetError("Texture format not supported");
        return -1;
    }

    data = (OVG_TextureData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        SDL_OutOfMemory();
        return -1;
    }

    if (texture->access == SDL_TEXTUREACCESS_STREAMING) {
        data->pitch = texture->w * SDL_BYTESPERPIXEL(texture->format);
        data->pixels = SDL_calloc(1, texture->h * data->pitch);
        if (!data->pixels) {
            SDL_OutOfMemory();
            SDL_free(data);
            return -1;
        }
    }

    texture->driverdata = data;
    
    vgGetError();
    
    data->texw = texture->w;
    data->texh = texture->h;
    data->format = format;
    data->texture = vgCreateImage(format, texture->w, texture->h, VG_IMAGE_QUALITY_NONANTIALIASED);

    result = vgGetError();
    if (result != VG_NO_ERROR) {
        OVG_SetError("vgCreateImage()", result);
        return -1;
    }
    return 0;
}

static int
OVG_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                   const SDL_Rect * rect, const void *pixels, int pitch)
{
    OVG_TextureData *data = (OVG_TextureData *) texture->driverdata;
    Uint8 *blob = NULL;
    Uint8 *src;
    int srcPitch;
    int y;

    OVG_ActivateRenderer(renderer);

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0)
        return 0;

    /* Reformat the texture data into a tightly packed array */
    srcPitch = rect->w * SDL_BYTESPERPIXEL(texture->format);
    src = (Uint8 *)pixels;
    if (pitch != srcPitch)
    {
        blob = (Uint8 *)SDL_malloc(srcPitch * rect->h);
        if (!blob)
        {
            SDL_OutOfMemory();
            return -1;
        }
        src = blob;
        for (y = 0; y < rect->h; ++y)
        {
            SDL_memcpy(src, pixels, srcPitch);
            src += srcPitch;
            pixels = (Uint8 *)pixels + pitch;
        }
        src = blob;
    }

    /* Create a texture subimage with the supplied data */
    vgGetError();
	
	vgImageSubData(data->texture, src, srcPitch, data->format, rect->x, rect->y, rect->w, rect->h);
    
    if (blob) {
        SDL_free(blob);
    }

    if (vgGetError() != VG_NO_ERROR)
    {
        SDL_SetError("Failed to update texture");
        return -1;
    }
    return 0;
}

static int
OVG_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                 const SDL_Rect * rect, void **pixels, int *pitch)
{
    OVG_TextureData *data = (OVG_TextureData *) texture->driverdata;

    *pixels =
        (void *) ((Uint8 *) data->pixels + rect->y * data->pitch +
                  rect->x * SDL_BYTESPERPIXEL(texture->format));
    *pitch = data->pitch;
    return 0;
}

static void
OVG_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    OVG_TextureData *data = (OVG_TextureData *) texture->driverdata;
    SDL_Rect rect;

    /* We do whole texture updates, at least for now */
    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    OVG_UpdateTexture(renderer, texture, &rect, data->pixels, data->pitch);
}

static int
OVG_UpdateViewport(SDL_Renderer * renderer)
{
    OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;

    if (SDL_CurrentContext != data->context) {
        /* We'll update the viewport after we rebind the context */
        return 0;
    }

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgLoadIdentity();
    return 0;
}

static void
OVG_SetColor(OVG_RenderData * data, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint32 color = ((r << 24) | (g << 16) | (b << 8) | a);

    if (color != data->current.color) {
		vgSetParameteri(data->current.paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
		vgSetColor(data->current.paint, color);
        data->current.color = color;
    }
}

static void
OVG_SetBlendMode(OVG_RenderData * data, int blendMode)
{
    if (blendMode != data->current.blendMode) {
        switch (blendMode) {
        case SDL_BLENDMODE_NONE:
            vgSeti(VG_BLEND_MODE, VG_BLEND_SRC);
            break;
        case SDL_BLENDMODE_BLEND:
            vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);
            break;
        case SDL_BLENDMODE_ADD:
            vgSeti(VG_BLEND_MODE, VG_BLEND_ADDITIVE);
            break;
        case SDL_BLENDMODE_MOD:
            vgSeti(VG_BLEND_MODE, VG_BLEND_MULTIPLY);
            break;
        }
        data->current.blendMode = blendMode;
    }
}

static void
OVG_SetDrawingState(SDL_Renderer * renderer)
{
    OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;

    OVG_ActivateRenderer(renderer);

    OVG_SetColor(data, renderer->r,
                       renderer->g,
                       renderer->b,
                       renderer->a);

    OVG_SetBlendMode(data, renderer->blendMode);
}

static int
OVG_RenderClear(SDL_Renderer * renderer)
{
    VGfloat clearColor[4];

    OVG_ActivateRenderer(renderer);

    clearColor[0] = renderer->r * inv255f;
    clearColor[1] = renderer->g * inv255f;
    clearColor[2] = renderer->b * inv255f;
    clearColor[3] = renderer->a * inv255f;

	//todo: transferblock 0xF0

    vgSetfv(VG_CLEAR_COLOR, 4, clearColor);
	#ifdef M2d_EXTEND
        vgM2dTransferBlock(0,0,renderer->viewport.w,renderer->viewport.h,NULL,0,0,0xF0);
	#else
	    vgClear(0, 0, renderer->viewport.w, renderer->viewport.h);
	#endif

    return 0;
}

static int
OVG_RenderDrawPoints(SDL_Renderer * renderer, const SDL_Point * points,
                      int count)
{
	OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;
    int i;
	VGubyte segments[2] = { VG_MOVE_TO_ABS, VG_LINE_TO_ABS };
    VGPath path;

    OVG_SetDrawingState(renderer);

    path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                        VG_PATH_DATATYPE_S_16,
                        1.0f, 0.0f, 2, 2,
                        VG_PATH_CAPABILITY_APPEND_TO);

	vgSetf(VG_STROKE_LINE_WIDTH, 1.0f);
	vgSetPaint(data->current.paint, VG_STROKE_PATH);

	for (i = 0; i < count; ++i) {
        VGshort vertices[4];
		vertices[0] = (VGshort)points[i].x;
        vertices[1] = (VGshort)points[i].y;
		vertices[2] = (VGshort)points[i].x+1;
        vertices[3] = (VGshort)points[i].y;

		vgAppendPathData(path, 2, segments, vertices);
		vgRemovePathCapabilities(path, VG_PATH_CAPABILITY_APPEND_TO);
		vgDrawPath(path, VG_STROKE_PATH);
		vgClearPath(path, VG_PATH_CAPABILITY_APPEND_TO);
    }
	vgDestroyPath(path);

    return 0;
}

static int
OVG_RenderDrawLines(SDL_Renderer * renderer, const SDL_Point * points,
                     int count)
{
	OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;
    int i;
	VGubyte segments[2] = { VG_MOVE_TO_ABS, VG_LINE_TO_ABS };
    VGPath path;

    OVG_SetDrawingState(renderer);

    path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
                        VG_PATH_DATATYPE_S_16,
                        1.0f, 0.0f, 2, 2,
                        VG_PATH_CAPABILITY_APPEND_TO);

	vgSetf(VG_STROKE_LINE_WIDTH, 1.0f);
	vgSetPaint(data->current.paint, VG_STROKE_PATH);

	for (i = 0; i < count; i+=2) {
        VGshort vertices[4];
		vertices[0] = (VGshort)points[i].x;
        vertices[1] = (VGshort)points[i].y;
		vertices[2] = (VGshort)points[i+1].x;
        vertices[3] = (VGshort)points[i+1].y;

		vgAppendPathData(path, 2, segments, vertices);
		vgRemovePathCapabilities(path, VG_PATH_CAPABILITY_APPEND_TO);
		vgDrawPath(path, VG_STROKE_PATH);
		vgClearPath(path, VG_PATH_CAPABILITY_APPEND_TO);
    }
	vgDestroyPath(path);

    return 0;
}

static int
OVG_RenderFillRects(SDL_Renderer * renderer, const SDL_Rect * rects,
                     int count)
{
    OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;
    int i;
    VGPath path;
	VGubyte segments[5] = { VG_MOVE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_CLOSE_PATH };

	OVG_SetDrawingState(renderer);

	path = vgCreatePath(VG_PATH_FORMAT_STANDARD,
	                        VG_PATH_DATATYPE_S_16,
	                        1.0f, 0.0f, 5, 4,
	                        VG_PATH_CAPABILITY_APPEND_TO);

    vgSetPaint(data->current.paint, VG_FILL_PATH);

#ifdef M2d_EXTEND
	if(renderer->blendMode == SDL_BLENDMODE_NONE)
    {
        VGfloat clearColor[4];

		OVG_SetDrawingState(renderer);

		clearColor[0] = renderer->r * inv255f;
		clearColor[1] = renderer->g * inv255f;
		clearColor[2] = renderer->b * inv255f;
		clearColor[3] = renderer->a * inv255f;
		vgSetfv(VG_CLEAR_COLOR, 4, clearColor);

		for (i = 0; i < count; ++i) {
			  const SDL_Rect *rect = &rects[i]; 
#if 0
			  if(rect->x >= 0 && rect->y >= 0 && rect->w >= 0 && rect->h >=0)
			  {
                    SDL_Rect viewportRect;
                    viewportRect.x = rect->x;
					viewportRect.y = rect->y;
                    viewportRect.w = rect->w;
					viewportRect.h = rect->h;

					if(rect->x < renderer->viewport.x)
						  viewportRect.x = renderer->viewport.x;

					if(rect->y < renderer->viewport.y)
						  viewportRect.y = renderer->viewport.y;

					if(rect->x + rect->w > renderer->viewport.x + renderer->viewport.w)
						  viewportRect.w = renderer->viewport.x + renderer->viewport.w - viewportRect.x;

					if(rect->y + rect->h > renderer->viewport.y + renderer->viewport.h)
						  viewportRect.h = renderer->viewport.y + renderer->viewport.h - viewportRect.y;
						

					vgM2dTransferBlock(viewportRect.x,viewportRect.y,viewportRect.w,viewportRect.h,NULL,0,0,0xF0);
			  }
			  else
#endif			  	
			  {
				  	VGshort minx = rect->x;
			        VGshort maxx = rect->x + rect->w;
			        VGshort miny = rect->y;
			        VGshort maxy = rect->y + rect->h;
			        VGshort vertices[8];
			        vertices[0] = minx;
			        vertices[1] = miny;
			        vertices[2] = maxx;
			        vertices[3] = miny;
			        vertices[4] = maxx;
			        vertices[5] = maxy;
			        vertices[6] = minx;
			        vertices[7] = maxy;

					vgAppendPathData(path, 5, segments, vertices);
					vgRemovePathCapabilities(path, VG_PATH_CAPABILITY_APPEND_TO);
					vgDrawPath(path, VG_FILL_PATH);
					vgClearPath(path, VG_PATH_CAPABILITY_APPEND_TO);
			  }
		}
    }
    else	
#else		
    {

	    for (i = 0; i < count; ++i) {
	        const SDL_Rect *rect = &rects[i];
	        VGshort minx = rect->x;
	        VGshort maxx = rect->x + rect->w;
	        VGshort miny = rect->y;
	        VGshort maxy = rect->y + rect->h;
	        VGshort vertices[8];
	        vertices[0] = minx;
	        vertices[1] = miny;
	        vertices[2] = maxx;
	        vertices[3] = miny;
	        vertices[4] = maxx;
	        vertices[5] = maxy;
	        vertices[6] = minx;
	        vertices[7] = maxy;

			vgAppendPathData(path, 5, segments, vertices);
			vgRemovePathCapabilities(path, VG_PATH_CAPABILITY_APPEND_TO);
			vgDrawPath(path, VG_FILL_PATH);
			vgClearPath(path, VG_PATH_CAPABILITY_APPEND_TO);
	    }
    }
#endif

	vgDestroyPath(path);

    return 0;
}

static int
OVG_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_Rect * dstrect)
{
	OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;
    OVG_TextureData *texturedata = (OVG_TextureData *) texture->driverdata;
	VGfloat scalex, scaley;
	VGImage childImage;
    int w, h;
	
    scalex = (VGfloat)dstrect->w / srcrect->w;
	scaley = (VGfloat)dstrect->h / srcrect->h;
    SDL_GetWindowSize(renderer->window, &w, &h);

    //todo: IF without blend and scale, use fast copy
  #ifdef M2d_EXTEND  
    if(texture->blendMode == SDL_BLENDMODE_NONE && scalex == 1 && scaley == 1)
    {
		OVG_ActivateRenderer(renderer);

		vgM2dSourceCopy(dstrect->x,dstrect->y,dstrect->w,dstrect->h,texturedata->texture,srcrect->x,srcrect->y);
    }
	else
  #else 		
	{
	    OVG_ActivateRenderer(renderer);

        if (texture->modMode) {
            OVG_SetColor(data, texture->r, texture->g, texture->b, texture->a);
        } else {
            OVG_SetColor(data, 255, 255, 255, 255);
        }        
        
        OVG_SetBlendMode(data, texture->blendMode);

		childImage = vgChildImage(texturedata->texture, srcrect->x, srcrect->y, srcrect->w, srcrect->h);
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
		vgLoadIdentity();
		vgScale(scalex, scaley);
		vgTranslate((renderer->viewport.x + dstrect->x) / scalex, (h - (renderer->viewport.y + dstrect->y) - dstrect->h) / scaley);
		vgDrawImage(childImage);
		//vgDestroyImage(childImage);   // WORKAROUND HW OPENVG DRIVR BUG
		vgLoadIdentity();
	}
   #endif
    return 0;
}

static int
OVG_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_Rect * dstrect,
                const double angle, const SDL_Point *center, const SDL_RendererFlip flip)
{
	OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;
    OVG_TextureData *texturedata = (OVG_TextureData *) texture->driverdata;
	VGfloat scalex, scaley;
    VGfloat centerx, centery;
    VGImage childImage;
    int w, h;

    scalex = (VGfloat)dstrect->w / srcrect->w;
	scaley = (VGfloat)dstrect->h / srcrect->h;
    SDL_GetWindowSize(renderer->window, &w, &h);

    OVG_ActivateRenderer(renderer);

    if (texture->modMode) {
        OVG_SetColor(data, texture->r, texture->g, texture->b, texture->a);
    } else {
        OVG_SetColor(data, 255, 255, 255, 255);
    }

    OVG_SetBlendMode(data, texture->blendMode);

    centerx = (VGfloat)center->x;
    centery = (VGfloat)center->y;

	childImage = vgChildImage(texturedata->texture, srcrect->x, srcrect->y, srcrect->w, srcrect->h);
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
	vgLoadIdentity();
	vgScale(scalex, scaley);
	vgTranslate((renderer->viewport.x + dstrect->x + centerx) / scalex, (h - (renderer->viewport.y + dstrect->y + centery) - dstrect->h) / scaley);
    vgRotate((VGfloat)angle);
	vgDrawImage(childImage);
	// vgDestroyImage(childImage);  // WORKAROUND HW OPENVG DRIVR BUG
	vgLoadIdentity();

    return 0;
}

static void
OVG_RenderPresent(SDL_Renderer * renderer)
{
    OVG_ActivateRenderer(renderer);

    SDL_GL_SwapWindow(renderer->window);
}

static void
OVG_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    OVG_TextureData *data = (OVG_TextureData *) texture->driverdata;

    OVG_ActivateRenderer(renderer);

    if (!data) {
        return;
    }
    if (data->texture) {
        vgDestroyImage(data->texture);
    }
    if (data->pixels) {
        SDL_free(data->pixels);
    }
    SDL_free(data);
    texture->driverdata = NULL;
}

static void
OVG_DestroyRenderer(SDL_Renderer * renderer)
{
    OVG_RenderData *data = (OVG_RenderData *) renderer->driverdata;

    if (data) {
		vgDestroyPaint(data->current.paint);
        if (data->context) {
            SDL_GL_DeleteContext(data->context);
        }
        SDL_free(data);
    }
    SDL_free(renderer);
}

#endif /* SDL_VIDEO_RENDER_OVG && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
