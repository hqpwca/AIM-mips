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
#include <raim/init.h>
#include <raim/mmu.h>
#include <raim/console.h>
#include <raim/early_kmmap.h>
#include <raim/panic.h>


__noreturn
void abs_jump(void *addr)
{
    kprintf("abs jump to %016llX\n", (uint64_t)addr);
	asm ("jr %0"::"r"(addr));
	__builtin_unreachable();
}

static pgindex_t bpgtbl; // boot page table

void arch_mm_init()
{
    page_index_clear(&bpgtbl);
    riscv_map_kernel(&bpgtbl, 1);
	mmu_init(&bpgtbl);
}


void arch_early_init(void)
{
	//early_mach_init();
}

