#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
#include <aim/vmm.h>
#include <aim/mmu.h>
#include <aim/panic.h>
#include <aim/console.h>
#include <libc/string.h>

#define SIZE_NUM 9

static void *top,*bottom;
static size_t ssize;

static inline void *simple_alloc(size_t size, gfp_t flags)
{
	if(top + size > bottom + ssize)
		return NULL;
	void *res = top;
	top += size;
	return res;
}

static inline void simple_free(void *obj) {return;}

static inline size_t simple_size(void *obj) {return 0;}

struct frisys
{
	struct frisys *lc,*rc;
	void *paddr;
	size_t size,remain;
};
static size_t fri_size[SIZE_NUM] = {0x1000,0x800,0x400,0x200,0x100,0x80,0x40,0x20,0x10};

struct alloced_page
{
	struct alloced_page *prev,*next;
	struct frisys *root;
};

static struct alloced_page *head,*tail;
static struct alloced_page h,t;
static void *now_plain_page;
static int offset;

static inline void *new_plain_page()
{
	addr_t paddr = pgalloc();
	offset = 0;
	if(paddr == -1)return NULL;
	else return (void *)paddr;
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
		void *obj = now_plain_page;
		offset += size;
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

static inline void *frisys_new_node(void *paddr, size_t sz)
{	
	struct frisys *a = get_plain_object(sizeof(struct frisys));
	a->lc = a->rc = NULL;
	a->size = a->remain = sz;
	a->paddr = paddr;
	
	return (void *)a;
}

static inline void *frisys_get_space(struct frisys *now, size_t sz)
{
	if(sz > now->remain)return NULL;
	if(sz == now->size){
		now->remain = 0;
		return now->paddr;
	}
	
	void *obj = NULL;
	
	if(now->lc == NULL) now->lc = frisys_new_node(now->paddr, now->size>>1);
	obj = frisys_get_space(now->lc, sz, now->size>>1);
	if(obj != NULL) {
		relax_up(now);
		return obj;
	}
	
	if(now->rc == NULL) now->rc = frisys_new_node(now->paddr + (now->size>>1), now->size>>1);
	obj = frisys_get_space(now->rc, sz, now->size>>1);
	if(obj != NULL) {
		relax_up(now);
		return obj;
	}
	
	return NULL;
}

static inline void *frisys_walk(struct frisys *now, void *obj)
{
	
}

static inline void *frisys_free_space(struct frisys *now, struct frisys *obj)
{
	
}

static inline void *alloc(size_t size, gfp_t flags)
{
	if(size == 0 || size > PAGE_SIZE) return NULL;
	
	int ii;
	for(ii = SIZE_NUM - 1; ii >= 0; --ii)
		if(fri_size[ii] > size)
			break;
	size = fri_size[ii];
	
	struct alloced_page *a;
	
	void *obj;
	for(a = head->next; a != tail; a = a->next)
		if(a->root->remain >= size)
			obj = frisys_get_space(a->root, size);
	if(a != tail)
		return obj;
	
	a = get_plain_object(sizeof(struct alloced_page));
	a->prev = head;
	a->next = head->next;
	head->next->prev = a;
	head->next = a;
	
	void *new_page = pgalloc();
	a->root = frisys_new_node(new_page,PAGE_SIZE);
	obj = frisys_get_space(a->root, size);
	
	return obj;
}
static inline void free(void *obj)
{
	static inline 
}

static inline size_t size(void *obj) 
{
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
