/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 *
 * This file is part of AIM.
 *
 * AIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/proc.h>
#include <aim/percpu.h>
#include <aim/console.h>
#include <aim/smp.h>
#include <aim/trap.h>
#include <arch-trap.h>
#include <arch-smp.h>
#include <context.h>

void sched_exit_critical(void);

extern void forkret(void);

void forkret(void)
{
	kpdebug("fork return here.\n");

	sched_exit_critical();
	proc_trap_return(current_proc);
}

extern void switch_regs(struct context *old, struct context *new);

static struct trapframe *__proc_trapframe(struct proc *proc)
{
	struct trapframe *tf;

	tf = (struct trapframe *)(kstacktop(proc) - sizeof(*tf));
	return tf;
}

static void __bootstrap_trapframe(struct trapframe *tf,
				   void *entry,
				   void *stacktop,
				   void *args)
{
	tf->status = read_c0_status();
 	tf->cause = read_c0_cause();
 	/* Enable interrupts in trap frame */
	tf->status |= ST_IE | ST_EXL;
	tf->epc = (unsigned long)entry;
	tf->gpr[_T9] = tf->epc;
	tf->gpr[_SP] = (unsigned long)stacktop;
	tf->gpr[_A0] = (unsigned long)args;
}

static void __bootstrap_context(struct context *context, struct trapframe *tf)
{
	context->status = read_c0_status();
 	context->cause = read_c0_cause();
	/* t9 is the register storing function entry address in PIC */
	context->gpr[_T9] = context->gpr[_RA] = (unsigned long)forkret;
	/* Kernel stack pointer just below trap frame */
	context->gpr[_SP] = (unsigned long)tf;
}

static void __bootstrap_user(struct trapframe *tf)
{
	tf->status = (tf->status & ~ST_KSU) | KSU_USER;
}

void __proc_ksetup(struct proc *proc, void *entry, void *args)
{
	struct trapframe *tf = __proc_trapframe(proc);
	__bootstrap_trapframe(tf, entry, kstacktop(proc), args);
	__bootstrap_context(&(proc->context), tf);
}

void __proc_usetup(struct proc *proc, void *entry, void *stacktop, void *args)
{
	struct trapframe *tf = __proc_trapframe(proc);
	__bootstrap_trapframe(tf, entry, stacktop, args);
	__bootstrap_context(&(proc->context), tf);
	__bootstrap_user(tf);
}

void __prepare_trapframe_and_stack(__unused struct trapframe *tf, __unused void *entry,
    __unused void *ustacktop, __unused int argc, __unused char *argv[], __unused char *envp[])
{
}

void proc_trap_return(struct proc *proc)
{
	struct trapframe *tf = __proc_trapframe(proc);

	trap_return(tf);
}

void __arch_fork(struct proc *child, struct proc *parent)
{
	struct trapframe *tf_child = __proc_trapframe(child);
	struct trapframe *tf_parent = __proc_trapframe(parent);

	*tf_child = *tf_parent;
	tf_child->gpr[_V0] = 0;
	tf_child->gpr[_V1] = 0;
	/* fill return value here */

	__bootstrap_context(&(child->context), tf_child);
}

void switch_context(struct proc *proc)
{
	struct proc *current = current_proc;
	current_proc = proc;

	/* Switch page directory */
	switch_pgindex(proc->mm->pgindex);

	current_kernelsp = (unsigned long)kstacktop(proc);
	/* Switch general registers */
	switch_regs(&(current->context), &(proc->context));
}
