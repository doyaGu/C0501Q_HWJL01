#ifndef __TS_DEBUG_H_K7JF1H8W_6V76_PWWJ_HIOM_KTKKIM5LMLAI__
#define __TS_DEBUG_H_K7JF1H8W_6V76_PWWJ_HIOM_KTKKIM5LMLAI__

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_dbg_time.h"
#include "ts_dbg_txt_conv.h"
#include "ts_dbg_stack_trace.h"

//////////////////////////////////////////////
#ifdef DEBUG
    #ifdef _MSC_VER // WIN32
        #define ts_dbg_trace(string, ...)   do{ printf(string, __VA_ARGS__); \
                                              printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                          }while(0)  
    #else
        #define ts_dbg_trace(string, args...)   do{printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                 }while(0)    
    #endif
#else
    #ifdef _MSC_VER // WIN32
        #define ts_dbg_trace(string, ...)
    #else
        #define ts_dbg_trace(string, args...)
    #endif 
#endif




#ifdef __cplusplus
}
#endif

#endif
