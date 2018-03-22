/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>	
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

#ifndef _ARCH_CONTEXT_H
#define _ARCH_CONTEXT_H

struct context {
	uint32_t esp;
	uint32_t ebp;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
};

#endif /* !_ARCH_CONTEXT_H */

