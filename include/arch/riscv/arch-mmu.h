// by ZBY

#ifndef _ARCH_MMU_H
#define _ARCH_MMU_H

#define KOFFSET (KERN_BASE - KERN_START)
/* addresses before and after early MMU mapping */
#define __premap_addr(kva)	(ULCAST(kva) - KOFFSET)
#define __postmap_addr(pa)	(ULCAST(pa) + KOFFSET)

#define PAGE_SHIFT 12
#define PAGE_SIZE   4096
#define PAGE_MASK   (PAGE_SIZE - 1)
#define PAGE_OFFSET(a)  (ULCAST(a) & PAGE_MASK)

/* kernel virtual address and physical address conversion */
#define kva2pa(kva)		(ULCAST(kva) - KOFFSET)
#define pa2kva(pa)		(PTRCAST(pa) + KOFFSET)

#ifndef __ASSEMBLER__

#include <sys/types.h>

typedef struct {
    uint64_t V : 1;
    uint64_t R : 1;
    uint64_t W : 1;
    uint64_t X : 1;
    uint64_t U : 1;
    uint64_t G : 1;
    uint64_t A : 1;
    uint64_t D : 1;
    uint64_t RSW : 2;
    uint64_t PPN : 54;
} pte_t;
static_assert(sizeof(pte_t)==8,"invalid pte_t size");
typedef struct {
    pte_t pte[1<<9];
} pgindex_t __attribute__ ((aligned (PAGE_SIZE)));;
static_assert(sizeof(pgindex_t)==PAGE_SIZE, "invalid pgindex_t size");

extern void *kern_end;


#define GIGAPAGE_SIZE (1024ULL*1024*1024)

#endif /* !__ASSEMBLER__ */

#endif /* !_ARCH_MMU_H */

