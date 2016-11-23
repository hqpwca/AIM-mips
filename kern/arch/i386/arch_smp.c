#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
#include <aim/init.h>
#include <aim/mmu.h>
#include <aim/vmm.h>
#include <aim/pmm.h>
#include <aim/smp.h>
#include <aim/panic.h>
#include <aim/trap.h>
#include <aim/console.h>
#include <aim/percpu.h>
#include <arch-mmu.h>
#include <arch-trap.h>
#include <arch-lapic.h>
#include <arch-mp.h>
#include <libc/string.h>

struct percpu cpus[CPUNUM];

int nr_cpus(void)
{
	return CPUNUM;
}

int cpuid(void)
{
	int apicid, i;

	if(!lapic)
		return 0;

	apicid = lapic[ID] >> 24;
	for(i = 0; i < nr_cpus(); ++i)
		if(cpus[i].apicid == apicid)
			return i;

	panic("unknown apicid\n");
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
		cpus[i].stack = p.paddr;
		kprintf("SMP: stack for cpu NO.%d at 0x%x\n", i, p.paddr);
	}
}

void smp_startup(void)
{
	alloc_stacks();

	int i;
	extern uint32_t _sentry_start[];
	uint32_t slave_entry = (uint32_t)_sentry_start;

	for(i = 1; i < nr_cpus(); ++i)
	{
		cpus[i].started = false;

		*(uint32_t *)(slave_entry - 4) = cpus[i].stack + KSTACKSIZE;
		*(pgindex_t *)(slave_entry - 8) = (pgindex_t *)premap_addr((void *)&pgindex);
		*(uint32_t *)(slave_entry - 12) = i;

		lapicstartap(cpus[i].apicid, premap_addr(slave_entry));

		while(cpus[i].started == 0);
	}
}

int handle_ipi_interrupt(unsigned int msg)
{
	return 0;
}

void panic_other_cpus()
{
	int now_id = cpuid();
	for(int id = 0; id < nr_cpus(); id ++)
	{
		if(id == now_id) continue;
		lapicw(ICRHI, cpus[id].apicid<<24);
		lapicw(ICRLO, 0x50);
		while(lapic[ICRLO] & DELIVS);

		//kpdebug("CPU %d panicked.\n", id);
	}
}

void arch_slave_init()
{
	lapic_init();
}
