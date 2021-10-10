/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file xcpu_msgq.h
 * Implementation of cross cpu message queue.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef XCPU_MSGQ_H
#define XCPU_MSGQ_H

//#include "core_config.h"
#include "sys_msgq.h"

#ifdef __cplusplus
extern "C" {
#endif

//#if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef QUEUE_MGR_ERROR_CODE XCPU_MSGQ_ERROR_CODE;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef SYS_MSG_OBJ XCPU_MSG_OBJ;

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * This method implements a handshake mechanism with the host CPU to inform the
 * host CPU that Castor has successfully booted up and is ready to receive or
 * send cross CPU message.
 *
 * @return  none
 */
//=============================================================================
void
xCpuMsgQ_Init(
    void);

//=============================================================================
/**
 * Send a message to the host CPU.
 *
 * @param ptStoMMsg the pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information to be sent
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_SendMessage(
    XCPU_MSG_OBJ* ptStoMMsg);

//=============================================================================
/**
 * Receive a message from the host CPU.
 *
 * @param ptMtoSMsg Pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information received from the host CPU.
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_ReceiveMessage(
    XCPU_MSG_OBJ* ptMtoSMsg);

//=============================================================================
/**
 * Periodically call this method to receive messages from the host CPU and to
 * dispatch the message into the corresponding system message queues according
 * to the types of messages.
 *
 * @return none
 */
//=============================================================================
void
xCpuMsgQ_RouteMessage(
    void);

//=============================================================================
/**
 * Release the related resources.
 *
 * @return  none
 */
//=============================================================================
void
xCpuMsgQ_Terminate(
    void);

MMP_UINT16
xCpuMsgQ_GetStandaloneBootFlag(
    void);

void
xCpuMsgQ_InitStandaloneFlag(
    void);

//#endif  // end of #if (!defined(WIN32)) && defined(ENABLE_XCPU_MSGQ)

#ifdef __cplusplus
}
#endif

#endif
