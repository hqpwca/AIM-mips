// by ZBY
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */


#include <sys/types.h>
#include <raim/mmu.h>
#include <raim/early_kmmap.h>
#include <libc/string.h>
#include <raim/debug.h>
#include <raim/console.h>
#include <raim/vmm.h>
#include <raim/pmm.h>

bool early_mapping_valid(__unused struct early_mapping *entry) { return true; } //DUMMY


void page_index_clear(pgindex_t *index)
{
    memset(index, 0, sizeof(*index));
}

int page_index_early_map(pgindex_t *index, addr_t paddr, void *va, size_t length)
{
    uintptr_t vaddr = (uintptr_t)va;
    
    kprintf("early map vaddr %016llX paddr %016llX\n", vaddr, paddr);
    
    
    assert(vaddr>>37 == (1ULL<<(64-37))-1);
    assert(vaddr % GIGAPAGE_SIZE == 0);
    assert(paddr % GIGAPAGE_SIZE == 0);
    assert(length <= GIGAPAGE_SIZE);
    uint64_t ppn = (paddr/GIGAPAGE_SIZE)&0x1ff;
    uint64_t vpn = (vaddr/GIGAPAGE_SIZE)&0x1ff;
    
    //kprintf("%x %x\n", (uint32_t)ppn, (uint32_t)vpn);
    index->pte[ppn] = index->pte[vpn] = (pte_t) {
        .V = 1,
        .R = 1,
        .W = 1,
        .X = 1,
        .U = 0,
        .G = 0,
        .A = 1,
        .D = 1,
        .RSW = 0,
        .PPN = paddr >> 12,
    };
    
	return 1;
}

void init_free_pages()
{
	size_t kend = kva2pa(&kern_end);
	kend = ALIGN_ABOVE(kend, PAGE_SIZE);
	size_t mend = RAM_PHYSBASE + MEM_SIZE; // memory end
	kprintf("Start Freeing: 0x%x ~ 0x%x...\n", kend, mend);

	struct pages *p = kmalloc(sizeof(*p), 0);
	p->paddr = kend;
	p->size = mend - kend;
	p->flags = GFP_UNSAFE;

	free_pages(p);

	kprintf("Finish Freed: 0x%x ~ 0x%x...\n", kend, mend);
}


/////////////////////////////////////////////////////////////////////////////


static pgindex_t *current_pgdir;


int switch_pgindex(pgindex_t *pgindex)
{
	current_pgdir = pgindex;
unimpl();
	return 0;
}

pgindex_t *get_pgindex(void)
{
	return current_pgdir;
}



pgindex_t *init_pgindex(void)
{
unimpl();
}


ssize_t unmap_pages(pgindex_t *pgindex,
	    void *vaddr,
	    size_t size,
	    addr_t *paddr)
{
unimpl();
}

int map_pages(pgindex_t *pgindex,
	  void *vaddr,
	  addr_t paddr,
	  size_t size,
	  uint32_t flags)
{
unimpl();
}


void destroy_pgindex(pgindex_t *pgindex)
{
unimpl();
}

int set_pages_perm(pgindex_t *pgindex, void *addr, size_t len, uint32_t flags)
{
unimpl();
}

void *uva2kva(pgindex_t *pgindex, void *uaddr)
{
unimpl();
}

