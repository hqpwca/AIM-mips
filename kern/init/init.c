#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/console.h>
#include <aim/init.h>
#include <aim/mmu.h>
#include <aim/pmm.h>
#include <aim/vmm.h>
#include <aim/smp.h>
#include <aim/trap.h>
#include <aim/panic.h>
#include <aim/device.h>
#include <aim/initcalls.h>
#include <aim/percpu.h>
#include <aim/proc.h>
#include <aim/sched.h>
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
	kfree((void *)(a32));
	void *c32 = kmalloc(32,0);
	kpdebug("c32 : 0x%x\n", c32);
	void *a16 = kmalloc(16,0);
	void *b16 = kmalloc(16,0);
	kpdebug("a16 : 0x%x\n", a16);
	kpdebug("b16 : 0x%x\n", b16);
	kfree((void *)(b32));
	kfree((void *)(c32));
	kfree((void *)(a16));
	kfree((void *)(b16));
	void *a64 = kmalloc(64,0);
	kpdebug("a64 : 0x%x\n", a64);
	kfree((void *)(a64));

	kputs("Ended Test allocators.\n");
	kputs("\n");
}

void output_running_message()
{
	int cid = cpuid();
	int i;
	for(i=0; ; ++i)
		if(i == 400000000)
		{
			kprintf("CPU NO.%d running...\n", cid);
			i = 0;
		}
}

void allocator_init()
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
}

__noreturn
void master_init(void)
{
	allocator_init();
	test_allocator();

	bsp_trap_init();
	trap_init();
	kputs("Trap initialized.\n");
	//trap_check();

	mm_init();
	kputs("uvm initialized.\n");

	extern void mm_test();
	mm_test();

	sched_init();
	kputs("scheduler initialized.\n");
	proc_init();
	idle_init();
	kputs("proc initialized.\n");

	do_early_initcalls();
	do_initcalls();

	smp_startup();
	//asm volatile("sti");
	//trap_check();

	spawn_initproc();
	kputs("initproc spawned.\n");
	while(1)
		schedule();

	output_running_message();

	goto panic;
panic:
	while(1);
/*
	asm volatile("cli");
	while(1)
		asm volatile("hlt");
*/
}

__noreturn
void slave_init(void)
{
	if(cpuid() == 0)
		goto panic;

	kpdebug("slave cpu NO.%d starting...\n", cpuid());

	arch_slave_init();
	trap_init();

	kpdebug("slave cpu NO.%d started.\n", cpuid());

	//asm volatile("sti");
	idle_init();
	spawn_initproc();
	//trap_check();
	claim_started();
	/*
	if(cpuid() == 3)
		panic("panic by CPU 3.\n");
	*/
	//output_running_message();
	while(1)
		schedule();

	goto panic;
panic:
	while(1);
/*
	asm volatile("cli");
	while(1)
		asm volatile("hlt");
*/
}
