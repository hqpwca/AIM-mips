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

#include <sys/types.h>
#include <aim/early_kmmap.h>
#include <aim/mmu.h>
#include <aim/panic.h>
#include <arch-mmu.h>
#include <libc/string.h>

bool early_mapping_valid(struct early_mapping *entry)
{
	return true;
}

void page_index_clear(pgindex_t *addr)
{
/*
	pgindex_t *a;
	for(; a < addr + PGSIZE; a++)
		*a = 0;
*/
	memset(addr, 0, PGSIZE);
}

int page_index_early_map(pgindex_t *pgindex, addr_t paddr, void *vaddr, size_t size)
{
	char *a, *last;
	
	a = (char *)PGROUNDDOWN2((uint32_t)vaddr);
	last = (char *)PGROUNDDOWN2(((uint32_t)vaddr) + size - 1);
	
	for(; a < last; a += PGSIZE2)
	{
		if(pgindex[PDX(a)] & PTE_P)
			return -1;
		pgindex[PDX(a)] = paddr ^ (paddr & 0xfff);
		pgindex[PDX(a)] |= PTE_P;
		paddr += PGSIZE2;
	}
	return 0;
}

void mmu_init(pgindex_t *boot_page_index)
{
	uint32_t cr4;
	
	asm volatile("movl %%cr4,%0" : "=r" (cr4));
	cr4 |= CR4_PSE;
	asm volatile("movl %0,%%cr4" : : "r" (cr4));
	asm volatile("movl %0,%%cr3" : : "r" (boot_page_index));
}

void jump_to_high_addr()
{
	
}

struct segdesc gdt[8];

void change_gdt()
{
	gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0x0, 0xffffffff, 0);
	gdt[SEG_KDATA] = SEG(STA_W, 0x0, 0xffffffff,0);
	
	lgdt(gdt, sizeof(gdt));
}

void arch_mm_init()
{
	page_index_init(pgindex);
	mmu_init(pgindex);
	
	uint32_t cr0;
	asm volatile("movl %%cr0,%0" : "=r" (cr0));
	cr0 |= CR0_PG;
	asm volatile("movl %0,%%cr0" : : "r" (cr0));
	
	jump_to_high_addr();
	
	
}

