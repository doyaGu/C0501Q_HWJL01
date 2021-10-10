#ifndef __ISP_UTIL_H_BZ3EHE0J_K7SO_CTDM_HUYD_HZM16W7MJ2AZ__
#define __ISP_UTIL_H_BZ3EHE0J_K7SO_CTDM_HUYD_HZM16W7MJ2AZ__

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================
static __inline MMP_UINT32 sisftol(MMP_FLOAT fVar)
{
    return (MMP_UINT32)fVar;
}

// sqrt(fVar)
static __inline MMP_FLOAT sissqrt(MMP_FLOAT fVar)
{
    return (MMP_FLOAT)(sqrt(fVar));
}

// cos(fVar)
static __inline MMP_FLOAT siscos(MMP_FLOAT fVar)
{
    return (MMP_FLOAT)(cos(fVar));
}

// sin(fVar)
static __inline MMP_FLOAT sissin(MMP_FLOAT fVar)
{
    return (MMP_FLOAT)(sin(fVar));
}

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================

//=============================================================================
//				  Public Function Definition
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
ISP_FloatToFix(
    MMP_FLOAT  value,
    MMP_UINT32 e,
    MMP_UINT32 m);

//=============================================================================
/**
 * Function for ISP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
sinc(
    MMP_FLOAT x);

//=============================================================================
/**
 * Function for ISP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
rcos(
    MMP_FLOAT x);

//=============================================================================
/**
 * Catmull-Rom Cubic interpolation.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
cubic01(
    MMP_FLOAT x);

//=============================================================================
/**
 * Matrix operation. For color correction matrix.
 */
//=============================================================================
void
    matrix_multiply_33(
    MMP_FLOAT M_in1[3][3],
    MMP_FLOAT M_in2[3][3],
    MMP_FLOAT M_out[3][3]);

void
    matrix_multiply_31(
    MMP_FLOAT M_in1[3][3],
    MMP_FLOAT M_in2[3][1],
    MMP_FLOAT M_out[3][1]);

#ifdef __cplusplus
}
#endif

#endif