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
#endif

#include <sys/types.h>
#include <aim/console.h>
#include <aim/debug.h>
#include <aim/panic.h>
#include <aim/percpu.h>
#include <aim/proc.h>
#include <aim/sched.h>
#include <aim/smp.h>

//static lock_t sched_lock;
//static unsigned long __sched_intrflags;

void sched_enter_critical(void)
{
	//spin_lock_irq_save(&sched_lock, __sched_intrflags);
}

void sched_exit_critical(void)
{
	//spin_unlock_irq_restore(&sched_lock, __sched_intrflags);
}

static void __update_proc(struct proc *proc)
{
	proc->state = PS_ONPROC;
	proc->oncpu = cpuid();
}

void schedule(void)
{
	struct proc *oldproc = current_proc, *newproc;

	sched_enter_critical();

	assert(!((oldproc->kpid == 0) ^ (oldproc == cpu_idleproc)));
	if (oldproc->state == PS_ONPROC)
		oldproc->state = PS_RUNNABLE;

	newproc = scheduler->pick();
	if (newproc == NULL)
		newproc = cpu_idleproc;

	__update_proc(newproc);

	switch_context(newproc);

	sched_exit_critical();
}

/*
 * Put the process holding lock @lock asleep on the bed @bed, releasing the
 * lock during the routine.
 * When woken up, the process reacquires the lock.
 */
void sleep_with_lock(void *bed, lock_t *lock)
{
	//unsigned long flags;
	/* Basically, sleep is also a kind of scheduling, so we need to enter
	 * critical section of scheduler as well. */
	//assert(lock != &sched_lock);
	kpdebug("sleeping on %p with lock %p\n", bed, lock);
	//spin_lock_irq_save(&sched_lock, flags);
	if (lock != NULL)
		//spin_unlock(lock);

	current_proc->bed = bed;
	current_proc->state = PS_SLEEPING;

	/* Switch to CPU IDLE - it will do the rescheduling for us. */
	switch_context(cpu_idleproc);

	current_proc->bed = NULL;

	//spin_unlock_irq_restore(&sched_lock, flags);
	if (lock != NULL)
		//spin_lock(lock);
	kpdebug("slept on %p with lock %p\n", bed, lock);
}

void sleep(void *bed)
{
	sleep_with_lock(bed, NULL);
}

/*
 * Wake up all processes sleeping on the given bed.
 */
void wakeup(void *bed)
{
	struct proc *proc;
	//unsigned long flags;

	/* We are changing process states.  It is better to enter critical
	 * section. */
	//spin_lock_irq_save(&sched_lock, flags);

	for (proc = proc_next(NULL); proc; proc = proc_next(proc)) {
		if (proc->bed == bed && proc->state == PS_SLEEPING)
			proc->state = PS_RUNNABLE;
	}
	kpdebug("woke up %p\n", bed);

	//spin_unlock_irq_restore(&sched_lock, flags);
}

void proc_add(struct proc *proc)
{
	scheduler->add(proc);
}

void proc_remove(struct proc *proc)
{
	scheduler->remove(proc);
}

struct proc *proc_next(struct proc *proc)
{
	return scheduler->next(proc);
}

void sched_init(void)
{
	//spinlock_init(&sched_lock);
}

