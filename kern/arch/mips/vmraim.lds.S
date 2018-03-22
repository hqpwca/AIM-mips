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

#define __LDSCRIPT__

#include <raim/mmu.h>
#include <asm-generic/vmraim.lds.h>

/*
 * Using the C preprocessor, we allow includes and macro expansion in this
 * linker script.
 */

ENTRY(__start)

SECTIONS {
	. = KERN_START;
	.text :
  {
		*(.text*)
	}

  STRUCT_ALIGN();
	.rodata :
  {
		*(.rodata*)
		EARLY_INIT(STRUCT_ALIGNMENT)
		NORM_INIT(STRUCT_ALIGNMENT)
		LATE_INIT(STRUCT_ALIGNMENT)
	}

	/* Global Offset Table (GOT) for position independent code.
	 * Since we are using our own linker script, we must provide
	 * the base address of GOT, denoted by symbol _gp, to our linker.
	 *
	 * Don't ask me why I select this value; this is the default choice
	 * of GCC.  You can view the default linker script used by GCC and
	 * see the same line via the following options:
	 * -Wl,--verbose
	 * You may notice one difference.  The reason I choose 'PROVIDE'
	 * instead of 'HIDDEN' is somewhat complex.  We could not ensure
	 * that BIOSes or bootloaders may set up GP and t9 registers for
	 * us.  We have to setup GP and t9 by our own.
	 */
	PROVIDE(_gp = ALIGN (16) + 0x7ff0);

	.got :
  {
		*(.got*)
	}
  .data :
  {
		*(.data*)
	}

	/* Uninitialized sections comes after */

  PROVIDE(__bss_begin__ = .);
	PROVIDE(_bss_begin = .);
	.bss :
  {
		*(.bss*)
	}
	/*
	 * MIPS ISA doesn't require alignment, paging or anything else
	 * to run a kernel.  This alignment is only for cache-friendliness.
	 */
	. = ALIGN(KSTACKSIZE);
  .boot : ALIGN(0x1000)
	{
		. = ALIGN(PAGE_SIZE);
		PROVIDE(pgindex = .);
		. += 32K;
		PROVIDE(boot_stack = .);
		PROVIDE(simple1_start = .);
		. += 32K;
  }
	PROVIDE(__bss_end__ = .);
	PROVIDE(_bss_end = .);
	PROVIDE(_kern_end = ALIGN(PAGE_SIZE));
  PROVIDE(kern_end = .);
}
