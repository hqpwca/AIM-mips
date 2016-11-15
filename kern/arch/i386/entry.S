/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <aim/mmu.h>
#include <arch-boot.h>

.section .text.entry

.globl _start
_start:
	cli
	cld
	
clean_bss:
  	movl	$__premap_addr(_bss_start), %edi
  	movl	$__premap_addr(_bss_end), %ecx
  	subl	%edi, %ecx
  	xorb	%al, %al
  	repz stosb
  	
load_segment:
	lgdt    gdtdesc
	movl    %cr0, %eax
	orl     $CR0_PE, %eax
	movl    %eax, %cr0
	
	movw    $(SEG_KDATA<<3), %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %ss
	movw    $0, %ax
	movw    %ax, %fs
	movw    %ax, %gs

	ljmp    $(SEG_KCODE<<3), $prepare_stack
	
prepare_stack:  	
  	movl	$__premap_addr(boot_stack), %esp
  	movl	%esp, %ebp
  	
jump_to_main:
  	movl	$__premap_addr(master_early_init), %eax
  	call	*%eax
  	
spin:
	jmp		spin

.globl high_address_entry
.text
high_address_entry:
	movl	$boot_stack, %esp
	movl	$master_init, %eax
	call	*%eax

.section .data.entry
.p2align 2
gdt:
	SEG_NULLASM
	SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)
	SEG_ASM(STA_W, 0x0, 0xffffffff)

gdtdesc:
	.word   (gdtdesc - gdt - 1)
	.long   gdt

/* SLAVE_CPUS_ENTRY */
.section .text.sentry

.code16
.globl slave_entry
slave_entry:
	cli

	xorw    %ax,%ax
	movw    %ax,%ds
	movw    %ax,%es
	movw    %ax,%ss

slave_prpgdt:
	lgdt    slave_gdtdesc
	movl    %cr0, %eax
	orl     $CR0_PE, %eax
	movl    %eax, %cr0

	ljmp    $(SEG_KCODE<<3), $slave_start32

.code32
slave_start32:
	movw    $(SEG_KDATA<<3), %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %ss
	movw    $0, %ax
	movw    %ax, %fs
	movw    %ax, %gs

setkvm:
	movl    %cr4, %eax
	orl     $(CR4_PSE), %eax
	movl    %eax, %cr4

	movl    (slave_entry-8), %eax
	movl    %eax, %cr3

	movl    %cr0, %eax
	orl     $(CR0_PG), %eax
	movl    %eax, %cr0

slave_jump_to_main:
	movl	(slave_entry-4), %esp
	call	slave_init

slave_spin:
	jmp		slave_spin

.section .data.sentry
.p2align 2
slave_gdt:
	SEG_NULLASM
	SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)
	SEG_ASM(STA_W, 0x0, 0xffffffff)

slave_gdtdesc:
	.word   (slave_gdtdesc - slave_gdt - 1)
	.long   slave_gdt

