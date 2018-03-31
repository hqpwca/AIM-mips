// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/console.h>
#include <mach-platform.h>
#include <raim/mmu.h>


static int bbl_putchar(int c)
{
    sbi_console_putchar(c & 0xFF);
    return 0;
}

static void set_bbl_console()
{
    set_console(bbl_putchar, DEFAULT_KPUTS);
    //kprintf("bbl_putchar located at: %016llx\n", (uint64_t)bbl_putchar);
}
int __early_console_init(struct bus_device *bus, addr_t base, addr_t mapped_base)
{
    set_bbl_console();
    jump_handlers_add(set_bbl_console);
    return 0;
}
