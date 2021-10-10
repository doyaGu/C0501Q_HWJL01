﻿/*
 *  tslib/src/ts_close.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
 *
 *
 * Close a touchscreen device.
 */
#include "config.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "tslib-private.h"

int ts_close(struct tsdev *ts)
{
	int ret;

	ret = close(ts->fd);
	free(ts);

	return ret;
}
