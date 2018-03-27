// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <libc/string.h>
#include <raim/trap.h>
#include <raim/debug.h>

void trap_init(void)
{
unimpl();
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
