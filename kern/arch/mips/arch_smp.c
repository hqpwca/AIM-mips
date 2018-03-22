//extern void mach_smp_startup(void);

#include <arch-smp.h>
#include <raim/console.h>
#include <raim/init.h>
#include <raim/percpu.h>
#include <raim/smp.h>

addr_t slave_stacks[MAX_CPUS];
struct percpu cpus[NR_CPUS];

int nr_cpus(void)
{
	return NR_CPUS;
}

int cpuid(void)
{
	return read_c0_ebase() & EBASE_CPUNUM_MASK;
}

static void alloc_stacks(void)
{
	int i;
	struct pages p;

	p.size = KSTACKSIZE;
	p.flags = 0;

	for(i = 1; i < nr_cpus(); ++i)
	{
		if(alloc_pages(&p) < 0)
			panic("SMP: memory for stack not enough.\n");
		slave_stacks[i] = p.paddr;
		kprintf("SMP: stack for cpu NO.%d at 0x%x\n", i, p.paddr);
	}
}

void smp_startup(void)
{
	alloc_stacks();

	//mach_smp_startup();
}

void arch_slave_init()
{

}

void panic_other_cpus()
{

}

void claim_started()
{

}
