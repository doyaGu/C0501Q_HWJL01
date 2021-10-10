/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The heap allocation functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_HEAP_H
#define PAL_HEAP_H

#include "async_file/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Heap name */
#define PAL_HEAP_DEFAULT    0 /**< Default heap name */

void*
PalHeapAlloc(
    MMP_INT name,
    MMP_SIZE_T size);

void
PalHeapFree(
    MMP_INT name,
    void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* PAL_MALLOC_H */

