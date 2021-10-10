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
#include "../iteDefs.h"

#include <windows.h>

void* OSGetCurrentThreadID(void)
{
	return (void*)GetCurrentThreadId();	//TODO this is not safe
}

static HANDLE mutex = NULL;
static int mutexRefCount = 0;
//acquired mutex cannot be deinited.
void OSDeinitMutex(void)
{
	BOOL ret;
	ITE_ASSERT(mutex);
	ITE_ASSERT(mutexRefCount == 0);
	ret = CloseHandle(mutex);
	ITE_ASSERT(ret);
	//RI_UNREF(ret);
}
void OSAcquireMutex(void)
{
	DWORD ret;
	if(!mutex)
    {
        mutex = CreateMutex(NULL, FALSE, NULL);	//initially not locked
        mutexRefCount = 0;
    }
	ITE_ASSERT(mutex);
	ret = WaitForSingleObject(mutex, INFINITE);
	ITE_ASSERT(ret != WAIT_FAILED);
	//RI_UNREF(ret);
	mutexRefCount++;
}
void OSReleaseMutex(void)
{
	BOOL ret;
	ITE_ASSERT(mutex);
	mutexRefCount--;
	ITE_ASSERT(mutexRefCount >= 0);
	ret = ReleaseMutex(mutex);
	ITE_ASSERT(ret);
	//RI_UNREF(ret);
}

static BOOL isBigEndian(void)
{
	static const ITEuint32 v = 0x12345678u;
	const ITEuint8* p = (const ITEuint8*)&v;
	ITE_ASSERT (*p == (ITEuint8)0x12u || *p == (ITEuint8)0x78u);
	return (*p == (ITEuint8)(0x12)) ? TRUE : FALSE;
}

//Windows native
#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct
{
    HWND                window;
	HDC					bufDC;
	HBITMAP				bufDIB;
    unsigned int*       tmp;
    int                 tmpWidth;
    int                 tmpHeight;
} OSWindowContext;

void* OSCreateWindowContext(EGLNativeWindowType window)
{
    OSWindowContext* ctx = NULL;
	HDC winDC;
	ctx = (OSWindowContext*)malloc(sizeof(OSWindowContext));
	if( !ctx )
	{
		return NULL;
	}

    ctx->window = (HWND)window;
    winDC = GetDC(ctx->window);
    ctx->bufDC = CreateCompatibleDC(winDC);
    ReleaseDC(ctx->window, winDC);
    if(!ctx->bufDC)
    {
        free(ctx);
        return NULL;
    }

    ctx->bufDIB = NULL;
    ctx->tmp = NULL;
    ctx->tmpWidth = 0;
    ctx->tmpHeight = 0;
    return ctx;
}

void OSDestroyWindowContext(void* context)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
        if(ctx->bufDC)
        {
            SelectObject(ctx->bufDC, NULL);
            DeleteDC(ctx->bufDC);
        }
        if(ctx->bufDIB)
            DeleteObject(ctx->bufDIB);
        free(ctx);
    }
}

ITEboolean OSIsWindow(const void* context)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
		if(IsWindow(ctx->window))
			return TRUE;
    }
    return FALSE;
}

void OSGetWindowSize(const void* context, int* width, int* height)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
		RECT rect;
		GetClientRect(ctx->window, &rect);
		*width = rect.right - rect.left;
		*height = rect.bottom - rect.top;
    }
    else
    {
        *width = 0;
        *height = 0;
    }
}

typedef struct
{
	BITMAPINFOHEADER	header;
	DWORD				rMask;
	DWORD				gMask;
	DWORD				bMask;
} BMI;

