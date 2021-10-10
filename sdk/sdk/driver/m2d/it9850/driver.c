/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file driver.c
 *  GFX API function file.
 *
 * @author Awin Huang
 * @version 1.0
 * @date 2014-05-28
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include "gfx.h"
#include "hw.h"
#include "driver.h"
#include "ite/ith.h"
#include "msg.h"

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static GFX_DRIVER GfxDriver = { PTHREAD_MUTEX_INITIALIZER };

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_gfxDriverReset(
    GFX_DRIVER* driver);

static GFXSurface*
_gfxDriverCreateDisplaySurface(
    uint32_t    index);

//=============================================================================
//                              Public Function Definition
//=============================================================================
bool
gfxInitialize()
{
    GFX_FUNC_ENTRY;
    gfxLock();

    ithLcdEnableHwFlip();

    if (GfxDriver.refCount == 0)
    {
        // Do this first, reset refCount inside.
        _gfxDriverReset(&GfxDriver);

        GfxDriver.refCount = 1;
        GfxDriver.hwDev = gfxHwInitialize();
        // Get display surface
        GfxDriver.displaySurfaceA = _gfxDriverCreateDisplaySurface(0);
        GfxDriver.displaySurfaceB = _gfxDriverCreateDisplaySurface(1);        
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        GfxDriver.displaySurfaceC = _gfxDriverCreateDisplaySurface(2);
#endif        

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        if (ithLcdGetFlip() == 0)
            GfxDriver.displaySurfaceIndex = 1;
        else if (ithLcdGetFlip() == 1)
        	  GfxDriver.displaySurfaceIndex = 2;
        else
            GfxDriver.displaySurfaceIndex = 0;
#else        
      if (ithLcdGetFlip() == 0)
          GfxDriver.displaySurfaceIndex = 1;
      else
          GfxDriver.displaySurfaceIndex = 0;
#endif        
    }
    else
    {
        GfxDriver.refCount++;
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return true;
}

void
gfxTerminate()
{
    GFX_FUNC_ENTRY;
    assert(GfxDriver.refCount > 0);
    gfxLock();

    if (--GfxDriver.refCount == 0)
    {
        _gfxDriverReset(&GfxDriver);
        gfxHwTerminate(GfxDriver.hwDev);

        // Terminate LCD buffer here!
        gfxDestroySurface(GfxDriver.displaySurfaceA);
        gfxDestroySurface(GfxDriver.displaySurfaceB);
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
        gfxDestroySurface(GfxDriver.displaySurfaceC);
#endif
    }

    gfxUnlock();
    GFX_FUNC_LEAVE;
}

uint8_t
gfxGetDispSurfIndex(void)
{	         
    return GfxDriver.displaySurfaceIndex;
}

uint8_t
gfxGetBackSurfIndex(void)
{	     
	  uint8_t surf_index;
	  
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    if (GfxDriver.displaySurfaceIndex == 0)
    	  surf_index = 2;
    else if (GfxDriver.displaySurfaceIndex == 1)
    	  surf_index = 0;
    else
    	  surf_index = 1;
#else
    surf_index = !GfxDriver.displaySurfaceIndex;
#endif	
    return surf_index;
}

void
gfxSetDispSurfIndex(void)
{
	  uint32_t lcd_index = ithLcdGetFlip();
    GFX_FUNC_ENTRY;
    gfxLock();        
       
    switch (lcd_index)
    {    
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    case 0:
        GfxDriver.displaySurfaceIndex = 1;
        break;   
    case 1:
        GfxDriver.displaySurfaceIndex = 2;
        break;
    case 2:
        GfxDriver.displaySurfaceIndex = 0;
        break;
#else    	
    default:
    case 0:
        GfxDriver.displaySurfaceIndex = 1;
        break;   
    case 1:
        GfxDriver.displaySurfaceIndex = 0;
        break;
#endif        
    }
    
    gfxUnlock();
    GFX_FUNC_LEAVE;
}

GFXSurface*
gfxGetDisplaySurface()
{
    GFXSurface* currentDisplaySurface;
    GFX_FUNC_ENTRY;

    gfxLock();
    currentDisplaySurface = NULL;

    if (GfxDriver.displaySurfaceIndex == 0)
    {
        currentDisplaySurface = GfxDriver.displaySurfaceA;
    }
    else if (GfxDriver.displaySurfaceIndex == 1)
    {
        currentDisplaySurface = GfxDriver.displaySurfaceB;
    }
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    else
    {
        currentDisplaySurface = GfxDriver.displaySurfaceC;
    }
#endif

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return currentDisplaySurface;
}

GFXSurface*
gfxGetBackSurface()
{
    GFXSurface* currentDisplaySurface = NULL;
    GFX_FUNC_ENTRY;

    gfxLock();
    
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    if (GfxDriver.displaySurfaceIndex == 0)
    {
        currentDisplaySurface = GfxDriver.displaySurfaceC;
    }
    else if (GfxDriver.displaySurfaceIndex == 1)
    {
        currentDisplaySurface = GfxDriver.displaySurfaceA;
    }
    else
    {
        currentDisplaySurface = GfxDriver.displaySurfaceB;
    }
#else
    if (!GfxDriver.displaySurfaceIndex)
    {
        currentDisplaySurface = GfxDriver.displaySurfaceB;
    }
    else
    {
        currentDisplaySurface = GfxDriver.displaySurfaceA;
    }
#endif

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return currentDisplaySurface;
}

bool
gfxFlip()
{
    GFX_FUNC_ENTRY;
    gfxLock();

#if (CFG_CHIP_FAMILY == 9920)
    ithCmdQFlip(GfxDriver.displaySurfaceIndex, ITH_CMDQ0_OFFSET);
#else
    ithCmdQFlip(GfxDriver.displaySurfaceIndex);
#endif

#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)
    GfxDriver.displaySurfaceIndex = (GfxDriver.displaySurfaceIndex + 1) % 3;	  
#else
    GfxDriver.displaySurfaceIndex = !GfxDriver.displaySurfaceIndex;
#endif

    gfxUnlock();
    GFX_FUNC_LEAVE;

    return true;
}

void
gfxLock(void)
{
    pthread_mutex_lock(&GfxDriver.mutex);
}

void
gfxUnlock(void)
{
    pthread_mutex_unlock(&GfxDriver.mutex);
}

GFX_DRIVER*
gfxGetDriver()
{
    return &GfxDriver;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
void
_gfxDriverReset(
    GFX_DRIVER* driver)
{
    uint8_t*    driverPtr = (uint8_t*)driver + sizeof(pthread_mutex_t);
    uint32_t    driverPtrSize = sizeof(GFX_DRIVER) - sizeof(pthread_mutex_t);

    memset(driverPtr, 0x00, driverPtrSize);
}

GFXSurface*
_gfxDriverCreateDisplaySurface(
    uint32_t    index)
{
    GFXSurface* surface = NULL;
    GFXColorFormat format;

    switch(ithLcdGetFormat())
    {
    case ITH_LCD_RGB565  : format = GFX_COLOR_RGB565;   break;
    case ITH_LCD_ARGB1555: format = GFX_COLOR_ARGB1555; break;
    case ITH_LCD_ARGB4444: format = GFX_COLOR_ARGB4444; break;
    case ITH_LCD_ARGB8888: format = GFX_COLOR_ARGB8888; break;
    default: // ITH_LCD_YUV
                           format = GFX_COLOR_UNKNOWN;
    }

    switch(index)
    {
    case 0:
        surface = gfxCreateSurfaceByPointer(ithLcdGetWidth(), ithLcdGetHeight(), ithLcdGetPitch(), format, (uint8_t*)ithLcdGetBaseAddrA(), ithLcdGetHeight()*ithLcdGetPitch());
        break;
    case 1:
        surface = gfxCreateSurfaceByPointer(ithLcdGetWidth(), ithLcdGetHeight(), ithLcdGetPitch(), format, (uint8_t*)ithLcdGetBaseAddrB(), ithLcdGetHeight()*ithLcdGetPitch());
        break;
#if defined(CFG_VIDEO_ENABLE) || defined(CFG_LCD_TRIPLE_BUFFER)

    case 2:
        surface = gfxCreateSurfaceByPointer(ithLcdGetWidth(), ithLcdGetHeight(), ithLcdGetPitch(), format, (uint8_t*)ithLcdGetBaseAddrC(), ithLcdGetHeight()*ithLcdGetPitch());
        break;
#endif        
    default:
        assert(0);
        break;
    }

    return surface;
}
