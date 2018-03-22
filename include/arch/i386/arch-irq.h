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

#ifndef _ARCH_IRQ_H
#define _ARCH_IRQ_H

#ifndef __ASSEMBLER__

static inline void local_irq_enable()
{
	asm volatile("sti");
}

static inline void local_irq_disable()
{
	asm volatile("cli");
}

static inline uint32_t __local_irq_save(void)
{
	uint32_t res;
	asm volatile("pushf ; pop %0"
				: "=rm" (res)
				: /* no input */
				: "memory");
	local_irq_disable();
	return res;
}
#define local_irq_save(flags) \
	flags = __local_irq_save()

#define local_irq_restore(flags) \
	asm volatile("push %0 ; popf" \
				: /* no output */ \
				: "g" (flags) \
				: "memory", "cc");

#endif /* !__ASSEMBLER__ */

#endif /* _ARCH_IRQ_H */

