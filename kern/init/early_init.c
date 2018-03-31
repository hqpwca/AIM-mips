/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 *
 * This file is part of RAIM.
 *
 * RAIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RAIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/console.h>
#include <raim/device.h>
#include <raim/early_kmmap.h>
#include <raim/init.h>
#include <raim/mmu.h>
#include <raim/pmm.h>
#include <raim/smp.h>
#include <raim/vmm.h>
#include <raim/trap.h>
#include <raim/panic.h>
#include <raim/initcalls.h>
#include <drivers/io/io-mem.h>
#include <drivers/io/io-port.h>
#include <platform.h>

static inline
int early_devices_init(void)
{
	if (io_mem_init(&early_memory_bus) < 0)
		return EOF;

#ifdef IO_PORT_ROOT
	if (io_port_init(&early_port_bus) < 0)
		return EOF;
#endif /* IO_PORT_ROOT */
	return 0;
}

void early_mm_init()
{
	arch_mm_init();
}

__noreturn
void master_early_init(void)
{
	/* clear address-space-related callback handlers */
	early_mapping_clear();
	mmu_handlers_clear();
	arch_early_init();
	/* prepare early devices like memory bus and port bus */
	if (early_devices_init() < 0)
		goto panic;
	/* other preperations, including early secondary buses */

	if (early_console_init(
		EARLY_CONSOLE_BUS,
		EARLY_CONSOLE_BASE,
		EARLY_CONSOLE_MAPPING
	) < 0)
		panic("Early console init failed.\n");

    // you know, it's a logo
    kputs("  /\\\\\\\\\\\\\\\\\\          /\\\\\\\\\\\\\\\\\\      /\\\\\\\\\\\\\\\\\\\\\\   /\\\\\\\\        /\\\\\\\\        \n");
    kputs(" /\\\\\\///////\\\\\\      /\\\\\\\\\\\\\\\\\\\\\\\\\\   \\/////\\\\\\///   \\/\\\\\\\\\\\\    /\\\\\\\\\\\\       \n");
    kputs(" \\/\\\\\\     \\/\\\\\\     /\\\\\\/////////\\\\\\      \\/\\\\\\      \\/\\\\\\//\\\\\\/\\\\\\//\\\\\\      \n");
    kputs("  \\/\\\\\\\\\\\\\\\\\\\\\\/     \\/\\\\\\       \\/\\\\\\      \\/\\\\\\      \\/\\\\\\\\///\\\\\\/ \\/\\\\\\     \n");
    kputs("   \\/\\\\\\//////\\\\\\     \\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\      \\/\\\\\\      \\/\\\\\\  \\///   \\/\\\\\\    \n");
    kputs("    \\/\\\\\\    \\//\\\\\\    \\/\\\\\\/////////\\\\\\      \\/\\\\\\      \\/\\\\\\         \\/\\\\\\   \n");
    kputs("     \\/\\\\\\     \\//\\\\\\   \\/\\\\\\       \\/\\\\\\      \\/\\\\\\      \\/\\\\\\         \\/\\\\\\  \n");
    kputs("      \\/\\\\\\      \\//\\\\\\  \\/\\\\\\       \\/\\\\\\   /\\\\\\\\\\\\\\\\\\\\\\  \\/\\\\\\         \\/\\\\\\ \n");
    kputs("       \\///        \\///   \\///        \\///   \\///////////   \\///          \\/// \n");
	

	early_mm_init();

	mmu_handlers_apply();

	extern uint32_t high_address_entry;
	abs_jump((void *)postmap_addr(&high_address_entry));

	goto panic;
panic:
	while(1);
/*
	asm volatile("cli");
	while(1)
		asm volatile("hlt");
*/
}

__noreturn
void slave_early_init(void)
{
	kprintf("KERN CPU %d: early init\n", cpuid());

	early_mm_init();

	extern uint32_t slave_upper_entry;
	abs_jump((void *)postmap_addr(&slave_upper_entry));
}
