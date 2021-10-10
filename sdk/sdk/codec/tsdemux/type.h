/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Common type definitions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef MMP_TYPE_H
#define MMP_TYPE_H

#include "def.h"

/** Result definitions */
typedef MMP_LONG MMP_RESULT;
#define MMP_RESULT_SHIFT        16

/** Result value definition */
#define MMP_SUCCESS             0

/** Logger standard zones definition */
#define MMP_ZONE_ERROR      (1ul << 0)
#define MMP_ZONE_WARNING    (1ul << 1)
#define MMP_ZONE_INFO       (1ul << 2)
#define MMP_ZONE_DEBUG      (1ul << 3)
#define MMP_ZONE_ENTER      (1ul << 4)
#define MMP_ZONE_LEAVE      (1ul << 5)
#define MMP_ZONE_ALL        (~0ul)

#endif /* MMP_TYPE_H */
