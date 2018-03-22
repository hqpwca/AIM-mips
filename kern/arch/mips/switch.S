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

#include <asm.h>
#include <regdef.h>
#include <cp0regdef.h>
#include <stack.h>

/* switch_regs(struct context *old, struct context *new) */

/* This function merely switches registers and you can safely
 * ignore this implementation if you're not interested. */
BEGIN(switch_regs)
	.set	push
	.set	noat
	PUSHUE	zero, a0
	PUSHUE	AT, a0
	PUSHUE	v0, a0
	PUSHUE	v1, a0
	PUSHUE	a0, a0
	PUSHUE	a1, a0
	PUSHUE	a2, a0
	PUSHUE	a3, a0
	PUSHUE	t0, a0
	PUSHUE	t1, a0
	PUSHUE	t2, a0
	PUSHUE	t3, a0
	PUSHUE	t4, a0
	PUSHUE	t5, a0
	PUSHUE	t6, a0
	PUSHUE	t7, a0
	PUSHUE	s0, a0
	PUSHUE	s1, a0
	PUSHUE	s2, a0
	PUSHUE	s3, a0
	PUSHUE	s4, a0
	PUSHUE	s5, a0
	PUSHUE	s6, a0
	PUSHUE	s7, a0
	PUSHUE	t8, a0
	PUSHUE	t9, a0
	PUSHUE	k0, a0
	PUSHUE	k1, a0
	PUSHUE	gp, a0
	PUSHUE	sp, a0
	PUSHUE	s8, a0
	PUSHUE	ra, a0
	mflo	a2
	PUSHUE	a2, a0
	mfhi	a2
	PUSHUE	a2, a0
	mfc0	a2, CP0_STATUS
	PUSHUE	a2, a0
	mfc0	a2, CP0_CAUSE
	PUSHUE	a2, a0
	MFC0	a2, CP0_EPC
	PUSHUE	a2, a0
	MFC0	a2, CP0_BADVADDR
	PUSHUE	a2, a0

	POPUF	a2, a1	/* discard zero */
	POPUF	AT, a1
	POPUF	v0, a1
	POPUF	v1, a1
	POPUF	a0, a1
	POPUF	a2, a1	/* discard a1 */
	POPUF	a2, a1
	POPUF	a3, a1
	POPUF	t0, a1
	POPUF	t1, a1
	POPUF	t2, a1
	POPUF	t3, a1
	POPUF	t4, a1
	POPUF	t5, a1
	POPUF	t6, a1
	POPUF	t7, a1
	POPUF	s0, a1
	POPUF	s1, a1
	POPUF	s2, a1
	POPUF	s3, a1
	POPUF	s4, a1
	POPUF	s5, a1
	POPUF	s6, a1
	POPUF	s7, a1
	POPUF	t8, a1
	POPUF	t9, a1
	POPUF	k0, a1
	POPUF	k1, a1
	POPUF	gp, a1
	POPUF	sp, a1
	POPUF	s8, a1
	POPUF	ra, a1
	POPUF	a2, a1
	mtlo	a2
	POPUF	a2, a1
	mthi	a2
	POPUF	a2, a1
	mtc0	a2, CP0_STATUS
	POPUF	a2, a1
	mtc0	a2, CP0_CAUSE
	POPUF	a2, a1
	MTC0	a2, CP0_EPC
	POPUF	a2, a1
	MTC0	a2, CP0_BADVADDR

	jr	ra
	.set	pop
END(switch_regs)
