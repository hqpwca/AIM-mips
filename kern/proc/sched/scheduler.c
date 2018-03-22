#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <raim/console.h>
#include <raim/debug.h>
#include <raim/panic.h>
#include <raim/percpu.h>
#include <raim/proc.h>
#include <raim/sched.h>
#include <raim/smp.h>
#include <raim/initcalls.h>
#include <raim/vmm.h>
#include <raim/pmm.h>

static struct list_head __head;
static struct list_head *head;
static struct proc * __pick(void);
static int __add(struct proc *p0);
static int __remove(struct proc *p0);
static struct proc * __next(struct proc *p0);
static struct proc * __find(pid_t pid, struct namespace *ns);

static struct scheduler plain_scheduler = {
	.pick = __pick,
	.add = __add,
	.remove = __remove,
	.next = __next,
	.find = __find
};

static struct proc * __pick(void)
{
	struct proc *a;

	for_each_entry_reverse(a, head, sched_node) {
		if(a->state == PS_RUNNABLE)
			break;
	}

	if((struct list_head *)a != head) return a;
	return NULL;
}

static int __add(struct proc *p0)
{
	list_init(&p0->sched_node);
	list_add(&p0->sched_node, head);

	return 0;
}

static int __remove(struct proc *p0)
{
	list_del(&p0->sched_node);

	return 0;
}

static struct proc * __next(struct proc *p0)
{
	struct proc *res = next_entry(p0, sched_node);
	if((struct list_head *)res == head) return NULL;
	return res;
}

static struct proc * __find(pid_t pid, __unused struct namespace *ns)
{
	struct proc *a;

	for_each_entry(a, head, sched_node) {
		if(a->pid == pid) break;
	}

	if((struct list_head *)a != head)return a;
	return NULL;
}

static int __init(void)
{
	kpdebug("initializing scheduler\n");

	list_init(&__head);
	head = &__head;
	/*
	scheduler->pick = __pick;
	scheduler->add = __add;
	scheduler->remove = __remove;
	scheduler->next = __next;
	scheduler->find = __find;
	*/

	return 0;
}

INITCALL_SCHED(__init);

struct scheduler *scheduler = &plain_scheduler;
