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

#ifndef _ARCH_MMU_H
#define _ARCH_MMU_H

/* addresses before and after early MMU mapping */
#define __premap_addr(kva)	(ULCAST(kva) - KERN_BASE)
#define __postmap_addr(pa)	(ULCAST(pa) + KERN_BASE)

/* kernel virtual address and physical address conversion */
#define kva2pa(kva)		(ULCAST(kva) - KERN_BASE)
#define pa2kva(pa)		(PTRCAST(pa) + KERN_BASE)

#ifndef __ASSEMBLER__

#define PGSIZE2 0x3fffff

#define PGROUNDUP2(sz)  (((sz)+PGSIZE2-1) & ~(PGSIZE2-1))
#define PGROUNDDOWN2(a) (((a)) & ~(PGSIZE2-1))

typedef uint32_t	pde_t;
typedef uint32_t	pte_t;

/*
typedef union {
	struct
	{
		uint32_t pfaddr:20;
		uint8_t avail:3;
		uint8_t zero1:2;
		uint8_t dirty:1;
		uint8_t a:1;
		uint8_t zero2:2;
		uint8_t user_super:1;
		uint8_t read_write:1;
		uint8_t present:1;
	};
	
	pde_t value;
}pgindex_t;
*/

typedef pde_t	pgindex_t;

#endif /* !__ASSEMBLER__ */

#endif /* !_ARCH_MMU_H */

