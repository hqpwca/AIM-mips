#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
//#include <aim/sync.h>
#include <aim/mmu.h>
#include <aim/pmm.h>
#include <libc/string.h>
#include <util.h>

//LRU Clock;

struct mem_block {
	struct mem_block *next;
};

static struct mem_block *now;
static addr_t free_size;

static inline int alloc(struct pages *pages)
{
	struct mem_block *a;

	if(now == NULL)
		return -1;
	if(pages->size % PAGE_SIZE)
		return -1;
	if(pages->size > free_size)
		return -1;
	
	
	pages->paddr = (addr_t)now;
	lsize_t psize = pages->size;
	
		
	return 0;
}

static inline void free(struct pages *pages)
{
	
}

static inline addr_t get_free(void)
{
	return 0;
}

int page_allocator_init(void)
{
	free_size = 0;
	now = NULL;
	
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
	
}
