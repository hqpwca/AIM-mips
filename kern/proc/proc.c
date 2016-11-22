/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 * Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
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
#include <libc/string.h>
#include <aim/debug.h>
#include <aim/namespace.h>
#include <aim/panic.h>
#include <aim/percpu.h>
#include <aim/vmm.h>
#include <aim/proc.h>
//#include <aim/sched.h>
#include <aim/smp.h>
//#include <aim/sync.h>
#include <aim/uvm.h>
#include <bitmap.h>

static struct {
	//lock_t lock;
	DECLARE_BITMAP(bitmap, MAX_PROCESSES);
} freekpid;

struct proc idleproc[MAX_CPUS];

/*
 * This should be a seperate function, don't directly use kernel memory
 * interfaces. There are different sets of interfaces we can allocate memory
 * from, and we can't say any one of them is best.
 */
void *alloc_kstack(void)
{
	/* currently we use pgalloc() */
	addr_t paddr;

	paddr = pgalloc();
	if (paddr == -1)
		return NULL;
	return (void *)pa2kva(paddr);
}

/* Exact opposite of alloc_kstack */
void free_kstack(void *kstack)
{
	pgfree(kva2pa(kstack));
}

void *alloc_kstack_size(size_t *size)
{
	/* calculate the actual size we allocate */
	panic("custom kstack size not implemented\n");
}

/* Find and remove the first available KPID from the free KPID set. */
static pid_t kpid_new(void)
{
	//unsigned long flags;
	pid_t kpid;

	//spin_lock_irq_save(&freekpid.lock, flags);
	kpid = bitmap_find_first_zero_bit(freekpid.bitmap, MAX_PROCESSES);
	atomic_set_bit(kpid, freekpid.bitmap);
	//spin_unlock_irq_restore(&freekpid.lock, flags);

	return kpid;
}

static void kpid_recycle(pid_t kpid)
{
	atomic_clear_bit(kpid, freekpid.bitmap);
}

struct proc *proc_new(struct namespace *ns)
{
	struct proc *proc = (struct proc *)kmalloc(sizeof(*proc), 0);

	if (proc == NULL)
		return NULL;

	proc->kstack = alloc_kstack();
	if (proc->kstack == NULL)
		goto rollback_proc;

	proc->kstack_size = PAGE_SIZE;

	proc->kpid = kpid_new();
	/* TODO: change this in case of implementing namespaces */
	proc->pid = pid_new(proc->kpid, ns);
	proc->tid = proc->kpid;
	proc->state = PS_EMBRYO;
	proc->exit_code = 0;
	proc->exit_signal = 0;
	proc->flags = 0;
	proc->oncpu = CPU_NONE;
	proc->bed = NULL;
	proc->namespace = ns;
	proc->mm = NULL;
	memset(&(proc->context), 0, sizeof(proc->context));
	proc->heapsize = 0;
	proc->heapbase = NULL;
	memset(&(proc->name), 0, sizeof(proc->name));
	//proc->cwd = proc->rootd = NULL;
	//memset(&proc->fd, 0, sizeof(proc->fd));
	//spinlock_init(&proc->fdlock);

	proc->tty = NULL;
	//proc->ttyvnode = NULL;

	proc->mainthread = NULL;
	proc->groupleader = NULL;
	proc->sessionleader = NULL;
	proc->parent = NULL;
	proc->first_child = NULL;
	proc->next_sibling = NULL;
	proc->prev_sibling = NULL;
	//proc->scheduler = scheduler;
	//list_init(&(proc->sched_node));

	return proc;
rollback_proc:
	kfree(proc);
	return NULL;
}

void proc_destroy(struct proc *proc)
{
	pid_recycle(proc->pid, proc->namespace);
	kpid_recycle(proc->kpid);
	free_kstack(proc->kstack);
	kfree(proc);
}

/* Arch-portion code there */
extern void __proc_ksetup(struct proc *proc, void *entry, void *args);
void proc_ksetup(struct proc *proc, void *entry, void *args)
{
	proc->mm = kernel_mm;
	__proc_ksetup(proc, entry, args);
}

/* Arch-portion code there */
extern void __proc_usetup(struct proc *proc, void *entry, void *stacktop,
    void *args);
void proc_usetup(struct proc *proc, void *entry, void *stacktop, void *args)
{
	__proc_usetup(proc, entry, stacktop, args);
}

void proc_init(void)
{
	//spinlock_init(&freekpid.lock);
}

void idle_init(void)
{
	current_proc = cpu_idleproc;
	cpu_idleproc->state = PS_RUNNABLE;
	cpu_idleproc->mm = kernel_mm;
	cpu_idleproc->kpid = 0;
}

