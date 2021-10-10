#ifndef __TS_DEMUXER_ERR_H_Y1L1FE4Y_UVUF_YC9D_Y6M4_VPNP1W4U596I__
#define __TS_DEMUXER_ERR_H_Y1L1FE4Y_UVUF_YC9D_Y6M4_VPNP1W4U596I__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//				  Constant Definition
//=============================================================================
/**
 *  ts demuxer error code
 */
#define  TSD_ERR_BASE            0xDD000000
#define  TSP_ERR_BASE            0xDDA00000
#define  SRVC_ERR_BASE           0xDDB00000
#define  CHNL_ERR_BASE           0xDDC00000
#define  EPG_ERR_BASE            0xDDE00000
typedef enum _TSD_ERR_TAG
{
    // ts demuxet
    TSD_ERR_OK                  = 0,
    TSD_ERR_ALLOCATE_FAIL       = (TSD_ERR_BASE | 0x00000001),
    TSD_ERR_NULL_POINTER        = (TSD_ERR_BASE | 0x00000002),
    TSD_ERR_INVALID_PARAMETER   = (TSD_ERR_BASE | 0x00000003),
    TSD_ERR_TIME_OUT            = (TSD_ERR_BASE | 0x00000004),
    TSD_ERR_NO_IMPLEMENT        = (TSD_ERR_BASE | 0x00000005),
    TSD_ERR_DEMOD_FAIL          = (TSD_ERR_BASE | 0x00000006),

    // ts parser
    TSP_ERR_OK                  = 0,
    TSP_ERR_ALLOCATE_FAIL       = (TSP_ERR_BASE | 0x00000001),
    TSP_ERR_NULL_POINTER        = (TSP_ERR_BASE | 0x00000002),
    TSP_ERR_INVALID_PARAMETER   = (TSP_ERR_BASE | 0x00000003),
    TSP_ERR_TIME_OUT            = (TSP_ERR_BASE | 0x00000004),
    TSP_ERR_NO_IMPLEMENT        = (TSP_ERR_BASE | 0x00000005),

    // service info
    SRVC_ERR_OK                 = 0,
    SRVC_ERR_ALLOCATE_FAIL      = (SRVC_ERR_BASE | 0x00000001),
    SRVC_ERR_NULL_POINTER       = (SRVC_ERR_BASE | 0x00000002),
    SRVC_ERR_INVALID_PARAMETER  = (SRVC_ERR_BASE | 0x00000003),
    SRVC_ERR_TIME_OUT           = (SRVC_ERR_BASE | 0x00000004),
    SRVC_ERR_NO_IMPLEMENT       = (SRVC_ERR_BASE | 0x00000005),

    // channel info
    CHNL_ERR_OK                 = 0,
    CHNL_ERR_ALLOCATE_FAIL      = (CHNL_ERR_BASE | 0x00000001),
    CHNL_ERR_NULL_POINTER       = (CHNL_ERR_BASE | 0x00000002),
    CHNL_ERR_INVALID_PARAMETER  = (CHNL_ERR_BASE | 0x00000003),
    CHNL_ERR_TIME_OUT           = (CHNL_ERR_BASE | 0x00000004),
    CHNL_ERR_NO_IMPLEMENT       = (CHNL_ERR_BASE | 0x00000005),

    // epg info
    EPG_ERR_OK                  = 0,
    EPG_ERR_ALLOCATE_FAIL       = (EPG_ERR_BASE | 0x00000001),
    EPG_ERR_NULL_POINTER        = (EPG_ERR_BASE | 0x00000002),
    EPG_ERR_INVALID_PARAMETER   = (EPG_ERR_BASE | 0x00000003),
    EPG_ERR_TIME_OUT            = (EPG_ERR_BASE | 0x00000004),
    EPG_ERR_NO_IMPLEMENT        = (EPG_ERR_BASE | 0x00000005),

    TSD_ERR_UNKNOW              = (TSD_ERR_BASE | 0x0000FFFF),
    TSP_ERR_UNKNOW              = (TSP_ERR_BASE | 0x0000FFFF),
    SRVC_ERR_UNKNOW             = (SRVC_ERR_BASE | 0x0000FFFF),
    CHNL_ERR_UNKNOW             = (CHNL_ERR_BASE | 0x0000FFFF),
    EPG_ERR_UNKNOW              = (EPG_ERR_BASE | 0x0000FFFF),

}TSD_ERR;

//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================


//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif
