#ifndef __test_item_isp_H_cyLsWLuy_DGhb_eBuM_U1FX_hdZnKYyWwitt__
#define __test_item_isp_H_cyLsWLuy_DGhb_eBuM_U1FX_hdZnKYyWwitt__

#ifdef __cplusplus
extern "C" {
#endif

#include "simple_draw.h"

#if (CFG_CHIP_FAMILY == 9920)
	#include "isp/mmp_isp_9920.h"
#else
	#include "isp/mmp_isp.h"
#endif


#include "it9070/isp_defs.h"
#include "it9070/isp_queue.h"

//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum DATA_COLOR_TYPE_TAG
{
    DATA_COLOR_YUV444,
    DATA_COLOR_YUV422,
    DATA_COLOR_YUV422R,
    DATA_COLOR_YUV420,
    DATA_COLOR_ARGB8888,
    DATA_COLOR_ARGB4444,
    DATA_COLOR_RGB565,
    DATA_COLOR_NV12,
    DATA_COLOR_NV21,

    DATA_COLOR_CNT,
}DATA_COLOR_TYPE;

typedef enum _ISP_ACT_CMD_TAG
{
    ISP_ACT_CMD_IDLE    = 0,
    ISP_ACT_CMD_INIT,
    ISP_ACT_CMD_TERMINATE,
    ISP_ACT_CMD_PROC,
    
}ISP_ACT_CMD;
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _CLIP_WND_INFO_TAG
{
    int         bClipEnable;
    int         bClipOutside;
    int         clipWndId;
    BASE_RECT   clipRect;

}CLIP_WND_INFO;
//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================

//=============================================================================
//				  Public Function Definition
//=============================================================================
void
test_isp_colorTrans(
    uint8_t             *srcAddr_rgby,
    uint8_t             *srcAddr_u,
    uint8_t             *srcAddr_v,
    DATA_COLOR_TYPE     colorType,
    CLIP_WND_INFO       *clipInfo,
    BASE_RECT           *srcRect);

#if 0 // difference isp version
void
test_isp_ui_frmFunc(
    ISP_ACT_CMD         cmd,
    unsigned char       *uiVram_00,
    unsigned char       *uiVram_01,
    unsigned char       *srcAddr_y,
    unsigned char       *srcAddr_u,
    unsigned char       *srcAddr_v,
    unsigned int        frm_w,
    unsigned int        frm_h,
    MMP_ISP_FRM_FUNC_INFO   *frmFunInfo);
#endif

#ifdef __cplusplus
}
#endif

#endif
