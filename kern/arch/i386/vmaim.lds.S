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

#define __LDSCRIPT__

/*
 * Using the C preprocessor, we allow includes and macro expansion in this
 * linker script.
 */

ENTRY(_start)

MEMORY
{
	P(WX) :
		ORIGIN = KERN_START,
		LENGTH = 0x1000000
	V(WX) :
		ORIGIN = KERN_BASE,
		LENGTH = 0x1000000
}

PHDRS
{
	entry PT_LOAD;
	edata PT_LOAD;
	text PT_LOAD;
	rodata PT_LOAD;
	data PT_LOAD;
}

SECTIONS
{
	.entry :
	{
		_entry_start = .;
		*(.entry)
		_entry_end = .;
	} >P AT>P : entry
	
	.edata :
	{
		_edata_start = .;
		*(.edata)
		_edata_end = .;
	} >P AT>P : edata
	
	.text :
	{
		_text_start = .;
		*(.text)
		_text_end = .;
	} >V AT>P : text
	
	.rodata :
	{
		_rodata_start = .;
		*(.rodata)
		_rodata_end = .;
	} >V AT>P : rodata
	
	.data :
	{
		_data_start = .;
		*(.data)
		_data_end = .;
	} >V AT>P : data
	
	.bss ALIGN(4) :
	{
		_bss_start = .;
		*(.bss) *(COMMON)
		_bss_end = .;
	}>V AT>P
	
	.boot :
	{
		PROVIDE(pgindex = .);
		. += 32K;
		PROVIDE(boot_stack = .);
	}>V AT>V
	
	PROVIDE(kern_end = .);
	
	/DISCARD/ :
	{
		*(.note.gnu.build-id)
		*(.eh_frame)
		*(.eh_frame_hdr)
	}
}
