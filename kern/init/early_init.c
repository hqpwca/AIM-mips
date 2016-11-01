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
#include <aim/console.h>
#include <aim/device.h>
#include <aim/early_kmmap.h>
#include <aim/init.h>
#include <aim/mmu.h>
#include <aim/pmm.h>
#include <aim/vmm.h>
#include <aim/trap.h>	
#include <aim/panic.h>
#include <drivers/io/io-mem.h>
#include <drivers/io/io-port.h>
#include <platform.h>

static inline
int early_devices_init(void)
{
#ifdef IO_MEM_ROOT
	if (io_mem_init(&early_memory_bus) < 0)
		return EOF;
#endif /* IO_MEM_ROOT */

#ifdef IO_PORT_ROOT
	if (io_port_init(&early_port_bus) < 0)
		return EOF;
#endif /* IO_PORT_ROOT */
	return 0;
}

void early_mm_init()
{	
	page_index_init((pgindex_t *)premap_addr((void *)&pgindex));
	mmu_init((pgindex_t *)premap_addr((void *)&pgindex));
	load_segment();
}

__noreturn
void master_early_init(void)
{
	/* clear address-space-related callback handlers */
	early_mapping_clear();
	mmu_handlers_clear();
	/* prepare early devices like memory bus and port bus */
	if (early_devices_init() < 0)
		goto panic;
	/* other preperations, including early secondary buses */
	
	if (early_console_init(
		EARLY_CONSOLE_BUS,
		EARLY_CONSOLE_BASE,
		EARLY_CONSOLE_MAPPING
	) < 0)
		panic("Early console init failed.\n");
	kputs("Hello, world!\n");
	
	arch_early_init();
	early_mm_init();
	
	extern uint32_t high_address_entry;
	abs_jump((void *)postmap_addr(&high_address_entry));

	goto panic;
panic:
	asm volatile("cli");
	while(1)
		asm volatile("hlt");
}

void init_free_pages()
{
	size_t kend = kva2pa(&kern_end);
	kend = ALIGN_ABOVE(kend, PAGE_SIZE);
	kprintf("Start Freeing: 0x%x ~ 0x%x...\n", kend, MEM_SIZE - 0x1000000);
	
	struct pages *p = kmalloc(sizeof(*p), 0);
	p->paddr = kend;
	p->size = MEM_SIZE - kend - 0x1000000;
	p->flags = GFP_UNSAFE;
	
	free_pages(p);
	
	kprintf("Finish Freed: 0x%x ~ 0x%x...\n", kend, MEM_SIZE - 0x1000000);
}

void test_allocator()
{
	kputs("\n");
	kputs("Start Test allocators.\n");
	void *page1 = (void *)(uint32_t)pgalloc();
	void *page2 = (void *)(uint32_t)pgalloc();
	kpdebug("page1 : 0x%x\n", page1);
	kpdebug("page2 : 0x%x\n", page2);
	
	pgfree((uint32_t)page1);
	void *page3 = (void *)(uint32_t)pgalloc();
	kpdebug("page3 : 0x%x\n", page3);
	
	pgfree((uint32_t)page2);
	pgfree((uint32_t)page3);
	
	void *a32 = kmalloc(32,0);
	void *b32 = kmalloc(32,0);
	kpdebug("a32 : 0x%x\n", a32);
	kpdebug("b32 : 0x%x\n", b32);
	kfree((void *)premap_addr(a32));
	void *c32 = kmalloc(32,0);
	kpdebug("c32 : 0x%x\n", c32);
	void *a16 = kmalloc(16,0);
	void *b16 = kmalloc(16,0);
	kpdebug("a16 : 0x%x\n", a16);
	kpdebug("b16 : 0x%x\n", b16);
	kfree((void *)premap_addr(b32));
	kfree((void *)premap_addr(c32));
	kfree((void *)premap_addr(a16));
	kfree((void *)premap_addr(b16));
	void *a64 = kmalloc(64,0);
	kpdebug("a64 : 0x%x\n", a64);
	kfree((void *)premap_addr(a64));
	
	kputs("Ended Test allocators.\n");
	kputs("\n");
}

__noreturn
void master_init(void)
{
	trap_init();

	extern uint32_t simple1_start;
	simple_allocator_bootstrap(&simple1_start, 0x8000);	
	kputs("Simple allocator 1 opened.\n");	
	
	page_allocator_init();
	kputs("Page allocator opened.\n");	
	
	init_free_pages();
	
	struct simple_allocator old;
	get_simple_allocator(&old);
	
	simple_allocator_init();
	kputs("Simple allocator 2 opened.\n");
	
	page_allocator_move(&old);
	kputs("Page allocator moved.\n");
	
	test_allocator();

	goto panic;
panic:
	asm volatile("cli");
	while(1)
		asm volatile("hlt");
}

