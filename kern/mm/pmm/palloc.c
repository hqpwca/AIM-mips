#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
//#include <aim/sync.h>
#include <aim/mmu.h>
#include <aim/pmm.h>
#include <aim/vmm.h>
#include <aim/panic.h>
#include <libc/string.h>
#include <util.h>

//LRU Clock;

struct mem_block {
	struct mem_block *prev,*next;
	struct pages;
};

static struct mem_block *head,*tail;
static struct mem_block h,t;
static addr_t free_size;

static inline int alloc(struct pages *pages)
{
	struct mem_block *a;

	if(pages->size % PAGE_SIZE)
		return -1;
	if(pages->size > free_size)
		return -1;
	
	for(a = head->next; a != tail; a = a->next)
		if(a->size >= pages->size)
			break;

	if(a == tail) return -1;

	pages->paddr = a->paddr;
	a->paddr += pages->size;
	a->size -= pages->size;
	
	if(a->size == 0)
	{
		a->prev->next = a->next;
		a->next->prev = a->prev;
		kfree(a);
	}

	free_size -= pages->size;
		
	return 0;
}

static inline void free(struct pages *pages)
{
	struct mem_block *a,*i;

	if(pages->size % PAGE_SIZE)
		return;
	if(pages->size % PAGE_SIZE)
		return;

	a = kmalloc(sizeof(struct mem_block),0);
	if(a == NULL)
		panic("simple1 used up.\n");
	a->paddr = pages->paddr;
	a->size = pages->size;
	
	for(i = head->next; i != tail; i = i->next)
		if(i->paddr >= a->paddr)
			break;

	a->prev = i->prev;
	a->next = i;
	i->prev->next = a;
	i->prev = a;

	if(a->prev != head && a->prev->paddr + a->prev->size == a->paddr)
	{
		a->prev->size += a->size;
		a->prev->next = a->next;
		a->next->prev = a->prev;
		i = a->prev;
		kfree(a);
		a = i;
	}

	if(a->next != tail && a->paddr + a->size == a->next->paddr)
	{
		a->size += a->next->size;
		a->next->next->prev = a;
		a->next = a->next->next;
		kfree(a->next);
	}

	free_size += pages->size;
}

static inline addr_t get_free(void)
{
	return free_size;
}

int page_allocator_init(void)
{
	free_size = 0;
	head = &h, tail = &t;
	head->prev = tail->next = NULL;
	head->next = tail;
	tail->prev = head;
	
	struct page_allocator pa = {
		.alloc		= alloc,
		.free		= free,
		.get_free	= get_free
	};
	set_page_allocator(&pa);
	return 0;
}

int page_allocator_move(struct simple_allocator *old)
{
	struct mem_block *now,*new;
	for(now = head->next; now != tail; now = now->next)
	{
		new = kmalloc(sizeof(struct mem_block),0);
		if(new == NULL)
		{
			panic("simple2 used up.\n");
			return -1;
		}
		new->paddr = now->paddr;
		new->size = now->size;
		new->prev = now->prev;
		new->next = now->next;
		now->prev->next = new;
		now->next->prev = new;
		old->free(now);
		now = new;
	}

	return 0;
}
