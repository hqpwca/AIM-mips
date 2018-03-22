// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/console.h>
#include <mach-platform.h>


static int bbl_putchar(int c)
{
    sbi_console_putchar(c & 0xFF);
    return 0;
}

int __early_console_init(struct bus_device *bus, addr_t base, addr_t mapped_base)
{
    set_console(bbl_putchar, DEFAULT_KPUTS);
    return 0;
}
