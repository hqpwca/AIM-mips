/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 *
 * This file is part of RAIM.
 *
 * RAIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RAIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ARCH_BITOPS_H
#define _ARCH_BITOPS_H

#include <util.h>

#define ffs(x)	__ffs(x)
#define fls(x)	__fls(x)
#define ffz(x)	__ffz(x)
#define flz(x)	__flz(x)

static inline int __fls(unsigned long x)
{
	long res;
#ifdef __LP64__
	asm (
		"dclz	%0, %1"
		: "=r"(res)
		: "r"(x)
	);
#else
	asm (
		"clz	%0, %1"
		: "=r"(res)
		: "r"(x)
	);
#endif
	return BITS_PER_LONG - res;
}

static inline int __ffs(unsigned long x)
{
	return __fls(x & (-x));
}

static inline int __flz(unsigned long x)
{
	long res;
#ifdef __LP64__
	asm (
		"dclo	%0, %1"
		: "=r"(res)
		: "r"(x)
	);
#else
	asm (
		"clo	%0, %1"
		: "=r"(res)
		: "r"(x)
	);
#endif
	return BITS_PER_LONG - res;
}

static inline int __ffz(unsigned long x)
{
	return __flz(~(~x & (-~x)));
}

#endif
