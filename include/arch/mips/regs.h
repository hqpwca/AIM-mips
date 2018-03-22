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

#ifndef _ASM_REGS_H
#define _ASM_REGS_H

#include <mipsregs.h>

struct regs {
	/* general purpose registers */
	unsigned long	gpr[32];

	/* coprocessor registers */
	unsigned long	lo;
	unsigned long	hi;
	unsigned long	status;
	unsigned long	cause;
	unsigned long	epc;
	unsigned long	badvaddr;
};

void switch_regs(struct regs *old, struct regs *new);

#endif
