#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#define IDT_ENTRY_NUM 256

#include <sys/types.h>
#include <sys/param.h>
#include <aim/init.h>
#include <aim/mmu.h>
#include <aim/panic.h>
#include <aim/trap.h>
#include <aim/smp.h>
#include <aim/console.h>
#include <arch-mmu.h>
#include <arch-trap.h>
#include <libc/string.h>

struct gatedesc idt[IDT_ENTRY_NUM];
extern uint32_t vectors[];
__noreturn extern void trapret(void *old_esp);

/* TODO: add something about Local APIC and IOAPIC. */
void trap_init()
{
	for(int i=0; i<IDT_ENTRY_NUM; i++)
		set_gate(&idt[i], SEG_KCODE<<3, (uint32_t)vectors[i], DPL_KERNEL, 0);
	set_gate(&idt[T_SYSCALL], SEG_KCODE<<3, (uint32_t)vectors[T_SYSCALL], DPL_USER, 1);

	lidt(idt, sizeof(idt));
}

void trap_exec(struct trapframe *tf)
{
	if(tf->trapno == T_PANIC) {
		kprintf("PANIC: CPU %d panicked.\n", cpuid());
	}
	if(tf->trapno == T_SYSCALL)
		handle_syscall(tf->eax, tf->ebx, tf->ecx, tf->edx, tf->esi, tf->edi, tf->ebp);
	else
	{
		int irqno = tf->trapno - T_IRQ0;
		if(irqno == IRQ_TIMER || irqno == IRQ_KBD || irqno == IRQ_COM1 ||
			 irqno == IRQ_IDE ||  irqno == IRQ_ERROR ||  irqno == IRQ_SPURIOUS)
			handle_interrupt(irqno);
		else
			panic("Unknown or undefined trap No.%d\n", tf->trapno);
	}
	trap_return(tf);
}

/*
Interrupts ID:

#define IRQ_TIMER        0
#define IRQ_KBD          1
#define IRQ_COM1         4
#define IRQ_IDE         14
#define IRQ_ERROR       19
#define IRQ_SPURIOUS    31
*/
__noreturn
void trap_return(struct trapframe *tf)
{
	trapret((void *)tf);
}

void trap_check()
{
	asm volatile("int $32;");
}