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
#include <raim/uvm.h>

bool early_mapping_valid(__unused struct early_mapping *entry) { return true; } //DUMMY


void page_index_clear(pgindex_t *index)
{
    memset(index, 0, sizeof(*index));
}

int page_index_early_map(pgindex_t *pgindex, addr_t paddr, void *vaddr, size_t size)
{
    unimpl();
}


void riscv_map_kernel(pgindex_t *pgtable, int map_identity)
{
    addr_t paddr = RAM_PHYSBASE;
    addr_t vaddr = paddr + KOFFSET;
    size_t length = MEM_SIZE;
    //kprintf("mapping kernel to page table %016llx\n", (uint64_t) pgtable);

    assert(vaddr>>38 == (1ULL<<(64-38))-1);
    assert(vaddr % GIGAPAGE_SIZE == 0);
    assert(paddr % GIGAPAGE_SIZE == 0);
    assert(length <= GIGAPAGE_SIZE);//can't handle more than 1GB phys memory
    uint64_t ppn = (paddr/GIGAPAGE_SIZE)&0x1ff;
    uint64_t vpn = (vaddr/GIGAPAGE_SIZE)&0x1ff;
    
    pgtable->pte[vpn] = (pte_t) {
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
    
    // map MMIO device FIXME: ugly
    // 0xffffffff80000000 -> 0x00000000
    // 0xffffffffC0000000 -> 0x40000000
    pgtable->pte[(0xffffffff80000000/GIGAPAGE_SIZE)&0x1ff] = (pte_t) {
        .V = 1,
        .R = 1,
        .W = 1,
        .X = 1,
        .U = 0,
        .G = 0,
        .A = 1,
        .D = 1,
        .RSW = 0,
        .PPN = 0x00000000 >> 12,
    };
    pgtable->pte[(0xffffffffC0000000/GIGAPAGE_SIZE)&0x1ff] = (pte_t) {
        .V = 1,
        .R = 1,
        .W = 1,
        .X = 1,
        .U = 0,
        .G = 0,
        .A = 1,
        .D = 1,
        .RSW = 0,
        .PPN = 0x40000000 >> 12,
    };
    
    
    
    if (map_identity) {
        pgtable->pte[ppn] = pgtable->pte[vpn];
    }
}



static void riscv_switch_pgtable(pgindex_t *pgtable)
{
    uint64_t pgindex_paddr = (uintptr_t) kva2pa(pgtable);
    union {
        struct {
            uint64_t PPN : 44;
            uint64_t ASID : 16;
            uint64_t MODE : 4;
        };
        uint64_t val;
    } new_satp = {

        .PPN = pgindex_paddr >> 12,
        .ASID = 0,
        .MODE = 8, // Sv39

    };
    
    __asm__ __volatile__ ("sfence.vma":::"memory");
    __asm__ __volatile__ ("csrw satp, %0"::"r"(new_satp.val):"memory");

}
void mmu_init(pgindex_t *boot_page_index)
{
//    early_mapping_add_memory();

    // WARL. Write-Any Read-Legal 
    
    // enable SUM bit
    uint32_t sstatus;
    __asm__ __volatile__ ("csrr %0, sstatus":"=r"(sstatus));
    sstatus |= 1<<18;
    __asm__ __volatile__ ("csrw sstatus, %0"::"r"(sstatus));
    
    kprintf("boot page table located at %016llX\n", (uint64_t)boot_page_index);
    
    riscv_switch_pgtable(pa2kva(boot_page_index));
    
    kprintf("paging enabled!\n");
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
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



static pgindex_t *current_pgdir;


int switch_pgindex(pgindex_t *pgindex)
{
	current_pgdir = pgindex;
    riscv_switch_pgtable(pgindex);
	return 0;
}

pgindex_t *get_pgindex(void)
{
	return current_pgdir;
}



pgindex_t *init_pgindex(void)
{
    assert(sizeof(pgindex_t)==PAGE_SIZE);
	addr_t paddr = pgalloc();
	if (paddr == (addr_t)-1)
		return NULL;
	pgindex_t *ret = (pgindex_t *) pa2kva(paddr);
	
	page_index_clear(ret);
	riscv_map_kernel(ret, 0);
	
    kprintf("init_pgindex: %p\n",ret);
	return ret;
}




// make a riscv pte
void riscv_pte(pte_t *pte, addr_t vaddr, addr_t paddr, uint32_t flags)
{
    *pte = (pte_t) {
        .V = 1,
        .R = !!(flags & VMA_READ),
        .W = !!(flags & VMA_WRITE),
        .X = !!(flags & VMA_EXEC),
        .U = !(vaddr >> 63),
        .G = 0,
        .A = 1,
        .D = 1,
        .RSW = 0,
        .PPN = paddr >> 12,
    };
    // if RWX is both zero, must mark it invalid
    // or RISCV will recognize it as a pointer to next level page table
    if ((flags & VMA_RWX) == 0) {
        pte->V = 0;
    }
}

// 2**12        =2**12=4KB    LEVEL0
// 2**(12+9)    =2**21=2MB    LEVEL1
// 2**(12+9+9)  =2**30=1GB    LEVEL2
// 2**(12+9+9+9)=2**39=512GB  total
void riscv_map_pages(pgindex_t *pgindex, addr_t vaddr, addr_t paddr, size_t size, uint32_t flags, int level)
{
    kprintf("map: pgindex=%016llx vaddr=%016llx paddr=%016llx size=%016llx flags=%x level=%d\n", (uint64_t)pgindex, vaddr, paddr, size, flags, level);
    assert(vaddr>>38==(1ULL<<(64-38))-1 || vaddr>>38==0);
    assert(IS_ALIGNED(vaddr, PAGE_SIZE));
    assert(IS_ALIGNED(paddr, PAGE_SIZE));
    assert(size<=1ULL<<((level+1)*9+12));

    uint64_t pgshift = level*9+12;
    uint64_t pgsize = 1<<pgshift;
    
    while (1) {
        uint64_t lvlvpn = (vaddr>>pgshift)&0x1ff; // vpn of current level
        pte_t *pte = &pgindex->pte[lvlvpn];
        
        if (level > 0) {
            addr_t nexttbl = pte->PPN<<12; // physical address of next level page table
            if (nexttbl == 0) { // need alloc
                nexttbl = pgalloc();
                memset(pa2kva(nexttbl),0,sizeof(pgindex_t));
                *pte = (pte_t) {
                    .V = 1,
                    .R = 0,
                    .W = 0,
                    .X = 0,
                    .U = 0,
                    .G = 0,
                    .A = 0,
                    .D = 0,
                    .RSW = 0,
                    .PPN = nexttbl >> 12,
                };
            }
            riscv_map_pages(pa2kva(nexttbl), vaddr, paddr, min2(size,pgsize), flags, level - 1);
        } else {
            riscv_pte(pte, vaddr, paddr, flags);
        }
        
        if (size <= pgsize) break;
        vaddr += pgsize;
        paddr += pgsize;
        size -= pgsize;
    }
}

size_t riscv_unmap_pages(pgindex_t *pgindex, addr_t vaddr, size_t size, addr_t *ppaddr, int level)
{
    kprintf("unmap: pgindex=%016llx vaddr=%016llx size=%016llx ppaddr=%016llx level=%d\n", (uint64_t)pgindex, vaddr, size, (uint64_t)ppaddr, level);
    
    assert(vaddr>>38==(1ULL<<(64-38))-1 || vaddr>>38==0);
    assert(IS_ALIGNED(vaddr, PAGE_SIZE));
    assert(size<=1ULL<<((level+1)*9+12));

    uint64_t pgshift = level*9+12;
    uint64_t pgsize = 1<<pgshift;
    
    size_t ret = 0;
    
    while (1) {
        uint64_t lvlvpn = (vaddr>>pgshift)&0x1ff; // vpn of current level
        pte_t *pte = &pgindex->pte[lvlvpn];
        
        if (level > 0) {
            addr_t nexttbl = pte->PPN<<12; // physical address of next level page table
            if (nexttbl != 0) {
                ret += riscv_unmap_pages(pa2kva(nexttbl), vaddr, min2(size,pgsize), ppaddr, level - 1);
                // FIXME: free next level page table if all pages are unmapped
            }
        } else {
            if (ppaddr && *ppaddr == 0) *ppaddr = pte->PPN<<12;
            ret += pgsize;
            pte->raw = 0;
        }
        
        if (size <= pgsize) break;
        vaddr += pgsize;
        size -= pgsize;
    }
    return ret;
}

void riscv_destroy_pgindex(pgindex_t *pgindex, int level)
{
    kprintf("destroy: pgindex=%016llx level=%d\n", (uint64_t)pgindex, level);
    
    for (int i = 0; i < (level==2?(1<<8):(1<<9)); i++) {
        if (level > 0) {
            addr_t nexttbl = pgindex->pte[i].PPN<<12;
            if (nexttbl) {
                riscv_destroy_pgindex(pa2kva(nexttbl), level - 1);
            }
        } else {
            assert(pgindex->pte[i].V == 0);
        }
    }
}

void *riscv_uva2kva(pgindex_t *pgindex, addr_t uaddr, int level)
{
    kprintf("uva2kva pgindex=%016llx uaddr=%016llx level=%d\n", (uint64_t)pgindex, uaddr, level);
    uint64_t pgshift = level*9+12;
    uint64_t lvlvpn = (uaddr>>pgshift)&0x1ff;
    pte_t *pte = &pgindex->pte[lvlvpn];
    if (!pte->V) return NULL;
    void *kva = pa2kva(pte->PPN<<12);
    if (level > 0) {
        return riscv_uva2kva(kva, uaddr, level-1);
    } else {
        return kva + (uaddr&0xfff);
    }
}

/// wrappers

ssize_t unmap_pages(pgindex_t *pgindex, void *vaddr, size_t size, addr_t *paddr)
{
    if (paddr) *paddr = 0;
    size_t ret = riscv_unmap_pages(pgindex, (addr_t)vaddr, size, paddr, 2);
    kprintf(" unmap() = %016llx\n", ret);
    return ret;
}

int map_pages(pgindex_t *pgindex, void *vaddr, addr_t paddr, size_t size, uint32_t flags)
{
    riscv_map_pages(pgindex, (addr_t)vaddr, paddr, size, flags, 2);
    kprintf(" map() ok\n");
    return 0;
}


void destroy_pgindex(pgindex_t *pgindex)
{
    riscv_destroy_pgindex(pgindex, 2);
    kprintf(" destroy() ok\n");
}

int set_pages_perm(pgindex_t *pgindex, void *addr, size_t len, uint32_t flags)
{
unimpl();
}

void *uva2kva(pgindex_t *pgindex, void *uaddr)
{
    void *ret = riscv_uva2kva(pgindex, (addr_t)uaddr, 2);
    kprintf(" uva2kva() = %016llx\n", (uint64_t)ret);
    return ret;
}

