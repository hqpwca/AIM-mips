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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <arch-boot.h>

.section .entry

.code16 
.globl _start
_start:

bootasm:

	cli

	xorw    %ax,%ax
	movw    %ax,%ds
	movw    %ax,%es
	movw    %ax,%ss

seta20.1:
	inb     $0x64,%al
	testb   $0x2,%al
	jnz     seta20.1

	movb    $0xd1,%al
	outb    %al,$0x64

seta20.2:
	inb     $0x64,%al
	testb   $0x2,%al
	jnz     seta20.2

	movb    $0xdf,%al
	outb    %al,$0x60

prpgdt:
	lgdt    gdtdesc
	movl    %cr0, %eax
	orl     $CR0_PE, %eax
	movl    %eax, %cr0

	ljmp    $(SEG_KCODE<<3), $start32

.code32
start32:

	movw    $(SEG_KDATA<<3), %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %ss
	movw    $0, %ax
	movw    %ax, %fs
	movw    %ax, %gs

  # Set up the stack pointer and call into C.
	movl    $bootasm, %esp
	
  # Clean the BSS section
clean_bss:
  	movw	_bss_start, %di
  	movw	_bss_end, %cx
  	subw	_bss_start, %cx
  	xorb	%al, %al
  	repz stosb
	
	call    bootmain

spin:
	jmp     spin

.data
.p2align 2
gdt:
	SEG_NULLASM
	SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)
	SEG_ASM(STA_W, 0x0, 0xffffffff)

gdtdesc:
	.word   (gdtdesc - gdt - 1)
	.long   gdt
	
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word __bss_end


