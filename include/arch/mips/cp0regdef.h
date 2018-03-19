/*
 * Copyright (C) 1994, 1995, 1996, 1997, 2000, 2001 by Ralf Baechle
 * Copyright (C) 2000 Silicon Graphics, Inc.
 * Modified for further R[236]000 support by Paul M. Antoine, 1996.
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000, 07 MIPS Technologies, Inc.
 * Copyright (C) 2003, 2004  Maciej W. Rozycki
 *
 * Copyright (C) 2015, Gan Quan <coin2028@hotmail.com>
 */
#ifndef _ASM_CP0REGDEF_H
#define _ASM_CP0REGDEF_H

/*
 * NOTE:
 * Not all of these are implemented in MSIM, but they're all available on
 * Loongson.
 */
#define CP0_INDEX	$0
#define CP0_RANDOM	$1
#define CP0_ENTRYLO0	$2
#define CP0_ENTRYLO1	$3
#define CP0_CONTEXT	$4
#define CP0_PAGEMASK	$5
# define CP0_PAGEGRAIN	$5, 1
#define CP0_WIRED	$6
#define CP0_HWRENA	$7
#define CP0_BADVADDR	$8	/* Bad Virtual Address Register */
#define CP0_COUNT	$9	/* Counter */
#define CP0_ENTRYHI	$10
#define CP0_COMPARE	$11	/* Comparer: raises interrupt when = $9 */
#define CP0_STATUS	$12	/* Status Register */
# define CP0_INTCTL	$12, 1	/* Interrupt control */
# define CP0_SRSCTL	$12, 2
# define CP0_SRSMAP	$12, 3
#define CP0_CAUSE	$13	/* Cause Register */
#define CP0_EPC		$14	/* Exception Program Counter */
#define CP0_PRID	$15
# define CP0_EBASE	$15, 1	/* Exception base, and CPU ID for multicore */
#define CP0_CONFIG	$16
# define CP0_CONFIG1	$16, 1
# define CP0_CONFIG2	$16, 2
# define CP0_CONFIG3	$16, 3
#define CP0_LLADDR	$17
#define CP0_WATCHLO	$18
#define CP0_WATCHHI	$19
#define CP0_XCONTEXT	$20
#define CP0_FRAMEMASK	$21
#define CP0_DIAGNOSTIC	$22
#define CP0_PERFCTL	$25
# define CP0_PERFCNT	$25, $1
#define CP0_ECC		$26
#define CP0_CACHEERR	$27
# define CP0_CERRADDR	$27, $1
#define CP0_TAGLO	$28
# define CP0_DATALO	$28, $1
#define CP0_TAGHI	$29
# define CP0_DATAHI	$29, $1
#define CP0_ERROREPC	$30
#define CP0_DESAVE	$31

/*
 * Status register (CP0_STATUS) mode bits
 */
#define ST_CU3	0x80000000U	/* Coprocessor 3 (MIPS IV User Mode) */
#define ST_CU2	0x40000000U	/* Coprocessor 2 */
#define ST_CU1	0x20000000U	/* Coprocessor 1 (FPU) */
#define ST_CU0	0x10000000U	/* Coprocessor 0 (this one) */
#define ST_RP	0x08000000U	/* Reduce power */
#define ST_FR	0x04000000U	/* Float register mode switch (?) */
#define ST_RE	0x02000000U	/* Reverse-endian */
#define ST_MX	0x01000000U	/* Enable DSP or MDMX */
#define ST_PX	0x00800000U	/* Enable 64-bit operations in user mode */
/* The exception handler would be at 0xbfc00000 if BEV=1, 0x80000000 
 * otherwise */
#define ST_BEV	0x00400000U	/* Bootstrap Exception Vector, usually 0 */
#define ST_TS	0x00200000U	/* TLB SHUTDOWN */
#define ST_SR	0x00100000U	/* Soft Reset */
#define ST_NMI	0x00080000U	/* Non-maskable Interrupt */
/* Interrupt Masks */
#define ST_IM	0x0000ff00U	/* All interrupt masks */
#define ST_IMx(i)	(1 << ((i) + 8))
/* eXtended addressing bits for 64-bit addresses */
#define ST_KX	0x00000080U	/* Kernel mode eXtended addressing */
#define ST_SX	0x00000040U	/* Supervisor mode eXtended addressing */
#define ST_UX	0x00000020U	/* User mode eXtended addressing */
/*
 * This mask is helpful since clearing these bits in exception handler
 * guarantees that:
 * 1. The processor runs in kernel mode.
 * 2. The processor is safe from interrupts.
 */
