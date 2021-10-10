#ifndef __log_msg_H_6rHlduyb_AJnD_1cQe_zUz6_ok7R0q6kCJKz__
#define __log_msg_H_6rHlduyb_AJnD_1cQe_zUz6_ok7R0q6kCJKz__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
/**
 * debug message fucntion
 **/
#if (_MSC_VER) // WIN32
    #ifndef trace
    #define trace(string, ...)                      do{ printf(string, __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                    }while(0)
    #endif

    #define _log_msg(type, string, ...)             ((void)((type) ? printf(string, __VA_ARGS__) : 0))
    #define _log_msg_ex(type, string, ...)          do{ if(type){ \
                                                        printf(string, __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                    }while(0)

    #define _enter_msg(type, string, ...)           do{ _log_msg(type, " enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                        _log_msg(type, string, ...);\
                                                    }while(0)
    #define _leave_msg(type)                        _log_msg(type, " leave: %s()[#%d]\n", __FUNCTION__, __LINE__);

#else /* _MSC_VER */
    #ifndef trace
    #define trace(string, args...)                  do{ printf(string, ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                    }while(0)
    #endif

    #define _log_msg(type, string, args...)         ((void)((type) ? printf(string, ## args) : 0))
    #define _log_msg_ex(type, string, args...)      do{ if(type){ \
                                                        printf(string, ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                    }while(0)

    #define _enter_msg(type, string, args...)       do{ _log_msg(type, " enter: %s()[#%d]\n", __FUNCTION__, __LINE__);\
                                                       _log_msg(type, string, args...);\
                                                    }while(0)
    #define _leave_msg(type)                        _log_msg(type, "  leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
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
