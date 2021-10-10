#ifndef __VP_ERROR_H_9QF78O71_NM87_4UCK_V470_VNEL6GRKEC5L__
#define __VP_ERROR_H_9QF78O71_NM87_4UCK_V470_VNEL6GRKEC5L__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                              Constant Definition
//=============================================================================
/**
 *  ISP error code
 */
#define  VP_ERR_BASE            0x50000000

typedef enum _VP_ERR_TAG
{
    VP_SUCCESS                                   = 0,
    VP_ERR_NOT_IDLE                              = (VP_ERR_BASE + 0x0002),
    VP_ERR_INPUT_SIZE_ERROR                      = (VP_ERR_BASE + 0x0006),
    VP_ERR_OUTPUT_SIZE_ERROR                     = (VP_ERR_BASE + 0x0007),
    VP_ERR_CONTEXT_ALLOC_FAIL                    = (VP_ERR_BASE + 0x0009),
    VP_ERR_NOT_INITIALIZE                        = (VP_ERR_BASE + 0x000A),
    VP_ERR_NO_MATCH_ENABLE_TYPE                  = (VP_ERR_BASE + 0x000E),
    VP_ERR_NO_MATCH_DISABLE_TYPE                 = (VP_ERR_BASE + 0x000F),
    VP_ERR_COLOR_CTRL_OUT_OF_RANGE               = (VP_ERR_BASE + 0x0012),
    VP_ERR_UNSUPPORTED_COLOR_CTRL                = (VP_ERR_BASE + 0x0013),
    VP_ERR_INVALID_GAIN_VALUE                    = (VP_ERR_BASE + 0x0014),
    VP_ERR_INVALID_DISPLAY_WINDOW                = (VP_ERR_BASE + 0x0016),
    VP_ERR_INVALID_INPUT_PARAM                   = (VP_ERR_BASE + 0x0017),
    VP_ERR_NO_MATCH_OUTPUT_FORMAT                = (VP_ERR_BASE + 0x0018),
    VP_ERR_NO_MATCH_INPUT_FORMAT                 = (VP_ERR_BASE + 0x0019),
    VP_ERR_QUEUE_FIRE_NOT_IDLE                   = (VP_ERR_BASE + 0x0020),
    VP_ERR_INVALID_PARAM                         = (VP_ERR_BASE + 0x0021),
    VP_ERR_NULL_POINTER                          = (VP_ERR_BASE + 0x0221),
}VP_RESULT;


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
