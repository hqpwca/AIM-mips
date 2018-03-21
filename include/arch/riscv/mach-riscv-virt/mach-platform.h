/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 * Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
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

#ifndef _MACH_PLATFORM_H
#define _MACH_PLATFORM_H

// we let qemu load ELF by abusing its 'initrd' feature
// initrd will be loaded at
//    bbl_entry + MIN(mem_size / 2, 128 * 1024 * 1024);
// see riscv-qemu: hw/riscv/virt.c for details

// memory size must larger than 256MB
#define KERN_ELF_ADDR 0x88000000



// dummy value
#define EARLY_CONSOLE_BUS 0
#define EARLY_CONSOLE_BASE 0
#define EARLY_CONSOLE_MAPPING 0


// ecall
#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2
#define SBI_CLEAR_IPI 3
#define SBI_SEND_IPI 4
#define SBI_REMOTE_FENCE_I 5
#define SBI_REMOTE_SFENCE_VMA 6
#define SBI_REMOTE_SFENCE_VMA_ASID 7
#define SBI_SHUTDOWN 8

#ifndef __ASSEMBLER__

#define SBI_CALL(which, arg0, arg1, arg2) ({                        \
        register uintptr_t a0 asm ("a0") = (uintptr_t)(arg0);        \
        register uintptr_t a1 asm ("a1") = (uintptr_t)(arg1);        \
        register uintptr_t a2 asm ("a2") = (uintptr_t)(arg2);        \
        register uintptr_t a7 asm ("a7") = (uintptr_t)(which);        \
        asm volatile ("ecall"                                        \
                      : "+r" (a0)                                \
                      : "r" (a1), "r" (a2), "r" (a7)                \
                      : "memory");                                \
        a0;                                                        \
})

// putchar using ecall
#define sbi_console_putchar(ch) SBI_CALL(SBI_CONSOLE_PUTCHAR, (ch), 0, 0)





#endif 

#endif /* _MACH_PLATFORM_H */
