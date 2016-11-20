/* find_last_bit.c: fallback find next bit implementation
 *
 * Copyright (C) 2008 IBM Corporation
 * Written by Rusty Russell <rusty@rustcorp.com.au>
 * (Inspired by David Howell's find_next_bit implementation)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <aim/debug.h>
#include <bitmap.h>
#include <bitops.h>
#include <util.h>

__weak unsigned long bitmap_find_last_bit(const unsigned long *addr,
		unsigned long size)
{
	unsigned long words;
	unsigned long tmp;

	/* Start at final word. */
	words = size / BITS_PER_LONG;

	/* Partial final word? */
	if (size & (BITS_PER_LONG-1)) {
		tmp = (addr[words] & (~0UL >> (BITS_PER_LONG
					 - (size & (BITS_PER_LONG-1)))));
		if (tmp)
			goto found;
	}

	while (words) {
		tmp = addr[--words];
		if (tmp) {
found:
			return words * BITS_PER_LONG + fls(tmp);
		}
	}

	/* Not found */
	return size;
}

