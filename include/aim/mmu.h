/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 * Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
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

#ifndef _AIM_MMU_H
#define _AIM_MMU_H

#include <arch-mmu.h>
#include <util.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE	4096
#endif /* !PAGE_SIZE */

#ifndef EARLY_PAGE_SIZE
#define EARLY_PAGE_SIZE	PAGE_SIZE
#endif /* !EARLY_PAGE_SIZE */

#ifndef __ASSEMBLER__

/* premap_addr: always returns low address.
 * The function which assumes that the argument is a high address
 * becomes __premap_addr(). */
#define premap_addr(a)	({ \
	size_t i = (size_t)(a); \
	(i >= KERN_BASE) ? __premap_addr(i) : i; \
})

/* postmap_addr: always returns high address.
 * The function which assumes that the argument is a low address
 * becomes __postmap_addr(). */
#define postmap_addr(a)	({ \
	size_t i = (size_t)(a); \
	(i >= KERN_BASE) ? (i) : __postmap_addr(i); \
})

// Control Register flags
#define CR0_PE          0x00000001      // Protection Enable
#define CR0_MP          0x00000002      // Monitor coProcessor
#define CR0_EM          0x00000004      // Emulation
#define CR0_TS          0x00000008      // Task Switched
#define CR0_ET          0x00000010      // Extension Type
#define CR0_NE          0x00000020      // Numeric Errror
#define CR0_WP          0x00010000      // Write Protect
#define CR0_AM          0x00040000      // Alignment Mask
#define CR0_NW          0x20000000      // Not Writethrough
#define CR0_CD          0x40000000      // Cache Disable
#define CR0_PG          0x80000000      // Paging

#define CR4_PSE         0x00000010      // Page size extension

// various segment selectors.
#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_KCPU  3  // kernel per-cpu data
#define SEG_UCODE 4  // user code
#define SEG_UDATA 5  // user data+stack
#define SEG_TSS   6  // this process's task state

// cpu->gdt[NSEGS] holds the above segments.
#define NSEGS     7

struct segdesc {
  addr_t lim_15_0 : 16;  // Low bits of segment limit
  addr_t base_15_0 : 16; // Low bits of segment base address
  addr_t base_23_16 : 8; // Middle bits of segment base address
  addr_t type : 4;       // Segment type (see STS_ constants)
  addr_t s : 1;          // 0 = system, 1 = application
  addr_t dpl : 2;        // Descriptor Privilege Level
  addr_t p : 1;          // Present
  addr_t lim_19_16 : 4;  // High bits of segment limit
  addr_t avl : 1;        // Unused (available for software use)
  addr_t rsv1 : 1;       // Reserved
  addr_t db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
  addr_t g : 1;          // Granularity: limit scaled by 4K when set
  addr_t base_31_24 : 8; // High bits of segment base address
};

#define SEG(type, base, lim, dpl) (struct segdesc)    \
{ ((lim) >> 12) & 0xffff, (addr_t)(base) & 0xffff,      \
  ((addr_t)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (addr_t)(lim) >> 28, 0, 0, 1, 1, (addr_t)(base) >> 24 }
#define SEG16(type, base, lim, dpl) (struct segdesc)  \
{ (lim) & 0xffff, (addr_t)(base) & 0xffff,              \
  ((addr_t)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (addr_t)(lim) >> 16, 0, 0, 1, 0, (addr_t)(base) >> 24 }

#define DPL_USER    0x3     // User DPL

// Application segment type bits
#define STA_X       0x8     // Executable segment
#define STA_E       0x4     // Expand down (non-executable segments)
#define STA_C       0x4     // Conforming code segment (executable only)
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)
#define STA_A       0x1     // Accessed

// System segment type bits
#define STS_T16A    0x1     // Available 16-bit TSS
#define STS_LDT     0x2     // Local Descriptor Table
#define STS_T16B    0x3     // Busy 16-bit TSS
#define STS_CG16    0x4     // 16-bit Call Gate
#define STS_TG      0x5     // Task Gate / Coum Transmitions
#define STS_IG16    0x6     // 16-bit Interrupt Gate
#define STS_TG16    0x7     // 16-bit Trap Gate
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_T32B    0xB     // Busy 32-bit TSS
#define STS_CG32    0xC     // 32-bit Call Gate
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

#define PDX(va)         (((uint32_t)(va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va)         (((uint32_t)(va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint32_t)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

#define PGSHIFT         12      // log2(PGSIZE)
#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        22      // offset of PDX in a linear address

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PWT         0x008   // Write-Through
#define PTE_PCD         0x010   // Cache-Disable
#define PTE_A           0x020   // Accessed
#define PTE_D           0x040   // Dirty
#define PTE_PS          0x080   // Page Size
#define PTE_MBZ         0x180   // Bits must be zero

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xFFF)
#define PTE_FLAGS(pte)  ((uint32_t)(pte) &  0xFFF)

