/* Copyright (C) xxxx Anonymous (from Linux source code)
 *
 * lib/bitmap.c
 * Helper functions for bitmap.h.
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 *
 * This file is part of AIM.
 *
 * AIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <aim/debug.h>
#include <bitmap.h>
#include <util.h>

/**
 * __bitmap_shift_right - logical right shift of the bits in a bitmap
 *   @dst : destination bitmap
 *   @src : source bitmap
 *   @shift : shift by this many bits
 *   @bits : bitmap size, in bits
 *
 * Shifting right (dividing) means moving bits in the MS -> LS bit
 * direction.  Zeros are fed into the vacated MS positions and the
 * LS bits shifted off the bottom are lost.
 */
__weak void __bitmap_shift_right(unsigned long *dst,
			const unsigned long *src, int shift, int bits)
{
	int k, lim = BITS_TO_LONGS(bits), left = bits % BITS_PER_LONG;
	int off = shift/BITS_PER_LONG, rem = shift % BITS_PER_LONG;
	unsigned long mask = (1UL << left) - 1;
	for (k = 0; off + k < lim; ++k) {
		unsigned long upper, lower;

		/*
		 * If shift is not word aligned, take lower rem bits of
		 * word above and make them the top rem bits of result.
		 */
		if (!rem || off + k + 1 >= lim)
			upper = 0;
		else {
			upper = src[off + k + 1];
			if (off + k + 1 == lim - 1 && left)
				upper &= mask;
		}
		lower = src[off + k];
		if (left && off + k == lim - 1)
			lower &= mask;
		dst[k] = upper << (BITS_PER_LONG - rem) | lower >> rem;
		if (left && k == lim - 1)
			dst[k] &= mask;
	}
	if (off)
		memset(&dst[lim - off], 0, off*sizeof(unsigned long));
}

