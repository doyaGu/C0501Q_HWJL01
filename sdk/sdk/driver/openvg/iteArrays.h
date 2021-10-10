/*
 * Copyright (c) 2007 Ivan Leben
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file COPYING;
 * if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*--------------------------------------------
 * Declarations of all the arrays used
 *--------------------------------------------*/

#ifndef __ITEARRAYS_H
#define __ITEARRAYS_H

#include "iteVectors.h"


#define _ITEM_T  ITEint
#define _ARRAY_T ITEIntArray
#define _FUNC_T  iteIntArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#define _ITEM_T  ITEuint8
#define _ARRAY_T ITEUint8Array
#define _FUNC_T  iteUint8Array
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#define _ITEM_T  ITEfloat
#define _ARRAY_T ITEFloatArray
#define _FUNC_T  iteFloatArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#define _ITEM_T  ITERectangle
#define _ARRAY_T ITERectArray
#define _FUNC_T  iteRectArray
#define _ARRAY_DECLARE
#include "iteArrayBase.h"

#endif
