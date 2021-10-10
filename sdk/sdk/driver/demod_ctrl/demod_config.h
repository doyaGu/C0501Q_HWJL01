#ifndef __DEMOD_CONFIG_H_38TMS8FN_U0BQ_4FFD_YRGR_C3LCDICLX3ZI__
#define __DEMOD_CONFIG_H_38TMS8FN_U0BQ_4FFD_YRGR_C3LCDICLX3ZI__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * enable demod type
 **/

#if (CFG_DEMOD_SUPPORT_COUNT > 0) && defined(CFG_DEMOD_OMEGA) && !defined(_MSC_VER)
    #define CONFIG_DEMOD_DESC_OMEGA_DESC    1
#else
    #define CONFIG_DEMOD_DESC_OMEGA_DESC    0
#endif

#if (CFG_DEMOD_SUPPORT_COUNT > 0) && defined(CFG_DEMOD_IT9135) && !defined(_MSC_VER)
    #define CONFIG_DEMOD_DESC_IT9135_DESC    1
#else
    #define CONFIG_DEMOD_DESC_IT9135_DESC    0
#endif

#if (CFG_DEMOD_SUPPORT_COUNT > 0) && defined(CFG_DEMOD_IT9137) && !defined(_MSC_VER)
    #define CONFIG_DEMOD_DESC_IT9137_DESC    1
#else
    #define CONFIG_DEMOD_DESC_IT9137_DESC    0
#endif

#if (CFG_DEMOD_SUPPORT_COUNT > 0) && defined(CFG_DEMOD_IT9137_USB) && !defined(_MSC_VER)
    #define CONFIG_DEMOD_DESC_IT9137_USB_DESC    1
#else
    #define CONFIG_DEMOD_DESC_IT9137_USB_DESC    0
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

