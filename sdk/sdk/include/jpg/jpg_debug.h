#ifndef __jpg_debug_H_q6eBMd8a_Y5Z7_jdHl_JLmM_GNf8YU4EGXGT__
#define __jpg_debug_H_q6eBMd8a_Y5Z7_jdHl_JLmM_GNf8YU4EGXGT__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Constant Definition
//=============================================================================
#define JPG_CHECK_CALLER    0

#if (JPG_CHECK_CALLER)
    #define JPG_EXTRA_INFO      ,const char *_caller, const int _line
    #define jpg_extra_param     ,__FUNCTION__,__LINE__
#else
    #define JPG_EXTRA_INFO
    #define jpg_extra_param
#endif
//=============================================================================
//                Macro Definition
//=============================================================================
/**
 * trace caller message fucntion
 **/
#if (_MSC_VER)
    #if (JPG_CHECK_CALLER)
        #define _jpg_trace_caller(string, ...)      do{ printf(string, __VA_ARGS__); \
                                                    }while(0)
    #else
        #define _jpg_trace_caller(string, ...)
    #endif
#else
    // (__GNUC__)
    #if (JPG_CHECK_CALLER)
        #define _jpg_trace_caller(string, args...)  do{ printf(string, ## args); \
                                                    }while(0)
    #else
        #define _jpg_trace_caller(string, args...)
    #endif
#endif

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
