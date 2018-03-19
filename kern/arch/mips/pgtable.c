#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <libc/string.h>
#include <aim/early_kmmap.h>
#include <aim/mmu.h>
#include <aim/vmm.h>
#include <aim/pmm.h>
#include <aim/uvm.h>
#include <aim/panic.h>
#include <aim/console.h>
#include <pgtable.h>
#include <util.h>
#include <tlb.h>
#include <errno.h>
#include <arch-smp.h>
#include <arch-mmu.h>
#include <libc/string.h>

pgindex_t *pgdir_slots[MAX_CPUS];

struct pagedesc {
	uint32_t	pdev;	/* page directory physical address */
	uint32_t	ptev;	/* leaf page table physical address */
	uint32_t	pdx;
	uint32_t	ptx;
};

/*
 * Get a page index descriptor (PDE, PTE in our case) for the page table,
 * possibly creating leaf page tables if needed.
 */
static int
__getpagedesc(pgindex_t *pgindex,
	      void *addr,
	      bool create,
	      struct pagedesc *pd)
{
	addr_t paddr;
	pde_t *pde = (pde_t *)pgindex;
	pd->pdev = (uint32_t)pgindex;
	pd->pdx = PDX(addr);
	if (pde[pd->pdx] != 0) {
		/* We already have the intermediate directory */
		pd->ptev = pde[pd->pdx];
	} else {
		/* We don't have it, fail or create one */
		if (!create) {
			return -ENOENT;
		}
		if ((paddr = pgalloc()) == (addr_t)-1)
			return -ENOMEM;
		pd->ptev = (uint32_t)pa2kva(paddr);
		memset((void *)pd->ptev, 0, PAGE_SIZE);
		pde[pd->pdx] = pd->ptev;
	}
	pd->ptx = PTX(addr);
	return 0;
}

/*
 * This function assumes that:
 * 1. (Leaf) page table entries for vaddr..vaddr+size are already zero.
 * 2. @vaddr and @size are page-aligned.
 */
static void
__free_intermediate_pgtable(pgindex_t *pgindex, void *vaddr, size_t size)
{
	pde_t *pde = (pde_t *)pgindex;
	uint32_t pdx = PDX(vaddr), pdx_end = PDX(vaddr + size - PAGE_SIZE);
	for (; pdx <= pdx_end; ++pdx) {
		pte_t *pte = (pte_t *)pde[pdx];
		for (int i = 0; i < NR_PTENTRIES; ++i) {
			if (pte[i] != 0)
				goto rollback_next_pde;
		}
		pgfree(kva2pa((void *)pde[pdx]));
		pde[pdx] = 0;
rollback_next_pde:
		/* nothing */;
	}
}

pgindex_t *
init_pgindex(void)
{
	addr_t paddr = pgalloc();
	if (paddr == (addr_t)-1)
		return NULL;

	//kpdebug("init_pgindex: 0x%x, 0x%x\n", (uint32_t)paddr, (uint32_t)PAGE_SIZE);
	pmemset(paddr, 0, (lsize_t)PAGE_SIZE);

	return pa2kva(paddr);
}

void
destroy_pgindex(pgindex_t *pgindex)
{
	pgfree(kva2pa(pgindex));
}

static uint32_t
__pgtable_perm(uint32_t vma_flags)
{
	/*
	 * Some machines (e.g. Loongson 2F) may specify additional flags
	 * (e.g. NOEXEC) to supplement the flags defined by MIPS
	 * standard.
	 *
	 * We will have to rely on machine-specific internal function, so
	 * I prepended the function with double underscores to indicate
	 * that this function should not be called elsewhere.
	 */
	uint32_t flags = PTE_VALID | PTE_GLOBAL | PTE_CACHEABLE |
		((vma_flags & VMA_WRITE) ? PTE_DIRTY : 0);
	return flags;
}

