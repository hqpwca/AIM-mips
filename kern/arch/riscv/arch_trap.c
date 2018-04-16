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


void dumptf(struct trapframe *regs)
{
    kprintf("================= TRAPFRAME %016llx =================\n", (uint64_t)regs);
    
    kprintf("[[REGISTER]]\n");
    static const char *regname[] = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
        "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
        "t3", "t4", "t5", "t6"
    };
    for (int i=0;i<32;i++){
        kprintf("x%2d | %5s = %016llx\t", i, regname[i], regs->reg[i]);
        if (i%2)kprintf("\n");
    }
    kprintf("[[CSR]]\n");
    kprintf("sscratch    = %016llx\n", regs->sscratch);
    kprintf("sepc        = %016llx\n", regs->sepc);
    kprintf("scause      = %016llx\n", regs->scause);
    kprintf("stval       = %016llx\n", regs->stval);
}
void crash(struct trapframe *regs)
{
    dumptf(regs); panic("crash");
}

void trap_init(void)
{
    uint64_t e = (uint64_t)&trap_entry;
    assert(e % 4 == 0);
    __asm__ __volatile__ ("csrw stvec, %0"::"r"(e));
    
    __asm__ __volatile__ ("csrw sscratch, x0");
}

__noreturn void trap_handler(struct trapframe *regs)
{
crash(regs);
unimpl();
trap_return(regs);
}

__noreturn
extern void trap_exit(struct trapframe *regs);

__noreturn void trap_return(struct trapframe *regs)
{
    trap_exit(regs);
}

void bsp_trap_init()
{
	
}

