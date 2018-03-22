// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/panic.h>
#include <mach-platform.h>

void arch_local_panic(void)
{
    SBI_CALL(SBI_SHUTDOWN, 0, 0, 0);
}

