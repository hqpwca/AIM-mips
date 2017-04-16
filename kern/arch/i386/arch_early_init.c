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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/init.h>
#include <aim/mmu.h>
#include <aim/early_kmmap.h>
#include <aim/panic.h>
#include <arch-mmu.h>
#include <libc/string.h>

__noreturn
void abs_jump(void *addr)
{
	asm volatile("jmp *%0" : : "r"(addr));
}

static struct segdesc gdt[NSEGS];

void load_segment(void)
{
	gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0x0, 0xffffffff, 0);
	gdt[SEG_KDATA] = SEG(STA_W, 0x0, 0xffffffff,0);
	gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0x0, 0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W, 0x0, 0xffffffff,DPL_USER);
	
	lgdt(gdt, sizeof(gdt));
}

void arch_mm_init(void)
{
	if(early_mapping_add_umem(0x0, USERTOP) == 0)
		panic("error early mapping memory\n");
	if(early_mapping_add_memory(0x0, KERN_START) == 0)
		panic("error early mapping memory\n");
	if(early_mapping_add_memory(KERN_START, MEM_SIZE - KERN_START - 0x10000000) == 0)
		panic("error early mapping memory\n");
	if(early_mapping_add_kmmap(MEM_SIZE - 0x10000000, 0x8000000) == 0)
		panic("error early mapping kmmap\n");
	if(early_mapping_add_devspace(0xfe000000,0x1000000) == 0)
		panic("error early mapping devspace\n");

	page_index_init((pgindex_t *)premap_addr((void *)&pgindex));
	mmu_init((pgindex_t *)premap_addr((void *)&pgindex));
	load_segment();
}

void arch_early_init(void)
{
	
}

