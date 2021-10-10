#ifndef __ts_split_H_HcVIjvbd_DhrG_zig1_sJAn_8EaXdDPl2RYB__
#define __ts_split_H_HcVIjvbd_DhrG_zig1_sJAn_8EaXdDPl2RYB__

#ifdef __cplusplus
extern "C" {
#endif


// #include "ts_extractor_defs.h"

#include <stdint.h>
#include <stdbool.h>

#include "ite_ts_extractor.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define TSS_CMD_USER_OPR_IDLE           0
#define TSS_CMD_USER_OPR_CREATE         1
#define TSS_CMD_USER_OPR_DESTROY        2

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ts split argument of msg box
 **/
typedef struct TSS_MBOX_ARG_T
{
#define TSS_MBOX_TYPE_USER_CB      0x11

    uint32_t        type;

    union{
        struct{
            uint32_t    index;
            uint8_t     *pBuf_addr;
            uint32_t    buf_size;
            uint32_t    feedback_cmd;
            void        *pTunnelInfo;
        }user;
    }arg;

}TSS_MBOX_ARG;

/**
 * ts split msg box
 **/
typedef struct TSS_MBOX_T
{
    uint32_t    (*func)(TSS_MBOX_ARG *pMbox_arg, void *extraData);

    TSS_MBOX_ARG    tss_mbox_arg;
}TSS_MBOX;


/**
 * user operator
 **/
typedef struct TSS_USER_OPR_T
{
    void        *privData;

    uint32_t    port_index;
    TSS_MBOX    user_create;
    TSS_MBOX    user_destroy;
    TSS_MBOX    user_proc;

}TSS_USER_OPR;


/**
 * info of aggre module
 **/
typedef struct TSS_PASSPORT_INFO_T
{
    // Coordinate with aggre module
    uint32_t        tag_len;

    // reg[tag_value3] << 24 | reg[tag_value2] << 16 | reg[tag_value1] << 8 | reg[tag_value0]
    uint32_t        tag_value;

    uint32_t        id;

}TSS_PASSPORT_INFO;

/**
 * initial paraments when create handle
 **/
typedef struct TSS_INIT_PARAM_T
{
    bool            bBy_Pass_Tss;

    //TSS_PASSPORT_INFO       tssPassportInfo;

}TSS_INIT_PARAM;


/**
 * ts split buffer info
 **/
typedef struct TSS_BUF_INFO_T
{
    uint8_t         *pBufAddr;
    uint32_t        bufLength;
}TSS_BUF_INFO;

/**
 * ts split handle
 **/
typedef struct TSS_HANDLE_T
{
    uint32_t        reserved;
}TSS_HANDLE;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
TSE_ERR
tss_CreateHandle(
    TSS_HANDLE        **pHTss,
    TSS_INIT_PARAM    *pInitParam,
    void              *extraData);


TSE_ERR
tss_DestroyHandle(
    TSS_HANDLE        **pHTss,
    void              *extraData);


TSE_ERR
tss_Attach_User_Operator(
    TSS_HANDLE      *pHTss,
    TSS_USER_OPR    *pUser_opr,
    void            *extraData);


TSE_ERR
tss_Add_Passport_Info(
    TSS_HANDLE          *pHTss,
    TSS_PASSPORT_INFO   *pPassportInfo,
    void                *extraData);


TSE_ERR
tss_Split(
    TSS_HANDLE      *pHTss,
    TSS_BUF_INFO    *pBuf_info,
    void            *extraData);


#ifdef __cplusplus
}
#endif

#endif
