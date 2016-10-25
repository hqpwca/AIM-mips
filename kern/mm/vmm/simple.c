#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
#include <aim/vmm.h>
#include <aim/mmu.h>
#include <aim/panic.h>
#include <libc/string.h>

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

static inline void *alloc(size_t size, gfp_t flags)
{
	return NULL;
}
static inline void free(void *obj)
{
	
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
	struct simple_allocator simple = {
		.alloc	= alloc,
		.free	= free,
		.size	= size
	};
	set_simple_allocator(&simple);
	
	return 0;
}
