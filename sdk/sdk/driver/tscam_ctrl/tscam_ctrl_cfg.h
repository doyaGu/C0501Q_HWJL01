#ifndef __tscam_ctrl_cfg_H_DVfvUzOk_becl_FKeR_Ow2X_eUplUXwMAvqr__
#define __tscam_ctrl_cfg_H_DVfvUzOk_becl_FKeR_Ow2X_eUplUXwMAvqr__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================

#if defined(CFG_TSCAM_PKT_FMT_SIMPLEX_RS232) && !defined(_MSC_VER)
    #define CONFIG_TSCM_DESC_SIMPLEX_RS232_DESC    1
#else
    #define CONFIG_TSCM_DESC_SIMPLEX_RS232_DESC    0
#endif

#if defined(CFG_TSCAM_PKT_FMT_DUPLEX_RS232) && !defined(_MSC_VER)
    #define CONFIG_TSCM_DESC_DUPLEX_RS232_DESC    1
#else
    #define CONFIG_TSCM_DESC_DUPLEX_RS232_DESC    0
#endif
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