int
map_pages(pgindex_t *pgindex,
	  void *vaddr,
	  addr_t paddr,
	  size_t size,
	  uint32_t flags)
{
	struct pagedesc pd;
	int retcode;
	pte_t *pte;
	addr_t pend = paddr + size;
	addr_t pcur = paddr;
	void *vcur = vaddr;

	if (!IS_ALIGNED(paddr, PAGE_SIZE) ||
	    !IS_ALIGNED(size, PAGE_SIZE) ||
	    !PTR_IS_ALIGNED(vaddr, PAGE_SIZE))
		return -EINVAL;

	/*
	 * 1st pass: allocate leaf page tables if needed, and validate
	 * if there are any conflicts or memory shortage, in either case
	 * we need to rollback.
	 */
	for (; pcur < pend; pcur += PAGE_SIZE, vcur += PAGE_SIZE) {
		retcode = __getpagedesc(pgindex, vcur, true, &pd);
		if (retcode == -ENOMEM)
			goto rollback;

		pte = (pte_t *)pd.ptev;
		if (pte[pd.ptx] != 0) {
			/* we are mapping on the exact same virtual
			 * page which is either valid or invalid (paged
			 * out), fail */
			retcode = -EEXIST;
			goto rollback;
		}
	}

	/* 2nd pass: fill the entries as there shouldn't be any failure */
	pcur = paddr;
	vcur = vaddr;
	for (; pcur < pend; pcur += PAGE_SIZE, vcur += PAGE_SIZE) {
		__getpagedesc(pgindex, vcur, false, &pd);
		pte = (pte_t *)pd.ptev;
		pte[pd.ptx] = (uint32_t)pcur | __pgtable_perm(flags);
	}

	return 0;

rollback:
	__free_intermediate_pgtable(pgindex, vaddr, (size_t)vcur - (size_t)vaddr);
	return retcode;
}

ssize_t
unmap_pages(pgindex_t *pgindex,
	    void *vaddr,
	    size_t size,
	    addr_t *paddr)
{
	void *vcur = vaddr, *vend = vaddr + size;
	ssize_t unmapped_bytes = 0;
	struct pagedesc pd;
	pte_t *pte;
	addr_t pcur = 0;

	for (; vcur < vend; vcur += PAGE_SIZE,
			    unmapped_bytes += PAGE_SIZE,
			    pcur += PAGE_SIZE) {
		if (__getpagedesc(pgindex, vcur, false, &pd) < 0)
			/* may return -ENOENT? */
			panic("unmap_pages non-existent: %p %p\n",
			    pgindex, vcur);
		pte = (pte_t *)pd.ptev;
		if (unmapped_bytes == 0) {
			/* unmapping the first page: store the physical
			 * address */
			pcur = PTE_PADDR(pte[pd.ptx]);
			if (paddr != NULL)
				*paddr = pcur;
		} else if (pte[pd.ptx] != pcur) {
			break;
		}
		pte[pd.ptx] = 0;
	}

	__free_intermediate_pgtable(pgindex, vaddr, (size_t)unmapped_bytes);

	return unmapped_bytes;
}

int
set_pages_perm(pgindex_t *pgindex, void *addr, size_t len, uint32_t flags)
{
	size_t i;
	pte_t *pte;
	struct pagedesc pd;

	for (i = 0; i < len; i += PAGE_SIZE, addr += PAGE_SIZE) {
		if (__getpagedesc(pgindex, addr, false, &pd) < 0)
			/* may return -ENOENT? */
			panic("change_pages_perm non-existent: %p %p\n",
			    pgindex, addr);
		pte = (pte_t *)pd.ptev;
		pte[pd.ptx] &= ~PTE_LOWMASK;
		pte[pd.ptx] |= __pgtable_perm(flags);
	}

	return 0;
}

int
switch_pgindex(pgindex_t *pgindex)
{
	current_pgdir = pgindex;
	tlb_flush();
	return 0;
}

pgindex_t *get_pgindex(void)
{
	return current_pgdir;
}

void *
uva2kva(pgindex_t *pgindex, void *uaddr)
{
	struct pagedesc pd;
	pte_t *pte;

	if (__getpagedesc(pgindex, uaddr, false, &pd) < 0)
		return NULL;
	pte = (pte_t *)pd.ptev;
	return pa2kva(PTE_PADDR(pte[pd.ptx]) | PAGE_OFFSET(uaddr));
}
