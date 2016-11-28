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

#ifndef _SCHED_H
#define _SCHED_H

#ifndef __ASSEMBLER__

#include <aim/namespace.h>
#include <aim/sync.h>

struct proc;

/* struct scheduler serves like abstract class in Java */
struct scheduler {
	/* Initialization routine should be registered statically as
	 * initcalls.  FIXME: ??? */

	/*
	 * All the following routines are atomic.
	 *
	 * Moreover, pick() method assumes that the caller is inside a
	 * critical section.
	 */

	/*
	 * Return a proc to be scheduled onto processor, or NULL to
	 * indicate that the processor should be idle.
	 */
	struct proc *	(*pick)(void);
	int		(*add)(struct proc *);
	/* Remove a specific proc from proc list */
	int		(*remove)(struct proc *);
	/* Retrieve next proc of @proc, NULL for first proc */
	struct proc *	(*next)(struct proc *);
	/*
	 * Locate the proc with PID @id and namespace @ns.
	 * Currently @ns should be always NULL.
	 */
	struct proc *	(*find)(pid_t pid, struct namespace *ns);
};

extern struct scheduler *scheduler;

void sched_enter_critical(void);
void sched_exit_critical(void);

void sched_init(void);
void schedule(void);

void proc_add(struct proc *proc);
void proc_remove(struct proc *proc);
struct proc *proc_next(struct proc *proc);

/* sleep()'s and wakeup()'s */
void sleep(void *bed);
void sleep_with_lock(void *bed, lock_t *lock);
void wakeup(void *bed);

#endif

#endif

