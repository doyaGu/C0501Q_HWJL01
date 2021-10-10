#ifndef __psi_table_desc_H_LR7kbGyz_8mj2_tEX2_ItFB_dyDHkyl4ovLe__
#define __psi_table_desc_H_LR7kbGyz_8mj2_tEX2_ItFB_dyDHkyl4ovLe__

#ifdef __cplusplus
extern "C" {
#endif


#include "psi_section_kit.h"
//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum PSI_TABLE_ID_T
{
    PSI_TABLE_NONE,
    PSI_TABLE_PAT,
    PSI_TABLE_PMT,
    PSI_TABLE_SDT,
    PSI_TABLE_NIT,  // not ready
    PSI_TABLE_EIT,  // not ready

}PSI_TABLE_ID;


//=============================================================================
//                Macro Definition
//=============================================================================


//=============================================================================
//                Structure Definition
//=============================================================================
struct PSI_TABLE_DECODER_T;

#if 0 // reserve code
/**
 * The callback function will be called after the decode of psi table is done.
 **/
struct PSI_TABLE_MBOX_ARG_T;
typedef uint32_t (*CB_PSI_TABLE_READY)(struct PSI_TABLE_MBOX_ARG_T *pPsiTableArg, void *extraData);
#endif

/**
 * msg box arguments of psi table decoder
 **/
typedef struct PSI_TABLE_MBOX_ARG_T
{
    PSI_TABLE_ID        psi_table_id;
    uint32_t            reserved;

    union{
        struct{
            struct PSI_TABLE_DECODER_T  *pPsi_Table_Decoder;
            void                        *pTunnelInfo;
            void                        *pPsi_Pat_Info;
        }pat;

        struct{
            struct PSI_TABLE_DECODER_T  *pPsi_Table_Decoder;
            void                        *pTunnelInfo;
            uint32_t                    pmtProgramNum;
            void                        *pPsi_Pmt_Info;
        }pmt;

        struct{
            struct PSI_TABLE_DECODER_T  *pPsi_Table_Decoder;
            void                        *pTunnelInfo;
            void                        *pPsiPktDecoder;
            void                        *pPriv_sect_demux;
            uint32_t                    table_id;
            uint32_t                    table_id_extension;
            void                        *pPsi_Sdt_Info;
        }sdt;
    }argv;

}PSI_TABLE_MBOX_ARG;

/**
 * msg box set to psi table decoder
 **/
typedef struct PSI_TABLE_MBOX_T
{
    uint32_t    (*func)(PSI_TABLE_MBOX_ARG *pPsi_table_mbox_arg, void *extraData);

    PSI_TABLE_MBOX_ARG  psi_table_mbox_arg;

}PSI_TABLE_MBOX;

/**
 * psi table operator
 **/
typedef struct PSI_TABLE_OPR_T
{
    char        *name;

    struct PSI_TABLE_OPR_T     *next;
    PSI_TABLE_ID                id;

    void        *privInfo;

    uint32_t    (*attach_decoder)(PSI_TABLE_MBOX *pPsiTableMbox, void *extraData);
    uint32_t    (*detach_decoder)(PSI_TABLE_MBOX *pPsiTableMbox, void *extraData);
    uint32_t    (*clear_table)(PSI_TABLE_MBOX *pPsiTableMbox, void *extraData);

}PSI_TABLE_OPR;

/**
 * psi table decoder
 **/
typedef void (*PSI_SECT_HANDLER)(struct PSI_TABLE_DECODER_T *pDecoder, PSI_SECT *pSect);

typedef struct PSI_TABLE_DECODER_T
{
    // Hang callback function to process PSI section. (process gathering
    // sections of a table to parse)
    PSI_SECT_HANDLER     pf_psi_sect_handler;

    // Point to individual PSI decoder structure. (PSI_XXX_DECODER)
    void*                pPrivDecoder;

    // Temporal store length.
    uint32_t             sectMaxSize;

    uint32_t             continuity_counter;
    bool                 bDiscontinuity;

    // Temporal store for a completed PSI section.
    PSI_SECT             *ptCurSect;

    // Indicate byte number in a section should be handled later.
    uint32_t             needByte;

    // Indicate whether header of section is processed. (3 bytes at header of
    // section to be parsed, to obtain section_length)
    bool                 bCompleteHeader;

    int                  allocId;
    SECT_PAYLOAD_ALLOC   pf_sect_alloc;
    SECT_PAYLOAD_FREE    pf_sect_free;

}PSI_TABLE_DECODER;


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
