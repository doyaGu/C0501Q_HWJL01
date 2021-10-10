#ifndef __psi_table_pat_H_Mj5FqqMn_MESc_WcNU_ic92_lek884rYyELD__
#define __psi_table_pat_H_Mj5FqqMn_MESc_WcNU_ic92_lek884rYyELD__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
// A strucut to store the program mapping information,
// that is, pogram_number -> program_map_PID.
typedef struct PSI_PAT_PROGRAM_T
{
    uint32_t                  program_number;
    uint32_t                  program_map_PID;
    struct PSI_PAT_PROGRAM_T* pNextProgram;
}PSI_PAT_PROGRAM;

// A structure to keep the information of the PAT table.
// The main key to identify different PAT table is by
// transport_stream_id.
typedef struct PSI_PAT_INFO_T
{
    uint32_t          transport_stream_id;
    uint32_t          version_number;
    uint32_t          current_next_indicator;
    uint32_t          totalProgramCount;
    PSI_PAT_PROGRAM*  pFirstProgram;

}PSI_PAT_INFO;
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
