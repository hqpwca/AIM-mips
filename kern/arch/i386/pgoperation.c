#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/early_kmmap.h>
#include <aim/mmu.h>
#include <aim/vmm.h>
#include <aim/pmm.h>
#include <aim/panic.h>
#include <aim/console.h>
#include <arch-mmu.h>
#include <libc/string.h>

pgindex_t *init_pgindex(void)
{
	pgindex_t *pgindex = (pgindex_t *)(uint32_t)pgalloc();
	int ret;

	struct early_mapping *mapping = early_mapping_next(NULL);

	for (; mapping != NULL; mapping = early_mapping_next(mapping)) {
		kpdebug("Early mapping : paddr: 0x%x, vaddr: 0x%x, size: 0x%x\n",
			(uint32_t)mapping->paddr, (uint32_t)mapping->vaddr, (uint32_t)mapping->size);
		ret = map_pages(pgindex, mapping->vaddr,
			mapping->paddr, mapping->size, 0);
		if (ret == EOF) return EOF;
	}
	
	return pgindex;
}

void destroy_pgindex(pgindex_t *pgindex)
{	
	pgfree((addr_t)(uint32_t)pgindex);
}

static pte_t *walkpgdir(pgindex_t *pgindex, const void *vaddr, int alloc)
{
	pde_t *pde;
	pte_t *pgtab;
	
	pde = &pgindex[PDX(vaddr)];
	if(*pde & PTE_P) {
		pgtab = (pte_t*)(postmap_addr(PTE_ADDR(*pde)));
	} else {
		if(!alloc || (pgtab = (pte_t*)(uint32_t)pgalloc()) == 0)
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