#define DPL_KERNEL 0x0
#define DPL_USER 0x3

extern pgindex_t *pgindex;

extern void *kern_end;

static inline void
lgdt(struct segdesc *p, int size)
{
  volatile unsigned short pd[3];

  pd[0] = size - 1;
  pd[1] = (unsigned)p;
  pd[2] = (unsigned)p >> 16;

  asm volatile("lgdt (%0)" : : "r" (pd));
}

addr_t get_mem_physbase();
addr_t get_mem_size();

void page_index_clear(pgindex_t *boot_page_index);
int page_index_early_map(pgindex_t *boot_page_index, addr_t paddr,
	void *vaddr, size_t size);
int page_index_init(pgindex_t *boot_page_index);
void mmu_init(pgindex_t *boot_page_index);

void early_mm_init(void);	/* arch-specific */

void mm_init(void);
void arch_mm_init(void);	/* arch-specific */

/* Clear all MMU init callback handlers */
void mmu_handlers_clear(void);
/* Add one MMU init callback handler, which will be called *after*
 * initializing MMU*/
int mmu_handlers_add(generic_fp entry);
void mmu_handlers_apply(void);

/* Likewise */
void jump_handlers_clear(void);
int jump_handlers_add(generic_fp entry);
void jump_handlers_apply(void);

/*
 * This routine jumps to an absolute address, regardless of MMU and page index
 * state.
 * by jumping to some address, callers acknowledge that C runtime components
 * like stack are not preserved, and no return-like operation will be performed.
 */
__noreturn
void abs_jump(void *addr);

/*
 * Architecture-specific interfaces
 * All addresses should be page-aligned.
 * Note that these interfaces are independent of struct mm and struct vma.
 * Also, these interfaces assume that the page index is exclusively owned.
 */
/* Initialize a page index table and fill in the structure @pgindex */
pgindex_t *init_pgindex(void);
/* Destroy the page index table itself assuming that everything underlying is
 * already done with */
void destroy_pgindex(pgindex_t *pgindex);
/* Map virtual address starting at @vaddr to physical pages at @paddr, with
 * VMA flags @flags (VMA_READ, etc.) Additional flags apply as in kmmap.h.
 */
int map_pages(pgindex_t *pgindex, void *vaddr, addr_t paddr, size_t size,
    uint32_t flags);
/*
 * Unmap but do not free the physical frames
 * Returns the size of unmapped physical memory (may not equal to @size), or
 * negative for error.
 * The physical address of unmapped pages are stored in @paddr.
 */
ssize_t unmap_pages(pgindex_t *pgindex, void *vaddr, size_t size, addr_t *paddr);
/*
 * Change the page permission flags without changing the actual mapping.
 */
int set_pages_perm(pgindex_t *pgindex, void *vaddr, size_t size, uint32_t flags);
/*
 * Invalidate pages, but without actually deleting the page index entries, 
 * if possible.
 */
ssize_t invalidate_pages(pgindex_t *pgindex, void *vaddr, size_t size,
    addr_t *paddr);
/* Switch page index to the given one */
int switch_pgindex(pgindex_t *pgindex);
/* Get the currently loaded page index structure */
pgindex_t *get_pgindex(void);
/* Trace a page index to convert from user address to kernel address */
void *uva2kva(pgindex_t *pgindex, void *uaddr);

#endif /* !__ASSEMBLER__ */

#endif /* _AIM_MMU_H */

