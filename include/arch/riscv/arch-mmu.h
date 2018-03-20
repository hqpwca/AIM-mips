// by ZBY

#ifndef _ARCH_MMU_H
#define _ARCH_MMU_H

/* addresses before and after early MMU mapping */
#define __premap_addr(kva)	(ULCAST(kva) - KERN_BASE)
#define __postmap_addr(pa)	(ULCAST(pa) + KERN_BASE)

#define PAGE_SHIFT 12
#define PAGE_SIZE   4096
#define PAGE_MASK   (PAGE_SIZE - 1)
#define PAGE_OFFSET(a)  (ULCAST(a) & PAGE_MASK)

/* kernel virtual address and physical address conversion */
#define kva2pa(kva)		(ULCAST(kva) - KERN_BASE)
#define pa2kva(pa)		(PTRCAST(pa) + KERN_BASE)

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
typedef struct {
    pte_t pte[1<<9];
} pgindex_t;

#endif /* !__ASSEMBLER__ */

#endif /* !_ARCH_MMU_H */

