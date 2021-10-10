#ifndef __psi_table_pmt_H_c4QauTOs_xJ4v_SVhi_cVnz_gYZi6tjx4624__
#define __psi_table_pmt_H_c4QauTOs_xJ4v_SVhi_cVnz_gYZi6tjx4624__

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
// The structure is used to store each Elementary stream information, and also
// wrapped the descriptor list for AP usage.
// Note: The ptFirstDescriptor is pointing to the first descriptor of the
//       second descriptor list (ES info descriptor list) which is located
//       behide the field ES_info_length of TS_program_map_sections()
//       See H222.0 p48 to get further details.
typedef struct PSI_PMT_ES_INFO_T
{
    uint32_t        stream_type;
    uint32_t        elementary_PID;
    PSI_DESCR       *ptFirstDescriptor;

    struct PSI_PMT_ES_INFO_T    *ptNexEsInfo;

} PSI_PMT_ES_INFO;

// The structure is used to store whole PMT table information
// Note1: The ptFirstDescriptor is pointing to the first descriptor of the
//        first descriptor list (program info descriptor list) which is
//        located behide the field program_info_length of
//        TS_program_map_sections() See H222.0 p48 to get further details.
// Note2: There exists one or more elementray streams of a PMT table. The
//        ptFirstEsInfo is pointing to the first elementary stream info of
//        the ES_Info list. See H222.0 p48 to get further details.
typedef struct PSI_PMT_INFO_T
{
    uint32_t            pid;
    uint32_t            program_number;
    uint32_t            version_number;
    uint32_t            current_next_indicator;
    uint32_t            pcr_pid;
    PSI_DESCR           *ptFirstDescriptor;  // in practice, check if subtitle,
                                             // teletext and ac3 exist
    uint32_t            totalEsCount;
    PSI_PMT_ES_INFO*    ptFirstEsInfo;
}PSI_PMT_INFO;
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
