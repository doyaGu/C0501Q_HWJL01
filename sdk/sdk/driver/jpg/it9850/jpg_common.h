#ifndef __jpg_common_H_JCIYt8Ac_OpND_w19w_jMbX_CQ4GpgstUhAA__
#define __jpg_common_H_JCIYt8Ac_OpND_w19w_jMbX_CQ4GpgstUhAA__

#ifdef __cplusplus
extern "C" {
#endif


#include "ite_jpg.h"
#include "jpg_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_ENC_DQT_LENGTH     (2 + ((JPG_Q_TABLE_SIZE+1) << 1))
#define JPG_ENC_DRI_LENGTH     4
#define JPG_ENC_SOF_LENGTH     (8 + 3 * 3)
#define JPG_ENC_DHT_LENGTH     (2 + ((29 + 179) << 1))
#define JPG_ENC_SOS_LENGTH     (6 + 2 * 3)

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

/**
 * initial parameters for jpg common handler
 **/
typedef struct JCOMM_INIT_PARAM_TAG
{
    JPG_CODEC_TYPE      codecType;

    JPG_DISP_INFO       *pJDispInfo;

    // I/O handler
    JPG_STREAM_HANDLE   *pHInJStream;
    JPG_STREAM_HANDLE   *pHOutJStream;

    // encode info
    bool                bPartialEnc;
    JPG_COLOR_SPACE     encColorSpace;
    uint32_t            encQuality;
    uint32_t            encWidth;
    uint32_t            encHeight;

}JCOMM_INIT_PARAM;

/** encoding flow
 *         set               H/w Write                   copy
 * LineBuf ---> Jpg H/W Enc -----------> H/w bs ring buf -----> jBsBuf (sysBsBuf)
 */
typedef struct JPG_BS_RING_BUF_INFO_TAG
{
    uint8_t             *pBsRingBuf;  // jpg H/W encoding bit stream ring buffer
    uint32_t            bsRingBufLeng;
    uint32_t            rwSize;

}JPG_BS_RING_BUF_INFO;

/**
 * handler for codec
 **/
typedef struct JCOMM_HANDLE_TAG
{
    uint32_t            chipId;

    int                 actIOBuf_idx;
    JPG_BUF_INFO        jInBufInfo[4];
    JPG_BUF_INFO        jOutBufInfo[4];

    JPG_FRM_COMP        *pJFrmComp;
    JPG_DISP_INFO       *pJDispInfo;

    uint32_t            remainSize;  // file remain size (un-process), from jPrsInfo
    uint32_t            realSzie;    // real filled data size, from jPrsInfo
    bool                b1stPass;    // the 1-st decoding section or not

    // for encode
    uint8_t             *pSysBsBufAddr;
    uint32_t            sysBsBufSize;
    uint32_t            sysValidBsBufSize;
    uint8_t             *pJHdrData;
    uint32_t            jHdrDataSize;
    uint32_t            encSectHight;

    // for win32
    JPG_BUF_INFO        jSysOutBufInfo[4];

}JCOMM_HANDLE;
//=============================================================================
//                  Global Data Definition
//=============================================================================
//extern uint8_t Def_QTable_Y[JPG_Q_TABLE_SIZE];
//extern uint8_t Def_QTable_UV[JPG_Q_TABLE_SIZE];
extern const uint8_t const Def_DCHuffTable[2][28];
extern const uint8_t const Def_ACHuffTable[2][178];

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_ERR
jComm_CreateHandle(
    JCOMM_HANDLE        **pHJComm,
    void                *extraData);


JPG_ERR
jComm_DestroyHandle(
    JCOMM_HANDLE    **pHJComm,
    void            *extraData);


JPG_ERR
jComm_Init(
    JCOMM_HANDLE        *pHJComm,
    JCOMM_INIT_PARAM    *pJCommInitParam,
    void                *extraData);


JPG_ERR
jComm_deInit(
    JCOMM_HANDLE        *pHJComm,
    void                *extraData);


JPG_ERR
jComm_Setup(
    JCOMM_HANDLE        *pHJComm,
    void                *extraData);


JPG_ERR
jComm_Fire(
    JCOMM_HANDLE        *pHJComm,
    bool                bLastSection,
    void                *extraData);


JPG_ERR
jComm_Control(
    JCOMM_HANDLE    *pHJComm,
    uint32_t        cmd,
    uint32_t        *value,
    void            *extraData);


#ifdef __cplusplus
}
#endif

#endif

