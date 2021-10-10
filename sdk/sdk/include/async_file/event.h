/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The event functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_EVENT_H
#define PAL_EVENT_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Event name */
#define PAL_EVENT_JPEG_MGR_TO_THREAD           (0)
#define PAL_EVENT_JPEG_THREAD_TO_MGR           (1)
#define PAL_EVENT_AUDIO_MGR_TO_THREAD          (2)
#define PAL_EVENT_AUDIO_THREAD_TO_MGR          (3)

#define PAL_EVENT_ELEMENT_THREAD_TO_MPS_THREAD (4)
#define PAL_EVENT_COMMAND_COMPLETED            (5)

/** Wait infinite value */
#define PAL_EVENT_INFINITE                     (0xFFFFFFFFul)

MMP_EVENT
PalCreateEvent(
    MMP_INT name);

MMP_INT
PalDestroyEvent(
    MMP_EVENT event);

MMP_INT
PalSetEvent(
    MMP_EVENT event);

MMP_INT
PalWaitEvent(
    MMP_EVENT event,
    MMP_ULONG timeout);

#ifdef __cplusplus
}
#endif

#endif /* PAL_EVENT_H */