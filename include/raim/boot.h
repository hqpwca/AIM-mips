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

#ifndef _RAIM_BOOT_H
#define _RAIM_BOOT_H

#ifndef __ASSEMBLER__

extern void bputc(int c);
extern void bputs(const char *s);
extern void bputh64(uint64_t x);




#if 0



struct DPT
{
	uint8_t status;
	uint8_t st_headID;
	uint16_t st_sec_cylID;
	uint8_t fs;
	uint8_t ed_headID;
	uint16_t ed_sec_cylID;
	uint32_t psectorID;
	uint32_t psectorNum;
};

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

// Routines to let C code use special x86 instructions.

// Memory layout

#define EXTMEM  0x100000            // Start of extended memory
#define PHYSTOP 0xE000000           // Top physical memory
#define DEVSPACE 0xFE000000         // Other devices are at high addresses

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked

#define V2P(a) (((uint) (a)) - KERNBASE)
#define P2V(a) (((void *) (a)) + KERNBASE)

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts


#endif


#endif /* !__ASSEMBLER__ */

#include <arch-boot.h>

#endif /* !_RAIM_BOOT_H */


