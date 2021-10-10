#ifndef __ts_packet_analysis_def_H_bs3KxQTy_Xvpv_1Pzl_KrOT_R7YiRROjpJ0T__
#define __ts_packet_analysis_def_H_bs3KxQTy_Xvpv_1Pzl_KrOT_R7YiRROjpJ0T__

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_extractor_defs.h"  // it may be to be deleted for module indepandence


#define TSPA_CHECK_MEM_LEAK_ENABLE       0

#if (_MSC_VER && TSPA_CHECK_MEM_LEAK_ENABLE)
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if (_MSC_VER && TSPA_CHECK_MEM_LEAK_ENABLE)
    #include <crtdbg.h>
#endif
//=============================================================================
//                Constant Definition
//=============================================================================
#define TSPA_MAX_PID_NUM                    (8192)

#define INVALID_VERSION_NUMBER              (0xFFFFFFFF)
#define INVALID_TRANSPORT_STREAM_ID         (0xFFFFFFFF)

#define VALID_SYNC_BYTE                     (0x47)
#define CONTINUITY_COUNTER_MASK             (0xF)
#define TS_PACKET_BYTE_NUMBER               (188)

#define PAT_PID         (0x00)
#define NIT_PID         (0x10)
#define SDT_PID         (0x11)
#define EIT_PID         (0x12)
#define TDT_TOT_PID     (0x14)

typedef enum ADAPTATION_FIELD_CONTROL_T
{
    PAYLOAD_EXIST           = 0x01,
    ADAPTATION_FIELD_EXIST  = 0x02
} ADAPTATION_FIELD_CONTROL;

//=============================================================================
//                Macro Definition
//=============================================================================
#if !defined(TSPA_LOCAL_MACRO_DISABLE)

#define ENABLE_PTHREAD_MUTEX    1
#define ENABLE_DBG_TRACE        0

#ifndef MEMBER_OFFSET
    #define MEMBER_OFFSET(type, member)     (uint32_t)&(((type *)0)->member)
#endif

#ifndef DOWN_CAST
    #define DOWN_CAST(type, ptr, member)    (type*)((uint32_t)ptr - MEMBER_OFFSET(type, member))
#endif

#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

/**
 *  Debug flag
 */
extern uint32_t  tspaMsgOnFlag;

typedef enum _TSPA_MSG_TYPE
{
    TSPA_MSG_ERR             = (0x1 << 0),

    TSPA_MSG_TRACE_TSAP      = (0X1 << 9),

}TSPA_MSG_TYPE;

/**
 * debug message fucntion
 **/
#ifdef _MSC_VER // WIN32
    #ifndef trace
    #define trace(string, ...)              do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define tspa_msg(type, string, ...)      ((void)((type & tspaMsgOnFlag) ? printf(string, __VA_ARGS__) : 0))
    #define tspa_msg_ex(type, string, ...)   do{ if(type & tspaMsgOnFlag){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)
    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, ...)    do{ if(type & tspaMsgOnFlag){ \
                                                       printf("enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                       printf(string, __VA_ARGS__);}\
                                                   }while(0)
        #define _trace_leave(type)                 tspa_msg(type, "leave: %s()[#%d]\n", __FUNCTION__, __LINE__);
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, ...)
        #define _trace_leave(type)
    #endif

    #define _tspa_thread_pid()                   -1
#else /* _MSC_VER */
    #ifndef trace
    #define trace(string, args...)              do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define tspa_msg(type, string, args...)      ((void)((type & tspaMsgOnFlag) ? printf(string, ## args) : 0))
    #define tspa_msg_ex(type, string, args...)   do{ if(type & tspaMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)

    #if ENABLE_DBG_TRACE
        #define _trace_enter(type, string, args...)    do{ if(type & tspaMsgOnFlag){ \
                                                           printf("  enter: %s()[#%d], ", __FUNCTION__, __LINE__);\
                                                           printf(string, ## args);}\
                                                       }while(0)
        #define _trace_leave(type)                     tspa_msg(type, "  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #else /* ENABLE_DBG_TRACE */
        #define _trace_enter(string, args...)
        #define _trace_leave(type)
    #endif

    #define _tspa_thread_pid()                   getpid()
#endif

#define tspa_enable_msg(type)                (tspaMsgOnFlag |= type)
#define tspa_disable_msg(type)               (tspaMsgOnFlag &= ~(type))



/**
 * Handle malloc memory. Compiler allow size = 0, but our sysetm can't.
 **/
#ifndef tspa_malloc
    #define tspa_malloc(size)                    ((size) ? malloc(size) : NULL)
#endif

#endif /* !defined(TSPA_LOCAL_MACRO_DISABLE) */
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
