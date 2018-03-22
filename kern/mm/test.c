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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <raim/mmu.h>
#include <raim/uvm.h>
#include <arch-mmu.h>
#include <raim/panic.h>
#include <raim/debug.h>
#include <raim/console.h>
#include <sys/types.h>

/* We can make test routines work like initcalls, if we want */
void
mm_test(void)
{
	unsigned long test_addr = PAGE_SIZE;
	unsigned long i;

	kprintf("==========mm_test()  started==========\n");
	struct mm *mm = mm_new(), *new_mm;
	kprintf("pgindex: %p\n", mm->pgindex);
	kprintf("creating uvm\n");
	assert(create_uvm(mm, (void *)0x100000, 5 * PAGE_SIZE, VMA_READ | VMA_WRITE) == 0);
	kprintf("destroying uvm1\n");
	assert(destroy_uvm(mm, (void *)0x100000, 2 * PAGE_SIZE) == 0);
	kprintf("destroying uvm2\n");
	assert(destroy_uvm(mm, (void *)0x100000 + 2 * PAGE_SIZE, 3 * PAGE_SIZE) == 0);
	kprintf("destroying mm\n");
	mm_destroy(mm);
	/* The following should be tested only after kmmap subsystem has
	 * been fully implemented since mm_new() needs it to create
	 * kernel mappings. */
	kprintf("another mm\n");
	mm = mm_new();
	kprintf("pgindex: %p\n", mm->pgindex);
	assert(create_uvm(mm, (void *)test_addr, 1 * PAGE_SIZE, VMA_READ | VMA_WRITE) == 0);
	assert(fill_uvm(mm, (void *)test_addr, 0x40, 1 * PAGE_SIZE) == 0);
	switch_pgindex(mm->pgindex);
	for (i = 0; i < PAGE_SIZE; i += sizeof(unsigned int))
		assert(*(unsigned int *)(test_addr + i) == 0x40404040);

	/* TODO: Find a way to let mm_clone wark. */

	for (i = 0; i < PAGE_SIZE; i += sizeof(unsigned int))
	{
		*(unsigned int *)(test_addr + i) = 0xdeadbeef;
		//kprintf("write (0x%x) : 0x%x\n", test_addr + i, *(unsigned int *)(test_addr + i));
	}
	new_mm = mm_new();
	kprintf("new pgindex: %p\n", new_mm->pgindex);
	mm_clone(new_mm, mm);
	switch_pgindex(new_mm->pgindex);
	for (i = 0; i < PAGE_SIZE; i += sizeof(unsigned int)){
		//kprintf("read (0x%x) : 0x%x\n", test_addr + i, *(unsigned int *)(test_addr + i));
		assert(*(unsigned int *)(test_addr + i) == 0xdeadbeef);
	}
	mm_destroy(new_mm);
	mm_destroy(mm);
	kprintf("yet another mm\n");
	mm = mm_new();
	kprintf("pgindex: %p\n", mm->pgindex);
	mm_destroy(mm);
	kprintf("==========mm_test() finished==========\n");
}
