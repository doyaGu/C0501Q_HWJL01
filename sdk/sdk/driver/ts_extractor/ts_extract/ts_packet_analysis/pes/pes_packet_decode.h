#ifndef __pes_packet_decode_H_0QZcd0Ov_WRrG_tfbc_6AsO_jvRzsIkDXPBg__
#define __pes_packet_decode_H_0QZcd0Ov_WRrG_tfbc_6AsO_jvRzsIkDXPBg__

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_packet_analysis_defs.h"

#include "pes_stream_operator.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define ENABLE_PES_COPY_PARSE           0 // use cacsh buffer queue mgr(no implement)


#define PES_DISCONTINUITY               (0x10000000)
#define PES_TRANSPORT_ERROR_INDICATOR   (0x20000000)

#define PES_START_CODE_PREFIX           (0x000001)

// See H222.0 p35 Table 2-18 to get further information
#define PROGRAM_STREAM_MAP_ID           (0xBC)
#define PADDING_STREAM_ID               (0xBE)
#define PRIVATE_STREAM_2_ID             (0xBF)
#define ECM_ID                          (0xF0)
#define EMM_ID                          (0xF1)
#define PROGRAM_STREAM_DIRECTORY_ID     (0xFF)
#define DSMCC_STREAM_ID                 (0xF2)
#define ITU_H222_1_TYPE_E_STREAM_ID     (0xF8)

#define PTS_FIELD_ONLY                  (0x2)
#define PTS_DTS_FIELD_EXIST             (0x3)
#define PES_PTS_DTS_SECTION_SIZE        (5)

#define PES_SEGMENT_SIZE                (40960)
#define PES_PRIOR_HEADER_DATA_LENGTH    (9)


typedef enum STREAM_TYPE_T
{
    ISO_IEC_RESERVED                = 0x00,
    ISO_IEC_11172_2_VIDEO           = 0x01,
    ISO_IEC_13818_2_VIDEO           = 0x02,
    ISO_IEC_11172_3_AUDIO           = 0x03,
    ISO_IEC_13818_3_AUDIO           = 0x04,
    ISO_IEC_13818_1_PRIVATE_SECTION = 0x05,
    ISO_IEC_13818_1_PES             = 0x06,
    ISO_IEC_13522_MHEG              = 0x07,
    ANNEX_A_DSM_CC                  = 0x08,
    ITU_T_REC_H_222_1               = 0x09,
    ISO_IEC_13818_6_TYPE_A          = 0x0A,
    ISO_IEC_13818_6_TYPE_B          = 0x0B,
    ISO_IEC_13818_6_TYPE_C          = 0x0C,
    ISO_IEC_13818_6_TYPE_D          = 0x0D,
    ISO_IEC_13818_1_AUXILIARY       = 0x0E,
    ISO_IEC_13818_7_AUDIO           = 0x0F,
    ISO_IEC_14496_2_VISUAL          = 0x10,
    ISO_IEC_14496_3_AUDIO           = 0x11,
    ISO_IEC_14496_10_VIDEO          = 0x1B,
    ISO_IEC_13818_1_RESERVED        = 0x1C,
    USER_PRIVATE                    = 0x80,

    // ccHDTv
    ITE_CCHDTV_DATA_CHANNEL         = 0x90,
    ITE_DTV_CAM_DEV_INFO            = 0x91,
} STREAM_TYPE;


//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct PES_PKT_DECODER_T
{
    PES_STREAM_OPR          *pCur_Pes_Stream_Desc;
    PES_STREAM_DECODER      pes_stream_decoder;

}PES_PKT_DECODER;
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
pes_pkt_register_all_decoder(
    void);


uint32_t
pes_pkt_create_decoder(
    PES_PKT_DECODER     **ppPesPktDecoder,
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData);


uint32_t
pes_pkt_destroy_decoder(
    PES_PKT_DECODER     **ppPesPktDecoder,
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData);


void
pes_pkt_decode(
    PES_PKT_DECODER     *pPesPktDecoder,
    uint8_t             *pPktData);





#ifdef __cplusplus
}
#endif

#endif
