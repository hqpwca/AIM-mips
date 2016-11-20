/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
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

#ifndef _BITMAP_H
#define _BITMAP_H

#include <libc/string.h>
#include <util.h>

/*
 * Declare a bitmap with n bits.
 * You can do declarations like
 *
 *     static DECLARE_BITMAP(a, 40);
 *
 * although it's pretty unreadable.
 */

#define DECLARE_BITMAP(var, nbit) \
	unsigned long var[BITS_TO_LONGS(nbit)]

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) % BITS_PER_LONG))
#define BITMAP_LAST_WORD_MASK(nbits)					\
(									\
	((nbits) % BITS_PER_LONG) ?					\
		(1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL		\
)

/*
 * NOTE: always use the interfaces provided HERE (in this comment).
 *
 * The available bitmap operations and their rough meaning in the
 * case that the bitmap is a single unsigned long are thus:
 *
 * Note that nbits should be always a compile time evaluable constant.
 * Otherwise many inlines will generate horrible code.
 *
 * Another note: the following operations are NOT necessarily atomic.
 *
 * bitmap_zero(dst, nbits)			*dst = 0UL
 * bitmap_fill(dst, nbits)			*dst = ~0UL
 * bitmap_copy(dst, src, nbits)			*dst = *src
 * bitmap_and(dst, src1, src2, nbits)		*dst = *src1 & *src2
 * bitmap_or(dst, src1, src2, nbits)		*dst = *src1 | *src2
 * bitmap_xor(dst, src1, src2, nbits)		*dst = *src1 ^ *src2
 * bitmap_andnot(dst, src1, src2, nbits)	*dst = *src1 & ~(*src2)
 * bitmap_complement(dst, src, nbits)		*dst = ~(*src)
 * bitmap_equal(src1, src2, nbits)		Are *src1 and *src2 equal?
 * bitmap_intersects(src1, src2, nbits) 	Do *src1 and *src2 overlap?
 * bitmap_subset(src1, src2, nbits)		Is *src1 a subset of *src2?
 * bitmap_empty(src, nbits)			Are all bits zero in *src?
 * bitmap_full(src, nbits)			Are all bits set in *src?
 * bitmap_weight(src, nbits)			Hamming Weight: number set bits
 * bitmap_set(dst, pos, nbits)			Set specified bit area
 * bitmap_clear(dst, pos, nbits)		Clear specified bit area
 * bitmap_find_first_zero_bit(addr, nbits)	Position first zero bit in *addr
 * bitmap_find_first_bit(addr, nbits)		Position first set bit in *addr
 * bitmap_find_next_zero_bit(addr, nbits, bit)	Position next zero bit in *addr >= bit
 * bitmap_find_next_bit(addr, nbits, bit)	Position next set bit in *addr >= bit
 * bitmap_test_bit(i, dst)			Whether i-th bit (1-based) is set
 *
 * The following from atomic.h also applies for bitmaps:
 * atomic_set_bit(i, dst)			Set i-th (1-based) bit
 * atomic_clear_bit(i, dst)			Clear i-th (1-based) bit
 */

extern int __bitmap_empty(const unsigned long *bitmap, int bits);
extern int __bitmap_full(const unsigned long *bitmap, int bits);
extern int __bitmap_equal(const unsigned long *bitmap1,
                	const unsigned long *bitmap2, int bits);
extern void __bitmap_complement(unsigned long *dst, const unsigned long *src,
			int bits);
extern void __bitmap_shift_right(unsigned long *dst,
                        const unsigned long *src, int shift, int bits);
extern void __bitmap_shift_left(unsigned long *dst,
                        const unsigned long *src, int shift, int bits);
extern int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern int __bitmap_intersects(const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern int __bitmap_subset(const unsigned long *bitmap1,
			const unsigned long *bitmap2, int bits);
extern int __bitmap_weight(const unsigned long *bitmap, int bits);

extern void bitmap_set(unsigned long *map, int i, int len);
extern void bitmap_clear(unsigned long *map, int start, int nr);

extern unsigned long bitmap_find_next_bit(const unsigned long *addr,
		unsigned long size, unsigned long offset);
extern unsigned long bitmap_find_next_zero_bit(const unsigned long *addr,
		unsigned long size, unsigned long offset);
extern unsigned long bitmap_find_first_bit(const unsigned long *addr,
		unsigned long size);
extern unsigned long bitmap_find_first_zero_bit(const unsigned long *addr,
		unsigned long size);
extern unsigned long bitmap_find_last_bit(const unsigned long *addr,
		unsigned long size);

static inline void bitmap_zero(unsigned long *dst, int nbits)
{
	int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
	memset(dst, 0, len);
}

static inline void bitmap_fill(unsigned long *dst, int nbits)
{
	size_t nlongs = BITS_TO_LONGS(nbits);
	int len = (nlongs - 1) * sizeof(unsigned long);
	memset(dst, 0xff,  len);
	dst[nlongs - 1] = BITMAP_LAST_WORD_MASK(nbits);
}

static inline int bitmap_and(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	return __bitmap_and(dst, src1, src2, nbits);
}

static inline void bitmap_or(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	__bitmap_or(dst, src1, src2, nbits);
}

static inline void bitmap_xor(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	__bitmap_xor(dst, src1, src2, nbits);
}

static inline int bitmap_andnot(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	return __bitmap_andnot(dst, src1, src2, nbits);
}

static inline void bitmap_complement(unsigned long *dst, const unsigned long *src,
		int nbits)
{
	__bitmap_complement(dst, src, nbits);
}

static inline int bitmap_equal(const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	return __bitmap_equal(src1, src2, nbits);
}

static inline int bitmap_intersects(const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	return __bitmap_intersects(src1, src2, nbits);
}

static inline int bitmap_subset(const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	return __bitmap_subset(src1, src2, nbits);
}

static inline int bitmap_empty(const unsigned long *src, int nbits)
{
	return __bitmap_empty(src, nbits);
}

static inline int bitmap_full(const unsigned long *src, int nbits)
{
	return __bitmap_full(src, nbits);
}

static inline int bitmap_weight(const unsigned long *src, int nbits)
{
	return __bitmap_weight(src, nbits);
}

static inline void bitmap_shift_right(unsigned long *dst,
		const unsigned long *src, int n, int nbits)
{
	__bitmap_shift_right(dst, src, n, nbits);
}

static inline void bitmap_shift_left(unsigned long *dst,
		const unsigned long *src, int n, int nbits)
{
	__bitmap_shift_left(dst, src, n, nbits);
}

static inline int bitmap_test_bit(int nr, const volatile unsigned long *addr)
{
	assert(nr > 0);
	return 1UL & (addr[BIT_WORD(nr)] >> ((nr - 1) & (BITS_PER_LONG-1)));
}

#endif

