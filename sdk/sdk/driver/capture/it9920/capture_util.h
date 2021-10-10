#ifndef __CAPTURE_UTIL_H__
#define __CAPTURE_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
//=============================================================================
//                Constant Definition
//=============================================================================


//=============================================================================
//                Macro Definition
//=============================================================================
static __inline MMP_UINT32 sisftol(MMP_FLOAT fVar)
{
    return (MMP_UINT32)fVar;
}

// sqrt(fVar)
static __inline MMP_FLOAT sissqrt(MMP_FLOAT fVar)
{
    return (MMP_FLOAT)(sqrt( fVar ));
}

// cos(fVar)
static __inline MMP_FLOAT siscos(MMP_FLOAT fVar)
{
    return (MMP_FLOAT)(cos( fVar ));
}

// sin(fVar)
static __inline MMP_FLOAT sissin(MMP_FLOAT fVar)
{
    return (MMP_FLOAT)(sin( fVar ));
}


//=============================================================================
//                Structure Definition
//=============================================================================


//=============================================================================
//                Global Data Definition
//=============================================================================


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Convert standard float s[8].23 to float s[e].m
 *
 * @param  value
 * @param  e
 * @param  m
 * @return
 */
//=============================================================================
MMP_UINT32
CAP_FLOATToFix(
    MMP_FLOAT   value,
    MMP_UINT32  e,
    MMP_UINT32  m);


//=============================================================================
/**
 * Function for ISP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
capSinc(
MMP_FLOAT   x);


//=============================================================================
/**
 * Function for ISP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
capRcos(
MMP_FLOAT   x);


//=============================================================================
/**
 * Catmull-Rom Cubic interpolation.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
capCubic01(
    MMP_FLOAT   x);


//=============================================================================
/**
 * sin/cos interpolation.
 *
 * @param
 * @return
 */
//=============================================================================
#if defined (CAP_USE_COLOR_EFFECT)
void
capGetSinCos(
    MMP_INT32 Th,
    MMP_FLOAT *sinTh,
    MMP_FLOAT *cosTh);
#endif

#ifdef __cplusplus
}
#endif

#endif

