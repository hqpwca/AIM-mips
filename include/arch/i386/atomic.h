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

#ifndef _ATOMIC_H
#define _ATOMIC_H

/* counter += val */
static inline void atomic_add(atomic_t *counter, uint32_t val)
{
	asm volatile(
		"lock addl %1,%0"
		: "+m" (counter)
		: "ir" (val));
}

/* counter -= val */
static inline void atomic_sub(atomic_t *counter, uint32_t val)
{
	asm volatile(
		"lock subl %1,%0"
		: "+m" (counter)
		: "ir" (val));
}

/* counter++ */
static inline void atomic_inc(atomic_t *counter)
{
	asm volatile(
		"lock incl %0"
		: "+m" (counter));
}

/* counter-- */
static inline void atomic_dec(atomic_t *counter)
{
	asm volatile(
		"lock decl %0"
		: "+m" (counter));
}

static inline void atomic_set_bit(
	unsigned long nr,
	volatile unsigned long *addr)
{
	asm volatile( 
		"btsl %1,%0" 
		: "=m" (addr) 
		: "ir" (nr));
}

static inline void atomic_clear_bit(
	unsigned long nr,
	volatile unsigned long *addr)
{
	asm volatile( 
		"btcl %1,%0" 
		: "=m" (addr) 
		: "ir" (nr));
}

#endif

