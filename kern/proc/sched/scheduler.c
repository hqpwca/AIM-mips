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
#include <aim/initcalls.h>
#include <aim/vmm.h>
#include <aim/pmm.h>

static struct list_head __head;
static struct list_head *head;

static struct proc * __pick(void)
{
	struct proc *a;
	if(list_empty(head))
		return NULL;

	for_each_entry_reverse(a, head, sched_node) {
		if(a->state == PS_RUNNABLE)
			break;
	}

	return a;
}

static int __add(struct proc *p0)
{
	list_init(&p0->sched_node);
	list_add(p0, head);
}

static int __remove(struct proc *p0)
{
	list_del(&p0->sched_node);
}

static struct proc * __next(struct proc *p0)
{
	struct proc *res = next_entry(p0, sched_node);
	if(res == head) return NULL;
	return res;
}

static struct proc * __find(pid_t pid, struct namespace *ns)
{
	struct proc *a;

	for_each_entry(a, head, sched_node) {
		if(a->pid == pid) break;
	}

	return a;
}

static int __init(void)
{
	kpdebug("initializing scheduler\n");

	list_init(&__head);
	head = &__head;
	struct scheduler sch0 = {
		.pick = __pick,
		.add = __add,
		.remove = __remove,
		.next = __next,
		.find = __find
	};

	set_scheduler(&sch0);

	return 0;
}

INITCALL_SCHED(__init);