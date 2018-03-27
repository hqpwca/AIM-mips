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

#include <asm-generic/vmraim.lds.h>
#include <arch-mmu.h>

/*
 * Using the C preprocessor, we allow includes and macro expansion in this
 * linker script.
 */

ENTRY(_start)

SECTIONS {
	. = KERN_BASE;
	.text : AT(KERN_START) {
	    *(*);
	    EARLY_INIT(STRUCT_ALIGNMENT)
		NORM_INIT(STRUCT_ALIGNMENT)
		LATE_INIT(STRUCT_ALIGNMENT)
	}
	
    PROVIDE(simple1_start = .);
		. += 32K;

    . = ALIGN(PAGE_SIZE);
	KERN_END = .;
	PROVIDE(kern_end = .);
};
