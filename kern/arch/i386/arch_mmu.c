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
#include <aim/mmu.h>
#include <aim/panic.h>
#include <arch-mmu.h>

#define MEM_STOP 0x3f000000

struct run{
	struct run *next;
};

extern void *end;

struct run *runlist;

void kfree(char *v)
{
	struct run *r;
	
	if((addr_t)v % PGSIZE || v < end || v > MEM_STOP)
		panic("kfree");
	
	memset(v, 1, PGSIZE)
	r = (struct run*)v;
	r -> next = runlist;
	runlist = r;
}

void freerange(void *vstart, void *vend)
{
	char *p = (char *)PGROUNDDOWN(vstart);
	
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
	addr_t pgaddr = kalloc();
	pgindex_t *pgindex;
	
	memset(pgindex, 0, sizeof(pgindex));
	pgindex -> pfaddr = pgaddr >> 12;
	pgindex -> present = 1;
	
	return pgindex;
}

void destroy_pgindex(pgindex_t *pgindex)
{
	addr_t pgaddr = pgindex -> pfaddr << 12;
	
	kfree(pgaddr);
}

static pte_t *walkpgdir(pgindex_t *pgindex, const void *vaddr, int alloc)
{
	pde_t *pde;
	pte_t *pgtab;
	
	pde = &pgdir[PDX(vaddr)];
	if(*pde & PTE_P) {
		pgtab = (pte_t*)(postmap_addr(PTE_ADDR(*pde)));
	} else {
		if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
			return 0;
		memset(pgtab, 0, PGSIZE);
		*pde = premap_addr(pgtab) | PTE_P | PTE_W | PTE_U;
	}
	return &pgtab[PTX(vaddr)];
}

int map_pages(pgindex_t *pgindex, void *vaddr, addr_t paddr, size_t size, uint32_t flags)
{
	char *a, *last;
	pte_t *pte;
	
	a = (char *)PGROUNDDOWN((addr_t)vaddr);
	last = (char *)PGROUNDDOWN(((addr_t)va) + size - 1);
	
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
	
	a = (char *)PGROUNDDOWN((addr_t)vaddr);
	last = (char *)PGROUNDDOWN(((addr_t)va) + size - 1);
	
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
	
	a = (char *)PGROUNDDOWN((addr_t)vaddr);
	last = (char *)PGROUNDDOWN(((addr_t)va) + size - 1);
	
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
	
	a = (char *)PGROUNDDOWN((addr_t)vaddr);
	last = (char *)PGROUNDDOWN(((addr_t)va) + size - 1);
	
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
}

pgindex_t *get_pgindex(void)
{
	addr_t pgindex;
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

void arch_mm_init()
{
	freerange((void *)0x100000, (void *)MEM_STOP);
}

void mmu_init(pgindex_t *boot_page_index)
{
}

