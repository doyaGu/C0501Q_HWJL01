/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: metadata.h 13769 2007-07-03 09:25:36Z dave $
 *
 * Copyright (C) 2005 Dave Chapman
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#ifndef __METADATA_COMMON_H__
#define __METADATA_COMMON_H__

#include "config.h"
#include "bswap.h"

static __inline int read_uint16le(uint16_t* buf)
{
    int n = read_filebuf((char*) buf, sizeof(char)*2);
    *buf = le2me_16(*buf);
    return n;
}

static __inline int read_uint32le(uint32_t* buf)
{
    int n = read_filebuf((char*) buf, sizeof(char)*4);
    *buf = le2me_32(*buf);
    return n;
}

static __inline int read_uint64le(uint64_t* buf)
{
    int n = read_filebuf(buf, sizeof(char)*8);
    *buf = le2me_64(*buf);
    return n;
}

/* Read an unaligned 32-bit little endian long from buffer. */
static __inline unsigned long get_long_le(void* buf)
{
    unsigned char* p = (unsigned char*) buf;

    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

/* Read an unaligned 16-bit little endian short from buffer. */
static __inline unsigned short get_short_le(void* buf)
{
    unsigned char* p = (unsigned char*) buf;

    return p[0] | (p[1] << 8);
}

/* Read an unaligned 32-bit big endian long from buffer. */
static __inline unsigned long get_long_be(void* buf)
{
    unsigned char* p = (unsigned char*) buf;

    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

#endif // __METADATA_COMMON_H__

