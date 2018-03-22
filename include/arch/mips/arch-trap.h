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

#ifndef _ARCH_TRAP_H
#define _ARCH_TRAP_H

#define TLB_REFILL_ENTRY    0xffffffff80000000
#define XTLB_REFILL_ENTRY   0xffffffff80000080
#define CACHE_ERROR_ENTRY   0xffffffff80000100
#define GENERIC_EXCEPT_ENTRY    0xffffffff80000180

#ifndef __ASSEMBLER__

#include <sys/types.h>
#include <cp0regdef.h>

struct trapframe {
	/* Here I just copy from struct regs.  We can make them
	 * different later. */
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

static inline bool from_kernel(struct trapframe *tf)
{
	return ((tf->status & ST_KSU) == KSU_KERNEL);
}
#endif	/* !__ASSEMBLER__ */

#endif /* _ARCH_TRAP_H */
