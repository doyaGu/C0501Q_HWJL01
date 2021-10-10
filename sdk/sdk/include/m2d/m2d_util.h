#include "ite/mmp_types.h"
#include "ite/ite_m2d.h"

#ifndef _M2D_UTIL_H
    #define _M2D_UTIL_H

    #ifdef __cplusplus
extern "C" {
    #endif

M2D_API MMP_BOOL
M2D_ValidDisplay(
    MMP_M2D_SURFACE disp);

M2D_API MMP_UINT32
M2D_GetDisplayWidth(
    MMP_M2D_SURFACE disp);

M2D_API MMP_UINT32
M2D_GetDisplayHeight(
    MMP_M2D_SURFACE disp);

M2D_API MMP_UINT32
M2D_GetDisplayBaseOffset(
    MMP_M2D_SURFACE disp);

M2D_API MMP_UINT32
M2D_GetDisplayBaseAddress(
    MMP_M2D_SURFACE disp);

    #ifdef __cplusplus
}
    #endif

#endif //_M2D_UTIL_H