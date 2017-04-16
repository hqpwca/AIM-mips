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
#include <aim/vmm.h>
#include <aim/pmm.h>
#include <aim/uvm.h>
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

void init_free_pages()
{
	size_t kend = kva2pa(&kern_end);
	kend = ALIGN_ABOVE(kend, PAGE_SIZE);
	kprintf("Start Freeing: 0x%x ~ 0x%x...\n", kend, MEM_SIZE - 0x10000000);
	
	struct pages *p = kmalloc(sizeof(*p), 0);
	p->paddr = kend;
	p->size = MEM_SIZE - kend - 0x10000000;
	p->flags = GFP_UNSAFE;
	
	free_pages(p);
	
	kprintf("Finish Freed: 0x%x ~ 0x%x...\n", kend, MEM_SIZE - 0x10000000);
}

int page_index_early_map(pgindex_t *pgindex, addr_t paddr, void *vaddr, size_t size)
{
	char *a, *last;
	
	a = (char *)PGROUNDDOWN2((uint32_t)vaddr);
	last = (char *)PGROUNDDOWN2(((uint32_t)vaddr) + size);
	
	for(; a < last; a += PGSIZE2)
	{
		if(pgindex[PDX(a)] & PTE_P)
			return -1;
		pgindex[PDX(a)] = paddr ^ (paddr & 0xfff);
		pgindex[PDX(a)] |= PTE_P | PTE_W | PTE_PS;
		paddr += PGSIZE2;
	}
	return 0;
}

void mmu_init(pgindex_t *boot_page_index)
{
	uint32_t cr0,cr4;
	
	asm volatile("movl %%cr4,%0" : "=r" (cr4));
	cr4 |= CR4_PSE;
	asm volatile("movl %0,%%cr4" : : "r" (cr4));
	asm volatile("movl %0,%%cr3" : : "r" (boot_page_index));
	
	asm volatile("movl %%cr0,%0" : "=r" (cr0));
	cr0 |= CR0_PG;
	asm volatile("movl %0,%%cr0" : : "r" (cr0));
}

