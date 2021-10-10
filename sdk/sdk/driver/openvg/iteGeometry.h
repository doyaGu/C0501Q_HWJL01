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

#ifndef __ITEGEOMETRY_H

#include "iteDefs.h"
#include "iteContext.h"
#include "iteVectors.h"
#include "itePath.h"

//void iteFlattenPath(ITEPath *p, ITEint surfaceSpace);
void iteFlattenPath(ITEPath* p, ITEint  surfaceSpace, ITEPathCommandArray* pPathCmdArray);
void iteGenStrokePath(VGContext* c, ITEPath *p, ITEuint* cmdData);
void iteGenFillPath(VGContext* c, ITEPath *p, ITEuint* cmdData);
void iteGenRectanglePath(ITEVector2 min, ITEVector2 max, ITEuint* cmdData);

#endif /* __ITEGEOMETRY_H */
