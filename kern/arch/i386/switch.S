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

/* switch_regs(struct context *old, struct context *new) */

.globl switch_regs
.globl forkret

.text
switch_regs:
	movl	4(%esp), %eax
	movl	8(%esp), %edx

	movl	%ebp, 4(%eax)
	movl	%edi, 8(%eax)
	movl	%esi, 12(%eax)
	movl	%ebx, 16(%eax)
	movl	%esp, (%eax)

	movl	(%edx), %esp
	movl	4(%edx), %ebp
	movl	8(%edx), %edi
	movl	12(%edx), %esi
	movl	16(%edx), %ebx
	
	ret
