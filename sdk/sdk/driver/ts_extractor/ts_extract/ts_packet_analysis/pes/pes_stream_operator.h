#ifndef __pes_stream_operator_H_Loms8Ekc_b828_4aac_2mje_6YoyKn5rmnrj__
#define __pes_stream_operator_H_Loms8Ekc_b828_4aac_2mje_6YoyKn5rmnrj__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * The PES header start with packet_start_code_prefix(24) + stream_id(8)
 * + PES_packet_length(16) = 6 bytes. After read these 6 bytes, then we
 * are able to get the packet length of PES packet.
 **/
#define PES_PRIOR_HEADER_SIZE               (6)

/**
 * PES stream type ID
 **/
typedef enum PES_STREAM_ID_T
{
    PES_STREAM_UNKNOW     = 0,
    PES_STREAM_VIDEO,
    PES_STREAM_AUDIO,
    PES_STREAM_TELETEXT,
    PES_STREAM_SUBTITLE,

}PES_STREAM_ID;
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * msg box arguments of pes stream decoder
 **/
typedef struct PES_STREAM_MBOX_ARG_T
{
    PES_STREAM_ID       pes_stream_id;
    uint32_t            reserved;

    union{
        struct{
            void        *pPes_stream_decoder;
            uint32_t    esPID;
            void        *pTunnelInfo;
            void        *pPes_info;
            uint32_t    data_size;
            uint8_t     *pData;
            uint32_t    video_stream_type;
            bool        bGetResolution;
        }v; // video

        struct{
            void        *pPes_stream_decoder;
            uint32_t    esPID;
            void        *pTunnelInfo;
            void        *pPes_info;
            uint32_t    data_size;
            uint8_t     *pData;
        }a; // audio

        struct{
            void        *pPes_stream_decoder;
            uint32_t    esPID;
            void        *pTunnelInfo;
            void        *pPes_info;
            uint32_t    data_size;
            uint8_t     *pData;
        }t; // teletext

        struct{
            void        *pPes_stream_decoder;
            uint32_t    esPID;
            void        *pTunnelInfo;
            void        *pPes_info;
            uint32_t    data_size;
            uint8_t     *pData;
        }s; // subtitle
    }arg;

}PES_STREAM_MBOX_ARG;

/**
 * msg box set to pes stream decoder
 **/
typedef struct PES_STREAM_MBOX_T
{
    uint32_t    (*func)(PES_STREAM_MBOX_ARG *pPes_stream_mbox_arg, void *extraData);

    PES_STREAM_MBOX_ARG  pes_stream_mbox_arg;

}PES_STREAM_MBOX;

/**
 * pes stream operator
 **/
typedef struct PES_STREAM_OPR_T
{
    char        *name;

    struct PES_STREAM_OPR_T     *next;
    PES_STREAM_ID               id;

    void        *privInfo;

    uint32_t    (*init)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    uint32_t    (*deinit)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    uint32_t    (*proc)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    uint32_t    (*get_info)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);

}PES_STREAM_OPR;

/**
 * pes info
 **/
typedef struct PES_INFO_T
{
    uint32_t      elementary_PID;

    uint32_t      packet_start_code_prefix;
    uint32_t      stream_id;
    uint32_t      PES_packet_length;

    uint32_t      errorFlag;
    uint32_t      errorCount;
    // iclai+
    // teletext needs the following fields to verify if the PES data is correct
    // or not
    uint32_t      PES_header_data_length;
    // iclai-

    uint8_t       *pData; // Include header and payload
    uint8_t       *pPayloadStartAddress;
    uint8_t       *pPayloadEndAddress;
    uint32_t      gatherSize;

}PES_INFO;

/**
 * pes stream decoder
 **/
typedef struct PES_STREAM_DECODER_T
{
    uint32_t              elementary_PID;
    uint32_t              continuity_counter;

    uint8_t               pPriorHeader[PES_PRIOR_HEADER_SIZE];
    uint32_t              priorHeaderReadByte; // total bytes read for prior header

    bool                  bDiscontinuity;
    bool                  bErrorIndicator;

    // Whether the first packet be handled by the decoder yet?
    bool                 bFirstPacket;

    PES_INFO             *ptBuildingPes;
    PES_INFO             tBuildingPes;

    PES_STREAM_MBOX      pes_pkt_dec_mbox; // from pes_packet_decoder mbox

    void                 *privData;
}PES_STREAM_DECODER;
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
