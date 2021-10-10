/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: bit.c,v 1.12 2004/01/23 09:41:32 rob Exp $
 */

# include "mp3_config.h"

# undef CHAR_BIT
# define CHAR_BIT  8

# include "bit.h"

/*
 * NAME:    bit->init()
 * DESCRIPTION: initialize bit pointer struct
 */
void mad_bit_init(struct mad_bitptr *bitptr, unsigned char const *byte)
{
  bitptr->byte  = byte;
  bitptr->cache = 0;
  bitptr->left  = CHAR_BIT;
}

/*
 * NAME:    bit->length()
 * DESCRIPTION: return number of bits between start and end points
 */
unsigned int mad_bit_length(struct mad_bitptr const *begin,
                struct mad_bitptr const *end)
{
  return begin->left +
    CHAR_BIT * (end->byte - (begin->byte + 1)) + (CHAR_BIT - end->left);
}

/*
 * NAME:    bit->read()
 * DESCRIPTION: read an arbitrary number of bits and return their UIMSBF value
 */
unsigned long mad_bit_read(struct mad_bitptr *bitptr, unsigned int len)
{
  register unsigned long value;
  register unsigned short cache = bitptr->cache;
  register unsigned short left = bitptr->left;

  if (left == CHAR_BIT)
    cache = *bitptr->byte;

  if (len < left) {
    value = (cache & ((1 << left) - 1)) >> (left - len);      
    bitptr->left -= len;
	bitptr->cache = cache;
    return value;
  }

  /* remaining bits in current byte */

  value = cache & ((1 << left) - 1);
  len  -= left;

  bitptr->byte++;
  left = CHAR_BIT;

  /* more bytes */

  while (len >= CHAR_BIT) {
    value = (value << CHAR_BIT) | *bitptr->byte++;
    len  -= CHAR_BIT;
  }

  if (len > 0) {
    cache = *bitptr->byte;

    value = (value << len) | (cache >> (CHAR_BIT - len));
    left -= len;
  }

  bitptr->left = left;
  bitptr->cache = cache;

  return value;
}
