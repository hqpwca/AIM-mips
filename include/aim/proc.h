/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
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

#ifndef _PROC_H
#define _PROC_H

#include <aim/limits.h>
#include <aim/namespace.h>
#include <aim/uvm.h>
#include <context.h>
//#include <list.h>
//#include <file.h>

struct proc {
	/* TODO: move thread-specific data into a separate structure */
	/*
	 * the kernel stack pointer is used to prepare C runtime, thus accessed
	 * in assembly. Force it here at offset 0 for easy access.
	 */
	void *kstack; /* bottom, or lower address */
	size_t kstack_size;

	/* other stuff go here */
	int		tid;	/* Thread ID (unused - for multithreading) */
	pid_t		pid;	/* Process ID within namespace @namespace */
	pid_t		kpid;	/* Process ID */
	unsigned int	state;	/* Process state (runnability) */
	/* The state values come from OpenBSD */
	/* TODO: may have more...? */
#define PS_EMBRYO	1	/* Just created */
#define PS_RUNNABLE	2	/* Runnable */
#define PS_SLEEPING	3	/* Currently sleeping */
#define PS_ZOMBIE	4
#define	PS_ONPROC	5
	unsigned int	exit_code;	/* Exit code */
	unsigned int	exit_signal;	/* Signal code */
	uint32_t	flags;	/* Flags */
	/* TODO: may have more...? */
#define PF_EXITING	0x00000004	/* getting shut down */
#define PF_SIGNALED	0x00000400	/* killed by a signal */
#define PF_KTHREAD	0x00200000	/* I am a kernel thread */
	int		oncpu;		/* CPU ID being running on */
#define CPU_NONE	-1
	uintptr_t	bed;		/* object we are sleeping on */
	struct namespace *namespace;	/* Namespace */
	struct mm 	*mm; /* Memory mapping structure including pgindex */
	/*
	 * Expandable heap is placed directly above user stack.
	 * User stack is placed above all loaded program segments.
	 * We put program arguments above user stack.
	 */
	struct context	context;	/* Context before switch */
	size_t		heapsize;	/* Expandable heap size */
	void		*heapbase;	/* Expandable heap base */

	char		name[PROC_NAME_LEN_MAX];

	/* File system related */
	//struct vnode	*cwd;		/* current working directory */
	//struct vnode	*rootd;		/* root directory (chroot) */
	//union {
	//	struct {
	//		struct file *fstdin;	/* stdin */
	//		struct file *fstdout;	/* stdout */
	//		struct file *fstderr;	/* stderr */
	//	};
	//	struct file *fd[OPEN_MAX];	/* opened files */
	//};
	//lock_t		fdlock;		/* lock of file table */

	/* TODO: POSIX process groups and sessions.  I'm not entirely sure
	 * how they should look like. */
	struct proc	*groupleader;	/* Process group leader */
	struct proc	*sessionleader;	/* Session leader */

	/* Session data */
	struct tty_device *tty;		/* Controlling terminal */
	//struct vnode	*ttyvnode;	/* vnode of terminal */

	/* Process tree related */
	struct proc	*mainthread;	/* Main thread of the same process */
	struct proc	*parent;
	struct proc	*first_child;
	struct proc	*next_sibling;
	struct proc	*prev_sibling;

	//struct scheduler *scheduler;	/* Scheduler for this process */
	//struct list_head sched_node;	/* List node in scheduler */
};

static inline void *kstacktop(struct proc *proc)
{
	return proc->kstack + proc->kstack_size;
}

/* Create a struct proc inside namespace @ns and initialize everything if we
 * can by default. */
struct proc *proc_new(struct namespace *ns);
/* Exact opposite of proc_new */
void proc_destroy(struct proc *proc);
void proc_init(void);
/* Setup per-CPU idle process */
void idle_init(void);
void spawn_initproc(void);
pid_t pid_new(pid_t kpid, struct namespace *ns);
void pid_recycle(pid_t pid, struct namespace *ns);

/* The following are architecture-specific code */

/*
 * Setup a kernel process with entry and arguments.
 * A kernel process always works on its kernel stack and with default
 * kernel memory mapping.
 *
 * Only sets up process trap frame and context.
 *
 * For architecture developers: you are not required to implement
 * proc_ksetup().  You only need to provide arch-dependent code
 * __proc_ksetup() (see kern/proc/proc.c)
 */
void proc_ksetup(struct proc *proc, void *entry, void *args);
/*
 * Setup a user process with entry, stack top, and arguments.
 *
 * The user-mode counterpart of proc_ksetup().
 *
 * For architecture developers: You only need to provide arch-dependent
 * code __proc_usetup().
 */
void proc_usetup(struct proc *proc, void *entry, void *stacktop, void *args);
void switch_context(struct proc *proc);
/* Return to trap frame in @proc.  Usually called once by fork child */
void proc_trap_return(struct proc *proc);

/*
 * Process tree maintenance
 */
void proctree_add_child(struct proc *child, struct proc *parent);

#endif /* _PROC_H */

