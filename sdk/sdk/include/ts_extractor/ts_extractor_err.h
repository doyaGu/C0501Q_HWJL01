#ifndef __ts_extractor_err_H_uEPgQRsB_pz0s_LnZ8_JoMl_uJBj5WmFtJIE__
#define __ts_extractor_err_H_uEPgQRsB_pz0s_LnZ8_JoMl_uJBj5WmFtJIE__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//				  Constant Definition
//=============================================================================
#define TSE_ERR_BASE            0xEE000000
#define TSEXT_ERR_BASE          0xEE100000
#define TSS_ERR_BASE            0xEE200000
#define TSPA_ERR_BASE           0xEE300000
#define TSPD_ERR_BASE           0xEE400000
typedef enum _TSE_ERR_TAG
{
    // ite ts extractor
    TSE_ERR_OK                  = 0,
    TSE_ERR_ALLOCATE_FAIL       = (TSE_ERR_BASE | 0x00000001),
    TSE_ERR_NULL_POINTER        = (TSE_ERR_BASE | 0x00000002),
    TSE_ERR_INVALID_PARAMETER   = (TSE_ERR_BASE | 0x00000003),
    TSE_ERR_TIME_OUT            = (TSE_ERR_BASE | 0x00000004),
    TSE_ERR_NO_IMPLEMENT        = (TSE_ERR_BASE | 0x00000005),
    TSE_ERR_DEMOD_FAIL          = (TSE_ERR_BASE | 0x00000006),
    TSE_ERR_AGGRE_FAIL          = (TSE_ERR_BASE | 0x00000007),

    // ts extract
    TSEXT_ERR_OK                  = 0,
    TSEXT_ERR_ALLOCATE_FAIL       = (TSEXT_ERR_BASE | 0x00000001),
    TSEXT_ERR_NULL_POINTER        = (TSEXT_ERR_BASE | 0x00000002),
    TSEXT_ERR_INVALID_PARAMETER   = (TSEXT_ERR_BASE | 0x00000003),
    TSEXT_ERR_TIME_OUT            = (TSEXT_ERR_BASE | 0x00000004),
    TSEXT_ERR_NO_IMPLEMENT        = (TSEXT_ERR_BASE | 0x00000005),

    // ts split
    TSS_ERR_OK                  = 0,
    TSS_ERR_ALLOCATE_FAIL       = (TSS_ERR_BASE | 0x00000001),
    TSS_ERR_NULL_POINTER        = (TSS_ERR_BASE | 0x00000002),
    TSS_ERR_INVALID_PARAMETER   = (TSS_ERR_BASE | 0x00000003),
    TSS_ERR_TIME_OUT            = (TSS_ERR_BASE | 0x00000004),
    TSS_ERR_NO_IMPLEMENT        = (TSS_ERR_BASE | 0x00000005),

    // ts packet analysis
    TSPA_ERR_OK                  = 0,
    TSPA_ERR_ALLOCATE_FAIL       = (TSPA_ERR_BASE | 0x00000001),
    TSPA_ERR_NULL_POINTER        = (TSPA_ERR_BASE | 0x00000002),
    TSPA_ERR_INVALID_PARAMETER   = (TSPA_ERR_BASE | 0x00000003),
    TSPA_ERR_TIME_OUT            = (TSPA_ERR_BASE | 0x00000004),
    TSPA_ERR_NO_IMPLEMENT        = (TSPA_ERR_BASE | 0x00000005),


    // ts packet demux
    TSPD_ERR_OK                  = 0,
    TSPD_ERR_ALLOCATE_FAIL       = (TSPD_ERR_BASE | 0x00000001),
    TSPD_ERR_NULL_POINTER        = (TSPD_ERR_BASE | 0x00000002),
    TSPD_ERR_INVALID_PARAMETER   = (TSPD_ERR_BASE | 0x00000003),
    TSPD_ERR_TIME_OUT            = (TSPD_ERR_BASE | 0x00000004),
    TSPD_ERR_NO_IMPLEMENT        = (TSPD_ERR_BASE | 0x00000005),

}TSE_ERR;
//=============================================================================
//				  Macro Definition
//=============================================================================

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

#ifdef __cplusplus
}
#endif

#endif
