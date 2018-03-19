/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 *
 * This file is part of AIMv6.
 *
 * AIMv6 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIMv6 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <libc/string.h>
#include <aim/trap.h>
#include <aim/console.h>
#include <aim/smp.h>
#include <regs.h>
#include <mipsregs.h>
#include <cp0regdef.h>
#include <arch-trap.h>
//#include <syscall.h>
#include <decode.h>
#include <aim/panic.h>
//#include <timer.h>
#include <errno.h>

void trap_init(void)
{
	extern uint32_t generic_exception_entry;
	extern uint32_t tlb_entry;
	memcpy((void *)GENERIC_EXCEPT_ENTRY, &generic_exception_entry, 0x80);
	memcpy((void *)XTLB_REFILL_ENTRY, &tlb_entry, 0x80);
	memcpy((void *)TLB_REFILL_ENTRY, &tlb_entry, 0x80);

	uint32_t status = read_c0_status();
	write_c0_status(status & ~ST_BEV);
}

static void dump_regs(struct trapframe *regs)
{
	kprintf("%d\tzero\t%p\n", cpuid(), regs->gpr[_ZERO]);
	kprintf("%d\tat\t%p\n", cpuid(), regs->gpr[_AT]);
	kprintf("%d\tv0\t%p\n", cpuid(), regs->gpr[_V0]);
	kprintf("%d\tv1\t%p\n", cpuid(), regs->gpr[_V1]);
	kprintf("%d\ta0\t%p\n", cpuid(), regs->gpr[_A0]);
	kprintf("%d\ta1\t%p\n", cpuid(), regs->gpr[_A1]);
	kprintf("%d\ta2\t%p\n", cpuid(), regs->gpr[_A2]);
	kprintf("%d\ta3\t%p\n", cpuid(), regs->gpr[_A3]);
	kprintf("%d\tt0\t%p\n", cpuid(), regs->gpr[_T0]);
	kprintf("%d\tt1\t%p\n", cpuid(), regs->gpr[_T1]);
	kprintf("%d\tt2\t%p\n", cpuid(), regs->gpr[_T2]);
	kprintf("%d\tt3\t%p\n", cpuid(), regs->gpr[_T3]);
	kprintf("%d\tt4\t%p\n", cpuid(), regs->gpr[_T4]);
	kprintf("%d\tt5\t%p\n", cpuid(), regs->gpr[_T5]);
	kprintf("%d\tt6\t%p\n", cpuid(), regs->gpr[_T6]);
	kprintf("%d\tt7\t%p\n", cpuid(), regs->gpr[_T7]);
	kprintf("%d\ts0\t%p\n", cpuid(), regs->gpr[_S0]);
	kprintf("%d\ts1\t%p\n", cpuid(), regs->gpr[_S1]);
	kprintf("%d\ts2\t%p\n", cpuid(), regs->gpr[_S2]);
	kprintf("%d\ts3\t%p\n", cpuid(), regs->gpr[_S3]);
	kprintf("%d\ts4\t%p\n", cpuid(), regs->gpr[_S4]);
	kprintf("%d\ts5\t%p\n", cpuid(), regs->gpr[_S5]);
	kprintf("%d\ts6\t%p\n", cpuid(), regs->gpr[_S6]);
	kprintf("%d\ts7\t%p\n", cpuid(), regs->gpr[_S7]);
	kprintf("%d\tt8\t%p\n", cpuid(), regs->gpr[_T8]);
	kprintf("%d\tt9\t%p\n", cpuid(), regs->gpr[_T9]);
	kprintf("%d\tgp\t%p\n", cpuid(), regs->gpr[_GP]);
	kprintf("%d\tsp\t%p\n", cpuid(), regs->gpr[_SP]);
	kprintf("%d\ts8\t%p\n", cpuid(), regs->gpr[_S8]);
	kprintf("%d\tra\t%p\n", cpuid(), regs->gpr[_RA]);
	kprintf("%d\tLO\t%p\n", cpuid(), regs->lo);
	kprintf("%d\tHI\t%p\n", cpuid(), regs->hi);
	kprintf("%d\tSTATUS\t%p\n", cpuid(), regs->status);
	kprintf("%d\tCAUSE\t%p\n", cpuid(), regs->cause);
	kprintf("%d\tEPC\t%p\n", cpuid(), regs->epc);
	kprintf("%d\tBVA\t%p\n", cpuid(), regs->badvaddr);
}

/*
 * Determine the branch target according to given program counter.
 *
 * @pc locates the branching or jumping instruction.
 */
