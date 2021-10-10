#ifndef __DBG_STACK_TRACE_H_7W9CHQJM_4DTE_OB5Y_H70K_BZY2LP65HL9E__
#define __DBG_STACK_TRACE_H_7W9CHQJM_4DTE_OB5Y_H70K_BZY2LP65HL9E__

#ifdef __cplusplus
extern "C" {
#endif

#include "ite/itp.h"
#include <execinfo.h>
//=============================================================================
//				  Constant Definition
//=============================================================================

#define TS_BACKTRACE_STACK_DEPTH    20
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
static void show_stack_info(void)
{
#if 0
    static void *dbg_btBuf[TS_BACKTRACE_STACK_DEPTH] = {0};
    static int  dbg_btCount = 0;
    
    dbg_btCount = backtrace(dbg_btBuf, TS_BACKTRACE_STACK_DEPTH);
    
    while( dbg_btCount > 0 )
        ithPrintf("0x%X ", dbg_btBuf[dbg_btCount--]);  
    ithPrintf("\n");
#else
    // not working
    return ;
#endif
}

//=============================================================================
//				  Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif
