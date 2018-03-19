/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 *
 * This file is part of AIMv6.
 *
 * AIMv6 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIMv6 is distributed in the hope that it will be useful,
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

#include <asm.h>
#include <regdef.h>
#include <cp0regdef.h>
#include <arch-smp.h>
#include <arch-trap.h>
#include <arch-mmu.h>
#include <pgtable.h>

BEGIN(tlb_entry)
	.set	push
	.set	noat
	/*
	 * Again I'm assuming that _gp indeed points to kernel GOT.  These
	 * should be changed in case of ASLR.
	 */
	lui	k1, %hi(_gp)
	ADDIU	k1, %lo(_gp)
	LOAD	k1, %got(__tlb_entry)(k1)
	jr	k1
	.set	pop
END(tlb_entry)

BEGIN(__tlb_entry)
	.set	push
	.set	noat
	cpuid	k0, k1
	SLL	k0, WORD_SHIFT
	lui	k1, %hi(_gp)
	ADDIU	k1, %lo(_gp)
	LOAD	k1, %got(pgdir_slots)(k1)
	ADDU	k1, k0
	LOAD	k1, (k1)	/* k1 = PGINDEX */
	beqz	k1, 9f

	MFC0	k0, CP0_BADVADDR
	SRL	k0, PDXSHIFT
	AND	k0, PTXMASK
	SLL	k0, WORD_SHIFT
	ADDU	k1, k0
	LOAD	k1, (k1)	/* k1 = PTE = PDE[PDX(va)] */
	beqz	k1, 9f

	MFC0	k0, CP0_BADVADDR
	SRL	k0, PTXSHIFT
	AND	k0, PTXMASK - 1
	SLL	k0, WORD_SHIFT
	ADDU	k1, k0
	LOAD	k0, (k1)		/* k0 = PTE[EVEN(PTX(va))] */
	LOAD	k1, WORD_SIZE(k1)	/* k1 = PTE[ODD(PTX(va))] */
  
	SRL	k0, PTE_SOFT_SHIFT
	SRL	k1, PTE_SOFT_SHIFT

	MTC0	k0, CP0_ENTRYLO0
	MTC0	k1, CP0_ENTRYLO1
	/* Check if the entry to be written is invalid */
	MFC0	k1, CP0_BADVADDR
	SRL	k1, PTXSHIFT
	AND	k1, 1
	beqz	k1, 1f
	MFC0	k0, CP0_ENTRYLO1
1:	AND	k0, PTE_VALID >> PTE_SOFT_SHIFT
	beqz	k0, 9f
	tlbwr
	eret

9:	LI	k0, GENERIC_EXCEPT_ENTRY
	jalr	k0
	.set	pop
END(__tlb_entry)
