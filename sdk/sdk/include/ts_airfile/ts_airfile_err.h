#ifndef __ts_airfile_err_H_hDGTD3d3_BSFz_qoyg_TJLF_qEoexskcbpaD__
#define __ts_airfile_err_H_hDGTD3d3_BSFz_qoyg_TJLF_qEoexskcbpaD__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================
/**
 *  ts demuxer error code
 */
#define  TSAF_ERR_BASE            0xAF000000
typedef enum _TSAF_ERR_TAG
{
    // ts airfile
    TSAF_ERR_OK                  = 0,
    TSAF_ERR_ALLOCATE_FAIL       = (TSAF_ERR_BASE | 0x00000001),
    TSAF_ERR_NULL_POINTER        = (TSAF_ERR_BASE | 0x00000002),
    TSAF_ERR_INVALID_PARAMETER   = (TSAF_ERR_BASE | 0x00000003),
    TSAF_ERR_TIME_OUT            = (TSAF_ERR_BASE | 0x00000004),
    TSAF_ERR_NO_IMPLEMENT        = (TSAF_ERR_BASE | 0x00000005),
    TSAF_ERR_DEMOD_FAIL          = (TSAF_ERR_BASE | 0x00000006),
    TSAF_ERR_SCAN_FAIL           = (TSAF_ERR_BASE | 0x00000007),

}TSAF_ERR;
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
