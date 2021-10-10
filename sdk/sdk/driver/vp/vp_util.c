//#include "host/host.h"
//#include "pal/pal.h"
//#include "sys/sys.h"
//#include "mmp_types.h"

#include "vp/vp_config.h"
#include "vp/vp_types.h"
#include "vp/vp_util.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
#if defined (USE_COLOR_EFFECT)
static MMP_FLOAT
sinCosTable[2][46] = {
  {
    0,           0.017452406, 0.034899497, 0.052335956, 0.069756474, 0.087155743, 0.104528463, 0.121869343,
    0.139173101, 0.156434465, 0.173648178, 0.190808995, 0.207911691, 0.224951054, 0.241921896, 0.258819045,
    0.275637356, 0.292371705, 0.309016994, 0.325568154, 0.342020143, 0.35836795,  0.374606593, 0.390731128,
    0.406736643, 0.422618262, 0.438371147, 0.4539905,   0.469471563, 0.48480962,  0.5,         0.515038075,
    0.529919264, 0.544639035, 0.559192903, 0.573576436, 0.587785252, 0.601815023, 0.615661475, 0.629320391,
    0.64278761,  0.656059029, 0.669130606, 0.68199836,  0.69465837,  0.707106781
  },
  {
    1,           0.999847695, 0.999390827, 0.998629535, 0.99756405,  0.996194698, 0.994521895, 0.992546152,
    0.990268069, 0.987688341, 0.984807753, 0.981627183, 0.978147601, 0.974370065, 0.970295726, 0.965925826,
    0.961261696, 0.956304756, 0.951056516, 0.945518576, 0.939692621, 0.933580426, 0.927183855, 0.920504853,
    0.913545458, 0.906307787, 0.898794046, 0.891006524, 0.882947593, 0.874619707, 0.866025404, 0.857167301,
    0.848048096, 0.838670568, 0.829037573, 0.819152044, 0.809016994, 0.79863551, 0.788010754, 0.777145961,
    0.766044443, 0.75470958,  0.743144825, 0.731353702, 0.7193398,   0.707106781
  }
};
#endif
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
    MMP_UINT32  m)
{
    MMP_UINT32  dwValue = *(MMP_UINT32*)&value;
    MMP_UINT16  i = 0;

    if (value < 0.0f)
    {
        value *= -1.0f;
        for (i = 0; i < m; i++)
           value *= 2;

        dwValue = sisftol(value);
        dwValue = ~dwValue + 1;
    }
    else
    {
        for (i = 0; i < m; i++)
           value *= 2;

        dwValue = sisftol(value);
    }

    return dwValue;
}

//=============================================================================
/**
 * Function for ISP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
vp_sinc(
    MMP_FLOAT   x)
{
    MMP_FLOAT pi=3.14159265358979f;

    if (x == 0.0f)
    {
        return 1.0f;
    }
    else
    {
        return (sissin(pi * x) / (pi * x));
    }
}

//=============================================================================
/**
 * Function for ISP scaling weighting matrix setting.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
vp_rcos(
    MMP_FLOAT   x)
{
    MMP_FLOAT   pi = 3.14159265358979f;
    MMP_FLOAT   r = 0.5f;

    if (x == 0.0f)
    {
        return 1.0f;
    }
    else if (x == -1.0f / (2.0f * r) || x == 1.0f / (2.0f * r))
    {
        return (r / 2.0f * sissin(pi / (2.0f * r)));
    }
    else
    {
        return (sissin(pi * x) / (pi * x) * siscos(r * pi * x) / (1 - 4 * r * r * x * x));
    }
}

//=============================================================================
/**
 * Catmull-Rom Cubic interpolation.
 *
 * @param
 * @return
 */
//=============================================================================
MMP_FLOAT
vp_cubic01(
    MMP_FLOAT   x)
{
    if (fabs(x) <= 1.0)
        return ((MMP_FLOAT) (1.5 * fabs(x*x*x) - 2.5* x*x + 1.0));
    else if (fabs(x) > 1.0 && fabs(x) <= 2.0)
        return ((MMP_FLOAT) (-0.5 * fabs(x*x*x) + 2.5* x*x - 4.0* fabs(x) + 2.0));
    else
        return 0.0;
}

//=============================================================================
/**
 * Matrix operation. For color correction matrix.
 *
 * @param
 * @return
 */
