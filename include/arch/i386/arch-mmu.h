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

#ifndef _ARCH_MMU_H
#define _ARCH_MMU_H

/* addresses before and after early MMU mapping */
#define __premap_addr(kva)	(ULCAST(kva) - KERN_BASE)
#define __postmap_addr(pa)	(ULCAST(pa) + KERN_BASE)

/* kernel virtual address and physical address conversion */
#define kva2pa(kva)		(ULCAST(kva) - KERN_BASE)
#define pa2kva(pa)		(PTRCAST(pa) + KERN_BASE)

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

#define PGSIZE2 0x400000

#define PGROUNDUP2(sz)  (((sz)+PGSIZE2-1) & ~(PGSIZE2-1))
#define PGROUNDDOWN2(a) (((a)) & ~(PGSIZE2-1))

#ifndef __ASSEMBLER__

typedef uint32_t	pde_t;
typedef uint32_t	pte_t;

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
/*
typedef union {
	struct
	{
		uint32_t pfaddr:20;
		uint8_t avail:3;
		uint8_t zero1:2;
		uint8_t dirty:1;
		uint8_t a:1;
		uint8_t zero2:2;
		uint8_t user_super:1;
		uint8_t read_write:1;
		uint8_t present:1;
	};
	
	pde_t value;
}pgindex_t;
*/

typedef pde_t	pgindex_t;

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

#endif /* !__ASSEMBLER__ */

#endif /* !_ARCH_MMU_H */

