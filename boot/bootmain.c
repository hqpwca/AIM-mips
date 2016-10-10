/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 *
 * This file is part of AIMv6.
 *
 * AIMv6 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIMv6 is distributed in the hope that it will be useful,
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
#include <aim/boot.h>
#include <elf.h>
#include <arch/i386/arch-boot.h>

#define SECTSIZE 512

void readseg(uchar*, uint, uint);
void readsect(void *dst, uint offset);

void bootmain(void)
{
	struct elfhdr *elf;
	struct proghdr *ph, *eph;
	void (*entry)(void);
	unsigned char* pa;
	
	struct DPT *Ptable;
	
	Ptable = (struct DPT*)0x7dbe;
	
	elf = (struct elfhdr*)0x10000;  // scratch space
	
	readsect((uchar*)elf, Ptable[1].psectorID);

	// Is this an ELF executable?
	if(elf->magic != ELF_MAGIC)
		return;  // let bootasm.S handle error

	// Load each program segment (ignores ph flags).
	ph = (struct proghdr*)((uchar*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph++){
		pa = (uchar*)ph->paddr;
		readseg(pa, ph->filesz, ph->off);
		if(ph->memsz > ph->filesz)
			stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
	}

	entry = (void(*)(void))(elf->entry);
	entry();
}

void
waitdisk(void)
{
	// Wait for disk ready.
	while((inb(0x1F7) & 0xC0) != 0x40);
}

// Read a single sector at offset into dst.
void
readsect(void *dst, uint offset)
{
	// Issue command.
	waitdisk();
	outb(0x1F2, 1);   // count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

	// Read data.
	waitdisk();
	insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void
readseg(uchar* pa, uint count, uint offset)
{
	uchar* epa;

	epa = pa + count;
	
	pa -= offset % SECTSIZE;

	offset = (offset / SECTSIZE) + *(uint32_t *)0x7dd6;

	for(; pa < epa; pa += SECTSIZE, offset++)
		readsect(pa, offset);
}


