/*------------------------------------------------------------------------
 *
 * EGL 1.3
 * -------
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
 * \brief	Simple implementation of EGL 1.3
 * \note	caveats:
			- always renders into the backbuffer and blits it to window (no single buffered rendering)
			- no native Windows or Mac OS X pixmap support
			- no power management events
			- no support for swap interval
 * \todo	what happens in egl functions when eglTerminate has been called but the context and surface are still in use?
 * \todo	OSDeinitMutex should be called in case getEGL fails.
 *//*-------------------------------------------------------------------*/

#include "egl.h"
#include "openvg.h"
#include "iteContext.h"
#include "iteImage.h"
#include "ite/ith.h"
#include "iteHardware.h"

//==============================================================================================

void* OSGetCurrentThreadID(void);
void OSAcquireMutex(void);
void OSReleaseMutex(void);
void OSDeinitMutex(void);

EGLDisplay OSGetDisplay(EGLNativeDisplayType display_id);
void* OSCreateWindowContext(EGLNativeWindowType window);
void OSDestroyWindowContext(void* context);
ITEboolean OSIsWindow(const void* context);
void OSGetWindowSize(const void* context, int* width, int* height);
void OSBlitToWindow(void* context, ITEuint8** data, ITEint width, ITEint height, VGImageFormat format);

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

#define EGL_IF_ERROR(COND, ERRORCODE, RETVAL) \
	if(COND) { eglSetError(ERRORCODE); return RETVAL; } \

#define EGL_RETURN(ERRORCODE, RETVAL) \
	{ \
		eglSetError(ERRORCODE); \
		return RETVAL; \
	}

/*
#define EGL_GET_DISPLAY(DISPLAY, RETVAL) \
	OSAcquireMutex(); \
	EGL* egl = getEGL(); \
    if(!egl) \
    { \
		OSReleaseMutex(); \
		return RETVAL; \
    } \
	RIEGLDisplay* display = egl->getDisplay(DISPLAY); \

#define EGL_GET_EGL(RETVAL) \
	OSAcquireMutex(); \
	EGL* egl = getEGL(); \
    if(!egl) \
    { \
		OSReleaseMutex(); \
		return RETVAL; \
    } \
*/

//==============================================================================================

typedef struct EGLPixconfig
{
	int redSize;
	int greenSize;
	int blueSize;
	int alphaSize;
	int luminanceSize;
	int maskSize;
	int configID;
	VGImageFormat format;
} EGLPixconfig;

#define EGL_NUMCONFIGS		9
const EGLPixconfig pixconfigs[] = {
/*    R  G  B  A  L  Mask ID  format        */	
	{ 8, 8, 8, 8, 0,  8,   0, VG_sABGR_8888},
	{ 8, 8, 8, 0, 0,  8,   1, VG_sXBGR_8888},
	{ 5, 6, 5, 0, 0,  8,   2, VG_sRGB_565},
	{ 5, 5, 5, 1, 0,  8,   3, VG_sRGBA_5551},
	{ 4, 4, 4, 4, 0,  8,   4, VG_sRGBA_4444},
	{ 0, 0, 0, 0, 8,  8,   5, VG_sL_8},
	{ 0, 0, 0, 8, 0,  8,   6, VG_A_8},
	{ 0, 0, 0, 4, 0,  8,   7, VG_A_4},
	{ 0, 0, 0, 0, 1,  8,   8, VG_BW_1},
};

static EGLint m_eglError;
static VGContext* m_context;
static ITESurface* m_surface;

//==============================================================================================

