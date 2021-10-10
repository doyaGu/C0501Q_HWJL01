#ifndef __ts_aggre_err_H_Ffkpn4dJ_lBOK_4TRb_3mdS_VpYkzzRS4YFJ__
#define __ts_aggre_err_H_Ffkpn4dJ_lBOK_4TRb_3mdS_VpYkzzRS4YFJ__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================
#define TSA_ERR_BASE    0xAE000000

typedef enum TSA_ERR_T
{
    TSA_ERR_OK                  = 0,
    TSA_ERR_ALLOCATE_FAIL       = (TSA_ERR_BASE | 0x00000001),
    TSA_ERR_NULL_POINTER        = (TSA_ERR_BASE | 0x00000002),
    TSA_ERR_INVALID_PARAMETER   = (TSA_ERR_BASE | 0x00000003),
    TSA_ERR_TIME_OUT            = (TSA_ERR_BASE | 0x00000004),
    TSA_ERR_NO_IMPLEMENT        = (TSA_ERR_BASE | 0x00000005),
    TSA_ERR_NO_TSA_DESC         = (TSA_ERR_BASE | 0x00000006),

    TSA_ERR_UNKNOW              = (TSA_ERR_BASE | 0x0000FFFF),
}TSA_ERR;
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
