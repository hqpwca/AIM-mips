// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <libc/string.h>
#include <raim/trap.h>
#include <raim/debug.h>
#include <raim/trap.h>
#include <arch-trap.h>
#include <raim/console.h>

void trap_init(void)
{
    uint64_t e = (uint64_t)&trap_entry;
    assert(e % 4 == 0);
    __asm__ __volatile__ ("csrw stvec, %0"::"r"(e));
}

void trap_handler(struct trapframe *regs)
{
unimpl();
}

__noreturn
extern void trap_exit(struct trapframe *regs);

__noreturn void trap_return(struct trapframe *regs)
{
unimpl();
}

void bsp_trap_init()
{
	
}
