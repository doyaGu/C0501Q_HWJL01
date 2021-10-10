#ifndef __demod_ctrl_err_H_vWLHSToL_0i8d_mPuz_6ddG_fbQJywZJW7Yq__
#define __demod_ctrl_err_H_vWLHSToL_0i8d_mPuz_6ddG_fbQJywZJW7Yq__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================
#define  DEM_ERR_BASE            0xDDD00000

typedef enum _DEM_ERR_TAG
{
    // demode
    DEM_ERR_OK                  = 0,
    DEM_ERR_ALLOCATE_FAIL       = (DEM_ERR_BASE | 0x00000001),
    DEM_ERR_NULL_POINTER        = (DEM_ERR_BASE | 0x00000002),
    DEM_ERR_INVALID_PARAMETER   = (DEM_ERR_BASE | 0x00000003),
    DEM_ERR_TIME_OUT            = (DEM_ERR_BASE | 0x00000004),
    DEM_ERR_NO_IMPLEMENT        = (DEM_ERR_BASE | 0x00000005),
    DEM_ERR_NO_DEM_DESC         = (DEM_ERR_BASE | 0x00000006),

    DEM_ERR_UNKNOW              = (DEM_ERR_BASE | 0x0000FFFF),

}DEM_ERR;
//=============================================================================
//                Macro Definition
//=============================================================================

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

#ifdef __cplusplus
}
#endif

#endif
