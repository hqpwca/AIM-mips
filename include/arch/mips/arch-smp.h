#ifndef _ARCH_SMP_H
#define _ARCH_SMP_H

#include <asm.h>
#include <regdef.h>
#include <cp0regdef.h>

#ifndef __ASSEMBLER__
#include <mipsregs.h>
#include <arch-mmu.h>
#include <stack.h>
/*
 * The header is included by a C header/source.
 */

//#define cpuid()		__cpuid()
#define current_kernelsp	kernelsp[cpuid()]
#define current_pgdir		pgdir_slots[cpuid()]

#else	/* __ASSEMBLER__ */
/*
 * The header is included by an assembly header/source.
 */
	.macro	cpuid result temp
	MFC0	\result, CP0_EBASE
	andi	\result, EBASE_CPUNUM_MASK
	.endm
#endif	/* !__ASSEMBLER__ */

#endif
