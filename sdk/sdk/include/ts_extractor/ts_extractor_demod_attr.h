#ifndef __ts_extractor_demo_attr_H_sAzjXZVP_2eyZ_vteZ_ilXH_Aw2lv8YjGrY9__
#define __ts_extractor_demo_attr_H_sAzjXZVP_2eyZ_vteZ_ilXH_Aw2lv8YjGrY9__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum TSE_DEMOD_TYPE_T
{
    TSE_DEMOD_TYPE_UNKNOW       = 0,
    TSE_DEMOD_TYPE_IT9135,
    TSE_DEMOD_TYPE_IT9137,

}TSE_DEMOD_TYPE;

typedef enum TSE_DEMOD_BUS_TYPE_T
{
    TSE_DEMOD_BUS_UNKNOW,
    TSE_DEMOD_BUS_I2C,
    TSE_DEMOD_BUS_USB,
}TSE_DEMOD_BUS_TYPE;
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *  ts demod attribute
 **/
typedef struct TSE_DEMOD_ATTR_T
{
    uint32_t                demod_idx;
    TSE_DEMOD_BUS_TYPE      bus_type;
    TSE_DEMOD_TYPE          demod_type;  // demod chip type
    uint32_t                demod_i2c_addr;

    uint32_t                linked_aggre_port_idx;
}TSE_DEMOD_ATTR;
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
