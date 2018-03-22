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
    kprintf("abs jump to %08x%08x\n", (unsigned)((uintptr_t)addr>>32),(unsigned)(uintptr_t)addr);
	asm ("jr %0"::"r"(addr));
	__builtin_unreachable();
}

static pgindex_t bpgtbl; // boot page table

void arch_mm_init()
{
    page_index_clear(&bpgtbl);
    page_index_early_map(&bpgtbl, RAM_PHYSBASE, (void*)(KERN_BASE-(KERN_START-RAM_PHYSBASE)), MEM_SIZE);
	mmu_init(&bpgtbl);
}





void mmu_init(pgindex_t *boot_page_index)
{
//    early_mapping_add_memory();

    // WARL. Write-Any Read-Legal 
    uint64_t pgindex_paddr = (uintptr_t) boot_page_index;
    
    kprintf("boot page table located at %08x%08x\n", (unsigned)(pgindex_paddr>>32),(unsigned)pgindex_paddr);
    
    union {
        struct {
            uint64_t PPN : 44;
            uint64_t ASID : 16;
            uint64_t MODE : 4;
        };
        uint64_t val;
    } new_satp = {

        .PPN = pgindex_paddr >> 12,
        .ASID = 0,
        .MODE = 8, // Sv39

    };
    
    __asm__ __volatile__ ("sfence.vma");
    __asm__ __volatile__ ("csrw satp, %0"::"r"(new_satp.val):"memory");
    
    kprintf("paging enabled!\n");
}




void arch_early_init(void)
{
	//early_mach_init();
}

void master_init2(void)
{
    panic("haha!!!");
}
