#ifndef __psi_table_sdt_H_1UdknMIx_AUKV_RTyT_nVau_uhjuhgKdiGOd__
#define __psi_table_sdt_H_1UdknMIx_AUKV_RTyT_nVau_uhjuhgKdiGOd__

#ifdef __cplusplus
extern "C" {
#endif


#include "psi_descriptor_kit.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
//
// This structure is used to store a decoded SDT service description.
// (ETSI EN 300 468 V1.7.1 section 5.2.3).
//
typedef struct PSI_SDT_SERVICE_T
{
    uint32_t    service_id;
    uint32_t    EIT_schedule_flag;
    uint32_t    EIT_present_following_flag;
    uint32_t    running_status;
    uint32_t    free_CA_mode;
    PSI_DESCR   *ptFirstDescriptor;

    struct PSI_SDT_SERVICE_T   *ptNextService;
} PSI_SDT_SERVICE;

//
// This structure is used to store a decoded SDT.
// (ETSI EN 300 468 V1.7.1 section 5.2.3).
//
typedef struct PSI_SDT_INFO_T
{
    uint32_t            transport_stream_id;
    uint32_t            version_number;
    uint32_t            current_next_indicator;
    uint32_t            original_network_id;
    PSI_SDT_SERVICE     *ptFirstService;
}PSI_SDT_INFO;
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
