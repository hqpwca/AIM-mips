/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 *
 * This file is part of RAIM.
 *
 * RAIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RAIM is distributed in the hope that it will be useful,
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

#include <sys/types.h>
#include <sys/param.h>
//#include <raim/sync.h>
#include <raim/mmu.h>
#include <raim/console.h>
#include <raim/pmm.h>
#include <libc/string.h>
#include <util.h>

/* dummy implementations */
static int __alloc(__unused struct pages *pages) { return EOF; }
static void __free(__unused struct pages *pages) {}
static addr_t __get_free(void) { return 0; }

static struct page_allocator __allocator = {
	.alloc		= __alloc,
	.free		= __free,
	.get_free	= __get_free
};

void set_page_allocator(struct page_allocator *allocator)
{
	memcpy(&__allocator, allocator, sizeof(*allocator));
}

void pmemset(addr_t paddr, unsigned char b, lsize_t size)
{
	for (; size > 0; size -= PAGE_SIZE, paddr += PAGE_SIZE){
		//kpdebug("size: 0x%x, paddr: 0x%x, byte: 0x%x\n", (uint32_t)size, (uint32_t)paddr, (uint32_t)b);
		memset((void *)pa2kva(paddr), b, PAGE_SIZE);
	}
}

int alloc_pages(struct pages *pages)
{
	int result;
	//unsigned long flags;
	if (pages == NULL)
		return EOF;
	//recursive_lock_irq_save(&memlock, flags);
	result = __allocator.alloc(pages);
	//kpdebug("0x%x, 0x%x, 0x%x\n", pages->paddr, pages->size, pages->flags);
	if (pages->flags & GFP_ZERO)
		pmemset(pages->paddr, 0, pages->size);
	//recursive_unlock_irq_restore(&memlock, flags);
	return result;
}

int alloc_aligned_pages(struct pages *pages, lsize_t align)
{
	struct pages buf, padding;
	int ret;

	/* we allocate extra pages */
	buf = *pages;
	buf.size += align - PAGE_SIZE;
	ret = alloc_pages(&buf);
	if (ret < 0) return ret;
	/* prepare return values */
	pages->paddr = ALIGN_ABOVE(buf.paddr, align);
	/* deal with padding */
	padding.flags = buf.flags;
	if (buf.paddr < pages->paddr){
		padding.paddr = buf.paddr;
		padding.size = pages->paddr - buf.paddr;
		free_pages(&padding);
	}
	if (buf.paddr + buf.size > pages->paddr + pages->size){
		padding.paddr = pages->paddr + pages->size;
		padding.size = buf.paddr + buf.size - pages->paddr - pages->size;
		free_pages(&padding);
	}
	return 0;
}

void free_pages(struct pages *pages)
{
	//unsigned long flags;
	if (!(pages->flags & GFP_UNSAFE))
		pmemset(pages->paddr, JUNKBYTE, pages->size);
	//recursive_lock_irq_save(&memlock, flags);
	__allocator.free(pages);
	//recursive_unlock_irq_restore(&memlock, flags);
}

addr_t get_free_memory(void)
{
	return __allocator.get_free();
}
