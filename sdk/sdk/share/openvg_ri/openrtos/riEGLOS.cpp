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
 * \brief	Generic OS EGL functionality (not thread safe, no window rendering)
 * \note
  *//*-------------------------------------------------------------------*/

#include <pthread.h>
#include "ite/itp.h"
#include "egl.h"
#include "openvg.h"
#include "riImage.h"

typedef enum _LCD_BUFFER_INDEX{
	LCD_BUFFER_A = 0,
	LCD_BUFFER_B
}LCD_BUFFER_INDEX;

namespace OpenVGRI
{

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

void* OSGetCurrentThreadID(void)
{
	return NULL;
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

static pthread_mutex_t eglMutex = (pthread_mutex_t)NULL;
static int mutexRefCount = 0;

//acquired mutex cannot be deinited
void OSDeinitMutex(void)
{
	int ret = 0;
	
	RI_ASSERT(eglMutex != (pthread_mutex_t)NULL);
	RI_ASSERT(mutexRefCount == 0);
	ret = pthread_mutex_destroy(&eglMutex);
	RI_ASSERT(ret == 0);
	eglMutex = (pthread_mutex_t)NULL;
}
void OSAcquireMutex(void)
{
	int ret = 0;
	
	if(!eglMutex)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    	ret = pthread_mutex_init(&eglMutex, &attr);
		RI_ASSERT(ret == 0);
    }
	ret = pthread_mutex_lock(&eglMutex);
	RI_ASSERT(ret == 0);
	mutexRefCount++;
}
void OSReleaseMutex(void)
{
	int ret = 0;
	
	RI_ASSERT(eglMutex != (pthread_mutex_t)NULL);
	mutexRefCount--;
	ret = pthread_mutex_unlock(&eglMutex);
	RI_ASSERT(ret == 0);
	RI_ASSERT(mutexRefCount >= 0);
}

/*-------------------------------------------------------------------*//*!
* \brief	
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

struct OSWindowContext
{
    int					lcdWidth;
    int					lcdHeight;
	ITHLcdFormat		lcdFormat;
	LCD_BUFFER_INDEX	lcdBufferIndex;
};

void* OSCreateWindowContext(EGLNativeWindowType window)
{
    OSWindowContext* ctx = NULL;
    try
    {
        ctx = RI_NEW(OSWindowContext, ());
    }
	catch(std::bad_alloc)
	{
		return NULL;
	}
    ctx->lcdWidth       = ithLcdGetWidth();
    ctx->lcdHeight      = ithLcdGetHeight();
	ctx->lcdFormat 	    = ithLcdGetFormat();
	ctx->lcdBufferIndex = LCD_BUFFER_A;
    return ctx;
}

void OSDestroyWindowContext(void* context)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
        RI_DELETE(ctx);
    }
}

bool OSIsWindow(const void* context)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
        return true;
    }
    return false;
}

void OSGetWindowSize(const void* context, int& width, int& height)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
        width = ctx->lcdWidth;
        height = ctx->lcdHeight;
    }
    else
    {
        width = 0;
        height = 0;
    }
}

void OSBlitToWindow(void* context, const Drawable* drawable)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
    	int           drawWidth  = drawable->getWidth();
		int           drawHeight = drawable->getHeight();
		VGImageFormat readFormat = VG_sRGB_565;

		switch ( ctx->lcdFormat )
		{
		case ITH_LCD_RGB565:	readFormat = VG_sRGB_565;	break;
		case ITH_LCD_ARGB1555:	readFormat = VG_sABGR_1555;	break;
		case ITH_LCD_ARGB4444:	readFormat = VG_sABGR_4444;	break;
		case ITH_LCD_ARGB8888:  readFormat = VG_sABGR_8888; break;
		default:
			RI_ASSERT(1);
			break;
		}

		if ( ctx->lcdBufferIndex == LCD_BUFFER_A )
		{
        #ifdef CFG_WIN32_SIMULATOR
			vgReadPixels((void*)ithLcdGetBaseAddrA(), drawWidth*sizeof(int), readFormat, 0, 0, drawWidth, drawHeight);
        #else
            void* ptr = ithMapVram(ithLcdGetBaseAddrA(), CFG_LCD_PITCH * CFG_LCD_HEIGHT, ITH_VRAM_WRITE);
            vgReadPixels(ptr, drawWidth * CFG_LCD_BPP, readFormat, 0, 0, drawWidth, drawHeight);
            ithUnmapVram(ptr, CFG_LCD_PITCH * CFG_LCD_HEIGHT);
        #endif
            ithCmdQFlip(LCD_BUFFER_A);
			ctx->lcdBufferIndex = LCD_BUFFER_B;
		}
		else
		{
        #ifdef CFG_WIN32_SIMULATOR
			vgReadPixels((void*)ithLcdGetBaseAddrB(), drawWidth*sizeof(int), readFormat, 0, 0, drawWidth, drawHeight);
        #else
            void* ptr = ithMapVram(ithLcdGetBaseAddrB(), CFG_LCD_PITCH * CFG_LCD_HEIGHT, ITH_VRAM_WRITE);
            vgReadPixels(ptr, drawWidth * CFG_LCD_BPP, readFormat, 0, 0, drawWidth, drawHeight);
            ithUnmapVram(ptr, CFG_LCD_PITCH * CFG_LCD_HEIGHT);
        #endif
            ithCmdQFlip(LCD_BUFFER_B);
			ctx->lcdBufferIndex=  LCD_BUFFER_A;
		}
    }
}

EGLDisplay OSGetDisplay(EGLNativeDisplayType display_id)
{
    RI_UNREF(display_id);
    return (EGLDisplay)1;    //support only a single display
}

}   //namespace OpenVGRI
