#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/param.h>
#include <aim/init.h>
#include <aim/mmu.h>
#include <aim/panic.h>
#include <aim/trap.h>
#include <aim/console.h>
#include <arch-mmu.h>
#include <arch-trap.h>
#include <libc/string.h>

void trap_init()
{

}

void trap_return(struct trapframe *tf)
{
	
}