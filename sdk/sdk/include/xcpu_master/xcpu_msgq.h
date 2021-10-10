/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file xcpu_msgq.h
 * Implementation of cross cpu message queue.
 *
 * @version 0.1
 */

#ifndef XCPU_MSGQ_H
#define XCPU_MSGQ_H

#include "sys_msgq.h"
#include "xcpu_io.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

typedef QUEUE_ERROR_CODE XCPU_MSGQ_ERROR_CODE;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef SYS_MSG_OBJ XCPU_MSG_OBJ;

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * This method implement a handshake mechanism with Castor to make sure Castor
 * has successfully booted up and is ready to receive or send cross CPU message.
 * This method will be blocked until Castor announces it is ready. Only call
 * this function once Castor boots.
 *
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
xCpuMsgQ_Init(
    void);

//=============================================================================
/**
 * Send a message to the Castor.
 *
 * @param ptMtoSMsg the pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information to be sent
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_SendMessage(
    XCPU_MSG_OBJ* ptMtoSMsg);

//=============================================================================
/**
 * Receive a message from Castor.
 *
 * @param ptStoMMsg Pointer to an XCPU_MSG_OBJ structure that contains the
 *                  message information received from Castor.
 * @return  0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
XCPU_MSGQ_ERROR_CODE
xCpuMsgQ_ReceiveMessage(
    XCPU_MSG_OBJ* ptStoMMsg);

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

#ifdef __cplusplus
}
#endif

#endif
