﻿#ifndef __SIMPLE_DRAW_H_D4SPD1AN_NYB9_O95T_YIYC_KRNY94BRESHN__
#define __SIMPLE_DRAW_H_D4SPD1AN_NYB9_O95T_YIYC_KRNY94BRESHN__

#ifdef __cplusplus
extern "C" {
#endif


#include "globle.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct _BASE_RECT_TAG
{
    int     x;
    int     y;
    int     w;
    int     h;
}BASE_RECT;

//=============================================================================
//                  Global Data Definition
//=============================================================================


//=============================================================================
//                  Private Function Definition
//=============================================================================


//=============================================================================
//                  Public Function Definition
//=============================================================================
int
Draw_Rect(
    uint8_t     *baseAddr,
    uint32_t    dstPitch,
    BASE_RECT   *rect,
    uint32_t    color);



#ifdef __cplusplus
}
#endif

#endif
