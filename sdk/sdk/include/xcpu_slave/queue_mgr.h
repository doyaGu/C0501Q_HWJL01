/*
 * Copyright (c) 2007 ITE technology Corp. All Rights Reserved.
 */
/** @file stream_mgr.h
 * A simple message queue and queue buffer implementation.
 *
 * @author Steven Hsiao
 * @version 0.01
 */
#ifndef QUEUE_MGR_H
#define QUEUE_MGR_H


#ifdef __cplusplus
extern "C" {
#endif

typedef enum QUEUE_MGR_ERROR_CODE_TAG
{
    QUEUE_MGR_NOT_INIT  = -6,
    QUEUE_INVALID_INPUT = -5,
    QUEUE_IS_EMPTY      = -4,
    QUEUE_IS_FULL       = -3,
    QUEUE_EXIST         = -2,
    QUEUE_NOT_EXIST     = -1,
    QUEUE_NO_ERROR      = 0
} QUEUE_MGR_ERROR_CODE;

#ifdef __cplusplus
}
#endif

#endif
