/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Header file of brush functions.
 *
 * @author Mandy Wu
 * @version 0.1
 */

#ifndef __PEN_H
#define __PEN_H

#include "m2d/m2d_graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define M2D_PEN_STYLE_DASH         0xFFFFC000           //  DASH         -------
#define M2D_PEN_STYLE_DOT          0xE38E3800           //  DOT          .......
#define M2D_PEN_STYLE_DASH_DOT     0xFF81C000           //  DASH_DOT     _._._._
#define M2D_PEN_STYLE_DASH_DOT_DOT 0xFF8E3800           //  DASH_DOT_DOT _.._.._

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/**
 * Create pen object.
 */
M2D_PENOBJ *
M2D_CreatePen(void);

#ifdef __cplusplus
}
#endif

#endif // End of ifndef __PEN_H