#ifndef __TS_AIRFILE_CFG_H_6NQ38BMO_SCWF_T388_FK6T_9MO2A3F9DBGD__
#define __TS_AIRFILE_CFG_H_6NQ38BMO_SCWF_T388_FK6T_9MO2A3F9DBGD__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * enable ts air file type
 **/
#define CONFIG_TS_AIRFILE_DESC_NETTV_DESC               0
#define CONFIG_TS_AIRFILE_DESC_GATEWAY_DESC             0

#if defined(CFG_TS_DEMUX_ENABLE)
    #define CONFIG_TS_AIRFILE_DESC_SPLIT_GATEWAY_DESC       1
#else
    #define CONFIG_TS_AIRFILE_DESC_SPLIT_GATEWAY_DESC       0
#endif

#if defined(CFG_TS_EXTRACTOR_ENABLE)
    #define CONFIG_TS_AIRFILE_DESC_AGGRE_GATEWAY_DESC       1
#else
    #define CONFIG_TS_AIRFILE_DESC_AGGRE_GATEWAY_DESC       0
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
