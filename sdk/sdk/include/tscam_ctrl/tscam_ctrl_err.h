#ifndef __tscam_ctrl_err_H_S52px3Ox_2GQ5_IqqO_hHQF_RgxNAxwvHfYi__
#define __tscam_ctrl_err_H_S52px3Ox_2GQ5_IqqO_hHQF_RgxNAxwvHfYi__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================
#define  TSCM_ERR_BASE            0x15CE0000

typedef enum TSCM_ERR_T
{
    TSCM_ERR_OK                  = 0,
    TSCM_ERR_ALLOCATE_FAIL       = (TSCM_ERR_BASE | 0x00000001),
    TSCM_ERR_NULL_POINTER        = (TSCM_ERR_BASE | 0x00000002),
    TSCM_ERR_INVALID_PARAMETER   = (TSCM_ERR_BASE | 0x00000003),
    TSCM_ERR_TIME_OUT            = (TSCM_ERR_BASE | 0x00000004),
    TSCM_ERR_NO_IMPLEMENT        = (TSCM_ERR_BASE | 0x00000005),
    TSCM_ERR_NO_TSCM_DESC        = (TSCM_ERR_BASE | 0x00000006),

    TSCM_ERR_UNKNOW              = (TSCM_ERR_BASE | 0x0000FFFF),
}TSCM_ERR;
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
