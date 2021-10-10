#ifndef __aggre_desc_H_MCxDqjCO_A2K1_eGmT_oCzi_mi6Y5ZX9ZWQU__
#define __aggre_desc_H_MCxDqjCO_A2K1_eGmT_oCzi_mi6Y5ZX9ZWQU__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
#include "ts_aggre_defs.h"
#include "ts_aggre_cfg.h"
#include "ts_aggre_ctrl.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ts aggregation dev
 **/
typedef struct TSA_DEV_T
{
    uint32_t        aggre_index;
    uint32_t        aggre_i2c_addr;
    TSA_MODE        tsa_mode;

    uint32_t        total_demod_port;

    TSA_TAG_LEN     tsa_tag_len;

    // reg[tag_value3] << 24 | reg[tag_value2] << 16 | reg[tag_value1] << 8 | reg[tag_value0]
    uint32_t        tag_value[TSA_MAX_PORT_NUM];

    // for demod
    TSA_BUS_TYPE    bus_type;
    uint32_t        demod_i2c_addr[TSA_MAX_PORT_NUM];
    uint32_t        linked_aggre_port_idx[TSA_MAX_PORT_NUM];

    void            *privData;

}TSA_DEV;

/**
 * ts aggregation descriptor
 **/
typedef struct TSA_DESC_T
{
    char        *name;
    struct TSA_DESC_T  *next;
    TS_AGGR_ID         id;

    void        *privInfo;

    uint32_t    (*init)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);

    uint32_t    (*deinit)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);

    uint32_t    (*enable_port)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);
    uint32_t    (*disable_port)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);

    uint32_t    (*set_aggre_mode)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);

    uint32_t    (*control)(TSA_DEV *pTsaDev, TSA_AGR *pTsaArg, void *extraData);

}TSA_DESC;
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
