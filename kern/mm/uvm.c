/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <libc/string.h>
#include <aim/debug.h>
#include <aim/mmu.h>
#include <aim/panic.h>
#include <aim/percpu.h>	/* current_proc */
#include <aim/sync.h>
#include <aim/vmm.h>
#include <atomic.h>
#include <errno.h>

struct mm *kernel_mm;
//rlock_t memlock = EMPTY_RLOCK(memlock);

struct mm *
mm_new(void)
{
	struct mm *mm = (struct mm *)kmalloc(sizeof(*mm), 0);

	if (mm != NULL) {
		list_init(&(mm->vma_head));
		mm->vma_count = 0;
		//spinlock_init(&(mm->lock));
		if ((mm->pgindex = init_pgindex()) == NULL) {
			kfree(mm);
			return NULL;
		}
		//kmmap_apply(mm->pgindex);
	}

	return mm;
}

/* Unmap the virtual memory area */
static void
__clean_vma(struct mm *mm, struct vma *vma)
{
	/* all assertions here are temporary */
	ssize_t unmapped;
	addr_t pa;

	assert(vma->size == vma->pages->size);

	unmapped = unmap_pages(mm->pgindex, vma->start, vma->size, &pa);

	assert(pa == vma->pages->paddr);
	assert(unmapped == vma->size);
}

static void
__ref_upages(struct upages *p)
{
	atomic_inc(&(p->refs));
}

#define __PAGES_FREED	1
static int
__unref_and_free_upages(struct upages *p)
{
	atomic_dec(&(p->refs));
	if (p->refs == 0) {
		free_pages(p);
		return __PAGES_FREED;
	}
	return 0;
	
}

void
mm_destroy(struct mm *mm)
{
	struct vma *vma, *vma_next;

	if (mm == NULL || mm == kernel_mm)	/* ignore kernel mm */
		return;

	for_each_entry_safe (vma, vma_next, &(mm->vma_head), node) {
		__clean_vma(mm, vma);
		if (__unref_and_free_upages(vma->pages) == __PAGES_FREED)
			kfree(vma->pages);
		kfree(vma);
	}

	destroy_pgindex(mm->pgindex);

	kfree(mm);
}

/* Assumes that mm lock is held */
static struct vma *
__find_vma_before(struct mm *mm, void *addr, size_t size)
{
	struct vma *vma, *vma_prev;

	if (list_empty(&(mm->vma_head)))
		return list_entry(&(mm->vma_head), struct vma, node);

	/* otherwise */
	for_each_entry (vma, &(mm->vma_head), node) {
		if (vma->start >= addr + size)
			break;
	}

	vma_prev = prev_entry(vma, node);
	if (addr < vma_prev->start + vma_prev->size)
		/* overlap detected */
		return NULL;

	return vma_prev;
}

/* Assumes that mm lock is held */
static struct vma *
__find_continuous_vma(struct mm *mm, void *addr, size_t len)
{
	struct vma *vma, *vma_start;
	size_t i = 0;

	assert(PTR_IS_ALIGNED(addr, PAGE_SIZE));
	assert(IS_ALIGNED(len, PAGE_SIZE));

	/* find the vma */
	for_each_entry (vma, &(mm->vma_head), node) {
		if (vma->start == addr)
			break;
	}
	/* not found? */
	if (&(vma->node) == &(mm->vma_head))
		return NULL;

	vma_start = vma;
	i += PAGE_SIZE;
	for (; i < len; i += PAGE_SIZE) {
		vma = next_entry(vma, node);
		if (vma->start != addr + i)
			/* requested region contain unmapped virtual page */
			return NULL;
	}

	return vma_start;
}

/* Unmapping takes place _at_ @vma_start */
static void
__unmap_and_free_vma(struct mm *mm, struct vma *vma_start, size_t size)
{
	struct vma *vma_cur = vma_start;
	size_t vma_size = 0;
	//unsigned long intr;
	for (size_t i = 0; i < size; i += vma_size) {
		struct vma *vma = vma_cur;
		vma_cur = next_entry(vma_cur, node);

		vma_size = vma->size;
		list_del(&(vma->node));

		//spin_lock_irq_save(&(vma->pages->lock), intr);
		list_del(&(vma->share_node));
		//spin_unlock_irq_restore(&(vma->pages->lock), intr);

		/* temporary in case of typo - assertion will be removed */
		assert(unmap_pages(mm->pgindex, vma->start, vma->size,
		    NULL) == vma_size);
		if (__unref_and_free_upages(vma->pages) == __PAGES_FREED)
			kfree(vma->pages);
		kfree(vma);
	}
}

