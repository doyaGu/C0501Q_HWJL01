/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The mutex functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_MUTEX_H
#define PAL_MUTEX_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Mutex name */
#define PAL_MUTEX_FILE                      (0)
#define PAL_MUTEX_MEDIA_STATE               (1)
#define PAL_MUTEX_MEDIAPLAY_PLAY_VIDEO      (2)
#define PAL_MUTEX_MEDIAPLAY_PARA_VIDEO      (3)
#define PAL_MUTEX_MEDIAPLAY_PARA_AUDIO      (4)
#define PAL_MUTEX_MEDIAPLAY_SET_STATE       (5)
#define PAL_MUTEX_MEDIAPLAY_PAUSE_VIDEO     (6)
#define PAL_MUTEX_MEDIAPLAY_PAUSE_AUDIO     (7)
#define PAL_MUTEX_MEDIAPLAY_PARA_SET        (8)
#define PAL_MUTEX_MEDIAPLAY_FILE_SYSTEM     (9)
#define PAL_MUTEX_MEDIAPLAY_CALLBACK        (10)
#define PAL_MUTEX_MEDIAPLAY_PAUSE_FILE      (11)
#define PAL_MUTEX_MEDIAPLAY_PLAY_AUDIO      (12)
#define PAL_MUTEX_MEDIAPLAY_PLAY_RAW        (13)
#define PAL_MUTEX_SLIDESHOW                 (14)
#define PAL_MUTEX_AUDIO_GENERAL             (15)
#define PAL_MUTEX_AUDIO_STRMOUT             (16)
#define PAL_MUTEX_AUDIO_STRMIN              (17)
#define PAL_MUTEX_JPEG                      (18)

/** Wait infinite value */
#define PAL_MUTEX_INFINITE  (0xFFFFFFFFul)

MMP_MUTEX
PalCreateMutex(
    MMP_INT name);

MMP_INT
PalDestroyMutex(
    MMP_MUTEX mutex);

MMP_INT
PalWaitMutex(
    MMP_MUTEX mutex,
    MMP_ULONG timeout);

MMP_INT
PalReleaseMutex(
    MMP_MUTEX mutex);

#ifdef __cplusplus
}
#endif

#endif /* PAL_MUTEX_H */