static void eglSetError(EGLint error)
{
	m_eglError = error;
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLint eglGetError(void)
{
    return m_eglError;
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id)
{
    return OSGetDisplay(display_id);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
	iteCreateContext();
	m_context = iteGetContext();

	if(major) *major = 1;
	if(minor) *minor = 2;
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglTerminate(EGLDisplay dpy)
{
	if( m_context )
	{
		iteDestroyContext();
		m_context = NULL;
	}
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

const char *eglQueryString(EGLDisplay dpy, EGLint name)
{
	static const char apis[] = "OpenVG";
	static const char extensions[] = "";
	static const char vendor[] = "Khronos Group";
	static const char version[] = "1.3";
	const char* ret = NULL;
	switch(name)
	{
	case EGL_CLIENT_APIS:
		ret = apis;
		break;

	case EGL_EXTENSIONS:
		ret = extensions;
		break;

	case EGL_VENDOR:
		ret = vendor;
		break;

	case EGL_VERSION:
		ret = version;
		break;

	default:
		EGL_RETURN(EGL_BAD_PARAMETER, NULL);
	}
	EGL_RETURN(EGL_SUCCESS, ret);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
	int i;
	EGL_IF_ERROR(!num_config, EGL_BAD_PARAMETER, EGL_FALSE);
	if(!configs)
	{
		*num_config = EGL_NUMCONFIGS;
		EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
	}
	*num_config = ITE_MIN(config_size, EGL_NUMCONFIGS);

	for(i=0; i<*num_config; i++) 
	{
		configs[i] = (EGLConfig)&pixconfigs[i];
	}
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
	int redSize = 0;
	int greenSize = 0;
	int blueSize = 0;
	int alphaSize = 0;
	int luminanceSize = 0;
	int configID = EGL_DONT_CARE;
	int nconfig;
	int i;
	EGL_IF_ERROR(!num_config, EGL_BAD_PARAMETER, EGL_FALSE);
	*num_config = 0;
	if(!config_size)
		EGL_RETURN(EGL_SUCCESS, EGL_TRUE);

	if(attrib_list)
	{
		for(i=0;attrib_list[i] != EGL_NONE;i+=2)
		{
			switch(attrib_list[i])
			{
			case EGL_RED_SIZE:					//bits of Red in the color buffer
				redSize = attrib_list[i+1];
				break;
			case EGL_GREEN_SIZE:				//bits of Green in the color buffer
				greenSize = attrib_list[i+1];
				break;
			case EGL_BLUE_SIZE:					//bits of Blue in the color buffer
				blueSize = attrib_list[i+1];
				break;
			case EGL_LUMINANCE_SIZE:			//bits of Luminance in the color buffer
				luminanceSize = attrib_list[i+1];
				break;
			case EGL_ALPHA_SIZE:				//bits of Alpha in the color buffer
				alphaSize = attrib_list[i+1];
				break;
			case EGL_CONFIG_ID:					//unique EGLConfig identifier
				configID = attrib_list[i+1];
				break;

			case EGL_ALPHA_MASK_SIZE:			//bits of Alpha in the alpha mask buffer
			case EGL_BUFFER_SIZE:				//no support depth of the color buffer
			case EGL_COLOR_BUFFER_TYPE:			//enum color buffer type (EGL_RGB_BUFFER, EGL_LUMINANCE_BUFFER)
			case EGL_SAMPLE_BUFFERS:			//integer number of multisample buffers
			case EGL_SAMPLES:					//integer number of samples per pixel
				break;

			case EGL_BIND_TO_TEXTURE_RGB:		//boolean True if bindable to RGB textures. (always EGL_FALSE)
			case EGL_BIND_TO_TEXTURE_RGBA:		//boolean True if bindable to RGBA textures. (always EGL_FALSE)
			case EGL_DEPTH_SIZE:				//integer bits of Z in the depth buffer (always 0)
			case EGL_LEVEL:						//integer frame buffer level (always 0)
			case EGL_NATIVE_RENDERABLE:			//boolean EGL TRUE if native rendering APIs can render to surface (always EGL_FALSE)
			case EGL_STENCIL_SIZE:				//integer bits of Stencil in the stencil buffer (always 0)
				if(attrib_list[i+1])
					EGL_RETURN(EGL_SUCCESS, EGL_TRUE);	//not supported
				break;

			case EGL_CONFIG_CAVEAT:				//enum any caveats for the configuration (always EGL_NONE)
			case EGL_NATIVE_VISUAL_TYPE:		//integer native visual type of the associated visual (always EGL_NONE)
				if(attrib_list[i+1] != EGL_NONE)
					EGL_RETURN(EGL_SUCCESS, EGL_TRUE);	//not supported
				break;

			case EGL_MAX_SWAP_INTERVAL:			//integer maximum swap interval (always 1)
			case EGL_MIN_SWAP_INTERVAL:			//integer minimum swap interval (always 1)
				if(attrib_list[i+1] != 1)
					EGL_RETURN(EGL_SUCCESS, EGL_TRUE);	//not supported
				break;

			case EGL_RENDERABLE_TYPE:			//bitmask which client rendering APIs are supported. (always EGL_OPENVG_BIT)
				if(!(attrib_list[i+1] & EGL_OPENVG_BIT))
					EGL_RETURN(EGL_SUCCESS, EGL_TRUE);	//not supported
				break;

			case EGL_SURFACE_TYPE:				//bitmask which types of EGL surfaces are supported. (always EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT | EGL_VG_COLORSPACE_LINEAR_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT)
				break;	//all types are always supported

			case EGL_TRANSPARENT_TYPE:			//enum type of transparency supported (always EGL_NONE)
			case EGL_NATIVE_VISUAL_ID:			//integer handle of corresponding native visual (always 0)
			case EGL_MAX_PBUFFER_WIDTH:			//integer maximum width of pbuffer (always INT_MAX)
			case EGL_MAX_PBUFFER_HEIGHT:		//integer maximum height of pbuffer (always INT_MAX)
			case EGL_MAX_PBUFFER_PIXELS:		//integer maximum size of pbuffer (always INT_MAX)
			case EGL_TRANSPARENT_RED_VALUE:		//integer transparent red value (undefined)
			case EGL_TRANSPARENT_GREEN_VALUE:	//integer transparent green value (undefined)
			case EGL_TRANSPARENT_BLUE_VALUE:	//integer transparent blue value (undefined)
				break;	//ignored

			default:
				EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);	//unknown attribute
			}
		}
	}

	if( configID != EGL_DONT_CARE && configID<EGL_NUMCONFIGS)
	{
		//configs[configID] = (EGLConfig)&pixconfigs[configID];
		configs[0] = (EGLConfig)&pixconfigs[configID];
		*num_config = 1;
		EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
	}

	nconfig = 0;
	for(i = 0; i<EGL_NUMCONFIGS; i++) {
		EGLPixconfig *p = (EGLPixconfig*)&pixconfigs[i];

		//printf("  config %d has red=%d green=%d blue=%d alpha=%d stencil=%d hwformat=%d\n",
		//	i, p->red_bits, p->green_bits, p->blue_bits, p->alpha_bits, 
		//	p->stencil_bits, p->hwformat);

		if (redSize != p->redSize)
			continue;
		if (greenSize != p->greenSize)
			continue;
		if (blueSize != p->blueSize)
			continue;
		if (alphaSize != p->alphaSize)
			continue;
		if (luminanceSize != p->luminanceSize && luminanceSize != EGL_DONT_CARE)
			continue;

		if (configs != NULL) {
			if (nconfig >= config_size)
				break;
			configs[nconfig] = (EGLPixconfig*)p;
		}
		nconfig++;
		//printf("found config %d 0x%x\n", nconfig, EGLCONFIGIDX(i, depth_size));
	}
	*num_config = nconfig;

	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
	EGLPixconfig *c = config;
	EGL_IF_ERROR(!config, EGL_BAD_CONFIG, EGL_FALSE);
	switch(attribute)
	{
	case EGL_BUFFER_SIZE:
		*value = ITE_MAX(c->redSize + c->greenSize + c->blueSize + c->alphaSize, c->luminanceSize + c->alphaSize);
		break;

	case EGL_RED_SIZE:
		*value = c->redSize;
		break;

	case EGL_GREEN_SIZE:
		*value = c->greenSize;
		break;

	case EGL_BLUE_SIZE:
		*value = c->blueSize;
		break;

	case EGL_LUMINANCE_SIZE:
		*value = c->luminanceSize;
		break;

	case EGL_ALPHA_SIZE:
		*value = c->alphaSize;
		break;

	case EGL_ALPHA_MASK_SIZE:
		*value = c->maskSize;
		break;

	case EGL_BIND_TO_TEXTURE_RGB:
	case EGL_BIND_TO_TEXTURE_RGBA:
		*value = EGL_FALSE;
		break;

	case EGL_COLOR_BUFFER_TYPE:
		if(c->redSize)
			*value = EGL_RGB_BUFFER;
		else
			*value = EGL_LUMINANCE_BUFFER;
		break;

	case EGL_CONFIG_CAVEAT:
		*value = EGL_NONE;
		break;

	case EGL_CONFIG_ID:
		*value = c->configID;
		break;

	case EGL_DEPTH_SIZE:
		*value = 0;
		break;

	case EGL_LEVEL:
		*value = 0;
		break;

	case EGL_MAX_PBUFFER_WIDTH:
	case EGL_MAX_PBUFFER_HEIGHT:
		*value = 16384;			//NOTE arbitrary maximum
		break;
		
	case EGL_MAX_PBUFFER_PIXELS:
		*value = 16384*16384;	//NOTE arbitrary maximum
		break;

	case EGL_MAX_SWAP_INTERVAL:
	case EGL_MIN_SWAP_INTERVAL:
		*value = 1;
		break;

	case EGL_NATIVE_RENDERABLE:
		*value = EGL_FALSE;
		break;

	case EGL_NATIVE_VISUAL_ID:
		*value = 0;
		break;

	case EGL_NATIVE_VISUAL_TYPE:
		*value = EGL_NONE;
		break;

	case EGL_RENDERABLE_TYPE:
		*value = EGL_OPENVG_BIT;
		break;

	case EGL_SAMPLE_BUFFERS:
		*value = 0;
		break;

	case EGL_SAMPLES:
		*value = 1;
		break;

	case EGL_STENCIL_SIZE:
		*value = 0;
		break;

	case EGL_SURFACE_TYPE:
		*value = EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT | EGL_VG_COLORSPACE_LINEAR_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT;
		break;

	case EGL_TRANSPARENT_TYPE:
		*value = EGL_NONE;
		break;

	case EGL_TRANSPARENT_RED_VALUE:
	case EGL_TRANSPARENT_GREEN_VALUE:
	case EGL_TRANSPARENT_BLUE_VALUE:
		*value = 0;
		break;

    case EGL_CONFORMANT:
        *value = EGL_OPENVG_BIT;  //TODO return proper value
        break;

	default:
		EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);
	}
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLSurface 
eglCreateWindowSurface(
	EGLDisplay          dpy, 
	EGLConfig           config, 
	EGLNativeWindowType win, 
	const EGLint*       attrib_list)
{
	int renderBuffer = EGL_BACK_BUFFER;
	int colorSpace = EGL_VG_COLORSPACE_sRGB;
	int alphaFormat = EGL_VG_ALPHA_FORMAT_NONPRE;
	int bpp;
	int i;
	void* wc = NULL;
	ITESurface* s = NULL;
	EGLPixconfig* c = config;
	int windowWidth = 0, windowHeight = 0;
	ITEboolean isWindow;

	EGL_IF_ERROR(!config, EGL_BAD_CONFIG, EGL_NO_SURFACE);
	if( c->format == VG_sABGR_8888 || c->format == VG_sXBGR_8888 )
	{
		bpp = 32;
	}
	else if( c->format == VG_sRGB_565 )
	{
		bpp = 16;
	}
	else
	{
	    bpp = 0;
	}
	EGL_IF_ERROR(!bpp, EGL_BAD_CONFIG, EGL_NO_SURFACE);

	if(attrib_list)
	{
		for(i=0; attrib_list[i] != EGL_NONE; i+=2)
		{
			switch(attrib_list[i])
			{
			case EGL_RENDER_BUFFER:
				renderBuffer = attrib_list[i+1];
				break;

			case EGL_VG_COLORSPACE:
				colorSpace = attrib_list[i+1];
				break;

			case EGL_VG_ALPHA_FORMAT:
				alphaFormat = attrib_list[i+1];
				break;

			default:
				EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);
			}
		}
	}
	//we ignore the renderBuffer parameter since we can only render to double buffered surfaces

	//TODO If the attributes of win do not correspond to config, then an EGL BAD MATCH error is generated.
	//TODO If there is already an EGLConfig associated with win (as a result of a previous eglCreateWindowSurface call), then an EGL BAD ALLOC error is generated

	wc = OSCreateWindowContext(win);
	ITE_ASSERT(wc);
	OSGetWindowSize(wc, &windowWidth, &windowHeight);
	isWindow = OSIsWindow(wc);
	if(windowWidth <= 0 || windowHeight <= 0 || !isWindow)
	{
		OSDestroyWindowContext(wc);
		EGL_IF_ERROR(!isWindow, EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
		EGL_IF_ERROR(windowWidth <= 0 || windowHeight <= 0, EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
	}
	s = iteContextAllocSurface(wc, renderBuffer, c->format, windowWidth, windowHeight, (c->maskSize > 0));
	if(!s) 
	{
		OSDestroyWindowContext(wc);
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
	}

#if 0
	s = (ITESurface*)SYS_Malloc(sizeof(ITESurface));
	if(!s) 
	{
		OSDestroyWindowContext(wc);
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
	}
	SYS_MemorySet(s, 0x00, sizeof(ITESurface));
	
	s->OSWindowContext = wc;
	s->renderBuffer = renderBuffer;
	CSET(color,0,0,0,0);
	s->colorImage = (ITEImage*)vgCreateImage(c->format, windowWidth, windowHeight, 0);
	EGL_IF_ERROR(!s->colorImage, EGL_BAD_ALLOC, EGL_NO_SURFACE);
	iteSetImage(s->colorImage, 0, 0, windowWidth, windowHeight, color);
	if( c->maskSize )
	{
		CSET(color,255,255,255,255);
		// ToDo: Need check, should not call vgCreateImage() to create a mask.
		s->maskImage = (ITEImage*)vgCreateImage(VG_A_8, windowWidth, windowHeight, 0);
		if ( !s->maskImage )
		{
			vgDestroyImage((VGImage)s->colorImage);
			EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
		}
		iteSetImage(s->maskImage, 0, 0, windowWidth, windowHeight, color);
	}

	/* Create coverage image for hardware */
	s->coverageImageA = (ITEImage*)vgCreateImage(VG_sRGB_565, windowWidth, windowHeight, 0);
	s->coverageImageB = (ITEImage*)vgCreateImage(VG_sRGB_565, windowWidth, windowHeight, 0);
	if ( !s->coverageImageA || !s->coverageImageB)
	{
		if ( s->coverageImageA )
		{
			vgDestroyImage((VGImage)s->coverageImageA);
		}
		if ( s->coverageImageB )
		{
			vgDestroyImage((VGImage)s->coverageImageB);
		}
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
	}
	s->coverageIndex = 0;

	/* Create valid image for hardware */
	s->validImage = (ITEImage*)vgCreateImage(VG_A_1, windowWidth, windowHeight, 0);
	if ( !s->validImage )
	{
		vgDestroyImage((VGImage)s->validImage);
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
	}
#endif
	
	EGL_RETURN(EGL_SUCCESS, (EGLSurface)s);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
	int width = 0, height = 0;
	ITEboolean largestPbuffer = ITE_FALSE;
	int colorSpace = EGL_VG_COLORSPACE_sRGB;
	int alphaFormat = EGL_VG_ALPHA_FORMAT_NONPRE;
	int bpp;
	int i;
	ITESurface* s = NULL;
	EGLPixconfig* c = config;

	EGL_IF_ERROR(!config, EGL_BAD_CONFIG, EGL_NO_SURFACE);
	if(   c->format == VG_sRGB_565 
	   || c->format == VG_sRGBA_5551 
	   || c->format == VG_sRGBA_4444)
	{
		bpp = 16;
	}
	else if( c->format == VG_A_8 )
	{
	    bpp = 8;
	}
	else if( c->format == VG_BW_1 )
	{
	    bpp = 1;
	}
	else
	{
		bpp = 32;
	}

	if(attrib_list)
	{
		for(i=0;attrib_list[i] != EGL_NONE;i+=2)
		{
			switch(attrib_list[i])
			{
			case EGL_WIDTH:
				width = attrib_list[i+1];
				break;

			case EGL_HEIGHT:
				height = attrib_list[i+1];
				break;

			case EGL_LARGEST_PBUFFER:
				largestPbuffer = attrib_list[i+1] ? EGL_TRUE : EGL_FALSE;
				break;

			case EGL_VG_COLORSPACE:
				colorSpace = attrib_list[i+1];
				break;

			case EGL_VG_ALPHA_FORMAT:
				alphaFormat = attrib_list[i+1];
				break;

			case EGL_TEXTURE_FORMAT:	//config doesn't support OpenGL ES
			case EGL_TEXTURE_TARGET:	//config doesn't support OpenGL ES
			case EGL_MIPMAP_TEXTURE:	//config doesn't support OpenGL ES
			default:
				EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);
			break;
			}
		}
	}
	EGL_IF_ERROR(width <= 0 || height <= 0, EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);
	s = iteContextAllocSurface(NULL, EGL_BACK_BUFFER, c->format, width, height, (c->maskSize > 0));
	if(!s) 
	{
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
	}

#if 0
	s = (ITESurface*)SYS_Malloc(sizeof(ITESurface));
	if(!s) 
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
	s->renderBuffer = EGL_BACK_BUFFER;
	CSET(color,0,0,0,0);
	s->colorImage = (ITEImage*)vgCreateImage(c->format, width, height,0);
	iteSetImage(s->colorImage, 0, 0, width, height, color);
	if( c->maskSize )
	{
		CSET(color,255,255,255,255);
		s->maskImage = (ITEImage*)vgCreateImage(VG_A_8, width, height,0);
		iteSetImage(s->maskImage, 0, 0, width, height, color);
	}
#endif

	EGL_RETURN(EGL_SUCCESS, (EGLSurface)s);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
	EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
	int           width          = 0;
	int           height         = 0;
	ITEboolean    largestPbuffer = ITE_FALSE;
	int           colorSpace     = EGL_VG_COLORSPACE_sRGB;
	int           alphaFormat    = EGL_VG_ALPHA_FORMAT_NONPRE;
	int           bpp            = 0;
	int           i              = 0;
	ITESurface*   s              = NULL;
	EGLPixconfig* c              = config;

	EGL_IF_ERROR(!config, EGL_BAD_CONFIG, EGL_NO_SURFACE);
	if(   c->format == VG_sRGB_565 
	   || c->format == VG_sRGBA_5551 
	   || c->format == VG_sRGBA_4444)
	{
		bpp = 16;
	}
	else if( c->format == VG_A_8 )
	{
	    bpp = 8;
	}
	else if( c->format == VG_BW_1 )
	{
	    bpp = 1;
	}
	else
	{
		bpp = 32;
	}

	if(attrib_list)
	{
		for(i=0;attrib_list[i] != EGL_NONE;i+=2)
		{
			switch(attrib_list[i])
			{
			case EGL_WIDTH:
				width = attrib_list[i+1];
				break;

			case EGL_HEIGHT:
				height = attrib_list[i+1];
				break;

			case EGL_LARGEST_PBUFFER:
				largestPbuffer = attrib_list[i+1] ? EGL_TRUE : EGL_FALSE;
				break;

			case EGL_VG_COLORSPACE:
				colorSpace = attrib_list[i+1];
				break;

			case EGL_VG_ALPHA_FORMAT:
				alphaFormat = attrib_list[i+1];
				break;

			case EGL_TEXTURE_FORMAT:	//config doesn't support OpenGL ES
			case EGL_TEXTURE_TARGET:	//config doesn't support OpenGL ES
			case EGL_MIPMAP_TEXTURE:	//config doesn't support OpenGL ES
			default:
				EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);
			break;
			}
		}
	}
	EGL_IF_ERROR(width <= 0 || height <= 0, EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);
	s = iteContextAllocSurface(NULL, EGL_BACK_BUFFER, c->format, width, height, (c->maskSize > 0));
	if(!s) 
	{
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
	}

	EGL_RETURN(EGL_SUCCESS, (EGLSurface)s);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
	ITESurface* iteSurface = (ITESurface*)surface;
	
	EGL_IF_ERROR(!surface, EGL_BAD_SURFACE, EGL_FALSE);

	if ( iteSurface->OSWindowContext )   { OSDestroyWindowContext(iteSurface->OSWindowContext); }

	if ( iteSurface->colorImage )        { iteDestroyImage(iteSurface->colorImage); }
	if ( iteSurface->maskImage )         { iteDestroyImage(iteSurface->maskImage); }
	if ( iteSurface->validImageA )       { iteDestroyImage(iteSurface->validImageA); }
	if ( iteSurface->validImageB )       { iteDestroyImage(iteSurface->validImageB); }
	if ( iteSurface->coverageImageA )    { iteDestroyImage(iteSurface->coverageImageA); }
	if ( iteSurface->coverageImageB )    { iteDestroyImage(iteSurface->coverageImageB); }
	if ( iteSurface->ImagePatternMerge ) { iteDestroyImage(iteSurface->ImagePatternMerge); }
	if ( iteSurface->image4X )           { iteDestroyImage(iteSurface->image4X); }
	if ( iteSurface->imageQuarter )      { iteDestroyImage(iteSurface->imageQuarter); }
	
	free((ITESurface*)surface);
	surface = NULL;
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
	ITESurface* s = (ITESurface*)surface;
	EGL_IF_ERROR(!surface, EGL_BAD_SURFACE, EGL_FALSE);

	switch(attribute)
	{
	case EGL_VG_ALPHA_FORMAT:
		*value = (EGL_TRUE) ? EGL_VG_ALPHA_FORMAT_PRE : EGL_VG_ALPHA_FORMAT_NONPRE;
		break;

	case EGL_VG_COLORSPACE:
		*value = (EGL_TRUE) ? EGL_VG_COLORSPACE_sRGB : EGL_VG_COLORSPACE_LINEAR;
		break;

	case EGL_CONFIG_ID:
		*value = EGL_DONT_CARE;
		break;

	case EGL_HEIGHT:
		*value = s->colorImage->height;
		break;

	case EGL_HORIZONTAL_RESOLUTION:
		*value = EGL_UNKNOWN;			//TODO Horizontal dot pitch
		break;

	case EGL_LARGEST_PBUFFER:
		if(!s->OSWindowContext)
			*value = EGL_FALSE;
		break;

	case EGL_MIPMAP_TEXTURE:
		if(!s->OSWindowContext)
			*value = EGL_FALSE;
		break;

	case EGL_MIPMAP_LEVEL:
		if(!s->OSWindowContext)
			*value = 0;
		break;

	case EGL_PIXEL_ASPECT_RATIO:
		*value = EGL_UNKNOWN;			//TODO Display aspect ratio
		break;

	case EGL_RENDER_BUFFER:
		*value = s->renderBuffer;
		break;

	case EGL_SWAP_BEHAVIOR:
		*value = EGL_BUFFER_PRESERVED;
		break;

	case EGL_TEXTURE_FORMAT:
		if(!s->OSWindowContext)
			*value = EGL_NO_TEXTURE;
		break;

	case EGL_TEXTURE_TARGET:
		if(!s->OSWindowContext)
			*value = EGL_NO_TEXTURE;
		break;

	case EGL_VERTICAL_RESOLUTION:
		*value = EGL_UNKNOWN;			//TODO Vertical dot pitch
		break;

	case EGL_WIDTH:
		*value = s->colorImage->width;
		break;

	default:
		EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);
	}
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLContext 
eglCreateContext(
	EGLDisplay    dpy, 
	EGLConfig     config, 
	EGLContext    share_context, 
	const EGLint* attrib_list)
{
/*
	EGL_GET_DISPLAY(dpy, EGL_NO_CONTEXT);
	EGL_IF_ERROR(!display, EGL_NOT_INITIALIZED, EGL_NO_CONTEXT);
	EGL_IF_ERROR(!display->configExists(config), EGL_BAD_CONFIG, EGL_NO_CONTEXT);

	RI_UNREF(attrib_list);

	RIEGLThread* thread = egl->getThread();
	if(!thread)
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_CONTEXT);

	//creation of OpenGL ES contexts is not allowed in this implementation
	if(thread->getBoundAPI() != EGL_OPENVG_API)
		EGL_RETURN(EGL_BAD_MATCH, EGL_NO_CONTEXT);

    OpenVGRI::VGContext* vgctx = NULL;
	RIEGLContext* c = NULL;
	try
	{
		vgctx = RI_NEW(OpenVGRI::VGContext, (share_context ? ((RIEGLContext*)share_context)->getVGContext() : NULL));	//throws bad_alloc
		c = RI_NEW(RIEGLContext, (vgctx, config));	//throws bad_alloc
		c->addReference();
		display->addContext(c);	//throws bad_alloc
	}
	catch(std::bad_alloc)
	{
        RI_DELETE(vgctx);
        RI_DELETE(c);
		EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_CONTEXT);
	}

	EGL_RETURN(EGL_SUCCESS, (EGLContext)c);
*/

	EGL_RETURN(EGL_SUCCESS, (EGLContext)m_context);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
	ITESurface* newSurface = (ITESurface*)draw;
	VGContext*  newContext = (VGContext*)ctx;

    if ( newSurface )
	{
		m_surface = newSurface;
	}

	if ( newContext )
	{
		m_context = newContext;
		m_context->surface = newSurface;
	}

	/*
	m_surface = (ITESurface*)draw;
	m_context = (VGContext*)ctx;
	if( m_context )
	{
		m_context->surface = m_surface;
		m_context->updateFlag.surfaceFlag = VG_TRUE;
		m_context->updateFlag.maskFlag = VG_TRUE;
	}
	*/
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLContext eglGetCurrentContext()
{
	EGL_RETURN(EGL_SUCCESS, (EGLContext)m_context);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLSurface eglGetCurrentSurface(EGLint readdraw)
{
	EGL_RETURN(EGL_SUCCESS, m_surface);
}

/*-------------------------------------------------------------------*//*!
* \brief	Returns the current display
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLDisplay eglGetCurrentDisplay(void)
{
    int ret = 1;
	EGL_RETURN(EGL_SUCCESS, (EGLDisplay)ret);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint* value)
{
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglBindAPI(EGLenum api)
{
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLenum eglQueryAPI(void)
{
	EGL_RETURN(EGL_SUCCESS, EGL_NONE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglWaitClient()
{
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	Waits for OpenGL ES
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglWaitGL(void)
{
	return EGL_TRUE;
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		We don't support native rendering
*//*-------------------------------------------------------------------*/

EGLBoolean eglWaitNative(EGLint engine)
{
	return EGL_TRUE;
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
	int windowWidth = 0, windowHeight = 0;
	ITESurface* s = (ITESurface*)surface;
	ITEImage *im = s->colorImage;
	EGL_IF_ERROR(s != m_surface, EGL_BAD_SURFACE, EGL_FALSE);
	EGL_IF_ERROR(!s->OSWindowContext, EGL_BAD_NATIVE_WINDOW, EGL_FALSE);

	vgFlush();
	OSBlitToWindow(s->OSWindowContext, &im->data, im->width, im->height, im->vgformat);

	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		We support only swap interval one
*//*-------------------------------------------------------------------*/

EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
	EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

typedef void RI_Proc();

void (*eglGetProcAddress(const char *procname))()
{
	return NULL;
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

EGLBoolean eglReleaseThread(void)
{
	if(m_surface)
	{
		if(m_surface->colorImage)
		{
			if(m_surface->colorImage->data)
			{
				VG_VMemFree((uint32_t)m_surface->colorImage->data);
			}
			free(m_surface->colorImage);
		}
		if(m_surface->maskImage)
		{
			if(m_surface->maskImage->data)
			{
				VG_VMemFree((uint32_t)m_surface->maskImage->data);
			}
			free(m_surface->maskImage);
		}

		free(m_surface);
	}

	OSReleaseMutex();
    OSDeinitMutex();

	return EGL_SUCCESS;
}

#undef EGL_NUMCONFIGS
