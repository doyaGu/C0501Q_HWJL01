#ifndef __psi_priv_sect_demux_H_LsV5AvAq_oher_3YvK_SrLN_5i5HHbPhHycC__
#define __psi_priv_sect_demux_H_LsV5AvAq_oher_3YvK_SrLN_5i5HHbPhHycC__

#ifdef __cplusplus
extern "C" {
#endif


#include "psi_table_operator.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define NIT_ACTUAL_TABLE_ID                             (0x40)
#define NIT_OTHER_TABLE_ID                              (0x41)
#define SDT_ACTUAL_TABLE_ID                             (0x42)
#define EIT_ACTUAL_PRESENT_FOLLOWING_EVENT_TABLE_ID     (0x4E)
#define EIT_ACTUAL_SCHEDULE_EVENT_MIN_TABLE_ID          (0x50)

#if defined(SCHEDULE_EVENT_TABLE_NUMBER)
    #define EIT_ACTIAL_SCHEDULE_EVENT_MAX_TABLE_ID      (0x4F + SCHEDULE_EVENT_TABLE_NUMBER)
#else
    #define EIT_ACTIAL_SCHEDULE_EVENT_MAX_TABLE_ID      (0x50)
#endif

#define TDT_TABLE_ID                                    (0x70)
#define TOT_TABLE_ID                                    (0x73)
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================3
/**
 * NOTE: In SI, it's not allowed to tell tables by PID, because some have the
 *       same one. So using "(table_id << 16) | table_id_extension" to be the
 *       only one identification.
 **/

// This structure contains the data specific to the decoding of one subtable.
typedef struct PSI_DEMUX_SUBDECODER_T
{
    uint32_t                            id; // (table_id << 16) | table_id_extension


    // The callback function will be called while a section needs to be decoded by
    // the subtable decoder.
    void (*gather_sect)(PSI_TABLE_DECODER    *pPsi_table_decoder,
                        void                 *pSubdecoder_priv_decoder,
                        PSI_SECT             *pSect);

    // A private decoder that be used and keep data at local for specific
    // subtable decoding.
    void*                               ptPrivDecoder;

    struct PSI_DEMUX_SUBDECODER_T       *ptNextSubdecoder;

    // Provide function callback in detach process.
    void (*detach_subdecoder)(struct PSI_PRIV_SECT_DEMUX_T *pPriv_sect_demux,
                                     PSI_TABLE_MBOX_ARG    *pPsi_table_mbox_arg,
                                     void                  *extraData);
}PSI_DEMUX_SUBDECODER;

/**
 * In ITU-T H.222.0, SDT/NIT/EIT are in private_section.
 * So we need a priv_sect_demux to split SDT/NIT/EIT section
 **/
typedef struct PSI_PRIV_SECT_DEMUX_T
{
    // This function will be called when find a new subtable,
    // callback to upper layer to do something like attaching subtable decoder.
    void (*attach_psi_table_decoder)(struct PSI_PRIV_SECT_DEMUX_T *pPriv_sect_demux,
                                            PSI_TABLE_MBOX_ARG    *pPsi_table_mbox_arg,
                                            void                  *extraData);
    // map to PSI_DEMUX_CALLBACK

    PSI_TABLE_DECODER       *pPsi_table_decoder;

    PSI_DEMUX_SUBDECODER    *ptFirstSubdecoder;

    PSI_TABLE_MBOX          psi_pkt_dec_mbox;   // from psi_packet_decoder mbox


    PSI_DEMUX_SUBDECODER*
    (*get_subtable_decoder)(struct PSI_PRIV_SECT_DEMUX_T *pPriv_sect_demux,
                                   PSI_TABLE_MBOX_ARG    *pPsi_table_mbox_arg,
                                   void                  *extraData);

}PSI_PRIV_SECT_DEMUX;
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
