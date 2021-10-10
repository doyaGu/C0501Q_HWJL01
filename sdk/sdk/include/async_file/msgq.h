/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The message queue functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_MSGQ_H
#define PAL_MSGQ_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** MsgQ definition */
typedef void *PAL_MSGQ;

/** Wait infinite value */
#define PAL_MSGQ_INFINITE (0xFFFFFFFFul)

/** MsgQ name */
#define PAL_MSGQ_FILE     0

/** Error codes */
#define PAL_MSGQ_EMPTY    1

PAL_MSGQ
PalCreateMsgQ(
    MMP_INT   name,
    MMP_ULONG msgSize,
    MMP_ULONG msgCount);

MMP_INT
PalDestroyMsgQ(
    PAL_MSGQ queue);

MMP_INT
PalReadMsgQ(
    PAL_MSGQ  queue,
    void      *msg,
    MMP_ULONG timeout);

MMP_INT
PalWriteMsgQ(
    PAL_MSGQ  queue,
    void      *msg,
    MMP_ULONG timeout);

#ifdef __cplusplus
}
#endif

#endif /* PAL_MSGQ_H */