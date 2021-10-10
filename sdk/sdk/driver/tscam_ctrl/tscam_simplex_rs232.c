
#include "tscam_ctrl_cfg.h"
#include "tscam_ctrl_def.h"
#include "tscam_ctrl.h"

#if (CONFIG_TSCM_DESC_SIMPLEX_RS232_DESC)

#include <pthread.h>
#include "ite/itp.h"
//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================
static pthread_mutex_t g_trans_mutex  = PTHREAD_MUTEX_INITIALIZER;
//=============================================================================
//				  Private Function Definition
//=============================================================================
TSCM_ERR
simplex_rs232_init(
    TSCM_HANDLE     *pHTscam,
    TSCM_INIT_INFO  *pInit_info,
    void            *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;

    return result;
}

TSCM_ERR
simplex_rs232_deinit(
    TSCM_HANDLE *pHTscam,
    void        *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;

    return result;
}

TSCM_ERR
simplex_rs232_stream_recv(
    TSCM_HANDLE     *pHTscam,
    TSCM_RECV_INFO  *pRecv_info,
    void            *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;

    return result;
}

TSCM_ERR
simplex_rs232_stream_trans(
    TSCM_HANDLE         *pHTscam,
    TSCM_TRANS_INFO     *pTrans_info,
    void                *extraData)
{
    TSCM_ERR    result = TSCM_ERR_OK;

    if( pHTscam && pTrans_info && pTrans_info->pSteam_buf && pTrans_info->stream_buf_size )
    {
        // only one uart 1 but we have multi return_channel
        pthread_mutex_lock(&g_trans_mutex);

        write(ITP_DEVICE_UART1, pTrans_info->pSteam_buf, pTrans_info->stream_buf_size);

        pthread_mutex_unlock(&g_trans_mutex);
    }

    return result;
}

TSCM_ERR
simplex_rs232_control(
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
TSCM_DESC TSCM_DESC_simplex_rs232_desc =
{
    "ts cam simplex rs232",     // char                *name;
    0,                          // struct TSCM_DESC_T  *next;
    TSCM_TYPE_SIMPLEX_RS232,    // TSCM_TYPE           tscm_type;
    0,                          // void        *pExtraInfo;
    0, //simplex_rs232_init,         // TSCM_ERR    (*init)(struct TSCM_HANDLE_T *pHTscam, TSCM_INIT_INFO *pInit_info, void *extraData);
    0, //simplex_rs232_deinit,       // TSCM_ERR    (*deinit)(struct TSCM_HANDLE_T *pHTscam, void *extraData);
    0, //simplex_rs232_stream_recv,  // TSCM_ERR    (*stream_recv)(struct TSCM_HANDLE_T *pHTscam, TSCM_RECV_INFO *pRecv_info, void *extraData);
    simplex_rs232_stream_trans, // TSCM_ERR    (*stream_trans)(struct TSCM_HANDLE_T *pHTscam, TSCM_TRANS_INFO *pTrans_info, void *extraData);
    simplex_rs232_control,      // TSCM_ERR    (*control)(struct TSCM_HANDLE_T *pHTscam, TSCM_ARG *pArg, void *extraData);
};

#else
TSCM_DESC TSCM_DESC_simplex_rs232_desc =
{
    "ts cam simplex rs232",     // char                *name;
    0,                          // struct TSCM_DESC_T  *next;
    TSCM_TYPE_SIMPLEX_RS232,    // TSCM_TYPE           tscm_type;
    0,                          // void        *pExtraInfo;
};
#endif