/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Platform definitions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_DEF_H
#define PAL_DEF_H

#include "ite/mmp_types.h"

/** Unicode definition */
//#define PAL_UNICODE

/** String definition */
#ifdef PAL_UNICODE
    #define PAL_T(x) L ## x /**< Unicode string type */
#else
    #define PAL_T(x) x      /**< ANSI string type */
#endif // PAL_UNICODE

#endif /* PAL_DEF_H */