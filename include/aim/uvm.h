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

#ifndef _UVM_H
#define _UVM_H

#include <aim/mmu.h>
#include <aim/pmm.h>
//#include <aim/sync.h>
#include <atomic.h>
#include <list.h>

#ifndef __ASSEMBLER__

struct mm {
	struct list_head vma_head;	/* virtual memory area list sentry */
	size_t		vma_count;	/* number of virtual memory areas */
	size_t		ref_count;	/* reference count (may be unused) */
	pgindex_t	*pgindex;	/* pointer to page index */
	//lock_t		lock;
};

/*
 * Architecture-independent interfaces
 * Address must be page-aligned.
 */
/* Create a size @len user space mapping starting at virtual address @addr */
int create_uvm(struct mm *mm, void *addr, size_t len, uint32_t flags);
/* Destroy a size @len user space mapping starting at @addr */
int destroy_uvm(struct mm *mm, void *addr, size_t len);
/* Create a size @len user space mapping which shares same physical pages
 * under @addr_src in struct mm @src, preferably at @addr_dst.
 * If @addr_dst == NULL, the routine chooses where to map.
 * Returns the virtual address we are mapping to. */
void *share_uvm(struct mm *dst, const void *addr_dst, struct mm *src,
    const void *addr_src, size_t len);
int set_uvm_perm(struct mm *mm, void *addr, size_t len, uint32_t flags);

/*
 * Architecture-independent interfaces
 * Address need not be page-aligned.
 */
/* Copy from address @vaddr in current memory mapping (including kernel's) to
 * user space at @uvaddr */
int copy_to_uvm(struct mm *mm, void *uvaddr, void *vaddr, size_t len);
/* Does the reverse */
int copy_from_uvm(struct mm *mm, void *uvaddr, void *vaddr, size_t len);
/* Fill a user space memory region with given byte */
int fill_uvm(struct mm *mm, void *uvaddr, unsigned char c, size_t len);

/* Create a struct mm with a new page index and _kernel mappings inserted_. */
struct mm *mm_new(void);
/* Destroy a struct mm and all the underlying memory mappings */
void mm_destroy(struct mm *mm);
/* Clone a memory mapping as a whole.  @dst should be initialized via mm_new */
int mm_clone(struct mm *dst, struct mm *src);

/*
 * The following are data structures which other models usually don't care
 * about.
 *
 * I (Gan) would rather put them into a separate header...
 */

/*
 * User page structure based on struct pages but including additional
 * members such as reference counts.
 */
struct upages {
	struct pages;
	//lock_t lock;
	atomic_t refs;	/* for shared memory */
	/*
	 * When swapping pages in and out, we normally save the page content
	 * and virtual page number (along with an identification of page
	 * index - could be PID if a page directory is _uniquely_ related
	 * to a process) together.  But if we are allowing shared memory,
	 * we need to save a _list_ of virtual page numbers (as well as page
	 * index identifications) since two virtual pages from different page
	 * indexes can be mapped to a same physical page, and if we are
	 * swapping such page out, we need to invalidate both virtual page
	 * entries.  To enable us finding all the virtual pages easily, we
	 * need to keep track of the list of virtual pages mapped to
	 * the same physical page (or page block).
	 *
	 * Here we are saving the list of such virtual pages inside the
	 * pages struct.  They are a list of struct vma (below).
	 */
	struct list_head vma_head;
};

/*
 * Virtual memory area structure
 *
 * The list of virtual memory areas must satisfy:
 * 1. that the virtual memory areas should be sorted in ascending order of
 *    virtual address, AND
 * 2. that each virtual memory area should be mapped to exactly one
 *    contiguous page block (stored in struct upages) in a one-to-one manner,
 *    AND
 * 3. that the virtual memory areas should never overlap.
 */
struct vma {
	/* Must be page-aligned */
	void		*start;
	size_t		size;

	uint32_t	flags;
	/* These flags match ELF segment flags */
#define VMA_EXEC	0x01
#define VMA_WRITE	0x02
#define VMA_READ	0x04
#define VMA_RW		(VMA_READ | VMA_WRITE)
#define VMA_RWX		(VMA_RW | VMA_EXEC)
#define VMA_RX		(VMA_READ | VMA_EXEC)
	/* More flags */
#define VMA_FILE	0x100		/* For mmap(2) */
	struct mm	*mm;
	/* Since we are not maintaining a list for all physical pages, we
	 * have to keep a struct upages pointer with struct vma in case of
	 * shared memory. */
	struct upages	*pages;
	/* A list of VMAs in the same memory mapping structure */
	struct list_head node;
	/* A list of VMAs sharing the same physical pages.  We don't require
	 * ordering for this list. */
	struct list_head share_node;
};

/* All kernel processes share this memory mapping structure. */
extern struct mm *kernel_mm;
//extern rlock_t memlock;

#endif /* !__ASSEMBLER__ */

#endif /* _UVM_H */

