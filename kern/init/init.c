#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/console.h>
#include <aim/init.h>
#include <aim/mmu.h>
#include <aim/pmm.h>
#include <aim/vmm.h>
#include <aim/trap.h>	
#include <aim/panic.h>
#include <aim/device.h>
#include <aim/initcalls.h>
#include <drivers/io/io-mem.h>
#include <drivers/io/io-port.h>
#include <platform.h>

int do_early_initcalls()
{
	extern initcall_t early_init_start[];
	extern initcall_t early_init_end[];
	initcall_t *entry;
	int result = 0;

	kpdebug("Early initcalls initialized from 0x%08x to 0x%08x\n", early_init_start, early_init_end);
	for(entry = early_init_start; entry < early_init_end; entry ++) {
		int ret = (*entry)();
		result |= ret;
	}

	return (result < 0) ? -1 : 0;
}

int do_initcalls()
{
	extern initcall_t norm_init_start[];
	extern initcall_t norm_init_end[];
	initcall_t *entry;
	int result = 0;

	kpdebug("Normal initcalls initialized from 0x%08x to 0x%08x\n", norm_init_start, norm_init_end);
	for(entry = norm_init_start; entry < norm_init_end; entry ++) {
		int ret = (*entry)();
		result |= ret;
	}

	return (result < 0) ? -1 : 0;
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
	//test_allocator();

	trap_init();
	kputs("Trap initialized.\n");

	//trap_check();

	to_link = -1;

	do_early_initcalls();
	do_initcalls();

	kputs("Test new console\n");

	goto panic;
panic:
	asm volatile("cli");
	while(1)
		asm volatile("hlt");
}