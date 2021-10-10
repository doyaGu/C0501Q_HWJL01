/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * The platform adaptation layer functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef PAL_PAL_H
#define PAL_PAL_H

#include "async_file/def.h"
#include "async_file/error.h"
#include "async_file/event.h"
#include "async_file/file.h"
#include "async_file/heap.h"
//#include "pal/keypad.h"
#include "async_file/msgq.h"
#include "async_file/mutex.h"
//#include "pal/print.h"
//#include "pal/rand.h"
//#include "pal/stat.h"
#include "async_file/string.h"
#include "async_file/thread.h"
#include "async_file/timer.h"
//#include "pal/sys.h"

#define SMTK_MAX(a,b)   (((a) > (b)) ? (a) : (b))
#define SMTK_MIN(a,b)   (((a) < (b)) ? (a) : (b))
#define SMTK_ABS(x)     (((x) >= 0)  ? (x) : -(x))

#define SMTK_COUNT_OF(array) (sizeof (array) / sizeof (array[0]))

#endif /* PAL_PAL_H */