void OSBlitToWindow(void* context, ITEuint8** vramData, ITEint width, ITEint height, VGImageFormat format)
{
    OSWindowContext* ctx = (OSWindowContext*)context;
    if(ctx)
    {
		BMI bmi;

        if(!ctx->tmp || !ctx->bufDIB || ctx->tmpWidth != width || ctx->tmpHeight != height)
        {
            if(ctx->bufDIB)
                DeleteObject(ctx->bufDIB);
            ctx->tmp = NULL;
            ctx->bufDIB = NULL;

            ctx->tmpWidth = width;
            ctx->tmpHeight = height;

            bmi.header.biSize			= sizeof(BITMAPINFOHEADER);
            bmi.header.biWidth			= width;
            bmi.header.biHeight			= height;
            bmi.header.biPlanes			= 1;
            bmi.header.biBitCount		= (WORD)32;
            bmi.header.biCompression	= BI_BITFIELDS;
            bmi.rMask = 0x000000ff;
            bmi.gMask = 0x0000ff00;
            bmi.bMask = 0x00ff0000;
            ctx->bufDIB = CreateDIBSection(ctx->bufDC, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&ctx->tmp, NULL, 0);
            if(!ctx->bufDIB)
            {
                ctx->tmp = NULL;
                return;
            }
        }

        if(ctx->tmp)
        {
			int i,j;
			ITEuint32 in = 0x0;
			unsigned char *SD, *DD;
            VGImageFormat df = VG_sXBGR_8888;
            HDC winDC = GetDC(ctx->window);
			int dbpp = 4;
			int sbpp;
			ITEuint8* data = NULL;
			ITEuint8* mappedSysRam = NULL;

            //NOTE: we assume here that the display is always in sRGB color space
			GdiFlush();
            if(isBigEndian())
                df = VG_sRGBX_8888;

			switch( format & 0x1f )
			{
			case 3: // VG_sRGB_565
				sbpp = 2;
				break;
			case 6: // VG_sL_8
			case 10: // VG_lL_8
				sbpp = 1;
				break;
			default:
				sbpp = 4;
				break;
			}

			data = (ITEuint8*)malloc(width * height * sbpp);
			if ( !data )
			{
				ReleaseDC(ctx->window, winDC);
            	SelectObject(ctx->bufDC, NULL);
				return;
			}
			mappedSysRam = ithMapVram(*vramData, width * height * sbpp, ITH_VRAM_READ);
			VG_Memcpy(data, mappedSysRam, width * height * sbpp);
			ithUnmapVram(mappedSysRam, width * height * sbpp);
			
			if( sbpp == 4 )
			{
				for (i=0; i<height; i++) 
				{
					DD = (unsigned char*)ctx->tmp + i*width*dbpp;
					SD = (unsigned char*)data + i*width*sbpp;
					memcpy(DD, SD, width*sbpp);
				}
			}
			else if( sbpp == 1 )
			{
				for (i=0; i<height; i++) 
				{
					for (j=0; j<width; j++) 
					{
						DD = (unsigned char*)ctx->tmp + i*width*dbpp + j*dbpp;
						SD = (unsigned char*)data + i*width*sbpp + j*sbpp;
						in = (ITEuint32) *((ITEuint8*)SD); 
						*DD = 0xff;
						*(DD+1) = in&0xff;
						*(DD+2) = in&0xff;
						*(DD+3) = in&0xff;
					}
				}
			}
			else if( sbpp == 2 )
			{
				for (i=0; i<height; i++) 
				{
					for (j=0; j<width; j++) 
					{
						DD = (unsigned char*)ctx->tmp + i*width*dbpp + j*dbpp;
						SD = (unsigned char*)data + i*width*sbpp + j*sbpp;
						in = (ITEuint32) *((ITEuint16*)SD); 
						*DD = 0xff;
						*(DD+1) = ((in&0xf800)>>8) | ((in&0xf800)>>13);
						*(DD+2) = ((in&0x7e0)>>2) | ((in&0x7e0)>>9);
						*(DD+3) = ((in&0x1f)<<3) | ((in&0x1f)>>2);
					}
				}
			}

            SelectObject(ctx->bufDC, ctx->bufDIB);
            BitBlt(winDC, 0, 0, width, height, ctx->bufDC, 0, 0, SRCCOPY);
            ReleaseDC(ctx->window, winDC);
            SelectObject(ctx->bufDC, NULL);

			free(data);
			data = NULL;
        }
    }
}

EGLDisplay OSGetDisplay(EGLNativeDisplayType display_id)
{
    //RI_UNREF(display_id);
    return (EGLDisplay)1;    //support only a single display
}