static struct vma *
__vma_new(struct mm *mm, void *addr, size_t size, uint32_t flags)
{
	struct vma *vma = (struct vma *)kmalloc(sizeof(*vma), 0);

	if (vma != NULL) {
		vma->mm = mm;
		vma->start = addr;
		vma->size = size;
		vma->flags = flags;
		vma->pages = NULL;
		list_init(&(vma->node));
		list_init(&(vma->share_node));
	}

	return vma;
}

static struct upages *
__upages_new(size_t size, uint32_t flags)
{
	struct upages *p = (struct upages *)kmalloc(sizeof(*p), 0);
	if (p != NULL) {
		p->paddr = 0;
		p->flags = flags;
		p->size = size;
		p->refs = 0;
		//spinlock_init(&(p->lock));
		list_init(&(p->vma_head));
	}
	return p;
}

int
create_uvm(struct mm *mm, void *addr, size_t len, uint32_t flags)
{
	int retcode = 0;
	struct vma *vma_start, *vma, *vma_cur;
	struct upages *p;
	void *vcur = addr;
	size_t mapped = 0;
	//unsigned long intr;

	if (!IS_ALIGNED(len, PAGE_SIZE) ||
	    mm == NULL ||
	    !PTR_IS_ALIGNED(addr, PAGE_SIZE))
		return -EINVAL;

	if (len == 0)
		return 0;

	//spin_lock(&(mm->lock));

	if (!(vma_start = __find_vma_before(mm, addr, len))) {
		retcode = -EFAULT;
		goto finalize;
	}

	vma_cur = vma_start;
	for (; mapped < len; mapped += PAGE_SIZE, vcur += PAGE_SIZE) {
		vma = __vma_new(mm, vcur, PAGE_SIZE, flags);
		if (vma == NULL) {
			retcode = -ENOMEM;
			goto rollback;
		}

		p = __upages_new(PAGE_SIZE, 0);
		if (p == NULL) {
			retcode = -ENOMEM;
			goto rollback_vma;
		}
		if (alloc_pages(p) < 0) {
			retcode = -ENOMEM;
			goto rollback_pages;
		}

		if ((retcode = map_pages(mm->pgindex, vcur, p->paddr,
		    PAGE_SIZE, flags)) < 0) {
			goto rollback_pgalloc;
		}

		vma->pages = p;
		__ref_upages(p);
		list_add_after(&(vma->node), &(vma_cur->node));

		//spin_lock_irq_save(&(p->lock), intr);
		list_add_after(&(vma->share_node), &(p->vma_head));
		//spin_unlock_irq_restore(&(p->lock), intr);

		vma_cur = vma;
		continue;

rollback_pgalloc:
		free_pages(p);
rollback_pages:
		kfree(p);
rollback_vma:
		kfree(vma);
		goto rollback;
	}

	retcode = 0;
	goto finalize;

rollback:
	__unmap_and_free_vma(mm, next_entry(vma_start, node), mapped);

finalize:
	//spin_unlock(&(mm->lock));
	return retcode;
}

int
destroy_uvm(struct mm *mm, void *addr, size_t len)
{
	struct vma *vma;
	int retcode;

	if (!IS_ALIGNED(len, PAGE_SIZE) ||
	    mm == NULL ||
	    !PTR_IS_ALIGNED(addr, PAGE_SIZE))
		return -EINVAL;

	//spin_lock(&(mm->lock));

	vma = __find_continuous_vma(mm, addr, len);
	if (vma == NULL) {
		retcode = -EFAULT;
		goto finalize;
	}

	__unmap_and_free_vma(mm, vma, len);
	retcode = 0;

finalize:
	//spin_unlock(&(mm->lock));
	return retcode;
}

int
set_uvm_perm(struct mm *mm, void *addr, size_t len, uint32_t flags)
{
	struct vma *vma;
	size_t i = 0;
	int retcode;

	if (!IS_ALIGNED(len, PAGE_SIZE) ||
	    mm == NULL ||
	    !PTR_IS_ALIGNED(addr, PAGE_SIZE))
		return -EINVAL;

	//spin_lock(&(mm->lock));

	vma = __find_continuous_vma(mm, addr, len);
	if (vma == NULL) {
		retcode = -EFAULT;
		goto finalize;
	}

	for (; i < len; i += PAGE_SIZE, addr += PAGE_SIZE) {
		vma->flags = flags;
		if (set_pages_perm(mm->pgindex, addr, PAGE_SIZE,
		    flags) != 0) {
			retcode = -ENOENT;
			goto finalize;
		}
	}

	retcode = 0;

finalize:
	//spin_unlock(&(mm->lock));
	return retcode;
}

