#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
#include <raim/mmu.h>
#include <raim/vmm.h>
#include <raim/pmm.h>
#include <raim/panic.h>
#include <raim/console.h>
#include <libc/string.h>

#define SIZE_NUM 9
#define BIT_SIZE 0x10
#define PG_EMPTY 0x7ffff

static void *top,*bottom;
static size_t ssize;

static inline void *simple_alloc(size_t size, __unused gfp_t flags)
{
	if(top + size > bottom + ssize)
		return NULL;
	void *res = top;
	top += size;
	return res;
}

static inline void simple_free(__unused void *obj) { return; }

static inline size_t simple_size(__unused void *obj) { return 0; }

static size_t fri_size[SIZE_NUM] = {0x1000,0x800,0x400,0x200,0x100,0x80,0x40,0x20,0x10};

struct alloced_page
{
	struct alloced_page *prev,*next;
	void *paddr;
	size_t size;
	int num;
	uint8_t status[32];
};

static struct alloced_page *head,*tail;
static struct alloced_page h,t;
static void *now_plain_page;
static size_t offset;

static inline void *new_plain_page()
{
	addr_t paddr = (addr_t)pgalloc();
//	kpdebug("new_plain_page: 0x%llx\n", (uint64_t)paddr);
	offset = 0;
	if (paddr == (addr_t)-1) return NULL;
	else return (void *)(addr_t)pa2kva(paddr);
}

static inline void *get_plain_object(size_t size)
{
	if(size > PAGE_SIZE)
	{
		panic("Plain OBJECT too large.\n");
		return NULL;
	}
	if(offset + size <= PAGE_SIZE)
	{
		void *obj = now_plain_page + offset;
		offset += size;
		//kpdebug("new_plain_object: 0x%llx, size: %llx\n", (uint64_t)obj, (uint64_t)size);
		return obj;
	}
	else
	{
		now_plain_page = new_plain_page();
		if(now_plain_page == NULL)
			panic("Page Allocator Full or An error occurred.\n");
		return now_plain_page;
	}
}

#define bit(x, a) ((x[a>>3] >> (a & 0x7)) & 1)
#define setbit(x, a) x[a>>3] ^= (unsigned char)(1U << (a & 0x7))

static inline size_t get_space(uint8_t *status, size_t sz)
{
	int leap = (int)sz / BIT_SIZE;
	
	int i;
	for(i = 0; i * BIT_SIZE < PAGE_SIZE; i += leap)
		if(!bit(status,i))
		{
			//kpdebug("Alloc: Before_set: i: %d, bit: %d\n", i, bit(status,i));
			setbit(status,i);
			//kpdebug("Alloc: After_set: i: %d, bit: %d\n", i, bit(status,i));
			break;
		}
	
	return BIT_SIZE * (size_t)i;
}

static inline void free_space(uint8_t *status, int pos)
{	
	if(bit(status,pos))
	{
		//kpdebug("Free: Before_set: i: %d, bit: %d\n", pos, bit(status,pos));
		setbit(status,pos);
		//kpdebug("Free: After_set: i: %d, bit: %d\n", pos, bit(status,pos));
	}
}

static inline void *alloc(size_t size, __unused gfp_t flags)
{
	//kpdebug("Alloc: head: 0x%x, tail: 0x%x\n", head, tail);

	if(size == 0 || size > PAGE_SIZE) return NULL;
	
	int ii;
	for(ii = SIZE_NUM - 1; ii >= 0; --ii)
		if(fri_size[ii] >= size)
			break;
	size = fri_size[ii];
	
	struct alloced_page *a;
	void *obj;
	size_t off;
	
	for(a = head->next; a != tail; a = a->next)
		if(size == a->size && (size_t)a->num * a->size < PAGE_SIZE)
			break;
	
	if(a == tail)
		for(a = head->next; a != tail; a = a->next)
			if(a->size == PG_EMPTY)
				break;
	
	if(a->size == PG_EMPTY || a == tail)
	{
		void *new_page = (void *)(addr_t)pa2kva(pgalloc());
		if(new_page == NULL) return NULL;
	
		if(a == tail)
		{
			a = get_plain_object(sizeof(struct alloced_page));
			a->prev = head;
			a->next = head->next;
			head->next->prev = a;
			head->next = a;
		}
		a->paddr = new_page;
		a->size = size;
		a->num = 0;
		memset(a->status,0,sizeof(a->status));
	}
	
	off = get_space(a->status, size);
	a->num ++;
	obj = a->paddr + off;

	//kpdebug("Alloc: head: 0x%x, tail: 0x%x, obj: 0x%x\n", head, tail, obj);
	
	return obj;
}
static inline void free(void *obj)
{
	//kpdebug("Free: head: 0x%x, tail: 0x%x, obj: 0x%x\n", head, tail, obj);
	if(obj == NULL) return;
	
	struct alloced_page *a;
	for(a = head->next; a != tail; a = a->next)
		if(obj >= a->paddr && obj < a->paddr + PAGE_SIZE)
			break;
	if(a == tail)
		panic("Free: Can't find object\n");
	int pos = (obj - a->paddr)/BIT_SIZE;
	
	free_space(a->status, pos);
	a->num --;
	
	if(a->num == 0)
	{
		a->size = PG_EMPTY;
		pgfree((addr_t)(kva2pa(a->paddr)));
	}
}

static inline size_t size(void *obj) 
{
	struct alloced_page *a;
	for(a = head->next; a != tail; a = a->next)
		if(obj >= a->paddr && obj < a->paddr + PAGE_SIZE)
			return a->size;
	return 0;
}

int simple_allocator_bootstrap(void *pt, size_t size)
{
	top = bottom = pt;
	ssize = size;
	
	struct simple_allocator simple1 = {
		.alloc	= simple_alloc,
		.free	= simple_free,
		.size	= simple_size
	};
	set_simple_allocator(&simple1);
	
	return 0;
}

int simple_allocator_init(void)
{
	head = &h, tail = &t;
	head->prev = tail->next = NULL;
	head->next = tail;
	tail->prev = head;
	
	now_plain_page = new_plain_page();
	
	struct simple_allocator simple = {
		.alloc	= alloc,
		.free	= free,
		.size	= size
	};
	set_simple_allocator(&simple);
	
	return 0;
}
