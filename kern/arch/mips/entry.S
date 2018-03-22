#include <asm.h>
#include <regdef.h>
#include <cp0regdef.h>
#include <arch-smp.h>
#include <util.h>
#include <addrspace.h>

/*
 * IMPORTANT NOTE:
 * Unless explicitly stated (either by filename, assembler directives,
 * or comments), all code inside kern/arch/mips directory should work
 * on both MIPS32 and MIPS64 architectures.
 */

/*
 * Here is the entry of kernel, as stated by vmraim.lds.S.
 *
 * Usually on a multiprocessor machine, only one processor (we call it
 * the master processor) is brought in to the kernel entry, while
 * other processors (called slave processors) are suspended by firmware
 * and wait for some sort of signal from the master.
 */
BEGIN(__start)
	/* PIC environment preparation */
setup_pic:
	/*
	 * MIPS did not consider PIC at time of design.  When the concept
	 * of PIC was mentioned, MIPS ABI designated register 't9' to store
	 * the address of entry of current routine the processor is running.
	 * Furthermore, MIPS ABI stated that register 'gp' points to the
	 * Global Offset Table (GOT).
	 *
	 * Unfortunately, we cannot assume that BIOS and/or bootloaders would
	 * set up a PIC environment (i.e. 'gp' and 't9') for us, as most
	 * operating systems including Linux runs a non-PIC kernel on MIPS.
	 * So we have to set up both registers by our own.
	 */

	/* We need to obtain the entry of __start dynamically first, by
	 * calling a Branch-And-Link, retrieving the return address, and
	 * compute the address manually.  To ensure correctness, optimization
	 * and instruction reordering should be turned off. */
	.set	noreorder
	bal	1f
	nop
	/* Idea from UBoot: we embed the value of _gp in the entry code, and
	 * fetch the value during execution there.
	 * Note for MIPS64: we know that _gp is above KSEG0, so storing
	 * the low 32 bits would suffice, as LW instruction would sign-extend
	 * the high 32 bits with all 1s.
	 */
	.word	_gp		/* at __start + 8 = t9 + 8 */
1:
	SUBU	t9, ra, 8
	lw	gp, 8(t9)
	.set	reorder

	/* Ensure that we are inside kernel mode. */
	MFC032	a0, CP0_STATUS
	or	a0, ST_EXCM
	xor	a0, ST_EXCM
	or	a0, ST_IM
	MTC032	a0, CP0_STATUS
	/*
	 * The kernel is already inside RAM, so .text section, .rodata
	 * section and .data section are in their right position with
	 * correct values.
	 * However, .bss section is still not guaranteed to be initialized
	 * to 0, so we are going to clean it first.
	 */
init_bss:
	LA	s1, __bss_begin__
	LA	s2, __bss_end__
1:	bge	s1, s2, 9f
	sw	zero, (s1)
	ADDU	s1, 4
	b	1b
9:
init_master_stack:
	/* boot_stack points to stack top.  Defined in vmraim.lds.S */
	LA	sp, boot_stack

main_entry:
	/*
	 * We know that we won't return from master_early_init(), so
	 * we don't need to save anything prior to calling the function.
	 *
	 * PIC note: ordinarily we need to save gp register since jumping
	 * to another global function would probably clobber the register.
	 * However, we are not returning here, so we'll just let it happen.
	 *
	 * The .cprestore directive, which saves gp onto stack, is a hack to
	 * suppress the warning message from assembler.  We don't care about
	 * the value anyway.
	 */
	.cprestore
	jal	master_early_init

1:	b	1b
END(__start)

BEGIN(high_address_entry)
	.set	noreorder
	.cpload	t9
	.set	reorder
	LA	sp, boot_stack
	.cprestore
	jal	master_init
1:	b	1b
END(high_address_entry)

BEGIN(slave_entry)
	/*
	 * The logic is mostly the same as master early entry except:
	 * 1. We do not clean bss (obviously)
	 * 2. The stack top is different (slave_stacks[cpuid()])
	 */
	.set	noreorder
	bal	1f
	nop
	.word	_gp
1:
	SUBU	t9, ra, 8
	lw	gp, 8(t9)
	.set	reorder

	MFC032	a0, CP0_STATUS
	or	a0, ST_EXCM
	xor	a0, ST_EXCM
	or	a0, ST_IM
	MTC032	a0, CP0_STATUS
	bal	load_slave_sp
	/*
	 * .cprestore just to supress warning. Same as above.
	 */
	.cprestore
	jal	slave_early_init
1:	b	1b
END(slave_entry)

BEGIN(slave_upper_entry)
	.set	noreorder
	.cpload	t9
	.set	reorder
	bal	load_slave_sp
	MFC032	a0, CP0_STATUS
	and	a0, ~ST_BEV
	MTC032	a0, CP0_STATUS
	.cprestore
	jal	slave_init
1:	b	1b
END(slave_upper_entry)

BEGIN(load_slave_sp)
	LA	t0, slave_stacks
	cpuid	t1, t2
	SLL	t1, WORD_SHIFT
	ADDU	t0, t1
	LOAD	sp, (t0)
	ADDU	sp, IO_CAC_BASE
	jr	ra
END(load_slave_sp)
