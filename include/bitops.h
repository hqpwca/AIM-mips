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

#ifndef _BITOPS_H
#define _BITOPS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 * If an architecture have instruction-level implementation of
 * ffs(), fls(), ffz() or flz(), define them there.
 */
#include <arch-bitops.h>

/*
 * Find first set: find the least significant set bit and return index
 * (1-based), or 0 if none.
 * See ffs(3) for definition.
 *
 * An equivalent operation is count trailing zeros (CTZ).
 */
#ifndef ffs
#define ffs(x)	__generic_ffs(x)
#endif

/*
 * Find last (most significant) set (1-based).
 * Related to counting leading zeros (CLZ)
 *
 * IMPORTANT NOTE:
 * The result is 0 if x = 0.
 */
#ifndef fls
#define fls(x)	__generic_fls(x)
#endif

/*
 * Find first zero, or count trailing ones.
 */
#ifndef ffz
#define ffz(x)	ffs(~(x))
#endif

/*
 * Find last zero, related to counting leading ones.
 *
 * IMPORTANT NOTE:
 * The result is 0 if x = -1.
 */
#ifndef flz
#define flz(x)	fls(~(x))
#endif

/*
 * for 64-bit
 */
#ifndef ffs64
#define ffs64(x)	__generic_ffs64(x)	/* Calls ffs() */
#endif

#ifndef fls64
#define fls64(x)	__generic_fls64(x)	/* Calls fls() */
#endif

#ifndef ffz64
#define ffz64(x)	__generic_ffz64(x)
#endif

#ifndef flz64
#define flz64(x)	__generic_flz64(x)
#endif

#include <sys/types.h>
#include <util.h>

static inline int __generic_ffs(unsigned long word)
{
	int num = 1;
	if (!word)
		return 0;

#if BITS_PER_LONG == 64
	if ((word & 0xffffffff) == 0) {
		num += 32;
		word >>= 32;
	}
#endif
	if ((word & 0xffff) == 0) {
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0) {
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0) {
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0) {
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

static inline int __generic_fls(unsigned long word)
{
	int num = BITS_PER_LONG;
	if (!word)
		return 0;

#if BITS_PER_LONG == 64
	if (!(word & (~0ul << 32))) {
		num -= 32;
		word <<= 32;
	}
#endif
	if (!(word & (~0ul << (BITS_PER_LONG-16)))) {
		num -= 16;
		word <<= 16;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-8)))) {
		num -= 8;
		word <<= 8;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-4)))) {
		num -= 4;
		word <<= 4;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-2)))) {
		num -= 2;
		word <<= 2;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-1))))
		num -= 1;
	return num;
}

static inline int __generic_ffs64(uint64_t word)
{
#if BITS_PER_LONG == 32
	if (((uint32_t)word) == 0UL)
		return ffs((uint32_t)(word >> 32)) + 32;
#endif
	return ffs((unsigned long)word);
}

static inline int __generic_fls64(uint64_t word)
{
#if BITS_PER_LONG == 32
	if (((uint32_t)word) == 0UL)
		return fls((uint32_t)(word >> 32)) + 32;
#endif
	return fls((unsigned long)word);
}

static inline int __generic_ffz64(uint64_t word)
{
#if bits_per_long == 32
	if (((uint32_t)word) == 0ul)
		return ffz((uint32_t)(word >> 32)) + 32;
#endif
	return ffz((unsigned long)word);
}

static inline int __generic_flz64(uint64_t word)
{
#if BITS_PER_LONG == 32
	if (((uint32_t)word) == 0UL)
		return flz((uint32_t)(word >> 32)) + 32;
#endif
	return flz((unsigned long)word);
}

/*
 * Hamming weight: count set bits
 */
static inline int hweight32(uint32_t w)
{
#ifndef HAVE___BUILTIN_POPCOUNT
	uint32_t res = w - ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res + (res >> 4)) & 0x0F0F0F0F;
	res = res + (res >> 8);
	return (int)((res + (res >> 16)) & 0x000000FF);
#else	/* __builtin_popcount available */
	return __builtin_popcount(w);
#endif	/* !HAVE___BUILTIN_POPCOUNT */
}

static inline int hweight64(uint64_t w)
{
#ifndef HAVE___BUILTIN_POPCOUNTLL
#if BITS_PER_LONG == 32
	return hweight32((uint32_t)(w >> 32)) + hweight32((uint32_t)w);
#elif BITS_PER_LONG == 64
	uint64_t res = w - ((w >> 1) & 0x5555555555555555ul);
	res = (res & 0x3333333333333333ul) + ((res >> 2) & 0x3333333333333333ul);
	res = (res + (res >> 4)) & 0x0F0F0F0F0F0F0F0Ful;
	res = res + (res >> 8);
	res = res + (res >> 16);
	return (int)((res + (res >> 32)) & 0x00000000000000FFul);
#endif
#else	/* __builtin_popcountll available */
	return __builtin_popcountll(w);
#endif	/* !HAVE___BUILTIN_POPCOUNTLL */
}

#ifndef HAVE___BUILTIN_POPCOUNTL
#if BITS_PER_LONG == 32
#define hweight(x)	hweight32(x)
#elif BITS_PER_LONG == 64
#define hweight(x)	hweight64(x)
#endif
#else	/* __builtin_popcount available */
#define hweight(x)	__builtin_popcountl(x)
#endif	/* !HAVE___BUILTIN_POPCOUNT */

/* Aliases */
#define count_set_bits(x)	hweight(x)
#define count_set_bits32(x)	hweight32(x)
#define count_set_bits64(x)	hweight64(x)

#define count_zero_bits(x)	count_set_bits(~(x))
#define count_zero_bits32(x)	count_set_bits32(~(x))
#define count_zero_bits64(x)	count_set_bits64(~(x))

#endif
