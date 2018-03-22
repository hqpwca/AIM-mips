/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
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

#ifndef _ARCH_IRQ_H
#define _ARCH_IRQ_H

#ifndef __ASSEMBLER__

#define local_irq_enable() \
	asm volatile ( \
		".set	push;" \
		".set	noat;" \
		"mfc0	$1, $12;" \
		"ori	$1, 1;" \
		"mtc0	$1, $12;" \
		".set	pop;" \
		: /* no output */ \
		: /* no input */ \
		: "$1", "memory" \
	)

#define local_irq_disable() \
	asm volatile ( \
		".set	push;" \
		".set	noat;" \
		"mfc0	$1, $12;" \
		"ori	$1, 1;" \
		"xori	$1, 1;" \
		"mtc0	$1, $12;" \
		".set	pop;" \
		: /* no output */ \
		: /* no input */ \
		: "$1", "memory" \
	)

#define local_irq_save(flags) \
	asm volatile ( \
		".set	push;" \
		".set	noat;" \
		"mfc0	%0, $12;" \
		"ori	$1, %0, 0x1f;" \
		"xori	$1, $1, 0x1f;" \
		"mtc0	$1, $12;" \
		".set	pop;" \
		: "=r"(flags) \
		: /* no input */ \
		: "$1", "memory" \
	)

#define local_irq_restore(flags) \
	asm volatile ( \
		"	.set	push;" \
		"	.set	reorder;" \
		"	.set	noat;" \
		"	mfc0	$1, $12;" \
		"	andi	%0, 1;" \
		"	ori	$1, 0x1f;" \
		"	xori	$1, 0x1f;" \
		"	or	%0, $1;" \
		"	mtc0	%0, $12;" \
		"	.set	pop;" \
		: /* no output */ \
		: "r"(flags) \
		: "$1", "memory" \
	)

#endif /* !__ASSEMBLER__ */

#endif /* _ARCH_IRQ_H */
