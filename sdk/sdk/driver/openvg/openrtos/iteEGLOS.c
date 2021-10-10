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
 * \todo	clarify getThread and getCurrentThread distinction.
 *//*-------------------------------------------------------------------*/

#include "egl.h"
#include "openvg.h"
#include "iteDefs.h"
#include "ite/ith.h"
#include "ite/itv.h"
#include <pthread.h>

//#define ITE_EGL_USE_SINGLE_BUFFER

typedef enum _LCD_BUFFER_INDEX
{
	LCD_BUFFER_A = 0,
	LCD_BUFFER_B
}LCD_BUFFER_INDEX;

typedef enum _EGL_BOOL
{
	_EGL_FALSE = 0,
	_EGL_TRUE  = 1
}EGL_BOOL;

static pthread_mutex_t eglMutex = (pthread_mutex_t)NULL;
static int mutexRefCount = 0;

void* OSGetCurrentThreadID(void)
{
	//return (void*)GetCurrentThreadId();	//TODO this is not safe
	return NULL;
}

//acquired mutex cannot be deinited.
void OSDeinitMutex(void)
{
	int ret = 0;
	return;
	
	ITE_ASSERT(eglMutex != (pthread_mutex_t)NULL);
	ITE_ASSERT(mutexRefCount == 0);
	ret = pthread_mutex_destroy(&eglMutex);
	ITE_ASSERT(ret == 0);
	eglMutex = (pthread_mutex_t)NULL;
}
void OSAcquireMutex(void)
{
	int ret = 0;
	return;
	
	if(!eglMutex)
    {
    	ret = pthread_mutex_init(&eglMutex, NULL);
		ITE_ASSERT(ret == 0);
    }
	ret = pthread_mutex_lock(&eglMutex);
	ITE_ASSERT(ret == 0);
	mutexRefCount++;
}
void OSReleaseMutex(void)
{
	int ret = 0;
	return;
	
	ITE_ASSERT(eglMutex != (pthread_mutex_t)NULL);
	mutexRefCount--;
	ret = pthread_mutex_unlock(&eglMutex);
	ITE_ASSERT(ret == 0);
	ITE_ASSERT(mutexRefCount >= 0);
}

static EGL_BOOL isBigEndian(void)
{
	static const ITEuint32 v = 0x12345678u;
	const ITEuint8* p = (const ITEuint8*)&v;
	ITE_ASSERT (*p == (ITEuint8)0x12u || *p == (ITEuint8)0x78u);
	return (*p == (ITEuint8)(0x12)) ? _EGL_TRUE : _EGL_FALSE;
}

typedef struct _OSWindowContext
{
    int					lcdWidth;
    int					lcdHeight;
	ITHLcdFormat		lcdFormat;
	LCD_BUFFER_INDEX	lcdBufferIndex;
} OSWindowContext;

void* OSCreateWindowContext(EGLNativeWindowType window)
{
    OSWindowContext* ctx = NULL;

	ctx = (OSWindowContext*)malloc(sizeof(OSWindowContext));
	if ( ctx )
	{
	    ctx->lcdWidth       = ithLcdGetWidth();
	    ctx->lcdHeight      = ithLcdGetHeight();
		ctx->lcdFormat 	    = ithLcdGetFormat();
		ctx->lcdBufferIndex = LCD_BUFFER_A;
	}
    return ctx;
}

void OSDestroyWindowContext(void* context)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
        free(ctx);
    }
}

ITEboolean OSIsWindow(const void* context)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
        return _EGL_TRUE;
    }
    return _EGL_FALSE;
}

void OSGetWindowSize(const void* context, int* width, int* height)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
        *width = ctx->lcdWidth;
        *height = ctx->lcdHeight;
    }
    else
    {
        *width = 0;
        *height = 0;
    }
}

void OSBlitToWindow(void* context, ITEuint8** vramData, ITEint width, ITEint height, VGImageFormat format)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
    	int           drawWidth  = width;
		int           drawHeight = height;
		VGImageFormat readFormat = VG_sRGB_565;

		switch ( ctx->lcdFormat )
		{
		case ITH_LCD_RGB565:	readFormat = VG_sRGB_565;	break;
		case ITH_LCD_ARGB1555:	readFormat = VG_sABGR_1555;	break;
		case ITH_LCD_ARGB4444:	readFormat = VG_sABGR_4444;	break;
		case ITH_LCD_ARGB8888:  readFormat = VG_sABGR_8888; break;
		default:
			ITE_ASSERT(1);
			break;
		}

#ifndef ITE_EGL_USE_SINGLE_BUFFER
		if ( ctx->lcdBufferIndex == LCD_BUFFER_A )
		{
			ithCmdQFlip(LCD_BUFFER_A);
			ctx->lcdBufferIndex = LCD_BUFFER_B;
			*vramData = (ITEuint8*)ithLcdGetBaseAddrB();
		}
		else
		{
			ithCmdQFlip(LCD_BUFFER_B);
			ctx->lcdBufferIndex = LCD_BUFFER_A;
			*vramData = (ITEuint8*)ithLcdGetBaseAddrA();
		}
#endif

		/* Below code for readback mode */
		/*
		if ( ctx->lcdBufferIndex == LCD_BUFFER_A )
		{
			vgReadPixels((void*)ithLcdGetBaseAddrA(), drawWidth*2, readFormat, 0, 0, drawWidth, drawHeight);
			ithCmdQFlip(LCD_BUFFER_A);
			ctx->lcdBufferIndex = LCD_BUFFER_B;
		}
		else
		{
			vgReadPixels((void*)ithLcdGetBaseAddrB(), drawWidth*sizeof(int), readFormat, 0, 0, drawWidth, drawHeight);
			ithCmdQFlip(LCD_BUFFER_B);
			ctx->lcdBufferIndex=  LCD_BUFFER_A;
		}
		*/
    }
}

EGLDisplay OSGetDisplay(EGLNativeDisplayType display_id)
{
    return (EGLDisplay)1;    //support only a single display
}


