/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file xcpu.h
 * Used to receive and execute the host CPU commands.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef XCPU_H
#define XCPU_H

//#include "core_config.h"
#include "sys_msgq.h"

#ifdef __cplusplus
extern "C" {
#endif

//#if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 *
 *
 * @return  0 if they succeed, and a failure code otherwise.
 */
//=============================================================================
MMP_RESULT
xCpu_Init(
    void);

//=============================================================================
/**
* Wait for load configuration files(config.cfg, service.cfg, custom.cfg) from Host complete.
*
* @return  0 if they succeed, and a failure code otherwise.
*/
//=============================================================================
MMP_RESULT
XCpu_WaitHostFileReady(
    void);

//=============================================================================
/**
 * 
 *
 * @return  0 if they succeed, and a failure code otherwise.
 */
//=============================================================================
MMP_RESULT
xCpu_Terminate(
    void);

void
xCpu_ProcessCmdMsg(
    void);

//#endif  // end of #if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

#ifdef __cplusplus
}
#endif

#endif  // XCPU_H
