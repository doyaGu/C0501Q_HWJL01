
#include "tscam_ctrl_cfg.h"
#include "tscam_ctrl_def.h"
#include "tscam_ctrl.h"

#if (CONFIG_TSCM_DESC_DUPLEX_RS232_DESC)

#include "ite/itp.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#define MY_MAX_COUNT            10
#define MY_CACHE_SIZE           (10 << 10)
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct RECV_MGR_T
{
    uint32_t    uid;
    uint8_t     *pCache;
}RECV_MGR;
//=============================================================================
//				  Global Data Definition
//=============================================================================
static RECV_MGR      recv_mgr[MY_MAX_COUNT] = {0};
//=============================================================================
//				  Private Function Definition
//=============================================================================
TSCM_ERR
duplex_rs232_init(
    TSCM_HANDLE     *pHTscam,
    TSCM_INIT_INFO  *pInit_info,
    void            *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;
    uint32_t    i = 0;
    bool        bExist = false;
    do{
        for(i = 0; i < MY_MAX_COUNT; i++)
        {
            if( recv_mgr[i].uid == (uint32_t)pHTscam )
            {
                bExist = true;
                break;
            }
        }

        if( bExist == true )    break;

        for(i = 0; i < MY_MAX_COUNT; i++)
        {
            if( recv_mgr[i].uid == 0 )
                break;
        }

        if( i < MY_MAX_COUNT )
        {
            recv_mgr[i].uid = (uint32_t)pHTscam;
            recv_mgr[i].pCache = malloc(MY_CACHE_SIZE);
            if( !recv_mgr[i].pCache )
            {
                recv_mgr[i].uid = 0;
                printf("err, malloc fail !! %s[%d]\n", __FUNCTION__, __LINE__);
            }
        }
    }while(0);

    return result;
}

TSCM_ERR
duplex_rs232_deinit(
    TSCM_HANDLE *pHTscam,
    void        *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;
    uint32_t    i = 0;
    bool        bExist = false;

    do{
        for(i = 0; i < MY_MAX_COUNT; i++)
        {
            if( recv_mgr[i].uid == (uint32_t)pHTscam )
            {
                bExist = true;
                break;
            }
        }

        if( bExist == true )
        {
            if( recv_mgr[i].pCache )    free(recv_mgr[i].pCache);
            recv_mgr[i].pCache = 0;
            recv_mgr[i].uid    = 0;
        }
    }while(0);
    return result;
}

TSCM_ERR
duplex_rs232_stream_recv(
    TSCM_HANDLE     *pHTscam,
    TSCM_RECV_INFO  *pRecv_info,
    void            *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;
    static

    if( pHTscam && pRecv_info )
    {
        uint32_t    i = 0;
        bool        bExist = false;

        for(i = 0; i < MY_MAX_COUNT; i++)
        {
            if( recv_mgr[i].uid == (uint32_t)pHTscam )
            {
                bExist = true;
                break;
            }
        }

        if( bExist == true )
        {
            int     n = 0;
            n = read(ITP_DEVICE_UART1, &recv_mgr[i].pCache, MY_CACHE_SIZE);
            pRecv_info->pSteam_buf      = recv_mgr[i].pCache;
            pRecv_info->stream_buf_size = n;
        }
        else
            pRecv_info->pSteam_buf = pRecv_info->stream_buf_size = 0;
    }

    return result;
}

TSCM_ERR
duplex_rs232_stream_trans(
    TSCM_HANDLE         *pHTscam,
    TSCM_TRANS_INFO     *pTrans_info,
    void                *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;

    if( pHTscam && pTrans_info && pTrans_info->pSteam_buf && pTrans_info->stream_buf_size )
        write(ITP_DEVICE_UART1, pTrans_info->pSteam_buf, pTrans_info->stream_buf_size);

    return result;
}

TSCM_ERR
duplex_rs232_control(
    TSCM_HANDLE     *pHTscam,
    TSCM_ARG        *pArg,
    void            *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;

    return result;
}
//=============================================================================
//				  Public Function Definition
//=============================================================================
TSCM_DESC TSCM_DESC_duplex_rs232_desc =
{
    "ts cam duplex rs232",     // char                *name;
    0,                          // struct TSCM_DESC_T  *next;
    TSCM_TYPE_DUPLEX_RS232,    // TSCM_TYPE           tscm_type;
    0,                          // void        *pExtraInfo;
    duplex_rs232_init,         // TSCM_ERR    (*init)(struct TSCM_HANDLE_T *pHTscam, TSCM_INIT_INFO *pInit_info, void *extraData);
    duplex_rs232_deinit,       // TSCM_ERR    (*deinit)(struct TSCM_HANDLE_T *pHTscam, void *extraData);
    duplex_rs232_stream_recv,  // TSCM_ERR    (*stream_recv)(struct TSCM_HANDLE_T *pHTscam, TSCM_RECV_INFO *pRecv_info, void *extraData);
    duplex_rs232_stream_trans, // TSCM_ERR    (*stream_trans)(struct TSCM_HANDLE_T *pHTscam, TSCM_TRANS_INFO *pTrans_info, void *extraData);
    duplex_rs232_control,      // TSCM_ERR    (*control)(struct TSCM_HANDLE_T *pHTscam, TSCM_ARG *pArg, void *extraData);
};

#else
TSCM_DESC TSCM_DESC_duplex_rs232_desc =
{
    "ts cam duplex rs232",     // char                *name;
    0,                          // struct TSCM_DESC_T  *next;
    TSCM_TYPE_DUPLEX_RS232,    // TSCM_TYPE           tscm_type;
    0,                          // void        *pExtraInfo;
};
#endif