static unsigned long __branch_target(struct trapframe *tf, unsigned long pc)
{
	unsigned int insn = read_insn(pc);
	unsigned int opcode = OPCODE(insn);
	unsigned int func, rs, rt, rd;
	unsigned long offset, branch_pc, cont_pc;

	cont_pc = pc + 8;

	/* Handle jumps */
	switch (opcode) {
	case OP_SPECIAL:
		rs = R_RS(insn);
		rd = R_RD(insn);
		func = R_FUNC(insn);
		switch (func) {
		case FN_JALR:
			/* JALR changes destination register (RA in most cases)
			 * to return address */
			tf->gpr[rd] = cont_pc;
			/* fallthru */
		case FN_JR:
			/* JR and JALR sets target to where the source register
			 * is pointing */
			return tf->gpr[rs];
		default:
			goto fail;
		}
		break;
	case OP_JAL:
		tf->gpr[_RA] = cont_pc;
		/* fallthru */
	case OP_J:
		return (pc & ~J_ACTUAL_INDEX_MASK) | J_INDEX(insn);
	}

	/* Handle branches */
	rs = I_RS(insn);
	offset = I_IOFFSET(insn);
	/* Offset are added to the address of delay slot, not the branch itself,
	 * as described in MIPS64 manual */
	branch_pc = pc + offset + 4;

	switch (opcode) {
	case OP_REGIMM:
		func = I_FUNC(insn);
		switch (func) {
		case FN_BLTZAL:
		case FN_BLTZALL:
			if ((long)tf->gpr[rs] < 0)
				/* question: will RA be overwritten even if a
				 * branch doesn't happen? */
				tf->gpr[_RA] = cont_pc;
			/* fallthru */
		case FN_BLTZ:
		case FN_BLTZL:
			if ((long)tf->gpr[rs] < 0)
				return branch_pc;
			else
				return cont_pc;
			break;
		case FN_BGEZAL:
		case FN_BGEZALL:
			if ((long)tf->gpr[rs] >= 0)
				tf->gpr[_RA] = cont_pc;
			/* fallthru */
		case FN_BGEZ:
		case FN_BGEZL:
			if ((long)tf->gpr[rs] >= 0)
				return branch_pc;
			else
				return cont_pc;
			break;
		default:
			goto fail;
		}
		break;
	case OP_BLEZ:
	case OP_BLEZL:
		if (tf->gpr[rs] <= 0)
			return branch_pc;
		else
			return cont_pc;
		break;
	case OP_BGTZ:
	case OP_BGTZL:
		if (tf->gpr[rs] > 0)
			return branch_pc;
		else
			return cont_pc;
		break;
	}

	/* Handle beq and bne */
	rt = I_RT(insn);
	switch (opcode) {
	case OP_BEQ:
	case OP_BEQL:
		if (tf->gpr[rs] == tf->gpr[rt])
			return branch_pc;
		else
			return cont_pc;
		break;
	case OP_BNE:
	case OP_BNEL:
		if (tf->gpr[rs] != tf->gpr[rt])
			return branch_pc;
		else
			return cont_pc;
		break;
	}

fail:
	kprintf("instruction address = %016x\r\n", pc);
	kprintf("instruction content = %08x\r\n", insn);
	panic("branch_target(): not a branch instruction\r\n");
	/* NOTREACHED */
	return 0;
}

/*
 * Skips the current exception victim instruction and move on if necessary,
 * i.e. after system call or some of breakpoints.
 *
 * This routine should be called after everything is done.
 */
static void __skip_victim(struct trapframe *tf)
{
	/* Check if victim instruction is inside a branch delay slot */
	if (tf->cause & CR_BD)
		tf->epc = __branch_target(tf, tf->epc);
	else
		tf->epc += 4;
}

void trap_handler(struct trapframe *regs)
{
	switch (EXCCODE(regs->cause)) {
	case EC_sys:
		__skip_victim(regs);
		local_irq_enable();
		handle_syscall((long)regs->cause);
		/*
		 * After executing ERET instruction MIPS processor return to
		 * the exception-throwing instruction (called _victim_), or
		 * the branching instruction directly preceding the victim in
		 * case of throwing an exception inside branch delay slot.
		 *
		 * In case of system calls, if we do not manually compute
		 * the desired return address (that is, directly following
		 * the system call instruction), the processor will execute
		 * SYSCALL instruction again and again.
		 *
		 * Unfortunately, dealing with system calls inside a branch
		 * delay slot involves disassembling the branching instruction
		 * and predicting the branch target, which is handled in
		 * __skip_victim().
		 */
		break;
	case EC_bp:
		/*
		 * As a test for __skip_victim(), we skip the victim to resume
		 * execution when handling breakpoints.
		 * We can add various debugging routines (or deliberate
		 * backdoors) later for breakpoints.
		 */
		__skip_victim(regs);
		break;
	case EC_int:
		//if (handle_interrupt(regs->cause) == 0)
		handle_interrupt((long)regs->cause);
			break;
		/* else fallthru */
	default:
		dump_regs(regs);
		panic("Unexpected trap\n");
	}
	trap_return(regs);
}

__noreturn
extern void trap_exit(struct trapframe *regs);

__noreturn void trap_return(struct trapframe *regs)
{
	/*
	 * Retain interrupt masks while changing other fields according to
	 * register set.
	 *
	 * Also, we want to reenable ST_EXL since we cleared it before
	 * entering C trap handler.
	 */
	uint32_t status = read_c0_status();
	uint32_t im = status & ST_IM;
	status &= ~ST_EXCM;
	write_c0_status(status);
	regs->status = (regs->status & ~ST_IM) | im;
	trap_exit(regs);
}

void bsp_trap_init()
{
	
}
