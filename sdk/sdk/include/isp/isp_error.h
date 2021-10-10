#ifndef __ISP_ERROR_H_9QF78O71_NM87_4UCK_V470_VNEL6GRKEC5L__
#define __ISP_ERROR_H_9QF78O71_NM87_4UCK_V470_VNEL6GRKEC5L__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
/**
 *  ISP error code
 */
#define  ISP_ERR_BASE 0x50000000

typedef enum _ISP_ERR_TAG
{
    ISP_SUCCESS                     = 0,
    ISP_ERR_NOT_IDLE                = (ISP_ERR_BASE + 0x0002),
    ISP_ERR_INPUT_SIZE_ERROR        = (ISP_ERR_BASE + 0x0006),
    ISP_ERR_PORT1_SIZE_ERROR        = (ISP_ERR_BASE + 0x0007),
    ISP_ERR_CONTEXT_ALLOC_FAIL      = (ISP_ERR_BASE + 0x0009),
    ISP_ERR_NOT_INITIALIZE          = (ISP_ERR_BASE + 0x000A),
    ISP_ERR_NO_MATCH_ENABLE_TYPE    = (ISP_ERR_BASE + 0x000E),
    ISP_ERR_NO_MATCH_DISABLE_TYPE   = (ISP_ERR_BASE + 0x000F),
    ISP_ERR_COLOR_CTRL_OUT_OF_RANGE = (ISP_ERR_BASE + 0x0012),
    ISP_ERR_UNSUPPORTED_COLOR_CTRL  = (ISP_ERR_BASE + 0x0013),
    ISP_ERR_INVALID_GAIN_VALUE      = (ISP_ERR_BASE + 0x0014),
    ISP_ERR_INVALID_DISPLAY_WINDOW  = (ISP_ERR_BASE + 0x0016),
    ISP_ERR_INVALID_INPUT_PARAM     = (ISP_ERR_BASE + 0x0017),
    ISP_ERR_NO_MATCH_OUTPUT_FORMAT  = (ISP_ERR_BASE + 0x0018),
    ISP_ERR_NO_MATCH_INPUT_FORMAT   = (ISP_ERR_BASE + 0x0019),
    ISP_ERR_QUEUE_FIRE_NOT_IDLE     = (ISP_ERR_BASE + 0x0020),
    ISP_ERR_INVALID_PARAM           = (ISP_ERR_BASE + 0x0021),
    ISP_ERR_NULL_POINTER            = (ISP_ERR_BASE + 0x0221),
} ISP_RESULT;

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