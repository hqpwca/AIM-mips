// by ZBY
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/smp.h>
#include <raim/percpu.h>

// FIXME: no SMP support

struct percpu cpus[NR_CPUS];

void panic_other_cpus()
{

}

int cpuid(void)
{
    return 0;
}

void smp_startup(void)
{
}

void arch_slave_init()
{
}

void claim_started()
{

}
