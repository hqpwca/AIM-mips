/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 *
 * This file is part of AIM.
 *
 * AIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ARCH_TRAP_H
#define _ARCH_TRAP_H

// Processor-defined:
#define T_DIVIDE         0      // divide error
#define T_DEBUG          1      // debug exception
#define T_NMI            2      // non-maskable interrupt
#define T_BRKPT          3      // breakpoint
#define T_OFLOW          4      // overflow
#define T_BOUND          5      // bounds check
#define T_ILLOP          6      // illegal opcode
#define T_DEVICE         7      // device not available
#define T_DBLFLT         8      // double fault
// #define T_COPROC      9      // reserved (not used since 486)
#define T_TSS           10      // invalid task switch segment
#define T_SEGNP         11      // segment not present
#define T_STACK         12      // stack exception
#define T_GPFLT         13      // general protection fault
#define T_PGFLT         14      // page fault
// #define T_RES        15      // reserved
#define T_FPERR         16      // floating point error
#define T_ALIGN         17      // aligment check
#define T_MCHK          18      // machine check
#define T_SIMDERR       19      // SIMD floating point error

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define T_SYSCALL       64      // system call
#define T_DEFAULT      500      // catchall

#define T_PANIC			80

#define T_IRQ0          32      // IRQ 0 corresponds to int T_IRQ

#define IRQ_TIMER        0
#define IRQ_KBD          1
#define IRQ_COM1         4
#define IRQ_IDE         14
#define IRQ_ERROR       19
#define IRQ_SPURIOUS    31

#ifndef __ASSEMBLER__

struct gatedesc {
	uint32_t offset_15_0      : 16;
	uint32_t segment          : 16;
	uint32_t pad0             : 8;
	uint32_t type             : 4;
	uint32_t system           : 1;
	uint32_t privilege_level  : 2;
	uint32_t present          : 1;
	uint32_t offset_31_16     : 16;
};

static inline void set_gate(struct gatedesc *ptr, uint32_t selector, uint32_t offset, uint32_t dpl, bool trap) {
	ptr->offset_15_0 = offset & 0xFFFF;
	ptr->segment = selector;
	ptr->pad0 = 0;
	ptr->type = trap ? STS_TG32 : STS_IG32;
	ptr->system = 0;
	ptr->privilege_level = dpl;
	ptr->present = 1;
	ptr->offset_31_16 = (offset >> 16) & 0xFFFF;
}

static inline void
lidt(struct gatedesc *p, int size)
{
  volatile unsigned short pd[3];

  pd[0] = size - 1;
  pd[1] = (unsigned)p;
  pd[2] = (unsigned)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}

void trap_exec(struct trapframe *tf);

struct trapframe {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t oesp;      // useless & ignored
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	// rest of trap frame
	uint16_t gs;
	uint16_t padding1;
	uint16_t fs;
	uint16_t padding2;
	uint16_t es;
	uint16_t padding3;
	uint16_t ds;
	uint16_t padding4;
	uint32_t trapno;

	// below here defined by x86 hardware
	uint32_t err;
	uint32_t eip;
	uint16_t cs;
	uint16_t padding5;
	uint32_t eflags;

	// below here only when crossing rings, such as from user to kernel
	uint32_t esp;
	uint16_t ss;
	uint16_t padding6;
};

#endif	/* !__ASSEMBLER__ */

#endif /* _ARCH_TRAP_H */

