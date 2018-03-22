/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
 * Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
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

#ifndef _MACH_PLATFORM_H
#define _MACH_PLATFORM_H

#define COM1		0x3f8
#define PORT_BASE 0x1fd00000
#define KSEG1_BASE 0xa0000000

// qemu
#define UART_BASE	0xbfd003f8//COM1+PORT_BASE+KSEG1_BASE

// real machine
//#define UART_BASE 0xbfe40000

#define UART_FREQ	1843200
//#define UART_FREQ 460800

#define EARLY_CONSOLE_BUS	(&early_memory_bus)
#define EARLY_CONSOLE_BASE	UART_BASE
#define EARLY_CONSOLE_MAPPING	MAP_NONE

#endif /* _MACH_PLATFORM_H */

