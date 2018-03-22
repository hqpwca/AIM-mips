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
#include <raim/console.h>
#include <raim/early_kmmap.h>
#include <raim/mmu.h>
#include <raim/panic.h>
#include <raim/vmm.h>
#include <raim/pmm.h>
#include <raim/uvm.h>
#include <arch-mmu.h>
#include <tlb.h>
#include <mipsregs.h>
#include <libc/string.h>

bool early_mapping_valid(__unused struct early_mapping *entry)
{
	return true;
}

void page_index_clear(__unused pgindex_t *index)
{
}

int page_index_early_map(__unused pgindex_t *index, __unused addr_t paddr,
	__unused void *vaddr, __unused size_t length)
{
	return 0;
}

void tlb_flush(void)
{
	int nr_entries = get_tlb_entries();
	for (int i = 0; i < nr_entries; ++i) {
		write_c0_index(i);
		write_c0_entryhi(ENTRYHI_DUMMY(i));
		write_c0_entrylo0(0);
		write_c0_entrylo1(0);
		write_c0_pagemask(PAGEMASK_VALUE);
		tlbwi();
	}
	/* Clear ASID */
	write_c0_entryhi(0);
}

void mmu_init(__unused pgindex_t *boot_page_index)
{
	tlb_flush();
}

void init_free_pages()
{
	size_t kend = kva2pa(&kern_end);
	kend = ALIGN_ABOVE(kend, PAGE_SIZE);
	kprintf("Start Freeing: 0x%x ~ 0x%x...\n", kend, LOWRAM_TOP);

	struct pages *p = kmalloc(sizeof(*p), 0);
	p->paddr = kend;
	p->size = LOWRAM_TOP - kend;
	p->flags = GFP_UNSAFE;

	free_pages(p);

	kprintf("Finish Freed: 0x%x ~ 0x%x...\n", kend, LOWRAM_TOP);

/*
#if HIGHRAM_SIZE != 0
	kprintf("Start Freeing: 0x%x ~ 0x%x...\n", HIGHRAM_BASE, HIGHRAM_BASE + HIGHRAM_SIZE);

	p = kmalloc(sizeof(*p), 0);
	p->paddr = HIGHRAM_BASE;
	p->size = HIGHRAM_SIZE;
	p->flags = GFP_UNSAFE;

	free_pages(p);

	kprintf("Finish Freed: 0x%x ~ 0x%x...\n", HIGHRAM_BASE, HIGHRAM_BASE + HIGHRAM_SIZE);
#endif
*/
}
