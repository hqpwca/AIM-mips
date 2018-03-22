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

bool early_mapping_valid(__unused struct early_mapping *entry) { return true; } //DUMMY


void page_index_clear(pgindex_t *index)
{
    memset(index, 0, sizeof(*index));
}

int page_index_early_map(pgindex_t *index, addr_t paddr, void *va, size_t length)
{
    uintptr_t vaddr = (uintptr_t)va;
    
    kprintf("early map vaddr %08x%08x", (unsigned)(vaddr>>32),(unsigned)vaddr);
    kprintf(" paddr %08x%08x\n", (unsigned)(paddr>>32),(unsigned)paddr);
    
    
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
