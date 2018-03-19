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

#include <sys/types.h>
#include <asm.h>
#include <arch-sync.h>

/* counter += val */
static inline void atomic_add(__unused atomic_t *counter, __unused uint32_t val)
{
}

/* counter -= val */
static inline void atomic_sub(__unused atomic_t *counter, __unused uint32_t val)
{
}

static inline void atomic_inc(atomic_t *counter)
{
	uint32_t reg;
	smp_mb();
	asm volatile (
		"	.set	push;"
		"	.set	reorder;"
		"1:	ll	%[reg], %[mem];"
		"	addu	%[reg], 1;"
		"	sc	%[reg], %[mem];"
		"	beqz	%[reg], 1b;"
		"	.set	pop;"
		: [reg] "=&r"(reg), [mem] "+m"(*counter)
	);
	smp_mb();
}

/* counter-- */
static inline void atomic_dec(atomic_t *counter)
{
	uint32_t reg;
	smp_mb();
	asm volatile (
		"	.set	push;"
		"	.set	reorder;"
		"1:	ll	%[reg], %[mem];"
		"	addu	%[reg], -1;"
		"	sc	%[reg], %[mem];"
		"	beqz	%[reg], 1b;"
		"	.set	pop;"
		: [reg] "=&r"(reg), [mem] "+m"(*counter)
	);
	smp_mb();
}

static inline void atomic_set_bit(unsigned long nr,
				  volatile unsigned long *addr)
{
	unsigned long *m = ((unsigned long *)addr) + ((--nr) >> BITS_PER_LONG_LOG);
	int bit = nr & BITS_PER_LONG_MASK;
	unsigned long temp;

	smp_mb();
	asm volatile (
		"1:	"__LL " %0, %1;"
		"	or	%0, %2;"
		"	"__SC " %0, %1;"
		"	beqzl	%0, 1b;"
		: "=&r"(temp), "+m"(*m)
		: "ir"(1UL << bit)
	);
	smp_mb();
}

static inline void atomic_clear_bit(unsigned long nr,
				    volatile unsigned long *addr)
{
	unsigned long *m = ((unsigned long *)addr) + ((--nr) >> BITS_PER_LONG_LOG);
	int bit = nr & BITS_PER_LONG_MASK;
	unsigned long temp;

	smp_mb();
	asm volatile (
		"1:	"__LL " %0, %1;"
		"	and	%0, %2;"
		"	"__SC " %0, %1;"
		"	beqzl	%0, 1b;"
		: "=&r"(temp), "+m"(*m)
		: "ir"(~(1UL << bit))
	);
	smp_mb();
}

#endif