//=============================================================================
void
vp_matrix_multiply_33(
    MMP_FLOAT   M_in1[3][3],
    MMP_FLOAT   M_in2[3][3],
    MMP_FLOAT   M_out[3][3])
{
    MMP_INT16 i, j, k;

    for (i = 0; i < 3; i++)
        for (j=  0; j < 3; j++)
        {
            M_out[i][j] = 0;
            for (k = 0; k < 3; k++)
                M_out[i][j] += M_in1[i][k] * M_in2[k][j];
        }
}

void
vp_matrix_multiply_31(
    MMP_FLOAT   M_in1[3][3],
    MMP_FLOAT   M_in2[3][1],
    MMP_FLOAT   M_out[3][1])
{
    MMP_INT16 i, j, k;

    j = 0;
    for (i = 0; i < 3; i++)
    {
        M_out[i][j] = 0;
        for (k = 0; k < 3; k++)
        {
            M_out[i][j] += M_in1[i][k] * M_in2[k][j];
        }
    }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
MMP_UINT16
vp_get_mem3Dfloat(
    MMP_FLOAT   ****array3D,
    int         frames,
    int         rows,
    int         columns)
{
    MMP_UINT16  j;

    if (((*array3D) = (MMP_FLOAT***)calloc(frames, sizeof(MMP_FLOAT**))) == MMP_NULL)
        VP_msg_ex(VP_MSG_TYPE_ERR, " get_mem3Dfloat: array3D %s() !\n", __FUNCTION__);

    for (j = 0; j < frames; j++)
        vp_get_mem2Dfloat( (*array3D)+j, rows, columns ) ;

    return frames*rows*columns*sizeof(MMP_FLOAT);
}

/*
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> MMP_FLOAT array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
MMP_UINT32
vp_get_mem2Dfloat(
    MMP_FLOAT   ***array2D,
    int         rows,
    int         columns)
{
    MMP_UINT16 i;

    if ((*array2D      = (MMP_FLOAT**)calloc(rows,        sizeof(MMP_FLOAT*))) == MMP_NULL)
        VP_msg_ex(VP_MSG_TYPE_ERR, " get_mem2Dfloat: array2D %s() !\n", __FUNCTION__);
    if (((*array2D)[0] = (MMP_FLOAT* )calloc(rows*columns,sizeof(MMP_FLOAT ))) == MMP_NULL)
        VP_msg_ex(VP_MSG_TYPE_ERR, " get_mem2Dfloat: array2D %s() !\n", __FUNCTION__);

    for (i = 1; i < rows; i++)
        (*array2D)[i] =  (*array2D)[i-1] + columns  ;

    return rows * columns * sizeof(MMP_FLOAT);
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dfloat()
 ************************************************************************
 */
void
vp_free_mem3Dfloat(
    MMP_FLOAT   ***array3D,
    int         frames)
{
    MMP_UINT16 i;

    if (array3D)
    {
        for (i = 0; i < frames; i++)
        {
            vp_free_mem2Dfloat(array3D[i]);
        }
        free(array3D);
    }
    else
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " free_mem3Dfloat: trying to free unused memory %s()!\n", __FUNCTION__);
    }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dfloat()
 ************************************************************************
 */
void
vp_free_mem2Dfloat(
    MMP_FLOAT   **array2D)
{
    if (array2D)
    {
        if (array2D[0])
            free(array2D[0]);
        else VP_msg_ex(VP_MSG_TYPE_ERR, " free_mem2Dfloat: trying to free unused memory %s()!\n", __FUNCTION__);

        free(array2D);
    }
    else
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " free_mem2Dfloat: trying to free unused memory %s()!\n", __FUNCTION__);
    }
}

//=============================================================================
/**
 * sin/cos interpolation.
 *
 * @param
 * @return
 */
//=============================================================================
#if defined (USE_COLOR_EFFECT)
void
vp_getSinCos(
    MMP_INT32 Th,
    MMP_FLOAT *sinTh,
    MMP_FLOAT *cosTh)
{
    MMP_INT32 sinSign, cosSign, sinCosSwap;

    while (Th > 360) Th -= 360;
    while (Th < 0) Th += 360;
    sinSign = cosSign = 1;
    sinCosSwap = 0;
    if (Th > 180) {sinSign = -1; Th = 360 - Th;}
    if (Th > 90) {cosSign = -1; Th = 180 - Th;}
    if (Th > 45) {sinCosSwap = 1; Th = 90 - Th;}
    *sinTh = sinCosTable[sinCosSwap][Th] * sinSign;
    *cosTh = sinCosTable[1 - sinCosSwap][Th] * cosSign;
}
#endif
