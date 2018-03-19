#ifndef _ASM_STACK_H
#define _ASM_STACK_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asm.h>
#include <arch-smp.h>
#include <util.h>

#ifdef __ASSEMBLER__
	.macro	get_saved_sp	reg temp
	cpuid	\temp, \reg
	SLL	\temp, WORD_SHIFT
	lui	\reg, %hi(_gp)
	ADDIU	\reg, %lo(_gp)
	LOAD	\reg, %got(kernelsp)(\reg)
	ADDU	\reg, \temp
	LOAD	\reg, (\reg)	/* kernelsp + cpuid() << WORD_SHIFT */
	.endm

	.macro	set_saved_sp	stackp temp temp2
	cpuid	\temp, \temp2
	SLL	\temp, WORD_SHIFT
	lui	\temp2, %hi(_gp)
	ADDIU	\temp2, %lo(_gp)
	LOAD	\temp2, %got(kernelsp)(\temp2)
	ADDU	\temp2, \temp
	STORE	\stackp, (\temp2)
	.endm

	/* Assumes a working sp */
	.macro	PUSH	reg
	SUBU	sp, WORD_SIZE
	STORE	\reg, (sp)
	.endm

	/* Assumes a working sp */
	.macro	POP	reg
	LOAD	\reg, (sp)
	ADDU	sp, WORD_SIZE
	.endm

	.macro	PUSHSTATIC
	PUSH	s7
	PUSH	s6
	PUSH	s5
	PUSH	s4
	PUSH	s3
	PUSH	s2
	PUSH	s1
	PUSH	s0
	.endm

	.macro	POPSTATIC
	POP	s0
	POP	s1
	POP	s2
	POP	s3
	POP	s4
	POP	s5
	POP	s6
	POP	s7
	.endm

	.macro	PUSHTEMP
	PUSH	t7
	PUSH	t6
	PUSH	t5
	PUSH	t4
	PUSH	t3
	PUSH	t2
	PUSH	t1
	PUSH	t0
	.endm

	.macro	POPTEMP
	POP	t0
	POP	t1
	POP	t2
	POP	t3
	POP	t4
	POP	t5
	POP	t6
	POP	t7
	.endm

	.macro	PUSHARGS
	PUSH	a3
	PUSH	a2
	PUSH	a1
	PUSH	a0
	PUSH	v1
	PUSH	v0
	PUSH	AT
	PUSH	zero	/* Theoretically we don't need it */
	.endm

	.macro	POPARGS
	POP	AT	/* Discard the stored zero */
	POP	AT
	POP	v0
	POP	v1
	POP	a0
	POP	a1
	POP	a2
	POP	a3
	.endm

	.macro	PUSHAL
	PUSHSTATIC
	PUSHTEMP
	PUSHARGS
	.endm

	.macro	POPAL
	POPARGS
	POPTEMP
	POPSTATIC
	.endm

	.macro	PUSHCOPR reg
	MFC0	\reg, CP0_BADVADDR
	PUSH	\reg
	MFC0	\reg, CP0_EPC
	PUSH	\reg
	mfc0	\reg, CP0_CAUSE
	PUSH	\reg
	mfc0	\reg, CP0_STATUS
	PUSH	\reg
	mfhi	\reg
	PUSH	\reg
	mflo	\reg
	PUSH	\reg
	.endm

	.macro	POPCOPR reg
	POP	\reg
	mtlo	\reg
	POP	\reg
	mthi	\reg
	POP	\reg
	mtc0	\reg, CP0_STATUS
	POP	\reg
	mtc0	\reg, CP0_CAUSE
	POP	\reg
	MTC0	\reg, CP0_EPC
	POP	\reg
	MTC0	\reg, CP0_BADVADDR	/* discarded */
	.endm

	/* Push onto upward-growing stack, filled stack top */
	.macro	PUSHUF	reg base
	ADDU	\base, WORD_SIZE
	STORE	\reg, (\base)
	.endm

	/* Push onto downward-growing stack, filled stack top */
	.macro	PUSHDF reg base
	SUBU	\base, WORD_SIZE
	STORE	\reg, (\base)
	.endm

	/* Pop from upward-growing stack, filled stack top */
	.macro	POPUF	reg base
	LOAD	\reg, (\base)
	ADDU	\base, WORD_SIZE
	.endm

	/* Pop from downward-growing stack, filled stack top */
	.macro	POPDF	reg base
	LOAD	\reg, (\base)
	SUBU	\base, WORD_SIZE
	.endm

	/* Push onto upward-growing stack, empty stack top */
	.macro	PUSHUE	reg base
	STORE	\reg, (\base)
	ADDU	\base, WORD_SIZE
	.endm

	/* Push onto downward-growing stack, empty stack top */
	.macro	PUSHDE reg base
	STORE	\reg, (\base)
	SUBU	\base, WORD_SIZE
	.endm

	/* Pop from upward-growing stack, empty stack top */
	.macro	POPUE	reg base
	ADDU	\base, WORD_SIZE
	LOAD	\reg, (\base)
	.endm

	/* Pop from downward-growing stack, empty stack top */
	.macro	POPDE	reg base
	SUBU	\base, WORD_SIZE
	LOAD	\reg, (\base)
	.endm

#else	/* !__ASSEMBLER__ */
#include <sys/types.h>
extern unsigned long kernelsp[];
#endif	/* __ASSEMBLER__ */

#endif