#define ST_EXCM	0x0000001fU	/* Status Register EXception Clear Mask */
/* Kernel/Supervisor/User mode switch */
#define ST_KSU	0x00000018U	/* KSU switch */
# define KSU_USER	0x00000010U	/* User mode */
# define KSU_SUPERVISOR	0x00000008U	/* Supervisor mode */
# define KSU_KERNEL	0x00000000U	/* Kernel mode */
#define ST_ERL	0x00000004U	/* Error Level */
#define ST_EXL	0x00000002U	/* Exception Level */
#define ST_IE	0x00000001U	/* Global Interrupt Enable */

#define NR_INTS	8		/* Number of Interrupt Mask Bits */

/*
 * Cause register (CP0_CAUSE) bits, for handling exceptions
 */

/* Branch Delay would be set if an exception occur in the delay slot, while
 * EPC points to the branching instruction. */
#define CR_BD	0x80000000U	/* Branch Delay */
#define CR_TI	0x40000000U	/* Timer Interrupt */
#define CR_CE	0x30000000U	/* Coprocessor Error */
#define CR_DC	0x08000000U	/* Disable Counter */
#define CR_PCI	0x04000000U	/* CP0 Performance Counter Overflow (?) */
#define CR_IV	0x00800000U	/* Interrupt Vector */
#define CR_WP	0x00400000U	/* Watchpoint */
#define CR_IP	0x0000ff00U	/* Interrupt Pending */
#define CR_IPx(i)	(1 << ((i + 8)))
#define CR_EC	0x0000007cU	/* Exception Code */
#define EXCCODE(x)	(((x) & CR_EC) >> 2)

/*
 * Exception codes
 */
#define EC_int		0
#define EC_tlbm		1
#define EC_tlbl		2
#define EC_tlbs		3
#define EC_adel		4
#define EC_ades		5
#define EC_ibe		6
#define EC_dbe		7
#define EC_sys		8
#define EC_bp		9
#define EC_ri		10
#define EC_cpu		11
#define EC_ov		12
#define EC_tr		13
#define EC_fpe		15
#define EC_is		16
#define EC_dib		19
#define EC_ddbs		20
#define EC_ddbl		21
#define EC_watch	23
#define EC_dbp		26
#define EC_dint		27
#define EC_dss		28
#define EC_cacheerr	30

/*
 * PageMask register
 */

#define PM_4K		0x00000000U
#define PM_8K		0x00002000U
#define PM_16K		0x00006000U
#define PM_32K		0x0000e000U
#define PM_64K		0x0001e000U
#define PM_128K		0x0003e000U
#define PM_256K		0x0007e000U
#define PM_512K		0x000fe000U
#define PM_1M		0x001fe000U
#define PM_2M		0x003fe000U
#define PM_4M		0x007fe000U
#define PM_8M		0x00ffe000U
#define PM_16M		0x01ffe000U
#define PM_32M		0x03ffe000U
#define PM_64M		0x07ffe000U
#define PM_256M		0x1fffe000U
#define PM_1G		0x7fffe000U

/*
 * PageGrain register used by Loongson 3A
 */

#define PG_ELPA		0x20000000U	/* Enable Large Page Address */

/*
 * Config register
 */
#define CONF_CM		0x80000000U	/* Config1 register (FPU) */
#define CONF_BE		0x00008000U	/* Big-endianness */
#define CONF_AT		0x00006000U	/* MIPS Architecture */
# define CONF_EM	0x00004000U	/* MIPS64 with 64-bit address space */
# define CONF_EB	0x00002000U	/* MIPS64 with 32-bit address space */
#define CONF_AR		0x00001c00U	/* MIPS release version */
# define CONF_R2	0x00000400U	/* MIPSr2 */
#define CONF_MT		0x00000380U	/* MMU type */
# define CONF_TLB	0x00000080U	/* Standard TLB */
#define CONF_VI		0x00000008U	/* Virtual instruction cache */
#define CONF_K0		0x00000007U	/* KSEG0 cache consistency */
# define CONF_CACHEABLE	0x00000003U	/* Cacheable */
# define CONF_UNCACHED	0x00000002U	/* Uncached */

#define EBASE_CPUNUM_MASK	0x3ffU

#endif

