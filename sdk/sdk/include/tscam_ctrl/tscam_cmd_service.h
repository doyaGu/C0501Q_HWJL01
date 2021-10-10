#ifndef __tscam_cmd_service_H_1RvHR49d_NfyK_sokl_hEGM_m6LobAURMVda__
#define __tscam_cmd_service_H_1RvHR49d_NfyK_sokl_hEGM_m6LobAURMVda__

#ifdef __cplusplus
extern "C" {
#endif


#include "cmd_pkt_ccHDtv.h"
#include "cmd_pkt_dev_IO.h"
#include "cmd_pkt_dev_mgt.h"
#include "cmd_pkt_image.h"
#include "cmd_pkt_media.h"
#include "cmd_pkt_metadata.h"
#include "cmd_pkt_PTZ.h"
#include "cmd_pkt_video_analytics.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * base memory need (MAX_CMD_NUM_PER_SERVICE*sizeof(TSCM_CMD_SRVC_ATTR)*15)
 * if MAX_CMD_NUM_PER_SERVICE == 256  => memory 60 KB
 * if MAX_CMD_NUM_PER_SERVICE == 0x96 => memory 36 KB
 **/
#define MAX_CMD_NUM_PER_SERVICE             0x96//256

#define CMD_SERVICE_TYPE_MASK               0xFF00
#define CMD_SERVICE_CMD_CODE_MASK           0x00FF

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

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
