// by ZBY
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */


#include <sys/types.h>
#include <raim/mmu.h>
#include <raim/early_kmmap.h>
#include <libc/string.h>

bool early_mapping_valid(__unused struct early_mapping *entry)
{
	return true;
}

void page_index_clear(pgindex_t *index)
{
    memset(index, 0, sizeof(*index));
}

int page_index_early_map(pgindex_t *index, addr_t paddr, void *vaddr, size_t length)
{
	return 0;
}
