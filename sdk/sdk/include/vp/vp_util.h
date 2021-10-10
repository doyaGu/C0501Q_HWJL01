#ifndef __VP_UTIL_H_BZ3EHE0J_K7SO_CTDM_HUYD_HZM16W7MJ2AZ__
#define __VP_UTIL_H_BZ3EHE0J_K7SO_CTDM_HUYD_HZM16W7MJ2AZ__

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
MMP_FLOATToFix(
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
sinc(
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
rcos(
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
cubic01(
    MMP_FLOAT   x);


//=============================================================================
/**
 * Matrix operation. For color correction matrix.
 */
//=============================================================================

void 
matrix_multiply_33(
    MMP_FLOAT   M_in1[3][3],
    MMP_FLOAT   M_in2[3][3],
    MMP_FLOAT   M_out[3][3]);

    
void 
matrix_multiply_31(
    MMP_FLOAT   M_in1[3][3],
    MMP_FLOAT   M_in2[3][1],
    MMP_FLOAT   M_out[3][1]);

    
/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
MMP_UINT16 vp_get_mem3Dfloat(MMP_FLOAT ****array3D, int frames, int rows, int columns);

/*
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> MMP_FLOAT array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
MMP_UINT32 vp_get_mem2Dfloat(MMP_FLOAT ***array2D, int rows, int columns);

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array 
 *    which was allocated with get_mem3Dfloat()
 ************************************************************************
 */
void vp_free_mem3Dfloat(MMP_FLOAT ***array3D, int frames);

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dfloat()
 ************************************************************************
 */
void vp_free_mem2Dfloat(MMP_FLOAT **array2D);       
    
//=============================================================================
/**
 * sin/cos interpolation.
 *
 * @param
 * @return
 */
//=============================================================================
void
getSinCos(
    MMP_INT32 Th,
    MMP_FLOAT *sinTh,
    MMP_FLOAT *cosTh);

#ifdef __cplusplus
}
#endif

#endif