/* Dumb copy implementation */
int
mm_clone(struct mm *dst, struct mm *src)
{
	struct vma *vma_new, *vma_cur, *vma_start, *vma;
	struct upages *p;
	size_t cloned_size = 0;
	int retcode = 0;
	//unsigned long intr;

	vma_start = list_entry(&(dst->vma_head), struct vma, node);
	vma_cur = vma_start;

	//spin_lock(&(dst->lock));
	//spin_lock(&(src->lock));

	for_each_entry (vma, &(src->vma_head), node) {
		/* TODO: I wonder if I should reuse the code in that of
		 * create_uvm() */
		vma_new = __vma_new(dst, vma->start, vma->size, vma->flags);
		if (vma_new == NULL) {
			retcode = -ENOMEM;
			goto rollback;
		}

		p = __upages_new(vma->pages->size, vma->pages->flags);
		if (p == NULL) {
			retcode = -ENOMEM;
			goto rollback_vma;
		}

		if (alloc_pages(p) < 0) {
			retcode = -ENOMEM;
			goto rollback_pages;
		}

		if ((retcode = map_pages(dst->pgindex, vma_new->start,
		    p->paddr, vma_new->size, vma_new->flags)) < 0) {
			goto rollback_pgalloc;
		}

		memmove(
			(void *)(size_t)pa2kva(p->paddr), 
			(void *)(size_t)pa2kva(vma->pages->paddr),
			vma_new->size
		);

		vma_new->pages = p;
		__ref_upages(p);
		list_add_after(&(vma_new->node), &(vma_cur->node));

		//spin_lock_irq_save(&(p->lock), intr);
		list_add_after(&(vma_new->share_node), &(p->vma_head));
		//spin_unlock_irq_restore(&(p->lock), intr);

		vma_cur = vma_new;
		cloned_size += vma_new->size;
		continue;

rollback_pgalloc:
		free_pages(p);
rollback_pages:
		kfree(p);
rollback_vma:
		kfree(vma_new);
		goto rollback;
	}

	retcode = 0;
	goto finalize;

rollback:
	__unmap_and_free_vma(dst, next_entry(vma_start, node), cloned_size);

finalize:
	//spin_unlock(&(src->lock));
	//spin_unlock(&(dst->lock));
	return retcode;
}

/*
 * A very dirty wrapper for common code of copy_to_uvm(), copy_from_uvm() and
 * fill_uvm().
 * NOTE:
 * The following implementation of copy_to_uvm() and copy_from_uvm() does NOT
 * support page faults.
 */
static int
__copy_fill_uvm(struct mm *mm, void *uvaddr, void *vaddr, unsigned char c,
    size_t len, bool fill, bool touser)
{
	struct vma *vma;
	size_t l;
	void *start = uvaddr, *end = uvaddr + len, *kuvaddr;
	void *start_page = PTR_ALIGN_BELOW(uvaddr, PAGE_SIZE);
	void *end_page = PTR_ALIGN_ABOVE(uvaddr + len, PAGE_SIZE);

	//if (!is_user(uvaddr) || !is_user(uvaddr + len - 1))
	//	return -EFAULT;

	//spin_lock(&mm->lock);
	vma = __find_continuous_vma(mm, start_page, end_page - start_page);
	if (vma == NULL) {
		//spin_unlock(&mm->lock);
		return -EFAULT;
	}

	/* If the page table is already loaded, we can exploit that */
	if (current_proc && mm == current_proc->mm) {
		assert(mm->pgindex == get_pgindex());
		if (fill)
			memset(uvaddr, c, len);
		else if (!touser)
			memmove(vaddr, uvaddr, len);
		else
			memmove(uvaddr, vaddr, len);
		//spin_unlock(&mm->lock);
		return 0;
	}

	/* Otherwise, we have to dive into the page table */
	for (; start < end; start = PTR_ALIGN_NEXT(start, PAGE_SIZE)) {
		kuvaddr = uva2kva(mm->pgindex, start);
		l = min2(PTR_ALIGN_NEXT(start, PAGE_SIZE), end) - start;
		if (kuvaddr == NULL) {
			//spin_unlock(&mm->lock);
			return -EACCES;
		}
		if (fill)
			memset(kuvaddr, c, l);
		else if (!touser)
			memmove(vaddr, kuvaddr, l);
		else
			memmove(kuvaddr, vaddr, l);
		vaddr += l;
	}

	//spin_unlock(&mm->lock);
	return 0;
}

int copy_to_uvm(struct mm *mm, void *uvaddr, void *vaddr, size_t len)
{
	return __copy_fill_uvm(mm, uvaddr, vaddr, 0, len, false, true);
}

int copy_from_uvm(struct mm *mm, void *uvaddr, void *vaddr, size_t len)
{
	return __copy_fill_uvm(mm, uvaddr, vaddr, 0, len, false, false);
}

int fill_uvm(struct mm *mm, void *uvaddr, unsigned char c, size_t len)
{
	return __copy_fill_uvm(mm, uvaddr, NULL, c, len, true, true);
}

void mm_init(void)
{
	//arch_mm_init();
	kernel_mm = mm_new();
	switch_pgindex(kernel_mm->pgindex);
}

