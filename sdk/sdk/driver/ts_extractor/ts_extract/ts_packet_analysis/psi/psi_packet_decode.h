#ifndef __psi_packet_decode_H_FZvmIsjJ_Cuqo_YRKD_Fd3e_9wgAntVk2gyL__
#define __psi_packet_decode_H_FZvmIsjJ_Cuqo_YRKD_Fd3e_9wgAntVk2gyL__

#ifdef __cplusplus
extern "C" {
#endif


#include "ts_extractor_defs.h"

#include "psi_table_operator.h"
#include "psi_priv_sect_demux.h"
#include "psi_table_pat.h"
#include "psi_table_pmt.h"
#include "psi_table_sdt.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct PSI_PKT_DECODER_T
{
    PSI_TABLE_OPR           *pCur_Psi_Table_Desc;

    PSI_TABLE_DECODER       psi_table_decoder;

}PSI_PKT_DECODER;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void
psi_pkt_register_all_decoder(
    void);


uint32_t
psi_pkt_create_decoder(
    PSI_PKT_DECODER     **ppPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData);


uint32_t
psi_pkt_destroy_decoder(
    PSI_PKT_DECODER     **ppPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData);


void
psi_pkt_decode(
    PSI_PKT_DECODER     *pPsiPktDecoder,
    uint8_t             *pPktData);


void
psi_pkt_clear_table(
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData);


#ifdef __cplusplus
}
#endif

#endif
