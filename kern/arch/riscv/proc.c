/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 *
 * This file is part of RAIM.
 *
 * RAIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RAIM is distributed in the hope that it will be useful,
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
#include <raim/proc.h>
#include <raim/percpu.h>
#include <raim/console.h>
#include <raim/smp.h>
#include <raim/trap.h>
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
unimpl();
}

static void __bootstrap_context(struct context *context, struct trapframe *tf)
{
unimpl();
}

static void __bootstrap_user(struct trapframe *tf)
{
unimpl();
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
unimpl();
	/* fill return value here */

	__bootstrap_context(&(child->context), tf_child);
}

void switch_context(struct proc *proc)
{
	struct proc *current = current_proc;
	current_proc = proc;

	/* Switch page directory */
	switch_pgindex(proc->mm->pgindex);

unimpl();
	/* Switch general registers */
	switch_regs(&(current->context), &(proc->context));
}
