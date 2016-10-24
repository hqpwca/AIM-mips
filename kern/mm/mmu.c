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

/*
 * This source file provides upper-level utilities to handle memory mappings.
 * On different systems, memory management unit (MMU)s may look different,
 * may have different names and interfaces, or may even be absent (like MIPS).
 * From the kernel's point of view, we want to unify their access interface,
 * so this wrapper is here.
 */

/*
 * It may take a lot of configuration logic to decide how early mappings should
 * be done. This leads to even more trouble when we mark these mappings
 * in proper kernel data structures later.
 * AIM uses a very simple queue located in .bss to solve the problem:
 * Early initialization routines submit mappings, and the platform-independent
 * routines will call underlying platform-dependent ones to apply them.
 * These data structure are kept in memory, and will later be used to
 * initialize the page allocator and the kmmap subsystem.
 */

void mymemset(void *st, char val, size_t size)
{
	void *a = st;
	
	for(; a <= st + size; a++)
		*(char *)a = val;
}

int page_index_init(pgindex_t *boot_page_index)
{
	struct early_mapping *mapping = early_mapping_next(NULL);
	int ret;

	page_index_clear(boot_page_index);

	for (; mapping != NULL; mapping = early_mapping_next(mapping)) {
		ret = page_index_early_map(boot_page_index, mapping->paddr,
			(void *)mapping->vaddr, mapping->size);
		if (ret == EOF) return EOF;
	}
	return 0;
}

/*
struct run{
	struct run *next;
};

static struct run *runlist;

void kfree(char *v)
{
	struct run *r;
	
	if((uint32_t)v % PGSIZE || (uint32_t)v < KERN_END || premap_addr(v) >= MEM_STOP)
		panic("kfree");
	
	mymemset(v, 1, PGSIZE);
	r = (struct run*)v;
	r -> next = runlist;
	runlist = r;
}

void freerange(void *vstart, void *vend)
{
	char *p = (char *)PGROUNDDOWN((uint32_t)vstart);
	
	for(; p + PGSIZE <= (char *)vend; p += PGSIZE) {
		kfree(p);
	}
}

char *kalloc(void)
{
	struct run *r = runlist;
	
	if(r)
		runlist = r -> next;
	
	return (char *)r;
}

pgindex_t *init_pgindex(void)
{
	pgindex_t *pgindex = (pgindex_t *)kalloc();
	
	return pgindex;
}

void destroy_pgindex(pgindex_t *pgindex)
{	
	kfree((char *)pgindex);
}

static pte_t *walkpgdir(pgindex_t *pgindex, const void *vaddr, int alloc)
{
	pde_t *pde;
	pte_t *pgtab;
	
	pde = &pgindex[PDX(vaddr)];
	if(*pde & PTE_P) {
		pgtab = (pte_t*)(postmap_addr(PTE_ADDR(*pde)));
	} else {
		if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
			return 0;
		mymemset(pgtab, 0, PGSIZE);
		*pde = premap_addr(pgtab) | PTE_P | PTE_W | PTE_U;
	}
	return &pgtab[PTX(vaddr)];
}

int map_pages(pgindex_t *pgindex, void *vaddr, addr_t paddr, size_t size, uint32_t flags)
{
	char *a, *last;
	pte_t *pte;
	
	a = (char *)PGROUNDDOWN((uint32_t)vaddr);
	last = (char *)PGROUNDDOWN(((uint32_t)vaddr) + size - 1);
	
	for(;;)
	{
		if((pte = walkpgdir(pgindex, a, 1)) == 0)
			return -1;
		if(*pte & PTE_P)
			panic("remap");
		*pte = paddr | flags | PTE_P;
		if(a == last)
			break;
		a += PGSIZE;
		paddr += PGSIZE;
	}
	return 0;
}

ssize_t unmap_pages(pgindex_t *pgindex, void *vaddr, size_t size, addr_t *paddr)
{
	char *a, *last;
	pte_t *pte;
	
	ssize_t unmap_size = 0;
	
	a = (char *)PGROUNDDOWN(((uint32_t)vaddr));
	last = (char *)PGROUNDDOWN(((uint32_t)vaddr) + size - 1);
	
	for(;;)
	{
		if((pte = walkpgdir(pgindex, a, 0)) == 0)
			return -1;
		if((*pte & PTE_P) == 0)
			break;
		*pte = 0x0;
		if(a == last)
			break;
		a += PGSIZE;
		paddr += PGSIZE;
		unmap_size += PGSIZE;
	}
	return unmap_size;
}

int set_pages_perm(pgindex_t *pgindex, void *vaddr, size_t size, uint32_t flags)
{
	char *a, *last;
	pte_t *pte;
	
	a = (char *)PGROUNDDOWN((uint32_t)vaddr);
	last = (char *)PGROUNDDOWN(((uint32_t)vaddr) + size - 1);
	
	for(;;)
	{
		if((pte = walkpgdir(pgindex, a, 0)) == 0)
			return -1;
		if((*pte & PTE_P) == 0)
			panic("set_perm: not mapped");
		*pte |= flags;
		if(a == last)
			break;
		a += PGSIZE;
	}
	return 0;
}

ssize_t invalidate_pages(pgindex_t *pgindex, void *vaddr, size_t size, addr_t *paddr)
{
	char *a, *last;
	pte_t *pte;
	
	ssize_t invalid_size = 0;
	
	a = (char *)PGROUNDDOWN((uint32_t)vaddr);
	last = (char *)PGROUNDDOWN(((uint32_t)vaddr) + size - 1);
	
	for(;;)
	{
		if((pte = walkpgdir(pgindex, a, 0)) == 0)
			return -1;
		if((*pte & PTE_P) == 0)
			break;
		*pte ^= PTE_P;
		if(a == last)
			break;
		a += PGSIZE;
		paddr += PGSIZE;
		invalid_size += PGSIZE;
	}
	return invalid_size;
}

int switch_pgindex(pgindex_t *pgindex)
{
	asm volatile("movl %0,%%cr3" : : "r" (pgindex));
	
	return 0;
}

pgindex_t *get_pgindex(void)
{
	pgindex_t *pgindex;
	asm volatile("movl %%cr3,%0" : "=r" (pgindex));
	return pgindex;
}

void *uva2kva(pgindex_t *pgindex, void *uaddr)
{
	pte_t *pte;
	
	pte = walkpgdir(pgindex, uaddr, 0);
	if((*pte & PTE_P) == 0)
		return 0;
	if((*pte & PTE_U) == 0)
		return 0;
	return (void *)postmap_addr(PTE_ADDR(*pte));
}
*/

/* handlers after mmu start and after jump */
#define MMU_HANDLER_QUEUE_LENGTH	10
static int __mmu_handler_queue_size;
static generic_fp __mmu_handler_queue[MMU_HANDLER_QUEUE_LENGTH];

void mmu_handlers_clear(void)
{
	__mmu_handler_queue_size = 0;
}

int mmu_handlers_add(generic_fp entry)
{
	if (__mmu_handler_queue_size > MMU_HANDLER_QUEUE_LENGTH) {
		/* Bad data structure. Panic immediately to prevent damage. */
		panic("MMU handler data structure invalid.\n");
	}
	if (__mmu_handler_queue_size == MMU_HANDLER_QUEUE_LENGTH) {
		/* Queue full */
		return EOF;
	}
	__mmu_handler_queue[__mmu_handler_queue_size] = entry;
	__mmu_handler_queue_size += 1;
	return 0;
}

void mmu_handlers_apply(void)
{
	for (int i = 0; i < __mmu_handler_queue_size; ++i) {
		__mmu_handler_queue[i]();
	}
}

