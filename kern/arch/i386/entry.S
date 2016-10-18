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

#include <arch-boot.h>

.text

.globl _start
_start:
	cli
	
clean_bss:
  	movl	_bss_start, %edi
  	movl	_bss_end, %ecx
  	subl	_bss_start, %ecx
  	xorb	%al, %al
  	repz stosb
  	
  	movl	$0x8000000, %esp
  	
  	call	master_early_init
  	
spin:
	jmp		spin

.data
.p2align 2

.globl _bss_start
_bss_start:
	.long __bss_start

.globl _bss_end
_bss_end:
	.long __bss_end


