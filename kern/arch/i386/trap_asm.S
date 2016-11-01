#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <arch-mmu.h>

.globl alltraps
alltraps:
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs
	pushal

	movw $(SEG_KDATA<<3), %ax
	movw %ax, %ds
	movw %ax, %es

	pushl %esp
	call trap_exec

.globl trapret
trapret:
	movl 4(%esp), %esp
	popal
	popl %gs
	popl %fs
	popl %es
	popl %ds
	addl $0x8, %esp
	iret 