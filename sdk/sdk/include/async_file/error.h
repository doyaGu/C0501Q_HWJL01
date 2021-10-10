/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The error handling functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_ERROR_H
#define PAL_ERROR_H

#include "ite/mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MMP_DEBUG
    #define PalAssert(e) ((e) ? (void) 0 : PalAssertFail(#e, __FILE__, __LINE__))
#else
    #define PalAssert(e) ((void) 0)

#endif /* MMP_DEBUG */

void
PalAssertFail(
    const MMP_CHAR *exp,
    const MMP_CHAR *file,
    MMP_UINT       line);

MMP_CHAR *
PalGetErrorString(
    MMP_INT errnum);

void
PalExit(
    MMP_INT status);

#ifdef __cplusplus
}
#endif

#endif /* PAL_ERROR_H */