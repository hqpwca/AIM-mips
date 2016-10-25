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

#include <aim/mmu.h>

/*
 * Using the C preprocessor, we allow includes and macro expansion in this
 * linker script.
 */

ENTRY(_start)

MEMORY
{
	PHDR(WX) :
		ORIGIN = KERN_START,
		LENGTH = 0x1000000
	VIRT(WX) :
		ORIGIN = KERN_BASE + KERN_START,
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
	.text.entry KERN_START : 
	{
		_entry_start = .;
		*(.text.entry*)
		_entry_end = .;
	} >PHDR AT>PHDR : entry
	
	.data.entry :
	{
		_edata_start = .;
		*(.data.entry*)
		_edata_end = .;
	} >PHDR AT>PHDR : edata
	
	.text __postmap_addr(_edata_end) :
	{
		_text_start = .;
		*(.text*)
		_text_end = .;
	} >VIRT AT>PHDR : text
	
	.got : ALIGN(4)
	{
		*(.got*)
		*(.igot*)
	} >VIRT AT>PHDR :rodata
	
	.rodata : ALIGN(4)
	{
		_rodata_start = .;
		*(.rodata*)
		_rodata_end = .;
	} >VIRT AT>PHDR : rodata
	
	.data : ALIGN(32)
	{
		_data_start = .;
		*(.data*)
		_data_end = .;
	} >VIRT AT>PHDR : data
	
	.bss ALIGN(32) :
	{
		_bss_start = .;
		*(.bss)
		_bss_end = .;
	}>VIRT AT>PHDR :NONE
	
	.boot :
	{
		. = ALIGN(PAGE_SIZE);
		PROVIDE(pgindex = .);
		. += 32K;
		PROVIDE(boot_stack = .);
		PROVIDE(simple1_start = .);
		. += 32K;
	}>VIRT AT>PHDR :NONE
	
	PROVIDE(kern_end = .);
	
	/DISCARD/ :
	{
		*(.note.gnu.build-id)
		*(.eh_frame)
		*(.eh_frame_hdr)
	}
}
