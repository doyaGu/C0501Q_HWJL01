/*
 * Copyright (c) 2014 ITE Corp. All Rights Reserved.
 */
/** @file driver.h
 *  GFX API function header file.
 *
 * @author Awin Huang
 * @version 1.0
 * @date 2014-05-28
 */
#ifndef __GFX_DRIVER_H__
#define __GFX_DRIVER_H__

#include <stdint.h>
#include <pthread.h>
#include "hw.h"

/**
 * DLL export API declaration for Win32.
 */

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct _GFX_DRIVER
{
    // Management
    pthread_mutex_t mutex;
    int32_t         refCount;

    // Display
    GFXSurface*     displaySurfaceA;     ///< the display surface A
    GFXSurface*     displaySurfaceB;     ///< the display surface B
    GFXSurface*     displaySurfaceC;     ///< the display surface C
    int32_t         displaySurfaceIndex; ///< the index of current display surface
    bool            enableMask;          ///< the enable flag of mask surface
    GFXSurface*     maskSurface;         ///< the pointer of mask surface

    // HW
    GFX_HW_DEVICE*  hwDev;               ///< the register of hardware device
}GFX_DRIVER;

//=============================================================================
//                              Global Data Definition
//=============================================================================
/**
 * This routine is used to initialize the GFX driver.
 *
 * @return a bool value true if succeed, otherwise return false.
 */
bool
gfxInitialize();

/**
 * This routine is used to terminate the GFX driver.
 *
 * @return void.
 */
void
gfxTerminate();

/**
 * This routine is used to get display surface index.
 *
 * @return a valid index of  display surface.
 */
uint8_t
gfxGetDispSurfIndex(void);

/**
 * This routine is used to get back display surface index.
 *
 * @return a valid index of back display surface.
 */
uint8_t
gfxGetBackSurfIndex(void);

/**
 * This routine is used to set next display surface.
 *
 * @return void.
 */
void
gfxSetDispSurfIndex(void);

/**
 * This routine is used to get current display surface.
 *
 * @return a valid pointer of current display surface.
 */
GFXSurface*
gfxGetDisplaySurface();

/**
 * This routine is used to get back display surface.
 *
 * @return a valid pointer of back display surface.
 */
GFXSurface*
gfxGetBackSurface();

/**
 * This routine is used to flip current display surface.
 *
 * @return a bool value true if flip succeed, otherwise return false.
 */
bool
gfxFlip();

/**
 * This routine is used to lock thread mutex.
 *
 * @return void.
 */
void
gfxLock();

/**
 * This routine is used to unlock thread mutex.
 *
 * @return void.
 */
void
gfxUnlock();


/**
 * This routine is used to get information of GFX driver.
 *
 * @return a valid pointer of information of GFX driver.
 */
GFX_DRIVER*
gfxGetDriver();

//=============================================================================
//                              Public Function Declaration
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif // __GFX_DRIVER_H__
