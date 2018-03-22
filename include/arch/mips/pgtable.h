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

#ifndef _PGTABLE_H
#define _PGTABLE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <arch-mmu.h>

#define PGTABLE_LEVEL	4

#define PTE_LOWMASK		0xfffU
/* These bits match the mode bits in TLB entries */
#define PTE_CACHE_MASK		0xe00U
# define PTE_CACHEABLE		0x600U
# define PTE_UNCACHED		0x400U
#define PTE_DIRTY		0x100U
#define PTE_VALID		0x080U
#define PTE_GLOBAL		0x040U
/* Loongson 2F has this bit for buffer overflow protection.  Not sure whether
 * Loongson 3A has it. */
#define PTE_NOEXEC		0x008U
/* Extra page table entry flags not needed by hardware */
#define PTE_SOFT_SHIFT		6

#define PTE_PADDR(pte)		((pte) & ~PTE_LOWMASK)
#define PTE_FLAGS(pte)		((pte) & PTE_LOWMASK)

#define PTXMASK		((1 << (PAGE_SHIFT - WORD_SHIFT)) - 1)
#define NR_PTENTRIES	(1 << (PAGE_SHIFT - WORD_SHIFT))

#define PTXSHIFT	PAGE_SHIFT
#define PDXSHIFT	(PTXSHIFT + PAGE_SHIFT - WORD_SHIFT)

#ifndef __ASSEMBLER__
#include <sys/types.h>

#define PDX(va)		(((ULCAST(va)) >> PDXSHIFT) & PTXMASK)
#define PTX(va)		(((ULCAST(va)) >> PTXSHIFT) & PTXMASK)

extern pgindex_t *pgdir_slots[];	/* Defined in vmraim.lds.S */
#endif	/* !__ASSEMBLER__ */

#endif
